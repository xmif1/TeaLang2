//
// Created by Xandru Mifsud on 14/04/2021.
//

#include "semantic_analysis.h"

// For section on error recovery, see:
// https://people.montefiore.uliege.be/geurts/Cours/compil/2015/04-semantic-2015-2016.pdf

// also note that TeaLang does not do any implicit typecasting; this must be kept in mind during curr_type--checking

// ----- ERROR REPORTING UTILITY FUNCTIONS -----

// Simple function which returns a textual representation of a type symbol, with '[]' appended for array types
string semantic_analysis::type_symbol2string(string type_str, grammarDFA::Symbol obj_class){
    // note that type_str can be a named type of a tlstruct
    if(obj_class == grammarDFA::ARRAY){
        return type_str += "[]";
    }
    else{
        return type_str;
    }
}

// Simple function for obtaining a textual representation of a sequence of types (eg. of a function signature)
string semantic_analysis::typeVect_symbol2string(vector<symbol*>* typeVect){
    string ret;
    for(auto &symb : *typeVect){
        ret += (type_symbol2string(symb->type.second, symb->object_class) + ", ");
    }
    ret.pop_back(); // remove last ' '
    ret.pop_back(); // remove last ','

    return ret;
}

/* Carries out type checking across the operands of a binary operation, using type deduction whenever possible to determine
 * indeterminate types, so as to recover from type-related semantic errors whenever possible and continue reporting more
 * semantic errors. In this case, in contrast to TeaLang, type checking also includes checking the object class (i.e. if
 * a singleton value or an arrat of values) between the operands.
 *
 * Type deduction CANNOT be carried out whenever:
 * (i) Both operands have an indeterminate type,
 * (ii) or both operands have a type but they do not match.
 *
 * In the case where both operands are of an anonymous type, we also report a semantic error.
 */
void semantic_analysis::binop_type_check(astBinaryOp* binop_node){
    // temporary variables to store state for each operand evaluation
    bool op1_type_err, op2_type_err;
    grammarDFA::Symbol op1_obj_class, op2_obj_class;
    type_t op1_type, op2_type;

    // evaluate first expression (operand) and store internal state
    if(binop_node->operand1 != nullptr){
        binop_node->operand1->accept(this);
        op1_type_err = type_deduction_reqd; type_deduction_reqd = false; op1_type = curr_type; op1_obj_class = curr_obj_class;
    }
    else{
        op1_type_err = true; type_deduction_reqd = true;
    }

    // evaluate second expression (operand) and store internal state
    if(binop_node->operand2 != nullptr){
        binop_node->operand2->accept(this);
        op2_type_err = type_deduction_reqd; type_deduction_reqd = false; op2_type = curr_type; op2_obj_class = curr_obj_class;
    }
    else{
        op2_type_err = true; type_deduction_reqd = true;
    }

    // if both have anonymous types, report semantic error
    if(!op1_type_err && !op2_type_err && op1_type.first == grammarDFA::T_AUTO && op1_type == op2_type){
        type_deduction_reqd = true;

        err_count++;
        std::cerr << "ln " << binop_node->line << ": (" << binop_node->op << ") operands cannot both have an anonymous type"
        << std::endl;
    }
    // else if 1st evaluation has a valid type but 2nd has an indeterminate type
    else if(!op1_type_err && op2_type_err){
        op2_type = op1_type; // then determine type of 2nd operand from 1st
        op2_obj_class = op1_obj_class; // and determine object class of 2nd operand from 1st
        type_deduction_reqd = false;
    }
    // else if 2nd evaluation has a valid type but 1st has an indeterminate type
    else if(op1_type_err && !op2_type_err){
        op1_type = op2_type; // then determine type of 1st operand from 2nd
        op1_obj_class = op2_obj_class; // and determine object class of 1st operand from 2nd
        type_deduction_reqd = false;
    }
    else if(op1_type_err){ // else if both have an indeterminate type, report type error
        type_deduction_reqd = true;
    }
    // otherwise if types or object classes do not match, flag and report type error
    else if(op1_type != op2_type || op1_obj_class != op2_obj_class){
        type_deduction_reqd = true;

        err_count++;
        std::cerr << "ln " << binop_node->line << ": (" << binop_node->op << ") operands have mismatched types " <<
        type_symbol2string(op1_type.second, op1_obj_class) << " and " << type_symbol2string(op2_type.second, op2_obj_class)
        << std::endl;
    }
    else{ // base case
        type_deduction_reqd = false;
    }

    curr_type = op1_type;
    curr_obj_class = op1_obj_class;
}

// ----- SEMANTIC ANALYSIS VISITOR RULES -----

void semantic_analysis::visit(astTYPE* node){}

void semantic_analysis::visit(astLITERAL* node){
    type_deduction_reqd = false;
    curr_type = type_t(node->type, node->type_str);
    curr_obj_class = grammarDFA::SINGLETON;
}

// Only called when the identifier refers to an operand standing for a variable/array/struct, not for eg. a function  call or array element
void semantic_analysis::visit(astIDENTIFIER* node){
    // find symbol in lookup symbol table
    symbol* ret_symbol = lookup_symbolTable->lookup(node->lexeme);
    lookup_symbolTable = curr_symbolTable; // and point back to current symbol table

    if(ret_symbol != nullptr){ // if symbol for identifier was found, then we can determine type and object class
        curr_type = ret_symbol->type;
        curr_obj_class = ret_symbol->object_class;
        type_deduction_reqd = false;
    }
    else{
        // otherwise report semantic error and flag type deduction required (since symbol not found => no associated type)
        err_count++;
        std::cerr << "ln " << node->line << ": identifier " << node->lexeme << " has not been declared" << std::endl;
        type_deduction_reqd = true;
    }
}

// Only called when the identifier refers to an operand standing for an array element, not for eg. a function  call
void semantic_analysis::visit(astELEMENT* node){
    // maintain current state
    type_t ret_type = curr_type;
    grammarDFA::Symbol ret_obj_class = curr_obj_class;

    // if syntax analysis yielded correct AST with astIDENTIFIER node
    if(node->identifier != nullptr){
        // extract identifier of array instance from astIDENTIFIER node
        string arr_ident = ((astIDENTIFIER*) node->identifier)->lexeme;

        // find symbol in lookup symbol table
        symbol* ret_symbol = lookup_symbolTable->lookup(arr_ident);
        lookup_symbolTable = curr_symbolTable; // and point back to current symbol table

        if(ret_symbol != nullptr){ // if symbol for identifier was found, then we can determine type and object class
            if(ret_symbol->object_class != grammarDFA::ARRAY){ // if symbol does not corresond to an array, i.e. does not have elements
                // report semantic error and flag type deduction required
                err_count++;
                std::cerr << "ln " << node->line << ": " << arr_ident << " is not an array" << std::endl;
                type_deduction_reqd = true;
            }
            else{
                // else, set final type to type of array (i.e. type of the element retrieved)
                ret_type = ret_symbol->type;
                ret_obj_class = grammarDFA::SINGLETON; // element is always a singular value (since we have 1D arrays only)
                type_deduction_reqd = false;
            }
        }
        else{
            // otherwise report semantic error and flag type deduction required (since symbol not found => no associated type)
            err_count++;
            std::cerr << "ln " << node->line << ": array " << arr_ident << " has not been declared" << std::endl;
            type_deduction_reqd = true;
        }
    }else{ // otherwise if AST have missing astIDENTIFIER node, flag semantic analysis error
        type_deduction_reqd = true;
    }

    // if syntax analysis yielded correct AST with astEXPRESSION node for the index
    if(node->index != nullptr){
        (node->index)->accept(this); // visit astEXPRESSION node

        if(type_deduction_reqd){ // if could not determine type, report semantic error
            err_count++;
            std::cerr << "ln " << node->line << ": array index is of an indeterminate type" << std::endl;
        } // otherwise if expression is not an integer singular value, report semantic error
        else if(curr_type.first != grammarDFA::T_INT || curr_obj_class != grammarDFA::SINGLETON){
            err_count++;
            std::cerr << "ln " << node->line << ": array index must be an integer (is of type " <<
            type_symbol2string(curr_type.second, curr_obj_class) << " instead)" << std::endl;
        }
    }

    curr_type = ret_type;
    curr_obj_class = ret_obj_class;
}

void semantic_analysis::visit(astMULTOP* node){
    binop_type_check(node); // type check operands

    if(!type_deduction_reqd){ // if operands have consistent types, check that operation supports the type
        // mult and div ops only support int and float types
        if((node->op == "*" || node->op == "/") && curr_type.first != grammarDFA::T_INT && curr_type.first != grammarDFA::T_FLOAT){
            err_count++;
            std::cerr << "ln " << node->line << ": binary operation " << node->op <<
            " requires matching int or float operands (given " << type_symbol2string(curr_type.second, curr_obj_class)
            << " instead)" << std::endl;
        }
        // 'and' logical operator only supports bool types
        else if(node->op == "and" && curr_type.first != grammarDFA::T_BOOL){
            err_count++;
            std::cerr << "ln " << node->line << ": binary operation " << node->op <<
            " requires matching boolean operands (given " << type_symbol2string(curr_type.second, curr_obj_class)
            << " instead)" << std::endl;
        }
    }
}

void semantic_analysis::visit(astADDOP* node){
    binop_type_check(node); // type check operands

    if(!type_deduction_reqd){ // if operands have consistent types, check that operation supports the type
        // + operation supports all types besides bool and tlstruct instances (eg. + for strings is concatenation)
        if(node->op == "+" && (curr_type.first == grammarDFA::T_BOOL || curr_type.first == grammarDFA::T_TLSTRUCT)){
            err_count++;
            std::cerr << "ln " << node->line << ": binary operation " << node->op
            << " requires matching int, float, char or string operands (given "
            << type_symbol2string(curr_type.second, curr_obj_class) << " instead)" << std::endl;
        }
        // - operation supports only int, float and char types
        else if(node->op == "-" && (curr_type.first == grammarDFA::T_STRING || curr_type.first == grammarDFA::T_BOOL
                                                                      || curr_type.first == grammarDFA::T_TLSTRUCT)){
            err_count++;
            std::cerr << "ln " << node->line << ": binary operation " << node->op
            << " requires matching int, float or char operands (given "
            << type_symbol2string(curr_type.second, curr_obj_class) << " instead)" << std::endl;
        }
        // 'or' logical operator only supports bool types
        else if(node->op == "or" && curr_type.first != grammarDFA::T_BOOL){
            err_count++;
            std::cerr << "ln " << node->line << ": binary operation " << node->op
            << " requires matching boolean operands (given " << type_symbol2string(curr_type.second, curr_obj_class)
            << " instead)" << std::endl;
        }
    }
}

void semantic_analysis::visit(astRELOP* node){
    binop_type_check(node); // type check operands

    // relational operations are supported on all types except tlstruct instances
    if(!type_deduction_reqd && curr_type.first == grammarDFA::T_TLSTRUCT){
        err_count++;
        std::cerr << "ln " << node->line << ": binary operation " << node->op <<
        " requires matching int, float, char, string or boolean operanrs (given "
        << type_symbol2string(curr_type.second, curr_obj_class) << " instead)" << std::endl;
    }

    curr_type = type_t(grammarDFA::T_BOOL, "bool"); // type returned is always a bool
}

void semantic_analysis::visit(astAPARAMS* node){
    bool final_type_deduction_check = false; // remains false if all aparams yield a valid type

    // for each astEXPRESSION child node
    for(size_t i = 0; i < node->n_children; i++){
        (node->children->at(i))->accept(this); // visit astEXPRESSION node

        if(type_deduction_reqd){ // if aparam expression doesn't type check, report semantic error
            final_type_deduction_check = true;

            err_count++;
            std::cerr << "ln " << node->line << ": function " << (functionStack->top().first)->identifier << ", argument "
            << i + 1 << " is of an indeterminate type" << std::endl;
        }
        else{ // update funcSymbol on top of functionStack with an appropriate symbol containing the type and object class
              // hence this constructs the type-signature of the function to lookup
            symbol* aparam;
            if(curr_obj_class == grammarDFA::SINGLETON){ // if expression yields a singleton, we add a varSymbol
                aparam = new varSymbol(nullptr, curr_type);

                if(curr_type.first == grammarDFA::T_TLSTRUCT){ // in the case of a tlstruct instance
                    // we lookup the defined name of the tlstruct and verify that it has been declared; if not we report a semantic error
                    symbol* ret_symbol = lookup_symbolTable->lookup(curr_type.second);
                    lookup_symbolTable = curr_symbolTable;

                    if(ret_symbol != nullptr){
                        aparam->set_object(ret_symbol->object);
                    }else{
                        err_count++;
                        std::cerr << "ln " << node->line << ": tlstruct " << curr_type.second << " has not been declared" << std::endl;
                    }
                }
            }
            else{ // otherwise we add an arrSymbol
                aparam = new arrSymbol(nullptr, curr_type, 0);
            }
            
            (functionStack->top().first)->fparams->push_back(aparam);
        }
    }

    type_deduction_reqd = final_type_deduction_check;
}

void semantic_analysis::visit(astFUNC_CALL* node){
    // if syntax analysis yielded correct AST with astIDENTIFIER node
    if(node->identifier != nullptr){
        // extract function identifier from astIDENTIFIER node
        string func_ident = ((astIDENTIFIER*) node->identifier)->lexeme;
        auto* expected_func = new funcSymbol(&func_ident, type_t(grammarDFA::T_TYPE, ""),
                                             grammarDFA::FUNCTION, new vector<symbol*>(0));
        functionStack->push(make_pair(expected_func, true)); // push on top of function stack

        // in case function being called is a member of a tlstruct instance
        symbol_table* ref_lookup_symbolTable = lookup_symbolTable; // maintain reference of lookup symbol table
        lookup_symbolTable = curr_symbolTable; // set lookup symbol table to current symbol table

        type_deduction_reqd = false;
        if(node->aparams != nullptr){ // if syntax analysis yielded correct AST with astAPARMS node
            node->aparams->accept(this); // visit astAPARAMS node to type check the parameters and build the function type-signature
        }

        if(!type_deduction_reqd){
            // lookup in symbol table based on fetched identifier and type-signature constructed from visiting astAPARAMS
            funcSymbol* func = ref_lookup_symbolTable->lookup(func_ident, expected_func->fparams);

            if(func != nullptr){ // if matching funcSymbol found
                curr_type = func->type;
                curr_obj_class = func->ret_obj_class;
                // function return should not be anonymous (type should be determined from return statements in astFUNC_DECL)
                if((func->type).first == grammarDFA::T_AUTO){
                    type_deduction_reqd = true;
                }
            }
            else{ // otherwise report semantic error
                err_count++;
                std::cerr << "ln " << node->line << ": function " << func_ident << "("
                << typeVect_symbol2string(expected_func->fparams) << ") has not been declared" << std::endl;

                type_deduction_reqd = true;
            }
        }

        delete expected_func;
        functionStack->pop();
    }
    else{
        type_deduction_reqd = true;
    }
}

void semantic_analysis::visit(astSUBEXPR* node){
    // if syntax analysis yielded correct AST with astEXPRESSION node
    if(node->subexpr != nullptr){
        node->subexpr->accept(this); // visit astEXPRESSION node
    }
    else{
        type_deduction_reqd = true;
    }
}

void semantic_analysis::visit(astUNARY* node){
    // if syntax analysis yielded correct AST with astEXPRESSION node
    if(node->operand != nullptr){
        node->operand->accept(this); // visit astEXPRESSION node

        if(!type_deduction_reqd){ // if expression yielded a valid type
            // - unary operator is only supported on int, float or char types; if not, report semantic error
            if(node->op == "-" && (curr_type.first == grammarDFA::T_BOOL || curr_type.first == grammarDFA::T_STRING
                                   || curr_type.first == grammarDFA::T_TLSTRUCT || curr_type.first == grammarDFA::T_AUTO)){
                err_count++;
                std::cerr << "ln " << node->line << ": binary operation " << node->op <<
                " requires matching int, float or char types (given " << type_symbol2string(curr_type.second, curr_obj_class)
                << " instead)" << std::endl;
            }
            // 'not' logical unary operator only supports bool types; if not, report semantic error
            else if(node->op == "not" && curr_type.first != grammarDFA::T_BOOL){
                err_count++;
                std::cerr << "ln " << node->line << ": binary operation " << node->op <<
                " requires a boolean operand (given " << type_symbol2string(curr_type.second, curr_obj_class) << " instead)"
                << std::endl;
            }
        }
    }
    else{
        type_deduction_reqd = true;
    }
}

void semantic_analysis::visit(astASSIGNMENT_IDENTIFIER* node){
    // if syntax analysis yielded correct AST with astIDENTIFIER node
    if(node->identifier != nullptr){
        node->identifier->accept(this); // visit astIDENTIFIER node
        bool found_var = !type_deduction_reqd; // maintain if symbol with identifier was found

        type_t obj_type = curr_type;
        grammarDFA::Symbol obj_class = curr_obj_class;

        // if syntax analysis yielded correct AST with astEXPRESSION node
        if(node->expression != nullptr){
            node->expression->accept(this); // visit astEXPRESSION node to type check

            if(found_var){
                // if expression is of an indeterminate type, report semantic error
                if(type_deduction_reqd){
                    err_count++;
                    std::cerr << "ln " << node->line << ": " << ((astIDENTIFIER*) node->identifier)->lexeme
                    << " of type " << type_symbol2string(obj_type.second, obj_class) <<
                    " cannot be assigned to an indeterminate type" << std::endl;
                }
                // if variable/array/etc has type auto and object class of variable and expression match, set type of variable
                else if(obj_type.first == grammarDFA::T_AUTO && obj_class == curr_obj_class){
                    symbol* ret_symbol = lookup_symbolTable->lookup(((astIDENTIFIER*) node->identifier)->lexeme);
                    ret_symbol->type = curr_type;
                }
                // else if the type or object class does not match between the variable/array/etc and expression, report semantic error
                else if(curr_type != obj_type || curr_obj_class != obj_class){
                    err_count++;
                    std::cerr << "ln " << node->line << ": variable " << ((astIDENTIFIER*) node->identifier)->lexeme
                    << " of type " << type_symbol2string(obj_type.second, obj_class)
                    << " cannot be assigned a value of type " << type_symbol2string(curr_type.second, curr_obj_class) << std::endl;
                }
            }
        }
    }
}

void semantic_analysis::visit(astASSIGNMENT_ELEMENT* node){
    if(node->element != nullptr){
        symbol_table* ref_lookup_symbolTable = lookup_symbolTable;

        node->element->accept(this);
        bool found_elt = !type_deduction_reqd;
        type_t elt_type = curr_type;

        if(node->expression != nullptr){
            node->expression->accept(this);

            if(found_elt){
                string arr_ident = ((astIDENTIFIER*) ((astELEMENT*) node->element)->identifier)->lexeme;

                if(type_deduction_reqd){
                    err_count++;
                    std::cerr << "ln " << node->line << ": array " << arr_ident << " of type " <<
                    type_symbol2string(elt_type.second, grammarDFA::ARRAY)
                    << " cannot have elements assigned to an indeterminate type" << std::endl;
                }
                else if(elt_type.first == grammarDFA::T_AUTO && curr_obj_class == grammarDFA::SINGLETON){
                    symbol* ret_symbol = ref_lookup_symbolTable->lookup(arr_ident);
                    ret_symbol->type = curr_type;
                }
                else if(curr_type != elt_type || curr_obj_class != grammarDFA::SINGLETON){
                    err_count++;
                    std::cerr << "ln " << node->line << ": array " << arr_ident << " of type " <<
                    type_symbol2string(elt_type.second, grammarDFA::ARRAY)
                    << " cannot have elements assigned a value of type " << type_symbol2string(curr_type.second, curr_obj_class)
                    << std::endl;
                }
            }
        }
    }
}

void semantic_analysis::visit(astASSIGNMENT_MEMBER* node) {
    if(node->tls_name != nullptr){
        string tls_ident = ((astIDENTIFIER*) node->tls_name)->lexeme;
        symbol* ret_symbol = lookup_symbolTable->lookup(tls_ident);
        lookup_symbolTable = curr_symbolTable;

        if(ret_symbol != nullptr){
            if((ret_symbol->type).first != grammarDFA::T_TLSTRUCT || ret_symbol->object_class != grammarDFA::SINGLETON){
                err_count++;
                std::cerr << "ln " << node->line << ": variable " << tls_ident << " is not a tlstruct type" << std::endl;
            }
            else if(node->assignment != nullptr){
                lookup_symbolTable = get<symbol_table*>(get<literal_t>(ret_symbol->object));
                node->assignment->accept(this);
            }
        }
        else{
            err_count++;
            std::cerr << "ln " << node->line << ": identifier " << tls_ident << " has not been declared" << std::endl;
        }
    }
}

void semantic_analysis::visit(astVAR_DECL* node){
    string var_ident = ((astIDENTIFIER*) node->identifier)->lexeme;
    type_t var_type = type_t(((astTYPE*) node->type)->type, ((astTYPE*) node->type)->lexeme);

    if(node->expression != nullptr){
        node->expression->accept(this);
        if(type_deduction_reqd){
            err_count++;
            std::cerr << "ln " << node->line << ": variable " << var_ident << " of type " << var_type.second
            << " cannot be initialised to an indeterminate type" << std::endl;
        }
        else if(var_type.first == grammarDFA::T_AUTO && curr_obj_class == grammarDFA::SINGLETON){
            var_type = curr_type;
        }
        else if(curr_type != var_type || curr_obj_class == grammarDFA::ARRAY || curr_obj_class == grammarDFA::FUNCTION){
            err_count++;
            std::cerr << "ln " << node->line << ": variable " << var_ident << " of type " << var_type.second
            << " cannot be initialised a value of type " << type_symbol2string(curr_type.second, curr_obj_class) << std::endl;
        }
    }

    bool insert = true;
    varSymbol* var;
    var = new varSymbol(&var_ident, var_type);

    if(var_type.first == grammarDFA::T_TLSTRUCT){
        symbol* ret_symbol = lookup_symbolTable->lookup(var_type.second);
        lookup_symbolTable = curr_symbolTable;

        if(ret_symbol != nullptr){
            var->set_object(ret_symbol->object);
        }else{
            insert = false;
            err_count++;
            std::cerr << "ln " << node->line << ": tlstruct " << var_type.second << " has not been declared" << std::endl;
        }
    }

    if(insert && !curr_symbolTable->insert(var)){
        delete var;
        err_count++;
        std::cerr << "ln " << node->line << ": identifier " << var_ident << " has already been declared" << std::endl;
    }
}

void semantic_analysis::visit(astARR_DECL* node){
    string arr_ident = ((astIDENTIFIER*) node->identifier)->lexeme;
    type_t arr_type = type_t(((astTYPE*) node->type)->type, ((astTYPE*) node->type)->lexeme);

    if(arr_type.first == grammarDFA::T_TLSTRUCT){
        symbol* ret_symbol = lookup_symbolTable->lookup(arr_type.second);
        lookup_symbolTable = curr_symbolTable;

        if(ret_symbol == nullptr){
            err_count++;
            std::cerr << "ln " << node->line << ": tlstruct " << arr_type.second << " has not been declared" << std::endl;
        }
    }

    if(node->size != nullptr){
        node->size->accept(this);

        if(type_deduction_reqd){
            err_count++;
            std::cerr << "ln " << node->line << ": array size is of an indeterminate type" << std::endl;
        }
        else if(curr_type.first != grammarDFA::T_INT || curr_obj_class != grammarDFA::SINGLETON){
            err_count++;
            std::cerr << "ln " << node->line << ": array size must be an integer (is of type " <<
            type_symbol2string(curr_type.second, curr_obj_class) << " instead)" << std::endl;
        }
    }

    for(int i = 3; i < node->n_children; i++){
        if(node->children->at(i) != nullptr){
            (node->children->at(i))->accept(this);

            if(type_deduction_reqd){
                err_count++;
                std::cerr << "ln " << node->line << ": array " << arr_ident << " of type " <<
                type_symbol2string(arr_type.second, grammarDFA::ARRAY)
                << " cannot be initialised to an indeterminate type at index " << i - 3 << std::endl;
            }
            else if(arr_type.first == grammarDFA::T_AUTO && curr_obj_class == grammarDFA::SINGLETON){
                arr_type = curr_type;
            }
            else if(curr_type != arr_type || curr_obj_class != grammarDFA::SINGLETON){
                err_count++;
                std::cerr << "ln " << node->line << ": array " << arr_ident << " of type " <<
                type_symbol2string(arr_type.second, grammarDFA::ARRAY)
                << " cannot be initialised to a value of type " << type_symbol2string(curr_type.second, curr_obj_class) <<
                " at index " << i - 3 << std::endl;
            }
        }
    }

    auto* arr = new arrSymbol(&arr_ident, arr_type, 0);

    if(!curr_symbolTable->insert(arr)){
        delete arr;
        err_count++;
        std::cerr << "ln " << node->line << ": identifier " << arr_ident << " has already been declared" << std::endl;
    }
}

void semantic_analysis::visit(astTLS_DECL* node){
    string tls_ident = ((astIDENTIFIER*) node->identifier)->lexeme;
    auto* tls = new tlsSymbol(&tls_ident, &tls_ident);

    auto* ref_curr_symbolTable = curr_symbolTable;
    auto* ref_lookup_symbolTable = lookup_symbolTable;
    curr_symbolTable = new symbol_table(ref_curr_symbolTable);
    lookup_symbolTable = curr_symbolTable;

    for(auto &c : *((astBLOCK*) node->tls_block)->children){
        c->accept(this);
    }

    tls->set_object(curr_symbolTable);
    curr_symbolTable = ref_curr_symbolTable;
    lookup_symbolTable = ref_lookup_symbolTable;

    if(!curr_symbolTable->insert(tls)){
        delete tls;
        err_count++;
        std::cerr << "ln " << node->line << ": tlstruct " << tls_ident << " has already been declared" << std::endl;
    }
}

void semantic_analysis::visit(astPRINT* node){
    if(node->expression != nullptr){
        node->expression->accept(this);

        if(type_deduction_reqd){
            err_count++;
            std::cerr << "ln " << node->line << ": print operation is unsupported on indeterminate types" << std::endl;
        }
        else if(curr_type.first == grammarDFA::T_TLSTRUCT){
            err_count++;
            std::cerr << "ln " << node->line << ": print operation is unsupported on tlstruct type " <<
            type_symbol2string(curr_type.second, curr_obj_class) << std::endl;
        }
    }
}

void semantic_analysis::visit(astRETURN* node){
    if(functionStack->empty()){
        err_count++;
        std::cerr << "ln " << node->line << ": return statement cannot be outside of a function scope"<< std::endl;
    }
    else if(node->expression != nullptr){
        node->expression->accept(this);
        functionStack->top().second = true;

        if(type_deduction_reqd && curr_type.first != grammarDFA::T_AUTO){
            if((functionStack->top().first)->type.first == grammarDFA::T_AUTO){
                err_count++;
                std::cerr << "ln " << node->line << ": function " << (functionStack->top().first)->identifier << "(" <<
                typeVect_symbol2string((functionStack->top().first)->fparams) << ")"
                << " returns a value of an indeterminate type" << std::endl;
            }else{
                err_count++;
                std::cerr << "ln " << node->line << ": function " << (functionStack->top().first)->identifier << "(" <<
                typeVect_symbol2string((functionStack->top().first)->fparams) << ")"
                << " returns a value of an indeterminate type (expected "
                << type_symbol2string((functionStack->top().first)->type.second, (functionStack->top().first)->ret_obj_class)
                << " return type)" << std::endl;
            }
        }
        else if(curr_type.first != grammarDFA::T_AUTO && (functionStack->top().first)->type.first == grammarDFA::T_AUTO){
            if((functionStack->top().first)->ret_obj_class == grammarDFA::ARRAY && curr_obj_class != grammarDFA::ARRAY){
                err_count++;
                std::cerr << "ln " << node->line << ": function " << (functionStack->top().first)->identifier << "(" <<
                typeVect_symbol2string((functionStack->top().first)->fparams) << ") returns a value of type "
                << type_symbol2string(curr_type.second, curr_obj_class) << " (expected array return type)" << std::endl;
            }else{
                (functionStack->top().first)->type = curr_type;
                (functionStack->top().first)->ret_obj_class = curr_obj_class;
            }
        }
        else if(!type_deduction_reqd && (curr_type != (functionStack->top().first)->type ||
                curr_obj_class != (functionStack->top().first)->ret_obj_class)){
            err_count++;
            std::cerr << "ln " << node->line << ": function " << (functionStack->top().first)->identifier << "(" <<
            typeVect_symbol2string((functionStack->top().first)->fparams) << ") returns a value of type " <<
            type_symbol2string(curr_type.second, curr_obj_class) << " (expected " <<
            type_symbol2string((functionStack->top().first)->type.second, (functionStack->top().first)->ret_obj_class)
            << " return type)" << std::endl;
        }
    }
}

void semantic_analysis::visit(astIF* node){
    if(node->expression != nullptr){
        node->expression->accept(this);

        if(type_deduction_reqd){
            err_count++;
            std::cerr << "ln " << node->line << ": if-else conditional is of an indeterminate type (possibly not boolean)"
            << std::endl;
        }
        else if(curr_type.first != grammarDFA::T_BOOL || curr_obj_class != grammarDFA::SINGLETON){
            err_count++;
            std::cerr << "ln " << node->line << ": if-else conditional is not of boolean type (but is of type " <<
            type_symbol2string(curr_type.second, curr_obj_class) << ")" << std::endl;
        }
    }

    bool curr_func_ret, if_ret, else_ret;
    if(!functionStack->empty()){ curr_func_ret = functionStack->top().second;}

    if(node->if_block != nullptr){
        if(!functionStack->empty()){ functionStack->top().second = false;}

        node->if_block->accept(this);

        if(!functionStack->empty()){ if_ret = functionStack->top().second;}
    }
    else{
        if_ret = false;
    }

    if(node->else_block != nullptr){
        if(!functionStack->empty()){ functionStack->top().second = false;}

        node->else_block->accept(this);

        if(!functionStack->empty()){ else_ret = functionStack->top().second;}
    }
    else{
        curr_func_ret = if_ret;
    }

    if(!functionStack->empty()){
        functionStack->top().second = curr_func_ret || (if_ret && else_ret);
    }
}

void semantic_analysis::visit(astFOR* node){
    curr_symbolTable->push_scope();

    if(node->decl != nullptr){ node->decl->accept(this);}

    if(node->expression != nullptr){
        node->expression->accept(this);

        if(type_deduction_reqd){
            err_count++;
            std::cerr << "ln " << node->line << ": for-loop conditional is of an indeterminate type (possibly not boolean)"
            << std::endl;
        }
        else if(curr_type.first != grammarDFA::T_BOOL || curr_obj_class != grammarDFA::SINGLETON){
            err_count++;
            std::cerr << "ln " << node->line << ": for-loop conditional is not of boolean type (but is of type " <<
            type_symbol2string(curr_type.second, curr_obj_class) << ")" << std::endl;
        }
    }

    if(node->assignment != nullptr){ node->assignment->accept(this);}

    for(auto &c : *((astBLOCK*) node->for_block)->children){
        c->accept(this);
    }

    curr_symbolTable->pop_scope();
}

void semantic_analysis::visit(astWHILE* node){
    if(node->expression != nullptr){
        node->expression->accept(this);

        if(type_deduction_reqd){
            err_count++;
            std::cerr << "ln " << node->line << ": while-loop conditional is of an indeterminate type (possibly not boolean)"
            << std::endl;
        }
        else if(curr_type.first != grammarDFA::T_BOOL || curr_obj_class != grammarDFA::SINGLETON){
            err_count++;
            std::cerr << "ln " << node->line << ": while-loop conditional is not of boolean type (but is of type " <<
            type_symbol2string(curr_type.second, curr_obj_class) << ")" << std::endl;
        }
    }

    if(node->while_block != nullptr){ node->while_block->accept(this);}
}

void semantic_analysis::visit(astFPARAMS* node){
    for(auto &c : *node->children){
        c->accept(this);
    }
}

void semantic_analysis::visit(astFPARAM* node){
    string fparam_ident = ((astIDENTIFIER*) node->identifier)->lexeme;
    type_t fparam_type = type_t(((astTYPE*) node->type)->type, ((astTYPE*) node->type)->lexeme);
    grammarDFA::Symbol fparam_obj_class = ((astTYPE*) node->type)->object_class;

    symbol* fparam;
    if(fparam_obj_class == grammarDFA::SINGLETON){
        fparam = new varSymbol(&fparam_ident, fparam_type);

        if(fparam_type.first == grammarDFA::T_TLSTRUCT){
            symbol* ret_symbol = lookup_symbolTable->lookup(fparam_type.second);
            lookup_symbolTable = curr_symbolTable;

            if(ret_symbol != nullptr){
                fparam->set_object(ret_symbol->object);
            }else{
                err_count++;
                std::cerr << "ln " << node->line << ": tlstruct " << fparam_type.second << " has not been declared" << std::endl;
            }
        }
    }
    else{
        fparam = new arrSymbol(&fparam_ident, fparam_type, 0);
    }

    if(fparam_type.first == grammarDFA::T_AUTO){
        err_count++;
        std::cerr << "ln " << node->line << ": identifier " << fparam_ident
        << " in function signature cannot have auto type specification" << std::endl;
    }

    if(!curr_symbolTable->insert(fparam)){
        delete fparam;

        err_count++;
        std::cerr << "ln " << node->line << ": identifier " << fparam_ident
        << " in function signature has already been declared" << std::endl;
    }
}

void semantic_analysis::visit(astFUNC_DECL* node){
    string func_ident = ((astIDENTIFIER*) node->identifier)->lexeme;
    type_t ret_type = type_t(((astTYPE*) node->type)->type, ((astTYPE*) node->type)->lexeme);
    grammarDFA::Symbol ret_obj_class = ((astTYPE*) node->type)->object_class;

    if(ret_type.first == grammarDFA::T_TLSTRUCT){
        symbol* ret_symbol = lookup_symbolTable->lookup(ret_type.second);
        lookup_symbolTable = curr_symbolTable;

        if(ret_symbol == nullptr){
            err_count++;
            std::cerr << "ln " << node->line << ": tlstruct " << ret_type.second << " has not been declared" << std::endl;
        }
    }

    auto* fparams = new vector<symbol*>(0);
    if(node->fparams != nullptr){
        for(auto &c : *((astFPARAMS*) node->fparams)->children){
            string obj_ident = ((astIDENTIFIER*) ((astFPARAM*) c)->children->at(0))->lexeme;
            type_t obj_type = type_t(((astTYPE*) ((astFPARAM*) c)->children->at(1))->type,
                                     ((astTYPE*) ((astFPARAM*) c)->children->at(1))->lexeme);
            grammarDFA::Symbol obj_class = ((astTYPE*) ((astFPARAM*) c)->children->at(1))->object_class;

            if(obj_class == grammarDFA::SINGLETON){
                auto* var = new varSymbol(&obj_ident, obj_type);

                if(obj_type.first == grammarDFA::T_TLSTRUCT){
                    symbol* ret_symbol = lookup_symbolTable->lookup(obj_type.second);
                    lookup_symbolTable = curr_symbolTable;

                    if(ret_symbol != nullptr){
                        var->set_object(ret_symbol->object);
                    }else{
                        err_count++;
                        std::cerr << "ln " << node->line << ": tlstruct " << obj_type.second << " has not been declared" << std::endl;
                    }
                }

                fparams->push_back(var);
            }
            else{
                fparams->push_back(new arrSymbol(&obj_ident, obj_type, 0));
            }
        }
    }

    auto* func = new funcSymbol(&func_ident, ret_type, ret_obj_class, fparams);

    bool inserted = curr_symbolTable->insert(func);
    if(!inserted){
        err_count++;
        std::cerr << "ln " << node->line << ": identifier " << func_ident <<
        " has already been declared; possible redeclaration of function with signature ("
        << typeVect_symbol2string(fparams) << ")" << std::endl;
    }

    functionStack->push(make_pair(func, false));

    curr_symbolTable->push_scope();
    if(node->fparams != nullptr){ node->fparams->accept(this);}
    for(auto &c : *((astBLOCK*) node->function_block)->children){
        c->accept(this);
    }
    curr_symbolTable->pop_scope();

    if(!functionStack->top().second){
        err_count++;
        std::cerr << "ln " << node->line << ": function " << func_ident << "(" << typeVect_symbol2string(fparams)
        << ") does not always return" << std::endl;
    }

    functionStack->pop();

    if(func->type.first == grammarDFA::T_AUTO){
        err_count++;
        std::cerr << "ln " << node->line << ": function " << func_ident << "(" << typeVect_symbol2string(fparams)
        << ") has type auto which cannot be resolved from the return statement (possible recursive call with no base case?)"
        << std::endl;

        exit(1);
    }

    if(!inserted){
        delete func;
    }
}

void semantic_analysis::visit(astMEMBER_ACCESS* node){
    if(node->tls_name != nullptr){
        string tls_ident = ((astIDENTIFIER*) node->tls_name)->lexeme;
        symbol* ret_symbol = lookup_symbolTable->lookup(tls_ident);
        lookup_symbolTable = curr_symbolTable;

        if(ret_symbol != nullptr){
            if((ret_symbol->type).first != grammarDFA::T_TLSTRUCT || ret_symbol->object_class != grammarDFA::SINGLETON){
                err_count++;
                std::cerr << "ln " << node->line << ": variable " << tls_ident << " is not a tlstruct type" << std::endl;
            }
            else if(node->member != nullptr){
                lookup_symbolTable = get<symbol_table*>(get<literal_t>(ret_symbol->object));
                node->member->accept(this);
            }
        }
        else{
            err_count++;
            std::cerr << "ln " << node->line << ": identifier " << tls_ident << " has not been declared" << std::endl;
        }
    }
}

void semantic_analysis::visit(astBLOCK* node){
    curr_symbolTable->push_scope();

    for(auto &c : *node->children){
        c->accept(this);
    }

    curr_symbolTable->pop_scope();
}

void semantic_analysis::visit(astPROGRAM* node){
    for(auto &c : *node->children){
        c->accept(this);
    }
}
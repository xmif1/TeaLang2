//
// Created by Xandru Mifsud on 14/04/2021.
//

#include "semantic_analysis.h"

// for section on error recovery, see:
// https://people.montefiore.uliege.be/geurts/Cours/compil/2015/04-semantic-2015-2016.pdf

// also note that TeaLang does not do any implicit typecasting; this must be kept in mind during curr_type--checking

string semantic_analysis::type_symbol2string(string type_str, grammarDFA::Symbol obj_class){
    if(obj_class == grammarDFA::ARRAY){
        return type_str += "[]";
    }
    else{
        return type_str;
    }
}

string semantic_analysis::typeVect_symbol2string(vector<symbol*>* typeVect){
    string ret;
    for(auto &symb : *typeVect){
        ret += (type_symbol2string(symb->type.second, symb->object_class) + ", ");
    }
    ret.pop_back(); // remove last ' '
    ret.pop_back(); // remove last ','

    return ret;
}

void semantic_analysis::binop_type_check(astBinaryOp* binop_node){
    bool op1_type_err, op2_type_err;
    grammarDFA::Symbol op1_obj_class, op2_obj_class;
    type_t op1_type, op2_type;

    if(binop_node->operand1 != nullptr){
        binop_node->operand1->accept(this);
        op1_type_err = type_deduction_reqd; type_deduction_reqd = false; op1_type = curr_type; op1_obj_class = curr_obj_class;
    }
    else{
        op1_type_err = true; type_deduction_reqd = true;
    }

    if(binop_node->operand2 != nullptr){
        binop_node->operand2->accept(this);
        op2_type_err = type_deduction_reqd; type_deduction_reqd = false; op2_type = curr_type; op2_obj_class = curr_obj_class;
    }
    else{
        op2_type_err = true; type_deduction_reqd = true;
    }

    if(!op1_type_err && op2_type_err){
        op2_type = op1_type;
        op2_obj_class = op1_obj_class;
        type_deduction_reqd = false;
    }
    else if(op1_type_err && !op2_type_err){
        op1_type = op2_type;
        op1_obj_class = op2_obj_class;
        type_deduction_reqd = false;
    }
    else if(op1_type_err){
        type_deduction_reqd = true;
    }
    else if(op1_type != op2_type || op1_obj_class != op2_obj_class){
        type_deduction_reqd = true;

        std::cerr << "ln " << binop_node->line << ": (" << binop_node->op << ") operands have mismatched types " <<
        type_symbol2string(op1_type.second, op1_obj_class) << " and " << type_symbol2string(op2_type.second, op2_obj_class)
        << std::endl;
    }
    else{
        type_deduction_reqd = false;
    }

    curr_type = op1_type;
    curr_obj_class = op1_obj_class;
}

void semantic_analysis::visit(astTYPE* node){}

void semantic_analysis::visit(astLITERAL* node){
    type_deduction_reqd = false;
    curr_type = type_t(node->type, node->type_str);
    curr_obj_class = grammarDFA::SINGLETON;
}

// only called when the identifier refers to an operand standing for a variable/array/struct, not for eg. a function  call
void semantic_analysis::visit(astIDENTIFIER* node){
    symbol* ret_symbol = lookup_symbolTable->lookup(node->lexeme);
    lookup_symbolTable = curr_symbolTable;

    if(ret_symbol != nullptr){
        curr_type = ret_symbol->type;
        curr_obj_class = ret_symbol->object_class;
        type_deduction_reqd = false;
    }
    else{
        std::cerr << "ln " << node->line << ": identifier " << node->lexeme << " has not been declared" << std::endl;
        type_deduction_reqd = true;
    }
}

// only called when the identifier refers to an operand standing for an array element, not for eg. a function  call
void semantic_analysis::visit(astELEMENT* node){
    type_t ret_type = curr_type;
    grammarDFA::Symbol ret_obj_class = curr_obj_class;

    if(node->identifier != nullptr){
        string arr_ident = ((astIDENTIFIER*) node->identifier)->lexeme;
        symbol* ret_symbol = lookup_symbolTable->lookup(arr_ident);
        lookup_symbolTable = curr_symbolTable;

        if(ret_symbol != nullptr){
            if(ret_symbol->object_class != grammarDFA::ARRAY){
                std::cerr << "ln " << node->line << arr_ident << " is not an array" << std::endl;
                type_deduction_reqd = true;
            }
            else{
                ret_type = ret_symbol->type;
                ret_obj_class = grammarDFA::SINGLETON;
                type_deduction_reqd = false;
            }
        }
        else{
            std::cerr << "ln " << node->line << ": array " << arr_ident << " has not been declared" << std::endl;
            type_deduction_reqd = true;
        }
    }else{
        type_deduction_reqd = true;
    }

    if(node->index != nullptr){
        (node->index)->accept(this);

        if(type_deduction_reqd){
            std::cerr << "ln " << node->line << ": array index is of an indeterminate type" << std::endl;
        }
        else if(curr_type.first != grammarDFA::T_INT || curr_obj_class != grammarDFA::SINGLETON){
            std::cerr << "ln " << node->line << ": array index must be an integer (is of type " <<
                      type_symbol2string(curr_type.second, curr_obj_class) << " instead)" << std::endl;
        }
    }

    curr_type = ret_type;
    curr_obj_class = ret_obj_class;
}

void semantic_analysis::visit(astMULTOP* node){
    binop_type_check(node);

    if(!type_deduction_reqd){
        if((node->op == "*" || node->op == "/") && curr_type.first != grammarDFA::T_INT && curr_type.first != grammarDFA::T_FLOAT){
            std::cerr << "ln " << node->line << ": binary operation " << node->op <<
            " requires matching int or float operands (given " << type_symbol2string(curr_type.second, curr_obj_class)
            << " instead)" << std::endl;
        }
        else if(node->op == "and" && curr_type.first != grammarDFA::T_BOOL){
            std::cerr << "ln " << node->line << ": binary operation " << node->op <<
            " requires matching boolean operands (given " << type_symbol2string(curr_type.second, curr_obj_class)
            << " instead)" << std::endl;
        }
    }
}

void semantic_analysis::visit(astADDOP* node){
    binop_type_check(node);

    if(!type_deduction_reqd){
        if(node->op == "+" && (curr_type.first == grammarDFA::T_BOOL || curr_type.first == grammarDFA::T_TLSTRUCT)){
            std::cerr << "ln " << node->line << ": binary operation " << node->op
            << " requires matching int, float, char or string operands (given "
            << type_symbol2string(curr_type.second, curr_obj_class) << " instead)" << std::endl;
        }
        else if(node->op == "-" && (curr_type.first == grammarDFA::T_STRING || curr_type.first == grammarDFA::T_BOOL
                                                                      || curr_type.first == grammarDFA::T_TLSTRUCT)){
            std::cerr << "ln " << node->line << ": binary operation " << node->op
            << " requires matching int, float or char operands (given "
            << type_symbol2string(curr_type.second, curr_obj_class) << " instead)" << std::endl;
        }
        else if(node->op == "or" && curr_type.first != grammarDFA::T_BOOL){
            std::cerr << "ln " << node->line << ": binary operation " << node->op
            << " requires matching boolean operands (given " << type_symbol2string(curr_type.second, curr_obj_class)
            << " instead)" << std::endl;
        }
    }
}

void semantic_analysis::visit(astRELOP* node){
    binop_type_check(node);

    if(!type_deduction_reqd && curr_type.first == grammarDFA::T_TLSTRUCT){
        std::cerr << "ln " << node->line << ": binary operation " << node->op <<
        " requires matching int, float, char, string or boolean operanrs (given "
        << type_symbol2string(curr_type.second, curr_obj_class) << " instead)" << std::endl;
    }

    curr_type = type_t(grammarDFA::T_BOOL, "bool");
}

void semantic_analysis::visit(astAPARAMS* node){
    bool final_type_deduction_check = false;

    for(size_t i = 0; i < node->n_children; i++){
        (node->children->at(i))->accept(this);

        if(type_deduction_reqd){
            final_type_deduction_check = true;

            std::cerr << "ln " << node->line << ": function " << (functionStack->top().first)->identifier << ", argument "
            << i + 1 << " is of an indeterminate type" << std::endl;
        }
        else{
            symbol* aparam;
            if(curr_obj_class == grammarDFA::SINGLETON){
                aparam = new varSymbol(nullptr, curr_type);

                if(curr_type.first == grammarDFA::T_TLSTRUCT){
                    symbol* ret_symbol = lookup_symbolTable->lookup(curr_type.second);
                    lookup_symbolTable = curr_symbolTable;

                    if(ret_symbol != nullptr){
                        aparam->set_object(ret_symbol->object);
                    }else{
                        std::cerr << "ln " << node->line << ": tlstruct " << curr_type.second << " has not been declared" << std::endl;
                    }
                }
            }
            else{
                aparam = new arrSymbol(nullptr, curr_type, 0);
            }
            
            (functionStack->top().first)->fparams->push_back(aparam);
        }
    }

    type_deduction_reqd = final_type_deduction_check;
}

void semantic_analysis::visit(astFUNC_CALL* node){
    if(node->identifier != nullptr){
        string func_ident = ((astIDENTIFIER*) node->identifier)->lexeme;
        auto* expected_func = new funcSymbol(&func_ident, type_t(grammarDFA::T_TYPE, ""),
                                             grammarDFA::FUNCTION, new vector<symbol*>(0));
        functionStack->push(make_pair(expected_func, true));

        symbol_table* ref_lookup_symbolTable = lookup_symbolTable;
        lookup_symbolTable = curr_symbolTable;

        type_deduction_reqd = false;
        if(node->aparams != nullptr){
            node->aparams->accept(this);
        }

        if(!type_deduction_reqd){
            funcSymbol* func = ref_lookup_symbolTable->lookup(func_ident, expected_func->fparams);

            if(func != nullptr){
                curr_type = func->type;
                curr_obj_class = func->ret_obj_class;
                if((func->type).first == grammarDFA::T_AUTO){
                    type_deduction_reqd = true;
                }
            }
            else{
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
    if(node->subexpr != nullptr){
        node->subexpr->accept(this);
    }
    else{
        type_deduction_reqd = true;
    }
}

void semantic_analysis::visit(astUNARY* node){
    if(node->operand != nullptr){
        node->operand->accept(this);

        if(!type_deduction_reqd){
            if(node->op == "-" && (curr_type.first == grammarDFA::T_BOOL || curr_type.first == grammarDFA::T_STRING
                                                                   || curr_type.first == grammarDFA::T_TLSTRUCT)){
                std::cerr << "ln " << node->line << ": binary operation " << node->op <<
                " requires matching int, float or char types (given " << type_symbol2string(curr_type.second, curr_obj_class)
                << " instead)" << std::endl;
            }
            else if(node->op == "not" && curr_type.first != grammarDFA::T_BOOL){
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
    if(node->identifier != nullptr){
        node->identifier->accept(this);
        bool found_var = !type_deduction_reqd;

        type_t obj_type = curr_type;
        grammarDFA::Symbol obj_class = curr_obj_class;

        if(node->expression != nullptr){
            node->expression->accept(this);
            if(found_var){
                if(type_deduction_reqd){
                    std::cerr << "ln " << node->line << ": " << ((astIDENTIFIER*) node->identifier)->lexeme
                    << " of type " << type_symbol2string(obj_type.second, obj_class) <<
                    " cannot be assigned to an indeterminate type" << std::endl;
                }
                else if(curr_type != obj_type || curr_obj_class != obj_class){
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
                    std::cerr << "ln " << node->line << ": array " << arr_ident << " of type " <<
                    type_symbol2string(elt_type.second, grammarDFA::ARRAY)
                    << " cannot have elements assigned to an indeterminate type" << std::endl;
                }
                else if(elt_type.first == grammarDFA::T_AUTO && curr_obj_class == grammarDFA::SINGLETON){
                    symbol* ret_symbol = ref_lookup_symbolTable->lookup(arr_ident);
                    ret_symbol->type = curr_type;
                }
                else if(curr_type != elt_type || curr_obj_class != grammarDFA::SINGLETON){
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
                std::cerr << "ln " << node->line << ": variable " << tls_ident << " is not a tlstruct type" << std::endl;
            }
            else if(node->assignment != nullptr){
                lookup_symbolTable = get<symbol_table*>(get<literal_t>(ret_symbol->object));
                node->assignment->accept(this);
            }
        }
        else{
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
            std::cerr << "ln " << node->line << ": variable " << var_ident << " of type " << var_type.second
            << " cannot be initialised to an indeterminate type" << std::endl;
        }
        else if(var_type.first == grammarDFA::T_AUTO && curr_obj_class == grammarDFA::SINGLETON){
            var_type = curr_type;
        }
        else if(curr_type != var_type || curr_obj_class == grammarDFA::ARRAY || curr_obj_class == grammarDFA::FUNCTION){
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
            std::cerr << "ln " << node->line << ": tlstruct " << var_type.second << " has not been declared" << std::endl;
        }
    }

    if(insert && !curr_symbolTable->insert(var)){
        delete var;
        std::cerr << "ln " << node->line << ": identifier " << var_ident << " has already been declared" << std::endl;
    }
}

void semantic_analysis::visit(astARR_DECL* node){
    string arr_ident = ((astIDENTIFIER*) node->identifier)->lexeme;
    type_t arr_type = type_t(((astTYPE*) node->type)->type, ((astTYPE*) node->type)->lexeme);

    if(node->size != nullptr){
        node->size->accept(this);

        if(type_deduction_reqd){
            std::cerr << "ln " << node->line << ": array size is of an indeterminate type" << std::endl;
        }
        else if(curr_type.first != grammarDFA::T_INT || curr_obj_class != grammarDFA::SINGLETON){
            std::cerr << "ln " << node->line << ": array size must be an integer (is of type " <<
            type_symbol2string(curr_type.second, curr_obj_class) << " instead)" << std::endl;
        }
    }

    for(int i = 3; i < node->n_children; i++){
        if(node->children->at(i) != nullptr){
            (node->children->at(i))->accept(this);

            if(type_deduction_reqd){
                std::cerr << "ln " << node->line << ": array " << arr_ident << " of type " <<
                type_symbol2string(arr_type.second, grammarDFA::ARRAY)
                << " cannot be initialised to an indeterminate type at index " << i - 3 << std::endl;
            }
            else if(arr_type.first == grammarDFA::T_AUTO && curr_obj_class == grammarDFA::SINGLETON){
                arr_type = curr_type;
            }
            else if(curr_type != arr_type || curr_obj_class != grammarDFA::SINGLETON){
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
        std::cerr << "ln " << node->line << ": identifier " << arr_ident << " has already been declared" << std::endl;
    }
}

void semantic_analysis::visit(astTLS_DECL* node){
    string tls_ident = ((astIDENTIFIER*) node->identifier)->lexeme;
    auto* tls = new tlsSymbol(&tls_ident, &tls_ident);

    auto* ref_curr_symbolTable = curr_symbolTable;
    auto* ref_lookup_symbolTable = lookup_symbolTable;
    curr_symbolTable = new symbol_table();
    lookup_symbolTable = curr_symbolTable;

    for(auto &c : *((astBLOCK*) node->tls_block)->children){
        c->accept(this);
    }

    tls->set_object(curr_symbolTable);
    curr_symbolTable = ref_curr_symbolTable;
    lookup_symbolTable = ref_lookup_symbolTable;

    if(!curr_symbolTable->insert(tls)){
        delete tls;
        std::cerr << "ln " << node->line << ": tlstruct " << tls_ident << " has already been declared" << std::endl;
    }
}

void semantic_analysis::visit(astPRINT* node){
    if(node->expression != nullptr){
        node->expression->accept(this);

        if(type_deduction_reqd){
            std::cerr << "ln " << node->line << ": print operation is unsupported on indeterminate types" << std::endl;
        }
        else if(curr_type.first == grammarDFA::T_TLSTRUCT){
            std::cerr << "ln " << node->line << ": print operation is unsupported on tlstruct type " <<
            type_symbol2string(curr_type.second, curr_obj_class) << std::endl;
        }
    }
}

void semantic_analysis::visit(astRETURN* node){
    if(functionStack->empty()){
        std::cerr << "ln " << node->line << ": return statement cannot be outside of a function scope"<< std::endl;
    }
    else if(node->expression != nullptr){
        node->expression->accept(this);
        functionStack->top().second = true;

        if(type_deduction_reqd && curr_type.first != grammarDFA::T_AUTO){
            if((functionStack->top().first)->type.first == grammarDFA::T_AUTO){
                std::cerr << "ln " << node->line << ": function " << (functionStack->top().first)->identifier << "(" <<
                typeVect_symbol2string((functionStack->top().first)->fparams) << ")"
                << " returns a value of an indeterminate type" << std::endl;
            }else{
                std::cerr << "ln " << node->line << ": function " << (functionStack->top().first)->identifier << "(" <<
                typeVect_symbol2string((functionStack->top().first)->fparams) << ")"
                << " returns a value of an indeterminate type (expected "
                << type_symbol2string((functionStack->top().first)->type.second, (functionStack->top().first)->ret_obj_class)
                << " return type)" << std::endl;
            }
        }
        else if(curr_type.first != grammarDFA::T_AUTO && (functionStack->top().first)->type.first == grammarDFA::T_AUTO){
            if((functionStack->top().first)->ret_obj_class == grammarDFA::ARRAY && curr_obj_class != grammarDFA::ARRAY){
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
            std::cerr << "ln " << node->line << ": if-else conditional is of an indeterminate type (possibly not boolean)"
            << std::endl;
        }
        else if(curr_type.first != grammarDFA::T_BOOL || curr_obj_class != grammarDFA::SINGLETON){
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
            std::cerr << "ln " << node->line << ": for-loop conditional is of an indeterminate type (possibly not boolean)"
            << std::endl;
        }
        else if(curr_type.first != grammarDFA::T_BOOL || curr_obj_class != grammarDFA::SINGLETON){
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
            std::cerr << "ln " << node->line << ": while-loop conditional is of an indeterminate type (possibly not boolean)"
                      << std::endl;
        }
        else if(curr_type.first != grammarDFA::T_BOOL || curr_obj_class != grammarDFA::SINGLETON){
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
                std::cerr << "ln " << node->line << ": tlstruct " << fparam_type.second << " has not been declared" << std::endl;
            }
        }
    }
    else{
        fparam = new arrSymbol(&fparam_ident, fparam_type, 0);
    }

    if(fparam_type.first == grammarDFA::T_AUTO){
        std::cerr << "ln " << node->line << ": identifier " << fparam_ident
        << " in function signature cannot have auto type specification" << std::endl;
    }

    if(!curr_symbolTable->insert(fparam)){
        delete fparam;

        std::cerr << "ln " << node->line << ": identifier " << fparam_ident
        << " in function signature has already been declared" << std::endl;
    }
}

void semantic_analysis::visit(astFUNC_DECL* node){
    string func_ident = ((astIDENTIFIER*) node->identifier)->lexeme;
    type_t ret_type = type_t(((astTYPE*) node->type)->type, ((astTYPE*) node->type)->lexeme);
    grammarDFA::Symbol ret_obj_class = ((astTYPE*) node->type)->object_class;

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
        std::cerr << "ln " << node->line << ": function " << func_ident << "(" << typeVect_symbol2string(fparams)
        << ") does not always return" << std::endl;
    }

    functionStack->pop();

    if(func->type.first == grammarDFA::T_AUTO){
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
                std::cerr << "ln " << node->line << ": variable " << tls_ident << " is not a tlstruct type" << std::endl;
            }
            else if(node->member != nullptr){
                lookup_symbolTable = get<symbol_table*>(get<literal_t>(ret_symbol->object));
                node->member->accept(this);
            }
        }
        else{
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
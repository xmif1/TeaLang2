//
// Created by Xandru Mifsud on 14/04/2021.
//

#include "semantic_analysis.h"

// for section on error recovery, see:
// https://people.montefiore.uliege.be/geurts/Cours/compil/2015/04-semantic-2015-2016.pdf

// also note that TeaLang does not do any implicit typecasting; this must be kept in mind during curr_type--checking

string semantic_analysis::type_symbol2string(grammarDFA::Symbol type){
    if(type == grammarDFA::T_BOOL){
        return "bool";
    }
    else if(type == grammarDFA::T_INT){
        return "int";
    }
    else if(type == grammarDFA::T_FLOAT){
        return "float";
    }
    else if(type == grammarDFA::T_STRING){
        return "string";
    }
    else if(type == grammarDFA::T_CHAR){
        return "char";
    }
    else{
        return "auto";
    }
}

string semantic_analysis::typeVect_symbol2string(vector<varSymbol*>* varVect){
    string ret;
    for(auto &var : *varVect){
        ret += (type_symbol2string(var->type) + ", ");
    }
    ret.pop_back(); // remove last ' '
    ret.pop_back(); // remove last ','

    return ret;
}

grammarDFA::Symbol semantic_analysis::binop_type_check(astBinaryOp* binop_node){
    bool op1_type_err, op2_type_err;
    grammarDFA::Symbol op1_type, op2_type;

    if(binop_node->operand1 != nullptr){
        binop_node->operand1->accept(this);
        op1_type_err = type_deduction_reqd; type_deduction_reqd = false; op1_type = curr_type;
    }
    else{
        op1_type_err = true; type_deduction_reqd = true;
    }

    if(binop_node->operand2 != nullptr){
        binop_node->operand2->accept(this);
        op2_type_err = type_deduction_reqd; type_deduction_reqd = false; op2_type = curr_type;
    }
    else{
        op2_type_err = true; type_deduction_reqd = true;
    }

    if(!op1_type_err && op2_type_err){
        op2_type = op1_type;
        type_deduction_reqd = false;
    }
    else if(op1_type_err && !op2_type_err){
        op1_type = op2_type;
        type_deduction_reqd = false;
    }
    else if(op1_type_err){
        type_deduction_reqd = true;
    }
    else if(op1_type != op2_type){
        type_deduction_reqd = true;
        std::cerr << "ln " << binop_node->line << ": (" << binop_node->op << ") operands have mismatched types " << type_symbol2string(op1_type) <<
                  " and " << type_symbol2string(op2_type) << std::endl;
    }
    else{
        type_deduction_reqd = false;
    }

    return op1_type;
}

void semantic_analysis::visit(astTYPE* node){}

void semantic_analysis::visit(astLITERAL* node){
    type_deduction_reqd = false;
    curr_type = node->type;
}

// only called when the identifier refers to an operand standing for a variable, not for eg. a function  call
void semantic_analysis::visit(astIDENTIFIER* node){
    if(varSymbol* ret_var = symbolTable->lookup(node->lexeme); ret_var != nullptr){
        curr_type = ret_var->type;
        type_deduction_reqd = false;
    }
    else{
        std::cerr << "ln " << node->line << ": identifier " << node->lexeme << " has not been declared" << std::endl;
        type_deduction_reqd = true;
    }
}

void semantic_analysis::visit(astMULTOP* node){
    grammarDFA::Symbol op1_type = binop_type_check(node);

    if(!type_deduction_reqd){
        if((node->op == "*" || node->op == "/") && op1_type != grammarDFA::T_INT && op1_type != grammarDFA::T_FLOAT){
            std::cerr << "ln " << node->line << ": binary operation " << node->op <<
            " requires matching int or float operands (given " << type_symbol2string(op1_type) << " instead)" << std::endl;
        }
        else if(node->op == "and" && op1_type != grammarDFA::T_BOOL){
            std::cerr << "ln " << node->line << ": binary operation " << node->op <<
            " requires matching boolean operands (given " << type_symbol2string(op1_type) << " instead)" << std::endl;
        }
        else{
            curr_type = op1_type;
        }
    }
}

void semantic_analysis::visit(astADDOP* node){
    grammarDFA::Symbol op1_type = binop_type_check(node);

    if(!type_deduction_reqd){
        if(node->op == "+" && op1_type == grammarDFA::T_BOOL){
            std::cerr << "ln " << node->line << ": binary operation " << node->op
                      << " requires matching int, float, char or string operands (given " << type_symbol2string(op1_type)
                      << " instead)" << std::endl;
        }
        else if(node->op == "-" && (op1_type == grammarDFA::T_STRING || op1_type == grammarDFA::T_BOOL)){
            std::cerr << "ln " << node->line << ": binary operation " << node->op
                      << " requires matching int, float or char operands (given " << type_symbol2string(op1_type)
                      << " instead)" << std::endl;
        }
        else if(node->op == "or" && op1_type != grammarDFA::T_BOOL){
            std::cerr << "ln " << node->line << ": binary operation " << node->op
                      << " requires matching boolean operands (given " << type_symbol2string(op1_type) << " instead)"
                      << std::endl;
        }
        else{
            curr_type = op1_type;
        }
    }
}

void semantic_analysis::visit(astRELOP* node){
    grammarDFA::Symbol op1_type = binop_type_check(node);

    if(!type_deduction_reqd){
        curr_type = grammarDFA::T_BOOL;
    }
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
            auto* var = new varSymbol(nullptr, curr_type);
            (functionStack->top().first)->fparams->push_back(var);
        }
    }

    type_deduction_reqd = final_type_deduction_check;
}

void semantic_analysis::visit(astFUNC_CALL* node){
    if(node->identifier != nullptr){
        string func_ident = ((astIDENTIFIER*) node->identifier)->lexeme;
        auto* expected_func = new funcSymbol(&func_ident, grammarDFA::T_TYPE, new vector<varSymbol*>(0));
        functionStack->push(make_pair(expected_func, true));

        if(node->aparams != nullptr){
            node->aparams->accept(this);

            if(!type_deduction_reqd){
                funcSymbol* func = symbolTable->lookup(func_ident, expected_func->fparams);

                if(func != nullptr){
                    curr_type = func->type;
                    if(func->type == grammarDFA::T_AUTO){
                        type_deduction_reqd = true;
                    }
                }
                else{
                    std::cerr << "ln " << node->line << ": function " << func_ident << "("
                    << typeVect_symbol2string(expected_func->fparams) << ") has not been declared" << std::endl;

                    type_deduction_reqd = true;
                }
            }
        }
        else{
            type_deduction_reqd = true;
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
            if(node->op == "-" && curr_type == grammarDFA::T_BOOL && curr_type != grammarDFA::T_STRING){
                std::cerr << "ln " << node->line << ": binary operation " << node->op <<
                " requires matching int, float or char types (given " << type_symbol2string(curr_type) << " instead)" << std::endl;
            }
            else if(node->op == "not" && curr_type != grammarDFA::T_BOOL){
                std::cerr << "ln " << node->line << ": binary operation " << node->op <<
                " requires a boolean operand (given " << type_symbol2string(curr_type) << " instead)" << std::endl;
            }
        }
    }
    else{
        type_deduction_reqd = true;
    }
}

void semantic_analysis::visit(astASSIGNMENT* node){
    if(node->identifier != nullptr){
        node->identifier->accept(this);
        bool found_var = !type_deduction_reqd;
        grammarDFA::Symbol var_type = curr_type;

        if(node->expression != nullptr){
            node->expression->accept(this);
            if(found_var){
                if(type_deduction_reqd){
                    std::cerr << "ln " << node->line << ": variable " << ((astIDENTIFIER*) node->identifier)->lexeme
                              << " of type " << type_symbol2string(var_type) << " cannot be assigned to an indeterminate type" << std::endl;
                }
                else if(curr_type != var_type){
                    std::cerr << "ln " << node->line << ": variable " << ((astIDENTIFIER*) node->identifier)->lexeme
                              << " of type " << type_symbol2string(var_type) << " cannot be assigned a value of type " <<
                              type_symbol2string(curr_type) << std::endl;
                }
            }
        }
    }
}

void semantic_analysis::visit(astVAR_DECL* node){
    string var_ident = ((astIDENTIFIER*) node->identifier)->lexeme;
    grammarDFA::Symbol var_type = ((astTYPE*) node->type)->type;

    if(node->expression != nullptr){
        node->expression->accept(this);
        if(type_deduction_reqd){
            std::cerr << "ln " << node->line << ": variable " << var_ident << " of type " << ((astTYPE*) node->type)->lexeme
                      << " cannot be initialised to an indeterminate type" << std::endl;
        }
        else if(var_type == grammarDFA::T_AUTO){
            var_type = curr_type;
        }
        else if(curr_type != var_type){
            std::cerr << "ln " << node->line << ": variable " << var_ident << " of type " << ((astTYPE*) node->type)->lexeme
                      << " cannot be initialised to value of type " << type_symbol2string(curr_type) << std::endl;
        }
    }

    auto* var = new varSymbol(&var_ident, var_type);

    if(!symbolTable->insert(var)){
        delete var;
        std::cerr << "ln " << node->line << ": identifier " << var_ident << " has already been declared" << std::endl;
    }
}

void semantic_analysis::visit(astPRINT* node){
    if(node->expression != nullptr){
        node->expression->accept(this);
    }
}

void semantic_analysis::visit(astRETURN* node){
    if(functionStack->empty()){
        std::cerr << "ln " << node->line << ": return statement cannot be outside of a function scope"<< std::endl;
    }
    else if(node->expression != nullptr){
        node->expression->accept(this);
        functionStack->top().second = true;

        if(type_deduction_reqd && curr_type != grammarDFA::T_AUTO){
            if((functionStack->top().first)->type == grammarDFA::T_AUTO){
                std::cerr << "ln " << node->line << ": function " << (functionStack->top().first)->identifier << "(" <<
                typeVect_symbol2string((functionStack->top().first)->fparams) << ")"
                << " returns a value of an indeterminate type" << std::endl;
            }else{
                std::cerr << "ln " << node->line << ": function " << (functionStack->top().first)->identifier << "(" <<
                typeVect_symbol2string((functionStack->top().first)->fparams) << ")"
                << " returns a value of an indeterminate type (expected " << type_symbol2string((functionStack->top().first)->type)
                << " return type)" << std::endl;
            }
        }
        else if(curr_type != grammarDFA::T_AUTO && (functionStack->top().first)->type == grammarDFA::T_AUTO){
            (functionStack->top().first)->type = curr_type;
        }
        else if(!type_deduction_reqd && curr_type != (functionStack->top().first)->type){
            std::cerr << "ln " << node->line << ": function " << (functionStack->top().first)->identifier << "(" <<
            typeVect_symbol2string((functionStack->top().first)->fparams) << ") returns a value of type " <<
            type_symbol2string(curr_type) << " (expected " << type_symbol2string((functionStack->top().first)->type)
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
        else if(curr_type != grammarDFA::T_BOOL){
            std::cerr << "ln " << node->line << ": if-else conditional is not of boolean type" << std::endl;
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
    symbolTable->push_scope();

    if(node->var_decl != nullptr){ node->var_decl->accept(this);}

    if(node->expression != nullptr){
        node->expression->accept(this);

        if(type_deduction_reqd){
            std::cerr << "ln " << node->line << ": for-loop conditional is of an indeterminate type (possibly not boolean)"
            << std::endl;
        }
        else if(curr_type != grammarDFA::T_BOOL){
            std::cerr << "ln " << node->line << ": for-loop conditional is not of boolean type" << std::endl;
        }
    }

    if(node->assignment != nullptr){ node->assignment->accept(this);}

    for(auto &c : *((astBLOCK*) node->for_block)->children){
        c->accept(this);
    }

    symbolTable->pop_scope();
}

void semantic_analysis::visit(astWHILE* node){
    if(node->expression != nullptr){
        node->expression->accept(this);

        if(type_deduction_reqd){
            std::cerr << "ln " << node->line << ": while-loop conditional is of an indeterminate type (possibly not boolean)"
                      << std::endl;
        }
        else if(curr_type != grammarDFA::T_BOOL){
            std::cerr << "ln " << node->line << ": while-loop conditional is not of boolean type" << std::endl;
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
    grammarDFA::Symbol fparam_type = ((astTYPE*) node->type)->type;
    auto* fparam = new varSymbol(&fparam_ident, fparam_type);

    if(fparam_type == grammarDFA::T_AUTO){
        std::cerr << "ln " << node->line << ": identifier " << fparam_ident
        << " in function signature cannot have auto type specification" << std::endl;
    }

    if(!symbolTable->insert(fparam)){
        delete fparam;

        std::cerr << "ln " << node->line << ": identifier " << fparam_ident
        << " in function signature has already been declared" << std::endl;
    }
}

void semantic_analysis::visit(astFUNC_DECL* node){
    string func_ident = ((astIDENTIFIER*) node->identifier)->lexeme;
    grammarDFA::Symbol ret_type = ((astTYPE*) node->type)->type;

    auto* fparams = new vector<varSymbol*>(0);
    if(node->fparams != nullptr){
        for(auto &c : *((astFPARAMS*) node->fparams)->children){
            string var_ident = ((astIDENTIFIER*) ((astFPARAM*) c)->children->at(0))->lexeme;
            grammarDFA::Symbol var_type = ((astTYPE*) ((astFPARAM*) c)->children->at(1))->type;
            fparams->push_back(new varSymbol(&var_ident, var_type));
        }
    }

    auto* func = new funcSymbol(&func_ident, ret_type, fparams);

    bool inserted = symbolTable->insert(func);
    if(!inserted){
        std::cerr << "ln " << node->line << ": identifier " << func_ident <<
                  " has already been declared; possible redeclaration of function with signature ("
                  << typeVect_symbol2string(fparams) << ")" << std::endl;
    }

    functionStack->push(make_pair(func, false));

    symbolTable->push_scope();
    if(node->fparams != nullptr){ node->fparams->accept(this);}
    for(auto &c : *((astBLOCK*) node->function_block)->children){
        c->accept(this);
    }
    symbolTable->pop_scope();

    if(!functionStack->top().second){
        std::cerr << "ln " << node->line << ": function " << func_ident << "(" << typeVect_symbol2string(fparams)
        << ") does not always return" << std::endl;
    }

    functionStack->pop();

    if(func->type == grammarDFA::T_AUTO){
        std::cerr << "ln " << node->line << ": function " << func_ident << "(" << typeVect_symbol2string(fparams)
        << ") has type auto which cannot be resolved from the return statement (possible recursive call with no base case?)"
        << std::endl;

        exit(1);
    }

    if(!inserted){
        delete func;
    }
}

void semantic_analysis::visit(astBLOCK* node){
    symbolTable->push_scope();

    for(auto &c : *node->children){
        c->accept(this);
    }

    symbolTable->pop_scope();
}

void semantic_analysis::visit(astPROGRAM* node){
    for(auto &c : *node->children){
        c->accept(this);
    }
}
//
// Created by Xandru Mifsud on 17/04/2021.
//

#include "interpreter.h"

void interpreter::visit(astTYPE* node){}

void interpreter::visit(astLITERAL* node){
    curr_type = node->type;

    if(curr_type == grammarDFA::T_BOOL){
        if(node->lexeme == "true"){
            curr_result = true;
        }
        else{
            curr_result = false;
        }
    }
    else if(curr_type == grammarDFA::T_INT){
        curr_result = stoi(node->lexeme);
    }
    else if(curr_type == grammarDFA::T_FLOAT){
        curr_result = stof(node->lexeme);
    }
    else if(curr_type == grammarDFA::T_CHAR){
        string literal_cpy = node->lexeme;

        std::size_t opening_apost = literal_cpy.find_first_of('\'');
        if(opening_apost != string::npos){
            literal_cpy.erase(opening_apost, 1);
        }

        std::size_t closing_apost = literal_cpy.find_last_of('\'');
        if(closing_apost != string::npos){
            literal_cpy.erase(closing_apost, 1);
        }

        if(literal_cpy[0] == '\\'){
            if(literal_cpy[1] == '0'){
                curr_result = '\0';
            }
            else if(literal_cpy[1] == '\\'){
                curr_result = '\\';
            }
            else if(literal_cpy[1] == '\''){
                curr_result = '\'';
            }
            else if(literal_cpy[1] == '"'){
                curr_result = '\"';
            }
            else if(literal_cpy[1] == 'n'){
                curr_result = '\n';
            }
            else if(literal_cpy[1] == 't'){
                curr_result = '\t';
            }
            else if(literal_cpy[1] == 'r'){
                curr_result = '\r';
            }
            else if(literal_cpy[1] == 'b'){
                curr_result = '\b';
            }
            else if(literal_cpy[1] == 'f'){
                curr_result = '\f';
            }
            else{
                curr_result = '\v';
            }
        }
        else{
            curr_result = (char) literal_cpy[0];
        }
    }
    else{
        string literal_cpy = node->lexeme;

        std::size_t opening_dquotes = literal_cpy.find_first_of('\"');
        if(opening_dquotes != string::npos){
            literal_cpy.erase(opening_dquotes, 1);
        }

        std::size_t closing_dquotes = literal_cpy.find_last_of('\"');
        if(closing_dquotes != string::npos){
            literal_cpy.erase(closing_dquotes, 1);
        }

        curr_result = literal_cpy;
    }
}

// only called when the identifier refers to an operand standing for a variable, not for eg. a function  call
void interpreter::visit(astIDENTIFIER* node){
    varSymbol* ret_var = symbolTable->lookup(node->lexeme);
    curr_type = ret_var->type;

    if(curr_type == grammarDFA::T_BOOL){
        curr_result = get<bool>(ret_var->literal);
    }
    else if(curr_type == grammarDFA::T_INT){
        curr_result = get<int>(ret_var->literal);
    }
    else if(curr_type == grammarDFA::T_FLOAT){
        curr_result = get<float>(ret_var->literal);
    }
    else if(curr_type == grammarDFA::T_CHAR){
        curr_result = get<char>(ret_var->literal);
    }
    else{
        string literal_cpy = get<string>(ret_var->literal);

        std::size_t opening_dquotes = literal_cpy.find_first_of('\"');
        if(opening_dquotes != string::npos){
            literal_cpy.erase(opening_dquotes, 1);
        }

        std::size_t closing_dquotes = literal_cpy.find_last_of('\"');
        if(closing_dquotes != string::npos){
            literal_cpy.erase(closing_dquotes, 1);
        }

        curr_result = literal_cpy;
    }
}

void interpreter::visit(astMULTOP* node){
    node->operand1->accept(this);
    literal_t op1_value = curr_result;

    node->operand2->accept(this);
    literal_t op2_value = curr_result;

    if(node->op == "*"){
        if(curr_type == grammarDFA::T_INT){
            curr_result = get<int>(op1_value) * get<int>(op2_value);
        }
        else{
            curr_result = get<float>(op1_value) * get<float>(op2_value);
        }
    }
    else if(node->op == "/"){
        if(curr_type == grammarDFA::T_INT){
            if(get<int>(op2_value) == 0){
                std::cerr << "ln " << node->line << ": division by zero encountered" << std::endl;
                throw std::runtime_error("Runtime errors encountered, see trace above.");
            }

            curr_result = get<int>(op1_value) / get<int>(op2_value);
        }
        else{
            if(get<float>(op2_value) == 0){
                std::cerr << "ln " << node->line << ": division by zero encountered" << std::endl;
                throw std::runtime_error("Runtime errors encountered, see trace above.");
            }

            curr_result = get<float>(op1_value) / get<float>(op2_value);
        }
    }
    else{
        curr_result = get<bool>(op1_value) && get<bool>(op2_value);
    }
}

void interpreter::visit(astADDOP* node){
    node->operand1->accept(this);
    literal_t op1_value = curr_result;

    node->operand2->accept(this);
    literal_t op2_value = curr_result;

    if(node->op == "+"){
        if(curr_type == grammarDFA::T_INT){
            curr_result = get<int>(op1_value) + get<int>(op2_value);
        }
        else if(curr_type == grammarDFA::T_FLOAT){
            curr_result = get<float>(op1_value) + get<float>(op2_value);
        }
        else if(curr_type == grammarDFA::T_CHAR){
            curr_result = (char) (get<char>(op1_value) + get<char>(op2_value));
        }
        else{
            curr_result = get<string>(op1_value) + get<string>(op2_value);
        }
    }
    else if(node->op == "-"){
        if(curr_type == grammarDFA::T_INT){
            curr_result = get<int>(op1_value) - get<int>(op2_value);
        }
        else if(curr_type == grammarDFA::T_FLOAT){
            curr_result = get<float>(op1_value) - get<float>(op2_value);
        }
        else{
            curr_result = (char) (get<char>(op1_value) - get<char>(op2_value));
        }
    }
    else{
        curr_result = get<bool>(op1_value) || get<bool>(op2_value);
    }
}

void interpreter::visit(astRELOP* node){
    node->operand1->accept(this);
    literal_t op1_value = curr_result;

    node->operand2->accept(this);
    literal_t op2_value = curr_result;

    if(node->op == "=="){
        if(curr_type == grammarDFA::T_BOOL){
            curr_result = get<bool>(op1_value) == get<bool>(op2_value);
        }
        else if(curr_type == grammarDFA::T_INT){
            curr_result = get<int>(op1_value) == get<int>(op2_value);
        }
        else if(curr_type == grammarDFA::T_FLOAT){
            curr_result = get<float>(op1_value) == get<float>(op2_value);
        }
        else if(curr_type == grammarDFA::T_CHAR){
            curr_result = get<char>(op1_value) == get<char>(op2_value);
        }
        else{
            curr_result = get<string>(op1_value) == get<string>(op2_value);
        }
    }
    else if(node->op == "!="){
        if(curr_type == grammarDFA::T_BOOL){
            curr_result = get<bool>(op1_value) != get<bool>(op2_value);
        }
        else if(curr_type == grammarDFA::T_INT){
            curr_result = get<int>(op1_value) != get<int>(op2_value);
        }
        else if(curr_type == grammarDFA::T_FLOAT){
            curr_result = get<float>(op1_value) != get<float>(op2_value);
        }
        else if(curr_type == grammarDFA::T_CHAR){
            curr_result = get<char>(op1_value) != get<char>(op2_value);
        }
        else{
            curr_result = get<string>(op1_value) != get<string>(op2_value);
        }
    }
    else if(node->op == "<="){
        if(curr_type == grammarDFA::T_BOOL){
            curr_result = get<bool>(op1_value) <= get<bool>(op2_value);
        }
        else if(curr_type == grammarDFA::T_INT){
            curr_result = get<int>(op1_value) <= get<int>(op2_value);
        }
        else if(curr_type == grammarDFA::T_FLOAT){
            curr_result = get<float>(op1_value) <= get<float>(op2_value);
        }
        else if(curr_type == grammarDFA::T_CHAR){
            curr_result = get<char>(op1_value) <= get<char>(op2_value);
        }
        else{
            curr_result = get<string>(op1_value) <= get<string>(op2_value);
        }
    }
    else if(node->op == ">="){
        if(curr_type == grammarDFA::T_BOOL){
            curr_result = get<bool>(op1_value) >= get<bool>(op2_value);
        }
        else if(curr_type == grammarDFA::T_INT){
            curr_result = get<int>(op1_value) >= get<int>(op2_value);
        }
        else if(curr_type == grammarDFA::T_FLOAT){
            curr_result = get<float>(op1_value) >= get<float>(op2_value);
        }
        else if(curr_type == grammarDFA::T_CHAR){
            curr_result = get<char>(op1_value) >= get<char>(op2_value);
        }
        else{
            curr_result = get<string>(op1_value) >= get<string>(op2_value);
        }
    }
    else if(node->op == "<"){
        if(curr_type == grammarDFA::T_BOOL){
            curr_result = get<bool>(op1_value) < get<bool>(op2_value);
        }
        else if(curr_type == grammarDFA::T_INT){
            curr_result = get<int>(op1_value) < get<int>(op2_value);
        }
        else if(curr_type == grammarDFA::T_FLOAT){
            curr_result = get<float>(op1_value) < get<float>(op2_value);
        }
        else if(curr_type == grammarDFA::T_CHAR){
            curr_result = get<char>(op1_value) < get<char>(op2_value);
        }
        else{
            curr_result = get<string>(op1_value) < get<string>(op2_value);
        }
    }
    else{
        if(curr_type == grammarDFA::T_BOOL){
            curr_result = get<bool>(op1_value) > get<bool>(op2_value);
        }
        else if(curr_type == grammarDFA::T_INT){
            curr_result = get<int>(op1_value) > get<int>(op2_value);
        }
        else if(curr_type == grammarDFA::T_FLOAT){
            curr_result = get<float>(op1_value) > get<float>(op2_value);
        }
        else if(curr_type == grammarDFA::T_CHAR){
            curr_result = get<char>(op1_value) > get<char>(op2_value);
        }
        else{
            curr_result = get<string>(op1_value) > get<string>(op2_value);
        }
    }

    curr_type = grammarDFA::T_BOOL;
}

void interpreter::visit(astAPARAMS* node){
    for(size_t i = 0; i < node->n_children; i++){
        (node->children->at(i))->accept(this);
        auto* var = new varSymbol(nullptr, curr_type);
        var->set_literal(curr_result);

        (functionStack->top().first)->fparams->push_back(var);
    }
}

void interpreter::visit(astFUNC_CALL* node){
    string func_ident = ((astIDENTIFIER*) node->identifier)->lexeme;
    auto* expected_func = new funcSymbol(&func_ident, grammarDFA::T_TYPE, new vector<varSymbol*>(0));

    if(node->aparams != nullptr){
        functionStack->push(make_pair(expected_func, false));
        node->aparams->accept(this);
        functionStack->pop();
    }

    functionStack->push(make_pair(symbolTable->lookup(func_ident, expected_func->fparams), false));

    symbolTable->push_scope();

    for(size_t i = 0; i < (functionStack->top().first)->fparams->size(); i++){
        auto* var = new varSymbol(&(functionStack->top().first)->fparams->at(i)->identifier,
                                   (functionStack->top().first)->fparams->at(i)->type);
        var->set_literal(expected_func->fparams->at(i)->literal);

        symbolTable->insert(var);
    }

    delete expected_func;

    for(auto &c : *(functionStack->top().first)->func_ref->children){
        c->accept(this);

        if(functionStack->top().second){
            break;
        }
    }

    curr_type = (functionStack->top().first)->type;
    (functionStack->top().first)->set_literal(curr_result);

    functionStack->pop();
    symbolTable->pop_scope();
}

void interpreter::visit(astSUBEXPR* node){
    node->subexpr->accept(this);
}

void interpreter::visit(astUNARY* node){
    node->operand->accept(this);
    literal_t op1_value = curr_result;

    if(node->op == "-"){
        if(curr_type == grammarDFA::T_INT){
            curr_result = -1 * get<int>(op1_value);
        }
        else if(curr_type == grammarDFA::T_FLOAT){
            curr_result =  -1 * get<float>(op1_value);
        }
    }
    else{
        curr_result = !get<bool>(op1_value);
    }
}

void interpreter::visit(astASSIGNMENT* node){
    varSymbol* ret_var = symbolTable->lookup(((astIDENTIFIER* )node->identifier)->lexeme);

    node->expression->accept(this);
    ret_var->set_literal(curr_result);
}

void interpreter::visit(astVAR_DECL* node){
    node->expression->accept(this);

    string var_ident = ((astIDENTIFIER*) node->identifier)->lexeme;
    grammarDFA::Symbol var_type = ((astTYPE*) node->type)->type;
    if(var_type == grammarDFA::T_AUTO){
        var_type = curr_type;
    }

    auto* var = new varSymbol(&var_ident, var_type);
    var->set_literal(curr_result);

    symbolTable->insert(var);
}

void interpreter::visit(astPRINT* node){
    node->expression->accept(this);

    if(curr_type == grammarDFA::T_BOOL){
        std::cout << get<bool>(curr_result) << std::endl;
    }
    else if(curr_type == grammarDFA::T_INT){
        std::cout << get<int>(curr_result) << std::endl;
    }
    else if(curr_type == grammarDFA::T_FLOAT){
        std::cout << get<float>(curr_result) << std::endl;
    }
    else if(curr_type == grammarDFA::T_CHAR){
        std::cout << get<char>(curr_result) << std::endl;
    }
    else{
        std::cout << get<string>(curr_result) << std::endl;
    }
}

void interpreter::visit(astRETURN* node){
    node->expression->accept(this);
    functionStack->top().second = true;

    if(functionStack->top().first->type == grammarDFA::T_AUTO){
        functionStack->top().first->type = curr_type;
    }
}

void interpreter::visit(astIF* node){
    node->expression->accept(this);

    if(get<bool>(curr_result)){
        node->if_block->accept(this);
    }
    else if(node->else_block != nullptr){
        node->else_block->accept(this);
    }
}

void interpreter::visit(astFOR* node){
    symbolTable->push_scope();

    if(node->var_decl != nullptr){ node->var_decl->accept(this);}

    while(true){
        node->expression->accept(this);

        if(get<bool>(curr_result)){
            node->for_block->accept(this);

            if(functionStack->empty() || !functionStack->top().second){
                if(node->assignment != nullptr){ node->assignment->accept(this);}
            }
            else{
                break;
            }
        }
        else{
            break;
        }
    }

    symbolTable->pop_scope();
}

void interpreter::visit(astWHILE* node){
    while(true){
        node->expression->accept(this);

        if(get<bool>(curr_result)){
            if(node->while_block != nullptr){ node->while_block->accept(this);}

            if(!functionStack->empty() && functionStack->top().second){
                break;
            }
        }
        else{
            break;
        }
    }
}

void interpreter::visit(astFPARAMS* node){}
void interpreter::visit(astFPARAM* node){}

void interpreter::visit(astFUNC_DECL* node){
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
    func->set_func_ref((astBLOCK*) node->function_block);

    symbolTable->insert(func);
}

void interpreter::visit(astBLOCK* node){
    symbolTable->push_scope();

    for(auto &c : *node->children){
        c->accept(this);

        if(!functionStack->empty() && functionStack->top().second){
            break;
        }
    }

    symbolTable->pop_scope();
}
void interpreter::visit(astPROGRAM* node){
    for(auto &c : *node->children){
        c->accept(this);

        if(!functionStack->empty() && functionStack->top().second){
            break;
        }
    }
}
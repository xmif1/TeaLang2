//
// Created by Xandru Mifsud on 17/04/2021.
//

#include "interpreter.h"

void interpreter::visit(astTYPE* node){}

void interpreter::visit(astLITERAL* node){
    curr_type = node->type;
    curr_obj_class = grammarDFA::SINGLETON;

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
// not called for assignment; only for value retrieval
void interpreter::visit(astIDENTIFIER* node){
    symbol* ret_symb = symbolTable->lookup(node->lexeme);
    curr_type = ret_symb->type;
    curr_obj_class = ret_symb->object_class;

    if(curr_obj_class == grammarDFA::ARRAY){
        curr_result = get<literal_arr_t>(ret_symb->object);
    }
    else{
        literal_t elt = get<literal_t>(ret_symb->object);

        if(curr_type == grammarDFA::T_BOOL){
            curr_result = get<bool>(elt);
        }
        else if(curr_type == grammarDFA::T_INT){
            curr_result = get<int>(elt);
        }
        else if(curr_type == grammarDFA::T_FLOAT){
            curr_result = get<float>(elt);
        }
        else if(curr_type == grammarDFA::T_CHAR){
            curr_result = get<char>(elt);
        }
        else{
            string literal_cpy = get<string>(elt);

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
}

// only called when the identifier refers to an operand standing for an array element, not for eg. a function  call
// not called for assignment; only for value retrieval
void interpreter::visit(astELEMENT* node){
    string arr_ident = ((astIDENTIFIER*) node->identifier)->lexeme;
    symbol* ret_symb = symbolTable->lookup(arr_ident);

    (node->index)->accept(this);
    int index = get<int>(get<literal_t>(curr_result));
    int size = ((arrSymbol*) ret_symb)->size;

    if(size <= index || index < 0){
        std::cerr << "ln " << node->line << ": index " << index << " is out of bounds of array " << arr_ident <<
        " with size " << size << std::endl;
        throw std::runtime_error("Runtime errors encountered, see trace above.");
    }

    curr_type = ret_symb->type;
    curr_obj_class = grammarDFA::SINGLETON;

    literal_t elt = get<literal_arr_t>(ret_symb->object)->at(index);
    if(curr_type == grammarDFA::T_BOOL){
        curr_result = get<bool>(elt);
    }
    else if(curr_type == grammarDFA::T_INT){
        curr_result = get<int>(elt);
    }
    else if(curr_type == grammarDFA::T_FLOAT){
        curr_result = get<float>(elt);
    }
    else if(curr_type == grammarDFA::T_CHAR){
        curr_result = get<char>(elt);
    }
    else{
        string literal_cpy = get<string>(elt);

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

literal_t interpreter::multop(string op, int line, literal_t lit1, literal_t lit2){
    literal_t result;

    if(op == "*"){
        if(curr_type == grammarDFA::T_INT){
            result = get<int>(lit1) * get<int>(lit2);
        }
        else{
            result = get<float>(lit1) * get<float>(lit2);
        }
    }
    else if(op == "/"){
        if(curr_type == grammarDFA::T_INT){
            if(get<int>(lit2) == 0){
                std::cerr << "ln " << line << ": division by zero encountered" << std::endl;
                throw std::runtime_error("Runtime errors encountered, see trace above.");
            }

            result = get<int>(lit1) / get<int>(lit2);
        }
        else{
            if(get<float>(lit2) == 0){
                std::cerr << "ln " << line << ": division by zero encountered" << std::endl;
                throw std::runtime_error("Runtime errors encountered, see trace above.");
            }

            result = get<float>(lit1) / get<float>(lit2);
        }
    }
    else{
        result = get<bool>(lit1) && get<bool>(lit2);
    }

    return result;
}

void interpreter::visit(astMULTOP* node){
    node->operand1->accept(this);
    obj_t op1_value = curr_result;

    node->operand2->accept(this);
    obj_t op2_value = curr_result;

    if(curr_obj_class == grammarDFA::ARRAY){
        literal_arr_t arr1 = get<literal_arr_t>(op1_value);
        int size1 = arr1->size();

        literal_arr_t arr2 = get<literal_arr_t>(op2_value);
        int size2 = arr2->size();

        if(size1 != size2){
            std::cerr << "ln " << node->line << ": arrays have mismatched sizes " << size1 << " and " << size2 << std::endl;
            throw std::runtime_error("Runtime errors encountered, see trace above.");
        }

        auto* result = new vector<literal_t>(size1);
        for(int i = 0; i < size1; i++){
            result->push_back(multop(node->op, node->line, arr1->at(i), arr2->at(i)));
        }

        curr_result = result;
    }else{
        curr_result = multop(node->op, node->line, get<literal_t>(op1_value), get<literal_t>(op2_value));
    }
}

literal_t interpreter::addop(string op, literal_t lit1, literal_t lit2){
    literal_t result;

    if(op == "+"){
        if(curr_type == grammarDFA::T_INT){
            result = get<int>(lit1) + get<int>(lit2);
        }
        else if(curr_type == grammarDFA::T_FLOAT){
            result = get<float>(lit1) + get<float>(lit2);
        }
        else if(curr_type == grammarDFA::T_CHAR){
            result = (char) (get<char>(lit1) + get<char>(lit2));
        }
        else{
            result = get<string>(lit1) + get<string>(lit2);
        }
    }
    else if(op == "-"){
        if(curr_type == grammarDFA::T_INT){
            result = get<int>(lit1) - get<int>(lit2);
        }
        else if(curr_type == grammarDFA::T_FLOAT){
            result = get<float>(lit1) - get<float>(lit2);
        }
        else{
            result = (char) (get<char>(lit1) - get<char>(lit2));
        }
    }
    else{
        result = get<bool>(lit1) || get<bool>(lit2);
    }

    return result;
}

void interpreter::visit(astADDOP* node){
    node->operand1->accept(this);
    obj_t op1_value = curr_result;

    node->operand2->accept(this);
    obj_t op2_value = curr_result;

    if(curr_obj_class == grammarDFA::ARRAY){
        literal_arr_t arr1 = get<literal_arr_t>(op1_value);
        int size1 = arr1->size();

        literal_arr_t arr2 = get<literal_arr_t>(op2_value);
        int size2 = arr2->size();

        if(size1 != size2){
            std::cerr << "ln " << node->line << ": arrays have mismatched sizes " << size1 << " and " << size2 << std::endl;
            throw std::runtime_error("Runtime errors encountered, see trace above.");
        }

        auto* result = new vector<literal_t>(size1);
        for(int i = 0; i < size1; i++){
            result->push_back(addop(node->op, arr1->at(i), arr2->at(i)));
        }

        curr_result = result;
    }
    else{
        curr_result = addop(node->op, get<literal_t>(op1_value), get<literal_t>(op2_value));
    }
}

literal_t interpreter::relop(string op, literal_t lit1, literal_t lit2){
    literal_t result;

    if(op == "=="){
        if(curr_type == grammarDFA::T_BOOL){
            result = get<bool>(lit1) == get<bool>(lit2);
        }
        else if(curr_type == grammarDFA::T_INT){
            result = get<int>(lit1) == get<int>(lit2);
        }
        else if(curr_type == grammarDFA::T_FLOAT){
            result = get<float>(lit1) == get<float>(lit2);
        }
        else if(curr_type == grammarDFA::T_CHAR){
            result = get<char>(lit1) == get<char>(lit2);
        }
        else{
            result = get<string>(lit1) == get<string>(lit2);
        }
    }
    else if(op == "!="){
        if(curr_type == grammarDFA::T_BOOL){
            result = get<bool>(lit1) != get<bool>(lit2);
        }
        else if(curr_type == grammarDFA::T_INT){
            result = get<int>(lit1) != get<int>(lit2);
        }
        else if(curr_type == grammarDFA::T_FLOAT){
            result = get<float>(lit1) != get<float>(lit2);
        }
        else if(curr_type == grammarDFA::T_CHAR){
            result = get<char>(lit1) != get<char>(lit2);
        }
        else{
            result = get<string>(lit1) != get<string>(lit2);
        }
    }
    else if(op == "<="){
        if(curr_type == grammarDFA::T_BOOL){
            result = get<bool>(lit1) <= get<bool>(lit2);
        }
        else if(curr_type == grammarDFA::T_INT){
            result = get<int>(lit1) <= get<int>(lit2);
        }
        else if(curr_type == grammarDFA::T_FLOAT){
            result = get<float>(lit1) <= get<float>(lit2);
        }
        else if(curr_type == grammarDFA::T_CHAR){
            result = get<char>(lit1) <= get<char>(lit2);
        }
        else{
            result = get<string>(lit1) <= get<string>(lit2);
        }
    }
    else if(op == ">="){
        if(curr_type == grammarDFA::T_BOOL){
            result = get<bool>(lit1) >= get<bool>(lit2);
        }
        else if(curr_type == grammarDFA::T_INT){
            result = get<int>(lit1) >= get<int>(lit2);
        }
        else if(curr_type == grammarDFA::T_FLOAT){
            result = get<float>(lit1) >= get<float>(lit2);
        }
        else if(curr_type == grammarDFA::T_CHAR){
            result = get<char>(lit1) >= get<char>(lit2);
        }
        else{
            result = get<string>(lit1) >= get<string>(lit2);
        }
    }
    else if(op == "<"){
        if(curr_type == grammarDFA::T_BOOL){
            result = get<bool>(lit1) < get<bool>(lit2);
        }
        else if(curr_type == grammarDFA::T_INT){
            result = get<int>(lit1) < get<int>(lit2);
        }
        else if(curr_type == grammarDFA::T_FLOAT){
            result = get<float>(lit1) < get<float>(lit2);
        }
        else if(curr_type == grammarDFA::T_CHAR){
            result = get<char>(lit1) < get<char>(lit2);
        }
        else{
            result = get<string>(lit1) < get<string>(lit2);
        }
    }
    else{
        if(curr_type == grammarDFA::T_BOOL){
            result = get<bool>(lit1) > get<bool>(lit2);
        }
        else if(curr_type == grammarDFA::T_INT){
            result = get<int>(lit1) > get<int>(lit2);
        }
        else if(curr_type == grammarDFA::T_FLOAT){
            result = get<float>(lit1) > get<float>(lit2);
        }
        else if(curr_type == grammarDFA::T_CHAR){
            result = get<char>(lit1) > get<char>(lit2);
        }
        else{
            result = get<string>(lit1) > get<string>(lit2);
        }
    }

    return result;
}

void interpreter::visit(astRELOP* node){
    node->operand1->accept(this);
    obj_t op1_value = curr_result;

    node->operand2->accept(this);
    obj_t op2_value = curr_result;

    if(curr_obj_class == grammarDFA::ARRAY){
        literal_arr_t arr1 = get<literal_arr_t>(op1_value);
        int size1 = arr1->size();

        literal_arr_t arr2 = get<literal_arr_t>(op2_value);
        int size2 = arr2->size();

        if(size1 != size2){
            std::cerr << "ln " << node->line << ": arrays have mismatched sizes " << size1 << " and " << size2 << std::endl;
            throw std::runtime_error("Runtime errors encountered, see trace above.");
        }

        auto* result = new vector<literal_t>(size1);
        for(int i = 0; i < size1; i++){
            result->push_back(relop(node->op, arr1->at(i), arr2->at(i)));
        }

        curr_result = result;
    }
    else{
        curr_result = relop(node->op, get<literal_t>(op1_value), get<literal_t>(op2_value));
    }

    curr_type = grammarDFA::T_BOOL;
}

void interpreter::visit(astAPARAMS* node){
    for(size_t i = 0; i < node->n_children; i++){
        (node->children->at(i))->accept(this);
        symbol* aparam;

        if(curr_obj_class == grammarDFA::SINGLETON){
            aparam = new varSymbol(nullptr, curr_type);
            aparam->set_object(curr_result);
        }
        else{
            aparam = new arrSymbol(nullptr, curr_type, get<literal_arr_t>(curr_result)->size());
            aparam->set_object(curr_result);
        }

        (functionStack->top().first)->fparams->push_back(aparam);
    }
}

void interpreter::visit(astFUNC_CALL* node){
    string func_ident = ((astIDENTIFIER*) node->identifier)->lexeme;
    auto* expected_func = new funcSymbol(&func_ident, grammarDFA::T_TYPE, grammarDFA::FUNCTION, new vector<symbol*>(0));

    if(node->aparams != nullptr){
        functionStack->push(make_pair(expected_func, false));
        node->aparams->accept(this);
        functionStack->pop();
    }

    functionStack->push(make_pair(symbolTable->lookup(func_ident, expected_func->fparams), false));

    symbolTable->push_scope();

    for(size_t i = 0; i < (functionStack->top().first)->fparams->size(); i++){
        symbol* aparam;
        grammarDFA::Symbol obj_class = (functionStack->top().first)->fparams->at(i)->object_class;

        if(obj_class == grammarDFA::SINGLETON){
            aparam = new varSymbol(&(functionStack->top().first)->fparams->at(i)->identifier,
                                   (functionStack->top().first)->fparams->at(i)->type);
        }
        else{
            aparam = new arrSymbol(&(functionStack->top().first)->fparams->at(i)->identifier,
                                   (functionStack->top().first)->fparams->at(i)->type,
                                   ((arrSymbol*) expected_func->fparams->at(i))->size);
        }

        aparam->set_object(expected_func->fparams->at(i)->object);
        symbolTable->insert(aparam);
    }

    delete expected_func;

    for(auto &c : *(functionStack->top().first)->func_ref->children){
        c->accept(this);

        if(functionStack->top().second){
            break;
        }
    }

    curr_type = (functionStack->top().first)->type;
    curr_obj_class = (functionStack->top().first)->ret_obj_class;
    (functionStack->top().first)->set_object(curr_result);

    functionStack->pop();
    symbolTable->pop_scope();
}

void interpreter::visit(astSUBEXPR* node){
    node->subexpr->accept(this);
}

literal_t interpreter::unary(string op, literal_t literal){
    literal_t result;

    if(op == "-"){
        if(curr_type == grammarDFA::T_INT){
            result = -1 * get<int>(literal);
        }
        else if(curr_type == grammarDFA::T_FLOAT){
            result =  -1 * get<float>(literal);
        }
    }
    else{
        result = !get<bool>(literal);
    }

    return result;
}

void interpreter::visit(astUNARY* node){
    node->operand->accept(this);
    obj_t op1_value = curr_result;

    if(curr_obj_class == grammarDFA::ARRAY){
        literal_arr_t arr1 = get<literal_arr_t>(op1_value);
        int size1 = arr1->size();

        auto* result = new vector<literal_t>(size1);
        for(int i = 0; i < size1; i++){
            result->push_back(unary(node->op, arr1->at(i)));
        }

        curr_result = result;
    }
    else{
        curr_result = unary(node->op, get<literal_t>(op1_value));
    }
}

void interpreter::visit(astASSIGNMENT_IDENTIFIER* node){
    symbol* ret_symb = symbolTable->lookup(((astIDENTIFIER* )node->identifier)->lexeme);

    node->expression->accept(this);

    if(curr_obj_class == grammarDFA::ARRAY && get<literal_arr_t>(curr_result)->size() != ((arrSymbol*) ret_symb)->size){
        std::cerr << "ln " << node->line << ": arrays have mismatched sizes " << get<literal_arr_t>(curr_result)->size()
        << " and " << ((arrSymbol*) ret_symb)->size << std::endl;
        throw std::runtime_error("Runtime errors encountered, see trace above.");
    }

    ret_symb->set_object(curr_result);
}

void interpreter::visit(astASSIGNMENT_ELEMENT* node){
    string arr_ident = ((astIDENTIFIER*) ((astELEMENT*) node->element)->identifier)->lexeme;
    symbol* ret_symb = symbolTable->lookup(arr_ident);

    (((astELEMENT*) node->element)->index)->accept(this);
    int index = get<int>(get<literal_t>(curr_result));
    int size = ((arrSymbol*) ret_symb)->size;

    if(size <= index || index < 0){
        std::cerr << "ln " << node->line << ": index " << index << " is out of bounds of array " << arr_ident <<
        " with size " << size << std::endl;
        throw std::runtime_error("Runtime errors encountered, see trace above.");
    }

    node->expression->accept(this);

    if(ret_symb->type == grammarDFA::T_AUTO){
        ret_symb->type = curr_type;
    }

    get<literal_arr_t>(ret_symb->object)->at(index) = get<literal_t>(curr_result);
}

void interpreter::visit(astVAR_DECL* node){
    node->expression->accept(this);

    string var_ident = ((astIDENTIFIER*) node->identifier)->lexeme;
    grammarDFA::Symbol var_type = ((astTYPE*) node->type)->type;
    if(var_type == grammarDFA::T_AUTO){
        var_type = curr_type;
    }

    auto* var = new varSymbol(&var_ident, var_type);
    var->set_object(curr_result);

    symbolTable->insert(var);
}

literal_t interpreter::default_literal(grammarDFA::Symbol type){
    literal_t result;

    if(curr_type == grammarDFA::T_BOOL){
        result = false;
    }
    else if(curr_type == grammarDFA::T_INT){
        result = 0;
    }
    else if(curr_type == grammarDFA::T_FLOAT){
        result = (float) 0.0;
    }
    else if(curr_type == grammarDFA::T_CHAR){
        result = '\0';
    }
    else{
        result = "";
    }

    return result;
}

void interpreter::visit(astARR_DECL* node){
    string arr_ident = ((astIDENTIFIER*) node->identifier)->lexeme;
    grammarDFA::Symbol arr_type = ((astTYPE*) node->type)->type;

    node->size->accept(this);
    int size = get<int>(get<literal_t>(curr_result));

    if(size < 1){
        std::cerr << "ln " << node->line << ": size of array " << arr_ident << " must be a positive integer" << std::endl;
        throw std::runtime_error("Runtime errors encountered, see trace above.");
    }

    int n_assignment_elts = node->n_children - 3;
    if(size < n_assignment_elts){
        std::cerr << "ln " << node->line << ": cannot assign " << n_assignment_elts << " to array " << arr_ident <<
        " of size " << size << std::endl;
        throw std::runtime_error("Runtime errors encountered, see trace above.");
    }

    literal_arr_t lit_arr = new vector<literal_t>(size);

    for(int i = 0; i < n_assignment_elts; i++){
        (node->children->at(i+3))->accept(this);

        if(arr_type == grammarDFA::T_AUTO){
            arr_type = curr_type;
        }

        lit_arr->at(i) = get<literal_t>(curr_result);
    }

    if(arr_type != grammarDFA::T_AUTO){
        literal_t default_lit_val = default_literal(arr_type);

        for(int i = n_assignment_elts; i < size; i++){
            lit_arr->at(i) = default_lit_val;
        }
    }

    auto* arr = new arrSymbol(&arr_ident, arr_type, size);
    arr->set_object(lit_arr);

    symbolTable->insert(arr);
}

void interpreter::visit(astPRINT* node){
    node->expression->accept(this);

    if(curr_obj_class == grammarDFA::ARRAY){
        std::cout << "{";
        for(int i = 0; i < get<literal_arr_t>(curr_result)->size(); i++){
            if(i != 0){
                std::cout << ", ";
            }

            if(curr_type == grammarDFA::T_BOOL){
               std::cout << get<bool>(get<literal_arr_t>(curr_result)->at(i));
            }
            else if(curr_type == grammarDFA::T_INT){
                std::cout << get<int>(get<literal_arr_t>(curr_result)->at(i));
            }
            else if(curr_type == grammarDFA::T_FLOAT){
                std::cout << get<float>(get<literal_arr_t>(curr_result)->at(i));
            }
            else if(curr_type == grammarDFA::T_CHAR){
                std::cout << get<char>(get<literal_arr_t>(curr_result)->at(i));
            }
            else{
                std::cout << get<string>(get<literal_arr_t>(curr_result)->at(i));
            }
        }

        std::cout << "}" << std::endl;
    }
    else if(curr_type == grammarDFA::T_BOOL){
        std::cout << get<bool>(get<literal_t>(curr_result)) << std::endl;
    }
    else if(curr_type == grammarDFA::T_INT){
        std::cout << get<int>(get<literal_t>(curr_result)) << std::endl;
    }
    else if(curr_type == grammarDFA::T_FLOAT){
        std::cout << get<float>(get<literal_t>(curr_result)) << std::endl;
    }
    else if(curr_type == grammarDFA::T_CHAR){
        std::cout << get<char>(get<literal_t>(curr_result)) << std::endl;
    }
    else{
        std::cout << get<string>(get<literal_t>(curr_result)) << std::endl;
    }
}

void interpreter::visit(astRETURN* node){
    node->expression->accept(this);
    functionStack->top().second = true;

    if(functionStack->top().first->type == grammarDFA::T_AUTO){
        functionStack->top().first->type = curr_type;
        functionStack->top().first->ret_obj_class = curr_obj_class;
    }
}

void interpreter::visit(astIF* node){
    node->expression->accept(this);

    if(get<bool>(get<literal_t>(curr_result))){
        node->if_block->accept(this);
    }
    else if(node->else_block != nullptr){
        node->else_block->accept(this);
    }
}

void interpreter::visit(astFOR* node){
    symbolTable->push_scope();

    if(node->decl != nullptr){ node->decl->accept(this);}

    while(true){
        node->expression->accept(this);

        if(get<bool>(get<literal_t>(curr_result))){
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

        if(get<bool>(get<literal_t>(curr_result))){
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
    grammarDFA::Symbol ret_obj_class = ((astTYPE*) node->type)->object_class;

    auto* fparams = new vector<symbol*>(0);
    if(node->fparams != nullptr){
        for(auto &c : *((astFPARAMS*) node->fparams)->children){
            string fparam_ident = ((astIDENTIFIER*) ((astFPARAM*) c)->children->at(0))->lexeme;
            grammarDFA::Symbol fparam_type = ((astTYPE*) ((astFPARAM*) c)->children->at(1))->type;
            grammarDFA::Symbol fparam_obj_class = ((astTYPE*) ((astFPARAM*) c)->children->at(1))->object_class;

            if(fparam_obj_class == grammarDFA::ARRAY){
                fparams->push_back(new arrSymbol(&fparam_ident, fparam_type, 0));
            }else{
                fparams->push_back(new varSymbol(&fparam_ident, fparam_type));
            }
        }
    }

    auto* func = new funcSymbol(&func_ident, ret_type, ret_obj_class, fparams);
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
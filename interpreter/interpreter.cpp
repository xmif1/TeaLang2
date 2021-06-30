//
// Created by Xandru Mifsud on 17/04/2021.
//

#include "interpreter.h"

void interpreter::visit(astTYPE* node){}

void interpreter::visit(astLITERAL* node){
    curr_obj_class = grammarDFA::SINGLETON; // literals are always singular values
    curr_type = type_t(node->type, node->type_str); // extract current type from node

    // carry out case by case analysis of each type, type casting the lexeme associated with the node to the correct type
    if(node->type == grammarDFA::T_BOOL){
        if(node->lexeme == "true"){
            curr_result = true;
        }
        else{
            curr_result = false;
        }
    }
    else if(node->type == grammarDFA::T_INT){
        curr_result = stoi(node->lexeme);
    }
    else if(node->type == grammarDFA::T_FLOAT){
        curr_result = stof(node->lexeme);
    }
    else if(node->type == grammarDFA::T_CHAR){
        string literal_cpy = node->lexeme;

        // recall that character literals are written between apostrophes ''
        // we begin by first removing the opening apostrophe
        std::size_t opening_apost = literal_cpy.find_first_of('\'');
        if(opening_apost != string::npos){
            literal_cpy.erase(opening_apost, 1);
        }

        // and then remove the closing apostrophe
        std::size_t closing_apost = literal_cpy.find_last_of('\'');
        if(closing_apost != string::npos){
            literal_cpy.erase(closing_apost, 1);
        }

        // in the case the character is an escaped character, bind to the appropriate escaped character in C++ by checking
        // the second character in the lexeme after apostrophe removal
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
        else{ // otherwise the input is a single character i.e. we can simply cast it to a char
            curr_result = (char) literal_cpy[0];
        }
    }
    else{
        string literal_cpy = node->lexeme;

        // recall that string literals are written between quotation marks ""
        // begin by removing the opening quoatation marks
        std::size_t opening_dquotes = literal_cpy.find_first_of('\"');
        if(opening_dquotes != string::npos){
            literal_cpy.erase(opening_dquotes, 1);
        }

        // and then the closing quoatation marks
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
    symbol* ret_symb = lookup_symbolTable->lookup(node->lexeme); // find symbol in symbol table with matching identifier
    lookup_symbolTable = curr_symbolTable;

    // extract the type and object class from the symbol
    curr_type = ret_symb->type;
    curr_obj_class = ret_symb->object_class;

    // if the object class is ARRAY, simply fetch the literal_arr_t value from the variant tagged-union container
    if(curr_obj_class == grammarDFA::ARRAY){
        curr_result = get<literal_arr_t>(ret_symb->object);
    }
    else{ // else type is SINGLETON and hence we fetch the literal_t value from the variant tagged-union container
        literal_t elt = get<literal_t>(ret_symb->object);

        // carry out further case by case analysis and fetch the correct type from the variant tagged-union container
        if(curr_type.first == grammarDFA::T_BOOL){
            curr_result = get<bool>(elt);
        }
        else if(curr_type.first == grammarDFA::T_INT){
            curr_result = get<int>(elt);
        }
        else if(curr_type.first == grammarDFA::T_FLOAT){
            curr_result = get<float>(elt);
        }
        else if(curr_type.first == grammarDFA::T_CHAR){
            curr_result = get<char>(elt);
        }
        else if(curr_type.first == grammarDFA::T_STRING){
            string literal_cpy = get<string>(elt);

            // recall that string literals are written between quotation marks ""
            // begin by removing the opening quoatation marks
            std::size_t opening_dquotes = literal_cpy.find_first_of('\"');
            if(opening_dquotes != string::npos){
                literal_cpy.erase(opening_dquotes, 1);
            }

            // and then the closing quoatation marks
            std::size_t closing_dquotes = literal_cpy.find_last_of('\"');
            if(closing_dquotes != string::npos){
                literal_cpy.erase(closing_dquotes, 1);
            }

            curr_result = literal_cpy;
        }
        else{
            curr_result = get<symbol_table*>(elt);
        }
    }
}

// only called when the identifier refers to an operand standing for an array element, not for eg. a function  call
// not called for assignment; only for value retrieval
void interpreter::visit(astELEMENT* node){
    type_t ret_type = curr_type; // maintain current type

    string arr_ident = ((astIDENTIFIER*) node->identifier)->lexeme; // extract the identifier of the array
    symbol* ret_symb = lookup_symbolTable->lookup(arr_ident); // find the arrSymbol instance corresponding to the identifier
    lookup_symbolTable = curr_symbolTable;

    ret_type = ret_symb->type; // set ret_type to that of the returned symbol (i.e. of the array)

    (node->index)->accept(this); // evaluate astEXPRESSION node corresponding to the index
    int index = get<int>(get<literal_t>(curr_result)); // result is stored in the int container of literal_t
    int size = ((arrSymbol*) ret_symb)->size; // maintain reference to size of the array as specified in its arrSymbol

    // run--time bounds checking: check that 0 <= index < size; if not, report a run--time error and terminate immediately
    if(size <= index || index < 0){
        std::cerr << "ln " << node->line << ": index " << index << " is out of bounds of array " << arr_ident <<
        " with size " << size << std::endl;
        throw std::runtime_error("Runtime errors encountered, see trace above.");
    }

    // otherwise fetch the literal_t held at the specified index of the arrSymbol's literal_arr_t container
    literal_t elt = get<literal_arr_t>(ret_symb->object)->at(index);

    // carry out case by case analysis and fetch the correct type from the variant tagged-union container
    if(ret_type.first == grammarDFA::T_BOOL){
        curr_result = get<bool>(elt);
    }
    else if(ret_type.first == grammarDFA::T_INT){
        curr_result = get<int>(elt);
    }
    else if(ret_type.first == grammarDFA::T_FLOAT){
        curr_result = get<float>(elt);
    }
    else if(ret_type.first == grammarDFA::T_CHAR){
        curr_result = get<char>(elt);
    }
    else if(ret_type.first == grammarDFA::T_STRING){
        string literal_cpy = get<string>(elt);

        // recall that string literals are written between quotation marks ""
        // begin by removing the opening quoatation marks
        std::size_t opening_dquotes = literal_cpy.find_first_of('\"');
        if(opening_dquotes != string::npos){
            literal_cpy.erase(opening_dquotes, 1);
        }

        // and then the closing quoatation marks
        std::size_t closing_dquotes = literal_cpy.find_last_of('\"');
        if(closing_dquotes != string::npos){
            literal_cpy.erase(closing_dquotes, 1);
        }

        curr_result = literal_cpy;
    }
    else{
        curr_result = get<symbol_table*>(elt);
    }

    curr_type = ret_type; // set current type to that of element i.e. of array
    curr_obj_class = grammarDFA::SINGLETON; // since we do not support multi-dim arrays, element is always SINGLETON
}

/* Utility function which given a multiplicative op and two literal_t instances, finds the corresponding literal_t
 * based on applying the op on the literal_t instances.
 */
literal_t interpreter::multop(string op, int line, literal_t lit1, literal_t lit2){
    literal_t result; // resulting literal_t

    if(op == "*"){ // in the case of multiplication
        // if integer types, multiply the two int values in the literal_t containers
        if(curr_type.first == grammarDFA::T_INT){
            result = get<int>(lit1) * get<int>(lit2);
        }
        else{ // else the only other valid type is float;
            // multiply the two float values in the literal_t containers
            result = get<float>(lit1) * get<float>(lit2);
        }
    }
    else if(op == "/"){ // else in the case of division
        if(curr_type.first == grammarDFA::T_INT){
            // if 2nd operand is 0, report divide by 0 runtime error and terminate
            if(get<int>(lit2) == 0){
                std::cerr << "ln " << line << ": division by zero encountered" << std::endl;
                throw std::runtime_error("Runtime errors encountered, see trace above.");
            }

            // divide the two int values in the literal_t containers
            result = get<int>(lit1) / get<int>(lit2);
        }
        else{
            // if 2nd operand is 0, report divide by 0 runtime error and terminate
            if(get<float>(lit2) == 0){
                std::cerr << "ln " << line << ": division by zero encountered" << std::endl;
                throw std::runtime_error("Runtime errors encountered, see trace above.");
            }

            // divide the two float values in the literal_t containers
            result = get<float>(lit1) / get<float>(lit2);
        }
    }
    else{ // otherwise op is logical AND; fetch the bool values in the literat_t containers and apply logical AND
        result = get<bool>(lit1) && get<bool>(lit2);
    }

    return result;
}

void interpreter::visit(astMULTOP* node){
    node->operand1->accept(this); // visit astEXPRESSION node corresponding to first operand
    obj_t op1_value = curr_result; // maintain result for op1

    node->operand2->accept(this); // visit astEXPRESSION node corresponding to second operand
    obj_t op2_value = curr_result; // maintain result for op2

    if(curr_obj_class == grammarDFA::ARRAY){ // in the case of array, carry out overloaded element-wise multop
        literal_arr_t arr1 = get<literal_arr_t>(op1_value); // hold reference to first array operand
        int size1 = arr1->size(); // hold reference to size of first array operand

        literal_arr_t arr2 = get<literal_arr_t>(op2_value); // hold reference to second array operand
        int size2 = arr2->size(); // hold reference to size of second array operand

        if(size1 != size2){ // if sizes do not match, report a run--time error
            std::cerr << "ln " << node->line << ": arrays have mismatched sizes " << size1 << " and " << size2 << std::endl;
            throw std::runtime_error("Runtime errors encountered, see trace above.");
        }

        auto* result = new vector<literal_t>(0);
        for(int i = 0; i < size1; i++){ // call multop on each pair of elements
            result->push_back(multop(node->op, node->line, arr1->at(i), arr2->at(i)));
        }

        curr_result = result;
    }else{ // else if single element, simply apply multop on the two operands
        curr_result = multop(node->op, node->line, get<literal_t>(op1_value), get<literal_t>(op2_value));
    }
}


/* Utility function which given a additive op and two literal_t instances, finds the corresponding literal_t
 * based on applying the op on the literal_t instances.
 */
literal_t interpreter::addop(string op, literal_t lit1, literal_t lit2){
    literal_t result; // resulting literal_t

    if(op == "+"){ // in the case of addition, apply case by case analysis and fetch the correct type from the variant tagged-union containers
        if(curr_type.first == grammarDFA::T_INT){
            result = get<int>(lit1) + get<int>(lit2);
        }
        else if(curr_type.first == grammarDFA::T_FLOAT){
            result = get<float>(lit1) + get<float>(lit2);
        }
        else if(curr_type.first == grammarDFA::T_CHAR){
            result = (char) (get<char>(lit1) + get<char>(lit2));
        }
        else{
            result = get<string>(lit1) + get<string>(lit2);
        }
    }
    else if(op == "-"){ // else in the case of subtraction, proceed similarly
        if(curr_type.first == grammarDFA::T_INT){
            result = get<int>(lit1) - get<int>(lit2);
        }
        else if(curr_type.first == grammarDFA::T_FLOAT){
            result = get<float>(lit1) - get<float>(lit2);
        }
        else{
            result = (char) (get<char>(lit1) - get<char>(lit2));
        }
    }
    else{ // otherwise if logical OR, fetch the boolean types from the variant tagged-union containers and apply logical OR
        result = get<bool>(lit1) || get<bool>(lit2);
    }

    return result;
}

void interpreter::visit(astADDOP* node){
    node->operand1->accept(this); // visit astEXPRESSION node corresponding to first operand
    obj_t op1_value = curr_result; // maintain result for op1

    node->operand2->accept(this); // visit astEXPRESSION node corresponding to second operand
    obj_t op2_value = curr_result; // maintain result for op2

    if(curr_obj_class == grammarDFA::ARRAY){ // in the case of array, carry out overloaded element-wise addop
        literal_arr_t arr1 = get<literal_arr_t>(op1_value); // hold reference to first array operand
        int size1 = arr1->size(); // hold reference to size of first array operand

        literal_arr_t arr2 = get<literal_arr_t>(op2_value); // hold reference to second array operand
        int size2 = arr2->size(); // hold reference to size of second array operand

        if(size1 != size2){ // if sizes do not match, report a run--time error
            std::cerr << "ln " << node->line << ": arrays have mismatched sizes " << size1 << " and " << size2 << std::endl;
            throw std::runtime_error("Runtime errors encountered, see trace above.");
        }

        auto* result = new vector<literal_t>(0);
        for(int i = 0; i < size1; i++){ // call addop on each pair of elements
            result->push_back(addop(node->op, arr1->at(i), arr2->at(i)));
        }

        curr_result = result;
    }
    else{ // else if single element, simply apply addop on the two operands
        curr_result = addop(node->op, get<literal_t>(op1_value), get<literal_t>(op2_value));
    }
}

/* Utility function which given a additive op and two literal_t instances, finds the corresponding literal_t
 * based on applying the op on the literal_t instances.
 */
literal_t interpreter::relop(string op, literal_t lit1, literal_t lit2){
    literal_t result; // resulting literal_t

    /* Apply case by case analysis based on:
     * (i) relational operator
     * (ii) type
     *
     * No run--time errors in this particular case.
     */

    if(op == "=="){
        if(curr_type.first == grammarDFA::T_BOOL){
            result = get<bool>(lit1) == get<bool>(lit2);
        }
        else if(curr_type.first == grammarDFA::T_INT){
            result = get<int>(lit1) == get<int>(lit2);
        }
        else if(curr_type.first == grammarDFA::T_FLOAT){
            result = get<float>(lit1) == get<float>(lit2);
        }
        else if(curr_type.first == grammarDFA::T_CHAR){
            result = get<char>(lit1) == get<char>(lit2);
        }
        else{
            result = get<string>(lit1) == get<string>(lit2);
        }
    }
    else if(op == "!="){
        if(curr_type.first == grammarDFA::T_BOOL){
            result = get<bool>(lit1) != get<bool>(lit2);
        }
        else if(curr_type.first == grammarDFA::T_INT){
            result = get<int>(lit1) != get<int>(lit2);
        }
        else if(curr_type.first == grammarDFA::T_FLOAT){
            result = get<float>(lit1) != get<float>(lit2);
        }
        else if(curr_type.first == grammarDFA::T_CHAR){
            result = get<char>(lit1) != get<char>(lit2);
        }
        else{
            result = get<string>(lit1) != get<string>(lit2);
        }
    }
    else if(op == "<="){
        if(curr_type.first == grammarDFA::T_BOOL){
            result = get<bool>(lit1) <= get<bool>(lit2);
        }
        else if(curr_type.first == grammarDFA::T_INT){
            result = get<int>(lit1) <= get<int>(lit2);
        }
        else if(curr_type.first == grammarDFA::T_FLOAT){
            result = get<float>(lit1) <= get<float>(lit2);
        }
        else if(curr_type.first == grammarDFA::T_CHAR){
            result = get<char>(lit1) <= get<char>(lit2);
        }
        else{
            result = get<string>(lit1) <= get<string>(lit2);
        }
    }
    else if(op == ">="){
        if(curr_type.first == grammarDFA::T_BOOL){
            result = get<bool>(lit1) >= get<bool>(lit2);
        }
        else if(curr_type.first == grammarDFA::T_INT){
            result = get<int>(lit1) >= get<int>(lit2);
        }
        else if(curr_type.first == grammarDFA::T_FLOAT){
            result = get<float>(lit1) >= get<float>(lit2);
        }
        else if(curr_type.first == grammarDFA::T_CHAR){
            result = get<char>(lit1) >= get<char>(lit2);
        }
        else{
            result = get<string>(lit1) >= get<string>(lit2);
        }
    }
    else if(op == "<"){
        if(curr_type.first == grammarDFA::T_BOOL){
            result = get<bool>(lit1) < get<bool>(lit2);
        }
        else if(curr_type.first == grammarDFA::T_INT){
            result = get<int>(lit1) < get<int>(lit2);
        }
        else if(curr_type.first == grammarDFA::T_FLOAT){
            result = get<float>(lit1) < get<float>(lit2);
        }
        else if(curr_type.first == grammarDFA::T_CHAR){
            result = get<char>(lit1) < get<char>(lit2);
        }
        else{
            result = get<string>(lit1) < get<string>(lit2);
        }
    }
    else{
        if(curr_type.first == grammarDFA::T_BOOL){
            result = get<bool>(lit1) > get<bool>(lit2);
        }
        else if(curr_type.first == grammarDFA::T_INT){
            result = get<int>(lit1) > get<int>(lit2);
        }
        else if(curr_type.first == grammarDFA::T_FLOAT){
            result = get<float>(lit1) > get<float>(lit2);
        }
        else if(curr_type.first == grammarDFA::T_CHAR){
            result = get<char>(lit1) > get<char>(lit2);
        }
        else{
            result = get<string>(lit1) > get<string>(lit2);
        }
    }

    return result;
}

void interpreter::visit(astRELOP* node){
    node->operand1->accept(this); // visit astEXPRESSION node corresponding to first operand
    obj_t op1_value = curr_result; // maintain result for op1

    node->operand2->accept(this); // visit astEXPRESSION node corresponding to second operand
    obj_t op2_value = curr_result; // maintain result for op2

    if(curr_obj_class == grammarDFA::ARRAY){ // in the case of array, carry out overloaded element-wise relop
        literal_arr_t arr1 = get<literal_arr_t>(op1_value); // hold reference to first array operand
        int size1 = arr1->size(); // hold reference to size of first array operand

        literal_arr_t arr2 = get<literal_arr_t>(op2_value); // hold reference to second array operand
        int size2 = arr2->size(); // hold reference to size of second array operand

        if(size1 != size2){ // if sizes do not match, report a run--time error
            std::cerr << "ln " << node->line << ": arrays have mismatched sizes " << size1 << " and " << size2 << std::endl;
            throw std::runtime_error("Runtime errors encountered, see trace above.");
        }

        auto* result = new vector<literal_t>(0);
        for(int i = 0; i < size1; i++){ // call relop on each pair of elements
            result->push_back(relop(node->op, arr1->at(i), arr2->at(i)));
        }

        curr_result = result;
    }
    else{ // else if single element, simply apply addop on the two operands
        curr_result = relop(node->op, get<literal_t>(op1_value), get<literal_t>(op2_value));
    }

    curr_type = type_t(grammarDFA::T_BOOL, "bool"); // for relational operators, the resulting type is always a boolean
}

void interpreter::visit(astAPARAMS* node){
    // for each specified param
    for(size_t i = 0; i < node->n_children; i++){
        (node->children->at(i))->accept(this); // visit astEXPRESSION node
        symbol* aparam;

        if(curr_obj_class == grammarDFA::SINGLETON){ // if singlular item, create new varSymbol to maintain result
            aparam = new varSymbol(nullptr, curr_type);
            aparam->set_object(curr_result);
        }
        else{ // else create new arrSymbol to maintain result (as must be array otherwise)
            aparam = new arrSymbol(nullptr, curr_type, get<literal_arr_t>(curr_result)->size());
            aparam->set_object(curr_result);
        }

        (functionStack->top().first)->fparams->push_back(aparam); // push symbol instance to fparams vector of funcSymbol at the stop of the functionStack
    }
}

void interpreter::visit(astFUNC_CALL* node){
    // extract function identifier from astIDENTIFIER node
    string func_ident = ((astIDENTIFIER*) node->identifier)->lexeme;
    auto* expected_func = new funcSymbol(&func_ident, type_t(grammarDFA::T_TYPE, ""), grammarDFA::FUNCTION, new vector<symbol*>(0));

    // in case function being called is a member of a tlstruct instance
    symbol_table* ref_tmp_symbolTable = lookup_symbolTable; // maintain reference of lookup symbol table
    lookup_symbolTable = curr_symbolTable; // set lookup symbol table to current symbol table

    if(node->aparams != nullptr){ // if we have at least 1 parameter...
        functionStack->push(make_pair(expected_func, false)); // push on top of function stack
        node->aparams->accept(this); // visit astAPARAMS node to type check the parameters and build the function type-signature
        functionStack->pop(); // pop expected_func with generated type signature from function stack
    }

    // revert back lookup and insertion symbol tables...
    lookup_symbolTable = ref_tmp_symbolTable;
    ref_tmp_symbolTable = curr_symbolTable;
    curr_symbolTable = lookup_symbolTable;

    // lookup in symbol table based on fetched identifier and type-signature constructed from visiting astAPARAMS
    // and then push returned funcSymbol onto the function stack...
    functionStack->push(make_pair(curr_symbolTable->lookup(func_ident, expected_func->fparams), false));

    // maintain scoping: new scope for the function definition block
    curr_symbolTable->push_scope();

    /* Binding formal parameters to evaluted right values:
     * For each fparam in the funcSymbol instance at the top of the function stack, there is a corresponding aparam in
     * the expected_func funcSymbol, which in particular has the right-value resulting from the evaluation of the corresponding
     * astEXPRESSION node.
     *
     * Hence for each pair of fparam and aparam, we create a new varSymbol or arrSymbol instance (depending on the object
     * class), with the left value from the fparam and the right value from the aparam. We then push these symbols in
     * the symbol table, i.e. the identifiers defined in the function signature are within the function scope, and bound
     * to the right-values positionally specified in the function call.
     */
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
        curr_symbolTable->insert(aparam);
    }

    delete expected_func; // expected_func no longer required; free associated memory

    // for each child node (i.e. statement) in the astBLOCK associated with the function definition
    for(auto &c : *(functionStack->top().first)->func_ref->children){
        c->accept(this); // visit the node

        if(functionStack->top().second){ // if a return statement is encountered, stop traversing astBLOCK subtree
            break;
        }
    }

    // set type and object class to that of the function return
    curr_type = (functionStack->top().first)->type;
    curr_obj_class = (functionStack->top().first)->ret_obj_class;

    // and set the result associated with the function to curr_result (the value set by astRETURN)
    (functionStack->top().first)->set_object(curr_result);

    functionStack->pop(); // remove funcSymbol from top of the function stack
    curr_symbolTable->pop_scope(); // maintain scoping: pop scope associated with the function

    // set both symbol table references to the 'calling' symbol table
    curr_symbolTable = ref_tmp_symbolTable;
    lookup_symbolTable = curr_symbolTable;
}

void interpreter::visit(astSUBEXPR* node){
    node->subexpr->accept(this);
}

literal_t interpreter::unary(string op, literal_t literal){
    literal_t result;

    if(op == "-"){
        if(curr_type.first == grammarDFA::T_INT){
            result = -1 * get<int>(literal);
        }
        else if(curr_type.first == grammarDFA::T_FLOAT){
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

        auto* result = new vector<literal_t>(0);
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
    symbol* ret_symb = lookup_symbolTable->lookup(((astIDENTIFIER*) node->identifier)->lexeme);
    lookup_symbolTable = curr_symbolTable;

    node->expression->accept(this);

    if(ret_symb->type.first == grammarDFA::T_AUTO){
        ret_symb->type = curr_type;
    }

    if(curr_obj_class == grammarDFA::ARRAY && get<literal_arr_t>(curr_result)->size() != ((arrSymbol*) ret_symb)->size){
        std::cerr << "ln " << node->line << ": arrays have mismatched sizes " << get<literal_arr_t>(curr_result)->size()
        << " and " << ((arrSymbol*) ret_symb)->size << std::endl;
        throw std::runtime_error("Runtime errors encountered, see trace above.");
    }

    ret_symb->set_object(curr_result);
}

void interpreter::visit(astASSIGNMENT_ELEMENT* node){
    string arr_ident = ((astIDENTIFIER*) ((astELEMENT*) node->element)->identifier)->lexeme;
    symbol* ret_symb = lookup_symbolTable->lookup(arr_ident);
    lookup_symbolTable = curr_symbolTable;

    (((astELEMENT*) node->element)->index)->accept(this);
    int index = get<int>(get<literal_t>(curr_result));
    int size = ((arrSymbol*) ret_symb)->size;

    if(size <= index || index < 0){
        std::cerr << "ln " << node->line << ": index " << index << " is out of bounds of array " << arr_ident <<
        " with size " << size << std::endl;
        throw std::runtime_error("Runtime errors encountered, see trace above.");
    }

    node->expression->accept(this);

    if(ret_symb->type.first == grammarDFA::T_AUTO){
        ret_symb->type = curr_type;
    }

    get<literal_arr_t>(ret_symb->object)->at(index) = get<literal_t>(curr_result);
}

void interpreter::visit(astASSIGNMENT_MEMBER* node) {
    string tls_ident = ((astIDENTIFIER*) node->tls_name)->lexeme;
    symbol* ret_symbol = lookup_symbolTable->lookup(tls_ident);

    lookup_symbolTable = get<symbol_table*>(get<literal_t>(ret_symbol->object));
    node->assignment->accept(this);
}

literal_t interpreter::default_literal(type_t type){
    literal_t result;

    if(type.first == grammarDFA::T_BOOL){
        result = false;
    }
    else if(type.first == grammarDFA::T_INT){
        result = 0;
    }
    else if(type.first == grammarDFA::T_FLOAT){
        result = (float) 0.0;
    }
    else if(type.first == grammarDFA::T_CHAR){
        result = '\0';
    }
    else if(type.first == grammarDFA::T_STRING){
        result = "";
    }
    else if(type.first == grammarDFA::T_TLSTRUCT){
        symbol* ret_symbol = lookup_symbolTable->lookup(type.second);
        lookup_symbolTable = curr_symbolTable;

        auto* ref_curr_symbolTable = curr_symbolTable;
        auto* ref_lookup_symbolTable = lookup_symbolTable;
        curr_symbolTable = new symbol_table(ref_curr_symbolTable);
        lookup_symbolTable = curr_symbolTable;

        for(auto &c : *((astBLOCK*) ((tlsSymbol*) ret_symbol)->tls_ref)->children){
            c->accept(this);
        }

        result = curr_symbolTable;
        curr_symbolTable = ref_curr_symbolTable;
        lookup_symbolTable = ref_lookup_symbolTable;
    }

    return result;
}

void interpreter::visit(astVAR_DECL* node){
    string var_ident = ((astIDENTIFIER*) node->identifier)->lexeme;
    type_t var_type(((astTYPE*) node->type)->type, ((astTYPE*) node->type)->lexeme);
    varSymbol* var;

    if(node->expression != nullptr){
        node->expression->accept(this);

        if(var_type.first == grammarDFA::T_AUTO){
            var_type = curr_type;
        }

        var = new varSymbol(&var_ident, var_type);
        var->set_object(curr_result);
    }
    else{
        var = new varSymbol(&var_ident, var_type);
        var->set_object(default_literal(var_type));
    }

    curr_symbolTable->insert(var);
}

void interpreter::visit(astARR_DECL* node){
    string arr_ident = ((astIDENTIFIER*) node->identifier)->lexeme;
    type_t arr_type(((astTYPE*) node->type)->type, ((astTYPE*) node->type)->lexeme);

    node->size->accept(this);
    int size = get<int>(get<literal_t>(curr_result));

    if(size < 1){
        std::cerr << "ln " << node->line << ": size of array " << arr_ident << " must be a positive integer" << std::endl;
        throw std::runtime_error("Runtime errors encountered, see trace above.");
    }

    int n_assignment_elts = node->n_children - 3;
    if(size < n_assignment_elts){
        std::cerr << "ln " << node->line << ": cannot assign " << n_assignment_elts << " elements to array " << arr_ident <<
        " of size " << size << std::endl;
        throw std::runtime_error("Runtime errors encountered, see trace above.");
    }

    literal_arr_t lit_arr = new vector<literal_t>(size);
    if(arr_type.first != grammarDFA::T_AUTO && n_assignment_elts == 0){
        literal_t default_lit_val = default_literal(arr_type);

        for(int i = 0; i < size; i++){
            lit_arr->at(i) = default_lit_val;
        }
    }
    else{
        for(int i = 0; i < n_assignment_elts; i++){
            (node->children->at(i+3))->accept(this);

            if(arr_type.first == grammarDFA::T_AUTO){
                arr_type = curr_type;
            }

            lit_arr->at(i) = get<literal_t>(curr_result);
        }

        for(int i = n_assignment_elts; i < size; i++){
            lit_arr->at(i) = get<literal_t>(curr_result);
        }
    }

    auto* arr = new arrSymbol(&arr_ident, arr_type, size);
    arr->set_object(lit_arr);

    curr_symbolTable->insert(arr);
}

void interpreter::visit(astTLS_DECL* node){
    string tls_ident = ((astIDENTIFIER*) node->identifier)->lexeme;
    auto* tls = new tlsSymbol(&tls_ident, &tls_ident);

    curr_symbolTable->insert(tls);
    tls->set_tls_ref((astBLOCK*) node->tls_block);
}

void interpreter::visit(astPRINT* node){
    node->expression->accept(this);

    if(curr_obj_class == grammarDFA::ARRAY){
        std::cout << "{";
        for(int i = 0; i < get<literal_arr_t>(curr_result)->size(); i++){
            if(i != 0){
                std::cout << ", ";
            }

            if(curr_type.first == grammarDFA::T_BOOL){
                if(get<bool>(get<literal_arr_t>(curr_result)->at(i))){
                    std::cout << "true";
                }
                else{
                    std::cout << "false";
                }
            }
            else if(curr_type.first == grammarDFA::T_INT){
                std::cout << get<int>(get<literal_arr_t>(curr_result)->at(i));
            }
            else if(curr_type.first == grammarDFA::T_FLOAT){
                std::cout << get<float>(get<literal_arr_t>(curr_result)->at(i));
            }
            else if(curr_type.first == grammarDFA::T_CHAR){
                std::cout << get<char>(get<literal_arr_t>(curr_result)->at(i));
            }
            else{
                std::cout << get<string>(get<literal_arr_t>(curr_result)->at(i));
            }
        }

        std::cout << "}" << std::endl;
    }
    else if(curr_type.first == grammarDFA::T_BOOL){
        if(get<bool>(get<literal_t>(curr_result))){
            std::cout << "true" << std::endl;
        }
        else{
            std::cout << "false" << std::endl;
        }
    }
    else if(curr_type.first == grammarDFA::T_INT){
        std::cout << get<int>(get<literal_t>(curr_result)) << std::endl;
    }
    else if(curr_type.first == grammarDFA::T_FLOAT){
        std::cout << get<float>(get<literal_t>(curr_result)) << std::endl;
    }
    else if(curr_type.first == grammarDFA::T_CHAR){
        std::cout << get<char>(get<literal_t>(curr_result)) << std::endl;
    }
    else{
        std::cout << get<string>(get<literal_t>(curr_result)) << std::endl;
    }
}

void interpreter::visit(astRETURN* node){
    node->expression->accept(this);
    functionStack->top().second = true;

    if(functionStack->top().first->type.first == grammarDFA::T_AUTO){
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
    curr_symbolTable->push_scope();

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

    curr_symbolTable->pop_scope();
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
    type_t ret_type = type_t(((astTYPE*) node->type)->type, ((astTYPE*) node->type)->lexeme);
    grammarDFA::Symbol ret_obj_class = ((astTYPE*) node->type)->object_class;

    auto* fparams = new vector<symbol*>(0);
    if(node->fparams != nullptr){
        for(auto &c : *((astFPARAMS*) node->fparams)->children){
            string fparam_ident = ((astIDENTIFIER*) ((astFPARAM*) c)->children->at(0))->lexeme;
            type_t fparam_type = type_t(((astTYPE*) ((astFPARAM*) c)->children->at(1))->type,
                                                    ((astTYPE*) ((astFPARAM*) c)->children->at(1))->lexeme);
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

    curr_symbolTable->insert(func);
}

void interpreter::visit(astMEMBER_ACCESS* node){
    string tls_ident = ((astIDENTIFIER*) node->tls_name)->lexeme;
    symbol* ret_symbol = lookup_symbolTable->lookup(tls_ident);

    lookup_symbolTable = get<symbol_table*>(get<literal_t>(ret_symbol->object));
    node->member->accept(this);
}

void interpreter::visit(astBLOCK* node){
    curr_symbolTable->push_scope();

    for(auto &c : *node->children){
        c->accept(this);

        if(!functionStack->empty() && functionStack->top().second){
            break;
        }
    }

    curr_symbolTable->pop_scope();
}
void interpreter::visit(astPROGRAM* node){
    for(auto &c : *node->children){
        c->accept(this);

        if(!functionStack->empty() && functionStack->top().second){
            break;
        }
    }
}
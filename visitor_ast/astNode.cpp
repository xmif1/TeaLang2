//
// Created by Xandru Mifsud on 27/03/2021.
//

#include "astNode.h"

int astNode::n_nodes = 0;

void astInnerNode::add_child(astNode* child){
    if(n_children < children->size()){
        children->at(n_children) = child;
    }
    else{
        children->push_back(child);
    }

    n_children++;
}

string astInnerNode::getLabel(){
    return to_string(node_id) + "_" + symbol;
}

string astLeafNode::getLabel(){
    string lexeme_copy = lexeme;

    std::size_t opening_dquotes = lexeme_copy.find_first_of('\"');
    if(opening_dquotes != string::npos){
        lexeme_copy.insert(opening_dquotes, 1, '\\');
    }

    std::size_t closing_dquotes = lexeme_copy.find_last_of('\"');
    if(closing_dquotes != string::npos){
        lexeme_copy.insert(closing_dquotes, 1, '\\');
    }

    return to_string(node_id) + "_" + symbol + "(" + lexeme_copy + ")";
}

string astBinaryOp::getLabel(){
    return to_string(node_id) + "_" + op;
}

string astUNARY::getLabel(){
    return to_string(node_id) + "_" + op;
}

void astTYPE::accept(visitor* v){
    v->visit(this);
}

void astLITERAL::accept(visitor* v){
    v->visit(this);
}

void astIDENTIFIER::accept(visitor* v){
    v->visit(this);
}

void astELEMENT::accept(visitor* v){
    identifier = children->at(0);
    index = children->at(1);
    v->visit(this);
}

void astMULTOP::accept(visitor* v){
    operand1 = children->at(0);
    operand2 = children->at(1);
    v->visit(this);
}

void astADDOP::accept(visitor* v){
    operand1 = children->at(0);
    operand2 = children->at(1);
    v->visit(this);
}

void astRELOP::accept(visitor* v){
    operand1 = children->at(0);
    operand2 = children->at(1);
    v->visit(this);
}

void astAPARAMS::accept(visitor* v){
    v->visit(this);
}

void astFUNC_CALL::accept(visitor* v){
    identifier = children->at(0);
    aparams = children->at(1);
    v->visit(this);
}

void astSUBEXPR::accept(visitor* v){
    subexpr = children->at(0);
    v->visit(this);
}

void astUNARY::accept(visitor* v){
    operand = children->at(0);
    v->visit(this);
}

void astASSIGNMENT_IDENTIFIER::accept(visitor* v){
    identifier = children->at(0);
    expression = children->at(1);
    v->visit(this);
}

void astASSIGNMENT_ELEMENT::accept(visitor* v){
    element = children->at(0);
    expression = children->at(1);
    v->visit(this);
}

void astVAR_DECL::accept(visitor* v){
    identifier = children->at(0);
    type = children->at(1);
    expression = children->at(2);
    v->visit(this);
}

void astARR_DECL::accept(visitor* v){
    identifier = children->at(0);
    size = children->at(1);
    type = children->at(2);
    v->visit(this);
}

void astPRINT::accept(visitor* v){
    expression = children->at(0);
    v->visit(this);
}

void astRETURN::accept(visitor* v){
    expression = children->at(0);
    v->visit(this);
}

void astIF::accept(visitor* v){
    expression = children->at(0);
    if_block = children->at(1);
    else_block = children->at(2);
    v->visit(this);
}

void astFOR::accept(visitor* v){
    var_decl = children->at(0);
    expression = children->at(1);
    assignment = children->at(2);
    for_block = children->at(3);
    v->visit(this);
}

void astWHILE::accept(visitor* v){
    expression = children->at(0);
    while_block = children->at(1);
    v->visit(this);
}

void astFPARAMS::accept(visitor* v){
    v->visit(this);
}

void astFPARAM::accept(visitor* v){
    identifier = children->at(0);
    type = children->at(1);
    v->visit(this);
}

void astFUNC_DECL::accept(visitor* v){
    type = children->at(0);
    identifier = children->at(1);
    fparams = children->at(2);
    function_block = children->at(3);
    v->visit(this);
}

void astBLOCK::accept(visitor* v){
    v->visit(this);
}

void astPROGRAM::accept(visitor* v){
    v->visit(this);
}
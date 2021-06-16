//
// Created by Xandru Mifsud on 27/03/2021.
//

#include "astNode.h"

int astNode::n_nodes = 0;

/* Convenience function for adding child nodes. Since we maintain positionality (see the discussion in the 'Concrete
 * Implementations' section of astNode.h as well as the section on the AST in the report), some child arrays have a pre-
 * declared size and we must ensure that we first populate those containers in the vector first. This function ensures
 * that.
 */
void astInnerNode::add_child(astNode* child){
    if(n_children < children->size()){
        children->at(n_children) = child;
    }
    else{
        children->push_back(child);
    }

    n_children++;
}

// Convenience function with returns a unique label for a node, based on its attributed (textual) symbol and unique id.
string astInnerNode::getLabel(){
    return to_string(node_id) + "_" + symbol;
}

/* Convenience function with returns a unique label for a node, based on its attributed (textual) symbol, unique id, and
 * associated lexeme. In the case of the lexeme being a string literal, we ensure that the double quotes are escaped for
 * printing.
 */
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

// Convenience function with returns a unique label for a node, based on its attributed opcode symbol and unique id.
string astBinaryOp::getLabel(){
    return to_string(node_id) + "_" + op;
}

// Convenience function with returns a unique label for a node, based on its attributed opcode symbol and unique id.
string astUNARY::getLabel(){
    return to_string(node_id) + "_" + op;
}

/* The following are the respective accept definitions for each concrete astNode implementation. Each call to accept:
 * (i) Binds any references (defined for convenience) of child nodes to the respective positional entry in the vector
 * (ii) Calls the visitors visit() function with the instance of the astNode concrete class, resulting in the execution
 *      of the correct overloaded handler.
 */

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

void astASSIGNMENT_MEMBER::accept(visitor* v){
    tls_name = children->at(0);
    assignment = children->at(1);
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

void astTLS_DECL::accept(visitor* v){
    identifier = children->at(0);
    tls_block = children->at(1);
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
    decl = children->at(0);
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

void astMEMBER_ACCESS::accept(visitor* v){
    tls_name = children->at(0);
    member = children->at(1);
    v->visit(this);
}

void astBLOCK::accept(visitor* v){
    v->visit(this);
}

void astPROGRAM::accept(visitor* v){
    v->visit(this);
}
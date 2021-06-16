//
// Created by Xandru Mifsud on 27/03/2021.
//

#ifndef CPS2000_ASTNODE_H
#define CPS2000_ASTNODE_H

#include <vector>
#include <string>
#include "visitor.h"
#include "../lexer/grammarDFA.h"

using namespace std;

class visitor;

/* Defines an instance of an abstract syntax tree node (constructed by the parser), outlining the minimum amount of meta
 * -data required to be maintained. Derivatives of this class may add further meta-data requirements. Indeed, we have a
 * concrete implementation for each (more or less) of the definitions in the EBNF.
 *
 * Typical to tree structures, we maintain a reference to the parent astNode instance (unless the node is the root, which
 * is always in fact an astPROGRAM node). We do not however maintain a reference to the child nodes. Instead, we further
 * define abstract internal and leaf node classes.
 *
 * We maintain a number of information which, per-se, is not necessary and for purposes outside this assignment, we can
 * do without. This includes textual information, a unique node_id, etc, which allows the ability to generate a detailed
 * and meaningful pictorial representation of the abstract syntax tree.
 *
 * We also maintain useful meta-data as well however, such as the line number of the token (or first token in the sequence
 * of tokens) associated with the astNode.
 *
 * Each concrete implementation must implement the accpet(visitor* v) function, to support the visitor design pattern
 * which we heavily use to traverse the abstract syntax tree and carry out specific operations based on the node instance.
 */
class astNode{
public:
    astNode* parent;
    string symbol;
    unsigned int line;
    static int n_nodes;
    int node_id;

    virtual void accept(visitor* v) = 0;
    virtual string getLabel() = 0;

    astNode(astNode* parent, string symbol, unsigned int line){
        n_nodes++;
        node_id = n_nodes;

        this->parent = parent;
        this->symbol = symbol;
        this->line = line;
    }
};

/* Adds the support of maintaining references to child astNode instances, by maintaining a vector of pointers to astNode
 * instances. A convenience function for adding child nodes is defined.
 */
class astInnerNode: public astNode{
public:
    vector<astNode*>* children = new vector<astNode*>(0);
    int n_children = 0;

    virtual void accept(visitor* v) = 0;
    void add_child(astNode*);
    string getLabel();

    astInnerNode(astInnerNode* parent, string symbol, unsigned int line) : astNode(parent, symbol, line){};
};

/* The abstract syntax tree we construct is in such a manner such that the leaf nodes represent some terminal symbol,
 * such as an identifier or literal. Hence we must maintain the associated lexeme, for use in eg. constructing the
 * symbol table, assignment with literals during compilation, etc. The astLeadNode implementation adds this support.
 */
class astLeafNode: public astNode{
public:
    string lexeme;

    virtual void accept(visitor* v) = 0;
    string getLabel();

    astLeafNode(astInnerNode* parent, string symbol, unsigned int line, string lexeme) :
        astNode(parent, symbol, line){
            this->lexeme = std::move(lexeme);
    };
};

/* For conveience, we also define an abstract astNode derivative for binary operands, which in particular maintains the
 * opcode and reference to the two astNode instances representing the operands.
 */
class astBinaryOp: public astInnerNode{
public:
    string op;
    astNode* operand1;
    astNode* operand2;

    virtual void accept(visitor* v) = 0;
    string getLabel();

    astBinaryOp(astInnerNode* parent, string op, string symbol, unsigned int line) :
        astInnerNode(parent, symbol, line){
            this->op = op;
            children->resize(2, nullptr);
            operand1 = children->at(0);
            operand2 = children->at(1);
    }
};

/* --------------------------------------------- CONCRETE IMPLEMENTATIONS ---------------------------------------------
 * What follows are the concrete implementations for the astNode instances associated with the EBNF definitions. Some
 * concrete implementations have positional children, which is to say we expect certain node instances at certain indices
 * in the vector of child nodes.
 *
 * For example, consider astFOR, which represents a for--statement. Syntactically, we can expect a declaration statement,
 * followed by an expression, then an assignment, and then a block of statements. In this case, these would respectively
 * occupy indices 0 up to 3 in the child node vector. However, the declaration and assignment statements are optional;
 * in the case that these are omitted, a nullptr reference is maintained instead at that positional index. In this manner,
 * we preserve positionality. To this extent, certain concrete node implementations bind certain elements in the child
 * node vector to a named variable, for convenience. For example, in the case of astFOR, we have astNode* for_block =
 * children->at(3).
 * --------------------------------------------------------------------------------------------------------------------
 */

class astTYPE: public astLeafNode{
public:
    grammarDFA::Symbol type;
    grammarDFA::Symbol object_class;

    astTYPE(astInnerNode* parent, string lexeme, grammarDFA::Symbol type, grammarDFA::Symbol object_class, unsigned int line):
        astLeafNode(parent, "T_TYPE", line, std::move(lexeme)){
        this->type = type;
        this->object_class = object_class;
    }

    void accept(visitor* v) override;
};

class astLITERAL: public astLeafNode{
public:
    grammarDFA::Symbol type;
    string type_str;

    astLITERAL(astInnerNode* parent, string lexeme, grammarDFA::Symbol type, string type_str, unsigned int line) :
        astLeafNode(parent, "LITERAL", line,std::move(lexeme)){
            this->type = type;
            this->type_str = type_str;
    }

    void accept(visitor* v) override;
};

class astIDENTIFIER: public astLeafNode{
public:
    astIDENTIFIER(astInnerNode* parent, string lexeme, unsigned int line) : astLeafNode(parent, "T_IDENTIFIER",
                                                                                        line, std::move(lexeme)){}

    void accept(visitor* v) override;
};

class astELEMENT: public astInnerNode{
public:
    astNode* identifier;
    astNode* index;

    explicit astELEMENT(astInnerNode* parent, unsigned int line) : astInnerNode(parent, "ELEMENT", line){
        children->resize(2, nullptr);
    }

    void accept(visitor* v) override;
};

class astMULTOP: public astBinaryOp{
public:
    astMULTOP(astInnerNode* parent, string op, unsigned int line) :
    astBinaryOp(parent, op, "MULTOP", line){}

    void accept(visitor* v) override;
};

class astADDOP: public astBinaryOp{
public:
    astADDOP(astInnerNode* parent, string op, unsigned int line) :
            astBinaryOp(parent, op, "ADDOP", line){}

    void accept(visitor* v) override;
};

class astRELOP: public astBinaryOp{
public:
    astRELOP(astInnerNode* parent, string op, unsigned int line) :
            astBinaryOp(parent, op, "RELOP", line){}

    void accept(visitor* v) override;
};

class astAPARAMS: public astInnerNode{
public:
    explicit astAPARAMS(astInnerNode* parent, unsigned int line) : astInnerNode(parent, "APARAMS", line){}

    void accept(visitor* v) override;
};

class astFUNC_CALL: public astInnerNode{
public:
    astNode* identifier;
    astNode* aparams;

    explicit astFUNC_CALL(astInnerNode* parent, unsigned int line) : astInnerNode(parent, "FUNC_CALL", line){
        children->resize(2, nullptr);
    }

    void accept(visitor* v) override;
};

class astSUBEXPR: public astInnerNode{
public:
    astNode* subexpr;

    explicit astSUBEXPR(astInnerNode* parent, unsigned int line) : astInnerNode(parent, "SUBEXPR", line){
        children->resize(1, nullptr);
    }

    void accept(visitor* v) override;
};

class astUNARY: public astInnerNode{
public:
    string op;
    astNode* operand;

    explicit astUNARY(astInnerNode* parent, string op, unsigned int line) :
        astInnerNode(parent, "UNARY", line){
            this->op = op;
            children->resize(1, nullptr);
    }

    string getLabel() override;
    void accept(visitor* v) override;
};

class astASSIGNMENT_IDENTIFIER: public astInnerNode{
public:
    astNode* identifier;
    astNode* expression;

    explicit astASSIGNMENT_IDENTIFIER(astInnerNode* parent, unsigned int line) : astInnerNode(parent, "ASSIGNMENT", line){
        children->resize(2, nullptr);
    }

    void accept(visitor* v) override;
};

class astASSIGNMENT_ELEMENT: public astInnerNode{
public:
    astNode* element;
    astNode* expression;

    explicit astASSIGNMENT_ELEMENT(astInnerNode* parent, unsigned int line) : astInnerNode(parent, "ASSIGNMENT", line){
        children->resize(2, nullptr);
    }

    void accept(visitor* v) override;
};

class astASSIGNMENT_MEMBER: public astInnerNode{
public:
    astNode* tls_name;
    astNode* assignment;

    explicit astASSIGNMENT_MEMBER(astInnerNode* parent, unsigned int line) : astInnerNode(parent, "ASSIGNMENT", line){
        children->resize(2, nullptr);
    }

    void accept(visitor* v) override;
};

class astVAR_DECL: public astInnerNode{
public:
    astNode* identifier;
    astNode* type;
    astNode* expression;

    explicit astVAR_DECL(astInnerNode* parent, unsigned int line) : astInnerNode(parent, "VAR_DECL", line){
        children->resize(3, nullptr);
    }

    void accept(visitor* v) override;
};

class astARR_DECL: public astInnerNode{
public:
    astNode* identifier;
    astNode* size;
    astNode* type;

    explicit astARR_DECL(astInnerNode* parent, unsigned int line) : astInnerNode(parent, "ARR_DECL", line){
        children->resize(3, nullptr);
    }

    void accept(visitor* v) override;
};

class astTLS_DECL: public astInnerNode{
public:
    astNode* identifier;
    astNode* tls_block;

    explicit astTLS_DECL(astInnerNode* parent, unsigned int line) : astInnerNode(parent, "TLS_DECL", line){
        children->resize(2, nullptr);
    }

    void accept(visitor* v) override;
};

class astPRINT: public astInnerNode{
public:
    astNode* expression;

    explicit astPRINT(astInnerNode* parent, unsigned int line) : astInnerNode(parent, "PRINT", line){
        children->resize(1, nullptr);
    }

    void accept(visitor* v) override;
};

class astRETURN: public astInnerNode{
public:
    astNode* expression;

    explicit astRETURN(astInnerNode* parent, unsigned int line) : astInnerNode(parent, "RETURN", line){
        children->resize(1, nullptr);
    }

    void accept(visitor* v) override;
};

class astIF: public astInnerNode{
public:
    astNode* expression;
    astNode* if_block;
    astNode* else_block;

    explicit astIF(astInnerNode* parent, unsigned int line) : astInnerNode(parent, "IF", line){
        children->resize(3, nullptr);
    }

    void accept(visitor* v) override;
};

class astFOR: public astInnerNode{
public:
    astNode* decl;
    astNode* expression;
    astNode* assignment;
    astNode* for_block;

    explicit astFOR(astInnerNode* parent, unsigned int line) : astInnerNode(parent, "FOR", line){
        children->resize(4, nullptr);
    }

    void accept(visitor* v) override;
};

class astWHILE: public astInnerNode{
public:
    astNode* expression;
    astNode* while_block;

    explicit astWHILE(astInnerNode* parent, unsigned int line) : astInnerNode(parent, "WHILE", line){
        children->resize(2, nullptr);
    }

    void accept(visitor* v) override;
};

class astFPARAMS: public astInnerNode{
public:
    explicit astFPARAMS(astInnerNode* parent, unsigned int line) : astInnerNode(parent, "FPARAMS", line){}

    void accept(visitor* v) override;
};

class astFPARAM: public astInnerNode{
public:
    astNode* identifier;
    astNode* type;

    explicit astFPARAM(astInnerNode* parent, unsigned int line) : astInnerNode(parent, "FPARAM", line){
        children->resize(2, nullptr);
    }

    void accept(visitor* v) override;
};

class astFUNC_DECL: public astInnerNode{
public:
    astNode* type;
    astNode* identifier;
    astNode* fparams;
    astNode* function_block;

    explicit astFUNC_DECL(astInnerNode* parent, unsigned int line) : astInnerNode(parent, "FUNC_DECL", line){
        children->resize(4, nullptr);
    }

    void accept(visitor* v) override;
};

class astMEMBER_ACCESS: public astInnerNode{
public:
    astNode* tls_name;
    astNode* member;

    explicit astMEMBER_ACCESS(astInnerNode* parent, unsigned int line) : astInnerNode(parent, "MEMBER_ACCESS", line){
        children->resize(2, nullptr);
    }

    void accept(visitor* v) override;
};

class astBLOCK: public astInnerNode{
public:
    explicit astBLOCK(astInnerNode* parent, unsigned int line) : astInnerNode(parent, "BLOCK", line){}

    void accept(visitor* v) override;
};

class astPROGRAM: public astInnerNode{
public:
    explicit astPROGRAM(astInnerNode* parent, unsigned int line) : astInnerNode(parent, "PROGRAM", line){}

    void accept(visitor* v) override;
};

#endif //CPS2000_ASTNODE_H

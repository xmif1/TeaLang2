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

class astInnerNode: public astNode{
public:
    vector<astNode*>* children = new vector<astNode*>(0);
    int n_children = 0;

    virtual void accept(visitor* v) = 0;
    void add_child(astNode*);
    string getLabel();

    astInnerNode(astInnerNode* parent, string symbol, unsigned int line) : astNode(parent, symbol, line){};
};

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

    astLITERAL(astInnerNode* parent, string lexeme, grammarDFA::Symbol type, unsigned int line) :
        astLeafNode(parent, "LITERAL", line,std::move(lexeme)){
            this->type = type;
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

class astASSIGNMENT_MEMBER_ACCESS: public astInnerNode{
public:
    astNode* member_acc;
    astNode* expression;

    explicit astASSIGNMENT_MEMBER_ACCESS(astInnerNode* parent, unsigned int line) : astInnerNode(parent, "ASSIGNMENT", line){
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
    explicit astMEMBER_ACCESS(astInnerNode* parent, unsigned int line) : astInnerNode(parent, "MEMBER_ACCESS", line){}

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

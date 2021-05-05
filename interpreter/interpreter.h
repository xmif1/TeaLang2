//
// Created by Xandru Mifsud on 17/04/2021.
//

#ifndef CPS2000_INTERPRETER_H
#define CPS2000_INTERPRETER_H

#include "../symbol_table/symbol_table.h"
#include "../symbol_table/symbol.h"
#include "../visitor_ast/visitor.h"
#include <iostream>

class interpreter: public visitor{
public:
    void visit(astTYPE* node) override;
    void visit(astLITERAL* node) override;
    void visit(astIDENTIFIER* node) override;
    void visit(astELEMENT* node) override;
    void visit(astMULTOP* node) override;
    void visit(astADDOP* node) override;
    void visit(astRELOP* node) override;
    void visit(astAPARAMS* node) override;
    void visit(astFUNC_CALL* node) override;
    void visit(astSUBEXPR* node) override;
    void visit(astUNARY* node) override;
    void visit(astASSIGNMENT_IDENTIFIER* node) override;
    void visit(astASSIGNMENT_ELEMENT* node) override;
    void visit(astVAR_DECL* node) override;
    void visit(astARR_DECL* node) override;
    void visit(astPRINT* node) override;
    void visit(astRETURN* node) override;
    void visit(astIF* node) override;
    void visit(astFOR* node) override;
    void visit(astWHILE* node) override;
    void visit(astFPARAMS* node) override;
    void visit(astFPARAM* node) override;
    void visit(astFUNC_DECL* node) override;
    void visit(astBLOCK* node) override;
    void visit(astPROGRAM* node) override;

private:
    symbol_table* symbolTable = new symbol_table();
    stack<pair<funcSymbol*, bool>>* functionStack = new stack<pair<funcSymbol*, bool>>; // 2nd used to check if func returns
    grammarDFA::Symbol curr_type;
    grammarDFA::Symbol curr_obj_class;
    obj_t curr_result;

    literal_t multop(string op, int line, literal_t lit1, literal_t lit2);
    literal_t addop(string op, literal_t lit1, literal_t lit2);
    literal_t relop(string op, literal_t lit1, literal_t lit2);
    literal_t unary(string op, literal_t literal);
    literal_t default_literal(grammarDFA::Symbol type);
};

#endif //CPS2000_INTERPRETER_H

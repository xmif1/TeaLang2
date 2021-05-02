//
// Created by Xandru Mifsud on 14/04/2021.
//

#ifndef CPS2000_SEMANTIC_ANALYSIS_H
#define CPS2000_SEMANTIC_ANALYSIS_H

#include "../symbol_table/symbol_table.h"
#include "../symbol_table/symbol.h"
#include "../visitor_ast/visitor.h"
#include <iostream>

class semantic_analysis: public visitor{
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
    stack<pair<funcSymbol*, bool>>* functionStack = new stack<pair<funcSymbol*, bool>>; // 2nd used to check if func returns
    symbol_table* symbolTable = new symbol_table();
    bool type_deduction_reqd = false;
    grammarDFA::Symbol curr_type;
    grammarDFA::Symbol curr_obj_class;

    static string type_symbol2string(grammarDFA::Symbol type, grammarDFA::Symbol obj_class);
    static string typeVect_symbol2string(vector<symbol*>* typeVect);
    void binop_type_check(astBinaryOp* binop_node);
};

#endif //CPS2000_SEMANTIC_ANALYSIS_H

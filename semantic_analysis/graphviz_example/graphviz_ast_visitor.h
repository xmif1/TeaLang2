//
// Created by Xandru Mifsud on 29/03/2021.
//

#ifndef CPS2000_GRAPHVIZ_AST_VISITOR_H
#define CPS2000_GRAPHVIZ_AST_VISITOR_H

#include <iostream>
#include <fstream>
#include "../../visitor_ast/visitor.h"

// Based off of the example in the class notes. Useful for debugging purposes etc.
class graphviz_ast_visitor: public visitor{
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
    void visit(astASSIGNMENT_MEMBER *node) override;
    void visit(astVAR_DECL* node) override;
    void visit(astARR_DECL* node) override;
    void visit(astTLS_DECL* node) override;
    void visit(astPRINT* node) override;
    void visit(astRETURN* node) override;
    void visit(astIF* node) override;
    void visit(astFOR* node) override;
    void visit(astWHILE* node) override;
    void visit(astFPARAMS* node) override;
    void visit(astFPARAM* node) override;
    void visit(astFUNC_DECL* node) override;
    void visit(astMEMBER_ACCESS* node) override;
    void visit(astBLOCK* node) override;
    void visit(astPROGRAM* node) override;

    explicit graphviz_ast_visitor();

private:
    std::ofstream outfile;
};


#endif //CPS2000_GRAPHVIZ_AST_VISITOR_H

//
// Created by Xandru Mifsud on 29/03/2021.
//

#ifndef CPS2000_VISITOR_H
#define CPS2000_VISITOR_H

#include "astNode.h"

class astTYPE;
class astLITERAL;
class astIDENTIFIER;
class astELEMENT;
class astMULTOP;
class astADDOP;
class astRELOP;
class astAPARAMS;
class astFUNC_CALL;
class astSUBEXPR;
class astUNARY;
class astASSIGNMENT_IDENTIFIER;
class astASSIGNMENT_ELEMENT;
class astASSIGNMENT_MEMBER_ACCESS;
class astVAR_DECL;
class astARR_DECL;
class astTLS_DECL;
class astPRINT;
class astRETURN;
class astIF;
class astFOR;
class astWHILE;
class astFPARAMS;
class astFPARAM;
class astFUNC_DECL;
class astMEMBER_ACCESS;
class astBLOCK;
class astPROGRAM;

class visitor{
public:
    virtual void visit(astTYPE* ast_type) = 0;
    virtual void visit(astLITERAL* ast_literal) = 0;
    virtual void visit(astIDENTIFIER* ast_identifier) = 0;
    virtual void visit(astELEMENT* ast_element) = 0;
    virtual void visit(astMULTOP* ast_multop) = 0;
    virtual void visit(astADDOP* ast_addop) = 0;
    virtual void visit(astRELOP* ast_relop) = 0;
    virtual void visit(astAPARAMS* ast_aparams) = 0;
    virtual void visit(astFUNC_CALL* ast_func_call) = 0;
    virtual void visit(astSUBEXPR* ast_subexpr) = 0;
    virtual void visit(astUNARY* ast_unary) = 0;
    virtual void visit(astASSIGNMENT_IDENTIFIER* ast_assignment) = 0;
    virtual void visit(astASSIGNMENT_ELEMENT* ast_assignment) = 0;
    virtual void visit(astASSIGNMENT_MEMBER_ACCESS* ast_assignment) = 0;
    virtual void visit(astVAR_DECL* ast_var_decl) = 0;
    virtual void visit(astARR_DECL* ast_arr_decl) = 0;
    virtual void visit(astTLS_DECL* ast_tls_decl) = 0;
    virtual void visit(astPRINT* ast_print) = 0;
    virtual void visit(astRETURN* ast_return) = 0;
    virtual void visit(astIF* ast_if) = 0;
    virtual void visit(astFOR* ast_for) = 0;
    virtual void visit(astWHILE* ast_while) = 0;
    virtual void visit(astFPARAMS* ast_fparams) = 0;
    virtual void visit(astFPARAM* ast_fparam) = 0;
    virtual void visit(astFUNC_DECL* ast_func_decl) = 0;
    virtual void visit(astMEMBER_ACCESS* ast_member_acc) = 0;
    virtual void visit(astBLOCK* ast_block) = 0;
    virtual void visit(astPROGRAM* ast_program) = 0;
};

#endif //CPS2000_VISITOR_H

//
// Created by Xandru Mifsud on 23/03/2021.
//

#ifndef CPS2000_PARSER_H
#define CPS2000_PARSER_H

#include <iostream>
#include <stack>
#include "../visitor_ast/astNode.h"
#include "../lexer/lexer.h"
#include "../lexer/grammarDFA.h"

class parser{
public:
    struct State{
        astInnerNode* parent;
        grammarDFA::Symbol symbol;
    };

    astPROGRAM* root;

    explicit parser(lexer* lexer_ptr);

private:
    typedef void (parser::*production_rule)(astInnerNode*,  lexer::Token*);
    typedef void (parser::*parse_error)(unsigned int);

    stack<State> state_stack;
    int err_count = 0;

    void genericError(unsigned int);
    void rulePROGRAM(astInnerNode*,  lexer::Token*);
    void ruleBLOCK(astInnerNode*,  lexer::Token*);
    void ruleBLOCK_ext(astInnerNode*,  lexer::Token*);
    void ruleVAR_DECL(astInnerNode*,  lexer::Token*);
    void ruleVAR_DECL_ASSIGNMENT(astInnerNode*,  lexer::Token*);
    void ruleARR_DECL(astInnerNode*,  lexer::Token*);
    void ruleARR_DECL_ASSIGNMENT(astInnerNode*,  lexer::Token*);
    void ruleARR_DECL_ASSIGNMENT_ext(astInnerNode*,  lexer::Token*);
    void ruleASSIGNMENT_IDENTIFIER(astInnerNode*,  lexer::Token*);
    void ruleASSIGNMENT_ELEMENT(astInnerNode*,  lexer::Token*);
    void ruleASSIGNMENT_MEMBER(astInnerNode *parent, lexer::Token *token_ptr);
    void rulePRINT(astInnerNode*,  lexer::Token*);
    void ruleRETURN(astInnerNode*,  lexer::Token*);
    void ruleWHILE(astInnerNode*,  lexer::Token*);
    void ruleFPARAM(astInnerNode*,  lexer::Token*);
    void ruleFPARAM_TYPE_ARR(astInnerNode*,  lexer::Token*);
    void ruleFPARAM_TYPE_VAR(astInnerNode*,  lexer::Token*);
    void ruleFPARAMS(astInnerNode*,  lexer::Token*);
    void ruleFPARAMS_ext_T_COMMA(astInnerNode*,  lexer::Token*);
    void ruleAPARAMS(astInnerNode*,  lexer::Token*);
    void ruleAPARAMS_ext_T_COMMA(astInnerNode*,  lexer::Token*);
    void ruleSUBEXPR(astInnerNode*,  lexer::Token*);
    void ruleLITERAL_T_BOOL(astInnerNode*,  lexer::Token*);
    void ruleLITERAL_T_INT(astInnerNode*,  lexer::Token*);
    void ruleLITERAL_T_FLOAT(astInnerNode*,  lexer::Token*);
    void ruleLITERAL_T_STRING(astInnerNode*,  lexer::Token*);
    void ruleLITERAL_T_CHAR(astInnerNode*,  lexer::Token*);
    void ruleUNARY_T_MINUS(astInnerNode*,  lexer::Token*);
    void ruleUNARY_T_NOT(astInnerNode*,  lexer::Token*);
    void ruleFUNC_CALL(astInnerNode*,  lexer::Token*);
    void ruleFUNC_CALL_APARAMS(astInnerNode*,  lexer::Token*);
    void ruleIF(astInnerNode*,  lexer::Token*);
    void ruleELSE_T_ELSE(astInnerNode*,  lexer::Token*);
    void ruleFUNC_DECL(astInnerNode*,  lexer::Token*);
    void ruleFUNC_DECL_ARR(astInnerNode*,  lexer::Token*);
    void ruleFUNC_DECL_FPARAMS(astInnerNode*,  lexer::Token*);
    void ruleFOR(astInnerNode*,  lexer::Token*);
    void ruleFOR_DECL(astInnerNode*,  lexer::Token*);
    void ruleFOR_EXPRESSION(astInnerNode*,  lexer::Token*);
    void ruleFOR_ASSIGNMENT(astInnerNode*,  lexer::Token*);
    void ruleSTATEMENT_T_LET_DECL(astInnerNode*,  lexer::Token*);
    void ruleSTATEMENT_T_IDENTIFIER_ASSIGNMENT(astInnerNode*,  lexer::Token*);
    void ruleSTATEMENT_T_IDENTIFIER_MEMBER_ACC(astInnerNode*,  lexer::Token*);
    void ruleSTATEMENT_T_IDENTIFIER_AS_TYPE(astInnerNode*,  lexer::Token*);
    void ruleSTATEMENT_T_PRINT(astInnerNode*,  lexer::Token*);
    void ruleSTATEMENT_T_IF(astInnerNode*,  lexer::Token*);
    void ruleSTATEMENT_T_FOR(astInnerNode*,  lexer::Token*);
    void ruleSTATEMENT_T_WHILE(astInnerNode*,  lexer::Token*);
    void ruleSTATEMENT_T_RETURN(astInnerNode*,  lexer::Token*);
    void ruleSTATEMENT_T_TYPE(astInnerNode*,  lexer::Token*);
    void ruleSTATEMENT_T_LBRACE(astInnerNode*,  lexer::Token*);
    void ruleSTATEMENT_T_TLSTRUCT(astInnerNode*,  lexer::Token*);
    void ruleFACTOR_LITERAL(astInnerNode*,  lexer::Token*);
    void ruleFACTOR_SUBEXPR(astInnerNode*,  lexer::Token*);
    void ruleFACTOR_UNARY(astInnerNode*,  lexer::Token*);
    void ruleFACTOR_FUNC_CALL(astInnerNode*,  lexer::Token*);
    void ruleFACTOR_ELEMENT(astInnerNode*,  lexer::Token*);
    void ruleFACTOR_IDENTIFIER(astInnerNode*,  lexer::Token*);
    void ruleFACTOR_MEMBER_ACCESS(astInnerNode*,  lexer::Token*);
    void ruleTERM(astInnerNode*,  lexer::Token*);
    void ruleTERM_ext(astInnerNode*,  lexer::Token*);
    void ruleS_EXPR(astInnerNode*,  lexer::Token*);
    void ruleS_EXPR_ext(astInnerNode*,  lexer::Token*);
    void ruleEXPRESSION(astInnerNode*,  lexer::Token*);
    void ruleEXPRESSION_ext(astInnerNode*,  lexer::Token*);
    void ruleTYPE_VAR_T_TYPE(astInnerNode*,  lexer::Token*);
    void ruleTYPE_ARR_T_TYPE(astInnerNode*,  lexer::Token*);
    void ruleTYPE_VAR_T_IDENTIFIER(astInnerNode*,  lexer::Token*);
    void ruleTYPE_ARR_T_IDENTIFIER(astInnerNode*,  lexer::Token*);
    void ruleIDENTIFIER(astInnerNode*,  lexer::Token*);
    void ruleELEMENT(astInnerNode*,  lexer::Token*);
    void ruleMEMBER_FUNC_CALL(astInnerNode*,  lexer::Token*);
    void ruleMEMBER_ELEMENT(astInnerNode*,  lexer::Token*);
    void ruleMEMBER_IDENTIFIER(astInnerNode*,  lexer::Token*);
    void ruleMEMBER_ACCESS(astInnerNode*, lexer::Token*);
    void ruleTLS_DECL(astInnerNode*, lexer::Token*);
    void null_rule(astInnerNode*,  lexer::Token*);
    void optional_pass_rule(astInnerNode*,  lexer::Token*);
    void panic_mode_recovery(lexer* lexer_ptr, lexer::Token*, State*);
    static parser::production_rule parse_table(int, int, lexer*);
    static parser::parse_error error_table(int, int);
    static grammarDFA::Symbol type_string2symbol(const string &type);
};

#endif //CPS2000_PARSER_H

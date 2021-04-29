//
// Created by Xandru Mifsud on 23/03/2021.
//

#include "parser.h"

parser::parser(lexer* lexer_ptr){
    state_stack.push({.parent = nullptr, .symbol = grammarDFA::T_EOF});
    root = new astPROGRAM(nullptr, 1);
    State curr_state = {.parent = root, .symbol = grammarDFA::PROGRAM};
    lexer::Token curr_token;

    do{ // fetch tokens, ignoring comment tokens
        if(!(*lexer_ptr).getNextToken(&curr_token)){
            throw std::runtime_error("Lexer encountered an error while fetching the next token...exiting...");
        }
    }while(curr_token.symbol == grammarDFA::T_COMMENT);

    while(!state_stack.empty()){
        if(curr_state.symbol >= grammarDFA::n_NTS){
            if(curr_token.symbol != curr_state.symbol){
                parser::parse_error err = error_table(curr_state.symbol, curr_token.symbol);
                (this->*err)(curr_token.line);

                panic_mode_recovery(lexer_ptr, &curr_token, &curr_state);
            }
            else{
                do{ // fetch tokens, ignoring comment tokens
                    if(!(*lexer_ptr).getNextToken(&curr_token)){
                        throw std::runtime_error("Lexer encountered an error while fetching the next token...exiting...");
                    }
                }while(curr_token.symbol == grammarDFA::T_COMMENT);

                curr_state = state_stack.top(); state_stack.pop();
            }
        }
        else{
            parser::production_rule pr = parse_table(curr_state.symbol, curr_token.symbol, lexer_ptr);
            if(pr == nullptr){
                parser::parse_error err = error_table(curr_state.symbol, curr_token.symbol);
                (this->*err)(curr_token.line);

                panic_mode_recovery(lexer_ptr, &curr_token, &curr_state);
            }
            else{
                (this->*pr)(curr_state.parent, &curr_token); // execute fetched production rule
                curr_state = state_stack.top(); state_stack.pop();
            }
        }
    }
}

// useful explanation at https://www.cs.clemson.edu/course/cpsc827/material/LLk/LL%20Error%20Recovery.pdf
void parser::panic_mode_recovery(lexer* lexer_ptr, lexer::Token* curr_token, State* curr_state){
    err_count++;

    while(curr_state->symbol != grammarDFA::T_SEMICOLON && curr_state->symbol != grammarDFA::T_EOF){
        *curr_state = state_stack.top(); state_stack.pop();
    }

    if(curr_state->symbol == grammarDFA::T_EOF){ // recovery failed, reached end of stack
        //set curr_token to T_EOF to signal termination of parsing to parser
        curr_token->symbol = grammarDFA::T_EOF;
        curr_token->lexeme = '\0';
        curr_token->line = -1;
    }
    else{
        while(curr_token->symbol != curr_state->symbol && curr_token->symbol != grammarDFA::T_EOF) {
            if(!(*lexer_ptr).getNextToken(curr_token)){
                throw std::runtime_error("Lexer encountered an error while fetching the next token...exiting..."); // how to report useful information on the error?
            }
        }

        if(curr_token->symbol == grammarDFA::T_EOF){ // recovery failed, reached end of source file
            //set curr_state.symbol to T_EOF to signal termination of parsing to parser, and empty state_stack
            curr_state->symbol = grammarDFA::T_EOF;

            while(!state_stack.empty()){
                state_stack.pop();
            }
        }
    }
}

void parser::genericError(unsigned int line){
    std::cerr << "ln " << line << ": (generic) syntax error detected while parsing" << std::endl;
}

void parser::rulePROGRAM(astInnerNode* parent, lexer::Token* token_ptr){
    state_stack.push({.parent = parent, .symbol = grammarDFA::PROGRAM});
    state_stack.push({.parent = parent, .symbol = grammarDFA::STATEMENT});
}

void parser::ruleBLOCK(astInnerNode* parent, lexer::Token* token_ptr){
    auto* ast_block = new astBLOCK(parent, token_ptr->line); parent->add_child(ast_block);
    state_stack.push({.parent = nullptr, .symbol = grammarDFA::T_RBRACE});
    state_stack.push({.parent = ast_block, .symbol = grammarDFA::BLOCK_ext});
    state_stack.push({.parent = nullptr, .symbol = grammarDFA::T_LBRACE});
}

void parser::ruleBLOCK_ext(astInnerNode* parent, lexer::Token* token_ptr){
    state_stack.push({.parent = parent, .symbol = grammarDFA::BLOCK_ext});
    state_stack.push({.parent = parent, .symbol = grammarDFA::STATEMENT});
}

void parser::ruleVAR_DECL(astInnerNode* parent, lexer::Token* token_ptr){
    auto* ast_var_decl = new astVAR_DECL(parent, token_ptr->line); parent->add_child(ast_var_decl);
    state_stack.push({.parent = ast_var_decl, .symbol = grammarDFA::EXPRESSION});
    state_stack.push({.parent = nullptr, .symbol = grammarDFA::T_EQUALS});
    state_stack.push({.parent = ast_var_decl, .symbol = grammarDFA::TYPE});
    state_stack.push({.parent = nullptr, .symbol = grammarDFA::T_COLON});
    state_stack.push({.parent = ast_var_decl, .symbol = grammarDFA::IDENTIFIER});
    state_stack.push({.parent = nullptr, .symbol = grammarDFA::T_LET});
}

void parser::ruleASSIGNMENT(astInnerNode* parent, lexer::Token* token_ptr){
    auto* ast_assignment = new astASSIGNMENT(parent, token_ptr->line); parent->add_child(ast_assignment);
    state_stack.push({.parent = ast_assignment, .symbol = grammarDFA::EXPRESSION});
    state_stack.push({.parent = nullptr, .symbol = grammarDFA::T_EQUALS});
    state_stack.push({.parent = ast_assignment, .symbol = grammarDFA::IDENTIFIER});
}

void parser::rulePRINT(astInnerNode* parent, lexer::Token* token_ptr){
    auto* ast_print = new astPRINT(parent, token_ptr->line);  parent->add_child(ast_print);
    state_stack.push({.parent = ast_print, .symbol = grammarDFA::EXPRESSION});
    state_stack.push({.parent = nullptr, .symbol = grammarDFA::T_PRINT});
}

void parser::ruleRETURN(astInnerNode* parent, lexer::Token* token_ptr){
    auto* ast_return = new astRETURN(parent, token_ptr->line); parent->add_child(ast_return);
    state_stack.push({.parent = ast_return, .symbol = grammarDFA::EXPRESSION});
    state_stack.push({.parent = nullptr, .symbol = grammarDFA::T_RETURN});
}

void parser::ruleWHILE(astInnerNode* parent, lexer::Token* token_ptr){
    auto* ast_while = new astWHILE(parent, token_ptr->line); parent->add_child(ast_while);
    state_stack.push({.parent = ast_while, .symbol = grammarDFA::BLOCK});
    state_stack.push({.parent = nullptr, .symbol = grammarDFA::T_RBRACKET});
    state_stack.push({.parent = ast_while, .symbol = grammarDFA::EXPRESSION});
    state_stack.push({.parent = nullptr, .symbol = grammarDFA::T_LBRACKET});
    state_stack.push({.parent = nullptr, .symbol = grammarDFA::T_WHILE});
}

void parser::ruleFPARAM(astInnerNode* parent, lexer::Token* token_ptr){
    auto* ast_fparam = new astFPARAM(parent, token_ptr->line); parent->add_child(ast_fparam);
    state_stack.push({.parent = ast_fparam, .symbol = grammarDFA::TYPE});
    state_stack.push({.parent = nullptr, .symbol = grammarDFA::T_COLON});
    state_stack.push({.parent = ast_fparam, .symbol = grammarDFA::IDENTIFIER});
}

void parser::ruleFPARAMS(astInnerNode* parent, lexer::Token* token_ptr){
    auto* ast_fparams = new astFPARAMS(parent, token_ptr->line); parent->add_child(ast_fparams);
    state_stack.push({.parent = ast_fparams, .symbol = grammarDFA::FPARAMS_ext});
    state_stack.push({.parent = ast_fparams, .symbol = grammarDFA::FPARAM});
}

void parser::ruleFPARAMS_ext_T_COMMA(astInnerNode* parent, lexer::Token* token_ptr){
    state_stack.push({.parent = parent, .symbol = grammarDFA::FPARAMS_ext});
    state_stack.push({.parent = parent, .symbol = grammarDFA::FPARAM});
    state_stack.push({.parent = nullptr, .symbol = grammarDFA::T_COMMA});
}

void parser::ruleAPARAMS(astInnerNode* parent, lexer::Token* token_ptr){
    auto* ast_aparams = new astAPARAMS(parent, token_ptr->line); parent->add_child(ast_aparams);
    state_stack.push({.parent = ast_aparams, .symbol = grammarDFA::APARAMS_ext});
    state_stack.push({.parent = ast_aparams, .symbol = grammarDFA::EXPRESSION});
}

void parser::ruleAPARAMS_ext_T_COMMA(astInnerNode* parent, lexer::Token* token_ptr){
    state_stack.push({.parent = parent, .symbol = grammarDFA::APARAMS_ext});
    state_stack.push({.parent = parent, .symbol = grammarDFA::EXPRESSION});
    state_stack.push({.parent = nullptr, .symbol = grammarDFA::T_COMMA});
}

void parser::ruleSUBEXPR(astInnerNode* parent, lexer::Token* token_ptr){
    auto* ast_subexpr = new astSUBEXPR(parent, token_ptr->line); parent->add_child(ast_subexpr);
    state_stack.push({.parent = nullptr, .symbol = grammarDFA::T_RBRACKET});
    state_stack.push({.parent = ast_subexpr, .symbol = grammarDFA::EXPRESSION});
    state_stack.push({.parent = nullptr, .symbol = grammarDFA::T_LBRACKET});
}

void parser::ruleLITERAL_T_BOOL(astInnerNode* parent, lexer::Token* token_ptr){
    parent->add_child(new astLITERAL(parent, token_ptr->lexeme, grammarDFA::T_BOOL, token_ptr->line));
    state_stack.push({.parent = nullptr, .symbol = grammarDFA::T_BOOL});
}

void parser::ruleLITERAL_T_INT(astInnerNode* parent, lexer::Token* token_ptr){
    parent->add_child(new astLITERAL(parent, token_ptr->lexeme, grammarDFA::T_INT, token_ptr->line));
    state_stack.push({.parent = nullptr, .symbol = grammarDFA::T_INT});
}

void parser::ruleLITERAL_T_FLOAT(astInnerNode* parent, lexer::Token* token_ptr){
    parent->add_child(new astLITERAL(parent, token_ptr->lexeme, grammarDFA::T_FLOAT, token_ptr->line));
    state_stack.push({.parent = nullptr, .symbol = grammarDFA::T_FLOAT});
}

void parser::ruleLITERAL_T_STRING(astInnerNode* parent, lexer::Token* token_ptr){
    parent->add_child(new astLITERAL(parent, token_ptr->lexeme, grammarDFA::T_STRING, token_ptr->line));
    state_stack.push({.parent = nullptr, .symbol = grammarDFA::T_STRING});
}

void parser::ruleLITERAL_T_CHAR(astInnerNode* parent, lexer::Token* token_ptr){
    parent->add_child(new astLITERAL(parent, token_ptr->lexeme, grammarDFA::T_CHAR, token_ptr->line));
    state_stack.push({.parent = nullptr, .symbol = grammarDFA::T_CHAR});
}

void parser::ruleUNARY_T_MINUS(astInnerNode* parent, lexer::Token* token_ptr){
    auto* ast_unary = new astUNARY(parent, "-", token_ptr->line); parent->add_child(ast_unary);
    state_stack.push({.parent = ast_unary, .symbol = grammarDFA::EXPRESSION});
    state_stack.push({.parent = nullptr, .symbol = grammarDFA::T_MINUS});
}

void parser::ruleUNARY_T_NOT(astInnerNode* parent, lexer::Token* token_ptr){
    auto* ast_unary = new astUNARY(parent, "not", token_ptr->line); parent->add_child(ast_unary);
    state_stack.push({.parent = ast_unary, .symbol = grammarDFA::EXPRESSION});
    state_stack.push({.parent = nullptr, .symbol = grammarDFA::T_NOT});
}

void parser::ruleFUNC_CALL(astInnerNode* parent, lexer::Token* token_ptr){
    auto* ast_func_call = new astFUNC_CALL(parent, token_ptr->line); parent->add_child(ast_func_call);
    state_stack.push({.parent = nullptr, .symbol = grammarDFA::T_RBRACKET});
    state_stack.push({.parent = ast_func_call, .symbol = grammarDFA::FUNC_CALL_APARAMS});
    state_stack.push({.parent = nullptr, .symbol = grammarDFA::T_LBRACKET});
    state_stack.push({.parent = ast_func_call, .symbol = grammarDFA::IDENTIFIER});
}

void parser::ruleFUNC_CALL_APARAMS(astInnerNode* parent, lexer::Token* token_ptr){
    state_stack.push({.parent = parent, .symbol = grammarDFA::APARAMS});
}

void parser::ruleIF(astInnerNode* parent, lexer::Token* token_ptr){
    auto* ast_if = new astIF(parent, token_ptr->line); parent->add_child(ast_if);
    state_stack.push({.parent = ast_if, .symbol = grammarDFA::ELSE});
    state_stack.push({.parent = ast_if, .symbol = grammarDFA::BLOCK});
    state_stack.push({.parent = nullptr, .symbol = grammarDFA::T_RBRACKET});
    state_stack.push({.parent = ast_if, .symbol = grammarDFA::EXPRESSION});
    state_stack.push({.parent = nullptr, .symbol = grammarDFA::T_LBRACKET});
    state_stack.push({.parent = nullptr, .symbol = grammarDFA::T_IF});
}

void parser::ruleELSE_T_ELSE(astInnerNode* parent, lexer::Token* token_ptr){
    state_stack.push({.parent = parent, .symbol = grammarDFA::BLOCK});
    state_stack.push({.parent = nullptr, .symbol = grammarDFA::T_ELSE});
}

void parser::ruleFUNC_DECL(astInnerNode* parent, lexer::Token* token_ptr){
    auto* ast_func_decl = new astFUNC_DECL(parent, token_ptr->line); parent->add_child(ast_func_decl);
    state_stack.push({.parent = ast_func_decl, .symbol = grammarDFA::BLOCK});
    state_stack.push({.parent = nullptr, .symbol = grammarDFA::T_RBRACKET});
    state_stack.push({.parent = ast_func_decl, .symbol = grammarDFA::FUNC_DECL_FPARAMS});
    state_stack.push({.parent = nullptr, .symbol = grammarDFA::T_LBRACKET});
    state_stack.push({.parent = ast_func_decl, .symbol = grammarDFA::IDENTIFIER});
    state_stack.push({.parent = ast_func_decl, .symbol = grammarDFA::TYPE});
}

void parser::ruleFUNC_DECL_FPARAMS(astInnerNode* parent, lexer::Token* token_ptr){
    state_stack.push({.parent = parent, .symbol = grammarDFA::FPARAMS});
}

void parser::ruleFOR(astInnerNode* parent, lexer::Token* token_ptr){
    auto* ast_for = new astFOR(parent, token_ptr->line); parent->add_child(ast_for);
    state_stack.push({.parent = ast_for, .symbol = grammarDFA::BLOCK});
    state_stack.push({.parent = nullptr, .symbol = grammarDFA::T_RBRACKET});
    state_stack.push({.parent = ast_for, .symbol = grammarDFA::FOR_ASSIGNMENT});
    state_stack.push({.parent = nullptr, .symbol = grammarDFA::T_SEMICOLON});
    state_stack.push({.parent = ast_for, .symbol = grammarDFA::FOR_EXPRESSION});
    state_stack.push({.parent = nullptr, .symbol = grammarDFA::T_SEMICOLON});
    state_stack.push({.parent = ast_for, .symbol = grammarDFA::FOR_VAR_DECL});
    state_stack.push({.parent = nullptr, .symbol = grammarDFA::T_LBRACKET});
    state_stack.push({.parent = nullptr, .symbol = grammarDFA::T_FOR});
}

void parser::ruleFOR_ASSIGNMENT(astInnerNode* parent, lexer::Token* token_ptr){
    state_stack.push({.parent = parent, .symbol = grammarDFA::ASSIGNMENT});
}

void parser::ruleFOR_EXPRESSION(astInnerNode* parent, lexer::Token* token_ptr){
    state_stack.push({.parent = parent, .symbol = grammarDFA::EXPRESSION});
}

void parser::ruleFOR_VAR_DECL(astInnerNode* parent, lexer::Token* token_ptr){
    state_stack.push({.parent = parent, .symbol = grammarDFA::VAR_DECL});
}

void parser::ruleSTATEMENT_T_LET(astInnerNode* parent, lexer::Token* token_ptr){
    state_stack.push({.parent = nullptr, .symbol = grammarDFA::T_SEMICOLON});
    state_stack.push({.parent = parent, .symbol = grammarDFA::VAR_DECL});
}

void parser::ruleSTATEMENT_T_IDENTIFIER(astInnerNode* parent, lexer::Token* token_ptr){
    state_stack.push({.parent = nullptr, .symbol = grammarDFA::T_SEMICOLON});
    state_stack.push({.parent = parent, .symbol = grammarDFA::ASSIGNMENT});
}

void parser::ruleSTATEMENT_T_PRINT(astInnerNode* parent, lexer::Token* token_ptr){
    state_stack.push({.parent = nullptr, .symbol = grammarDFA::T_SEMICOLON});
    state_stack.push({.parent = parent, .symbol = grammarDFA::PRINT});
}

void parser::ruleSTATEMENT_T_IF(astInnerNode* parent, lexer::Token* token_ptr){
    state_stack.push({.parent = parent, .symbol = grammarDFA::IF});
}

void parser::ruleSTATEMENT_T_FOR(astInnerNode* parent, lexer::Token* token_ptr){
    state_stack.push({.parent = parent, .symbol = grammarDFA::FOR});
}

void parser::ruleSTATEMENT_T_WHILE(astInnerNode* parent, lexer::Token* token_ptr){
    state_stack.push({.parent = parent, .symbol = grammarDFA::WHILE});
}

void parser::ruleSTATEMENT_T_RETURN(astInnerNode* parent, lexer::Token* token_ptr){
    state_stack.push({.parent = nullptr, .symbol = grammarDFA::T_SEMICOLON});
    state_stack.push({.parent = parent, .symbol = grammarDFA::RETURN});
}

void parser::ruleSTATEMENT_T_TYPE(astInnerNode* parent, lexer::Token* token_ptr){
    state_stack.push({.parent = parent, .symbol = grammarDFA::FUNC_DECL});
}

void parser::ruleSTATEMENT_T_LBRACE(astInnerNode* parent, lexer::Token* token_ptr){
    state_stack.push({.parent = parent, .symbol = grammarDFA::BLOCK});
}

void parser::ruleFACTOR_LITERAL(astInnerNode* parent, lexer::Token* token_ptr){
    state_stack.push({.parent = parent, .symbol = grammarDFA::LITERAL});
}

void parser::ruleFACTOR_IDENTIFIER(astInnerNode* parent, lexer::Token* token_ptr){
    state_stack.push({.parent = parent, .symbol = grammarDFA::IDENTIFIER});
}

void parser::ruleFACTOR_FUNC_CALL(astInnerNode* parent, lexer::Token* token_ptr){
    state_stack.push({.parent = parent, .symbol = grammarDFA::FUNC_CALL});
}

void parser::ruleFACTOR_SUBEXPR(astInnerNode* parent, lexer::Token* token_ptr){
    state_stack.push({.parent = parent, .symbol = grammarDFA::SUBEXPR});
}

void parser::ruleFACTOR_UNARY(astInnerNode* parent, lexer::Token* token_ptr){
    state_stack.push({.parent = parent, .symbol = grammarDFA::UNARY});
}

void parser::ruleTERM(astInnerNode* parent, lexer::Token* token_ptr){
    state_stack.push({.parent = parent, .symbol = grammarDFA::TERM_ext});
    state_stack.push({.parent = parent, .symbol = grammarDFA::FACTOR});
}

void parser::ruleTERM_ext(astInnerNode* parent, lexer::Token* token_ptr){
    auto* ast_multop = new astMULTOP(parent, token_ptr->lexeme, token_ptr->line);
    ast_multop->add_child(parent->children->at(parent->n_children - 1));
    ast_multop->children->at(ast_multop->n_children - 1)->parent = ast_multop;
    parent->n_children--;
    parent->add_child(ast_multop);

    state_stack.push({.parent = ast_multop, .symbol = grammarDFA::TERM_ext});
    state_stack.push({.parent = ast_multop, .symbol = grammarDFA::FACTOR});
    state_stack.push({.parent = nullptr, .symbol = token_ptr->symbol});
}

void parser::ruleS_EXPR(astInnerNode* parent, lexer::Token* token_ptr){
    state_stack.push({.parent = parent, .symbol = grammarDFA::S_EXPR_ext});
    state_stack.push({.parent = parent, .symbol = grammarDFA::TERM});
}

void parser::ruleS_EXPR_ext(astInnerNode* parent, lexer::Token* token_ptr){
    auto* ast_addop = new astADDOP(parent, token_ptr->lexeme, token_ptr->line);
    ast_addop->add_child(parent->children->at(parent->n_children - 1));
    ast_addop->children->at(ast_addop->n_children - 1)->parent = ast_addop;
    parent->n_children--;
    parent->add_child(ast_addop);

    state_stack.push({.parent = ast_addop, .symbol = grammarDFA::S_EXPR_ext});
    state_stack.push({.parent = ast_addop, .symbol = grammarDFA::TERM});
    state_stack.push({.parent = nullptr, .symbol = token_ptr->symbol});
}

void parser::ruleEXPRESSION(astInnerNode* parent, lexer::Token* token_ptr){
    state_stack.push({.parent = parent, .symbol = grammarDFA::EXPRESSION_ext});
    state_stack.push({.parent = parent, .symbol = grammarDFA::S_EXPR});
}

void parser::ruleEXPRESSION_ext(astInnerNode* parent, lexer::Token* token_ptr){
    auto* ast_relop = new astRELOP(parent, token_ptr->lexeme, token_ptr->line);
    ast_relop->add_child(parent->children->at(parent->n_children - 1));
    ast_relop->children->at(ast_relop->n_children - 1)->parent = ast_relop;
    parent->n_children--;
    parent->add_child(ast_relop);

    state_stack.push({.parent = ast_relop, .symbol = grammarDFA::EXPRESSION_ext});
    state_stack.push({.parent = ast_relop, .symbol = grammarDFA::S_EXPR});
    state_stack.push({.parent = nullptr, .symbol = token_ptr->symbol});
}

void parser::ruleTYPE(astInnerNode* parent, lexer::Token* token_ptr){
    grammarDFA::Symbol type;
    if(token_ptr->lexeme == "bool"){
        type = grammarDFA::T_BOOL;
    }
    else if(token_ptr->lexeme == "int"){
        type = grammarDFA::T_INT;
    }
    else if(token_ptr->lexeme == "float"){
        type = grammarDFA::T_FLOAT;
    }
    else if(token_ptr->lexeme == "string"){
        type = grammarDFA::T_STRING;
    }
    else if(token_ptr->lexeme == "char"){
        type = grammarDFA::T_CHAR;
    }
    else{
        type = grammarDFA::T_AUTO;
    }

    parent->add_child(new astTYPE(parent, token_ptr->lexeme, type, token_ptr->line));
    state_stack.push({.parent = nullptr, .symbol = grammarDFA::T_TYPE});
}

void parser::ruleIDENTIFIER(astInnerNode* parent, lexer::Token* token_ptr){
    parent->add_child(new astIDENTIFIER(parent, token_ptr->lexeme, token_ptr->line));
    state_stack.push({.parent = nullptr, .symbol = grammarDFA::T_IDENTIFIER});
}

void parser::null_rule(astInnerNode* parent, lexer::Token* token_ptr){}

void parser::optional_pass_rule(astInnerNode* parent, lexer::Token* token_ptr){
    parent->add_child(nullptr); // preserves ordering of positional optional symbols
}

parser::production_rule parser::parse_table(int curr_symbol, int curr_tok_symbol, lexer* lexer_ptr){
    production_rule pr;

    switch(curr_symbol){
        case grammarDFA::PROGRAM:{
            if(curr_tok_symbol == grammarDFA::T_EOF){
                                                pr = &parser::null_rule;
            }else{
                                                pr = &parser::rulePROGRAM;
            }
        }                                                                                       break;
        case grammarDFA::BLOCK:                 pr = &parser::ruleBLOCK;                        break;
        case grammarDFA::BLOCK_ext:{
            if(curr_tok_symbol == grammarDFA::T_RBRACE){
                                                pr = &parser::null_rule;
            }else{
                                                pr = &parser::ruleBLOCK_ext;
            }
        }                                                                                       break;
        case grammarDFA::VAR_DECL:              pr = &parser::ruleVAR_DECL;                     break;
        case grammarDFA::ASSIGNMENT:            pr = &parser::ruleASSIGNMENT;                   break;
        case grammarDFA::PRINT:                 pr = &parser::rulePRINT;                        break;
        case grammarDFA::RETURN:                pr = &parser::ruleRETURN;                       break;
        case grammarDFA::WHILE:                 pr = &parser::ruleWHILE;                        break;
        case grammarDFA::FPARAM:                pr = &parser::ruleFPARAM;                       break;
        case grammarDFA::FPARAMS:               pr = &parser::ruleFPARAMS;                      break;
        case grammarDFA::FPARAMS_ext: {
            if(curr_tok_symbol == grammarDFA::T_COMMA){
                                                pr = &parser::ruleFPARAMS_ext_T_COMMA;
            }else{
                                                pr = &parser::null_rule;
            }
        }                                                                                       break;
        case grammarDFA::APARAMS:               pr = &parser::ruleAPARAMS;                      break;
        case grammarDFA::APARAMS_ext: {
            if(curr_tok_symbol == grammarDFA::T_COMMA){
                                                pr = &parser::ruleAPARAMS_ext_T_COMMA;
            }else{
                                                pr = &parser::null_rule;
            }
        }                                                                                       break;
        case grammarDFA::SUBEXPR:               pr = &parser::ruleSUBEXPR;                      break;
        case grammarDFA::LITERAL: {
            switch(curr_tok_symbol){
                case grammarDFA::T_INT:         pr = &parser::ruleLITERAL_T_INT;                break;
                case grammarDFA::T_BOOL:        pr = &parser::ruleLITERAL_T_BOOL;               break;
                case grammarDFA::T_STRING:      pr = &parser::ruleLITERAL_T_STRING;             break;
                case grammarDFA::T_FLOAT:       pr = &parser::ruleLITERAL_T_FLOAT;              break;
                case grammarDFA::T_CHAR:        pr = &parser::ruleLITERAL_T_CHAR;               break;
                default:                        pr = nullptr;
            }
        }                                                                                       break;
        case grammarDFA::UNARY: {
            switch(curr_tok_symbol){
                case grammarDFA::T_MINUS:       pr = &parser::ruleUNARY_T_MINUS;                break;
                case grammarDFA::T_NOT:         pr = &parser::ruleUNARY_T_NOT;                  break;
                default:                        pr = nullptr;
            }
        }                                                                                       break;
        case grammarDFA::FUNC_CALL:             pr = &parser::ruleFUNC_CALL;                    break;
        case grammarDFA::FUNC_CALL_APARAMS: {
            if(curr_tok_symbol == grammarDFA::T_RBRACKET){
                                                pr = &parser::optional_pass_rule;
            }else{
                                                pr = &parser::ruleFUNC_CALL_APARAMS;
            }
        }                                                                                       break;
        case grammarDFA::IF:                    pr = &parser::ruleIF;                           break;
        case grammarDFA::ELSE: {
            if(curr_tok_symbol == grammarDFA::T_ELSE){
                                                pr = &parser::ruleELSE_T_ELSE;
            }else{
                                                pr = &parser::optional_pass_rule;
            }
        }                                                                                       break;
        case grammarDFA::FUNC_DECL:             pr = &parser::ruleFUNC_DECL;                    break;
        case grammarDFA::FUNC_DECL_FPARAMS: {
            if(curr_tok_symbol == grammarDFA::T_RBRACKET){
                                                pr = &parser::optional_pass_rule;
            }else{
                                                pr = &parser::ruleFUNC_DECL_FPARAMS;
            }
        }                                                                                       break;
        case grammarDFA::FOR:                   pr = &parser::ruleFOR;                          break;
        case grammarDFA::FOR_VAR_DECL: {
            if(curr_tok_symbol == grammarDFA::T_SEMICOLON){
                                                pr = &parser::optional_pass_rule;
            }else{
                                                pr = &parser::ruleFOR_VAR_DECL;
            }
        }                                                                                       break;
        case grammarDFA::FOR_EXPRESSION: {
            if(curr_tok_symbol == grammarDFA::T_SEMICOLON){
                                                pr = nullptr;
            }else{
                                                pr = &parser::ruleFOR_EXPRESSION;
            }
        }                                                                                       break;
        case grammarDFA::FOR_ASSIGNMENT: {
            if(curr_tok_symbol == grammarDFA::T_RBRACKET){
                                                pr = &parser::optional_pass_rule;
            }else{
                                                pr = &parser::ruleFOR_ASSIGNMENT;
            }
        }                                                                                       break;
        case grammarDFA::STATEMENT: {
            switch(curr_tok_symbol){
                case grammarDFA::T_LET:         pr = &parser::ruleSTATEMENT_T_LET;              break;
                case grammarDFA::T_IDENTIFIER:  pr = &parser::ruleSTATEMENT_T_IDENTIFIER;       break;
                case grammarDFA::T_PRINT:       pr = &parser::ruleSTATEMENT_T_PRINT;            break;
                case grammarDFA::T_IF:          pr = &parser::ruleSTATEMENT_T_IF;               break;
                case grammarDFA::T_FOR:         pr = &parser::ruleSTATEMENT_T_FOR;              break;
                case grammarDFA::T_WHILE:       pr = &parser::ruleSTATEMENT_T_WHILE;            break;
                case grammarDFA::T_RETURN:      pr = &parser::ruleSTATEMENT_T_RETURN;           break;
                case grammarDFA::T_TYPE:        pr = &parser::ruleSTATEMENT_T_TYPE;             break;
                case grammarDFA::T_LBRACE:      pr = &parser::ruleSTATEMENT_T_LBRACE;           break;
                default:                        pr = nullptr;
            }
        }                                                                                       break;
        case grammarDFA::FACTOR: {
            switch(curr_tok_symbol){
                case grammarDFA::T_BOOL:
                case grammarDFA::T_INT:
                case grammarDFA::T_FLOAT:
                case grammarDFA::T_STRING:
                case grammarDFA::T_CHAR:        pr = &parser::ruleFACTOR_LITERAL;               break;
                case grammarDFA::T_LBRACKET:    pr = &parser::ruleFACTOR_SUBEXPR;               break;
                case grammarDFA::T_MINUS:
                case grammarDFA::T_NOT:         pr = &parser::ruleFACTOR_UNARY;                 break;
                case grammarDFA::T_IDENTIFIER: {
                    lexer::Token peek_token;
                    if(!(*lexer_ptr).peekTokens(&peek_token, 1)){
                        throw std::runtime_error("Lexer encountered an error while peeking tokens...exiting...");
                    }

                    if(peek_token.symbol == grammarDFA::T_LBRACKET){
                                                pr = &parser::ruleFACTOR_FUNC_CALL;
                    }else{
                                                pr = &parser::ruleFACTOR_IDENTIFIER;
                    }
                }                                                                               break;
                default:                        pr = nullptr;
            }
        }                                                                                       break;
        case grammarDFA::TERM:                  pr = &parser::ruleTERM;                         break;
        case grammarDFA::TERM_ext: {
            if(curr_tok_symbol == grammarDFA::T_MUL || curr_tok_symbol == grammarDFA::T_DIV ||
            curr_tok_symbol == grammarDFA::T_AND){
                                                pr = &parser::ruleTERM_ext;
            }else{
                                                pr = &parser::null_rule;
            }
        }                                                                                       break;
        case grammarDFA::S_EXPR:                pr = &parser::ruleS_EXPR;                       break;
        case grammarDFA::S_EXPR_ext: {
            if(curr_tok_symbol == grammarDFA::T_PLUS || curr_tok_symbol == grammarDFA::T_MINUS ||
               curr_tok_symbol == grammarDFA::T_OR){
                                                pr = &parser::ruleS_EXPR_ext;
            }else{
                                                pr = &parser::null_rule;
            }
        }                                                                                       break;
        case grammarDFA::EXPRESSION:            pr = &parser::ruleEXPRESSION;                   break;
        case grammarDFA::EXPRESSION_ext: {
            if(curr_tok_symbol == grammarDFA::T_RELOP){
                                                pr = &parser::ruleEXPRESSION_ext;
            }else{
                                                pr = &parser::null_rule;
            }
        }                                                                                       break;
        case grammarDFA::TYPE:                  pr = &parser::ruleTYPE;                         break;
        case grammarDFA::IDENTIFIER:            pr = &parser::ruleIDENTIFIER;                   break;
        default:                                pr = nullptr;
    }

    return pr;
}

parser::parse_error parser::error_table(int curr_symbol, int curr_token){
    return &parser::genericError;
}
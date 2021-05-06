//
// Created by Xandru Mifsud on 16/03/2021.
//

#ifndef CPS2000_GRAMMARDFA_H
#define CPS2000_GRAMMARDFA_H

#include <string>
using namespace std;

class grammarDFA{
public:
    enum State{
        S0, S1, S2, S3, S4, S5, S6, S7, S8, S9, S10, S11, S12, S13, S14, S15, S16, S17, S18, S19, S20, S21, S22, S23,
        S24, S25, S26, S27, S28, S29, S30, S31, S32, S33, S34, S_E
    };

    enum TransitionType{
        LETTER, ESC, ZERO, DIGIT, PRINTABLE, UNDERSCORE, PERIOD, COLON, SEMICOLON, DOUBLEQUOTES, AST, FSLASH, BSLASH,
        LBRACKET, RBRACKET, LBRACE, RBRACE, EQUALS, LRANGLE, EXCL, PLUS, MINUS, NEWLINE, COMMA, EOSTR, APOST, LSQUARE,
        RSQUARE
    };

    enum Symbol{
        PROGRAM, STATEMENT, BLOCK, BLOCK_ext, EXPRESSION, ASSIGNMENT, PRINT, RETURN, WHILE, FPARAM, FPARAMS,
        FPARAMS_ext, APARAMS, APARAMS_ext, SUBEXPR, LITERAL, UNARY, FUNC_CALL, FUNC_CALL_APARAMS, IF, ELSE, FUNC_DECL,
        FUNC_DECL_FPARAMS, FOR, FOR_DECL, FOR_EXPRESSION, FOR_ASSIGNMENT, FACTOR, TERM, TERM_ext, S_EXPR, S_EXPR_ext,
        EXPRESSION_ext, TYPE_VAR, TYPE_ARR, IDENTIFIER, DECL, VAR_DECL_ASSIGNMENT, ARR_DECL_ASSIGNMENT,
        ARR_DECL_ASSIGNMENT_ext, ELEMENT, FPARAM_TYPE, TLS_DECL, MEMBER, MEMBER_ACCESS, SINGLETON, ARRAY, FUNCTION,
        STRUCT, // Non--Terminal Symbols, n_NTS = 49

        T_INT, T_FLOAT, T_STRING, T_CHAR, T_AUTO, T_MUL, T_DIV, T_PLUS, T_MINUS, T_EQUALS, T_RELOP, // Terminal Symbols, n_token_types = 38
        T_LBRACKET, T_RBRACKET, T_LBRACE, T_RBRACE, T_PERIOD, T_COLON, T_SEMICOLON, T_COMMENT, T_INVALID, T_COMMA, T_EOF,
        T_IDENTIFIER, T_AND, T_OR, T_NOT, T_BOOL, T_TYPE, T_LET, T_PRINT, T_RETURN, T_IF, T_ELSE, T_FOR, T_WHILE,
        T_LSQUARE, T_RSQUARE, T_TLSTRUCT
    };

    const static int n_NTS = 49, n_token_types = 38;

    Symbol state_tok(State, string*);
    State transition(State, char);
private:
    const static int n_states = 37, n_delta_types = 28;

    const State transition_table[n_states][n_delta_types] = {
   // LETTER| ESC|ZERO| DIGIT| PRINT.| '_'| '.'| ':'| ';'| '"'| '*'| '/'| '\'| '('| ')'| '{'| '}'| '='|'<''>'| '!'| '+'| '-'|'\n'| ','|'\0'| '''| '['| ']'|
     {    S1,  S1,  S2,    S2,    S_E,  S1, S24, S25, S26,  S5, S17, S12, S_E, S20, S21, S22, S23, S10,    S8,  S9, S18, S19, S_E, S27, S28, S29, S33, S34}, // S0  -> T_INVALID
     {    S1,  S1,  S1,    S1,    S_E,  S1, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E,   S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E}, // S1  -> T_IDENTIFIER
     {   S_E, S_E,  S2,    S2,    S_E, S_E,  S3, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E,   S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E}, // S2  -> T_INT
     {   S_E, S_E,  S4,    S4,    S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E,   S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E}, // S3  -> T_INVALID
     {   S_E, S_E,  S4,    S4,    S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E,   S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E}, // S4  -> T_FLOAT
     {    S5,  S5,  S5,    S5,     S5,  S5,  S5,  S5,  S5,  S7,  S5,  S5,  S6,  S5,  S5,  S5,  S5,  S5,    S5,  S5,  S5,  S5,  S5,  S5, S_E,  S5,  S5,  S5}, // S5  -> T_INVALID
     {    S5,  S5,  S5,    S5,     S5,  S5,  S5,  S5,  S5,  S5,  S5,  S5,  S5,  S5,  S5,  S5,  S5,  S5,    S5,  S5,  S5,  S5,  S5,  S5, S_E,  S5,  S5,  S5}, // S6  -> T_INVALID
     {   S_E, S_E, S_E,   S_E,    S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E,   S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E}, // S7  -> T_STRING
     {   S_E, S_E, S_E,   S_E,    S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S11,   S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E}, // S8  -> T_RELOP
     {   S_E, S_E, S_E,   S_E,    S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S11,   S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E}, // S9  -> T_INVALID
     {   S_E, S_E, S_E,   S_E,    S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S11,   S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E}, // S10 -> T_EQUALS
     {   S_E, S_E, S_E,   S_E,    S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E,   S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E}, // S11 -> T_RELOP
     {   S_E, S_E, S_E,   S_E,    S_E, S_E, S_E, S_E, S_E, S_E, S14, S13, S_E, S_E, S_E, S_E, S_E, S_E,   S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E}, // S12 -> T_DIV
     {   S13, S13, S13,   S13,    S13, S13, S13, S13, S13, S13, S13, S13, S13, S13, S13, S13, S13, S13,   S13, S13, S13, S13, S16, S13, S16, S13, S13, S13}, // S13 -> T_INVALID
     {   S14, S14, S14,   S14,    S14, S14, S14, S14, S14, S14, S15, S14, S14, S14, S14, S14, S14, S14,   S14, S14, S14, S14, S14, S14, S_E, S14, S14, S14}, // S14 -> T_INVALID
     {   S14, S14, S14,   S14,    S14, S14, S14, S14, S14, S14, S15, S16, S14, S14, S14, S14, S14, S14,   S14, S14, S14, S14, S14, S14, S_E, S14, S14, S14}, // S15 -> T_INVALID
     {   S_E, S_E, S_E,   S_E,    S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E,   S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E}, // S16 -> T_COMMENT
     {   S_E, S_E, S_E,   S_E,    S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E,   S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E}, // S17 -> T_MUL
     {   S_E, S_E, S_E,   S_E,    S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E,   S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E}, // S18 -> T_PLUS
     {   S_E, S_E, S_E,   S_E,    S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E,   S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E}, // S19 -> T_MINUS
     {   S_E, S_E, S_E,   S_E,    S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E,   S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E}, // S20 -> T_LBRACKET
     {   S_E, S_E, S_E,   S_E,    S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E,   S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E}, // S21 -> T_RBRACKET
     {   S_E, S_E, S_E,   S_E,    S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E,   S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E}, // S22 -> T_LBRACE
     {   S_E, S_E, S_E,   S_E,    S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E,   S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E}, // S23 -> T_RBRACE
     {   S_E, S_E, S_E,   S_E,    S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E,   S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E}, // S24 -> T_PERIOD
     {   S_E, S_E, S_E,   S_E,    S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E,   S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E}, // S25 -> T_COLON
     {   S_E, S_E, S_E,   S_E,    S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E,   S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E}, // S26 -> T_SEMICOLON
     {   S_E, S_E, S_E,   S_E,    S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E,   S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E}, // S27 -> T_COMMA
     {   S_E, S_E, S_E,   S_E,    S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E,   S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E}, // S28 -> T_EOF
     {   S31, S31, S31,   S31,    S31, S31, S31, S31, S31, S31, S31, S31, S30, S31, S31, S31, S31, S31,   S31, S31, S31, S31, S31, S31, S31, S31, S31, S31}, // S29 -> T_INVALID
     {   S_E, S31, S31,   S_E,    S_E, S_E, S_E, S_E, S_E, S31, S_E, S_E, S31, S_E, S_E, S_E, S_E, S_E,   S_E, S_E, S_E, S_E, S_E, S_E, S_E, S31, S_E, S_E}, // S30 -> T_INVALID
     {   S_E, S_E, S_E,   S_E,    S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E,   S_E, S_E, S_E, S_E, S_E, S_E, S_E, S32, S_E, S_E}, // S31 -> T_INVALID
     {   S_E, S_E, S_E,   S_E,    S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E,   S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E}, // S32 -> T_CHAR
     {   S_E, S_E, S_E,   S_E,    S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E,   S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E}, // S33 -> T_LSQUARE
     {   S_E, S_E, S_E,   S_E,    S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E,   S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E}, // S34 -> T_RSQUARE
     {   S_E, S_E, S_E,   S_E,    S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E,   S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E, S_E}  // S_E -> T_INVALID
    };

    const Symbol final_state_tokens[n_states] = {T_INVALID, T_IDENTIFIER, T_INT, T_INVALID, T_FLOAT, T_INVALID, T_INVALID,
                                                 T_STRING, T_RELOP, T_INVALID, T_EQUALS, T_RELOP, T_DIV, T_INVALID,
                                                 T_INVALID, T_INVALID, T_COMMENT, T_MUL, T_PLUS, T_MINUS, T_LBRACKET,
                                                 T_RBRACKET, T_LBRACE, T_RBRACE, T_PERIOD, T_COLON, T_SEMICOLON, T_COMMA,
                                                 T_EOF, T_INVALID, T_INVALID, T_INVALID, T_CHAR, T_LSQUARE, T_RSQUARE,
                                                 T_INVALID};

    static TransitionType get_transition_type(char);
};

#endif //CPS2000_GRAMMARDFA_H

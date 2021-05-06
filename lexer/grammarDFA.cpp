//
// Created by Xandru Mifsud on 16/03/2021.
//

#include "grammarDFA.h"

grammarDFA::Symbol grammarDFA::state_tok(State state, string* lexeme){
    Symbol symbol = final_state_tokens[state];

    if(symbol == T_IDENTIFIER){
        if(*lexeme == "and"){
            return T_AND;
        }
        else if(*lexeme == "or"){
            return T_OR;
        }
        else if(*lexeme == "not"){
            return T_NOT;
        }
        else if(*lexeme == "true" || *lexeme == "false"){
            return T_BOOL;
        }
        else if(*lexeme == "int" || *lexeme == "float" || *lexeme == "bool" || *lexeme == "string" || *lexeme == "char"
                || *lexeme == "auto"){
            return T_TYPE;
        }
        else if(*lexeme == "let"){
            return T_LET;
        }
        else if(*lexeme == "print"){
            return T_PRINT;
        }
        else if(*lexeme == "return"){
            return T_RETURN;
        }
        else if(*lexeme == "if"){
            return T_IF;
        }
        else if(*lexeme == "else"){
            return T_ELSE;
        }
        else if(*lexeme == "for"){
            return T_FOR;
        }
        else if(*lexeme == "while"){
            return T_WHILE;
        }
        else if(*lexeme == "tlstruct"){
            return T_TLSTRUCT;
        }
        else{
            return symbol;
        }
    }
    else{
        return symbol;
    }
}

grammarDFA::State grammarDFA::transition(State state, char c){
    return transition_table[state][get_transition_type(c)];
}

grammarDFA::TransitionType grammarDFA::get_transition_type(char c){
    if(c == 'n' || c == 't' || c == 'r' || c == 'b' || c == 'f' || c == 'v'){
        return ESC;
    }
    else if((0x41 <= c && c <= 0x5A) || (0x61 <= c && c <= 0x7A)){
        return LETTER;
    }
    else if(c == 0x30 ){
        return ZERO;
    }
    else if(0x31 <= c && c <= 0x39){
        return DIGIT;
    }
    else if(c == '_'){
        return UNDERSCORE;
    }
    else if(c == '.'){
        return PERIOD;
    }
    else if(c == ':'){
        return COLON;
    }
    else if(c == ';'){
        return SEMICOLON;
    }
    else if(c == '"'){
        return DOUBLEQUOTES;
    }
    else if(c == '*'){
        return AST;
    }
    else if(c == '/'){
        return FSLASH;
    }
    else if(c == '\\'){
        return BSLASH;
    }
    else if(c == '('){
        return LBRACKET;
    }
    else if(c == ')'){
        return RBRACKET;
    }
    else if(c == '['){
        return LSQUARE;
    }
    else if(c == ']'){
        return RSQUARE;
    }
    else if(c == '{'){
        return LBRACE;
    }
    else if(c == '}'){
        return RBRACE;
    }
    else if(c == '='){
        return EQUALS;
    }
    else if(c == '<' || c == '>'){
        return LRANGLE;
    }
    else if(c == '!'){
        return EXCL;
    }
    else if(c == '+'){
        return PLUS;
    }
    else if(c == '-'){
        return MINUS;
    }
    else if(c == '\n'){
        return NEWLINE;
    }
    else if(c == ','){
        return COMMA;
    }
    else if(c == '\0'){
        return EOSTR;
    }
    else if(c == '\''){
        return APOST;
    }
    else {
        return PRINTABLE;
    }
}
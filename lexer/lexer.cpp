//
// Created by Xandru Mifsud on 16/03/2021.
//

#include "lexer.h"

#include <utility>

lexer::lexer(string input_source){
    source = std::move(input_source);
    index = 0;
    line = 1;
}

bool lexer::getNextToken(Token* token_ptr){
    Token token;
    token.line = 0;
    grammarDFA::State state = grammarDFA::S0;
    auto* dfa = new grammarDFA();

    states_stack_clear();

    while(isspace(source[index])){
        if(source[index] == '\n'){
            line++;
        }
        index++;
    }

    while(state != grammarDFA::S_E){
        token.lexeme += source[index];

        if(dfa->state_tok(state, &token.lexeme) != grammarDFA::T_INVALID){
            states_stack_clear();
        }

        states_stack.push(state);
        state = dfa->transition(state, source[index]);

        index++;
    }

    while(rollback(&token, &state));

    if(dfa->state_tok(state, &token.lexeme) != grammarDFA::T_INVALID){
        token.symbol = dfa->state_tok(state, &token.lexeme);
        token.line = line;

        // comment may terminate with \n or \0; these must be truncated from lexeme
        if(token.symbol == grammarDFA::T_COMMENT && (token.lexeme.back() == '\n' || token.lexeme.back() == '\0')){
            token.lexeme = token.lexeme.substr(0,token.lexeme.length()-1);
            index--;
        }
        else if(token.symbol == grammarDFA::T_STRING || token.symbol == grammarDFA::T_COMMENT){
            for(char c : token.lexeme){
                if(c == '\n'){
                    line++;
                }
            }
        }

        *token_ptr = token;
        return true;
    }
    else{
        return false;
    }
}

// inherently ignores comment tokens
bool lexer::peekTokens(Token* token_array, int k){
    unsigned long int curr_index = index;
    unsigned int curr_line = line;
    bool ret = true;

    for(int i = 0; i < k; i++){
        if(!getNextToken(token_array + i)){
            ret = false;
            break;
        }

        if((token_array + i)->symbol == grammarDFA::T_COMMENT){
            i--; // ignore comment
        }
    }

    index = curr_index;
    line = curr_line;
    return ret;
}

void lexer::states_stack_clear(){
    while(!states_stack.empty()){
        states_stack.pop();
    }
}

bool lexer::rollback(Token* token_ptr, grammarDFA::State* state_ptr){
    if(!states_stack.empty()){
        *state_ptr = states_stack.top();

        states_stack.pop();
        token_ptr->lexeme = token_ptr->lexeme.substr(0,token_ptr->lexeme.length()-1);

        index--;

        return true;
    }
    else{
        return false;
    }
}
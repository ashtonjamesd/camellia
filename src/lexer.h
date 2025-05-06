#ifndef LEXER_H
#define LEXER_H

#include "token.h"

typedef enum {
    INVALID_ESCAPE_SEQUENCE,
    EMPTY_CHAR_LITERAL,
    NO_LEXER_ERROR
} LexerError;

typedef struct {
    char *source;
    Token *tokens;
    int current;
    int token_capacity;
    int token_count;

    // whether to print debug information, used in development
    int debug;

    // the error that occurred, null if none took place
    LexerError err;
} Lexer;

extern Lexer *init_lexer(char *source);
extern void tokenize(Lexer *lexer);
extern void free_lexer(Lexer *lexer);

#endif
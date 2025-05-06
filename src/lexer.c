#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "lexer.h"

static SymbolToken SYMBOLS[] = {
    {";", TOKEN_SEMICOLON},
    {"(", TOKEN_LEFT_PAREN},
    {")", TOKEN_RIGHT_PAREN},
    {"{", TOKEN_LEFT_BRACE},
    {"}", TOKEN_RIGHT_BRACE},
    {"=", TOKEN_SINGLE_EQUALS},
    {",", TOKEN_COMMA},
};

static SymbolToken KEYWORDS[] = {
    {"int", TOKEN_INT},
    {"return", TOKEN_RETURN},
    {"char", TOKEN_CHAR},
};

static const int SYMBOL_COUNT = sizeof(SYMBOLS) / sizeof(SymbolToken);
static const int KEYWORDS_COUNT = sizeof(KEYWORDS) / sizeof(SymbolToken);

Lexer *init_lexer(char *source) {
    Lexer *lexer = (Lexer *)malloc(sizeof(Lexer));
    lexer->source = strdup(source);
    lexer->current = 0;
    lexer->token_capacity = 1;
    lexer->token_count = 0;
    lexer->tokens = (Token *)malloc(sizeof(Token));
    lexer->err = NO_LEXER_ERROR;

    return lexer;
}

char *lexer_err_to_str(LexerError err) {
    switch (err) {
        case EMPTY_CHAR_LITERAL: return "A char literal cannot be empty.\n";
        case INVALID_ESCAPE_SEQUENCE: return "Invalid escape character.\n";
        case NO_LEXER_ERROR: return "No lexer error.";
        default: return "Unknown Error - uhhhh, oops.\n";
    }
}

static inline char current_char(Lexer *lexer) {
    return lexer->source[lexer->current];
}

static inline void advance(Lexer *lexer) {
    lexer->current++;
}

static inline void recede(Lexer *lexer) {
    lexer->current--;
}

static Token *parse_symbol(Lexer *lexer) {
    TokenType type;

    for (int i = 0; i < SYMBOL_COUNT; i++) {
        const char *symbol = SYMBOLS[i].symbol;
        size_t len = strlen(symbol);

        if (strncmp(symbol, &lexer->source[lexer->current], len) == 0) {
            return init_token(symbol, SYMBOLS[i].type);
        }
    }

    return NULL;
}

static inline int is_valid_esc(char c) {
    return c == 't' || c == 'n';
}

static inline void lexer_err(Lexer *lexer, LexerError error) {
    lexer->err = error;
}

static Token* parse_char(Lexer *lexer) {
    advance(lexer);

    char *str;
    char esc = current_char(lexer);
    if (esc == '\'') {
        lexer_err(lexer, EMPTY_CHAR_LITERAL);
        return NULL;
    }

    if (esc == '\\') {
        advance(lexer);

        char c = current_char(lexer);
        if (!is_valid_esc(c)) {
            lexer_err(lexer, INVALID_ESCAPE_SEQUENCE);
            return NULL;
        }

        str = (char *)malloc(3);

        str[0] = '\\';
        str[1] = c;
        str[2] = '\0';
    }
    else {
        str = (char *)malloc(2);
        str[0] = esc;
        str[1] = '\0';
    }

    advance(lexer);

    Token *token = init_token(str, TOKEN_CHAR_LITERAL);
    free(str);

    return token;
}

static Token* parse_string(Lexer *lexer) {
    int start = lexer->current;

    advance(lexer);
    while (1) {
        char c = current_char(lexer);

        if (c == '\"') {
            break;
        }
    }
    
    int len = lexer->current - start;
    char *lexeme = (char *)malloc(len + 1);
    strncpy(lexeme, lexer->source + start, len);
    lexeme[len] = '\0';

    recede(lexer);

    Token *token = init_token(lexeme, TOKEN_STRING);
    free(lexeme);

    return token;
}

static Token* parse_numeric(Lexer *lexer) {
    int start = lexer->current;
    while (isdigit(current_char(lexer))) {
        advance(lexer);
    }

    int len = lexer->current - start;
    char *lexeme = (char *)malloc(len + 1);
    strncpy(lexeme, lexer->source + start, len);
    lexeme[len] = '\0';

    recede(lexer);

    Token *token = init_token(lexeme, TOKEN_INTEGER_LITERAL);
    free(lexeme);

    return token;
}

static Token* parse_identifier(Lexer *lexer) {
    int start = lexer->current;
    while (isalpha(current_char(lexer))) {
        advance(lexer);
    }

    int len = lexer->current - start;
    char *lexeme = (char *)malloc(len + 1);
    strncpy(lexeme, lexer->source + start, len);
    lexeme[len] = '\0';

    recede(lexer);

    for (int i = 0; i < KEYWORDS_COUNT; i++) {
        if (strcmp(lexeme, KEYWORDS[i].symbol) == 0) {
            Token *token = init_token(lexeme, KEYWORDS[i].type);
            free(lexeme);

            return token;
        }
    }

    Token *token = init_token(lexeme, TOKEN_IDENTIFIER);
    free(lexeme);

    return token;
}

static Token *parse_token(Lexer *lexer) {
    char c = current_char(lexer);

    if (isalpha(c)) {
        return parse_identifier(lexer);
    }
    else if (isdigit(c)) {
        return parse_numeric(lexer);
    }
    else if (c == '\"') {
        return parse_string(lexer);
    }
    else if (c == '\'') {
        return parse_char(lexer);
    }
    else {
        return parse_symbol(lexer);
    }
}

void tokenize(Lexer *lexer) {
    while (lexer->source[lexer->current]) {
        Token *token = parse_token(lexer);
        advance(lexer);

        if (!token) continue;

        if (lexer->token_count >= lexer->token_capacity) {
            lexer->token_capacity *= 2;
            lexer->tokens = realloc(lexer->tokens, sizeof(Token) * lexer->token_capacity);
        }

        lexer->tokens[lexer->token_count++] = *token;
        free(token);
    }

    if (lexer->err != NO_LEXER_ERROR) {
        printf("%s",lexer_err_to_str(lexer->err));
        return;
    }

    for (int i = 0; i < lexer->token_count; i++) {
        printf("%d '%s': %s\n", i, lexer->tokens[i].lexeme, token_type_to_str(lexer->tokens[i].type));
    }
}

void free_lexer(Lexer *lexer) {
    for (int i = 0; i < lexer->token_count; i++) {
        free(lexer->tokens[i].lexeme);
    }

    free(lexer->source);
    free(lexer->tokens);
    free(lexer);
}
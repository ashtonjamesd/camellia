#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "ast.h"

int main() {
  FILE *fptr = fopen("example/example.c", "r");
  if (!fptr) {
    perror("Error opening source file.");
    return 1;
  }

  fseek(fptr, 0, SEEK_END);
  unsigned long long sz = ftell(fptr);
  rewind(fptr);

  char *buff = (char *)malloc(sz + 1);
  fread(buff, 1, sz, fptr);
  buff[sz] = '\0';

  int lexer_debug = 1;
  Lexer *lexer = init_lexer(buff, lexer_debug);
  if (lexer->err != NO_LEXER_ERROR) return 1;

  fclose(fptr);
  free(buff);

  tokenize(lexer);

  int parser_debug = 1;
  Parser *parser = init_parser(lexer->tokens, parser_debug);
  parse_ast(parser);


  free_lexer(lexer);

  // compile

  free_parser(parser);

  return 0;
}
#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "ast.h"
#include "compile.h"
#include "analyze.h"

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

  int lexer_debug = 0;
  Lexer *lexer = init_lexer(buff, lexer_debug);
  if (lexer->err != NO_LEXER_ERROR) return 1;

  fclose(fptr);
  free(buff);

  tokenize(lexer);

  int parser_debug = 1;
  Parser *parser = init_parser(lexer->tokens, parser_debug);
  parse_ast(parser);

  free_lexer(lexer);

  // Analyzer *analyzer = init_analyzer(parser->tree, parser->node_count);
  // analyze_ast(analyzer);

  // Compiler *compiler = init_compiler(parser->tree, parser->node_count);
  // compile(compiler);

  free_parser(parser);
  // free_compiler(compiler);

  return 0;
}
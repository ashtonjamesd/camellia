#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "ast.h"
#include "compile.h"
#include "analyze.h"
#include "ppd.h"
#include "version.h"

#define LEXER_DEBUG 1
#define PARSER_DEBUG 1

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: %s [ *.c ... ]\n", argv[0]);
    return 1;
  }
  
  if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
      printf("Usage: %s [ *.c ... ]\n", argv[0]);
      printf("Options:\n");
      printf("  --version | -v   Show compiler version\n");
      printf("  --help    | -h   Show this message\n");
      return 0;
  }

  if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0) {
    printf("%s version %s\n", COMPILER_NAME, VERSION_STRING);
    return 0;
  }

  char *file_path = argv[1];

  FILE *fptr = fopen(file_path, "r");
  if (!fptr) {
    fprintf(stderr, "error: file not found '%s'\n", file_path);
    return 1;
  }

  fseek(fptr, 0, SEEK_END);
  unsigned long long sz = ftell(fptr);
  rewind(fptr);

  char *source = (char *)malloc(sz + 1);
  if (!source) {
    printf("Allocation failed for source.");
    fprintf(stderr, "Allocation failed for source.");
    return 1;
  }

  fread(source, 1, sz, fptr);
  source[sz] = '\0';

  fclose(fptr);

  char *preprocessed_source = preprocess(source);
  free(source);

  Lexer *lexer = init_lexer(preprocessed_source, LEXER_DEBUG);
  free(preprocessed_source);

  tokenize(lexer);
  if (lexer->err != NO_LEXER_ERROR) {
    free_lexer(lexer);
    return 1;
  }

  Parser *parser = init_parser(lexer->tokens, PARSER_DEBUG);
  parse_ast(parser);

  free_lexer(lexer);

  if (parser->err != NO_PARSER_ERROR) {
    free_parser(parser);
    return 1;
  }

  // Analyzer *analyzer = init_analyzer(parser->tree, parser->node_count);
  // analyze_ast(analyzer);

  // Compiler *compiler = init_compiler(parser->tree, parser->node_count);
  // compile(compiler);

  free_parser(parser);
  // free_compiler(compiler);

  return 0;
}
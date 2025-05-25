#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "ast.h"
#include "compile.h"
#include "analyze.h"
#include "ppd.h"
#include "version.h"
#include "camc.h"

#define match(long_arg, short_arg) strcmp(argv[i], long_arg) == 0 || strcmp(argv[i], short_arg) == 0

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: %s [ *.c ... ] -o <out>\n", argv[0]);
    return 1;
  }

  // camc init
  //  initializes a c project in the current directory
  //  creates:
  //    - /src/main.c (hello, world)
  //    - /build
  //
  // camc build
  // camc run
  //
  // project file:
  //    camc.yaml
  //
  //    project: "camellia"
  //    version: "0.0.0"
  //    packages: []
  //
  // camc publish
  //    publishes the current .git dir as a c package
  // 
  // camc package
  //    creates a new c package project
  //
  // camc add
  // camc remove
  // camc update
  //
  // camc bump M // bump major version in camc.yaml
  // camc bump m // bump minor version in camc.yaml
  // camc bump p // bump patch version in camc.yaml

  char *file_path = argv[1];
  char *exe_path = "out";

  int emitAsm = 0;
  int emitObj = 0;
  int debug = 0;

  for (int i = 1; i < argc; i++) {
    if (match("--help", "-h")) {
      printf("Usage: %s [ *.c ... ] -o <out> [ flags ]\n", argv[i]);
      printf("Options:\n");
      printf("  --version       | -v    Show compiler version\n");
      printf("  --help          | -h    Show this message\n");
      printf("  --output <file> | -o    Specify an output file\n");
      printf("  --emitasm       | -ea   Tells the compiler not to delete the generated .asm file\n");
      printf("  --emitobj       | -eo   Tells the compiler not to delete the generated .o file\n");
      printf("  --debug         | -d    Prints the compiler debug output\n");
      return 0;
    }
    else if (match("--version", "-v")) {
      printf("%s version %s\n", COMPILER_NAME, VERSION_STRING);
      return 0;
    }
    else if (match("init", "")) {
      return camc_init(argc, argv);
    }
    else if (match("build", "")) {
      return camc_build(argc, argv);
    }
    else if (match("run", "")) {
      return camc_run(argc, argv);
    }
    else if (match("bump", "")) {
      return camc_bump(argc, argv);
    }
    else if (match("--emitasm", "-ea")) {
      emitAsm = 1;
    }
    else if (match("--emitobj", "-eo")) {
      emitObj = 1;
    }
    else if (match("--debug", "-d")) {
      debug = 1;
    }
    else {
      file_path = argv[i]; 
    }
  }

  if (!exe_path) {
    printf("Output directory was not specified.\n");
    return 1;
  }

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
    fprintf(stderr, "Allocation failed for source.");
    return 1;
  }

  fread(source, 1, sz, fptr);
  source[sz] = '\0';

  fclose(fptr);

  char *preprocessed_source = preprocess(source);
  free(source);

  Lexer *lexer = init_lexer(preprocessed_source, debug);
  free(preprocessed_source);

  tokenize(lexer);
  if (lexer->err != NO_LEXER_ERROR) {
    free_lexer(lexer);
    return 1;
  }

  Parser *parser = init_parser(lexer->tokens, debug, file_path);
  parse_ast(parser);

  free_lexer(lexer);

  if (parser->err != NO_PARSER_ERROR) {
    free_parser(parser);
    return 1;
  }

  // Analyzer *analyzer = init_analyzer(parser->tree, parser->node_count);
  // analyze_ast(analyzer);


  // Compiler *compiler = init_compiler(parser->tree, parser->node_count, exe_path, emitAsm, emitObj);
  // compile(compiler);

  free_parser(parser);
  // free_compiler(compiler);

  return 0;
}
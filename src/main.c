#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"

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

  Lexer *lexer = init_lexer(buff);

  fclose(fptr);
  free(buff);

  tokenize(lexer);
  free_lexer(lexer);

  return 0;
}
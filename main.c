#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#define line__ "-------------------------------"

//---------- Brainfuck operations ----------

enum OP_CODE
{
  OP_MVR,          // >
  OP_MVL,          // <
  OP_INC,          // +
  OP_DEC,          // -
  OP_IN,           // ,
  OP_OUT,          // .
  OP_JMPZ,         // [
  OP_JMPNZ,        // ]
  OP_NULL,         // for debug
};

struct BF_OP
{
  enum OP_CODE op;
  int count;
};

//---------- Abstract Syntax Tree ----------

struct AST
{
  struct BF_OP *ops;
  size_t len;
};

void ast_append(struct AST *ast, struct BF_OP op)
{
  size_t size = (ast->len + 1) * sizeof(struct BF_OP);
  struct BF_OP *new = realloc(ast->ops, size);
  if(new == NULL) return;
  memcpy(&new[ast->len*sizeof(struct BF_OP)], &op, size); 
  ast->ops = new;
  ast->len++;
}

void ast_free(struct AST *ast)
{
  free(ast->ops);
}

//---------- Parsing ----------

struct BF_OP char_to_op(char c, int count)
{
  switch(c){
  case '>':
    return (struct BF_OP){OP_MVR,   count};
  case '<':
    return (struct BF_OP){OP_MVL,   count};
  case '+':
    return (struct BF_OP){OP_INC,   count};
  case '-':
    return (struct BF_OP){OP_DEC,   count};
  case ',':
    return (struct BF_OP){OP_IN,    count};
  case '.':
    return (struct BF_OP){OP_OUT,   count};
  case '[':
    return (struct BF_OP){OP_JMPZ,  count};
  case ']':
    return (struct BF_OP){OP_JMPNZ, count};
  }
  return (struct BF_OP){OP_NULL, 0};
}

struct AST* parse_source(char *src, size_t size)
{
  char *c = src;
  struct AST *ast = malloc(sizeof(struct AST));
  for(size_t i = 0; i < size; i++){
    int count = 1;
    switch(c[i]){
    case '>':
      while(c[i+1] == '>'){
	count++;
	i++;
      }
      ast_append(ast, char_to_op(c[i], count));
      #ifdef AST_DEBUG
      printf("OK:\tAdded operation of type OP_MVR %d times to AST.\n", count);
      #endif
      break;
    case '<':
      while(c[i+1] == '<'){
	count++;
	i++;
      }
      ast_append(ast, char_to_op(c[i], count));
      #ifdef AST_DEBUG
      printf("OK:\tAdded operation of type OP_MVL %d times to AST.\n", count);
      #endif
      break;
    case '+':
      while(c[i+1] == '+'){
	count++;
	i++;
      }
      ast_append(ast, char_to_op(c[i], count));
      #ifdef AST_DEBUG
      printf("OK:\tAdded operation of type OP_INC %d times to AST.\n", count);
      #endif
      break;
    case '-':
      while(c[i+1] == '-'){
	count++;
	i++;
      }
      ast_append(ast, char_to_op(c[i], count));
      #ifdef AST_DEBUG
      printf("OK:\tAdded operation of type OP_DEC %d times to AST.\n", count);
      #endif
      break;
    case ',':
      while(c[i+1] == ','){
	count++;
	i++;
      }
      ast_append(ast, char_to_op(c[i], count));
      #ifdef AST_DEBUG
      printf("OK:\tAdded operation of type OP_IN %d times to AST.\n", count);
      #endif
      break;
    case '.':
      while(c[i+1] == '.'){
	count++;
	i++;
      }
      ast_append(ast, char_to_op(c[i], count));
      #ifdef AST_DEBUG
      printf("OK:\tAdded operation of type OP_OUT %d times to AST.\n", count);
      #endif
      break;
    case '[':
      while(c[i+1] == '['){
	count++;
	i++;
      }
      ast_append(ast, char_to_op(c[i], count));
      #ifdef AST_DEBUG
      printf("OK:\tAdded operation of type OP_JMPZ %d times to AST.\n", count);
      #endif
      break;
    case ']':
      while(c[i+1] == ']'){
	count++;
	i++;
      }
      ast_append(ast, char_to_op(c[i], count));
      #ifdef AST_DEBUG
      printf("OK:\tAdded operation of type OP_JMPNZ %d times to AST.\n", count);
      #endif
      break;
    default:
      #ifdef AST_DEBUG
      printf("OK:\tIgnored non-op character\n");
      #endif
      break;
    }
  }
  return ast; 
}

unsigned char tape[30000] = {0};
unsigned char *ptr = tape;

int main(int argc, char **argv)
{
  FILE *src_fd;
  char *filename = NULL;

  int c;
  opterr = 0;

  while((c = getopt(argc, argv, "f:")) != -1){
    switch(c){
    case 'f':
      filename = optarg;
      break;
    case '?':
      if(optopt == 'f')
	fprintf(stderr, "Option -%c requires an argument.\n", optopt);
      else if(isprint(optopt))
	fprintf(stderr, "Unkown option -%c.\n", optopt);
      break;
    default:
      abort();
    }
  }

  src_fd = fopen(filename, "r");
  if(src_fd == NULL){
    perror("fopen");
    exit(1);
  }
  fseek(src_fd, 0, SEEK_END);
  size_t src_size = (size_t)ftell(src_fd);
  fseek(src_fd, 0, SEEK_SET);

  char *src = malloc(src_size + 1);
  fread(src, 1, src_size, src_fd);

  printf("%s\n", line__);
  printf("%s\n", src);
  printf("%s\n", line__);
  printf("File size:\t%ld\n", src_size);
  printf("Filename:\t%s\n\n", filename);

  struct AST *ast = parse_source(src, src_size);
  ast_free(ast);
  free(ast);

  return 0;
}

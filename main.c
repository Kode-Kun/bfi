#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#define line__ "-------------------------------"

//---------- Type declarations ----------

typedef struct BF_OP BF_OP;
typedef struct AST AST;

//---------- Brainfuck operations ----------

typedef enum OP_CODE
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
}OP_CODE;

union OP_PROP
{
  int count;
  BF_OP *match;
};

struct BF_OP
{
  OP_CODE op;
  union OP_PROP prop;
};

//---------- Abstract Syntax Tree ----------

struct AST
{
  BF_OP *ops;
  size_t len;
};

//TODO: fix this cursed ass function. i can't figure this bug out for the life of me.
// it works perfectly for the first two operations appended, but then it starts crashing out.
// it doesn't seem to copy the operation from op to the new array after appending a couple operations successfully.
// it also fails to copy the count sometimes, whenever the count is 1 apparently.
void ast_append(AST *ast, BF_OP op)
{
  size_t size = (ast->len + 1) * sizeof(BF_OP);
  BF_OP *new = realloc(ast->ops, size);
  if(new == NULL) return;
  memcpy(&new[ast->len * sizeof(BF_OP)], &op, sizeof(BF_OP)); 
  ast->ops = new;
  ast->ops[ast->len].op = op.op;
  ast->ops[ast->len].prop.count = op.prop.count;
  ast->len++;
}

void ast_free(AST *ast)
{
  free(ast->ops);
}

//---------- Parsing ----------

BF_OP char_to_op(char c, union OP_PROP prop)
{
  switch(c){
  case '>':
    return (BF_OP){OP_MVR,   {prop}};
  case '<': 
    return (BF_OP){OP_MVL,   {prop}};
  case '+': 
    return (BF_OP){OP_INC,   {prop}};
  case '-': 
    return (BF_OP){OP_DEC,   {prop}};
  case ',': 
    return (BF_OP){OP_IN,    {prop}};
  case '.': 
    return (BF_OP){OP_OUT,   {prop}};
  case '[': 
    return (BF_OP){OP_JMPZ,  {prop}};
  case ']': 
    return (BF_OP){OP_JMPNZ, {prop}};
  }
  return   (BF_OP){OP_NULL,      {0}};
}

AST* parse_source(char *src, size_t size)
{
  char *c = src;
  AST *ast = malloc(sizeof(AST));
  for(size_t i = 0; i < size; i++){
    int count = 1;
    BF_OP *match = NULL;
    switch(c[i]){
    case '>':
      while(c[i+1] == '>'){
	count++;
	i++;
      }
      ast_append(ast, char_to_op(c[i], {count}));
      #ifdef AST_DEBUG
      printf("OK:\tAdded operation of type OP_MVR %d times to AST.\n", count);
      #endif
      break;
    case '<':
      while(c[i+1] == '<'){
	count++;
	i++;
      }
      ast_append(ast, char_to_op(c[i], {count}));
      #ifdef AST_DEBUG
      printf("OK:\tAdded operation of type OP_MVL %d times to AST.\n", count);
      #endif
      break;
    case '+':
      while(c[i+1] == '+'){
	count++;
	i++;
      }
      ast_append(ast, char_to_op(c[i], {count}));
      #ifdef AST_DEBUG
      printf("OK:\tAdded operation of type OP_INC %d times to AST.\n", count);
      #endif
      break;
    case '-':
      while(c[i+1] == '-'){
	count++;
	i++;
      }
      ast_append(ast, char_to_op(c[i], {count}));
      #ifdef AST_DEBUG
      printf("OK:\tAdded operation of type OP_DEC %d times to AST.\n", count);
      #endif
      break;
    case ',':
      while(c[i+1] == ','){
	count++;
	i++;
      }
      ast_append(ast, char_to_op(c[i], {count}));
      #ifdef AST_DEBUG
      printf("OK:\tAdded operation of type OP_IN %d times to AST.\n", count);
      #endif
      break;
    case '.':
      while(c[i+1] == '.'){
	count++;
	i++;
      }
      ast_append(ast, char_to_op(c[i], {count}));
      #ifdef AST_DEBUG
      printf("OK:\tAdded operation of type OP_OUT %d times to AST.\n", count);
      #endif
      break;
    case '[':
      
      ast_append(ast, char_to_op(c[i], {match}));
      #ifdef AST_DEBUG
      printf("OK:\tAdded operation of type OP_JMPZ %d times to AST.\n", count);
      #endif
      break;
    case ']':

      ast_append(ast, char_to_op(c[i], {match}));
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
  fread(src, sizeof(char), src_size, src_fd);

  printf("%s\n", line__);
  printf("%s\n", src);
  printf("%s\n", line__);
  printf("File size:\t%ld\n", src_size);
  printf("Filename:\t%s\n\n", filename);

  AST *ast = parse_source(src, src_size);

  // here's where the actual interpretation happens.
  
  printf("%s\n", line__);

  int last_jmp;
  for(int i = 0; i < (int)ast->len; i++){
    int count = ast->ops[i].prop.count;
    switch(ast->ops[i].op){
    case OP_MVR:
      ptr += count;
      break;
    case OP_MVL:
      ptr -= count;
      break;
    case OP_INC:
      *ptr += count;
      break;
    case OP_DEC:
      *ptr -= count;
      break;
    case OP_IN:
      *ptr = (unsigned char)fgetc(stdin);
      break;
    case OP_OUT:
      for(int j = 0; j < count; j++){
	fprintf(stdout, "%c", *ptr);
      }
      break;
    case OP_JMPZ:
      if(*ptr == 0){
	while(ast->ops[i].op != OP_JMPNZ) i++;
      } else{
	last_jmp = i;
      }
      break;
    case OP_JMPNZ:
      if(*ptr != 0){
	i = last_jmp;
      }
      break;
    case OP_NULL:
      fprintf(stderr, "WARNING: OP_NULL encountered. Ignoring...\n");
    }
  }

  ast_free(ast);
  free(ast);

  return 0;
}

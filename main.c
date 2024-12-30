#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define line__ "-------------------------------"
#define TAPE_LENGTH 30000

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

struct BF_OP
{
  OP_CODE op;
  int count;
};

//---------- Abstract Syntax Tree ----------

struct AST
{
  BF_OP *ops;
  int len;
};

void ast_append(AST *ast, BF_OP op)
{
  size_t size = (ast->len + 1) * sizeof(BF_OP);
  BF_OP *new = realloc(ast->ops, size);
  if(new == NULL) return;
  memcpy(&new[ast->len * sizeof(BF_OP)], &op, sizeof(BF_OP)); 
  ast->ops = new;
  ast->ops[ast->len].op = op.op;
  ast->ops[ast->len].count = op.count;
  ast->len++;
}

void ast_free(AST *ast)
{
  free(ast->ops);
  free(ast);
}

//---------- Parsing ----------

BF_OP char_to_op(char c, int prop)
{
  BF_OP op = {OP_NULL, 0};
  switch(c){
  case '>':
    op = (BF_OP){ .op = OP_MVR,   .count = prop };
    break;
  case '<': 
    op = (BF_OP){ .op = OP_MVL,   .count = prop };
    break;
  case '+': 
    op = (BF_OP){ .op = OP_INC,   .count = prop };
    break;
  case '-': 
    op = (BF_OP){ .op = OP_DEC,   .count = prop };
    break;
  case ',': 
    op = (BF_OP){ .op = OP_IN,    .count = prop };
    break;
  case '.': 
    op = (BF_OP){ .op = OP_OUT,   .count = prop };
    break;
  case '[': 
    op = (BF_OP){ .op = OP_JMPZ,  .count = prop };
    break;			   
  case ']': 			   
    op = (BF_OP){ .op = OP_JMPNZ, .count = prop };
    break;
  }
  return op;
}

char *op_to_str(BF_OP op)
{
  char *s;
  switch(op.op){
  case OP_MVR:
    s = "OP_MVR";
    break;
  case OP_MVL:
    s = "OP_MVL";
    break;
  case OP_INC:
    s = "OP_INC";
    break;
  case OP_DEC:
    s = "OP_DEC";
    break;
  case OP_IN:
    s = "OP_IN";
    break;
  case OP_OUT:
    s = "OP_OUT";
    break;
  case OP_JMPZ:
    s = "OP_JMPZ";
    break;
  case OP_JMPNZ:
    s = "OP_JMPNZ";
    break;
  case OP_NULL:
    s = "OP_NULL";
    break;
  }     
  return s;
}

// returns pointer to heap-allocated AST struct. Caller must call free()
AST* parse_source(char *src, size_t size)
{
  char *c = src;
  AST *ast = malloc(sizeof(AST));
  for(size_t i = 0; i < size; i++){
    int count = 1;
    BF_OP op;
    switch(c[i]){
    case '>':
    case '<':
    case '+':
    case '-':
    case ',':
    case '.':
      while(c[i+count] == c[i]){
	count++;
      }
      i += (count - 1);
      op = char_to_op(c[i], count);
      ast_append(ast, op);
      #ifdef AST_DEBUG
      fprintf(stderr, "OK:\tAdded operation of type %s %d time%sto AST.\n", op_to_str(op), count, count > 1 ? "s " : " ");
      #endif
      break;
    case '[':
    case ']':
      op = char_to_op(c[i], count);
      ast_append(ast, op);
      #ifdef AST_DEBUG
      fprintf(stderr, "OK:\tAdded operation of type %s %d time to AST.\n", op_to_str(op), 1);
      #endif
      break;
    default:
      #ifdef AST_DEBUG
      fprintf(stderr, "OK:\tIgnored non-op character\n");
      #endif
      break;
    }
  }
  return ast;
}

#ifdef AST_DEBUG
void log_ast(AST ast)
{
  FILE *logfile = fopen("ast.log", "w");
  if(logfile == NULL){
    perror("fopen");
    exit(1);
  }
  for(int i = 0; i < ast.len; i++){
    char *op = op_to_str(ast.ops[i]);
    int count = ast.ops[i].count;
    char *fstr = i == ast.len - 1 ? "%s : %d\n" : "%s : %d, ";
    fprintf(logfile, fstr, op, count);
  }
  fclose(logfile);
}
#endif

unsigned char tape[TAPE_LENGTH] = {0};
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
  printf("Filename:\t%s\n", filename);
  #ifdef AST_DEBUG
  printf("\n");
  #endif

  AST *ast = parse_source(src, src_size);
  #ifdef AST_DEBUG
  log_ast(*ast);
  #endif
  free(src);

  // here's where the actual interpretation happens.
  
  printf("%s\n", line__);
  for(int i = 0; i < ast->len; i++){
    int count = ast->ops[i].count;
    int nested_jmps = 0;
    switch(ast->ops[i].op){
    case OP_MVR:
      if(ptr == &tape[TAPE_LENGTH]) ptr = &tape[0];
      else ptr += count;
      break;
    case OP_MVL:
      if(ptr == &tape[0]) ptr = &tape[TAPE_LENGTH];
      else ptr -= count;
      break;
    case OP_INC:
      if(*ptr == UCHAR_MAX) *ptr = 0;
      else *ptr += count;
      break;
    case OP_DEC:
      if(*ptr == 0) *ptr = UCHAR_MAX;
      else *ptr -= count;
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
	for(int j = i; j < (ast->len - i); j++){
	  if(ast->ops[j].op == OP_JMPZ && j != i) nested_jmps++;
	  if(ast->ops[j].op == OP_JMPNZ && nested_jmps > 0){
	    nested_jmps--;
	  }
	  if(ast->ops[j].op == OP_JMPNZ && nested_jmps == 0){
	    i = j;
	    break;
	  }
	}
      }
      break;
    case OP_JMPNZ:
      if(*ptr != 0){
	for(int j = i; j > 0; j--){
	  if(ast->ops[j].op == OP_JMPNZ && j != i) nested_jmps++;
	  if(ast->ops[j].op == OP_JMPZ && nested_jmps > 0){
	    nested_jmps--;
	  }
	  if(ast->ops[j].op == OP_JMPZ && nested_jmps == 0){
	    i = j;
	    break;
	  }
	}
      }
      break;
    case OP_NULL:
      fprintf(stderr, "WARNING: OP_NULL encountered. Ignoring...\n");
    }
  }

  ast_free(ast);

  return 0;
}

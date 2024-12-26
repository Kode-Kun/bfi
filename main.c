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
  int match;
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
  ast->ops[ast->len].prop.count = op.prop.count;
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
  switch(c){
  case '>':
    return (BF_OP){OP_MVR,   {.count = prop}};
  case '<': 
    return (BF_OP){OP_MVL,   {.count = prop}};
  case '+': 
    return (BF_OP){OP_INC,   {.count = prop}};
  case '-': 
    return (BF_OP){OP_DEC,   {.count = prop}};
  case ',': 
    return (BF_OP){OP_IN,    {.count = prop}};
  case '.': 
    return (BF_OP){OP_OUT,   {.count = prop}};
  case '[': 
    return (BF_OP){OP_JMPZ,  {.match = prop}};
  case ']': 
    return (BF_OP){OP_JMPNZ, {.match = prop}};
  }
  return   (BF_OP){OP_NULL,              {0}};
}

// returns pointer to heap-allocated AST struct. Caller must call free()
AST* parse_source(char *src, size_t size)
{
  char *c = src;
  AST *ast = malloc(sizeof(AST));
  for(size_t i = 0; i < size; i++){
    int count = 1;
    int match = 0;
    switch(c[i]){
    // TODO: compact this up. Every single case except the two JMPs do the exact same thing,
    // so it should only be written once. Create function to convert from OP_CODE to string,
    // then compare c[i+count] to c[i] and add count-1 to i 
    case '>':
      while(c[i+1] == '>'){
	count++;
	i++;
      }
      ast_append(ast, char_to_op(c[i], count));
      #ifdef AST_DEBUG
      fprintf(stderr, "OK:\tAdded operation of type OP_MVR %d times to AST.\n", count);
      #endif
      break;
    case '<':
      while(c[i+1] == '<'){
	count++;
	i++;
      }
      ast_append(ast, char_to_op(c[i], count));
      #ifdef AST_DEBUG
      fprintf(stderr, "OK:\tAdded operation of type OP_MVL %d times to AST.\n", count);
      #endif
      break;
    case '+':
      while(c[i+1] == '+'){
	count++;
	i++;
      }
      ast_append(ast, char_to_op(c[i], count));
      #ifdef AST_DEBUG
      fprintf(stderr, "OK:\tAdded operation of type OP_INC %d times to AST.\n", count);
      #endif
      break;
    case '-':
      while(c[i+1] == '-'){
	count++;
	i++;
      }
      ast_append(ast, char_to_op(c[i], count));
      #ifdef AST_DEBUG
      fprintf(stderr, "OK:\tAdded operation of type OP_DEC %d times to AST.\n", count);
      #endif
      break;
    case ',':
      while(c[i+1] == ','){
	count++;
	i++;
      }
      ast_append(ast, char_to_op(c[i], count));
      #ifdef AST_DEBUG
      fprintf(stderr, "OK:\tAdded operation of type OP_IN %d times to AST.\n", count);
      #endif
      break;
    case '.':
      while(c[i+1] == '.'){
	count++;
	i++;
      }
      ast_append(ast, char_to_op(c[i], count));
      #ifdef AST_DEBUG
      fprintf(stderr, "OK:\tAdded operation of type OP_OUT %d times to AST.\n", count);
      #endif
      break;
    case '[':
      ast_append(ast, char_to_op(c[i], match));
      #ifdef AST_DEBUG
      fprintf(stderr, "OK:\tAdded operation of type OP_JMPZ %d times to AST.\n", count);
      #endif
      break;
    case ']':
      ast_append(ast, char_to_op(c[i], match));
      #ifdef AST_DEBUG
      fprintf(stderr, "OK:\tAdded operation of type OP_JMPNZ %d times to AST.\n", count);
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

//TODO: fix this cursed function (flashbacks, anyone?)
// I honestly have no fucking idea how to implement error handling here (which would just be
// in case there's a JMPZ without a matching JMPNZ and vice-versa, and give the exact position
// of the error would be cool too. But i can't even get the JMPs to match in error-less code...)
void match_jmps(AST *ast)
{
  int cur_jmp_i = 0;                             // Index of JMP operation currently being processed
  int unas_jmps = 0; 		                 // Unassigned JMP operations (without a matching JMP)
  int first_jmp = 0;
  for(int i = 0; i < ast->len; i++){
    if(i == ast->len - 1 && unas_jmps != 0) i = first_jmp;
    if(ast->ops[i].op != OP_JMPZ &&
       ast->ops[i].op != OP_JMPNZ) continue;
    else{
      if(first_jmp == 0 && ast->ops[i].op == OP_JMPZ) first_jmp = i;
      if(cur_jmp_i && ast->ops[i].op == OP_JMPZ){
	unas_jmps++;
	continue;
      }
      else if(unas_jmps == 0 && ast->ops[i].op == OP_JMPZ){
	cur_jmp_i = i;
	continue;
      }

      if(ast->ops[i].op == OP_JMPNZ &&
	 unas_jmps == 0){
	ast->ops[cur_jmp_i].prop.match = i;
	ast->ops[i].prop.match = cur_jmp_i;
	cur_jmp_i = 0;
      } else if(ast->ops[i].op == OP_JMPNZ && unas_jmps > 0){
	unas_jmps--;
      }
    }
  }
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
  printf("Filename:\t%s\n", filename);
  #ifdef AST_DEBUG
  printf("\n");
  #endif

  AST *ast = parse_source(src, src_size);
  free(src);
  match_jmps(ast);

  // here's where the actual interpretation happens.
  
  printf("%s\n", line__);
  // int last_jmp;
  for(int i = 0; i < ast->len; i++){
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
	// while(ast->ops[i].op != OP_JMPNZ) i++;
	i = ast->ops[i].prop.match;
      } //else{
	// last_jmp = i;
      //}
      break;
    case OP_JMPNZ:
      if(*ptr != 0){
	// i = last_jmp;
	i = ast->ops[i].prop.match;
      }
      break;
    case OP_NULL:
      fprintf(stderr, "WARNING: OP_NULL encountered. Ignoring...\n");
    }
  }

  ast_free(ast);

  return 0;
}

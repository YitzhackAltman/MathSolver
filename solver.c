#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "./stack.c"


char* read_file(const char* file_path, long* file_size) {
  FILE* f = fopen(file_path, "r");
  if (f == NULL) { goto error; }

  if (fseek(f, 0, SEEK_END) < 0) { goto error; }

  long m = ftell(f);
  if (m < 0) { goto error; }

  if (file_size) *file_size = m ;

  char* buffer = malloc(m + 1);
  if (buffer == NULL) { goto error; }

  if (fseek(f, 0, SEEK_SET) < 0) { goto error; }

  fread(buffer, 1, m, f);

  if(ferror(f)) { goto error; }

  buffer[m] = '\0';

  if(f) { fclose(f); }

  return buffer;
				   
error:
  fprintf(stderr, "[ERROR]: Could not open/read %s: %s\n", file_path, strerror(errno));
  if (f) { fclose(f); }
  if (buffer) { free(buffer); }
  return NULL;
}

char* shift(int* argc, char*** argv) {
  assert(*argc > 0);

  char* result = **argv;
  *argv += 1;
  *argc -= 1;
  return result;
}

#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof((arr)[0]))

#define FOREACH_ARRAY_SIZE(type, item, array, size, body)	\
  do {								\
    for(size_t i = 0;						\
	i < ARRAY_SIZE(array); ++i ) {				\
      type item = array[0];					\
      body;							\
    }								\
}while(0)


typedef enum {
  TOKEN_AND,
  TOKEN_OR,
  TOKEN_XOR,
  TOKEN_THEN,
  TOKEN_ONLY_THEN,
  TOKEN_NOT,
  TOKEN_OPEN_CURLY,
  TOKEN_CLOSE_CURLY,
  TOKEN_SENTENCE,
}Token_Type;

typedef struct {
  Token_Type type;
  char symbol;
}Token;


#define BINARY_CAP 128

typedef struct {
  char symbol;
  char binary_value[BINARY_CAP];
  size_t value_at;
}Sentence_Table;


typedef struct {
  Token* tokens;
  size_t token_size;
  size_t token_capacity;
  
  Sentence_Table* sentences;
  size_t sentence_size;
  size_t bits_count;
  size_t symbols_amount;
}Formula;

typedef struct {
  Formula* formula_data;
  size_t formula_size;
  size_t formala_capacity;
}Formulas;

typedef struct {
  Formulas formulas;
  struct Node* left;
  struct Node* right;
}Node;


/*void token_append(Formula* formula, Token token) {
  if (formula->token_size >= formula->token_capacity) {
    ///resize_array()
    return;
  }
  formula->token_data[formula->token_size++] = token;
}*/

void estimate_formula(Formula * formula) {
  
}


/*void dump_formula_curly(Formula* formula) {
  for(size_t i = 0; i < token_size; ++i) {
    printf("%c ", formula->tokens[i]);
  }

  printf("\n");
}
 */
void dump_formula_table(Formula* formula) {
  // Table 
  for(size_t i = 0; i < formula->symbols_amount; ++i) {
    printf("%c: ", formula->sentences[i].symbol);
    for (size_t j = 0; j < formula->bits_count; ++j) {
      printf("%c ", formula->sentences[i].binary_value[j]);
    }
    printf("\n");
  }
}

void append_sentence(Formula* formula, char symbol) {
  formula->sentences[formula->sentence_size].symbol = symbol;
  for (size_t i = 0; i < formula->bits_count; ++i) {
    formula->sentences[formula->sentence_size].binary_value[i] = ((i >> formula->sentence_size) & 1) == 1 ? '1' : '0';
  }

  formula->sentence_size += 1;
}

Token* parse_token(Formula* formula, const char* expr, size_t expr_size) {
  Token* tokens = malloc(sizeof(Token) * expr_size);
  size_t token_size = 0;
  Stack* stack = stack_create();

  for(size_t i =0; i < expr_size; ++i) {
    if (expr[i] == ' ') continue;
    
    Token token;
    if(expr[i] == '(') {
      token.type = TOKEN_OPEN_CURLY;
      token.symbol = expr[i];
      stack_push(stack, expr[i]);
    }else if(expr[i] == ')') {
      char close_curly = stack_pop(stack);
      char open_curly = stack_pop(stack);
      assert(open_curly == '(');
      dump_formula_table(formula);
      estimate_formula(formula);
    }else if(expr[i] >= 'a' && expr[i] <= 'z') {
      char symbol = expr[i];
      append_sentence(formula, symbol);
    }
  }
  stack_delete(&stack);
  return NULL;
}


//void (*binary_op)(Sentence_Table* table, Formula* formula);

void and(Sentence_Table* table, Formula* formula);
void or(Sentence_Table* table, Formula* formula);
void xor(Sentence_Table* table, Formula* formula);
void then(Sentence_Table* table, Formula* formula);
void not(Sentence_Table* table, Formula* formula);
void only_then(Sentence_Table* table, Formula* formula);

size_t lookup_symbols(char* expr, size_t expr_size) {
  char arr[26];
  for (size_t i = 0; i < expr_size; ++i) {
    if (expr[i] >= 'a' && expr[i] <= 'z') {
      int index = ((expr[i] - '0') - 49) % 26;
      assert(index < 26);
      arr[index] = 1;
    }
  }

  size_t count = 0;
  for (size_t i = 0; i < 26; ++i) {
    if (arr[i] == 1) {
      count += 1;
    }
  } 

  assert(count > 0);
  return count;
}

#define BINARY_OPS 10
void (*binary_op[BINARY_OPS])(Sentence_Table*, Formula*) = { and, or, xor, then, only_then, not };

size_t true_table_bits(size_t symbols) {
  if (symbols == 0) {
    return 1;
  }

  return 2 * true_table_bits(symbols - 1);
}


void done(char** content) {
  free(*content);
  *content = 0;
}


int main(int argc, char** argv) {
  const char* program_name = shift(&argc, &argv);
  (void)program_name;

  if(argc == 0) {
    fprintf(stderr, "[ERROR]: No input file has provided\n");
    exit(1);
  }


  const char* file_path = shift(&argc, &argv);
  long content_size; 
  char* content = read_file(file_path, &content_size);
  size_t symbols_amount = lookup_symbols(content, content_size);

  Formula formula;
  formula.sentences = malloc(sizeof(Sentence_Table) * symbols_amount);
  formula.sentence_size = 0;
  formula.symbols_amount = symbols_amount;
  
  size_t bits_count = true_table_bits(symbols_amount);
  formula.bits_count = bits_count;
  
  assert(bits_count < BINARY_CAP);

  parse_token(&formula, content, content_size);

  dump_formula_table(&formula);
  

  done(&content);
  
  
  return 0;
}


void and(Sentence_Table* table, Formula* formula) {
  (void)table;
  (void)formula;
}

void or(Sentence_Table* table, Formula* formula) {
  (void)table;
  (void)formula;
}

void xor(Sentence_Table* table, Formula* formula) {
  (void)table;
  (void)formula;
}

void then(Sentence_Table* table, Formula* formula) {
  (void)table;
  (void)formula;
}

void not(Sentence_Table* table, Formula* formula) {
  (void)table;
  (void)formula;
}

void only_then(Sentence_Table* table, Formula* formula) {
  (void)table;
  (void)formula;

}

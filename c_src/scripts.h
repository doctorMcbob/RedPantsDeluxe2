#include "utlist.h"
#include "uthash.h"

#ifndef SPRITES_LOAD
#define SPRITES_LOAD 1

// Define Literal Types
#define NONE 0
#define INT 1
#define FLOAT 2
#define STRING 3
#define OPERATOR 4
#define DOT 5
#define QRAND 6
#define QWORLD 7
#define QSONG 8
#define QCOLLIDE 9
#define LIST  10
#define INP_A 11
#define INP_B 12
#define INP_X 13
#define INP_Y 14
#define INP_LEFT 15
#define INP_UP 16
#define INP_RIGHT 17
#define INP_DOWN 18
#define INP_START 19
#define INP_EVENTS 20

// Define Operators
#define PLUS 0
#define MINUS 1
#define MULT 2
#define FLOORDIV 3
#define FLOATDIV 4
#define MOD 5
#define POW 6
#define EQUALS 7
#define LESSTHAN 8
#define MORETHAN 9
#define LESSEQUAL 10
#define MOREEQUAL 11
#define NOTEQUAL 12
#define AND 13
#define OR 14
#define NOT 15
#define NOR 16
#define IN 17
#define AT 18
#define CASTINT 19
#define CASTSTR 20
#define MIN 21
#define MAX 22
#define LEN 23
#define COUNTOF 24
#define EXISTS 25
#define HASFRAME 26
#define CHOICEOF 27
#define ISFRAME 28
#define ISINPUTSTATE 29
#define ABS 30
#define RANGE 31
#define INWORLD 32

// Define Verbs
#define QUIT 0
#define GOODBYE 1
#define BREAK 2
#define RESET 3
#define SET 4
#define REASSIGN 5
#define IF 6
#define ENDIF 7
#define EXEC 8
#define BACK 9
#define FRONT 10
#define IMG 11
#define ACTIVATE 12
#define DEACTIVATE 13
#define KILLFRAME 14
#define MAKEFRAME 15
#define FOCUS 16
#define SCROLLBOUND 17
#define VIEW 18
#define MOVE 19
#define PLACE 20
#define TAKE 21
#define TAKEALL 22
#define REBRAND 23
#define REMOVE 24
#define ADD 25
#define HITBOXES 26
#define HURTBOXES 27
#define CREATE 28
#define UPDATE 29
#define SFX 30
#define SONG 31
#define SFXOFF 32
#define SONGOFF 33
#define OFFSETBGSCROLLX 34
#define OFFSETBGSCROLLY 35
#define FOR 36
#define ENDFOR 37
#define PRINT 38
#define UPDATE_STICKS 39

typedef struct Script {
  int scriptKey;
  struct Statement* statements;
  UT_hash_handle hh;
} Script;

typedef struct SyntaxNode {
  int type;
  union {
    int i;
    float f;
    char *s;
  } data;
  struct SyntaxNode* next;
  struct SyntaxNode* prev;
} SyntaxNode;

typedef struct Statement {
  int verb;
  struct SyntaxNode* params;
  struct SyntaxNode* buffer;
  struct SyntaxNode* script;
  struct Statement* next;
  struct Statement* prev;
} Statement;

typedef struct ScriptMapEntry {
  int scriptKey;
  char state[32];
  int frame;
  struct ScriptMapEntry* next;
  struct ScriptMapEntry* prev;
} ScriptMapEntry;

typedef struct ScriptMap {
  char name[32];
  struct ScriptMapEntry* entries;
  UT_hash_handle hh;
} ScriptMap;

typedef struct ListTypeEntry {
  int listKey;
  struct SyntaxNode* head;
  UT_hash_handle hh;
} ListTypeEntry;

void add_script(int scriptKey);
Script* get_script(int scriptKey);
void add_statement_to_script(int scriptKey, Statement* statement);
Statement* new_statement(int verb);
SyntaxNode* new_syntax_node(int type);
void add_node_to_statement(Statement* statement, SyntaxNode* node);
void add_script_map(const char* name);
ScriptMap* get_script_map(const char* name);
void add_script_to_script_map(const char* name, char* state, int frame, int scriptKey);
int resolve_script(int scriptKey, char* worldKey, char* selfActorKey, char* relatedActorKey, int debug);
void clean_statement(Statement* statement);
void evaluate_literals(Statement* statement,
		       char* worldKey,
		       char* selfActorKey,
		       char* relatedActorKey);
void resolve_operators(Statement* statement,
		       char* worldKey,
		       char* selfActorKey,
		       char* relatedActorKey);
void free_SyntaxNode(SyntaxNode* del);
SyntaxNode* copy_SyntaxNode(SyntaxNode* orig);
int add_list();
ListTypeEntry* get_list(int listKey);
void clear_list(int listKey);
#endif

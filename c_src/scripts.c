#include "scripts.h"
#include "scriptdata.h"
#include "utlist.h"
#include "actors.h"
#include "worlds.h"
#include "debug.h"
#include "stringmachine.h"

ScriptMap *SCRIPT_MAPS[SCRIPT_MAP_SIZE];
int BUFFER[512];
int PARAMS[512];

void print_buffer() {
  int bp = 0;
  printf("%i", BUFFER[bp]);
  while (bp < 512 && BUFFER[bp] != -1) {
    bp++;
    printf(", %i", BUFFER[bp]);
  }
  printf("\n");
}

void print_params() {
  int pp = 0;
  printf("%i", PARAMS[pp]);
  while (pp < 512 && PARAMS[pp] != -1) {
    pp++;
    printf(", %i", PARAMS[pp]);
  }
  printf("\n");
}

void _clear() {
  int i;
  for(i = 0; i < 512; i++) {
    BUFFER[i] = -1;
    PARAMS[i] = -1;
  }
}

void add_script_map(int key) {
  ScriptMap *sm = malloc(sizeof(ScriptMap));
  sm->entries = NULL;
  sm->key = key;
  SCRIPT_MAPS[key] = sm;
}

void add_script_to_script_map(int key, int stateStringKey, int frame, int scriptIdx) {
  ScriptMap *sm = SCRIPT_MAPS[key];
  ScriptMapEntry *sme = malloc(sizeof(ScriptMapEntry));
  sme->state = stateStringKey;
  sme->frame = frame;
  sme->scriptIdx = scriptIdx;
  DL_APPEND(sm->entries, sme);
}

ScriptMap* get_script_map(int key) {
  return (key>512 || key<0) ? NULL : SCRIPT_MAPS[key];
}

int resolve_script(int scriptIdx, Actor* self, Actor* related, World* world) {
  int executionPointer = scriptIdx;
  printf("Resolving script for %s\n", get_string(self->name));
  while (SCRIPTS[executionPointer] != -2000) {
    _clear();
    int verb = SCRIPTS[executionPointer];
    // for each statement in script
    printf("Statement:\n  ");
    print_statement(executionPointer);
    int bufferPointer = 0;
    while (SCRIPTS[executionPointer] != -1000) {
      // evaluate literals
      executionPointer++;
      int type = SCRIPTS[executionPointer];
      switch(type) {
      case INT:
      case STRING:
      case FLOAT:
      case OPERATOR:
	BUFFER[bufferPointer++] = SCRIPTS[executionPointer];
	BUFFER[bufferPointer++] = SCRIPTS[++executionPointer];
	break;
      case NONE:
	BUFFER[bufferPointer++] = INT;
	BUFFER[bufferPointer++] = 0;
      case QRAND:
	BUFFER[bufferPointer++] = INT;
	BUFFER[bufferPointer++] = rand() % 2;
      case QWORLD:
	BUFFER[bufferPointer++] = STRING;
	BUFFER[bufferPointer++] = world->name;
      }
    }
    printf("Buffer:\n  ");
    print_buffer();
    // resolve operators
    bufferPointer = 0;
    while (bufferPointer < 512 && BUFFER[bufferPointer] != -1) {
      if (BUFFER[bufferPointer] != OPERATOR) {
	bufferPointer++;
	switch (BUFFER[bufferPointer]) {
	  
	}
	bufferPointer++;
	continue;
      }
      bufferPointer++;
      bufferPointer++;
    }
    printf("Params:\n  ");
    print_params();
    // resolve verb
    switch (verb) {
    case QUIT:
      return -2;
      break;
    }
     
    executionPointer++;
  }
  printf("End of Script\n");
  return 0;
}


#include "scripts.h"
#include "scriptdata.h"
#include "utlist.h"
#include "actors.h"
#include "worlds.h"
#include "debug.h"
#include "stringmachine.h"
#include "floatmachine.h"
#ifndef STRING_DATA_LOAD
#include "stringdata.h"
#endif

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

void resolve_operators() {
  int bufferPointer = 0;
  int paramPointer = 0;
  while (bufferPointer < 512 && BUFFER[bufferPointer] != -1) {
    int type = BUFFER[bufferPointer++];
    if (type == OPERATOR) {
      int operatorType = BUFFER[bufferPointer];
      switch (operatorType) {
      case PLUS: {
	if (paramPointer == 0) {
	  printf("Cannot + without left hand side\n");
	  break;
	}
	int leftType = PARAMS[paramPointer-2];
	int leftValue = PARAMS[paramPointer-1];
	int rightType = BUFFER[bufferPointer+2];
	int rightValue = BUFFER[bufferPointer+3];
	
	if (rightType == -1) {
	  printf("Cannot + without right hand side\n");
	  break;
	}
	switch (leftType + 3*rightType) {
	case (INT + 3*INT): {
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = leftValue + rightValue;
	  break;
	}
	case (INT + 3*FLOAT): {
	  float f = get_float(rightValue);
	  int i = push_float(f + leftValue);
	  PARAMS[paramPointer-2] = FLOAT;
	  PARAMS[paramPointer-1] = i;
	  break;
	}
	case (INT + 3*STRING): {
	  char *s = int_to_string(leftValue);
	  char *s2 = get_string(rightValue);
	  int i = concat_strings(s, s2);
	  PARAMS[paramPointer-2] = STRING;
	  PARAMS[paramPointer-1] = i;
	  free(s);
	  break;
	}
	case (FLOAT + 3*INT): {
	  float f = get_float(leftValue);
	  int i = push_float(f + rightValue);
	  PARAMS[paramPointer-2] = FLOAT;
	  PARAMS[paramPointer-1] = i;
	  break;
	}
	case (FLOAT + 3*FLOAT): {
	  float f = get_float(leftValue);
	  float f2 = get_float(leftValue);
	  int i = push_float(f + f2);
	  PARAMS[paramPointer-2] = FLOAT;
	  PARAMS[paramPointer-1] = i;
	  break;
	}
	case (FLOAT + 3*STRING): {
	  float f = get_float(leftValue);
	  char *s = float_to_string(f);
	  char *s2 = get_string(rightValue);
	  int i = concat_strings(s, s2);
	  PARAMS[paramPointer-2] = STRING;
	  PARAMS[paramPointer-1] = i;
	  free(s);
	  break;
	}
	case (STRING + 3*INT): {
	  char *s = get_string(leftValue);
	  char *s2 = int_to_string(rightValue);
	  int i = concat_strings(s, s2);
	  PARAMS[paramPointer-2] = STRING;
	  PARAMS[paramPointer-1] = i;
	  free(s);
	  break;
	}
	case (STRING + 3*FLOAT): {
	  char *s = get_string(leftValue);
	  float f = get_float(rightValue);
	  char *s2 = float_to_string(f);
	  int i = concat_strings(s, s2);
	  PARAMS[paramPointer-2] = STRING;
	  PARAMS[paramPointer-1] = i;
	  free(s);
	  break;
	}
	case (STRING + 3*STRING): {
	  char *s = get_string(leftValue);
	  char *s2 = get_string(rightValue);
	  int i = concat_strings(s, s2);
	  PARAMS[paramPointer-2] = STRING;
	  PARAMS[paramPointer-1] = i;
	  break;
	}
	default: {
	  printf("Failed to + types %i + %i\n", leftType, rightType);
	  break;
	}
	}
	break;
      }
      case MINUS: {
	if (paramPointer == 0) {
	  printf("Cannot - without left hand side\n");
	  break;
	}
	int leftType = PARAMS[paramPointer-2]; // -2 type, -1 value, @ paramPointer
	int leftValue = PARAMS[paramPointer-1];
	int rightType = BUFFER[bufferPointer+2]; // @ bufferPointer, +1 value, +2 type, +3 value
	int rightValue = BUFFER[bufferPointer+3];
	
	if (rightType == -1) {
	  printf("Cannot - without right hand side\n");
	  break;
	}
	switch (leftType + 3*rightType) {
	case (INT + 3*INT): {
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = leftValue - rightValue;
	  break;
	}
	case (INT + 3*FLOAT): {
	  float f = get_float(rightValue);
	  int i = push_float(f - leftValue);
	  PARAMS[paramPointer-2] = FLOAT;
	  PARAMS[paramPointer-1] = i;
	  break;
	}
	case (FLOAT + 3*INT): {
	  float f = get_float(leftValue);
	  int i = push_float(f - rightValue);
	  PARAMS[paramPointer-2] = FLOAT;
	  PARAMS[paramPointer-1] = i;
	  break;
	}
	case (FLOAT + 3*FLOAT): {
	  float f = get_float(leftValue);
	  float f2 = get_float(leftValue);
	  int i = push_float(f - f2);
	  PARAMS[paramPointer-2] = FLOAT;
	  PARAMS[paramPointer-1] = i;
	  break;
	}
	default: {
	  printf("Failed to - types %i - %i\n", leftType, rightType);
	  break;
	}
	}
	break;
      }
      case MULT: {
	if (paramPointer == 0) {
	  printf("Cannot * without left hand side\n");
	  break;
	}
	int leftType = PARAMS[paramPointer-2]; // -2 type, -1 value, @ paramPointer
	int leftValue = PARAMS[paramPointer-1];
	int rightType = BUFFER[bufferPointer+2]; // @ bufferPointer, +1 value, +2 type, +3 value
	int rightValue = BUFFER[bufferPointer+3];
	
	if (rightType == -1) {
	  printf("Cannot * without right hand side\n");
	  break;
	}
	switch (leftType + 3*rightType) {
	case (INT + 3*INT): {
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = leftValue * rightValue;
	  break;
	}
	case (INT + 3*FLOAT): {
	  float f = get_float(rightValue);
	  int i = push_float(f * leftValue);
	  PARAMS[paramPointer-2] = FLOAT;
	  PARAMS[paramPointer-1] = i;
	  break;
	}
	case (FLOAT + 3*INT): {
	  float f = get_float(leftValue);
	  int i = push_float(f * rightValue);
	  PARAMS[paramPointer-2] = FLOAT;
	  PARAMS[paramPointer-1] = i;
	  break;
	}
	case (FLOAT + 3*FLOAT): {
	  float f = get_float(leftValue);
	  float f2 = get_float(leftValue);
	  int i = push_float(f * f2);
	  PARAMS[paramPointer-2] = FLOAT;
	  PARAMS[paramPointer-1] = i;
	  break;
	}
	default: {
	  printf("Failed to - types %i * %i\n", leftType, rightType);
	  break;
	}
	}
	break;
      }
      case FLOORDIV: {}
      case FLOATDIV: {}
      case MOD: {}
      case POW: {}
      case EQUALS: {}
      case LESSTHAN: {}
      case MORETHAN: {}
      case LESSEQUAL: {}
      case MOREEQUAL: {}
      case NOTEQUAL: {}
      case AND: {}
      case OR: {}
      case NOT: {}
      case NOR: {}
      case IN: {}
      case AT: {}
      case CASTINT: {}
      case CASTSTR: {}
      case MIN: {}
      case MAX: {}
      case LEN: {}
      case COUNTOF: {}
      case EXISTS: {}
      case HASFRAME: {}
      case CHOICEOF: {}
      case ISFRAME: {}
      case ABS: {}
      case RANGE: {}
      case INWORLD: {}
      }
      bufferPointer++;
    } else {
      PARAMS[paramPointer++] = type;
      PARAMS[paramPointer++] = BUFFER[bufferPointer++];
    }
  }
}


int resolve_script(int scriptIdx, Actor* self, Actor* related, World* world) {
  int executionPointer = scriptIdx;
  printf("Resolving script for %s\n", get_string(self->name));
  while (SCRIPTS[executionPointer] != -2000) {
    _clear();
    clear_float_buffer();
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
      case OPERATOR: {
	BUFFER[bufferPointer++] = SCRIPTS[executionPointer];
	BUFFER[bufferPointer++] = SCRIPTS[++executionPointer];
	break;
      }
      case FLOAT: {
	BUFFER[bufferPointer++] = FLOAT;
	float f = get_float_literal(SCRIPTS[++executionPointer]);
	BUFFER[bufferPointer++] = push_float(f);
	break;
      }
      case NONE: {
	BUFFER[bufferPointer++] = INT;
	BUFFER[bufferPointer++] = 0;
      }
      case QRAND: {
	BUFFER[bufferPointer++] = INT;
	BUFFER[bufferPointer++] = rand() % 2;
      }
      case QWORLD: {
	BUFFER[bufferPointer++] = STRING;
	BUFFER[bufferPointer++] = world->name;
      }
      case DOT: {
	if (bufferPointer < 2) {
	  printf("Could not . with no left hand side\n");
	  break;
	}
	int leftType = BUFFER[bufferPointer-2];
	int leftValue = BUFFER[bufferPointer-1];
	int rightType = SCRIPTS[++executionPointer];
	int rightValue = SCRIPTS[++executionPointer];

	if (rightType != STRING || leftType != STRING) {
	  printf("Dot must be string . string, got %i . %i\n", leftType, rightType);
	  break;
	}

	Actor *a;
	if (leftValue == SELF) a = self;
	else if (leftValue == RELATED) a = related;

	switch (rightValue) {
	case NAME:
	  BUFFER[bufferPointer-2] = STRING;
	  BUFFER[bufferPointer-1] = a->name;
	  break;
	case STATE:
	  BUFFER[bufferPointer-2] = STRING;
	  BUFFER[bufferPointer-1] = a->state;
	  break;
	case X:
	  BUFFER[bufferPointer-2] = INT;
	  BUFFER[bufferPointer-1] = a->ECB->x;	  
	  break;
	case Y:
	  BUFFER[bufferPointer-2] = INT;
	  BUFFER[bufferPointer-1] = a->ECB->y;
	  break;
	case W:
	  BUFFER[bufferPointer-2] = INT;
	  BUFFER[bufferPointer-1] = a->ECB->w;
	  break;
	case H:
	  BUFFER[bufferPointer-2] = INT;
	  BUFFER[bufferPointer-1] = a->ECB->h;
	  break;
	case TOP:
	  BUFFER[bufferPointer-2] = INT;
	  BUFFER[bufferPointer-1] = a->ECB->y;
	  break;
	case LEFT:
	  BUFFER[bufferPointer-2] = INT;
	  BUFFER[bufferPointer-1] = a->ECB->x;
	  break;
	case BOTTOM:
	  BUFFER[bufferPointer-2] = INT;
	  BUFFER[bufferPointer-1] = a->ECB->y + a->ECB->h;
	  break;
	case RIGHT:
	  BUFFER[bufferPointer-2] = INT;
	  BUFFER[bufferPointer-1] = a->ECB->x + a->ECB->w;
	  break;
	case DIRECTION:
	  BUFFER[bufferPointer-2] = INT;
	  BUFFER[bufferPointer-1] = a->direction;
	  break;
	case PLATFORM:
	  BUFFER[bufferPointer-2] = INT;
	  BUFFER[bufferPointer-1] = a->platform;
	  break;
	case ROTATION:
	  BUFFER[bufferPointer-2] = INT;
	  BUFFER[bufferPointer-1] = a->rotation;
	  break;
	case TANGIBLE:
	  BUFFER[bufferPointer-2] = INT;
	  BUFFER[bufferPointer-1] = a->tangible;
	  break;
	case PHYSICS:
	  BUFFER[bufferPointer-2] = INT;
	  BUFFER[bufferPointer-1] = a->physics;
	  break;
	case X_VEL:
	  BUFFER[bufferPointer-2] = FLOAT;
	  BUFFER[bufferPointer-1] = push_float(a->x_vel);
	  break;
	case Y_VEL:
	  BUFFER[bufferPointer-2] = FLOAT;
	  BUFFER[bufferPointer-1] = push_float(a->y_vel);
	  break;
	default: {
	  // todo, attribute map
	}
	}
	break;
	/**/
      }
      case LIST: {}
      case INP_A: {}
      case INP_B: {}
      case INP_X: {}
      case INP_Y: {}
      case INP_LEFT: {}
      case INP_UP: {}
      case INP_RIGHT: {}
      case INP_DOWN: {}
      case INP_START: {}
      case INP_EVENTS: {}
      }
    }
    printf("Buffer:\n  ");
    print_buffer();
    // resolve operators
    resolve_operators();
    printf("Params:\n  ");
    print_params();
    // resolve verb
    switch (verb) {
    case QUIT: {
      return -2;
      break;
    }
    case GOODBYE: {}
    case BREAK: {}
    case RESET: {}
    case SET: {}
    case REASSIGN: {}
    case IF: {}
    case ENDIF: {}
    case EXEC: {}
    case BACK: {}
    case FRONT: {}
    case IMG: {}
    case ACTIVATE: {}
    case DEACTIVATE: {}
    case KILLFRAME: {}
    case MAKEFRAME: {}
    case FOCUS: {}
    case SCROLLBOUND: {}
    case VIEW: {}
    case MOVE: {}
    case PLACE: {}
    case TAKE: {}
    case TAKEALL: {}
    case REBRAND: {}
    case REMOVE: {}
    case ADD: {}
    case HITBOXES: {}
    case HURTBOXES: {}
    case CREATE: {}
    case UPDATE: {}
    case SFX: {}
    case SONG: {}
    case SFXOFF: {}
    case SONGOFF: {}
    case OFFSETBGSCROLLX: {}
    case OFFSETBGSCROLLY: {}
    case FOR:{}
    case ENDFOR:{}
    case PRINT:{}
    case UPDATE_STICKS: {}
    }
    
    executionPointer++;
  }
  printf("End of Script\n");
  return 0;
}


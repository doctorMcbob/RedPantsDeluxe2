#include "scripts.h"
#include "scriptdata.h"
#include "utlist.h"
#include "actors.h"
#include "worlds.h"
#include "debug.h"
#include "inputs.h"
#include "math.h"
#include "stringmachine.h"
#include "floatmachine.h"
#include "lists.h"
#include "frames.h"
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

int get_script_map_key_by_name(int name) {
  int i;
  for(i = 0; i < SCRIPT_MAP_SIZE; i++) {
	if (SCRIPT_MAPS[i] != NULL && SCRIPT_MAPS[i]->name == name) {
	  return i;
	}
  }
  return -1;
}

void add_script_map(int key, int name) {
  ScriptMap *sm = malloc(sizeof(ScriptMap));
  sm->entries = NULL;
  sm->key = key;
  sm->name = name;
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

void resolve_operators(int statement, World* world, int debug) {
  int bufferPointer = 0;
  int paramPointer = 0;
  while (bufferPointer < 512 && BUFFER[bufferPointer] != -1) {
	if (debug == 2) {
		printf("\n   bufferPointer: %i : ", bufferPointer);
		print_buffer();
		printf("   paramPointer : %i : ", paramPointer);
		print_params();
	}
    int type = BUFFER[bufferPointer++];
    if (type == OPERATOR) {
      int operatorType = BUFFER[bufferPointer];
      switch (operatorType) {
      case PLUS: {
	if (paramPointer == 0) {
	  print_statement(statement);
	  printf("Cannot + without left hand side\n");
	  break;
	}

	int leftType = PARAMS[paramPointer-2];
	int leftValue = PARAMS[paramPointer-1];
	int rightType = BUFFER[++bufferPointer];
	int rightValue = BUFFER[++bufferPointer];
	
	if (rightType == -1) {
	  print_statement(statement);
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
	  if (i == -1) {
	    print_statement(statement);
	    printf("Failed Concat %s %s\n", s, s2);
		free(s);
	    break;
	  }
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
	  float f2 = get_float(rightValue);
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
	  if (i == -1) {
	    print_statement(statement);
	    printf("Failed Concat %s %s\n", s, s2);
		free(s);
	    break;
	  }
	  PARAMS[paramPointer-2] = STRING;
	  PARAMS[paramPointer-1] = i;
	  free(s);
	  break;
	}
	case (STRING + 3*INT): {
	  char *s = get_string(leftValue);
	  char *s2 = int_to_string(rightValue);
	  int i = concat_strings(s, s2);
	  if (i == -1) {
	    print_statement(statement);
	    printf("Failed Concat %s %s\n", s, s2);
		free(s2);
	    break;
	  }
	  PARAMS[paramPointer-2] = STRING;
	  PARAMS[paramPointer-1] = i;
	  free(s2);
	  break;
	}
	case (STRING + 3*FLOAT): {
	  char *s = get_string(leftValue);
	  float f = get_float(rightValue);
	  char *s2 = float_to_string(f);
	  int i = concat_strings(s, s2);
	  if (i == -1) {
	    print_statement(statement);
	    printf("Failed Concat %s %s\n", s, s2);
		free(s2);
	    break;
	  }
	  PARAMS[paramPointer-2] = STRING;
	  PARAMS[paramPointer-1] = i;
	  free(s2);
	  break;
	}
	case (STRING + 3*STRING): {
	  char *s = get_string(leftValue);
	  char *s2 = get_string(rightValue);
	  int i = concat_strings(s, s2);
	  if (i == -1) {
	    print_statement(statement);
	    printf("Failed Concat %s %s\n", s, s2);
	    break;
	  }
	  PARAMS[paramPointer-2] = STRING;
	  PARAMS[paramPointer-1] = i;
	  break;
	}
	default: {
	  print_statement(statement);
	  printf("Failed to + types %i + %i\n", leftType, rightType);
	  break;
	}
	}
	break;
      }
      case MINUS: {
	if (paramPointer == 0) {
	  print_statement(statement);
	  printf("Cannot - without left hand side\n");
	  break;
	}

	int leftType = PARAMS[paramPointer-2];
	int leftValue = PARAMS[paramPointer-1];
	int rightType = BUFFER[++bufferPointer];
	int rightValue = BUFFER[++bufferPointer];
	
	if (rightType == -1) {
	  print_statement(statement);
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
	  float f2 = get_float(rightValue);
	  int i = push_float(f - f2);
	  PARAMS[paramPointer-2] = FLOAT;
	  PARAMS[paramPointer-1] = i;
	  break;
	}
	default: {
	  print_statement(statement);
	  printf("Failed to - types %i - %i\n", leftType, rightType);
	  break;
	}
	}
	break;
      }
      case MULT: {
	if (paramPointer == 0) {
	  print_statement(statement);
	  printf("Cannot * without left hand side\n");
	  break;
	}
	
	int leftType = PARAMS[paramPointer-2]; // -2 type, -1 value, @ paramPointer
	int leftValue = PARAMS[paramPointer-1];
	int rightType = BUFFER[++bufferPointer]; // @ bufferPointer, +1 value, +2 type, +3 value
	int rightValue = BUFFER[++bufferPointer];
	
	if (rightType == -1) {
	  print_statement(statement);
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
	  float f2 = get_float(rightValue);
	  int i = push_float(f * f2);
	  PARAMS[paramPointer-2] = FLOAT;
	  PARAMS[paramPointer-1] = i;
	  break;
	}
	default: {
	  print_statement(statement);
	  printf("Failed to * types %i * %i\n", leftType, rightType);
	  break;
	}
	}
	break;
      }
      case FLOORDIV: {
      	if (paramPointer == 0) {
	  print_statement(statement);
	  printf("Cannot // without left hand side\n");
	  break;
	}
	
	int leftType = PARAMS[paramPointer-2];
	int leftValue = PARAMS[paramPointer-1];
	int rightType = BUFFER[++bufferPointer];
	int rightValue = BUFFER[++bufferPointer];
	
	if (rightType == -1) {
	  print_statement(statement);
	  printf("Cannot // without right hand side\n");
	  break;
	}
	switch (leftType + 3*rightType) {
	case (INT + 3*INT): {
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = leftValue / rightValue;
	  break;
	}
	case (INT + 3*FLOAT): {
	  float f = get_float(rightValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = leftValue / (int)f;
	  break;
	}
	case (FLOAT + 3*INT): {
	  float f = get_float(leftValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = (int)f / rightValue;
	  break;
	}
	case (FLOAT + 3*FLOAT): {
	  float f = get_float(leftValue);
	  float f2 = get_float(rightValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = (int)f / (int)f2;
	  break;
	}
	default: {
	  print_statement(statement);
	  printf("Failed to // types %i // %i\n", leftType, rightType);
	  break;
	}
	}
	break;
      }
      case FLOATDIV: {
	if (paramPointer == 0) {
	  print_statement(statement);
	  printf("Cannot / without left hand side\n");
	  break;
	}
	
	int leftType = PARAMS[paramPointer-2]; // -2 type, -1 value, @ paramPointer
	int leftValue = PARAMS[paramPointer-1];
	int rightType = BUFFER[++bufferPointer]; // @ bufferPointer, +1 value, +2 type, +3 value
	int rightValue = BUFFER[++bufferPointer];
	
	if (rightType == -1) {
	  print_statement(statement);
	  printf("Cannot / without right hand side\n");
	  break;
	}
	switch (leftType + 3*rightType) {
	case (INT + 3*INT): {
	  int i = push_float((float)leftValue / rightValue);
	  PARAMS[paramPointer-2] = FLOAT;
	  PARAMS[paramPointer-1] = i;
	  break;
	}
	case (INT + 3*FLOAT): {
	  float f = get_float(rightValue);
	  int i = push_float((float)leftValue / f);
	  PARAMS[paramPointer-2] = FLOAT;
	  PARAMS[paramPointer-1] = i;
	  break;
	}
	case (FLOAT + 3*INT): {
	  float f = get_float(leftValue);
	  int i = push_float(f / rightValue);
	  PARAMS[paramPointer-2] = FLOAT;
	  PARAMS[paramPointer-1] = i;
	  break;
	}
	case (FLOAT + 3*FLOAT): {
	  float f = get_float(leftValue);
	  float f2 = get_float(rightValue);
	  int i = push_float(f / f2);
	  PARAMS[paramPointer-2] = FLOAT;
	  PARAMS[paramPointer-1] = i;
	  break;
	}
	default: {
	  print_statement(statement);
	  printf("Failed to / types %i / %i\n", leftType, rightType);
	  break;
	}
	}
	break;
      }
      case MOD: {
      	if (paramPointer == 0) {
	  print_statement(statement);
	  printf("Cannot %% without left hand side\n");
	  break;
	}
	
	int leftType = PARAMS[paramPointer-2];
	int leftValue = PARAMS[paramPointer-1];
	int rightType = BUFFER[++bufferPointer];
	int rightValue = BUFFER[++bufferPointer];
	
	if (rightType == -1) {
	  print_statement(statement);
	  printf("Cannot %% without right hand side\n");
	  break;
	}
	switch (leftType + 3*rightType) {
	case (INT + 3*INT): {
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = leftValue % rightValue;
	  break;
	}
	case (INT + 3*FLOAT): {
	  float f = get_float(rightValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = leftValue % (int)f;
	  break;
	}
	case (FLOAT + 3*INT): {
	  float f = get_float(leftValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = (int)f % rightValue;
	  break;
	}
	case (FLOAT + 3*FLOAT): {
	  float f = get_float(leftValue);
	  float f2 = get_float(rightValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = (int)f % (int)f2;
	  break;
	}
	default: {
	  print_statement(statement);
	  printf("Failed to %% types %i %% %i\n", leftType, rightType);
	  break;
	}
	}
	break;
      }
      case POW: {
      	if (paramPointer == 0) {
	  print_statement(statement);
	  printf("Cannot ** without left hand side\n");
	  break;
	}
	
	int leftType = PARAMS[paramPointer-2];
	int leftValue = PARAMS[paramPointer-1];
	int rightType = BUFFER[++bufferPointer];
	int rightValue = BUFFER[++bufferPointer];
	
	if (rightType == -1) {
	  print_statement(statement);
	  printf("Cannot ** without right hand side\n");
	  break;
	}
	switch (leftType + 3*rightType) {
	case (INT + 3*INT): {
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = pow(leftValue, rightValue);
	  break;
	}
	case (INT + 3*FLOAT): {
	  float f = get_float(rightValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = pow(leftValue, f);
	  break;
	}
	case (FLOAT + 3*INT): {
	  float f = get_float(leftValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = pow(f, rightValue);
	  break;
	}
	case (FLOAT + 3*FLOAT): {
	  float f = get_float(leftValue);
	  float f2 = get_float(rightValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = pow(f, f2);
	  break;
	}
	default: {
	  print_statement(statement);
	  printf("Failed to ** types %i ** %i\n", leftType, rightType);
	  break;
	}
	}
	break;
      }
      case EQUALS: {
	if (paramPointer == 0) {
	  print_statement(statement);
	  printf("Cannot == without left hand side\n");
	  break;
	}
	
	int leftType = PARAMS[paramPointer-2];
	int leftValue = PARAMS[paramPointer-1];
	int rightType = BUFFER[++bufferPointer];
	int rightValue = BUFFER[++bufferPointer];
	
	if (rightType == -1) {
	  print_statement(statement);
	  printf("Cannot == without right hand side\n");
	  break;
	}
	switch (leftType + 3*rightType) {
	case (INT + 3*INT): {
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = leftValue == rightValue;
	  break;
	}
	case (INT + 3*FLOAT): {
	  float f = get_float(rightValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = (float)leftValue == f;
	  break;
	}
	case (FLOAT + 3*INT): {
	  float f = get_float(leftValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = f == (float)rightValue;
	  break;
	}
	case (FLOAT + 3*FLOAT): {
	  float f = get_float(leftValue);
	  float f2 = get_float(rightValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = f == f2;
	  break;
	}
	case (INT + 3*STRING): {
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = 0;
	  break;
	}
	case (STRING + 3*INT): {
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = 0;
	  break;
	}
	case (FLOAT + 3*STRING): {
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = 0;
	  break;
	}
	case (STRING + 3*FLOAT): {
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = 0;
	  break;
	}
	case (STRING + 3*STRING): {
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = leftValue == rightValue;
	  break;
	}
	default: {
	  print_statement(statement);
	  printf("Failed to == types %i == %i\n", leftType, rightType);
	  break;
	}
	}
	break;
      }
      case LESSTHAN: {
      	if (paramPointer == 0) {
	  print_statement(statement);
	  printf("Cannot < without left hand side\n");
	  break;
	}
	
	int leftType = PARAMS[paramPointer-2];
	int leftValue = PARAMS[paramPointer-1];
	int rightType = BUFFER[++bufferPointer];
	int rightValue = BUFFER[++bufferPointer];
	
	if (rightType == -1) {
	  print_statement(statement);
	  printf("Cannot < without right hand side\n");
	  break;
	}
	switch (leftType + 3*rightType) {
	case (INT + 3*INT): {
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = leftValue < rightValue;
	  break;
	}
	case (INT + 3*FLOAT): {
	  float f = get_float(rightValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = (float)leftValue < f;
	  break;
	}
	case (FLOAT + 3*INT): {
	  float f = get_float(leftValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = f < (float)rightValue;
	  break;
	}
	case (FLOAT + 3*FLOAT): {
	  float f = get_float(leftValue);
	  float f2 = get_float(rightValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = f < f2;
	  break;
	}
	default: {
	  print_statement(statement);
	  printf("Failed to < types %i < %i\n", leftType, rightType);
	  break;
	}
	}
	break;
      }
      case MORETHAN: {
	if (paramPointer == 0) { 
	  print_statement(statement);
	  printf("Cannot > without left hand side\n");
	}	
		
	int leftType = PARAMS[paramPointer-2];
	int leftValue = PARAMS[paramPointer-1];
	int rightType = BUFFER[++bufferPointer];
	int rightValue = BUFFER[++bufferPointer];
	
	if (rightType == -1) {
	  print_statement(statement);
	  printf("Cannot > without right hand side\n");
	  break;
	}
	switch (leftType + 3*rightType) {
	case (INT + 3*INT): {
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = leftValue > rightValue;
	  break;
	}
	case (INT + 3*FLOAT): {
	  float f = get_float(rightValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = (float)leftValue > f;
	  break;
	}
	case (FLOAT + 3*INT): {
	  float f = get_float(leftValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = f > (float)rightValue;
	  break;
	}
	case (FLOAT + 3*FLOAT): {
	  float f = get_float(leftValue);
	  float f2 = get_float(rightValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = f > f2;
	  break;
	}
	default: {
	  print_statement(statement);
	  printf("Failed to > types %i > %i\n", leftType, rightType);
	  break;
	}
	}
	break;
      }
      case LESSEQUAL: {
	if (paramPointer == 0) { 
	  print_statement(statement);
	  printf("Cannot <= without left hand side\n");
	}	
		
	int leftType = PARAMS[paramPointer-2];
	int leftValue = PARAMS[paramPointer-1];
	int rightType = BUFFER[++bufferPointer];
	int rightValue = BUFFER[++bufferPointer];
	
	if (rightType == -1) {
	  print_statement(statement);
	  printf("Cannot <= without right hand side\n");
	  break;
	}
	switch (leftType + 3*rightType) {
	case (INT + 3*INT): {
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = leftValue <= rightValue;
	  break;
	}
	case (INT + 3*FLOAT): {
	  float f = get_float(rightValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = (float)leftValue <= f;
	  break;
	}
	case (FLOAT + 3*INT): {
	  float f = get_float(leftValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = f <= (float)rightValue;
	  break;
	}
	case (FLOAT + 3*FLOAT): {
	  float f = get_float(leftValue);
	  float f2 = get_float(rightValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = f <= f2;
	  break;
	}
	default: {
	  print_statement(statement);
	  printf("Failed to <= types %i <= %i\n", leftType, rightType);
	  break;
	}
	}
	break;
      }
      case MOREEQUAL: {
	if (paramPointer == 0) { 
	  print_statement(statement);
	  printf("Cannot >= without left hand side\n");
	}
		
	int leftType = PARAMS[paramPointer-2];
	int leftValue = PARAMS[paramPointer-1];
	int rightType = BUFFER[++bufferPointer];
	int rightValue = BUFFER[++bufferPointer];
	
	if (rightType == -1) {
	  print_statement(statement);
	  printf("Cannot >= without right hand side\n");
	  break;
	}
	switch (leftType + 3*rightType) {
	case (INT + 3*INT): {
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = leftValue >= rightValue;
	  break;
	}
	case (INT + 3*FLOAT): {
	  float f = get_float(rightValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = (float)leftValue >= f;
	  break;
	}
	case (FLOAT + 3*INT): {
	  float f = get_float(leftValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = f >= (float)rightValue;
	  break;
	}
	case (FLOAT + 3*FLOAT): {
	  float f = get_float(leftValue);
	  float f2 = get_float(rightValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = f >= f2;
	  break;
	}
	default: {
	  print_statement(statement);
	  printf("Failed to >= types %i >= %i\n", leftType, rightType);
	  break;
	}
	}
	break;
      }
      case NOTEQUAL: {
	if (paramPointer == 0) { 
	  print_statement(statement);
	  printf("Cannot != without left hand side\n");
	}
		
	int leftType = PARAMS[paramPointer-2];
	int leftValue = PARAMS[paramPointer-1];
	int rightType = BUFFER[++bufferPointer];
	int rightValue = BUFFER[++bufferPointer];
	
	if (rightType == -1) {
	  print_statement(statement);
	  printf("Cannot != without right hand side\n");
	  break;
	}
	switch (leftType + 3*rightType) {
	case (INT + 3*INT): {
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = leftValue != rightValue;
	  break;
	}
	case (INT + 3*FLOAT): {
	  float f = get_float(rightValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = (float)leftValue != f;
	  break;
	}
	case (FLOAT + 3*INT): {
	  float f = get_float(leftValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = f != (float)rightValue;
	  break;
	}
	case (FLOAT + 3*FLOAT): {
	  float f = get_float(leftValue);
	  float f2 = get_float(rightValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = f != f2;
	  break;
	}
	default: {
	  print_statement(statement);
	  printf("Failed to != types %i != %i\n", leftType, rightType);
	  break;
	}
	}
	break;
      }
      case AND: {
	if (paramPointer == 0) { 
	  print_statement(statement);
	  printf("Cannot and without left hand side\n");
	}
		
	int leftType = PARAMS[paramPointer-2];
	int leftValue = PARAMS[paramPointer-1];
	int rightType = BUFFER[++bufferPointer];
	int rightValue = BUFFER[++bufferPointer];
	
	if (rightType == -1) {
	  print_statement(statement);
	  printf("Cannot and without right hand side\n");
	  break;
	}
	switch (leftType + 3*rightType) {
	case (INT + 3*INT): {
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = leftValue && rightValue;
	  break;
	}
	case (INT + 3*FLOAT): {
	  float f = get_float(rightValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = leftValue && f;
	  break;
	}
	case (FLOAT + 3*INT): {
	  float f = get_float(leftValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = f && rightValue;
	  break;
	}
	case (FLOAT + 3*FLOAT): {
	  float f = get_float(leftValue);
	  float f2 = get_float(rightValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = f && f2;
	  break;
	}
	case (INT + 3*STRING): {
	  PARAMS[paramPointer-2] = INT;
	  int l2 = strlen(get_string(rightValue));
	  PARAMS[paramPointer-1] = leftValue && l2;
	  break;
	}
	case (STRING + 3*INT): {
	  PARAMS[paramPointer-2] = INT;
	  int l1 = strlen(get_string(leftValue));
	  PARAMS[paramPointer-1] = l1 && rightValue;
	  break;
	}
	case (FLOAT + 3*STRING): {
	  PARAMS[paramPointer-2] = INT;
	  float f = get_float(leftValue);
	  int l2 = strlen(get_string(rightValue));
	  PARAMS[paramPointer-1] = f && l2;
	  break;
	}
	case (STRING + 3*FLOAT): {
	  PARAMS[paramPointer-2] = INT;
	  float f = get_float(rightValue);
	  int l1 = strlen(get_string(leftValue));
	  PARAMS[paramPointer-1] = l1 && f;
	  break;
	}
	case (STRING + 3*STRING): {
	  PARAMS[paramPointer-2] = INT;
	  int l1 = strlen(get_string(leftValue)), l2 = strlen(get_string(rightValue));
	  PARAMS[paramPointer-1] = l1 && l2;
	  break;
	}
	case (INT + 3*LIST): {
	  PARAMS[paramPointer-2] = INT;
	  int l2 = len_list(rightValue);
	  PARAMS[paramPointer-1] = leftValue && l2;
	  break;
	}
	case (LIST + 3*INT): {
	  PARAMS[paramPointer-2] = INT;
	  int l1 = len_list(leftValue);
	  PARAMS[paramPointer-1] = l1 && rightValue;
	  break;
	}
	case (FLOAT + 3*LIST): {
	  PARAMS[paramPointer-2] = INT;
	  float f = get_float(leftValue);
	  int l2 = len_list(rightValue);
	  PARAMS[paramPointer-1] = f && l2;
	  break;
	}
	case (LIST + 3*FLOAT): {
	  PARAMS[paramPointer-2] = INT;
	  float f = get_float(rightValue);
	  int l1 = len_list(leftValue);
	  PARAMS[paramPointer-1] = l1 && f;
	  break;
	}
	case (STRING + 3*LIST): {
	  PARAMS[paramPointer-2] = INT;
	  int l1 = strlen(get_string(leftValue)), l2 = len_list(rightValue);
	  PARAMS[paramPointer-1] = l1 && l2;
	  break;
	}
	case (LIST + 3*STRING): {
	  PARAMS[paramPointer-2] = INT;
	  int l1 = len_list(leftValue), l2 = strlen(get_string(rightValue));
	  PARAMS[paramPointer-1] = l1 && l2;
	  break;
	}
	case (LIST + 3*LIST): {
	  PARAMS[paramPointer-2] = INT;
	  int l1 = len_list(leftValue), l2 = len_list(rightValue);
	  PARAMS[paramPointer-1] = l1 && l2;
	  break;
	}
	default: {
	  print_statement(statement);
	  printf("Failed to and types %i and %i\n", leftType, rightType);
	  break;
	}
	}
	break;
	  }
      case OR: {
	if (paramPointer == 0) { 
	  print_statement(statement);
	  printf("Cannot or without left hand side\n");
	}

	int leftType = PARAMS[paramPointer-2];
	int leftValue = PARAMS[paramPointer-1];
	int rightType = BUFFER[++bufferPointer];
	int rightValue = BUFFER[++bufferPointer];
	
	if (rightType == -1) {
	  print_statement(statement);
	  printf("Cannot or without right hand side\n");
	  break;
	}
	switch (leftType + 3*rightType) {
	case (INT + 3*INT): {
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = leftValue || rightValue;
	  break;
	}
	case (INT + 3*FLOAT): {
	  float f = get_float(rightValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = leftValue || f;
	  break;
	}
	case (FLOAT + 3*INT): {
	  float f = get_float(leftValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = f || rightValue;
	  break;
	}
	case (FLOAT + 3*FLOAT): {
	  float f = get_float(leftValue);
	  float f2 = get_float(rightValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = f || f2;
	  break;
	}
	case (STRING + 3*STRING): {
	  PARAMS[paramPointer-2] = INT;
	  int l1 = strlen(get_string(leftValue)), l2 = strlen(get_string(rightValue));
	  PARAMS[paramPointer-1] = l1 || l2;
	}
	case (INT + 3*LIST): {
	  PARAMS[paramPointer-2] = INT;
	  int l2 = len_list(rightValue);
	  PARAMS[paramPointer-1] = leftValue || l2;
	  break;
	}
	case (LIST + 3*INT): {
	  PARAMS[paramPointer-2] = INT;
	  int l1 = len_list(leftValue);
	  PARAMS[paramPointer-1] = l1 || rightValue;
	  break;
	}
	case (FLOAT + 3*LIST): {
	  PARAMS[paramPointer-2] = INT;
	  float f = get_float(leftValue);
	  int l2 = len_list(rightValue);
	  PARAMS[paramPointer-1] = f || l2;
	  break;
	}
	case (LIST + 3*FLOAT): {
	  PARAMS[paramPointer-2] = INT;
	  float f = get_float(rightValue);
	  int l1 = len_list(leftValue);
	  PARAMS[paramPointer-1] = l1 || f;
	  break;
	}
	case (STRING + 3*LIST): {
	  PARAMS[paramPointer-2] = INT;
	  int l1 = strlen(get_string(leftValue)), l2 = len_list(rightValue);
	  PARAMS[paramPointer-1] = l1 || l2;
	  break;
	}
	case (LIST + 3*STRING): {
	  PARAMS[paramPointer-2] = INT;
	  int l1 = len_list(leftValue), l2 = strlen(get_string(rightValue));
	  PARAMS[paramPointer-1] = l1 || l2;
	  break;
	}
	case (LIST + 3*LIST): {
	  PARAMS[paramPointer-2] = INT;
	  int l1 = len_list(leftValue), l2 = len_list(rightValue);
	  PARAMS[paramPointer-1] = l1 || l2;
	  break;
	}
	default: {
	  print_statement(statement);
	  printf("Failed to or types %i or %i\n", leftType, rightType);
	  break;
	}
	}
	break;
	  }
      case NOT: {
	int rightType = BUFFER[++bufferPointer];
	int rightValue = BUFFER[++bufferPointer];
	
	if (rightType == -1) {
	  print_statement(statement);
	  printf("Cannot not without right hand side\n");
	  break;
	}
	switch (rightType) {
	case (INT): {
	  PARAMS[paramPointer++] = INT;
	  PARAMS[paramPointer++] = rightValue == 0;
	  break;
	}
	case (FLOAT): {
	  float f = get_float(rightValue);
	  PARAMS[paramPointer++] = INT;
	  PARAMS[paramPointer++] = f == 0;
	  break;
	}
	case (STRING): {
	  PARAMS[paramPointer++] = INT;
	  char* s = get_string(rightValue);
	  PARAMS[paramPointer++] = s[0] == "\0";
	  break;
	}
	case (LIST): {
	  PARAMS[paramPointer++] = INT;
	  int l = len_list(rightValue);
	  PARAMS[paramPointer++] = l == 0;
	  break;
	}
	default: {
	  print_statement(statement);
	  printf("Failed to not type %i\n", rightType);
	  break;
	}
	}
	break;
	  }
      case NOR: {
	if (paramPointer == 0) { 
	  print_statement(statement);
	  printf("Cannot nor without left hand side\n");
	}
		
	int leftType = PARAMS[paramPointer-2];
	int leftValue = PARAMS[paramPointer-1];
	int rightType = BUFFER[++bufferPointer];
	int rightValue = BUFFER[++bufferPointer];
	
	if (rightType == -1) {
	  print_statement(statement);
	  printf("Cannot nor without right hand side\n");
	  break;
	}
	switch (leftType + 3*rightType) {
	case (INT + 3*INT): {
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = leftValue && rightValue == 0;
	  break;
	}
	case (INT + 3*FLOAT): {
	  float f = get_float(rightValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = leftValue && f == 0;
	  break;
	}
	case (FLOAT + 3*INT): {
	  float f = get_float(leftValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = f && rightValue == 0;
	  break;
	}
	case (FLOAT + 3*FLOAT): {
	  float f = get_float(leftValue);
	  float f2 = get_float(rightValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = f && f2 == 0;
	  break;
	}
	case (STRING + 3*STRING): {
	  PARAMS[paramPointer-2] = INT;
	  int l1 = strlen(get_string(leftValue)), l2 = strlen(get_string(rightValue));
	  PARAMS[paramPointer-1] = l1 && l2 == 0;
	}
	case (STRING + 3*INT): {
	  PARAMS[paramPointer-2] = INT;
	  int l1 = strlen(get_string(leftValue));
	  PARAMS[paramPointer-1] = l1 && rightValue == 0;
	  break;
	}
	case (INT + 3*STRING): {
	  PARAMS[paramPointer-2] = INT;
	  int l2 = strlen(get_string(rightValue));
	  PARAMS[paramPointer-1] = leftValue && l2 == 0;
	  break;
	}
	case (STRING + 3*FLOAT): {
	  float f = get_float(rightValue);
	  PARAMS[paramPointer-2] = INT;
	  int l1 = strlen(get_string(leftValue));
	  PARAMS[paramPointer-1] = l1 && f == 0;
	  break;
	}
	case (FLOAT + 3*STRING): {
	  float f = get_float(leftValue);
	  PARAMS[paramPointer-2] = INT;
	  int l2 = strlen(get_string(rightValue));
	  PARAMS[paramPointer-1] = f && l2 == 0;
	  break;
	}
	case (LIST + 3*LIST): {
	  PARAMS[paramPointer-2] = INT;
	  int l1 = len_list(leftValue), l2 = len_list(rightValue);
	  PARAMS[paramPointer-1] = l1 && l2 == 0;
	  break;
	}
	case (LIST + 3*INT): {
	  PARAMS[paramPointer-2] = INT;
	  int l1 = len_list(leftValue);
	  PARAMS[paramPointer-1] = l1 && rightValue == 0;
	  break;
	}
	case (INT + 3*LIST): {
	  PARAMS[paramPointer-2] = INT;
	  int l2 = len_list(rightValue);
	  PARAMS[paramPointer-1] = leftValue && l2 == 0;
	  break;
	}
	case (LIST + 3*FLOAT): {
	  float f = get_float(rightValue);
	  PARAMS[paramPointer-2] = INT;
	  int l1 = len_list(leftValue);
	  PARAMS[paramPointer-1] = l1 && f == 0;
	  break;
	}
	case (FLOAT + 3*LIST): {
	  float f = get_float(leftValue);
	  PARAMS[paramPointer-2] = INT;
	  int l2 = len_list(rightValue);
	  PARAMS[paramPointer-1] = f && l2 == 0;
	  break;
	}
	default: {
	  print_statement(statement);
	  printf("Failed to nor types %i nor %i\n", leftType, rightType);
	  break;
	}
	}
	break;
	  }
      case IN: {
	if (paramPointer == 0) { 
	  print_statement(statement);
	  printf("Cannot in without left hand side\n");
	  break;
	}
		
	int leftType = PARAMS[paramPointer-2];
	int leftValue = PARAMS[paramPointer-1];
	int rightType = BUFFER[++bufferPointer];
	int rightValue = BUFFER[++bufferPointer];
	
	if (rightType == -1) {
	  print_statement(statement);
	  printf("Cannot in without right hand side\n");
	  break;
	}
	if (rightType != STRING && rightType != LIST) { 
	  print_statement(statement);
	  printf("Cannot in with non-string or list\n");
	  break;
	}
	if (rightType == LIST) {
		int i = in_list(rightValue, leftType, leftValue);
		PARAMS[paramPointer-2] = INT;
		PARAMS[paramPointer-1] = i;
	} else {
		switch (leftType) {
			case INT: {
				char *s1 = int_to_string(leftValue), *s2 = get_string(rightValue);
				int i = strstr(s2, s1) != NULL;
				PARAMS[paramPointer-2] = INT;
				PARAMS[paramPointer-1] = i;
				free(s1);
				break;
			}
			case FLOAT: {
				float f = get_float(leftValue);
				char *s1 = float_to_string(f), *s2 = get_string(rightValue);
				int i = strstr(s2, s1) != NULL;
				PARAMS[paramPointer-2] = INT;
				PARAMS[paramPointer-1] = i;
				free(s1);
				break;
			}
			case STRING: {
				char *s1 = get_string(leftValue), *s2 = get_string(rightValue);
				int i = strstr(s2, s1) != NULL;
				PARAMS[paramPointer-2] = INT;
				PARAMS[paramPointer-1] = i;
				break;
			}
			default: {
				print_statement(statement);
				printf("Cannot in with types %i in %i\n", leftType, rightType);
				break;
			}
		}
	}
	break;
	  }
      case AT: {
	if (paramPointer == 0) {
	  print_statement(statement);
	  printf("Cannot at without left hand side\n");
	  break;
	}

	int leftType = PARAMS[paramPointer-2];
	int leftValue = PARAMS[paramPointer-1];
	int rightType = BUFFER[++bufferPointer];
	int rightValue = BUFFER[++bufferPointer];

	if (rightType == -1) {
	  print_statement(statement);
	  printf("Cannot at without right hand side\n");
	  break;
	}
	if (leftType != STRING && leftType != LIST) { 
	  print_statement(statement);
	  printf("Cannot at with non-string or list\n");
	  break;
	}
	if (rightType != INT) {
	  print_statement(statement);
	  printf("Cannot at with non-int\n");
	  break;
	}
	if (leftType == LIST) {
		ListNode *ln = get_from_list(leftValue, rightValue);
		if (ln != NULL) {
			PARAMS[paramPointer-2] = ln->type;
			PARAMS[paramPointer-1] = ln->value;
		}
	} else {
		char *s = get_string(leftValue);
		int len = strlen(s);
		if (rightValue < len) {
			char* c = malloc(sizeof(char) * 2);
			c[0] = s[rightValue];
			c[1] = '\0';
			PARAMS[paramPointer-2] = STRING;
			PARAMS[paramPointer-1] = add_string(c, 0);
		}
	}
	break;
	  }
      case CASTINT: {
	int rightType = BUFFER[++bufferPointer];
	int rightValue = BUFFER[++bufferPointer];
	
	if (rightType == -1) {
	  print_statement(statement);
	  printf("Cannot int without right hand side\n");
	  break;
	}
	
	switch (rightType) {
	case INT: {
	  break;
	}
	case FLOAT: {
	  float f = get_float(rightValue);
	  PARAMS[paramPointer++] = INT;
	  PARAMS[paramPointer++] = (int) f;
	  break;
	}
	case STRING: {
	  char *s = get_string(rightValue);
	  PARAMS[paramPointer++] = INT;
	  PARAMS[paramPointer++] = atoi(s);
	  break;
	}
	}

	break;
	  }
      case CASTSTR: {
	int rightType = BUFFER[++bufferPointer];
	int rightValue = BUFFER[++bufferPointer];

	if (rightType == -1) {
	  print_statement(statement);
	  printf("Cannot str without right hand side\n");
	  break;
	}

	switch (rightType) {
	case INT: {
	  PARAMS[paramPointer++] = STRING;
	  PARAMS[paramPointer++] = add_string(int_to_string(rightValue), 0);
	  break;
	}
	case FLOAT: {
	  float f = get_float(rightValue);
	  PARAMS[paramPointer++] = STRING;
	  PARAMS[paramPointer++] = add_string(float_to_string(f), 0);
	  break;
	}
	case STRING: {
	  PARAMS[paramPointer++] = STRING;
	  PARAMS[paramPointer++] = rightValue;
	  break;
	}
	}
	break;
	  }
      case MIN: {
	int aType = BUFFER[++bufferPointer];
	int aValue = BUFFER[++bufferPointer];
	int bType = BUFFER[++bufferPointer];
	int bValue = BUFFER[++bufferPointer];

	if (aType == -1 || bType == -1) {
	  print_statement(statement);
	  printf("Cannot min without right hand side\n");
	  break;
	}

	if (aType != INT && aType != FLOAT) {
	  print_statement(statement);
	  printf("Cannot min with non-int or float\n");
	  break;
	}

	if (bType != INT && bType != FLOAT) {
	  print_statement(statement);
	  printf("Cannot min with non-int or float\n");
	  break;
	}

	if (aType == INT && bType == INT) {
	  PARAMS[paramPointer++] = INT;
	  PARAMS[paramPointer++] = aValue < bValue ? aValue : bValue;
	} else {
	  float a = aType == INT ? (float) aValue : get_float(aValue);
	  float b = bType == INT ? (float) bValue : get_float(bValue);
	  PARAMS[paramPointer++] = FLOAT;
	  PARAMS[paramPointer++] = push_float(a < b ? a : b);
	}
	break;
	  }
	  case MAX: {
	int aType = BUFFER[++bufferPointer];
	int aValue = BUFFER[++bufferPointer];
	int bType = BUFFER[++bufferPointer];
	int bValue = BUFFER[++bufferPointer];

	if (aType == -1 || bType == -1) {
	  print_statement(statement);
	  printf("Cannot max without right hand side\n");
	  break;
	}

	if (aType != INT && aType != FLOAT) {
	  print_statement(statement);
	  printf("Cannot max with non-int or float\n");
	  break;
	}

	if (bType != INT && bType != FLOAT) {
	  print_statement(statement);
	  printf("Cannot max with non-int or float\n");
	  break;
	}

	if (aType == INT && bType == INT) {
	  PARAMS[paramPointer++] = INT;
	  PARAMS[paramPointer++] = aValue > bValue ? aValue : bValue;
	} else {
	  float a = aType == INT ? (float) aValue : get_float(aValue);
	  float b = bType == INT ? (float) bValue : get_float(bValue);
	  PARAMS[paramPointer++] = FLOAT;
	  PARAMS[paramPointer++] = push_float(a > b ? a : b);
	}
	break;
	  }
      case LEN: {
	int rightType = BUFFER[++bufferPointer];
	int rightValue = BUFFER[++bufferPointer];

	if (rightType == -1) {
	  print_statement(statement);
	  printf("Cannot len without right hand side\n");
	  break;
	}
	if (rightType != STRING && rightType != LIST) { 
	  print_statement(statement);
	  printf("Cannot len with non-string or list\n");
	  break;
	}
	if (rightType == LIST) {
		int len = len_list(rightValue);
		PARAMS[paramPointer++] = INT;
		PARAMS[paramPointer++] = len;
	} else {
		char *s = get_string(rightValue);
		int len = strlen(s);
		PARAMS[paramPointer++] = INT;
		PARAMS[paramPointer++] = len;
	}
	break;
	  }
      case COUNTOF: {
	int aType = BUFFER[++bufferPointer];
	int aValue = BUFFER[++bufferPointer];
	int bType = BUFFER[++bufferPointer];
	int bValue = BUFFER[++bufferPointer];

	if (aType == -1 || bType == -1) {
	  print_statement(statement);
	  printf("Cannot countof without right hand side\n");
	  break;
	}

	if (aType != STRING && aType != LIST) {
	  print_statement(statement);
	  printf("Cannot countof with non-string or list\n");
	  break;
	}

	if (aType == LIST) {
		int len = len_list(aValue);
		if (len == -1) {
		  print_statement(statement);
		  printf("countof found no list\n");
		  break;
		}
		int count = 0;
		for (int i = 0; i < len; i++) {
			ListNode *ln = get_from_list(aValue, i);
			if (ln == NULL) break;
			if (ln->type == bType && ln->value == bValue) {
				count++;
			}
		}
		PARAMS[paramPointer++] = INT;
		PARAMS[paramPointer++] = count;
	} else {
		char *s = get_string(aValue);
		char *s2 = get_string(bValue);
		int len = strlen(s);
		int len2 = strlen(s2);
		int count = 0;
		for (int i = 0; i < len; i++) {
			if (i + len2 > len) break;
			int j = 0;
			for (; j < len2; j++) {
				if (s[i + j] != s2[j]) break;
			}
			if (j == len2) {
				count++;
			}
		}
		PARAMS[paramPointer++] = INT;
		PARAMS[paramPointer++] = count;
	}

	break;
	  }
      case EXISTS: {
		int rightType = BUFFER[++bufferPointer];
		int rightValue = BUFFER[++bufferPointer];
		
		if (rightType == -1) {
		  print_statement(statement);
		  printf("Cannot exists without right hand side\n");
		  break;
		}

		if (rightType != STRING) {
		  PARAMS[paramPointer++] = INT;
		  PARAMS[paramPointer++] = 0;
		  break;
		}

		PARAMS[paramPointer++] = INT;
		PARAMS[paramPointer++] = exists(rightValue);
		break;
	  }
      case HASFRAME: {
		int rightType = BUFFER[++bufferPointer];
		int rightValue = BUFFER[++bufferPointer];
		
		if (rightType == -1) {
		  print_statement(statement);
		  printf("Cannot hasframe without right hand side\n");
		  break;
		}

		if (rightType != STRING) {
		  print_statement(statement);
		  printf("Cannot hasframe with non-string\n");
		  break;
		}

		PARAMS[paramPointer++] = INT;
		PARAMS[paramPointer++] = has_frame(rightValue);
		break;
	  }
      case CHOICEOF: {
		int rightType = BUFFER[++bufferPointer];
		int rightValue = BUFFER[++bufferPointer];
		if (rightType != LIST) {
		  print_statement(statement);
		  printf("Cannot choiceof with non-list\n");
		  break;
		}
		int len = len_list(rightValue);

		if (len == -1) {
		  print_statement(statement);
		  printf("choiceof found no list\n");
		  break;
		}
		int i = rand() % len;
		ListNode *ln = get_from_list(rightValue, i);
		if (ln == NULL) {
		  print_statement(statement);
		  printf("choiceof found no list\n");
		  break;
		}
		
		PARAMS[paramPointer++] = ln->type;
		PARAMS[paramPointer++] = ln->value;
		break;
	  }
      case ISFRAME: {
		int rightType = BUFFER[++bufferPointer];
		int rightValue = BUFFER[++bufferPointer];

		if (rightType == -1) {
		  print_statement(statement);
		  printf("Cannot isframe without right hand side\n");
		  break;
		}

		if (rightType != STRING) {
		  print_statement(statement);
		  printf("Cannot isframe with non-string\n");
		  break;
		}

		Frame *f = get_frame(rightValue);
		if (f == NULL) {
		  PARAMS[paramPointer++] = INT;
		  PARAMS[paramPointer++] = 0;
		} else {
		  PARAMS[paramPointer++] = INT;
		  PARAMS[paramPointer++] = 1;
		}
		break;
	  }
      case ABS: {
		int rightType = BUFFER[++bufferPointer];
		int rightValue = BUFFER[++bufferPointer];

		if (rightType == -1) {
		  print_statement(statement);
		  printf("Cannot abs without right hand side\n");
		  break;
		}

		if (rightType != INT && rightType != FLOAT) {
		  print_statement(statement);
		  printf("Cannot abs with non-int or float\n");
		  break;
		}

		if (rightType == INT) {
		  PARAMS[paramPointer++] = INT;
		  PARAMS[paramPointer++] = abs(rightValue);
		} else {
		  PARAMS[paramPointer++] = FLOAT;
		  PARAMS[paramPointer++] = push_float(abs(get_float(rightValue)));
		}

		break;
	  }
      case RANGE: {
		int rightType = BUFFER[++bufferPointer];
		int rightValue = BUFFER[++bufferPointer];

		if (rightType == -1) {
		  print_statement(statement);
		  printf("Cannot range without right hand side\n");
		  break;
		}

		if (rightType != INT) {
		  print_statement(statement);
		  printf("Cannot range with non-int\n");
		  break;
		}

		int list = add_list();
		for (int i = 0; i < rightValue; i++) {
		  add_to_list(list, INT, i);
		}

		PARAMS[paramPointer++] = LIST;
		PARAMS[paramPointer++] = list;
		break;
	  }
      case INWORLD: {
		int rightType = BUFFER[++bufferPointer];
		int rightValue = BUFFER[++bufferPointer];

		if (rightType == -1) {
		  print_statement(statement);
		  printf("Cannot inworld without right hand side\n");
		  break;
		}

		if (rightType != STRING) {
		  print_statement(statement);
		  printf("Cannot inworld with non-string\n");
		  break;
		}

		ActorEntry *ae;
		int in_world = 0;
		DL_FOREACH(world->actors, ae) {
		  if (ae->actorKey == rightValue) {
			in_world = 1;
			break;
		  }
		}
		PARAMS[paramPointer++] = INT;
		PARAMS[paramPointer++] = in_world;
		break;
	  }
	  case ISINPUTSTATE: {
		int rightType = BUFFER[++bufferPointer];
		int rightValue = BUFFER[++bufferPointer];

		if (rightType == -1) {
		  print_statement(statement);
		  printf("Cannot isinputstate without right hand side\n");
		  break;
		}

		if (rightType != STRING) {
		  print_statement(statement);
		  printf("Cannot isinputstate with non-string\n");
		  break;
		}
		
		InputState *is = get_input_state(rightValue);

		PARAMS[paramPointer++] = INT;
		PARAMS[paramPointer++] = is != NULL;
		break;
	  }
      }
      bufferPointer++;
    } else {
      PARAMS[paramPointer++] = type;
      PARAMS[paramPointer++] = BUFFER[bufferPointer++];
    }
  }
}

int resolve_script(
	int scriptIdx,
	Actor* self, 
	Actor* related, 
	World* world, 
	int debug,
	int eject,
	int keyType,
	int keyValue,
	int replacerType,
	int replacerValue
 ) {
  int executionPointer = scriptIdx;
  int ifNested = 0;
  while (SCRIPTS[executionPointer] != -2000) {
	if (eject > 0 && eject <= executionPointer) {
		return 0;
	}

	if (debug == 2) {
		printf("Resolving for self (%i) %s\n", self->name, get_string(self->name));
		print_statement(executionPointer);
	}
	int verb = SCRIPTS[executionPointer];
	if (ifNested) {
		if (verb == IF) ifNested++;
		if (verb == ENDIF) ifNested--;
		while (SCRIPTS[++executionPointer] != -1000) {
			if (SCRIPTS[executionPointer] != DOT)
				executionPointer++;
		}
		executionPointer++;
		continue;
	}
    _clear();
    clear_float_buffer();
    // for each statement in script
    int bufferPointer = 0;
    int statement = executionPointer;
	if (debug == 2) printf("Evaluating Literals...\n");
    while (SCRIPTS[executionPointer] != -1000) {
      // evaluate literals
      executionPointer++;
      int type = SCRIPTS[executionPointer];

      switch(type) {
      case INT:
      case STRING:
      case OPERATOR: {
	if (SCRIPTS[executionPointer] == keyType && SCRIPTS[executionPointer+1] == keyValue) {
		BUFFER[bufferPointer++] = replacerType;
		BUFFER[bufferPointer++] = replacerValue;
		executionPointer++;	
		break;
	}
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
	break;
      }
      case QRAND: {
	BUFFER[bufferPointer++] = INT;
	BUFFER[bufferPointer++] = rand() % 2;
	break;
      }
      case QWORLD: {
	BUFFER[bufferPointer++] = STRING;
	BUFFER[bufferPointer++] = world->name;
	break;
      }
	  case QCOLLIDE: {
		int list = add_list();
		BUFFER[bufferPointer++] = LIST;
		BUFFER[bufferPointer++] = list;
		ActorEntry *ae;
		DL_FOREACH(world->actors, ae) {
			Actor *a = get_actor(ae->actorKey);
			if (a == NULL) continue;
			if (SDL_HasIntersection(self->ECB, a->ECB)) {
				add_to_list(list, STRING, a->name);
			}
		}
		break;
	  }
      case DOT: {
	if (bufferPointer < 2) {
	  print_statement(statement);
	  printf("Could not . with no left hand side\n");
	  break;
	}
	int leftType = BUFFER[bufferPointer-2];
	int leftValue = BUFFER[bufferPointer-1];
	int rightType = SCRIPTS[++executionPointer];
	int rightValue = SCRIPTS[++executionPointer];

	if (rightType != STRING || leftType != STRING) {
	  print_statement(statement);
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
	case FRAME:
	  BUFFER[bufferPointer-2] = INT;
	  BUFFER[bufferPointer-1] = a->frame;
	  break;
	case _X:
	  BUFFER[bufferPointer-2] = INT;
	  BUFFER[bufferPointer-1] = a->ECB->x;	  
	  break;
	case _Y:
	  BUFFER[bufferPointer-2] = INT;
	  BUFFER[bufferPointer-1] = a->ECB->y;
	  break;
	case _WIDTH:
	case W:
	  BUFFER[bufferPointer-2] = INT;
	  BUFFER[bufferPointer-1] = a->ECB->w;
	  break;
	case _HEIGHT:
	case H:
	  BUFFER[bufferPointer-2] = INT;
	  BUFFER[bufferPointer-1] = a->ECB->h;
	  break;
	case TOP:
	  BUFFER[bufferPointer-2] = INT;
	  BUFFER[bufferPointer-1] = a->ECB->y;
	  break;
	case _LEFT:
	  BUFFER[bufferPointer-2] = INT;
	  BUFFER[bufferPointer-1] = a->ECB->x;
	  break;
	case BOTTOM:
	  BUFFER[bufferPointer-2] = INT;
	  BUFFER[bufferPointer-1] = a->ECB->y + a->ECB->h;
	  break;
	case _RIGHT:
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
	case _INPUT_NAME:
	  BUFFER[bufferPointer-2] = STRING;
	  BUFFER[bufferPointer-1] = a->_input_name;
	  break;
	default: {
	  Attribute* attr;
	  HASH_FIND_INT(a->attributes, &rightValue, attr);
	  if (attr == NULL) {
	    BUFFER[bufferPointer-2] = INT;
	    BUFFER[bufferPointer-1] = 0;
	  } else {
	    BUFFER[bufferPointer-2] = attr->type;
	    BUFFER[bufferPointer-1] = attr->type == FLOAT ?
	      push_float(attr->value.f) : attr->value.i;
	  }	  
	}
	}
	break;
      }
      case LIST: {
		BUFFER[bufferPointer++] = LIST;
		BUFFER[bufferPointer++] = add_list();
		break;
	  }
      case INP_A: {
		struct InputState *is = get_input_state(self->_input_name);
		if (is == NULL) {
		  BUFFER[bufferPointer++] = INT;
		  BUFFER[bufferPointer++] = 0;
		} else {
		  BUFFER[bufferPointer++] = INT;
		  BUFFER[bufferPointer++] = is->A;
		}
		break;
	  }
      case INP_B: {
		struct InputState *is = get_input_state(self->_input_name);
		if (is == NULL) {
		  BUFFER[bufferPointer++] = INT;
		  BUFFER[bufferPointer++] = 0;
		} else {
		  BUFFER[bufferPointer++] = INT;
		  BUFFER[bufferPointer++] = is->B;
		}
		break;
	  }
      case INP_X: {
		struct InputState *is = get_input_state(self->_input_name);
		if (is == NULL) {
		  BUFFER[bufferPointer++] = INT;
		  BUFFER[bufferPointer++] = 0;
		} else {
		  BUFFER[bufferPointer++] = INT;
		  BUFFER[bufferPointer++] = is->X;
		}
		break;
	  }
      case INP_Y: {
		struct InputState *is = get_input_state(self->_input_name);
		if (is == NULL) {
		  BUFFER[bufferPointer++] = INT;
		  BUFFER[bufferPointer++] = 0;
		} else {
		  BUFFER[bufferPointer++] = INT;
		  BUFFER[bufferPointer++] = is->Y;
		}
		break;
	  }
      case INP_LEFT: {
		struct InputState *is = get_input_state(self->_input_name);
		if (is == NULL) {
		  BUFFER[bufferPointer++] = INT;
		  BUFFER[bufferPointer++] = 0;
		} else {
		  BUFFER[bufferPointer++] = INT;
		  BUFFER[bufferPointer++] = is->LEFT;
		}
		break;
	  }
      case INP_UP: {
		struct InputState *is = get_input_state(self->_input_name);
		if (is == NULL) {
		  BUFFER[bufferPointer++] = INT;
		  BUFFER[bufferPointer++] = 0;
		} else {
		  BUFFER[bufferPointer++] = INT;
		  BUFFER[bufferPointer++] = is->UP;
		}
		break;
	  }
      case INP_RIGHT: {
		struct InputState *is = get_input_state(self->_input_name);
		if (is == NULL) {
		  BUFFER[bufferPointer++] = INT;
		  BUFFER[bufferPointer++] = 0;
		} else {
		  BUFFER[bufferPointer++] = INT;
		  BUFFER[bufferPointer++] = is->RIGHT;
		}
		break;
	  }
      case INP_DOWN: {
		struct InputState *is = get_input_state(self->_input_name);
		if (is == NULL) {
		  BUFFER[bufferPointer++] = INT;
		  BUFFER[bufferPointer++] = 0;
		} else {
		  BUFFER[bufferPointer++] = INT;
		  BUFFER[bufferPointer++] = is->DOWN;
		}
		break;
	  }
      case INP_START: {
		struct InputState *is = get_input_state(self->_input_name);
		
		if (is == NULL) {
		  BUFFER[bufferPointer++] = INT;
		  BUFFER[bufferPointer++] = 0;
		} else {
		  BUFFER[bufferPointer++] = INT;
		  BUFFER[bufferPointer++] = is->START;
		}
		break;
	  }
      case INP_EVENTS: {
		struct InputState *is = get_input_state(self->_input_name);

		int list = add_list();
		BUFFER[bufferPointer++] = LIST;
		BUFFER[bufferPointer++] = list;

		if (is == NULL) break;
		if (is->EVENTS[A_DOWN]) add_to_list(list, STRING, _A_DOWN);
		if (is->EVENTS[A_UP]) add_to_list(list, STRING, _A_UP);
		if (is->EVENTS[B_DOWN]) add_to_list(list, STRING, _B_DOWN);
		if (is->EVENTS[B_UP]) add_to_list(list, STRING, _B_UP);
		if (is->EVENTS[X_DOWN]) add_to_list(list, STRING, _X_DOWN);
		if (is->EVENTS[X_UP]) add_to_list(list, STRING, _X_UP);
		if (is->EVENTS[Y_DOWN]) add_to_list(list, STRING, _Y_DOWN);
		if (is->EVENTS[Y_UP]) add_to_list(list, STRING, _Y_UP);
		if (is->EVENTS[LEFT_DOWN]) add_to_list(list, STRING, _LEFT_DOWN);
		if (is->EVENTS[LEFT_UP]) add_to_list(list, STRING, _LEFT_UP);
		if (is->EVENTS[UP_DOWN]) add_to_list(list, STRING, _UP_DOWN);
		if (is->EVENTS[UP_UP]) add_to_list(list, STRING, _UP_UP);
		if (is->EVENTS[RIGHT_DOWN]) add_to_list(list, STRING, _RIGHT_DOWN);
		if (is->EVENTS[RIGHT_UP]) add_to_list(list, STRING, _RIGHT_UP);
		if (is->EVENTS[DOWN_DOWN]) add_to_list(list, STRING, _DOWN_DOWN);
		if (is->EVENTS[DOWN_UP]) add_to_list(list, STRING, _DOWN_UP);
		if (is->EVENTS[START_DOWN]) add_to_list(list, STRING, _START_DOWN);
		if (is->EVENTS[START_UP]) add_to_list(list, STRING, _START_UP);
		break;
	  }
      }
	  if (debug == 2) {
		printf("  B ");
		print_buffer();
		printf("  P ");
		print_params();
	  }
    }
	if (debug == 2) printf("  Done...\n");
    // resolve operators
    resolve_operators(statement, world, debug);
	if (debug == 2) {
	  printf("  B ");
	  print_buffer();
	  printf("  P ");
	  print_params();
	}
    // resolve verb
    switch (verb) {
    case QUIT: {
      return -2;
    }
    case GOODBYE: {
	  remove_actor_from_worlds(self->name);
	  free_actor(self);
      clear_ownerless_lists();
	  return -1;
	}
    case BREAK: {
	  return 0;
	}
    case RESET: {
		break;
	}
    case SET: {
      int nameType = PARAMS[0];
      int nameValue = PARAMS[1];
      int attrType = PARAMS[2];
      int attrValue = PARAMS[3];
      int valueType = PARAMS[4];
      int valueValue = PARAMS[5];

      if (nameType != STRING || attrType != STRING || valueType == -1) {
	print_statement(statement);
	printf("Missing or Incorrect Parameter for SET\n");
	break;
      }

      Actor* a;
      if (nameValue == SELF) a = self;
      else if (nameValue == RELATED) a = related;
      else a = get_actor(nameValue);
      if (a == NULL) {
	print_statement(statement);
	printf("Could not find actor %s, %i for SET (is self %i? %i)\n", get_string(nameValue), nameValue, SELF, nameValue == SELF);
	break;

      }
      switch (attrValue) {
      case NAME:
	if (valueType != STRING) break;
	a->name = valueValue;
	break;
      case STATE:
	if (valueType != STRING) break;
	a->state = valueValue;
	break;
	  case FRAME:
	if (valueType != INT) break;
	a->frame = valueValue;
	break;
      case _X: 
	if (valueType != INT) break;
	a->ECB->x = valueValue;
	break;
      case _Y:
	if (valueType != INT) break;
	a->ECB->y = valueValue;
	break;
	  case _WIDTH:
      case W:
	if (valueType != INT) break;
	a->ECB->w = valueValue;
	break;
	  case _HEIGHT:
      case H:
	if (valueType != INT) break;
	a->ECB->h = valueValue;
	break;
      case TOP:
	if (valueType != INT) break;
	a->ECB->y = valueValue;
	break;
      case _LEFT:
	if (valueType != INT) break;
	a->ECB->x = valueValue;
	break;
      case BOTTOM:
	if (valueType != INT) break;
	a->ECB->y = valueValue - a->ECB->h;
	break;
      case _RIGHT:
	if (valueType != INT) break;
	a->ECB->x = valueValue - a->ECB->w;
	break;
      case DIRECTION:
	if (valueType != INT) break;
	a->direction = valueValue;
	break;
      case PLATFORM:
	if (valueType != INT) break;
	a->platform = valueValue;
	break;
      case ROTATION:
	if (valueType != INT) break;
	a->rotation = valueValue;
	break;
      case TANGIBLE:
	if (valueType != INT) break;
	a->tangible = valueValue;
	break;
      case PHYSICS:
	if (valueType != INT) break;
	a->physics = valueValue;
	break;
      case X_VEL:
	if (valueType == FLOAT)
		a->x_vel = get_float(valueValue);
	else if (valueType == INT)
		a->x_vel = valueValue;
	break;
      case Y_VEL:
	if (valueType == FLOAT)
		a->y_vel = get_float(valueValue);
	else if (valueType == INT)
		a->y_vel = (float)valueValue;
	break;
	  case _INPUT_NAME:
		if (valueType != STRING) break;
		a->_input_name = valueValue;
		break;
      default: {
	Attribute* attr;
	HASH_FIND_INT(a->attributes, &attrValue, attr);
	if (attr == NULL) {
	  attr = malloc(sizeof(Attribute));
	  attr->name = attrValue;
	  HASH_ADD_INT(a->attributes, name, attr);
	}
	if (attr->type == LIST) remove_owner(attr->value.i);
	attr->type = valueType;
	if (attr->type == FLOAT) attr->value.f = get_float(valueValue);
	else attr->value.i = valueValue;
	if (attr->type == LIST) add_owner(attr->value.i);
      }
      }
	  break;
    }
    case REASSIGN: {
		break;
	}
    case IF: {
		int conditionalType = PARAMS[0];
		int conditionalValue = PARAMS[1];

		int conditional = 0;
		switch (conditionalType) {
		case INT: {
		  if (conditionalValue != 0) conditional = 1;
		  break;
		}
		case FLOAT: {
		  if (get_float(conditionalValue) != 0) conditional = 1;
		  break;
		}
		case STRING: {
		  if (conditionalValue != EMPTY) conditional = 1;
		  break;
		}
		case LIST: {
		  if (len_list(conditionalValue) != 0) conditional = 1;
		  break;
		}
		}
		if (!conditional) ifNested++;
		
		break;
	}
    case EXEC: {
		int scriptType = PARAMS[0];
		int scriptValue = PARAMS[1];

		if (scriptType != STRING) {
		  print_statement(statement);
		  printf("Missing or Incorrect Parameter for EXEC\n");
		  break;
		}
		int frame = -1;
		int state = scriptValue;
		char* scriptValueStr = strdup(get_string(scriptValue));
		
		char* delim = ":";
		
	    char* stateStr = strtok(scriptValueStr, delim);
		char* frameStr = strtok(NULL, delim);

		if (frameStr != NULL) {
			state = index_string(stateStr);
			frame = atoi(frameStr);
		}

		int script = find_script_from_map(self, state, frame);

		if (script != -1)
			resolve_script(script, self, related, world, debug, -1, -1, -1, -1, -1);
		break;
	}
    case BACK: {
		ActorEntry *ae, *tmp;
		int found = 0;
		DL_FOREACH_SAFE(world->actors, ae, tmp) {
		  if (ae->actorKey == self->name) {
			DL_DELETE(world->actors, ae);
			found++;
			break;
		  }
		}
		if (found)
			DL_APPEND(world->actors, ae);
		break;
	}
    case FRONT: {
		ActorEntry *ae, *tmp;
		int found = 0;
		DL_FOREACH_SAFE(world->actors, ae, tmp) {
		  if (ae->actorKey == self->name) {
			DL_DELETE(world->actors, ae);
			found++;
			break;
		  }
		}
		if (found)
			DL_PREPEND(world->actors, ae);
		break;
	}
    case IMG: {
		int imgType = PARAMS[0];
		int imgValue = PARAMS[1];

		if (imgType != STRING) {
		  print_statement(statement);
		  printf("Missing or Incorrect Parameter for IMG\n");
		  break;
		}
		self->img = imgValue;
		break;
	}
    case ACTIVATE: {
		int frameType = PARAMS[0];
		int frameValue = PARAMS[1];

		if (frameType != STRING) {
		  print_statement(statement);
		  printf("Missing or Incorrect Parameter for ACTIVATE\n");
		  break;
		}

		Frame *f = get_frame(frameValue);
		if (f != NULL) {
		  f->active = 1;
		}

		break;
	}
    case DEACTIVATE: {
		int frameType = PARAMS[0];
		int frameValue = PARAMS[1];

		if (frameType != STRING) {
		  print_statement(statement);
		  printf("Missing or Incorrect Parameter for ACTIVATE\n");
		  break;
		}

		Frame *f = get_frame(frameValue);
		if (f != NULL) {
		  f->active = 0;
		}

		break;
	}
    case KILLFRAME: {
		break;
	}
    case MAKEFRAME: {
		break;
	}
    case FOCUS: {
		break;
	}
    case SCROLLBOUND: {
		break;
	}
    case VIEW: {
		break;
	}
    case MOVE: {
		break;
	}
    case PLACE: {
		break;
	}
    case TAKE: {
		break;
	}
    case TAKEALL: {
		break;
	}
    case REBRAND: {
		int actorType = PARAMS[0];
		int actorValue = PARAMS[1];

		if (actorType != STRING) {
		  print_statement(statement);
		  printf("Missing or Incorrect Parameter for REBRAND\n");
		  break;
		}

		self->spritemapkey = actorValue;
		self->scriptmapkey = get_script_map_key_by_name(actorValue);
		
	    int scriptKey = find_script_from_map(self, _START, 0);
   		if (scriptKey != -1) {
    		int resolution = resolve_script(scriptKey, self, NULL, world, debug, -1, -1, -1, -1, -1);
    		if (resolution < 0) return resolution;
		}
		break;
	}
    case REMOVE: {
		int listType = PARAMS[0];
		int listValue = PARAMS[1];
		int valueType = PARAMS[2];
		int valueValue = PARAMS[3];
		if (listType != LIST) {
		  print_statement(statement);
		  printf("Missing or Incorrect Parameter for REMOVE\n");
		  break;
		}
		remove_from_list(listValue, valueType, valueValue);
		break;
	}
    case ADD: {
		int listType = PARAMS[0];
		int listValue = PARAMS[1];
		int valueType = PARAMS[2];
		int valueValue = PARAMS[3];
		if (listType != LIST) {
		  print_statement(statement);
		  printf("Missing or Incorrect Parameter for ADD\n");
		  break;
		}
		add_to_list(listValue, valueType, valueValue);
		break;
	}
    case HITBOXES: {}
    case HURTBOXES: {
		break;
	}
    case CREATE: {
		int templateNameType = PARAMS[0];
		int templateNameValue = PARAMS[1];
		int nameType = PARAMS[2];
		int nameValue = PARAMS[3];
		int xType = PARAMS[4];
		int xValue = PARAMS[5];
		int yType = PARAMS[6];
		int yValue = PARAMS[7];

		if (templateNameType != STRING || nameType != STRING || xType != INT || yType != INT) {
		  print_statement(statement);
		  printf("Missing or Incorrect Parameter for CREATE\n");
		  break;
		}

		Actor *a = add_actor_from_templatekey(templateNameValue, nameValue);
		a->ECB->x = xValue;
		a->ECB->y = yValue;

		add_actor_to_world(world->name, nameValue);

		int script = get_script_for_actor(a);
		if (script != -1) {
			int resolution = resolve_script(script, a, NULL, world, debug, -1, -1, -1, -1, -1);
			if (resolution < 0) return resolution;
		}
		
		a->updated = 1;
		break;
	}
    case UPDATE: {}
    case SFX: {}
    case SONG: {}
    case SFXOFF: {}
    case SONGOFF: {}
    case OFFSETBGSCROLLX: {}
    case OFFSETBGSCROLLY: {}
    case FOR: {
		int keyType = PARAMS[0];
		int keyValue = PARAMS[1];
		int iterType = PARAMS[2];
		int iterValue = PARAMS[3];

		if (keyType != STRING || (iterType != STRING && iterType != LIST)) {
		  print_statement(statement);
		  printf("Missing or Incorrect Parameter for FOR\n");
		  break;
		}
		int forNest = 1;
		int idx = executionPointer;
		while (SCRIPTS[idx++] != -1000);
		int exit = idx;
		while (forNest > 0) {
	      if (SCRIPTS[exit] == FOR) forNest++;
		  if (SCRIPTS[exit] == ENDFOR) forNest--;

		  while (SCRIPTS[exit++] != -1000) {
			if (SCRIPTS[exit] == -2000 || exit >= SCRIPTS_SIZE) {
			  printf("Missing ENDFOR\n");
			  return -1;
			}
		  }
		}
		exit--;

		int len;
		if (iterType == STRING) {
		  len = strlen(get_string(iterValue));
		} else {
		  len = len_list(iterValue);
		}

		for (int i = 0; i < len; i++) {
		  int replacementType;
		  int replacementValue;
		  if (iterType == STRING) {
			replacementType = STRING;
			char* str = get_string(iterValue);
			char v[2];
			v[0] = str[i];
			v[1] = '\0';
			replacementValue = index_string(&v);
			if (replacementValue == -1) {
			  char* newStr = malloc(sizeof(v));
			  strcpy(newStr, v);
			  replacementValue = add_string(newStr, 1);
			}
		  } else {
			ListNode* ln;
			ln = get_from_list(iterValue, i);
			if (ln == NULL) {
				break;
			}
			replacementType = ln->type;
			replacementValue = ln->value;
		  }

		  int resolution = resolve_script(idx, self, NULL, world, debug,
		                                  exit, keyType, keyValue, replacementType, replacementValue);
		  if (resolution < 0) return resolution;
		}
		executionPointer = exit;
		break;
	}
    case ENDFOR:{
		break;
	}
    case PRINT:{
		int type = PARAMS[0];
		int value = PARAMS[1];

		switch (type) {
		case STRING:
			printf("%s", get_string(value));
			break;
		case INT:
			printf("%d", value);
			break;
		case FLOAT:
			printf("%f", get_float(value));
			break;
		case LIST:
			printf("Lists are #TODO hahaha");
			break;
		case NONE:
			printf("None");
			break;
		}
		printf("\n");
		break;
	}
    case UPDATE_STICKS: {}
    }
    executionPointer++;
  }

  clear_ownerless_lists();
  if (debug == 2) {
	printf("Done. number of lists %i\n", get_num_lists());
  }
  return 0;
}

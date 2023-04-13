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

void resolve_operators(int statement, int debug) {
  int bufferPointer = 0;
  int paramPointer = 0;
  while (bufferPointer < 512 && BUFFER[bufferPointer] != -1) {
	if (debug) {
		printf("\nbufferPointer: %i : ", bufferPointer);
		print_buffer();
		printf("paramPointer: %i : ", paramPointer);
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
	  float f2 = get_float(leftValue);
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
	  float f2 = get_float(leftValue);
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
	  float f2 = get_float(leftValue);
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
	  float f2 = get_float(leftValue);
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
	  float f2 = get_float(leftValue);
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
	  float f2 = get_float(leftValue);
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
	  float f2 = get_float(leftValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = f == f2;
	  break;
	}
	case (STRING + 3*STRING): {
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = leftValue == rightValue;
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
	  float f2 = get_float(leftValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = f < f2;
	  break;
	}
	case (STRING + 3*STRING): {
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = leftValue < rightValue;
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
	  float f2 = get_float(leftValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = f > f2;
	  break;
	}
	case (STRING + 3*STRING): {
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = leftValue > rightValue;
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
	  float f2 = get_float(leftValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = f <= f2;
	  break;
	}
	case (STRING + 3*STRING): {
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = leftValue <= rightValue;
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
	  float f2 = get_float(leftValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = f >= f2;
	  break;
	}
	case (STRING + 3*STRING): {
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = leftValue >= rightValue;
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
	  float f2 = get_float(leftValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = f != f2;
	  break;
	}
	case (STRING + 3*STRING): {
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = leftValue != rightValue;
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
	  float f2 = get_float(leftValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = f && f2;
	  break;
	}
	case (STRING + 3*STRING): {
	  PARAMS[paramPointer-2] = INT;
	  int l1 = strlen(get_string(leftValue)), l2 = strlen(get_string(rightValue));
	  PARAMS[paramPointer-1] = l1 && l2;
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
	  float f2 = get_float(leftValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = f || f2;
	  break;
	}
	case (STRING + 3*STRING): {
	  PARAMS[paramPointer-2] = INT;
	  int l1 = strlen(get_string(leftValue)), l2 = strlen(get_string(rightValue));
	  PARAMS[paramPointer-1] = l1 || l2;
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
	  printf("Cannot and without right hand side\n");
	  break;
	}
	switch (rightType) {
	case (INT): {
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = rightValue == 0;
	  break;
	}
	case (FLOAT): {
	  float f = get_float(rightValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = f == 0;
	  break;
	}
	case (STRING): {
	  PARAMS[paramPointer-2] = INT;
	  char* s = get_string(rightValue);
	  PARAMS[paramPointer-1] = s[0] == "\0";
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
	  float f2 = get_float(leftValue);
	  PARAMS[paramPointer-2] = INT;
	  PARAMS[paramPointer-1] = f && f2 == 0;
	  break;
	}
	case (STRING + 3*STRING): {
	  PARAMS[paramPointer-2] = INT;
	  int l1 = strlen(get_string(leftValue)), l2 = strlen(get_string(rightValue));
	  PARAMS[paramPointer-1] = l1 && l2 == 0;
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
	if (rightType != STRING && rightType != LIST) { 
	  print_statement(statement);
	  printf("Cannot at with non-string or list\n");
	  break;
	}
	if (leftType != INT) {
	  print_statement(statement);
	  printf("Cannot at with non-int\n");
	  break;
	}
	if (rightType == LIST) {
		ListNode *ln = get_from_list(rightValue, leftValue);
		if (ln != NULL) {
			PARAMS[paramPointer-2] = ln->type;
			PARAMS[paramPointer-1] = ln->value;
		}
	} else {
		char *s = get_string(rightValue);
		int len = strlen(s);
		if (leftValue < len) {
			PARAMS[paramPointer-2] = STRING;
			PARAMS[paramPointer-1] = add_string(s[leftValue]);
		}
	}
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
	  PARAMS[paramPointer++] = add_string(int_to_string(rightValue));
	  break;
	}
	case FLOAT: {
	  float f = get_float(rightValue);
	  PARAMS[paramPointer++] = STRING;
	  PARAMS[paramPointer++] = add_string(float_to_string(f));
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
	  }
      case COUNTOF: {}
      case EXISTS: {}
      case HASFRAME: {
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
		printf("CHOICEOF: %i\n", i);
		ListNode *ln = get_from_list(rightValue, i);
		if (ln == NULL) {
		  print_statement(statement);
		  printf("choiceof found no list\n");
		  break;
		}
		printf("CHOICEOF: %i %i\n", ln->type, ln->value);
		PARAMS[paramPointer++] = ln->type;
		PARAMS[paramPointer++] = ln->value;
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
	  }
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

int resolve_script(int scriptIdx, Actor* self, Actor* related, World* world, int debug) {
  int executionPointer = scriptIdx;
  while (SCRIPTS[executionPointer] != -2000) {
	if (debug) {
		printf("Resolving for self %s\n", get_string(self->name));
		print_statement(executionPointer);
	}
    _clear();
    clear_float_buffer();
    int verb = SCRIPTS[executionPointer];
    // for each statement in script
    int bufferPointer = 0;
    int statement = executionPointer;
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
	case _X:
	  BUFFER[bufferPointer-2] = INT;
	  BUFFER[bufferPointer-1] = a->ECB->x;	  
	  break;
	case _Y:
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
		// TODO: implement input events in list
	  }
      }
    }
    // resolve operators
    resolve_operators(statement, debug);
	if (debug) {
	  printf("B ");
	  print_buffer();
	  printf("P ");
	  print_params();
	}
    // resolve verb
    switch (verb) {
    case QUIT: {
      return -2;
    }
    case GOODBYE: {
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
	printf("Could not find actor %s for SET\n", get_string(nameValue));
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
      case _X: 
	if (valueType != INT) break;
	a->ECB->x = valueValue;
	break;
      case _Y:
	if (valueType != INT) break;
	a->ECB->y = valueValue;
	break;
      case W:
	if (valueType != INT) break;
	a->ECB->w = valueValue;
	break;
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
	if (valueType != FLOAT) break;
	a->x_vel = get_float(valueValue);
	break;
      case Y_VEL:
	if (valueType != FLOAT) break;
	a->y_vel = get_float(valueValue);
	break;
      default: {
	Attribute* attr;
	HASH_FIND_INT(a->attributes, &attrValue, attr);
	if (attr == NULL) {
	  attr = malloc(sizeof(Attribute));
	  attr->name = attrValue;
	  HASH_ADD_INT(a->attributes, name, attr);
	}
	attr->type = valueType;
	if (attr->type == FLOAT) attr->value.f = get_float(valueValue);
	else attr->value.i = valueValue;
      }
      }
	  break;
    }
    case REASSIGN: {
		break;
	}
    case IF: {
		break;
	}
    case ENDIF: {
		break;
	}
    case EXEC: {
		break;
	}
    case BACK: {
		break;
	}
    case FRONT: {
		break;
	}
    case IMG: {
		break;
	}
    case ACTIVATE: {
		break;
	}
    case DEACTIVATE: {
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

  return 0;
}


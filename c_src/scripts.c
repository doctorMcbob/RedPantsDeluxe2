#include "scripts.h"
#include "actors.h"
#include "debug.h"
#include "floatmachine.h"
#include "frames.h"
#include "inputs.h"
#include "lists.h"
#include "math.h"
#include "scriptdata.h"
#include "sounds.h"
#include "stringmachine.h"
#include "utlist.h"
#include "worlds.h"
#ifndef STRING_DATA_LOAD
#include "stringdata.h"
#endif

ScriptMap *scriptmaps;
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
  for (i = 0; i < 512; i++) {
    BUFFER[i] = -1;
    PARAMS[i] = -1;
  }
}

void add_script_map(int name, int idx) {
  ScriptMap *map = malloc(sizeof(ScriptMap));
  map->name = name;
  map->idx = idx;
  HASH_ADD_INT(scriptmaps, name, map);
}

void load_script_map_into_actor(Actor *a, int scriptMapName) {
  ScriptMap *sm;
  HASH_FIND_INT(scriptmaps, &scriptMapName, sm);
  if (sm == NULL) {
    printf("Failed to find script map %s\n", get_string(scriptMapName));
    return;
  }
  int idx = sm->idx;
  int i = 0;
  for (int i = 0; i < LARGEST_SCRIPT_MAP; i++) {
    a->scriptmap[i] = -1;
  }
  while (i < LARGEST_SCRIPT_MAP) {
    if (SCRIPT_MAPS[idx] == -1000) {
      break;
    }
    a->scriptmap[i++] = SCRIPT_MAPS[idx++];
    a->scriptmap[i++] = SCRIPT_MAPS[idx++];
    a->scriptmap[i++] = SCRIPT_MAPS[idx++];
  }
}

void resolve_operators(int statement, World *world, int debug) {
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

        int leftType = PARAMS[paramPointer - 2];
        int leftValue = PARAMS[paramPointer - 1];
        int rightType = BUFFER[++bufferPointer];
        int rightValue = BUFFER[++bufferPointer];

        if (rightType == -1) {
          print_statement(statement);
          printf("Cannot + without right hand side\n");
          break;
        }

        switch (leftType + 3 * rightType) {
        case (INT + 3 * INT): {
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = leftValue + rightValue;
          break;
        }
        case (INT + 3 * FLOAT): {
          float f = get_float(rightValue);
          int i = push_float((float)leftValue + f);
          PARAMS[paramPointer - 2] = FLOAT;
          PARAMS[paramPointer - 1] = i;
          break;
        }
        case (INT + 3 * STRING): {
          char *s = int_to_string(leftValue);
          char *s2 = get_string(rightValue);
          int i = concat_strings(s, s2);
          if (i == -1) {
            print_statement(statement);
            printf("Failed Concat %s %s\n", s, s2);
            free(s);
            break;
          }
          PARAMS[paramPointer - 2] = STRING;
          PARAMS[paramPointer - 1] = i;
          free(s);
          break;
        }
        case (FLOAT + 3 * INT): {
          float f = get_float(leftValue);
          int i = push_float(f + rightValue);
          PARAMS[paramPointer - 2] = FLOAT;
          PARAMS[paramPointer - 1] = i;
          break;
        }
        case (FLOAT + 3 * FLOAT): {
          float f = get_float(leftValue);
          float f2 = get_float(rightValue);
          int i = push_float(f + f2);
          PARAMS[paramPointer - 2] = FLOAT;
          PARAMS[paramPointer - 1] = i;
          break;
        }
        case (FLOAT + 3 * STRING): {
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
          PARAMS[paramPointer - 2] = STRING;
          PARAMS[paramPointer - 1] = i;
          free(s);
          break;
        }
        case (STRING + 3 * INT): {
          char *s = get_string(leftValue);
          char *s2 = int_to_string(rightValue);
          int i = concat_strings(s, s2);
          if (i == -1) {
            print_statement(statement);
            printf("Failed Concat %s %s\n", s, s2);
            free(s2);
            break;
          }
          PARAMS[paramPointer - 2] = STRING;
          PARAMS[paramPointer - 1] = i;
          free(s2);
          break;
        }
        case (STRING + 3 * FLOAT): {
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
          PARAMS[paramPointer - 2] = STRING;
          PARAMS[paramPointer - 1] = i;
          free(s2);
          break;
        }
        case (STRING + 3 * STRING): {
          char *s = get_string(leftValue);
          char *s2 = get_string(rightValue);
          int i = concat_strings(s, s2);
          if (i == -1) {
            print_statement(statement);
            printf("Failed Concat %s %s\n", s, s2);
            break;
          }
          PARAMS[paramPointer - 2] = STRING;
          PARAMS[paramPointer - 1] = i;
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

        int leftType = PARAMS[paramPointer - 2];
        int leftValue = PARAMS[paramPointer - 1];
        int rightType = BUFFER[++bufferPointer];
        int rightValue = BUFFER[++bufferPointer];

        if (rightType == -1) {
          print_statement(statement);
          printf("Cannot - without right hand side\n");
          break;
        }
        switch (leftType + 3 * rightType) {
        case (INT + 3 * INT): {
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = leftValue - rightValue;
          break;
        }
        case (INT + 3 * FLOAT): {
          float f = get_float(rightValue);
          int i = push_float(f - leftValue);
          PARAMS[paramPointer - 2] = FLOAT;
          PARAMS[paramPointer - 1] = i;
          break;
        }
        case (FLOAT + 3 * INT): {
          float f = get_float(leftValue);
          int i = push_float(f - rightValue);
          PARAMS[paramPointer - 2] = FLOAT;
          PARAMS[paramPointer - 1] = i;
          break;
        }
        case (FLOAT + 3 * FLOAT): {
          float f = get_float(leftValue);
          float f2 = get_float(rightValue);
          int i = push_float(f - f2);
          PARAMS[paramPointer - 2] = FLOAT;
          PARAMS[paramPointer - 1] = i;
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

        int leftType =
            PARAMS[paramPointer - 2]; // -2 type, -1 value, @ paramPointer
        int leftValue = PARAMS[paramPointer - 1];
        int rightType = BUFFER[++bufferPointer]; // @ bufferPointer, +1 value,
                                                 // +2 type, +3 value
        int rightValue = BUFFER[++bufferPointer];

        if (rightType == -1) {
          print_statement(statement);
          printf("Cannot * without right hand side\n");
          break;
        }
        switch (leftType + 3 * rightType) {
        case (INT + 3 * INT): {
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = leftValue * rightValue;
          break;
        }
        case (INT + 3 * FLOAT): {
          float f = get_float(rightValue);
          int i = push_float(f * leftValue);
          PARAMS[paramPointer - 2] = FLOAT;
          PARAMS[paramPointer - 1] = i;
          break;
        }
        case (FLOAT + 3 * INT): {
          float f = get_float(leftValue);
          int i = push_float(f * rightValue);
          PARAMS[paramPointer - 2] = FLOAT;
          PARAMS[paramPointer - 1] = i;
          break;
        }
        case (FLOAT + 3 * FLOAT): {
          float f = get_float(leftValue);
          float f2 = get_float(rightValue);
          int i = push_float(f * f2);
          PARAMS[paramPointer - 2] = FLOAT;
          PARAMS[paramPointer - 1] = i;
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

        int leftType = PARAMS[paramPointer - 2];
        int leftValue = PARAMS[paramPointer - 1];
        int rightType = BUFFER[++bufferPointer];
        int rightValue = BUFFER[++bufferPointer];

        if (rightType == -1) {
          print_statement(statement);
          printf("Cannot // without right hand side\n");
          break;
        }
        switch (leftType + 3 * rightType) {
        case (INT + 3 * INT): {
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = leftValue / rightValue;
          break;
        }
        case (INT + 3 * FLOAT): {
          float f = get_float(rightValue);
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = leftValue / (int)f;
          break;
        }
        case (FLOAT + 3 * INT): {
          float f = get_float(leftValue);
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = (int)f / rightValue;
          break;
        }
        case (FLOAT + 3 * FLOAT): {
          float f = get_float(leftValue);
          float f2 = get_float(rightValue);
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = (int)f / (int)f2;
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

        int leftType =
            PARAMS[paramPointer - 2]; // -2 type, -1 value, @ paramPointer
        int leftValue = PARAMS[paramPointer - 1];
        int rightType = BUFFER[++bufferPointer]; // @ bufferPointer, +1 value,
                                                 // +2 type, +3 value
        int rightValue = BUFFER[++bufferPointer];

        if (rightType == -1) {
          print_statement(statement);
          printf("Cannot / without right hand side\n");
          break;
        }
        switch (leftType + 3 * rightType) {
        case (INT + 3 * INT): {
          int i = push_float((float)leftValue / rightValue);
          PARAMS[paramPointer - 2] = FLOAT;
          PARAMS[paramPointer - 1] = i;
          break;
        }
        case (INT + 3 * FLOAT): {
          float f = get_float(rightValue);
          int i = push_float((float)leftValue / f);
          PARAMS[paramPointer - 2] = FLOAT;
          PARAMS[paramPointer - 1] = i;
          break;
        }
        case (FLOAT + 3 * INT): {
          float f = get_float(leftValue);
          int i = push_float(f / rightValue);
          PARAMS[paramPointer - 2] = FLOAT;
          PARAMS[paramPointer - 1] = i;
          break;
        }
        case (FLOAT + 3 * FLOAT): {
          float f = get_float(leftValue);
          float f2 = get_float(rightValue);
          int i = push_float(f / f2);
          PARAMS[paramPointer - 2] = FLOAT;
          PARAMS[paramPointer - 1] = i;
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

        int leftType = PARAMS[paramPointer - 2];
        int leftValue = PARAMS[paramPointer - 1];
        int rightType = BUFFER[++bufferPointer];
        int rightValue = BUFFER[++bufferPointer];

        if (rightType == -1) {
          print_statement(statement);
          printf("Cannot %% without right hand side\n");
          break;
        }
        switch (leftType + 3 * rightType) {
        case (INT + 3 * INT): {
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = leftValue % rightValue;
          break;
        }
        case (INT + 3 * FLOAT): {
          float f = get_float(rightValue);
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = leftValue % (int)f;
          break;
        }
        case (FLOAT + 3 * INT): {
          float f = get_float(leftValue);
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = (int)f % rightValue;
          break;
        }
        case (FLOAT + 3 * FLOAT): {
          float f = get_float(leftValue);
          float f2 = get_float(rightValue);
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = (int)f % (int)f2;
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

        int leftType = PARAMS[paramPointer - 2];
        int leftValue = PARAMS[paramPointer - 1];
        int rightType = BUFFER[++bufferPointer];
        int rightValue = BUFFER[++bufferPointer];

        if (rightType == -1) {
          print_statement(statement);
          printf("Cannot ** without right hand side\n");
          break;
        }
        switch (leftType + 3 * rightType) {
        case (INT + 3 * INT): {
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = pow(leftValue, rightValue);
          break;
        }
        case (INT + 3 * FLOAT): {
          float f = get_float(rightValue);
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = pow(leftValue, f);
          break;
        }
        case (FLOAT + 3 * INT): {
          float f = get_float(leftValue);
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = pow(f, rightValue);
          break;
        }
        case (FLOAT + 3 * FLOAT): {
          float f = get_float(leftValue);
          float f2 = get_float(rightValue);
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = pow(f, f2);
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

        int leftType = PARAMS[paramPointer - 2];
        int leftValue = PARAMS[paramPointer - 1];
        int rightType = BUFFER[++bufferPointer];
        int rightValue = BUFFER[++bufferPointer];

        if (rightType == -1) {
          print_statement(statement);
          printf("Cannot == without right hand side\n");
          break;
        }
        switch (leftType + 3 * rightType) {
        case (INT + 3 * INT): {
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = leftValue == rightValue;
          break;
        }
        case (INT + 3 * FLOAT): {
          float f = get_float(rightValue);
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = (float)leftValue == f;
          break;
        }
        case (FLOAT + 3 * INT): {
          float f = get_float(leftValue);
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = f == (float)rightValue;
          break;
        }
        case (FLOAT + 3 * FLOAT): {
          float f = get_float(leftValue);
          float f2 = get_float(rightValue);
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = f == f2;
          break;
        }
        case (STRING + 3 * STRING): {
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = leftValue == rightValue;
          break;
        }
        case (NONE + 3 * NONE): {
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = 1;
          break;
        }
        case (INT + 3 * STRING):
        case (STRING + 3 * INT):
        case (FLOAT + 3 * STRING):
        case (STRING + 3 * FLOAT):
        case (INT + 3 * NONE):
        case (FLOAT + 3 * NONE):
        case (STRING + 3 * NONE):
        case (NONE + 3 * INT):
        case (NONE + 3 * STRING):
        case (NONE + 3 * FLOAT): {
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = 0;
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

        int leftType = PARAMS[paramPointer - 2];
        int leftValue = PARAMS[paramPointer - 1];
        int rightType = BUFFER[++bufferPointer];
        int rightValue = BUFFER[++bufferPointer];

        if (rightType == -1) {
          print_statement(statement);
          printf("Cannot < without right hand side\n");
          break;
        }
        switch (leftType + 3 * rightType) {
        case (INT + 3 * INT): {
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = leftValue < rightValue;
          break;
        }
        case (INT + 3 * FLOAT): {
          float f = get_float(rightValue);
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = (float)leftValue < f;
          break;
        }
        case (FLOAT + 3 * INT): {
          float f = get_float(leftValue);
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = f < (float)rightValue;
          break;
        }
        case (FLOAT + 3 * FLOAT): {
          float f = get_float(leftValue);
          float f2 = get_float(rightValue);
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = f < f2;
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

        int leftType = PARAMS[paramPointer - 2];
        int leftValue = PARAMS[paramPointer - 1];
        int rightType = BUFFER[++bufferPointer];
        int rightValue = BUFFER[++bufferPointer];

        if (rightType == -1) {
          print_statement(statement);
          printf("Cannot > without right hand side\n");
          break;
        }
        switch (leftType + 3 * rightType) {
        case (INT + 3 * INT): {
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = leftValue > rightValue;
          break;
        }
        case (INT + 3 * FLOAT): {
          float f = get_float(rightValue);
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = (float)leftValue > f;
          break;
        }
        case (FLOAT + 3 * INT): {
          float f = get_float(leftValue);
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = f > (float)rightValue;
          break;
        }
        case (FLOAT + 3 * FLOAT): {
          float f = get_float(leftValue);
          float f2 = get_float(rightValue);
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = f > f2;
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

        int leftType = PARAMS[paramPointer - 2];
        int leftValue = PARAMS[paramPointer - 1];
        int rightType = BUFFER[++bufferPointer];
        int rightValue = BUFFER[++bufferPointer];

        if (rightType == -1) {
          print_statement(statement);
          printf("Cannot <= without right hand side\n");
          break;
        }
        switch (leftType + 3 * rightType) {
        case (INT + 3 * INT): {
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = leftValue <= rightValue;
          break;
        }
        case (INT + 3 * FLOAT): {
          float f = get_float(rightValue);
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = (float)leftValue <= f;
          break;
        }
        case (FLOAT + 3 * INT): {
          float f = get_float(leftValue);
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = f <= (float)rightValue;
          break;
        }
        case (FLOAT + 3 * FLOAT): {
          float f = get_float(leftValue);
          float f2 = get_float(rightValue);
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = f <= f2;
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

        int leftType = PARAMS[paramPointer - 2];
        int leftValue = PARAMS[paramPointer - 1];
        int rightType = BUFFER[++bufferPointer];
        int rightValue = BUFFER[++bufferPointer];

        if (rightType == -1) {
          print_statement(statement);
          printf("Cannot >= without right hand side\n");
          break;
        }
        switch (leftType + 3 * rightType) {
        case (INT + 3 * INT): {
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = leftValue >= rightValue;
          break;
        }
        case (INT + 3 * FLOAT): {
          float f = get_float(rightValue);
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = (float)leftValue >= f;
          break;
        }
        case (FLOAT + 3 * INT): {
          float f = get_float(leftValue);
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = f >= (float)rightValue;
          break;
        }
        case (FLOAT + 3 * FLOAT): {
          float f = get_float(leftValue);
          float f2 = get_float(rightValue);
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = f >= f2;
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

        int leftType = PARAMS[paramPointer - 2];
        int leftValue = PARAMS[paramPointer - 1];
        int rightType = BUFFER[++bufferPointer];
        int rightValue = BUFFER[++bufferPointer];

        if (rightType == -1) {
          print_statement(statement);
          printf("Cannot != without right hand side\n");
          break;
        }
        switch (leftType + 3 * rightType) {
        case (INT + 3 * INT): {
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = leftValue != rightValue;
          break;
        }
        case (INT + 3 * FLOAT): {
          float f = get_float(rightValue);
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = (float)leftValue != f;
          break;
        }
        case (FLOAT + 3 * INT): {
          float f = get_float(leftValue);
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = f != (float)rightValue;
          break;
        }
        case (FLOAT + 3 * FLOAT): {
          float f = get_float(leftValue);
          float f2 = get_float(rightValue);
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = f != f2;
          break;
        }
        case (INT + 3 * STRING): {
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = 1;
          break;
        }
        case (STRING + 3 * INT): {
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = 1;
          break;
        }
        case (STRING + 3 * STRING): {
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = leftValue != rightValue;
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

        int leftType = PARAMS[paramPointer - 2];
        int leftValue = PARAMS[paramPointer - 1];
        int rightType = BUFFER[++bufferPointer];
        int rightValue = BUFFER[++bufferPointer];

        if (rightType == -1) {
          print_statement(statement);
          printf("Cannot and without right hand side\n");
          break;
        }
        switch (leftType + 3 * rightType) {
        case (INT + 3 * INT): {
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = leftValue && rightValue;
          break;
        }
        case (INT + 3 * FLOAT): {
          float f = get_float(rightValue);
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = leftValue && f;
          break;
        }
        case (FLOAT + 3 * INT): {
          float f = get_float(leftValue);
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = f && rightValue;
          break;
        }
        case (FLOAT + 3 * FLOAT): {
          float f = get_float(leftValue);
          float f2 = get_float(rightValue);
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = f && f2;
          break;
        }
        case (INT + 3 * STRING): {
          PARAMS[paramPointer - 2] = INT;
          int l2 = strlen(get_string(rightValue));
          PARAMS[paramPointer - 1] = leftValue && l2;
          break;
        }
        case (STRING + 3 * INT): {
          PARAMS[paramPointer - 2] = INT;
          int l1 = strlen(get_string(leftValue));
          PARAMS[paramPointer - 1] = l1 && rightValue;
          break;
        }
        case (FLOAT + 3 * STRING): {
          PARAMS[paramPointer - 2] = INT;
          float f = get_float(leftValue);
          int l2 = strlen(get_string(rightValue));
          PARAMS[paramPointer - 1] = f && l2;
          break;
        }
        case (STRING + 3 * FLOAT): {
          PARAMS[paramPointer - 2] = INT;
          float f = get_float(rightValue);
          int l1 = strlen(get_string(leftValue));
          PARAMS[paramPointer - 1] = l1 && f;
          break;
        }
        case (STRING + 3 * STRING): {
          PARAMS[paramPointer - 2] = INT;
          int l1 = strlen(get_string(leftValue)),
              l2 = strlen(get_string(rightValue));
          PARAMS[paramPointer - 1] = l1 && l2;
          break;
        }
        case (INT + 3 * LIST): {
          PARAMS[paramPointer - 2] = INT;
          int l2 = len_list(rightValue);
          PARAMS[paramPointer - 1] = leftValue && l2;
          break;
        }
        case (LIST + 3 * INT): {
          PARAMS[paramPointer - 2] = INT;
          int l1 = len_list(leftValue);
          PARAMS[paramPointer - 1] = l1 && rightValue;
          break;
        }
        case (FLOAT + 3 * LIST): {
          PARAMS[paramPointer - 2] = INT;
          float f = get_float(leftValue);
          int l2 = len_list(rightValue);
          PARAMS[paramPointer - 1] = f && l2;
          break;
        }
        case (LIST + 3 * FLOAT): {
          PARAMS[paramPointer - 2] = INT;
          float f = get_float(rightValue);
          int l1 = len_list(leftValue);
          PARAMS[paramPointer - 1] = l1 && f;
          break;
        }
        case (STRING + 3 * LIST): {
          PARAMS[paramPointer - 2] = INT;
          int l1 = strlen(get_string(leftValue)), l2 = len_list(rightValue);
          PARAMS[paramPointer - 1] = l1 && l2;
          break;
        }
        case (LIST + 3 * STRING): {
          PARAMS[paramPointer - 2] = INT;
          int l1 = len_list(leftValue), l2 = strlen(get_string(rightValue));
          PARAMS[paramPointer - 1] = l1 && l2;
          break;
        }
        case (LIST + 3 * LIST): {
          PARAMS[paramPointer - 2] = INT;
          int l1 = len_list(leftValue), l2 = len_list(rightValue);
          PARAMS[paramPointer - 1] = l1 && l2;
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

        int leftType = PARAMS[paramPointer - 2];
        int leftValue = PARAMS[paramPointer - 1];
        int rightType = BUFFER[++bufferPointer];
        int rightValue = BUFFER[++bufferPointer];

        if (rightType == -1) {
          print_statement(statement);
          printf("Cannot or without right hand side\n");
          break;
        }
        switch (leftType + 3 * rightType) {
        case (INT + 3 * INT): {
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = leftValue || rightValue;
          break;
        }
        case (INT + 3 * FLOAT): {
          float f = get_float(rightValue);
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = leftValue || f;
          break;
        }
        case (FLOAT + 3 * INT): {
          float f = get_float(leftValue);
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = f || rightValue;
          break;
        }
        case (FLOAT + 3 * FLOAT): {
          float f = get_float(leftValue);
          float f2 = get_float(rightValue);
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = f || f2;
          break;
        }
        case (STRING + 3 * STRING): {
          PARAMS[paramPointer - 2] = INT;
          int l1 = strlen(get_string(leftValue)),
              l2 = strlen(get_string(rightValue));
          PARAMS[paramPointer - 1] = l1 || l2;
        }
        case (INT + 3 * LIST): {
          PARAMS[paramPointer - 2] = INT;
          int l2 = len_list(rightValue);
          PARAMS[paramPointer - 1] = leftValue || l2;
          break;
        }
        case (LIST + 3 * INT): {
          PARAMS[paramPointer - 2] = INT;
          int l1 = len_list(leftValue);
          PARAMS[paramPointer - 1] = l1 || rightValue;
          break;
        }
        case (FLOAT + 3 * LIST): {
          PARAMS[paramPointer - 2] = INT;
          float f = get_float(leftValue);
          int l2 = len_list(rightValue);
          PARAMS[paramPointer - 1] = f || l2;
          break;
        }
        case (LIST + 3 * FLOAT): {
          PARAMS[paramPointer - 2] = INT;
          float f = get_float(rightValue);
          int l1 = len_list(leftValue);
          PARAMS[paramPointer - 1] = l1 || f;
          break;
        }
        case (STRING + 3 * LIST): {
          PARAMS[paramPointer - 2] = INT;
          int l1 = strlen(get_string(leftValue)), l2 = len_list(rightValue);
          PARAMS[paramPointer - 1] = l1 || l2;
          break;
        }
        case (LIST + 3 * STRING): {
          PARAMS[paramPointer - 2] = INT;
          int l1 = len_list(leftValue), l2 = strlen(get_string(rightValue));
          PARAMS[paramPointer - 1] = l1 || l2;
          break;
        }
        case (LIST + 3 * LIST): {
          PARAMS[paramPointer - 2] = INT;
          int l1 = len_list(leftValue), l2 = len_list(rightValue);
          PARAMS[paramPointer - 1] = l1 || l2;
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
          char *s = get_string(rightValue);
          PARAMS[paramPointer++] = s[0] == '\0';
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

        int leftType = PARAMS[paramPointer - 2];
        int leftValue = PARAMS[paramPointer - 1];
        int rightType = BUFFER[++bufferPointer];
        int rightValue = BUFFER[++bufferPointer];

        if (rightType == -1) {
          print_statement(statement);
          printf("Cannot nor without right hand side\n");
          break;
        }
        switch (leftType + 3 * rightType) {
        case (INT + 3 * INT): {
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = leftValue && rightValue == 0;
          break;
        }
        case (INT + 3 * FLOAT): {
          float f = get_float(rightValue);
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = leftValue && f == 0;
          break;
        }
        case (FLOAT + 3 * INT): {
          float f = get_float(leftValue);
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = f && rightValue == 0;
          break;
        }
        case (FLOAT + 3 * FLOAT): {
          float f = get_float(leftValue);
          float f2 = get_float(rightValue);
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = f && f2 == 0;
          break;
        }
        case (STRING + 3 * STRING): {
          PARAMS[paramPointer - 2] = INT;
          int l1 = strlen(get_string(leftValue)),
              l2 = strlen(get_string(rightValue));
          PARAMS[paramPointer - 1] = l1 && l2 == 0;
        }
        case (STRING + 3 * INT): {
          PARAMS[paramPointer - 2] = INT;
          int l1 = strlen(get_string(leftValue));
          PARAMS[paramPointer - 1] = l1 && rightValue == 0;
          break;
        }
        case (INT + 3 * STRING): {
          PARAMS[paramPointer - 2] = INT;
          int l2 = strlen(get_string(rightValue));
          PARAMS[paramPointer - 1] = leftValue && l2 == 0;
          break;
        }
        case (STRING + 3 * FLOAT): {
          float f = get_float(rightValue);
          PARAMS[paramPointer - 2] = INT;
          int l1 = strlen(get_string(leftValue));
          PARAMS[paramPointer - 1] = l1 && f == 0;
          break;
        }
        case (FLOAT + 3 * STRING): {
          float f = get_float(leftValue);
          PARAMS[paramPointer - 2] = INT;
          int l2 = strlen(get_string(rightValue));
          PARAMS[paramPointer - 1] = f && l2 == 0;
          break;
        }
        case (LIST + 3 * LIST): {
          PARAMS[paramPointer - 2] = INT;
          int l1 = len_list(leftValue), l2 = len_list(rightValue);
          PARAMS[paramPointer - 1] = l1 && l2 == 0;
          break;
        }
        case (LIST + 3 * INT): {
          PARAMS[paramPointer - 2] = INT;
          int l1 = len_list(leftValue);
          PARAMS[paramPointer - 1] = l1 && rightValue == 0;
          break;
        }
        case (INT + 3 * LIST): {
          PARAMS[paramPointer - 2] = INT;
          int l2 = len_list(rightValue);
          PARAMS[paramPointer - 1] = leftValue && l2 == 0;
          break;
        }
        case (LIST + 3 * FLOAT): {
          float f = get_float(rightValue);
          PARAMS[paramPointer - 2] = INT;
          int l1 = len_list(leftValue);
          PARAMS[paramPointer - 1] = l1 && f == 0;
          break;
        }
        case (FLOAT + 3 * LIST): {
          float f = get_float(leftValue);
          PARAMS[paramPointer - 2] = INT;
          int l2 = len_list(rightValue);
          PARAMS[paramPointer - 1] = f && l2 == 0;
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

        int leftType = PARAMS[paramPointer - 2];
        int leftValue = PARAMS[paramPointer - 1];
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
          PARAMS[paramPointer - 2] = INT;
          PARAMS[paramPointer - 1] = i;
        } else {
          switch (leftType) {
          case INT: {
            char *s1 = int_to_string(leftValue), *s2 = get_string(rightValue);
            int i = strstr(s2, s1) != NULL;
            PARAMS[paramPointer - 2] = INT;
            PARAMS[paramPointer - 1] = i;
            free(s1);
            break;
          }
          case FLOAT: {
            float f = get_float(leftValue);
            char *s1 = float_to_string(f), *s2 = get_string(rightValue);
            int i = strstr(s2, s1) != NULL;
            PARAMS[paramPointer - 2] = INT;
            PARAMS[paramPointer - 1] = i;
            free(s1);
            break;
          }
          case STRING: {
            char *s1 = get_string(leftValue), *s2 = get_string(rightValue);
            int i = strstr(s2, s1) != NULL;
            PARAMS[paramPointer - 2] = INT;
            PARAMS[paramPointer - 1] = i;
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

        int leftType = PARAMS[paramPointer - 2];
        int leftValue = PARAMS[paramPointer - 1];
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
            PARAMS[paramPointer - 2] = ln->type;
            PARAMS[paramPointer - 1] = ln->value;
          }
        } else {
          char *s = get_string(leftValue);
          int len = strlen(s);
          if (rightValue < len) {
            char *c = malloc(sizeof(char) * 2);
            c[0] = s[rightValue];
            c[1] = '\0';
            PARAMS[paramPointer - 2] = STRING;
            PARAMS[paramPointer - 1] = add_string(c, 0);
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
          PARAMS[paramPointer++] = (int)f;
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
          float a = aType == INT ? (float)aValue : get_float(aValue);
          float b = bType == INT ? (float)bValue : get_float(bValue);
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
          float a = aType == INT ? (float)aValue : get_float(aValue);
          float b = bType == INT ? (float)bValue : get_float(bValue);
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
            if (ln == NULL)
              break;
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
            if (i + len2 > len)
              break;
            int j = 0;
            for (; j < len2; j++) {
              if (s[i + j] != s2[j])
                break;
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
          float rightFloat = get_float(rightValue);
          int rightInt = (int)rightFloat;
          float absFloat = (float)abs(rightInt);
          PARAMS[paramPointer++] = push_float(absFloat);
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

        if (rightType != INT && rightType != FLOAT) {
          print_statement(statement);
          printf("Cannot range with non-number\n");
          break;
        }
        if (rightType == FLOAT) {
          rightValue = (int)get_float(rightValue);
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

        PARAMS[paramPointer++] = INT;
        PARAMS[paramPointer++] = world_has(world, rightValue);
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
      case SIN: {
        int rightType = BUFFER[++bufferPointer];
        int rightValue = BUFFER[++bufferPointer];

        if (rightType != INT && rightType != FLOAT) {
          print_statement(statement);
          printf("Cannot sin with non-number\n");
          break;
        }
        if (rightType == FLOAT) {
          PARAMS[paramPointer++] = FLOAT;
          PARAMS[paramPointer++] = push_float(sin(get_float(rightValue)));
        } else {
          PARAMS[paramPointer++] = FLOAT;
          PARAMS[paramPointer++] = push_float(sin(rightValue));
        }
        break;
      }
      case COS: {
        int rightType = BUFFER[++bufferPointer];
        int rightValue = BUFFER[++bufferPointer];

        if (rightType != INT && rightType != FLOAT) {
          print_statement(statement);
          printf("Cannot cos with non-number\n");
          break;
        }
        if (rightType == FLOAT) {
          PARAMS[paramPointer++] = FLOAT;
          PARAMS[paramPointer++] = push_float(cos(get_float(rightValue)));
        } else {
          PARAMS[paramPointer++] = FLOAT;
          PARAMS[paramPointer++] = push_float(cos(rightValue));
        }
        break;
      }
      case TAN: {
        int rightType = BUFFER[++bufferPointer];
        int rightValue = BUFFER[++bufferPointer];

        if (rightType != INT && rightType != FLOAT) {
          print_statement(statement);
          printf("Cannot tan with non-number\n");
          break;
        }
        if (rightType == FLOAT) {
          PARAMS[paramPointer++] = FLOAT;
          PARAMS[paramPointer++] = push_float(tan(get_float(rightValue)));
        } else {
          PARAMS[paramPointer++] = FLOAT;
          PARAMS[paramPointer++] = push_float(tan(rightValue));
        }
        break;
      }
      case ASIN: {
        int rightType = BUFFER[++bufferPointer];
        int rightValue = BUFFER[++bufferPointer];

        if (rightType != INT && rightType != FLOAT) {
          print_statement(statement);
          printf("Cannot asin with non-number\n");
          break;
        }
        if (rightType == FLOAT) {
          PARAMS[paramPointer++] = FLOAT;
          PARAMS[paramPointer++] = push_float(asin(get_float(rightValue)));
        } else {
          PARAMS[paramPointer++] = FLOAT;
          PARAMS[paramPointer++] = push_float(asin(rightValue));
        }
        break;
      }
      case ACOS: {
        int rightType = BUFFER[++bufferPointer];
        int rightValue = BUFFER[++bufferPointer];

        if (rightType != INT && rightType != FLOAT) {
          print_statement(statement);
          printf("Cannot acos with non-number\n");
          break;
        }
        if (rightType == FLOAT) {
          PARAMS[paramPointer++] = FLOAT;
          PARAMS[paramPointer++] = push_float(acos(get_float(rightValue)));
        } else {
          PARAMS[paramPointer++] = FLOAT;
          PARAMS[paramPointer++] = push_float(acos(rightValue));
        }
        break;
      }
      case ATAN: {
        int rightType = BUFFER[++bufferPointer];
        int rightValue = BUFFER[++bufferPointer];

        if (rightType != INT && rightType != FLOAT) {
          print_statement(statement);
          printf("Cannot atan with non-number\n");
          break;
        }
        if (rightType == FLOAT) {
          PARAMS[paramPointer++] = FLOAT;
          PARAMS[paramPointer++] = push_float(atan(get_float(rightValue)));
        } else {
          PARAMS[paramPointer++] = FLOAT;
          PARAMS[paramPointer++] = push_float(atan(rightValue));
        }
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

int resolve_script(int scriptIdx, Actor *self, Actor *related, World *world,
                   int debug, int eject, int keyTypes[], int keyValues[],
                   int replacerTypes[], int replacerValues[],
                   int replacerCount) {
  int executionPointer = scriptIdx;
  int ifNested = 0;
  while (SCRIPTS[executionPointer] != -2000) {
    if (eject > 0 && eject <= executionPointer) {
      return 0;
    }

    if (debug == 2) {
      printf("Resolving for self (%i) %s\n", self->name,
             get_string(self->name));
      print_statement(executionPointer);
    }
    int verb = SCRIPTS[executionPointer];
    if (ifNested) {
      if (verb == IF)
        ifNested++;
      if (verb == ENDIF)
        ifNested--;
      while (SCRIPTS[++executionPointer] != -1000) {
        if ( // special cases, dont have data to increment over
            SCRIPTS[executionPointer] != DOT &&
            SCRIPTS[executionPointer] != NONE &&
            SCRIPTS[executionPointer] != QRAND &&
            SCRIPTS[executionPointer] != QWORLD &&
            SCRIPTS[executionPointer] != QCOLLIDE &&
            SCRIPTS[executionPointer] != QSTICKS &&
            SCRIPTS[executionPointer] != QSONG &&
            SCRIPTS[executionPointer] != INP_A &&
            SCRIPTS[executionPointer] != INP_B &&
            SCRIPTS[executionPointer] != INP_X &&
            SCRIPTS[executionPointer] != INP_Y &&
            SCRIPTS[executionPointer] != INP_START &&
            SCRIPTS[executionPointer] != INP_LEFT &&
            SCRIPTS[executionPointer] != INP_RIGHT &&
            SCRIPTS[executionPointer] != INP_UP &&
            SCRIPTS[executionPointer] != INP_DOWN &&
            SCRIPTS[executionPointer] != INP_EVENTS)
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
    if (debug == 2)
      printf("Evaluating Literals...\n");
    while (SCRIPTS[executionPointer] != -1000) {
      // evaluate literals
      executionPointer++;
      int type = SCRIPTS[executionPointer];

      switch (type) {
      case INT:
      case STRING:
      case OPERATOR: {
        int found = 0;
        for (int i = 0; i < replacerCount; i++) {
          int keyType = keyTypes[i];
          int keyValue = keyValues[i];
          int replacerType = replacerTypes[i];
          int replacerValue = replacerValues[i];
          if (SCRIPTS[executionPointer] == keyType &&
              SCRIPTS[executionPointer + 1] == keyValue) {
            found = 1;
            BUFFER[bufferPointer++] = replacerType;
            BUFFER[bufferPointer++] = replacerValue;
            executionPointer++;
            break;
          }
        }
        if (found)
          break;
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
        BUFFER[bufferPointer++] = NONE;
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
      case QSTICKS: {
        BUFFER[bufferPointer++] = INT;
        BUFFER[bufferPointer++] = get_num_sticks();
        break;
      }
      case QCOLLIDE: {
        int list = add_list();
        BUFFER[bufferPointer++] = LIST;
        BUFFER[bufferPointer++] = list;
        for (int i = 0; i < WORLD_BUFFER_SIZE; i++) {
          if (world->actors[i] == -1)
            break;
          if (world->actors[i] == self->name)
            continue;
          Actor *a = get_actor(world->actors[i]);
          if (a == NULL)
            continue;
          if (a->tangible == 0)
            continue;
          if (SDL_HasIntersection(&self->ECB, &a->ECB)) {
            add_to_list(list, STRING, a->name);
          }
        }
        break;
      }
      case QSONG: {
        BUFFER[bufferPointer++] = STRING;
        BUFFER[bufferPointer++] = get_song();
        break;
      }
      case DOT: {
        if (bufferPointer < 2) {
          printf("Actor %s error: ", get_string(self->name));
          print_statement(statement);
          printf("Could not . with no left hand side\n");
          break;
        }
        int leftType = BUFFER[bufferPointer - 2];
        int leftValue = BUFFER[bufferPointer - 1];
        int rightType = SCRIPTS[++executionPointer];
        int rightValue = SCRIPTS[++executionPointer];

        if (rightType != STRING || leftType != STRING) {
          printf("Actor %s error: ", get_string(self->name));
          print_statement(statement);
          printf("Dot must be string . string, got %i . %i\n", leftType,
                 rightType);
          break;
        }

        Actor *a;

        if (leftValue == SELF)
          a = self;
        else if (leftValue == RELATED)
          a = related;
        else
          a = get_actor(leftValue);

        if (a == NULL) {
          printf("Actor %s error: ", get_string(self->name));
          print_statement(statement);
          printf("Could not find actor for dot\n");
          break;
        }
        switch (rightValue) {
        case NAME:
          BUFFER[bufferPointer - 2] = STRING;
          BUFFER[bufferPointer - 1] = a->name;
          break;
        case STATE:
          BUFFER[bufferPointer - 2] = STRING;
          BUFFER[bufferPointer - 1] = a->state;
          break;
        case FRAME:
          BUFFER[bufferPointer - 2] = INT;
          BUFFER[bufferPointer - 1] = a->frame;
          break;
        case _X:
          BUFFER[bufferPointer - 2] = INT;
          BUFFER[bufferPointer - 1] = a->ECB.x;
          break;
        case _Y:
          BUFFER[bufferPointer - 2] = INT;
          BUFFER[bufferPointer - 1] = a->ECB.y;
          break;
        case _WIDTH:
        case W:
          BUFFER[bufferPointer - 2] = INT;
          BUFFER[bufferPointer - 1] = a->ECB.w;
          break;
        case _HEIGHT:
        case H:
          BUFFER[bufferPointer - 2] = INT;
          BUFFER[bufferPointer - 1] = a->ECB.h;
          break;
        case TOP:
          BUFFER[bufferPointer - 2] = INT;
          BUFFER[bufferPointer - 1] = a->ECB.y;
          break;
        case _LEFT:
          BUFFER[bufferPointer - 2] = INT;
          BUFFER[bufferPointer - 1] = a->ECB.x;
          break;
        case BOTTOM:
          BUFFER[bufferPointer - 2] = INT;
          BUFFER[bufferPointer - 1] = a->ECB.y + a->ECB.h;
          break;
        case _RIGHT:
          BUFFER[bufferPointer - 2] = INT;
          BUFFER[bufferPointer - 1] = a->ECB.x + a->ECB.w;
          break;
        case DIRECTION:
          BUFFER[bufferPointer - 2] = INT;
          BUFFER[bufferPointer - 1] = a->direction;
          break;
        case PLATFORM:
          BUFFER[bufferPointer - 2] = INT;
          BUFFER[bufferPointer - 1] = a->platform;
          break;
        case ROTATION:
          BUFFER[bufferPointer - 2] = INT;
          BUFFER[bufferPointer - 1] = a->rotation;
          break;
        case TANGIBLE:
          BUFFER[bufferPointer - 2] = INT;
          BUFFER[bufferPointer - 1] = a->tangible;
          break;
        case PHYSICS:
          BUFFER[bufferPointer - 2] = INT;
          BUFFER[bufferPointer - 1] = a->physics;
          break;
        case X_VEL:
          BUFFER[bufferPointer - 2] = FLOAT;
          BUFFER[bufferPointer - 1] = push_float(a->x_vel);
          break;
        case Y_VEL:
          BUFFER[bufferPointer - 2] = FLOAT;
          BUFFER[bufferPointer - 1] = push_float(a->y_vel);
          break;
        case _INPUT_NAME:
          BUFFER[bufferPointer - 2] = STRING;
          BUFFER[bufferPointer - 1] = a->_input_name;
          break;
        default: {
          Attribute *attr;
          HASH_FIND_INT(a->attributes, &rightValue, attr);
          if (attr == NULL) {
            BUFFER[bufferPointer - 2] = INT;
            BUFFER[bufferPointer - 1] = 0;
          } else {
            BUFFER[bufferPointer - 2] = attr->type;
            BUFFER[bufferPointer - 1] =
                attr->type == FLOAT ? push_float(attr->value.f) : attr->value.i;
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

        if (is == NULL)
          break;
        if (is->EVENTS[A_DOWN])
          add_to_list(list, STRING, _A_DOWN);
        if (is->EVENTS[A_UP])
          add_to_list(list, STRING, _A_UP);
        if (is->EVENTS[B_DOWN])
          add_to_list(list, STRING, _B_DOWN);
        if (is->EVENTS[B_UP])
          add_to_list(list, STRING, _B_UP);
        if (is->EVENTS[X_DOWN])
          add_to_list(list, STRING, _X_DOWN);
        if (is->EVENTS[X_UP])
          add_to_list(list, STRING, _X_UP);
        if (is->EVENTS[Y_DOWN])
          add_to_list(list, STRING, _Y_DOWN);
        if (is->EVENTS[Y_UP])
          add_to_list(list, STRING, _Y_UP);
        if (is->EVENTS[LEFT_DOWN])
          add_to_list(list, STRING, _LEFT_DOWN);
        if (is->EVENTS[LEFT_UP])
          add_to_list(list, STRING, _LEFT_UP);
        if (is->EVENTS[UP_DOWN])
          add_to_list(list, STRING, _UP_DOWN);
        if (is->EVENTS[UP_UP])
          add_to_list(list, STRING, _UP_UP);
        if (is->EVENTS[RIGHT_DOWN])
          add_to_list(list, STRING, _RIGHT_DOWN);
        if (is->EVENTS[RIGHT_UP])
          add_to_list(list, STRING, _RIGHT_UP);
        if (is->EVENTS[DOWN_DOWN])
          add_to_list(list, STRING, _DOWN_DOWN);
        if (is->EVENTS[DOWN_UP])
          add_to_list(list, STRING, _DOWN_UP);
        if (is->EVENTS[START_DOWN])
          add_to_list(list, STRING, _START_DOWN);
        if (is->EVENTS[START_UP])
          add_to_list(list, STRING, _START_UP);
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
    if (debug == 2)
      printf("  Done...\n");
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
      remove_actor_from_frames(self->name);
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
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("Missing or Incorrect Parameter for SET\n");
        break;
      }

      Actor *a;
      if (nameValue == SELF)
        a = self;
      else if (nameValue == RELATED)
        a = related;
      else
        a = get_actor(nameValue);
      if (a == NULL) {
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("Could not find actor %s, %i for SET (is self %i? %i)\n",
               get_string(nameValue), nameValue, SELF, nameValue == SELF);
        break;
      }
      switch (attrValue) {
      case NAME:
        if (valueType != STRING)
          break;
        a->name = valueValue;
        break;
      case STATE:
        if (valueType != STRING)
          break;
        a->state = valueValue;
        break;
      case FRAME:
        if (valueType != INT)
          break;
        a->frame = valueValue;
        break;
      case _X:
        switch (valueType) {
        case INT:
          a->ECB.x = valueValue;
          break;
        case FLOAT:
          a->ECB.x = (int)get_float(valueValue);
          break;
        }
        break;
      case _Y:
        switch (valueType) {
        case INT:
          a->ECB.y = valueValue;
          break;
        case FLOAT:
          a->ECB.y = (int)get_float(valueValue);
          break;
        }
        break;
      case _WIDTH:
      case W:
        switch (valueType) {
        case INT:
          a->ECB.w = valueValue;
          break;
        case FLOAT:
          a->ECB.w = (int)get_float(valueValue);
          break;
        }
        break;
      case _HEIGHT:
      case H:
        switch (valueType) {
        case INT:
          a->ECB.h = valueValue;
          break;
        case FLOAT:
          a->ECB.h = (int)get_float(valueValue);
          break;
        }
        break;
      case TOP:
        switch (valueType) {
        case INT:
          a->ECB.y = valueValue;
          break;
        case FLOAT:
          a->ECB.y = (int)get_float(valueValue);
          break;
        }
        break;
      case _LEFT:
        switch (valueType) {
        case INT:
          a->ECB.x = valueValue;
          break;
        case FLOAT:
          a->ECB.x = (int)get_float(valueValue);
          break;
        }
        break;
      case BOTTOM:
        switch (valueType) {
        case INT:
          a->ECB.y = valueValue - a->ECB.h;
          break;
        case FLOAT:
          a->ECB.y = (int)get_float(valueValue) - a->ECB.h;
          break;
        }
        break;
      case _RIGHT:
        switch (valueType) {
        case INT:
          a->ECB.x = valueValue - a->ECB.w;
          break;
        case FLOAT:
          a->ECB.x = (int)get_float(valueValue) - a->ECB.w;
          break;
        }
        break;
      case DIRECTION:
        if (valueType != INT)
          break;
        a->direction = valueValue;
        break;
      case PLATFORM:
        if (valueType != INT)
          break;
        a->platform = valueValue;
        break;
      case ROTATION:
        if (valueType != INT)
          break;
        a->rotation = valueValue;
        break;
      case TANGIBLE:
        if (valueType != INT)
          break;
        a->tangible = valueValue;
        break;
      case PHYSICS:
        if (valueType != INT)
          break;
        a->physics = valueValue;
        break;
      case X_VEL:
        if (valueType != FLOAT && valueType != INT)
          break;
        if (valueType == FLOAT) {
          a->x_vel = get_float(valueValue);
        } else if (valueType == INT)
          a->x_vel = valueValue;
        break;
      case Y_VEL:
        if (valueType != FLOAT && valueType != INT)
          break;
        if (valueType == FLOAT)
          a->y_vel = get_float(valueValue);
        else if (valueType == INT)
          a->y_vel = (float)valueValue;
        break;
      case _INPUT_NAME:
        if (valueType != STRING)
          break;
        a->_input_name = valueValue;
        break;
      case BACKGROUND:
        if (valueType != INT)
          break;
        a->background = valueValue;
        break;
      default: {
        Attribute *attr;
        HASH_FIND_INT(a->attributes, &attrValue, attr);
        if (attr == NULL) {
          attr = malloc(sizeof(Attribute));
          attr->name = attrValue;
          HASH_ADD_INT(a->attributes, name, attr);
        }
        if (attr->type == LIST)
          remove_owner(attr->value.i);
        attr->type = valueType;
        if (attr->type == FLOAT)
          attr->value.f = get_float(valueValue);
        else
          attr->value.i = valueValue;
        if (attr->type == LIST)
          add_owner(attr->value.i);
      }
      }
      break;
    }
    case REASSIGN: {
      int actorType = PARAMS[0];
      int actorValue = PARAMS[1];
      int oldKeyType = PARAMS[2];
      int oldKeyValue = PARAMS[3];
      int newKeyType = PARAMS[4];
      int newKeyValue = PARAMS[5];

      if (actorType != STRING || oldKeyType != STRING || newKeyType != STRING) {
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("Missing or Incorrect Parameter for REASSIGN\n");
        break;
      }
      Actor *actor = NULL;
      if (actorValue == SELF)
        actor = self;
      else if (actorValue == RELATED)
        actor = related;
      else
        actor = get_actor(actorValue);

      if (actor == NULL) {
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("Could not find actor %s for REASSIGN\n",
               get_string(actorValue));
        break;
      }

      int oldFrame = -1;
      int oldState = oldKeyValue;
      int newFrame = -1;
      int newState = newKeyValue;

      char *delim = ":";

      char *_string = get_string(oldKeyValue);
      char *scriptValueStr = malloc(strlen(_string) + 1);
      strcpy(scriptValueStr, _string);

      char *oldStateStr = strtok(scriptValueStr, delim);
      char *oldFrameStr = strtok(NULL, delim);

      if (oldFrameStr != NULL) {
        oldState = index_string(oldStateStr);
        oldFrame = atoi(oldFrameStr);
      }
      free(scriptValueStr);

      _string = get_string(newKeyValue);
      scriptValueStr = malloc(strlen(_string) + 1);
      strcpy(scriptValueStr, _string);

      char *newStateStr = strtok(scriptValueStr, delim);
      char *newFrameStr = strtok(NULL, delim);

      if (newFrameStr != NULL) {
        newState = index_string(newStateStr);
        newFrame = atoi(newFrameStr);
      }
      free(scriptValueStr);

      int scriptKey = find_script_from_map(actor, oldState, oldFrame);
      if (scriptKey == -1) {
        break;
      }

      // ensure the new script is not already in the map
      pop_from_script_map(actor, newState, newFrame);

      int i = 0;
      while (i < LARGEST_SCRIPT_MAP) {
        if (actor->scriptmap[i] == -1) {
          return -1;
        }
        int state = actor->scriptmap[i++];
        int frame = actor->scriptmap[i++];
        int idx = actor->scriptmap[i++];
        if (state == oldState && frame == oldFrame) {
          actor->scriptmap[i - 3] = newState;
          actor->scriptmap[i - 2] = newFrame;
          break;
        }
      }
      break;
    }
    case IF: {
      int conditionalType = PARAMS[0];
      int conditionalValue = PARAMS[1];

      int conditional = 0;
      switch (conditionalType) {
      case INT: {
        if (conditionalValue != 0)
          conditional = 1;
        break;
      }
      case FLOAT: {
        if (get_float(conditionalValue) != 0)
          conditional = 1;
        break;
      }
      case STRING: {
        if (conditionalValue != EMPTY)
          conditional = 1;
        break;
      }
      case LIST: {
        if (len_list(conditionalValue) != 0)
          conditional = 1;
        break;
      }
      }
      if (!conditional)
        ifNested = 1;

      break;
    }
    case EXEC: {
      int scriptType = PARAMS[0];
      int scriptValue = PARAMS[1];

      if (scriptType != STRING) {
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("Missing or Incorrect Parameter for EXEC\n");
        break;
      }
      int frame = -1;
      int state = scriptValue;

      char *_string = get_string(scriptValue);
      char *scriptValueStr = malloc(strlen(_string) + 1);
      strcpy(scriptValueStr, _string);

      char *delim = ":";

      char *stateStr = strtok(scriptValueStr, delim);
      char *frameStr = strtok(NULL, delim);

      if (frameStr != NULL) {
        state = index_string(stateStr);
        frame = atoi(frameStr);
      }

      int script = find_script_from_map(self, state, frame);
      free(scriptValueStr);
      if (script != -1) {
        int resolution = resolve_script(script, self, related, world, debug, -1,
                                        0, 0, 0, 0, 0);
        if (resolution < 0)
          return resolution;
      }
      break;
    }
    case BACK: {
      remove_actor_from_world(world, self->name);
      int i;
      for (i = 0; i < WORLD_BUFFER_SIZE; i++) {
        if (world->actors[i] == -1) {
          break;
        }
        Actor *a = get_actor(world->actors[i]);
        if (a->background) {
          continue;
        }
        for (int j = WORLD_BUFFER_SIZE - 1; j > i; j--) {
          world->actors[j] = world->actors[j - 1];
        }
        break;
      }
      world->actors[i] = self->name;
      break;
    }
    case FRONT: {
      remove_actor_from_world(world, self->name);
      add_actor_to_world(world->name, self->name);
      break;
    }
    case IMG: {
      int imgType = PARAMS[0];
      int imgValue = PARAMS[1];

      if (imgType != STRING) {
        printf("Actor %s error: ", get_string(self->name));
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
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("Missing or Incorrect Parameter for ACTIVATE\n");
        break;
      }

      Frame *f = get_frame(frameValue);
      if (f == NULL) {
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("Frame %s does not exist for ACTIVATE\n",
               get_string(frameValue));
        break;
      }

      f->active = 1;
      break;
    }
    case DEACTIVATE: {
      int frameType = PARAMS[0];
      int frameValue = PARAMS[1];

      if (frameType != STRING) {
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("Missing or Incorrect Parameter for ACTIVATE\n");
        break;
      }

      Frame *f = get_frame(frameValue);
      if (f == NULL) {
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("Frame %s does not exist for DEACTIVATE\n",
               get_string(frameValue));
        break;
      }

      f->active = 0;
      break;
    }
    case KILLFRAME: {
      int frameType = PARAMS[0];
      int frameValue = PARAMS[1];

      if (frameType != STRING) {
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("Missing or Incorrect Parameter for KILLFRAME\n");
        break;
      }

      Frame *f = get_frame(frameValue);
      if (f == NULL) {
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("Frame %s does not exist for KILLFRAME\n",
               get_string(frameValue));
        break;
      }

      delete_frame(f);
      break;
    }
    case MAKEFRAME: {
      int nameType = PARAMS[0];
      int nameValue = PARAMS[1];
      int worldType = PARAMS[2];
      int worldValue = PARAMS[3];
      int xType = PARAMS[4];
      int xValue = PARAMS[5];
      int yType = PARAMS[6];
      int yValue = PARAMS[7];
      int wType = PARAMS[8];
      int wValue = PARAMS[9];
      int hType = PARAMS[10];
      int hValue = PARAMS[11];

      if (nameType != STRING || worldType != STRING || xType != INT ||
          yType != INT || wType != INT || hType != INT) {
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("Missing or Incorrect Parameter for MAKEFRAME\n");
        break;
      }

      World *w = get_world(worldValue);
      if (w == NULL) {
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("World %s not found for MAKEFRAME\n", get_string(worldValue));
        break;
      }

      add_frame(nameValue, w, NULL, xValue, yValue, wValue, hValue);
      break;
    }
    case FOCUS: {
      int frameType = PARAMS[0];
      int frameValue = PARAMS[1];
      int nameType = PARAMS[2];
      int nameValue = PARAMS[3];

      if (frameType != STRING || nameType != STRING) {
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("Missing or Incorrect Parameter for FOCUS\n");
        break;
      }

      Frame *f = get_frame(frameValue);
      if (f == NULL) {
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("Frame %s not found for FOCUS\n", get_string(frameValue));
        break;
      }

      Actor *a = NULL;
      if (nameValue == SELF)
        a = self;
      else if (nameValue == RELATED)
        a = related;
      else
        a = get_actor(nameValue);
      if (a == NULL) {
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("Actor %s not found for FOCUS\n", get_string(nameValue));
        break;
      }
      f->focus = a->name;

      break;
    }
    case SCROLLBOUND: {
      int frameType = PARAMS[0];
      int frameValue = PARAMS[1];
      int directionType = PARAMS[2];
      int directionValue = PARAMS[3];
      int valueType = PARAMS[4];
      int valueValue = PARAMS[5];

      if (frameType != STRING || directionType != STRING ||
          (valueType != INT && valueType != NONE)) {
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("Missing or Incorrect Parameter for SCROLLBOUND\n");
        break;
      }

      Frame *f = get_frame(frameValue);
      if (f == NULL) {
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("Frame %s not found for SCROLLBOUND\n", get_string(frameValue));
        break;
      }

      switch (directionValue) {
      case TOP:
        f->bound_top = valueType != NONE;
        f->top_bind = valueValue;
        break;
      case _LEFT:
        f->bound_left = valueType != NONE;
        f->left_bind = valueValue;
        break;
      case BOTTOM:
        f->bound_bottom = valueType != NONE;
        f->bottom_bind = valueValue;
        break;
      case _RIGHT:
        f->bound_right = valueType != NONE;
        f->right_bind = valueValue;
        break;
      }
      break;
    }
    case VIEW: {
      int frameType = PARAMS[0];
      int frameValue = PARAMS[1];
      int worldType = PARAMS[2];
      int worldValue = PARAMS[3];

      if (frameType != STRING || worldType != STRING) {
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("Missing or Incorrect Parameter for VIEW\n");
        break;
      }

      Frame *f = get_frame(frameValue);
      if (f == NULL) {
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("Frame %s not found for VIEW\n", get_string(frameValue));
        break;
      }

      World *w = get_world(worldValue);
      if (w == NULL) {
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("World %s not found for VIEW\n", get_string(worldValue));
        break;
      }

      f->world = w;
      break;
    }
    case MOVE: {
      int nameType = PARAMS[0];
      int nameValue = PARAMS[1];
      int worldType = PARAMS[2];
      int worldValue = PARAMS[3];

      if (nameType != STRING || worldType != STRING) {
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("Missing or Incorrect Parameter for MOVE\n");
        break;
      }

      if (nameValue == SELF) {
        nameValue = self->name;
      } else if (nameValue == RELATED) {
        nameValue = related->name;
      }

      if (remove_actor_from_world(world, nameValue) != 1) {
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("Actor %s not found for MOVE\n", get_string(nameValue));
        break;
      }

      add_actor_to_world(worldValue, nameValue);

      break;
    }
    case PLACE: {
      int nameType = PARAMS[0];
      int nameValue = PARAMS[1];
      int worldType = PARAMS[2];
      int worldValue = PARAMS[3];

      if (nameType != STRING || worldType != STRING) {
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("Missing or Incorrect Parameter for PLACE\n");
        break;
      }

      Actor *a = NULL;
      if (nameValue == SELF) {
        a = self;
      } else if (nameValue == RELATED) {
        a = related;
      } else {
        a = get_actor(nameValue);
      }
      if (a == NULL) {
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("Actor %s not found for PLACE\n", get_string(nameValue));
        break;
      }
      World *w = get_world(worldValue);
      if (w == NULL) {
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("World %s not found for PLACE\n", get_string(worldValue));
        break;
      }
      if (world_has(w, a->name) == 0) {
        add_actor_to_world(worldValue, a->name);
      }
      break;
    }
    case TAKE: {
      int worldType = PARAMS[0];
      int worldValue = PARAMS[1];
      int nameType = PARAMS[2];
      int nameValue = PARAMS[3];

      if (worldType != STRING || nameType != STRING) {
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("Missing or Incorrect Parameter for TAKE\n");
        break;
      }

      Actor *a = NULL;
      if (nameValue == SELF) {
        a = self;
      } else if (nameValue == RELATED) {
        a = related;
      } else {
        a = get_actor(nameValue);
      }
      if (a == NULL) {
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("Actor %s not found for TAKE\n", get_string(nameValue));
        break;
      }
      World *w = get_world(worldValue);
      if (w == NULL) {
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("World %s not found for TAKE\n", get_string(worldValue));
        break;
      }
      if (world_has(w, a->name) == 1) {
        remove_actor_from_world(w, a->name);
      }
      break;
    }
    case TAKEALL: {
      int nameType = PARAMS[0];
      int nameValue = PARAMS[1];

      if (nameType != STRING) {
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("Missing or Incorrect Parameter for TAKEALL\n");
        break;
      }

      if (nameValue == SELF) {
        nameValue = self->name;
      } else if (nameValue == RELATED) {
        nameValue = related->name;
      }

      remove_actor_from_worlds(nameValue);
      break;
    }
    case REBRAND: {
      int actorType = PARAMS[0];
      int actorValue = PARAMS[1];

      if (actorType != STRING) {
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("Missing or Incorrect Parameter for REBRAND\n");
        break;
      }

      load_script_map_into_actor(self, actorValue);
      self->spritemapkey = actorValue;

      int scriptKey = find_script_from_map(self, _START, 0);
      if (scriptKey != -1) {
        int resolution = resolve_script(scriptKey, self, NULL, world, debug, -1,
                                        0, 0, 0, 0, 0);
        if (resolution < 0)
          return resolution;
      }
      break;
    }
    case REMOVE: {
      int listType = PARAMS[0];
      int listValue = PARAMS[1];
      int valueType = PARAMS[2];
      int valueValue = PARAMS[3];
      if (listType != LIST) {
        printf("Actor %s error: ", get_string(self->name));
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
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("Missing or Incorrect Parameter for ADD\n");
        break;
      }
      add_to_list(listValue, valueType, valueValue);
      break;
    }
    case HITBOXES: {
      int hitboxType = PARAMS[0];
      int hitboxValue = PARAMS[1];

      if (hitboxType != STRING) {
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("Missing or Incorrect Parameter for HITBOXES\n");
        break;
      }

      self->hitboxkey = hitboxValue;
      break;
    }
    case HURTBOXES: {
      int hurtboxType = PARAMS[0];
      int hurtboxValue = PARAMS[1];

      if (hurtboxType != STRING) {
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("Missing or Incorrect Parameter for HURTBOXES\n");
        break;
      }

      self->hurtboxkey = hurtboxValue;
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

      if (templateNameType != STRING || nameType != STRING ||
          (xType != INT && xType != FLOAT) ||
          (yType != INT && yType != FLOAT)) {
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("Missing or Incorrect Parameter for CREATE\n");
        break;
      }
      if (xType == FLOAT)
        xValue = (int)get_float(xValue);
      if (yType == FLOAT)
        yValue = (int)get_float(yValue);

      Actor *a;
      a = get_actor(nameValue);
      if (a != NULL) {
        remove_actor_from_worlds(a->name);
        remove_actor_from_frames(a->name);
        free_actor(a);
        clear_ownerless_lists();
      }

      a = add_actor_from_templatekey(templateNameValue, nameValue);
      a->ECB.x = xValue;
      a->ECB.y = yValue;

      add_actor_to_world(world->name, nameValue);

      int script = get_script_for_actor(a);
      if (script != -1) {
        int resolution =
            resolve_script(script, a, NULL, world, debug, -1, 0, 0, 0, 0, 0);
        if (resolution < 0)
          return resolution;
      }

      a->updated = 1;
      break;
    }
    case UPDATE: {
      int worldType = PARAMS[0];
      int worldValue = PARAMS[1];

      if (worldType != STRING) {
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("Missing or Incorrect Parameter for UPDATE\n");
        break;
      }

      World *w = get_world(worldValue);
      if (w == NULL) {
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("World %s not found for UPDATE\n", get_string(worldValue));
        break;
      }

      w->flagged_for_update = 1;
      break;
    }
    case SFX: {
      int sfxType = PARAMS[0];
      int sfxValue = PARAMS[1];

      if (sfxType != STRING) {
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("Missing or Incorrect Parameter for SFX\n");
        break;
      }

      play_sound(sfxValue);
      break;
    }
    case SONG: {
      int songType = PARAMS[0];
      int songValue = PARAMS[1];

      if (songType != STRING) {
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("Missing or Incorrect Parameter for SONG\n");
        break;
      }

      play_song(songValue);
      break;
    }
    case SFXOFF: {
      disable_sfx();
      break;
    }
    case SONGOFF: {
      disable_music();
      break;
    }
    case OFFSETBGSCROLLX: {
      int worldType = PARAMS[0];
      int worldValue = PARAMS[1];
      int valueType = PARAMS[2];
      int valueValue = PARAMS[3];

      if (worldType != STRING || valueType != INT) {
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("Missing or Incorrect Parameter for OFFSETBGSCROLLX\n");
        break;
      }

      World *w = get_world(worldValue);
      if (w == NULL) {
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("World %s not found for OFFSETBGSCROLLX\n",
               get_string(worldValue));
        break;
      }
      w->background_x_scroll += valueValue;
      break;
    }
    case OFFSETBGSCROLLY: {
      int worldType = PARAMS[0];
      int worldValue = PARAMS[1];
      int valueType = PARAMS[2];
      int valueValue = PARAMS[3];

      if (worldType != STRING || valueType != INT) {
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("Missing or Incorrect Parameter for OFFSETBGSCROLLY\n");
        break;
      }

      World *w = get_world(worldValue);
      if (w == NULL) {
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("World %s not found for OFFSETBGSCROLLY\n",
               get_string(worldValue));
        break;
      }
      w->background_y_scroll += valueValue;
      break;
    }
    case FOR: {
      int keyType = PARAMS[0];
      int keyValue = PARAMS[1];
      int iterType = PARAMS[2];
      int iterValue = PARAMS[3];

      if (keyType != STRING || (iterType != STRING && iterType != LIST)) {
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("Missing or Incorrect Parameter for FOR\n");
        break;
      }
      int forNest = 1;
      int idx = executionPointer;
      while (SCRIPTS[idx++] != -1000)
        ;
      int exit = idx;
      while (forNest > 0) {
        if (SCRIPTS[exit] == FOR)
          forNest++;
        if (SCRIPTS[exit] == ENDFOR)
          forNest--;

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
        add_owner(iterValue);
        len = len_list(iterValue);
      }
      for (int i = 0; i < len; i++) {
        int replacementType;
        int replacementValue;
        if (iterType == STRING) {
          replacementType = STRING;
          char *str = get_string(iterValue);
          char v[2];
          v[0] = str[i];
          v[1] = '\0';
          replacementValue = index_string(&v[0]);
          if (replacementValue == -1) {
            char *newStr = malloc(sizeof(v));
            strcpy(newStr, v);
            replacementValue = add_string(newStr, 1);
          }
        } else {
          ListNode *ln;
          ln = get_from_list(iterValue, i);
          if (ln == NULL) {
            remove_owner(iterValue);
            break;
          }
          replacementType = ln->type;
          replacementValue = ln->value;
        }

        int *_keyTypes = malloc(sizeof(int) * (replacerCount + 1));
        int *_keyValues = malloc(sizeof(int) * (replacerCount + 1));
        int *_replacerTypes = malloc(sizeof(int) * (replacerCount + 1));
        int *_replacerValues = malloc(sizeof(int) * (replacerCount + 1));
        for (int j = 0; j < replacerCount; j++) {
          _keyTypes[j] = keyTypes[j];
          _keyValues[j] = keyValues[j];
          _replacerTypes[j] = replacerTypes[j];
          _replacerValues[j] = replacerValues[j];
        }
        _keyTypes[replacerCount] = keyType;
        _keyValues[replacerCount] = keyValue;
        _replacerTypes[replacerCount] = replacementType;
        _replacerValues[replacerCount] = replacementValue;

        int resolution = resolve_script(idx, self, NULL, world, debug, exit,
                                        _keyTypes, _keyValues, _replacerTypes,
                                        _replacerValues, replacerCount + 1);
        free(_keyTypes);
        free(_keyValues);
        free(_replacerTypes);
        free(_replacerValues);

        if (resolution < 0) {
          remove_owner(iterValue);
          return resolution;
        }
      }
      executionPointer = exit;
      remove_owner(iterValue);
      break;
    }
    case ENDFOR: {
      break;
    }
    case PRINT: {
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
        print_list(value);
        break;
      case NONE:
        printf("None");
        break;
      }
      printf("\n");
      break;
    }
    case UPDATE_STICKS: {
      update_sticks();
      break;
    }
    case SET_JOY: {
      int inputStateType = PARAMS[0];
      int inputStateValue = PARAMS[1];
      int stickType = PARAMS[2];
      int stickValue = PARAMS[3];

      if (inputStateType != STRING || stickType != INT) {
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("Missing or Incorrect Parameter for SET_JOY\n");
        break;
      }
      
      add_stick_to_input_state(inputStateValue, stickValue);
      break;
    }
    case ADD_INPUT_STATE: {
      int inputStateType = PARAMS[0];
      int inputStateValue = PARAMS[1];

      if (inputStateType != STRING) {
        printf("Actor %s error: ", get_string(self->name));
        print_statement(statement);
        printf("Missing or Incorrect Parameter for ADD_INPUT_STATE\n");
        break;
      }

      add_input_state(inputStateValue, -1);
      break;
    }
    }
    executionPointer++;
  }

  clear_ownerless_lists();
  if (debug == 2) {
    printf("Done. number of lists %i\n", get_num_lists());
  }

  return 0;
}

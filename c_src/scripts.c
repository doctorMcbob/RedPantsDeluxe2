#include "utlist.h"
#include "uthash.h"
#include "scripts.h"
#include "actors.h"
#include "frames.h"
#include "inputs.h"
#include "worlds.h"
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

Script* scripts = NULL;
ScriptMap* scriptmaps = NULL;
ListTypeEntry* lists = NULL;
int nextListKey = 0;

void _debug_print_sytanxNode(SyntaxNode* sn) {
  if (sn == NULL) return;
  switch(sn->type) {
    case NONE:
      printf("None ");
      break;
    case INT:
      printf("%i ", sn->data.i);
      break;
    case FLOAT:
      printf("%f ", sn->data.f);
      break;
    case STRING:
      printf("'%s' ", sn->data.s);
      break;
    case LIST: {
      printf("[");
      ListTypeEntry *list = get_list(sn->data.i);
      if (list != NULL) {
        SyntaxNode *entry;
        DL_FOREACH(list->head, entry) {
          _debug_print_sytanxNode(entry);
        }
      }
      printf("] ");
      break;
    }
    case OPERATOR: {
      switch(sn->data.i) {
        case PLUS:
        printf("+ ");
        break;
        case MINUS:
        printf("- ");
        break;
        case MULT:
        printf("* ");
        break;
        case FLOORDIV:
        printf("/ ");
        break;
        case FLOATDIV:
        printf("// ");
        break;
        case MOD:
        printf("%% ");
        break;
        case POW:
        printf("** ");
        break;
        case EQUALS:
        printf("== ");
        break;
        case LESSTHAN:
        printf("< ");
        break;
        case MORETHAN:
        printf("> ");
        break;
        case LESSEQUAL:
        printf("<= ");
        break;
        case MOREEQUAL:
        printf(">= ");
        break;
        case NOTEQUAL:
        printf("!= ");
        break;
        case AND:
        printf("and ");
        break;
        case OR:
        printf("or ");
        break;
        case NOT:
        printf("not ");
        break;
        case NOR:
        printf("nor ");
        break;
        case IN:
        printf("in ");
        break;
        case AT:
        printf("at ");
        break;
        case CASTINT:
        printf("int ");
        break;
        case CASTSTR:
        printf("str ");
        break;
        case MIN:
        printf("min ");
        break;
        case MAX:
        printf("max ");
        break;
        case LEN:
        printf("len ");
        break;
        case COUNTOF:
        printf("countof ");
        break;
        case EXISTS:
        printf("exists ");
        break;
        case HASFRAME:
        printf("hasframe ");
        break;
        case CHOICEOF:
        printf("choiceof ");
        break;
        case ISFRAME:
        printf("isframe ");
        break;
        case ISINPUTSTATE:
        printf("isinputstate ");
        break;
        case ABS:
        printf("abs ");
        break;
        case RANGE:
        printf("range ");
        break;
        case INWORLD:
        printf("inworld ");
        break;
      }
      break;
    }
    case DOT:
      printf(". ");
      break;
    case QRAND:
      printf("RAND? ");
      break;
    case QWORLD:
      printf("WORLD? ");
      break;
    case QSONG:
      printf("SONG? ");
      break;
    case QCOLLIDE:
      printf("COLLIDE? ");
      break;
    case INP_A:
      printf("INP_A ");
      break;
    case INP_B:
      printf("INP_B ");
      break;
    case INP_X:
      printf("INP_X ");
      break;
    case INP_Y:
      printf("INP_Y ");
      break;
    case INP_LEFT:
      printf("INP_LEFT ");
      break;
    case INP_UP:
      printf("INP_UP ");
      break;
    case INP_RIGHT:
      printf("INP_RIGHT ");
      break;
    case INP_DOWN:
      printf("INP_DOWN ");
      break;
    case INP_START:
      printf("INP_START ");
      break;
    case INP_EVENTS:
      printf("INP_EVENTS ");
      break;
  }
};
void _debug_print_verb(int verb) {
  switch (verb) {
    case QUIT:
      printf("QUIT ");
      break;
    case GOODBYE:
      printf("GOODBYE ");
      break;
    case BREAK:
      printf("BREAK ");
      break;
    case RESET:
      printf("RESET ");
      break;
    case SET:
      printf("SET ");
      break;
    case REASSIGN:
      printf("REASSIGN ");
      break;
    case IF:
      printf("IF ");
      break;
    case ENDIF:
      printf("ENDIF ");
      break;
    case EXEC:
      printf("EXEC ");
      break;
    case BACK:
      printf("BACK ");
      break;
    case FRONT:
      printf("FRONT ");
      break;
    case IMG:
      printf("IMG ");
      break;
    case ACTIVATE:
      printf("ACTIVATE ");
      break;
    case DEACTIVATE:
      printf("DEACTIVATE ");
      break;
    case KILLFRAME:
      printf("KILLFRAME ");
      break;
    case MAKEFRAME:
      printf("MAKEFRAME ");
      break;
    case FOCUS:
      printf("FOCUS ");
      break;
    case SCROLLBOUND:
      printf("SCROLLBOUND ");
      break;
    case VIEW:
      printf("VIEW ");
      break;
    case MOVE:
      printf("MOVE ");
      break;
    case PLACE:
      printf("PLACE ");
      break;
    case TAKE:
      printf("TAKE ");
      break;
    case TAKEALL:
      printf("TAKEALL ");
      break;
    case REBRAND:
      printf("REBRAND ");
      break;
    case REMOVE:
      printf("REMOVE ");
      break;
    case ADD:
      printf("ADD ");
      break;
    case HITBOXES:
      printf("HITBOXES ");
      break;
    case HURTBOXES:
      printf("HURTBOXES ");
      break;
    case CREATE:
      printf("CREATE ");
      break;
    case UPDATE:
      printf("UPDATE ");
      break;
    case SFX:
      printf("SFX ");
      break;
    case SONG:
      printf("SONG ");
      break;
    case SFXOFF:
      printf("SFXOFF ");
      break;
    case SONGOFF:
      printf("SONGOFF ");
      break;
    case OFFSETBGSCROLLX:
      printf("OFFSETBGSCROLLX ");
      break;
    case OFFSETBGSCROLLY:
      printf("OFFSETBGSCROLLY ");
      break;
    case FOR:
      printf("FOR ");
      break;
    case ENDFOR:
      printf("ENDFOR ");
      break;
    case PRINT:
      printf("PRINT ");
      break;
    case UPDATE_STICKS:
      printf("UPDATE_STICKS ");
      break;
  }
}
void _debug_print_statement(Statement* statement) {
  printf("Verb: ");
  _debug_print_verb(statement->verb);
  printf("\n");
  printf("  SCRIPT:\n     ");
  SyntaxNode *sn;
  _debug_print_verb(statement->verb);
  DL_FOREACH(statement->script, sn) {
    _debug_print_sytanxNode(sn);

  }
  printf("\n\n  BUFFER:\n     ");
  SyntaxNode *sn2;
  _debug_print_verb(statement->verb);

  DL_FOREACH(statement->buffer, sn2) {
    _debug_print_sytanxNode(sn2);
  }
  printf("\n\n  PARAMS:\n     ");
  _debug_print_verb(statement->verb);
  SyntaxNode *sn3;
  DL_FOREACH(statement->params, sn3) {
    _debug_print_sytanxNode(sn3);
  }
  printf("\n");
}

int add_list() {
  ListTypeEntry *lte = malloc(sizeof(ListTypeEntry));
  lte->head = NULL;
  lte->listKey = nextListKey;
  nextListKey++;
  HASH_ADD_INT(lists, listKey, lte);
  return lte->listKey;
}

ListTypeEntry* get_list(int listKey) {
  ListTypeEntry *lte = NULL;
  HASH_FIND_INT(lists, &listKey, lte);
  if (lte == NULL) return NULL;
  else return lte;
}

void clear_list(int listKey) {
  ListTypeEntry *lte = get_list(listKey);
  if (lte == NULL) return;
  SyntaxNode *sn, *tmp;
  DL_FOREACH_SAFE(lte->head, sn, tmp) {
    DL_DELETE(lte->head, sn);
    free_SyntaxNode(sn);
  }
}

void free_SyntaxNode(SyntaxNode* del) {
  if (del == NULL) return;
  switch (del->type) {
  case STRING:
    free(del->data.s);
    break;
  }
  free(del);
}

SyntaxNode* copy_SyntaxNode(SyntaxNode* orig) {
  SyntaxNode *copy = malloc(sizeof(SyntaxNode));
  copy->type = orig->type;
  switch (orig->type) {
  case STRING: {
    copy->data.s = malloc(strlen(orig->data.s)+1);
    strcpy(copy->data.s, orig->data.s);
    break;
  }
  default:
    copy->data = orig->data;
    break;
  }
  return copy;
}

void add_script(int scriptKey) {
  struct Script* sc;
  sc = malloc(sizeof(Script));
  if (sc == NULL) {
    exit(-1);
  }
  sc->scriptKey = scriptKey;
  sc->statements = NULL;
  HASH_ADD_INT(scripts, scriptKey, sc);
};

Script* get_script(int scriptKey) {
  struct Script* sc;
  HASH_FIND_INT(scripts, &scriptKey, sc);
  if (sc)
    return sc;
  return NULL;
}

void add_statement_to_script(int scriptKey, Statement* statement) {
  struct Script* sc;
  sc = get_script(scriptKey);

  DL_APPEND(sc->statements, statement);
};

Statement* new_statement(int verb) {
  struct Statement* s;
  s = malloc(sizeof(Statement));
  if (s == NULL) {
    exit(-1);
  }
  s->params = NULL;
  s->buffer = NULL;
  s->script = NULL;
  s->verb = verb;
  return s;
}

SyntaxNode* new_syntax_node(int type) {
  struct SyntaxNode* sn;
  sn = malloc(sizeof(SyntaxNode));
  if (sn == NULL) {
    exit(-1);
  }
  sn->type = type;
  return sn;
};

SyntaxNode* _get_last(SyntaxNode *head) {
  SyntaxNode *curr = head;
  while (curr != NULL && curr->next != NULL)
    curr = curr->next;
  return curr;
}

void add_node_to_statement(Statement* statement, SyntaxNode* node) {
  DL_APPEND(statement->script, node);
}

void add_script_map(const char* name) {
  struct ScriptMap* scm;
  scm = malloc(sizeof(ScriptMap));
  if (scm == NULL) {
    exit(-1);
  }
  strncpy(scm->name, name, 32);
  scm->entries = NULL;
  HASH_ADD_STR(scriptmaps, name, scm);
}

ScriptMap* get_script_map(const char* name) {
  struct ScriptMap* scm;
  HASH_FIND_STR(scriptmaps, name, scm);
  if (scm) 
    return scm;
  return NULL;
}

void add_script_to_script_map(const char* name, char* state, int frame, int scriptKey) {
  struct ScriptMap* scm = get_script_map(name);
  if (!scm) {
    printf("No ScriptMap %s\n", name);
    return;
  }
  struct ScriptMapEntry* sme;
  sme = malloc(sizeof(ScriptMapEntry));
  strncpy(sme->state, state, 32);
  sme->frame = frame;
  sme->scriptKey = scriptKey;

  DL_APPEND(scm->entries, sme);
}

// ###########
int resolve_script(int scriptKey,
		    char* worldKey,
		    char* selfActorKey,
		    char* relatedActorKey,
        int debug) {
  Script* script = get_script(scriptKey);
  Statement* statement;
  int if_depth = 0;
  int for_depth = 0;
  DL_FOREACH(script->statements, statement) {

    clean_statement(statement);

    if (if_depth > 0) {
      switch (statement->verb) {
        case IF: 
          if_depth++;
          break;
        case ENDIF:
          if_depth--;
          break;
      } 
      continue;
    }
    if (for_depth > 0) {
      switch (statement->verb) {
        case FOR: 
          for_depth++;
          break;
        case ENDFOR:
          for_depth--;
          break;
      } 
      continue;
    }
    if (debug) {
      _debug_print_statement(statement);
      printf("Evaluating literals\n");
    }
    evaluate_literals(statement, worldKey, selfActorKey, relatedActorKey);
    if (debug) {
      _debug_print_statement(statement);
      printf("Resolving operators\n");
    }
    resolve_operators(statement, worldKey, selfActorKey, relatedActorKey);
    if (debug) {
      _debug_print_statement(statement);
      printf("Resolving verb\n");
    }
    
    switch (statement->verb) {
    case QUIT:
      return -1;
    case GOODBYE:
      return -2;
      break;
    case BREAK:
      return 1;
    case RESET:
      break;
    case SET: {
      SyntaxNode *actorKey, *attrKey, *value;
      actorKey = statement->params;
      if (actorKey == NULL) {
        _debug_print_statement(statement);
        printf("Actor %s: Missing all parameters for set.\n", selfActorKey);
        break;
      }
      if (actorKey->type != STRING) {
        _debug_print_statement(statement);
        printf("Actor %s: first parameter for set must be string.\n", selfActorKey);
        break;
      }
      attrKey = actorKey->next;
      if (attrKey == NULL) {
        _debug_print_statement(statement);
        printf("Actor %s: Missing attribute parameter for set.\n", selfActorKey);
        break;
      }
      if (attrKey->type != STRING) {
        _debug_print_statement(statement);
        printf("Actor %s: second parameter for set must be string.\n", selfActorKey);
        break;
      }
      value = attrKey->next;
      if (value == NULL) {
        _debug_print_statement(statement);
        printf("Actor %s: Missing value parameter for set.\n", selfActorKey);
        _debug_print_statement(statement);
        break;
      }
      Actor* actor;
      if (strcmp(actorKey->data.s, "self") == 0) actor = get_actor(selfActorKey);
      else if (strcmp(actorKey->data.s, "related") == 0) actor = get_actor(relatedActorKey);
      else actor = get_actor(actorKey->data.s);
      if (!actor) {
        _debug_print_statement(statement);
        printf("Actor %s: Could not find actor %s for set\n", selfActorKey, actorKey->data.s);
        break;
      }

      if (strcmp(attrKey->data.s, "x") == 0) {
        if (value->type != INT && value->type != FLOAT) {
          _debug_print_statement(statement);
          printf("Actor %s: Incorrect type for set x %i\n", selfActorKey, value->type);
          break;
        } else if (value->type == INT) {
          actor->ECB->x = value->data.i;
        } else {
          actor->ECB->x = (int)value->data.f;
        } 
        break;
      }

      if (strcmp(attrKey->data.s, "y") == 0) {
        if (value->type != INT && value->type != FLOAT) {
          _debug_print_statement(statement);
          printf("Actor %s: Incorrect type for set y %i\n", selfActorKey, value->type);
          break;
        }
        if (value->type == INT) {
          actor->ECB->y = value->data.i;
        } else {
          actor->ECB->y = (int)value->data.f;
        }
        break;
      }

      if (strcmp(attrKey->data.s, "w") == 0) {
        if (value->type != INT && value->type != FLOAT) {
          _debug_print_statement(statement);
          printf("Actor %s: Incorrect type for set w %i\n", selfActorKey, value->type);
          break;
        }
        if (value->type == INT) {
          actor->ECB->w = value->data.i;
        } else {
          actor->ECB->w = (int)value->data.f;
        }
        break;
      }

      if (strcmp(attrKey->data.s, "h") == 0) {
        if (value->type != INT && value->type != FLOAT) {
          _debug_print_statement(statement);
          printf("Actor %s: Incorrect type for set h %i\n", selfActorKey, value->type);
          break;
        }
        if (value->type == INT) {
          actor->ECB->h = value->data.i;
        } else {
          actor->ECB->h = (int)value->data.f;
        }
        break;
      }

      if (strcmp(attrKey->data.s, "top") == 0) {
        if (value->type != INT && value->type != FLOAT) {
          _debug_print_statement(statement);
          printf("Actor %s: Incorrect type for special case set top %i\n", selfActorKey, value->type);
          break;
        }
        if (value->type == INT) {
          actor->ECB->y = value->data.i;
        } else {
          actor->ECB->y = (int)value->data.f;
        }
        break;
      }

      if (strcmp(attrKey->data.s, "left") == 0) {
        if (value->type != INT && value->type != FLOAT) {
          _debug_print_statement(statement);
          printf("Actor %s: Incorrect type for special case set x left %i\n", selfActorKey, value->type);
          break;
        }
        if (value->type == INT) {
          actor->ECB->x = value->data.i;
        } else {
          actor->ECB->x = (int)value->data.f;
        }
        break;
      }

      if (strcmp(attrKey->data.s, "bottom") == 0) {
        if (value->type != INT && value->type != FLOAT) {
          _debug_print_statement(statement);
          printf("Actor %s: Incorrect type for special case set bottom %i\n", selfActorKey, value->type);
          break;
        }
        if (value->type == INT) {
          actor->ECB->y = value->data.i - actor->ECB->h;
        } else {
          actor->ECB->y = (int)value->data.f - actor->ECB->h;
        }
        break;
      }

      if (strcmp(attrKey->data.s, "right") == 0) {
        if (value->type != INT && value->type != FLOAT) {
          _debug_print_statement(statement);
          printf("Actor %s: Incorrect type for special case set right %i\n", selfActorKey, value->type);
          break;
        }
        if (value->type == INT) {
          actor->ECB->x = value->data.i - actor->ECB->w;
        } else {
          actor->ECB->x = (int)value->data.f - actor->ECB->w;
        }
        break;
      }

      if (strcmp(attrKey->data.s, "name") == 0) {
        if (value->type != STRING) {
          _debug_print_statement(statement);
          printf("Actor %s: Incorrect type for special case set name %i\n", selfActorKey, value->type);
          break;
        }
        strncpy(actor->name, value->data.s, 32);
        break;
      }

      if (strcmp(attrKey->data.s, "state") == 0) {
        if (value->type != STRING) {
          _debug_print_statement(statement);
          printf("Actor %s: Incorrect type for special case set state %i\n", selfActorKey, value->type);
          break;
        }
        strncpy(actor->state, value->data.s, 32);
        break;
      }

      if (strcmp(attrKey->data.s, "frame") == 0) {
        if (value->type != INT && value->type != FLOAT) {
          _debug_print_statement(statement);
          printf("Actor %s: Incorrect type for special case set frame %i\n", selfActorKey, value->type);
          break;
        }
        if (value->type == INT) {
          actor->frame = value->data.i;
        } else {
          actor->frame = (int)value->data.f;
        }
        break;
      }

      if (strcmp(attrKey->data.s, "x_vel") == 0) {
        if (value->type != INT && value->type != FLOAT) {
          _debug_print_statement(statement);
          printf("Actor %s: Incorrect type for special case set x_vel %i\n", selfActorKey, value->type);
          break;
        }
        if (value->type == INT) {
          actor->x_vel = value->data.i;
        } else {
          actor->x_vel = value->data.f;
        }
        break;
      }

      if (strcmp(attrKey->data.s, "y_vel") == 0) {
        if (value->type != INT && value->type != FLOAT) {
          _debug_print_statement(statement);
          printf("Actor %s: Incorrect type for special case set y_vel %i\n", selfActorKey, value->type);
          break;
        }
        if (value->type == INT) {
          actor->y_vel = value->data.i;
        } else {
          actor->y_vel = (int)value->data.f;
        }
        break;
      }

      if (strcmp(attrKey->data.s, "direction") == 0) {
        if (value->type != INT && value->type != FLOAT) {
          _debug_print_statement(statement);
          printf("Actor %s: Incorrect type for special case set direction %i\n", selfActorKey, value->type);
          break;
        }
        if (value->type == INT) {
          actor->direction = value->data.i;
        } else {
          actor->direction = (int)value->data.f;
        }
        break;
      }

      if (strcmp(attrKey->data.s, "rotation") == 0) {
        if (value->type != INT && value->type != FLOAT) {
          _debug_print_statement(statement);
          printf("Actor %s: Incorrect type for special case set rotation %i\n", selfActorKey, value->type);
          break;
        }
        if (value->type == INT) {
          actor->rotation = value->data.i;
        } else {
          actor->rotation = (int)value->data.f;
        }
        break;
      }

      if (strcmp(attrKey->data.s, "platform") == 0) {
        if (value->type != INT && value->type != FLOAT) {
          _debug_print_statement(statement);
          printf("Actor %s: Incorrect type for special case set platform %i\n", selfActorKey, value->type);
          break;
        }
        if (value->type == INT) {
          actor->platform = value->data.i;
        } else {
          actor->platform = (int)value->data.f;
        }
        break;
      }

      if (strcmp(attrKey->data.s, "tangible") == 0) {
        if (value->type != INT && value->type != FLOAT) {
          _debug_print_statement(statement);
          printf("Actor %s: Incorrect type for special case set tangible %i\n", selfActorKey, value->type);
          break;
        }
        if (value->type == INT) {
          actor->tangible = value->data.i;
        } else {
          actor->tangible = (int)value->data.f;
        }
        break;
      }

      if (strcmp(attrKey->data.s, "physics") == 0) {
        if (value->type != INT && value->type != FLOAT) {
          _debug_print_statement(statement);
          printf("Actor %s: Incorrect type for special case set physics %i\n", selfActorKey, value->type);
          break;
        }
        if (value->type == INT) {
          actor->physics = value->data.i;
        } else {
          actor->physics = (int)value->data.f;
        }
        break;
      }

      Attribute *attr = NULL;
      HASH_FIND_STR(actor->attributes, attrKey->data.s, attr);
      if (attr == NULL) {
        attr = malloc(sizeof(Attribute));
        strncpy(attr->name, attrKey->data.s, 32);
        attr->value = copy_SyntaxNode(value);
        HASH_ADD_STR(actor->attributes, name, attr);      
      } else {
        free_SyntaxNode(attr->value);
        attr->value = copy_SyntaxNode(value);
      }
      break;
    }
    case REASSIGN:
      break;
    case IF: {
      SyntaxNode *conditional;
      conditional = statement->params;
      if (conditional == NULL) {
        _debug_print_statement(statement);
        printf("Actor %s: Missing conditional for if.\n", selfActorKey);
        break;
      }

      int conditionalPass = 0;
      switch (conditional->type) {
        case INT:
          conditionalPass = conditional->data.i != 0;
          break;
        case FLOAT:
          conditionalPass = conditional->data.f != 0;
          break;
        case STRING:
          conditionalPass = strlen(conditional->data.s) != 0;
          break;
        case LIST: {
          ListTypeEntry *list = get_list(conditional->data.i);
          conditionalPass = list->head != NULL;
          break;
        }
      }

      if (!conditionalPass) {
        if_depth++;
      }

      break;
    }
    case EXEC: {
      if (statement->params == NULL) {
        _debug_print_statement(statement);
        printf("Missing script parameter for EXEC\n");
        break;
      }
      if (statement->params->type != STRING) {
        _debug_print_statement(statement);
        printf("Incorrect type for EXEC parameter %i\n", statement->params->type);
        break;
      }
      char *scriptName = statement->params->data.s;
      Actor *actor = get_actor(selfActorKey);
      int scriptKey = find_script_from_map(actor, scriptName);
      if (scriptKey == -1) {
       _debug_print_statement(statement);
        printf("Could not find script named %s for EXEC \n", scriptName);
        break;
      }
      int resolution = resolve_script(scriptKey, worldKey, selfActorKey, NULL, debug);
      if (resolution < 0) return resolution;
      break;
    }
    case BACK:
      break;
    case FRONT:
      break;
    case IMG:
      break;
    case ACTIVATE:
      break;
    case DEACTIVATE:
      break;
    case KILLFRAME:
      break;
    case MAKEFRAME:
      break;
    case FOCUS:
      break;
    case SCROLLBOUND:
      break;
    case VIEW:
      break;
    case MOVE:
      break;
    case PLACE:
      break;
    case TAKE:
      break;
    case TAKEALL:
      break;
    case REBRAND:
      break;
    case REMOVE: {
      if (statement->params == NULL) {
        _debug_print_statement(statement);
        printf("Missing all parameters for REMOVE\n");
        break;
      }
      if (statement->params->type != LIST) {
        _debug_print_statement(statement);
        printf("First parameter of REMOVE must be type LIST, not %i", statement->params->type);
        break;
      }
      SyntaxNode *listParam = statement->params;
      if (statement->params->next == NULL) {
        _debug_print_statement(statement);
        printf("Missing value parameters for REMOVE\n");
        break;
      }
      ListTypeEntry *list = get_list(listParam->data.i);
      if (list == NULL)  {
        _debug_print_statement(statement);
        printf("No list found at key %i for REMOVE\n", listParam->data.i);
        break;
      }
      SyntaxNode *del = statement->params->next;
      SyntaxNode *sn, *tmp, *match = NULL;

      DL_FOREACH_SAFE(list->head, sn, tmp) {
        if (del->type != sn->type) continue;
        switch (del->type) {
          case NONE:
            match = sn;
            break;
          case INT:
            if (del->data.i == sn->data.i) match = sn;
            break;
          case FLOAT:
            if (del->data.f == sn->data.f) match = sn;
            break;
          case STRING:
            if (strcmp(del->data.s, sn->data.s) == 0) match = sn;
            break;
          case LIST:
            if (del->data.i == sn->data.i) match = sn;
            break;
        }
      };
      if (match != NULL) {
        DL_DELETE(listParam, match);
        free_SyntaxNode(match);
      }
      break;
    }
    case ADD: {
      if (statement->params == NULL) {
        _debug_print_statement(statement);
        printf("Missing all parameters for ADD\n");
        break;
      }
      if (statement->params->type != LIST) {
        _debug_print_statement(statement);
        printf("First parameter of ADD must be type LIST, not %i", statement->params->type);
        break;
      }
      SyntaxNode *listParam = statement->params;
      if (statement->params->next == NULL) {
        _debug_print_statement(statement);
        printf("Missing value parameters for ADD\n");
        break;
      }
      ListTypeEntry *list = get_list(listParam->data.i);
      if (list == NULL) {
        _debug_print_statement(statement);
        printf("No list found at key %i for ADD\n", listParam->data.i);
        break;
      }
      
      SyntaxNode *new = copy_SyntaxNode(statement->params->next);
      DL_APPEND(list->head, new);
      break;
    }
    case HITBOXES:
      break;
    case HURTBOXES:
      break;
    case CREATE:
      break;
    case UPDATE:
      break;
    case SFX:
      break;
    case SONG:
      break;
    case SFXOFF:
      break;
    case SONGOFF:
      break;
    case OFFSETBGSCROLLX:
      break;
    case OFFSETBGSCROLLY:
      break;
    case FOR:
      break;
    case PRINT: {
      if (statement->params != NULL) {
        _debug_print_sytanxNode(statement->params);
      }
      printf("\n");
      break;
    }
    case UPDATE_STICKS:
      break;
    }  
  }
  return 0;
}

void clean_statement(Statement* statement) {
  if (statement->buffer != NULL) {
    SyntaxNode *sn, *tmp;
    DL_FOREACH_SAFE(statement->buffer, sn, tmp) {
      DL_DELETE(statement->buffer, sn);
      free_SyntaxNode(sn);
    }
    statement->buffer = NULL;
  }
  if (statement->params != NULL) {
    SyntaxNode *sn2, *tmp2;
    DL_FOREACH_SAFE(statement->params, sn2, tmp2) {
      DL_DELETE(statement->params, sn2);
      free_SyntaxNode(sn2);
    }
    statement->params = NULL;
  }
}

void evaluate_literals(Statement* statement,
		       char* worldKey,
		       char* selfActorKey,
		       char* relatedActorKey) {
  SyntaxNode *sn;
  int skipCauseDot = 0;
  DL_FOREACH(statement->script, sn) {
    if (skipCauseDot) {
      skipCauseDot = 0;
      continue;
    }
    SyntaxNode *new;
    switch (sn->type) {
    case NONE:
    case FLOAT:
    case INT:
    case STRING: { 
      new = copy_SyntaxNode(sn);
      DL_APPEND(statement->buffer, new);
      break;
    }
    case OPERATOR: {
      new = copy_SyntaxNode(sn);
      DL_APPEND(statement->buffer, new);
      break;
    }
    case DOT: {
      SyntaxNode* parameter = _get_last(statement->buffer);
      if (parameter == NULL || sn->next == NULL) {
        _debug_print_statement(statement);
	      printf("actor %s: Missing parameter for dot notation on verb %i\n", selfActorKey, statement->verb);
	      break;
      }
      if (parameter->type != STRING) {
        _debug_print_statement(statement);
        printf("actor %s: Cannot use dot notation on type %i\n", selfActorKey, parameter->type);
        break;
      }
      if (sn->next->type != STRING) {
        _debug_print_statement(statement);
        printf("actor %s: Cannot use dot notation with type %i\n", selfActorKey, sn->next->type);
        break;
      }
      char *actorKey = parameter->data.s;
      if (strcmp(actorKey, "self") == 0) actorKey = selfActorKey;
      else if (strcmp(actorKey, "related") == 0) actorKey= relatedActorKey;
      Actor *actor = get_actor(actorKey);
      if (actor == NULL) {
        _debug_print_statement(statement);
        printf("Actor %s: Could not find actor %s\n", selfActorKey, actorKey);
        skipCauseDot = 1;
        break;
      }

      free(parameter->data.s);
      // check special (top level actor attributes, not in hash)
      if (strcmp(sn->next->data.s, "x") == 0) {
        parameter->type = INT;
        parameter->data.i = actor->ECB->x;
        skipCauseDot = 1;
        break;
      }
      if (strcmp(sn->next->data.s, "y") == 0) {
        parameter->type = INT;
        parameter->data.i = actor->ECB->y;
        skipCauseDot = 1;
        break;
      }
      if (strcmp(sn->next->data.s, "w") == 0) {
        parameter->type = INT; 
        parameter->data.i = actor->ECB->w;
        skipCauseDot = 1;
        break;
      }
      if (strcmp(sn->next->data.s, "h") == 0) {
        parameter->type = INT;
        parameter->data.i = actor->ECB->h;
        skipCauseDot = 1;
        break;
      }
      if (strcmp(sn->next->data.s, "top") == 0) {
        parameter->type = INT;
        parameter->data.i = actor->ECB->y;
        skipCauseDot = 1;
        break;
      }
      if (strcmp(sn->next->data.s, "left") == 0) {
        parameter->type = INT;
        parameter->data.i = actor->ECB->x;
        skipCauseDot = 1;
        break;
      }
      if (strcmp(sn->next->data.s, "bottom") == 0) {
        parameter->type = INT;
        parameter->data.i = actor->ECB->y + actor->ECB->h;
        skipCauseDot = 1;
        break;
      }
      if (strcmp(sn->next->data.s, "right") == 0) {
        parameter->type = INT;
        parameter->data.i = actor->ECB->x + actor->ECB->w;
        skipCauseDot = 1;
        break;
      }
      if (strcmp(sn->next->data.s, "name") == 0) {
        parameter->type = STRING;
        parameter->data.s = malloc(strlen(actor->name)+1);
        strncpy(parameter->data.s, actor->name, strlen(actor->name)+1);
        skipCauseDot = 1;
        break;
      }
      if (strcmp(sn->next->data.s, "state") == 0) {
        parameter->type = STRING;
        parameter->data.s = malloc(strlen(actor->state)+1);
        strncpy(parameter->data.s, actor->state, strlen(actor->name)+1);
        skipCauseDot = 1;
        break;
      }
      if (strcmp(sn->next->data.s, "frame") == 0) {
        parameter->type = INT;
        parameter->data.i = actor->frame;
        skipCauseDot = 1;
        break;
      }
      if (strcmp(sn->next->data.s, "x_vel") == 0) {
        parameter->type = INT;
        parameter->data.i = actor->x_vel;
        skipCauseDot = 1;
        break;
      }
      if (strcmp(sn->next->data.s, "y_vel") == 0) {
        parameter->type = INT;
        parameter->data.i = actor->y_vel;
        skipCauseDot = 1;
        break;
      }
      if (strcmp(sn->next->data.s, "direction") == 0) {
        parameter->type = INT;
        parameter->data.i = actor->direction;
        skipCauseDot = 1;
        break;
      }
      if (strcmp(sn->next->data.s, "rotation") == 0) {
        parameter->type = INT;
        parameter->data.i = actor->rotation;
        skipCauseDot = 1;
        break;
      }
      if (strcmp(sn->next->data.s, "platform") == 0) {
        parameter->type = INT;
        parameter->data.i = actor->frame;
        skipCauseDot = 1;
        break;
      }
      if (strcmp(sn->next->data.s, "tangible") == 0) {
        parameter->type = INT;
        parameter->data.i = actor->tangible;
        skipCauseDot = 1;
        break;
      }
      if (strcmp(sn->next->data.s, "physics") == 0) {
        parameter->type = INT;
        parameter->data.i = actor->physics;
        skipCauseDot = 1;
        break;
      }

      Attribute *attribute = NULL;
      HASH_FIND_STR(actor->attributes, sn->next->data.s, attribute);
      if (attribute == NULL) {
        parameter = new_syntax_node(NONE);
        parameter->data.i = 0;
        skipCauseDot = 1;
        break;
      }
      
      parameter->type = attribute->value->type;
      if (attribute->value->type == STRING) {
        parameter->data.s = (char*)malloc(strlen(attribute->value->data.s));
        strncpy(parameter->data.s, attribute->value->data.s, strlen(attribute->value->data.s));
      } else {
      	parameter->data =  attribute->value->data;
      }
      skipCauseDot = 1;
      break;
    }
    case QRAND: {
      new = malloc(sizeof(SyntaxNode));
      new->type = INT;
      new->data.i = rand() % 2;
      DL_APPEND(statement->buffer, new);
      break;
    }
    case QWORLD: {
      // put new SyntaxNode with type STRING with the worldKey
      new = new_syntax_node(STRING);
      new->data.s = malloc(strlen(worldKey)+1);
      strcpy(new->data.s, worldKey);
      break;
    }
    case QSONG:
      // put new SyntaxNode with type STRING with the key for the active song
      break;
    case QCOLLIDE:
      // check if the actorKey is colliding with any other tangible actors int he worldKey
      // put new SyntaxNode with type INT 0 or 1
      break;
    case LIST: {
      // put new SyntaxNode with type LIST onto buffer, setting data.list to NULL for the head of a DL
      new = new_syntax_node(LIST);
      new->data.i = add_list();
      DL_APPEND(statement->buffer, new);
      break;
    }
    case INP_A: {
      // put new SyntaxNode with type INT onto buffer, setting data.i to actorKey->_input_state A
      Actor *a = get_actor(selfActorKey);
      if (a == NULL) break;
      InputState *is = get_input_state(a->_input_name);
      if (is == NULL) break;
      new = new_syntax_node(INT);
      new->data.i = is->A;
      DL_APPEND(statement->buffer, new);
      break;
    }
    case INP_B: {
      // put new SyntaxNode with type INT onto buffer, setting data.i to actorKey->_input_state B
      Actor *a = get_actor(selfActorKey);
      if (a == NULL) break;
      InputState *is = get_input_state(a->_input_name);
      if (is == NULL) break;
      new = new_syntax_node(INT);
      new->data.i = is->B;
      DL_APPEND(statement->buffer, new);
      break;
    }
    case INP_X: {
      // put new SyntaxNode with type INT onto buffer, setting data.i to actorKey->_input_state X
      Actor *a = get_actor(selfActorKey);
      if (a == NULL) break;
      InputState *is = get_input_state(a->_input_name);
      if (is == NULL) break;
      new = new_syntax_node(INT);
      new->data.i = is->X;
      DL_APPEND(statement->buffer, new);
      break;
    }
    case INP_Y: {
      // put new SyntaxNode with type INT onto buffer, setting data.i to actorKey->_input_state Y
      Actor *a = get_actor(selfActorKey);
      if (a == NULL) break;
      InputState *is = get_input_state(a->_input_name);
      if (is == NULL) break;
      new = new_syntax_node(INT);
      new->data.i = is->Y;
      DL_APPEND(statement->buffer, new);
      break;
    }
    case INP_LEFT: {
      // put new SyntaxNode with type INT onto buffer, setting data.i to actorKey->_input_state LEFT
      Actor *a = get_actor(selfActorKey);
      if (a == NULL) break;
      InputState *is = get_input_state(a->_input_name);
      if (is == NULL) break;
      new = new_syntax_node(INT);
      new->data.i = is->LEFT;
      DL_APPEND(statement->buffer, new);
      break;
    }
    case INP_UP: {
      // put new SyntaxNode with type INT onto buffer, setting data.i to actorKey->_input_state UP
      Actor *a = get_actor(selfActorKey);
      if (a == NULL) break;
      InputState *is = get_input_state(a->_input_name);
      if (is == NULL) break;
      new = new_syntax_node(INT);
      new->data.i = is->UP;
      DL_APPEND(statement->buffer, new);
      break;
    }
    case INP_RIGHT: {
      // put new SyntaxNode with type INT onto buffer, setting data.i to actorKey->_input_state RIGHT
      Actor *a = get_actor(selfActorKey);
      if (a == NULL) break;
      InputState *is = get_input_state(a->_input_name);
      if (is == NULL) break;
      new = new_syntax_node(INT);
      new->data.i = is->RIGHT;
      DL_APPEND(statement->buffer, new);
      break;
    }
    case INP_DOWN: {
      // put new SyntaxNode with type INT onto buffer, setting data.i to actorKey->_input_state DOWN
        Actor *a = get_actor(selfActorKey);
      if (a == NULL) break;
      InputState *is = get_input_state(a->_input_name);
      if (is == NULL) break;
      new = new_syntax_node(INT);
      new->data.i = is->DOWN;
      DL_APPEND(statement->buffer, new);
      break;
    }
    case INP_START: {
      // put new SyntaxNode with type INT onto buffer, setting data.i to actorKey->_input_state START
      Actor *a = get_actor(selfActorKey);
      if (a == NULL) break;
      InputState *is = get_input_state(a->_input_name);
      if (is == NULL) break;
      new = new_syntax_node(INT);
      new->data.i = is->START;
      DL_APPEND(statement->buffer, new);
      break;
    }
    case INP_EVENTS: {
      // create a new SyntaxNode DL and fill it with SyntaxNodes with type STRING for each event we have in actorKey->_input_state EVENTS
      Actor *a = get_actor(selfActorKey);
      if (a == NULL) break;
      InputState *is = get_input_state(a->_input_name);
      if (is == NULL) break;
      clear_list(0);
      ListTypeEntry *lte = get_list(0);
      SyntaxNode *eventNode;
      if (is->EVENTS[A_DOWN]) {
        eventNode = new_syntax_node(STRING);
        eventNode->data.s = malloc(7);
        strcpy(eventNode->data.s, "A_DOWN");
        DL_APPEND(lte->head, eventNode);
      }
      if (is->EVENTS[A_UP]) {
        eventNode = new_syntax_node(STRING);
        eventNode->data.s = malloc(5);
        strcpy(eventNode->data.s, "A_UP");
        DL_APPEND(lte->head, eventNode);
      }
      if (is->EVENTS[B_DOWN]) {
        eventNode = new_syntax_node(STRING);
        eventNode->data.s = malloc(7);
        strcpy(eventNode->data.s, "B_DOWN");
        DL_APPEND(lte->head, eventNode);
      }
      if (is->EVENTS[B_UP]) {
        eventNode = new_syntax_node(STRING);
        eventNode->data.s = malloc(5);
        strcpy(eventNode->data.s, "B_UP");
        DL_APPEND(lte->head, eventNode);
      }
      if (is->EVENTS[X_DOWN]) {
        eventNode = new_syntax_node(STRING);
        eventNode->data.s = malloc(7);
        strcpy(eventNode->data.s, "X_DOWN");
        DL_APPEND(lte->head, eventNode);
      }
      if (is->EVENTS[X_UP]) {
        eventNode = new_syntax_node(STRING);
        eventNode->data.s = malloc(5);
        strcpy(eventNode->data.s, "X_UP");
        DL_APPEND(lte->head, eventNode);
      }
      if (is->EVENTS[Y_DOWN]) {
        eventNode = new_syntax_node(STRING);
        eventNode->data.s = malloc(7);
        strcpy(eventNode->data.s, "Y_DOWN");
        DL_APPEND(lte->head, eventNode);
      }
      if (is->EVENTS[Y_UP]) {
        eventNode = new_syntax_node(STRING);
        eventNode->data.s = malloc(5);
        strcpy(eventNode->data.s, "Y_UP");
        DL_APPEND(lte->head, eventNode);
      }
      if (is->EVENTS[LEFT_DOWN]) {
        eventNode = new_syntax_node(STRING);
        eventNode->data.s = malloc(10);
        strcpy(eventNode->data.s, "LEFT_DOWN");
        DL_APPEND(lte->head, eventNode);
      }
      if (is->EVENTS[LEFT_UP]) {
        eventNode = new_syntax_node(STRING);
        eventNode->data.s = malloc(8);
        strcpy(eventNode->data.s, "LEFT_UP");
        DL_APPEND(lte->head, eventNode);
      }
      if (is->EVENTS[UP_DOWN]) {
        eventNode = new_syntax_node(STRING);
        eventNode->data.s = malloc(8);
        strcpy(eventNode->data.s, "UP_DOWN");
        DL_APPEND(lte->head, eventNode);
      }
      if (is->EVENTS[UP_UP]) {
        eventNode = new_syntax_node(STRING);
        eventNode->data.s = malloc(6);
        strcpy(eventNode->data.s, "UP_UP");
        DL_APPEND(lte->head, eventNode);
      }
      if (is->EVENTS[RIGHT_DOWN]) {
        eventNode = new_syntax_node(STRING);
        eventNode->data.s = malloc(11);
        strcpy(eventNode->data.s, "RIGHT_DOWN");
        DL_APPEND(lte->head, eventNode);
      }
      if (is->EVENTS[RIGHT_UP]) {
        eventNode = new_syntax_node(STRING);
        eventNode->data.s = malloc(9);
        strcpy(eventNode->data.s, "RIGHT_UP");
        DL_APPEND(lte->head, eventNode);
      }
      if (is->EVENTS[DOWN_DOWN]) {
        eventNode = new_syntax_node(STRING);
        eventNode->data.s = malloc(10);
        strcpy(eventNode->data.s, "DOWN_DOWN");
        DL_APPEND(lte->head, eventNode);
      }
      if (is->EVENTS[DOWN_UP]) {
        eventNode = new_syntax_node(STRING);
        eventNode->data.s = malloc(8);
        strcpy(eventNode->data.s, "DOWN_UP");
        DL_APPEND(lte->head, eventNode);
      }
      if (is->EVENTS[START_DOWN]) {
        eventNode = new_syntax_node(STRING);
        eventNode->data.s = malloc(11);
        strcpy(eventNode->data.s, "START_DOWN");
        DL_APPEND(lte->head, eventNode);
      }
      if (is->EVENTS[START_UP]) {
        eventNode = new_syntax_node(STRING);
        eventNode->data.s = malloc(9);
        strcpy(eventNode->data.s, "START_UP");
        DL_APPEND(lte->head, eventNode);
      }

      new = new_syntax_node(LIST);
      new->data.i = 0;
      DL_APPEND(statement->buffer, new);
      break;
    }
    }
  }
}

void normalize_left_right(SyntaxNode* left, SyntaxNode* right) {
  if (left->type == INT && right->type == STRING) {
    int i = left->data.i;
    int len = snprintf(NULL, 0, "%i", i);
    left->data.s = malloc(len+1);
    left->type = STRING;
    sprintf(left->data.s, "%i", i);
  }

  if (left->type == FLOAT && right->type == STRING) {
    float f = left->data.f;
    left->type = STRING;
    sprintf(left->data.s, "%f", f);
  }

  if (left->type == STRING && right->type == INT) {
    int i = right->data.i;
    right->type = STRING;
    int len = snprintf(NULL, 0, "%i", i);
    right->data.s = malloc(len+1);
    sprintf(right->data.s, "%i", i);
  }

  if (left->type == STRING && right->type == FLOAT) {
    float f = right->data.f;
    right->type = STRING;
    sprintf(right->data.s, "%f", f);
  }
}

void resolve_operators(Statement* statement,
		       char* worldKey,
		       char* selfActorKey,
		       char* relatedActorKey) {
  SyntaxNode *sn, *tmp;
  int skipNbr = 0;

  DL_FOREACH_SAFE(statement->buffer, sn, tmp) {
    
    if (skipNbr>0) {
      skipNbr--;
      continue;
    }

    SyntaxNode *new = NULL, *param = _get_last(statement->params);
    if (sn->type != OPERATOR) {
      new = copy_SyntaxNode(sn);
      DL_APPEND(statement->params, new);
      continue;
    }
    skipNbr = 1;
    switch (sn->data.i) {
    case PLUS: {
      if (param == NULL || sn->next == NULL) break;
      normalize_left_right(param, sn->next);
      if (param->type == STRING && sn->next->type == STRING) {
        int len = strlen(param->data.s) + strlen(sn->next->data.s) + 1;
        char* combined_str = (char*)malloc(len * sizeof(char)+1);
        snprintf(combined_str, len, "%s%s", param->data.s, sn->next->data.s);
        
        new = new_syntax_node(STRING);
        new->data.s = combined_str;
        DL_APPEND(statement->params, new);
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
      } else if (param->type == INT && sn->next->type == INT) {
        new = new_syntax_node(INT);
        new->data.i = param->data.i + sn->next->data.i;
        DL_APPEND(statement->params, new);
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);

      } else if (param->type == INT && sn->next->type == FLOAT) {
        new = new_syntax_node(FLOAT);
        new->data.f = param->data.i + sn->next->data.f;
        DL_APPEND(statement->params, new);
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);

      } else if (param->type == FLOAT && sn->next->type == INT) {
        new = new_syntax_node(FLOAT);
        new->data.f = param->data.f + sn->next->data.i;
        DL_APPEND(statement->params, new);
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);

      } else if (param->type == FLOAT && sn->next->type == FLOAT) {
        new = new_syntax_node(FLOAT);
        new->data.f = param->data.f + sn->next->data.f;
        DL_APPEND(statement->params, new);
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
      } else {
        _debug_print_statement(statement);
        printf("Could not + types: %i, %i\n", param->type, sn->next->type);
      }
      break;
    }
    case MINUS: {
      if (param == NULL || sn->next == NULL) break;
      normalize_left_right(param, sn->next);
      if (param->type == INT && sn->next->type == INT) {
        new = new_syntax_node(INT);
        new->data.i = param->data.i - sn->next->data.i;
        DL_APPEND(statement->params, new);
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
      } else if (param->type == INT && sn->next->type == FLOAT) {
        new = new_syntax_node(FLOAT);
        new->data.f = param->data.i - sn->next->data.f;
        DL_APPEND(statement->params, new);
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
      } else if (param->type == FLOAT && sn->next->type == INT) {
        new = new_syntax_node(FLOAT);
        new->data.f = param->data.f - sn->next->data.i;
        DL_APPEND(statement->params, new);
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
      } else if (param->type == FLOAT && sn->next->type == FLOAT) {
        new = new_syntax_node(FLOAT);
        new->data.f = param->data.f - sn->next->data.f;
        DL_APPEND(statement->params, new);
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
      } else {
        _debug_print_statement(statement);
      	printf("Could not - types: %i, %i\n", param->type, sn->next->type);
      }
      break;
    }
    case MULT: {
      if (param == NULL || sn->next == NULL) break;
      normalize_left_right(param, sn->next);
      if (param->type == INT && sn->next->type == INT) {
        new = new_syntax_node(INT);
        new->data.i = param->data.i * sn->next->data.i;
        DL_APPEND(statement->params, new);
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
      } else if (param->type == INT && sn->next->type == FLOAT) {
        new = new_syntax_node(FLOAT);
        new->data.f = param->data.i * sn->next->data.f;
        DL_APPEND(statement->params, new);
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
      } else if (param->type == FLOAT && sn->next->type == INT) {
        new = new_syntax_node(FLOAT);
        new->data.f = param->data.f * sn->next->data.i;
        DL_APPEND(statement->params, new);
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
      } else if (param->type == FLOAT && sn->next->type == FLOAT) {
        new = new_syntax_node(FLOAT);
        new->data.f = param->data.f * sn->next->data.f;
        DL_APPEND(statement->params, new);
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
      } else {
        _debug_print_statement(statement);
      	printf("Could not * types: %i, %i\n", param->type, sn->next->type);
      }
      break;
    }
    case FLOORDIV: {
      if (param == NULL || sn->next == NULL) break;
      normalize_left_right(param, sn->next);
      new = new_syntax_node(INT);
      DL_APPEND(statement->params, new);
      if (param->type == INT && sn->next->type == INT) {
        new->data.i = param->data.i / sn->next->data.i;
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
      } else if (param->type == INT && sn->next->type == FLOAT) {
      	new->data.i = param->data.i / (int)sn->next->data.f;
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
      } else if (param->type == FLOAT && sn->next->type == INT) {
      	new->data.i = (int)param->data.f / sn->next->data.i;
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
      } else if (param->type == FLOAT && sn->next->type == FLOAT) {
      	new->data.i = (int)param->data.f / (int)sn->next->data.f;
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
      } else {
        _debug_print_statement(statement);
      	printf("Could not // types: %i, %i\n", param->type, sn->next->type);
      }
      break;
    }
    case FLOATDIV: {
      if (param == NULL || sn->next == NULL) break;
      normalize_left_right(param, sn->next);
      new = new_syntax_node(FLOAT);
      DL_APPEND(statement->params, new);
      if (param->type == INT && sn->next->type == INT) {
	      new->data.f = param->data.i / (float)sn->next->data.i;
      DL_DELETE(statement->params, param);
      free_SyntaxNode(param);
      } else if (param->type == INT && sn->next->type == FLOAT) {
	      new->data.f = param->data.i / sn->next->data.f;
      DL_DELETE(statement->params, param);
      free_SyntaxNode(param);
      } else if (param->type == FLOAT && sn->next->type == INT) {
	      new->data.f = param->data.f / sn->next->data.i;
      DL_DELETE(statement->params, param);
      free_SyntaxNode(param);
      } else if (param->type == FLOAT && sn->next->type == FLOAT) {
	      new->data.f = param->data.f / sn->next->data.f;
      DL_DELETE(statement->params, param);
      free_SyntaxNode(param);
      } else {
        _debug_print_statement(statement);
	      printf("Could not / types: %i, %i\n", param->type, sn->next->type);
      }
      break;
    }
    case MOD: {
      if (param == NULL || sn->next == NULL) break;
      normalize_left_right(param, sn->next);
      new = new_syntax_node(INT);
      DL_APPEND(statement->params, new);
      if (param->type == INT && sn->next->type == INT) {
      	new->data.i = param->data.i % sn->next->data.i;
      DL_DELETE(statement->params, param);
      free_SyntaxNode(param);
      } else if (param->type == INT && sn->next->type == FLOAT) {
	      new->data.f = param->data.i % (int)sn->next->data.f;
      DL_DELETE(statement->params, param);
      free_SyntaxNode(param);
      } else if (param->type == FLOAT && sn->next->type == INT) {
	      new->data.f = (int)param->data.f % sn->next->data.i;
      DL_DELETE(statement->params, param);
      free_SyntaxNode(param);
      } else if (param->type == FLOAT && sn->next->type == FLOAT) {
	      new->data.f = (int)param->data.f % (int)sn->next->data.f;
      DL_DELETE(statement->params, param);
      free_SyntaxNode(param);
      } else {
        _debug_print_statement(statement);
      	printf("Could not %% types: %i, %i\n", param->type, sn->next->type);
      }
      break;
    }
    case POW: {
      if (param == NULL || sn->next == NULL) break;
      normalize_left_right(param, sn->next);
      if (param->type == INT && sn->next->type == INT) {
        new = new_syntax_node(INT);
        new->data.i = pow(param->data.i,  sn->next->data.i);
      	DL_APPEND(statement->params, new);
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
      } else if (param->type == INT && sn->next->type == FLOAT) {
        new = new_syntax_node(FLOAT);
        new->data.f = pow(param->data.i, sn->next->data.f);
        DL_APPEND(statement->params, new);
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
      } else if (param->type == FLOAT && sn->next->type == INT) {
        new = new_syntax_node(FLOAT);
        new->data.f = pow(param->data.f, sn->next->data.i);
        DL_APPEND(statement->params, new);
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
      } else if (param->type == FLOAT && sn->next->type == FLOAT) {
        new = new_syntax_node(FLOAT);
        new->data.f = pow(param->data.f, sn->next->data.f);
        DL_APPEND(statement->params, new);
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
      } else {
        _debug_print_statement(statement);
      	printf("Could not ** types: %i, %i\n", param->type, sn->next->type);
      }
      break;
    }
    case EQUALS: {
      if (param == NULL || sn->next == NULL) break;
      normalize_left_right(param, sn->next);
      if (param->type == STRING && sn->next->type == STRING) {
        new = new_syntax_node(INT);
        DL_APPEND(statement->params, new);
        if (strcmp(param->data.s, sn->next->data.s) == 0) 
          new->data.i = 1;
        else
          new->data.i = 0;
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
      } else if (param->type == INT && sn->next->type == INT) {
        new = new_syntax_node(INT);
        DL_APPEND(statement->params, new);
        if (param->data.i == sn->next->data.i) 
          new->data.i = 1;
        else
          new->data.i = 0;
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
      } else if (param->type == INT && sn->next->type == FLOAT) {
        new = new_syntax_node(INT);
        DL_APPEND(statement->params, new);
        if ((float)param->data.i == sn->next->data.f) 
          new->data.i = 1;
        else
          new->data.i = 0;
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
      } else if (param->type == FLOAT && sn->next->type == INT) {
        new = new_syntax_node(INT);
        DL_APPEND(statement->params, new);
        if (param->data.f == (float)sn->next->data.i) 
          new->data.i = 1;
        else
          new->data.i = 0;
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
      } else if (param->type == FLOAT && sn->next->type == FLOAT) {
        new = new_syntax_node(INT);
        DL_APPEND(statement->params, new);
        if (param->data.f == sn->next->data.f) 
          new->data.i = 1;
        else
          new->data.i = 0;
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
      } else {
        _debug_print_statement(statement);
	      printf("Could not == types: %i, %i\n", param->type, sn->next->type);
      }
      break;
    }
    case LESSTHAN: {
      if (param == NULL || sn->next == NULL) break;
      normalize_left_right(param, sn->next);
      if (param->type == INT && sn->next->type == INT) {
        new = new_syntax_node(INT);
        DL_APPEND(statement->params, new);
        if (param->data.i < sn->next->data.i) 
          new->data.i = 1;
        else
          new->data.i = 0;
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
      } else if (param->type == INT && sn->next->type == FLOAT) {
        new = new_syntax_node(INT);
        DL_APPEND(statement->params, new);
        if ((float)param->data.i < sn->next->data.f) 
          new->data.i = 1;
        else
          new->data.i = 0;
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
      } else if (param->type == FLOAT && sn->next->type == INT) {
        new = new_syntax_node(INT);
        DL_APPEND(statement->params, new);
        if (param->data.f < (float)sn->next->data.i) 
          new->data.i = 1;
        else
          new->data.i = 0;
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
      } else if (param->type == FLOAT && sn->next->type == FLOAT) {
        new = new_syntax_node(INT);
        DL_APPEND(statement->params, new);
        if (param->data.f < sn->next->data.f) 
          new->data.i = 1;
        else
          new->data.i = 0;
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
      } else {
        _debug_print_statement(statement);
      	printf("Could not < types: %i, %i\n", param->type, sn->next->type);
      }
      break;
    }
    case MORETHAN: {
      if (param == NULL || sn->next == NULL) break;
      normalize_left_right(param, sn->next);
      if (param->type == INT && sn->next->type == INT) {
        new = new_syntax_node(INT);
        DL_APPEND(statement->params, new);
        if (param->data.i > sn->next->data.i) 
          new->data.i = 1;
        else
          new->data.i = 0;
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
      } else if (param->type == INT && sn->next->type == FLOAT) {
        new = new_syntax_node(INT);
        DL_APPEND(statement->params, new);
        if ((float)param->data.i > sn->next->data.f) 
          new->data.i = 1;
        else
          new->data.i = 0;
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
      } else if (param->type == FLOAT && sn->next->type == INT) {
        new = new_syntax_node(INT);
        DL_APPEND(statement->params, new);
        if (param->data.f > (float)sn->next->data.i) 
          new->data.i = 1;
        else
          new->data.i = 0;
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
      } else if (param->type == FLOAT && sn->next->type == FLOAT) {
        new = new_syntax_node(INT);
        DL_APPEND(statement->params, new);
        if (param->data.f > sn->next->data.f) 
          new->data.i = 1;
        else
          new->data.i = 0;
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
      } else {
        _debug_print_statement(statement);
      	printf("Could not > types: %i, %i\n", param->type, sn->next->type);
      }
      break;
    }
    case LESSEQUAL: {
      if (param == NULL || sn->next == NULL) break;
      normalize_left_right(param, sn->next);
      if (param->type == INT && sn->next->type == INT) {
        new = new_syntax_node(INT);
        DL_APPEND(statement->params, new);
        if (param->data.i <= sn->next->data.i) 
          new->data.i = 1;
        else
          new->data.i = 0;
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
      } else if (param->type == INT && sn->next->type == FLOAT) {
        new = new_syntax_node(INT);
        DL_APPEND(statement->params, new);
        if ((float)param->data.i <= sn->next->data.f) 
          new->data.i = 1;
        else
          new->data.i = 0;
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
      } else if (param->type == FLOAT && sn->next->type == INT) {
        DL_APPEND(statement->params, new);
        new = new_syntax_node(INT);
        if (param->data.f <= (float)sn->next->data.i) 
          sn->data.i = 1;
        else
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
          sn->data.i = 0;
      } else if (param->type == FLOAT && sn->next->type == FLOAT) {
        new = new_syntax_node(INT);
        DL_APPEND(statement->params, new);
        if (param->data.f <= sn->next->data.f) 
          sn->data.i = 1;
        else
          sn->data.i = 0;
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
      } else {
        _debug_print_statement(statement);
      	printf("Could not <= types: %i, %i\n", param->type, sn->next->type);
      }
      break;
    }
    case MOREEQUAL: {
      if (param == NULL || sn->next == NULL) break;
      normalize_left_right(param, sn->next);
      if (param->type == INT && sn->next->type == INT) {
        new = new_syntax_node(INT);
        DL_APPEND(statement->params, new);
        if (param->data.i >= sn->next->data.i) 
          new->data.i = 1;
        else
          new->data.i = 0;
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
      } else if (param->type == INT && sn->next->type == FLOAT) {
        new = new_syntax_node(INT);
        DL_APPEND(statement->params, new);
        if ((float)param->data.i >= sn->next->data.f) 
          new->data.i = 1;
        else
          new->data.i = 0;
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
      } else if (param->type == FLOAT && sn->next->type == INT) {
        new = new_syntax_node(INT);
        DL_APPEND(statement->params, new);
        if (param->data.f >= (float)sn->next->data.i) 
          new->data.i = 1;
        else
          new->data.i = 0;
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
      } else if (param->type == FLOAT && sn->next->type == FLOAT) {
        new = new_syntax_node(INT);
        DL_APPEND(statement->params, new);
        if (param->data.f >= sn->next->data.f) 
          new->data.i = 1;
        else
          new->data.i = 0;
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
      } else {
        _debug_print_statement(statement);
      	printf("Could not >= types: %i, %i\n", param->type, sn->next->type);
      }
      break;
    }
    case NOTEQUAL: {
      if (param == NULL || sn->next == NULL) break;
      normalize_left_right(param, sn->next);
      if (param->type == STRING && sn->next->type == STRING) {
        new = new_syntax_node(INT);
        DL_APPEND(statement->params, new);
        if (strcmp(param->data.s, sn->next->data.s) != 0) 
          new->data.i = 1;
        else
          new->data.i = 0;
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
      } else if (param->type == INT && sn->next->type == INT) {
        new = new_syntax_node(INT);
        DL_APPEND(statement->params, new);
        if (param->data.i != sn->next->data.i) 
          new->data.i = 1;
        else
          new->data.i = 0;
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
      } else if (param->type == INT && sn->next->type == FLOAT) {
        new = new_syntax_node(INT);
        DL_APPEND(statement->params, new);
        if ((float)param->data.i != sn->next->data.f) 
          new->data.i = 1;
        else
          new->data.i = 0;
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
      } else if (param->type == FLOAT && sn->next->type == INT) {
        new = new_syntax_node(INT);
        DL_APPEND(statement->params, new);
        if (param->data.f != (float)sn->next->data.i) 
          new->data.i = 1;
        else
          new->data.i = 0;
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
      } else if (param->type == FLOAT && sn->next->type == FLOAT) {
        new = new_syntax_node(INT);
        DL_APPEND(statement->params, new);
        if (param->data.f != sn->next->data.f) 
          new->data.i = 1;
        else
          new->data.i = 0;
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
      } else {
        _debug_print_statement(statement);
	      printf("Could not != types: %i, %i\n", param->type, sn->next->type);
      }
      break;
    }
    case AND: {
      if (param == NULL || sn->next == NULL) break;
      new = new_syntax_node(INT);
      new->data.i = 1;
      DL_APPEND(statement->params, new);
      switch(param->type) {
        case INT:
          if (param->data.i == 0) {
            new->data.i = 0;
          }
          break;
        case STRING:
          if (param->data.s[0] == '\0') {
            new->data.i = 0;
          }
          break;
        case FLOAT:
          if (param->data.f == 0) {
            new->data.i = 0;
          }
          break;
        case LIST: {
          ListTypeEntry *lte = get_list(param->data.i);
          if (lte == NULL || lte->head == NULL) {
            new->data.i = 0;
          }
          break;
        }
      }
      switch(sn->next->type) {
        case INT:
          if (sn->next->data.i == 0) {
            new->data.i = 0;
          }
          break;
        case STRING:
          if (sn->next->data.s[0] == '\0') {
            new->data.i = 0;
          }
          break;
        case FLOAT:
          if (sn->next->data.f == 0) {
            new->data.i = 0;
          }
          break;
        case LIST: {
          ListTypeEntry *lte = get_list(sn->next->data.i);
          if (lte == NULL || lte->head == NULL) {
            new->data.i = 0;
          }
          break;
        }
      }
      DL_DELETE(statement->params, param);
      free_SyntaxNode(param);
      break;
    }
    case OR: {
      if (param == NULL || sn->next == NULL) break;
      new = new_syntax_node(INT);
      new->data.i = 0;
      DL_APPEND(statement->params, new);
      switch(param->type) {
        case INT:
          if (param->data.i != 0) {
            new->data.i = 1;
          }
          break;
        case STRING:
          if (param->data.s[0] != '\0') {
            new->data.i = 1;
          }
          break;
        case FLOAT:
          if (param->data.f != 0) {
            new->data.i = 1;
          }
          break;
        case LIST: {
          ListTypeEntry *lte = get_list(param->data.i);
          if (lte != NULL && lte->head != NULL) {
            new->data.i = 1;
          }
          break;
        }
      }
      switch(sn->next->type) {
        case INT:
          if (sn->next->data.i != 0) {
            new->data.i = 1;
          }
          break;
        case STRING:
          if (sn->next->data.s[0] != '\0') {
            new->data.i = 1;
          }
          break;
        case FLOAT:
          if (sn->next->data.f != 0) {
            new->data.i = 1;
          }
          break;
        case LIST: {
          ListTypeEntry *lte = get_list(sn->next->data.i);
          if (lte != NULL && lte->head != NULL) {
            new->data.i = 0;
          }
          break;
        }
      }
      DL_DELETE(statement->params, param);
      free_SyntaxNode(param);
      break;
    }
    case NOT: {
      if (sn->next == NULL) break;
      new = new_syntax_node(INT);
      new->data.i = 1;
      DL_APPEND(statement->params, new);
      switch(sn->next->type) {
        case INT:
          if (sn->next->data.i != 0) {
            new->data.i = 0;
          }
          break;
        case STRING:
          if (sn->next->data.s[0] != '\0') {
            new->data.i = 0;
          }
          break;
        case FLOAT:
          if (sn->next->data.f != 0) {
            new->data.i = 0;
          }
          break;
        case LIST: {
          ListTypeEntry *lte = get_list(sn->next->data.i);
          if (lte != NULL && lte->head != NULL) {
            new->data.i = 0;
          }
          break;
        }
      }
      break;
    }
    case NOR: {
      if (param == NULL || sn->next == NULL) break;
      new = new_syntax_node(INT);
      new->data.i = 1;
      DL_APPEND(statement->params, new);
      switch(param->type) {
        case INT:
          if (param->data.i != 0) {
            new->data.i = 0;
          }
          DL_DELETE(statement->params, param);
          break;
        case STRING:
          if (param->data.s[0] != '\0') {
            new->data.i = 0;
          }
          DL_DELETE(statement->params, param);
          break;
        case FLOAT:
          if (param->data.f != 0) {
            new->data.i = 0;
          }
          DL_DELETE(statement->params, param);
          break;
        case LIST: {
          ListTypeEntry *lte = get_list(param->data.i);
          if (lte != NULL && lte->head != NULL) {
            new->data.i = 0;
          }
          DL_DELETE(statement->params, param);
          break;
        }
      }
      switch(sn->next->type) {
        case INT:
          if (param->data.i == 0) {
            new->data.i = 0;
          }
          break;
        case STRING:
          if (param->data.s[0] == '\0') {
            new->data.i = 0;
          }
          break;
        case FLOAT:
          if (param->data.f == 0) {
            new->data.i = 0;
          }
          break;
        case LIST: {
          ListTypeEntry *lte = get_list(param->data.i);
          if (lte == NULL || lte->head == NULL) {
            new->data.i = 0;
          }
          break;
        }
      }
      break;
    }
    case IN: { // Ive decided for now not to support list in list checks
      if (param == NULL || sn->next == NULL) break;
      if (sn->next->type == LIST) {
        new = new_syntax_node(INT);
        new->data.i = 0;
        ListTypeEntry *lte = get_list(sn->next->data.i);
        if (lte == NULL) break;
        SyntaxNode *find = lte->head;
        DL_FOREACH(lte->head, find) {
          if (find->type != param->type) continue;
          if (find->type == STRING) {
            if (strcmp(find->data.s, param->data.s) == 0) {
              new->data.i = 1;
              break;
            }
          } else if (find->type == FLOAT) {
            if (find->data.i == param->data.i) {
              new->data.i = 1;
              break;          
            }
          } else if (find->type == INT) {
            if (find->data.i == param->data.i) {
              new->data.i = 1;
              break;            
            }
          } else {
            new->data.i = 1;
            break;            
          }
        }
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
        DL_APPEND(statement->params, new);
        break;
      } else if (sn->next->type == STRING) {
        char* test = NULL;
        switch (param->type) {
          case INT: {
            int size = sprintf(NULL, 0, "%d", param->data.i);
            char* buffer = malloc(size + 1);
            sprintf(buffer, "%i", param->data.i);
            test = strstr(sn->next->data.s, buffer);
            free(buffer);
            break;
          }
          case FLOAT: {
            int size = sprintf(NULL, 0, "%d", param->data.f);
            char* buffer = malloc(size + 1);
            sprintf(buffer, "%f", param->data.f);
            test = strstr(sn->next->data.s, buffer);
            free(buffer);
            break;
          }
          case STRING: {
            test = strstr(sn->next->data.s, param->data.s);
            break;
          }
        }
        new = new_syntax_node(INT);
        new->data.i = test == NULL;

        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
        DL_APPEND(statement->params, new);
      }
    }
    case AT:{
      if (param == NULL || sn->next == NULL) break;
      if (param->type != INT) break;
      if (sn->next->type == LIST) {
        ListTypeEntry *lte = get_list(sn->next->data.i);
        if (lte == NULL) break;
        SyntaxNode *find = lte->head;
        for (int i = param->data.i; i>0; i--) {
          if (find == NULL) break;
          find = find->next;
        }
        if (find == NULL) break;
        new = copy_SyntaxNode(find);
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
        DL_APPEND(statement->params, new);
        break;
      } else if (sn->next->type == STRING) {
        if (param->data.i >= strlen(sn->next->data.s)) break;
        new = new_syntax_node(STRING);
        new->data.s = malloc(2);
        strcpy(new->data.s, &(sn->next->data.s[param->data.i]));
        DL_DELETE(statement->params, param);
        free_SyntaxNode(param);
        DL_APPEND(statement->params, new);
      } 
    }
    case CASTINT: {
      if (sn->next == NULL) break;
      switch (sn->type) {
        case INT: {
          new = copy_SyntaxNode(sn->next);
          DL_APPEND(statement->params, new);
          break;
        }
        case FLOAT: {
          new = new_syntax_node(INT);
          new->data.i = (int)sn->next->data.i;
          DL_APPEND(statement->params, new);
          break;
        }
        case STRING: {
          new = new_syntax_node(INT);
          new->data.i = atoi(sn->next->data.s);
          DL_APPEND(statement->params, new);
          break;
        }
      }
      break;
    }
    case CASTSTR: {
      if (sn->next == NULL) break;
      switch (sn->type) {
        case INT: {
          new = new_syntax_node(STRING);
          int size = sprintf(NULL, 0, "%i", sn->next->data.i);
          new->data.s = malloc(size+1);
          sprintf(new->data.s, "%i", sn->next->data.i);
          DL_APPEND(statement->params, new);
          break;
        }
        case FLOAT: {
          new = new_syntax_node(STRING);
          int size = sprintf(NULL, 0, "%f", sn->next->data.f);
          new->data.s = malloc(size+1);
          sprintf(new->data.s, "%f", sn->next->data.f);
          DL_APPEND(statement->params, new);
          break;
        }
        case STRING: {
          new = copy_SyntaxNode(sn->next);
          DL_APPEND(statement->params, new);
          break;
        }
      }
      break;
    }
    case MIN: {
      if (sn->next == NULL || sn->next->next == NULL) break;
      skipNbr = 2;
      if (sn->next->type != INT && sn->next->type != FLOAT) {
        _debug_print_statement(statement);
        printf("Called min on non numeric type: %i\n", sn->next->type);
        break;
      }
      if (sn->next->next->type != INT && sn->next->next->type != FLOAT) {
        _debug_print_statement(statement);
        printf("Called min on non numeric type: %i\n", sn->next->next->type);
        break;
      }
      switch (sn->next->type + (sn->next->next->type * 2)) {
        case (INT + (INT * 2)): {
          if (sn->next->data.i < sn->next->next->data.i) {
            new = copy_SyntaxNode(sn->next);
          } else {
            new = copy_SyntaxNode(sn->next->next);
          }
          break;
        }
        case (INT + (FLOAT * 2)): {
          if (sn->next->data.i < sn->next->next->data.f) {
            new = copy_SyntaxNode(sn->next);
          } else {
            new = copy_SyntaxNode(sn->next->next);
          }
          break;
        }
        case (FLOAT + (INT * 2)): {
          if (sn->next->data.f < sn->next->next->data.i) {
            new = copy_SyntaxNode(sn->next);
          } else {
            new = copy_SyntaxNode(sn->next->next);
          }
          break;
        }
        case (FLOAT + (FLOAT * 2)): {
          if (sn->next->data.f < sn->next->next->data.f) {
            new = copy_SyntaxNode(sn->next);
          } else {
            new = copy_SyntaxNode(sn->next->next);
          }
          break;
        }
      }
      if (new == NULL) break;
      DL_APPEND(statement->params, new);
      break;
    }
    case MAX: {
      if (sn->next == NULL || sn->next->next == NULL) break;
      skipNbr = 2;
      if (sn->next->type != INT && sn->next->type != FLOAT) {
        _debug_print_statement(statement);
        printf("Called max on non numeric type: %i\n", sn->next->type);
        break;
      }
      if (sn->next->next->type != INT && sn->next->next->type != FLOAT) {
        _debug_print_statement(statement);
        printf("Called max on non numeric type: %i\n", sn->next->next->type);
        break;
      }
      switch (sn->next->type + (sn->next->next->type * 2)) {
        case (INT + (INT * 2)): {
          if (sn->next->data.i > sn->next->next->data.i) {
            new = copy_SyntaxNode(sn->next);
          } else {
            new = copy_SyntaxNode(sn->next->next);
          }
          break;
        }
        case (INT + (FLOAT * 2)): {
          if (sn->next->data.i > sn->next->next->data.f) {
            new = copy_SyntaxNode(sn->next);
          } else {
            new = copy_SyntaxNode(sn->next->next);
          }
          break;
        }
        case (FLOAT + (INT * 2)): {
          if (sn->next->data.f > sn->next->next->data.i) {
            new = copy_SyntaxNode(sn->next);
          } else {
            new = copy_SyntaxNode(sn->next->next);
          }
          break;
        }
        case (FLOAT + (FLOAT * 2)): {
          if (sn->next->data.f > sn->next->next->data.f) {
            new = copy_SyntaxNode(sn->next);
          } else {
            new = copy_SyntaxNode(sn->next->next);
          }
          break;
        }
      }
      if (new == NULL) break;
      DL_APPEND(statement->params, new);
      break;
    }
    case LEN: {
      if (sn->next == NULL || (sn->next->type != LIST && sn->next->type != STRING)) break;
      if (sn->next->type == LIST) {
        ListTypeEntry *lte = get_list(sn->next->data.i);
        if (lte == NULL) break;
        SyntaxNode *tmp;
        new = new_syntax_node(INT);
        DL_COUNT(lte->head, tmp, new->data.i);
        DL_APPEND(statement->params, new);
      } else if (sn->next->type == STRING) {
        new = new_syntax_node(INT);
        new->data.i = strlen(sn->next->data.s);
        DL_APPEND(statement->params, new);
      }
      break;
    }
    case COUNTOF: {
      if (sn->next == NULL || sn->next->next == NULL || (sn->next->type != LIST && sn->next->type != STRING)) break;
      if (sn->next->type == LIST) {
        ListTypeEntry *lte = get_list(sn->next->data.i);
        if (lte == NULL) break;
        SyntaxNode *check;
        int count = 0;
        DL_FOREACH(lte->head, check) {
          if (check->type != sn->next->next->type) continue;
          switch (check->type) {
            case INT: {
              if (check->data.i == sn->next->next->data.i) count++;
              break;
            }
            case FLOAT: {
              if (check->data.f == sn->next->next->data.f) count++;
              break;
            }
            case STRING: {
              if (strcmp(check->data.s, sn->next->next->data.s) == 0) count++;
              break;
            }
            case NONE:
              count++;
              break;
          }
        }
        new = new_syntax_node(INT);
        new->data.i = count;
        DL_APPEND(statement->params, new);
      } else if (sn->next->type == STRING) {
        int count = 0;
        int i, j, k;
        
        for (i = 0; i < strlen(sn->next->data.s); i++) {
            j = 0;
            k = i;
            while (sn->next->data.s[k] == sn->next->next->data.s[j] && j < strlen(sn->next->next->data.s)) {
                k++;
                j++;
            }
            if (j == strlen(sn->next->next->data.s)) {
                count++;
            }
        }
        new = new_syntax_node(INT);
        new->data.i = count;
        DL_APPEND(statement->params, new);
      }
      skipNbr = 2;
      break;
    }
    case EXISTS: {
      if (sn->next == NULL || sn->next->type != STRING) continue;
      new = new_syntax_node(INT);
      new->data.i = exists(sn->next->data.s);
      DL_APPEND(statement->params, new);
      break;
    }
    case HASFRAME: {
      if (sn->next == NULL || sn->next->type != STRING) continue;
      new = new_syntax_node(INT);
      new->data.i = has_frame(sn->next->data.s);
      break;
    }
    case CHOICEOF: {
      if (sn->next == NULL || sn->next->type != LIST) break;
      ListTypeEntry *lte = get_list(sn->next->data.i);
      if (lte == NULL) break;
      SyntaxNode *check;
      int length, rando;
      DL_COUNT(lte->head, check, length);
      rando = rand() % length;
      SyntaxNode *find = lte->head;
      for (int i = rando; i>0; i--) {
        if (find == NULL) break;
        find = find->next;
      }
      if (find == NULL) break;
      new = copy_SyntaxNode(find);
      DL_APPEND(statement->params, new);
      break;
    }
    case INWORLD: {
      if (sn->next == NULL || sn->next->type != STRING) continue;
      World* world = get_world(worldKey);
      if (world == NULL) continue;
      new = new_syntax_node(INT);
      new->data.i = 0;
      DL_APPEND(statement->params, new);
      ActorEntry *ae;
      DL_FOREACH(world->actors, ae) {
        if (strcmp(ae->actorKey, sn->next->data.s) != 0) continue;
        new->data.i = 1;
        break;
      }
      break;
    }
    case ISFRAME: {
      if (sn->next == NULL || sn->next->type != STRING) continue;
      Frame *f = get_frame(sn->next->data.s);
      new = new_syntax_node(INT);
      new->data.i = f == NULL;
      DL_APPEND(statement->params, new);
      break;
    }
    case ISINPUTSTATE: {
      if (sn->next == NULL || sn->next->type != STRING) continue;
      InputState *is = get_input_state(sn->next->data.s);
      new = new_syntax_node(INT);
      new->data.i = is == NULL;
      DL_APPEND(statement->params, new);
      break;
    }
    case ABS: {
      if (sn->next == NULL) break;
      if (sn->next->type != INT && sn->next->type != FLOAT) break;
      switch(sn->next->type) {
        case INT:
          new = new_syntax_node(INT);
          new->data.i = abs(sn->next->data.i);
          DL_APPEND(statement->params, new);
          break;
        case FLOAT:
          new = new_syntax_node(FLOAT);
          new->data.i = fabs(sn->next->data.f);
          DL_APPEND(statement->params, new);
          break;
      }
      break;
    }
    case RANGE: {
      if (sn->next == NULL || sn->next->type != INT) continue;
      clear_list(0);
      ListTypeEntry *lte = get_list(0);
      for (int i = 0; i < sn->next->data.i; i++) {
        SyntaxNode *listNode = new_syntax_node(INT);
        listNode->data.i = i;
        DL_APPEND(lte->head, listNode);
      }
      new = new_syntax_node(LIST);
      new->data.i = 0;
      DL_APPEND(statement->params, new);
      break;
    }
    }
  }
}

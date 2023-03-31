#include "scripts.h"
#include "scriptdata.h"
#include "utlist.h"
#include "actors.h"
#include "worlds.h"

ScriptMap *SCRIPT_MAPS[SCRIPT_MAP_SIZE];
int BUFFER[512];
int PARAMS[512];

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

void resolve_script(int scriptIdx, Actor* self, Actor* related, World* world) {

}


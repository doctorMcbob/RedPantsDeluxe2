#include "floatmachine.h"
#include "frames.h"
#include "scriptdata.h"
#include "scripts.h"
#include "stdio.h"
#include "stringmachine.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

TTF_Font *font;

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

void _debug_print_operator(int operator) {
  switch (operator) {
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
}

void print_statement(int statementKey) {
  int sp = statementKey;
  int verb = SCRIPTS[sp++];
  _debug_print_verb(verb);
  while (SCRIPTS[sp] != -1000) {
    int type = SCRIPTS[sp++];
    switch (type) {
    case OPERATOR:
      _debug_print_operator(SCRIPTS[sp++]);
      break;
    case INT:
      printf("%i ", SCRIPTS[sp++]);
      break;
    case STRING:
      printf("%s ", get_string(SCRIPTS[sp++]));
      break;
    case FLOAT:
      printf("%f ", get_float_literal(SCRIPTS[sp++]));
      break;
    case LIST:
      printf("[] ");
      break;
    case NONE:
      printf("None ");
      break;
    case DOT:
      printf(". ");
      break;
    case QRAND:
      printf("RAND? ");
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
  }
  printf("\n");
}

void draw_debug_overlay(World *world, SDL_Renderer *rend, Frame *frame) {
  int mouseX, mouseY;
  SDL_GetMouseState(&mouseX, &mouseY);
  int hasTextDrawn = 0;
  for (int i = 0; i < WORLD_BUFFER_SIZE; i++) {
    if (world->actors[i] == -1) {
      break;
    }
    Actor *a;
    a = get_actor(world->actors[i]);
    if (a == NULL) {
      continue;
    }
    SDL_Rect ECB = {a->ECB.x, a->ECB.y, a->ECB.w, a->ECB.h};

    scrolled(&ECB, frame);

    SDL_SetRenderDrawColor(rend, 0, 0, 255, 255);

    SDL_RenderDrawRect(rend, &ECB);
    BoxMapEntry *bme;
    bme = get_hurtboxes_for_actor(a);
    if (bme != NULL) {
      for (int i = 0; i < bme->count; i++) {
        SDL_Rect hurtbox = {bme->rect[i].x, bme->rect[i].y, bme->rect[i].w,
                            bme->rect[i].h};
        translate_rect_by_actor(a, &hurtbox);
        scrolled(&hurtbox, frame);
        SDL_SetRenderDrawColor(rend, 0, 255, 0, 255);
        SDL_RenderDrawRect(rend, &hurtbox);
      }
    }

    bme = get_hitboxes_for_actor(a);
    if (bme != NULL) {
      for (int i = 0; i < bme->count; i++) {
        SDL_Rect hitbox = {bme->rect[i].x, bme->rect[i].y, bme->rect[i].w,
                           bme->rect[i].h};
        translate_rect_by_actor(a, &hitbox);
        scrolled(&hitbox, frame);
        SDL_SetRenderDrawColor(rend, 255, 0, 0, 255);
        SDL_RenderDrawRect(rend, &hitbox);
      }
    }

    if (!hasTextDrawn) {
      if (mouseX >= ECB.x && mouseX < ECB.x + ECB.w && mouseY >= ECB.y &&
          mouseY < ECB.y + ECB.h) {
        char data[100]; // buffer to hold the formatted string
        sprintf(data, "%s %s:%iT?%i", get_string(a->name), get_string(a->state),
                a->frame, a->tangible);

        SDL_Color textColor = {0, 0, 0};
        SDL_Surface *surface = TTF_RenderText_Solid(font, data, textColor);

        SDL_Texture *Message = SDL_CreateTextureFromSurface(rend, surface);
        SDL_Rect Message_rect = {mouseX, mouseY, surface->w, surface->h};
        SDL_RenderCopy(rend, Message, NULL, &Message_rect);

        SDL_FreeSurface(surface);
        SDL_DestroyTexture(Message);
        hasTextDrawn = 1;
      }
    }
  }
  char data[100]; // buffer to hold the formatted string
  sprintf(data, "ACTOR COUNT: %i", DEEPEST_ACTOR);
  SDL_Color textColor = {0, 0, 0};
  SDL_Surface *surface = TTF_RenderText_Solid(font, data, textColor);

  SDL_Texture *Message = SDL_CreateTextureFromSurface(rend, surface);
  SDL_Rect Message_rect = {16, 16, surface->w, surface->h};
  SDL_RenderCopy(rend, Message, NULL, &Message_rect);

  SDL_FreeSurface(surface);
  SDL_DestroyTexture(Message);
}

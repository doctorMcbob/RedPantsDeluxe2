#ifndef DEBUG_LOAD
#define DEBUG_LOAD 1

void print_statement(int statementKey);
void _debug_print_operator(int operator);
void draw_debug_overlay(World* world, SDL_Renderer* rend, Frame* frame);
#endif

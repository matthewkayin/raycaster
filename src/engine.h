#pragma once

#include "state.h"

#include <stdbool.h>

extern const int SCREEN_WIDTH;
extern const int SCEEN_HEIGHT;

bool engine_init();
void engine_quit();

void engine_set_resolution(int width, int height);
void engine_toggle_fullscreen();

void engine_clock_init();
float engine_clock_tick();

void engine_render_preview(State* state);
void engine_render_state(State* state);

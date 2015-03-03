#ifndef OUTPUT_H
#define OUTPUT_H

#include "snake.h"

#if DEBUG & (1 | 2)
void setDebugMap(int16_t board[HEIGHT][WIDTH]);
void drawDebug();
void clearDebug();
#endif

void Output_clear();
void Output_finalize();
void Output_flush(bool ioflush);
void Output_init();
void Output_redraw(GameState *state);
void Output_update(GameState *state, Point p); //Try to lump calls to identical points to optimize for branch prediction
#endif

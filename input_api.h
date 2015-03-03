#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>

#include "snake.h"

typedef struct _Input_Module{
    bool (*init)();
    Direction (*run)(GameState *state, uint16_t snakeNo);
    void (*finalize)();
} Input_Module;

#endif

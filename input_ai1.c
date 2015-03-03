#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "input_ai1.h"
#include "snake.h"

static Direction ai1_run(GameState *state, uint16_t snakeNo)
{
    //Find poisonous moves
    unsigned int oldGood = 1, good = 0;
    CellState dboard[HEIGHT][WIDTH];
    memcpy(dboard, state->board, sizeof(state->board));
    while(oldGood != good)
    {
        oldGood = good;
        good = 0;

        int i, j;
        for(i = 0; i < HEIGHT; i++)
            for(j = 0; j < WIDTH; j++)
            {
                unsigned char bad = 0;
                Point tests[] = {
                    {state->snakes[snakeNo].head.x, state->snakes[snakeNo].head.y - 1},
                    {state->snakes[snakeNo].head.x, state->snakes[snakeNo].head.y + 1},
                    {state->snakes[snakeNo].head.x - 1, state->snakes[snakeNo].head.y},
                    {state->snakes[snakeNo].head.x + 1, state->snakes[snakeNo].head.y}
                };
                int k;
                for(k = 0; k < 4; k++)
                {
                    CellState c = dboard[tests[k].y][tests[k].x];
                    if(!(c == CELL_EMPTY || c == CELL_FOOD))
                        bad++;
                }
                if(bad > 2)
                    dboard[i][j] = CELL_HORWALL;
                else
                    good++;
            }
    }


    //What is around me?
    int i;
    Point tests[] = {
        {state->snakes[snakeNo].head.x, state->snakes[snakeNo].head.y - 1},
        {state->snakes[snakeNo].head.x, state->snakes[snakeNo].head.y + 1},
        {state->snakes[snakeNo].head.x - 1, state->snakes[snakeNo].head.y},
        {state->snakes[snakeNo].head.x + 1, state->snakes[snakeNo].head.y}
    };

    //Make sure a move exists
    bool movable = false;
    for(i = 0; i < 4; i++)
    {
        CellState c = dboard[tests[i].y][tests[i].x];
        if(c == CELL_EMPTY || c == CELL_FOOD)
        {
            movable = true;
            break;
        }
    }
    
    if(!movable)
        return DIR_NONE;

    //Randomly select a move
    while(true)
    {
        i = rand() % 4;
        CellState c = dboard[tests[i].y][tests[i].x];
        if(c == CELL_EMPTY || c == CELL_FOOD)
            return i;
    }
}

static bool ai1_init()
{
}

static void ai1_finalize()
{
}

const Input_Module Input_AI1 = (Input_Module){&ai1_init, &ai1_run, &ai1_finalize};

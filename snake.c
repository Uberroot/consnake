#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "input_api.h"
#include "input_ai1.h"
//#include "input_ai2.h"
//#include "input_ai3.h"
#include "input_ai4.h"
#include "input_user1.h"
#include "output.h"
#include "snake.h"

bool interrupt = false;

//Offsets point based upon cell value(UP/DOWN/LEFT/RIGHT)
void cellPointTranslate(Point *p, CellState cell){
    switch(cell & 0xFFFF){
        case CELL_UP:
            p->y--;
            return;
        case CELL_DOWN:
            p->y++;
            return;
        case CELL_LEFT:
            p->x--;
            return;
        case CELL_RIGHT:
            p->x++;
            return;
        default:
            return;  
    }
}

static void updateCell(GameState *state, Point p, CellState val)
{
    state->board[p.y][p.x] = val;
#if !(DEBUG & (1 | 2))
    Output_update(state, p);
#endif
}

//Put val in a random unoccupied cell
static Point putRandCell(GameState *state, CellState val){
    Point p;
    do{
        p.x = rand() % WIDTH;
        p.y = rand() % HEIGHT;
    }
    while(state->board[p.y][p.x] != CELL_EMPTY);
    updateCell(state, p, val);
    return p;
}

//Move a snake. Check the result.
static MoveResult doMove(GameState* state, int snakeNo, Direction dir){
    Point newHead = state->snakes[snakeNo].head, newTail = state->snakes[snakeNo].tail;
    MoveResult ret = MOVE_LIVE;

    if(dir != DIR_NONE)
        updateCell(state, state->snakes[snakeNo].head, dir | (snakeNo << 16));
    
    //Where will the head be?
    cellPointTranslate(&newHead, state->board[state->snakes[snakeNo].head.y][state->snakes[snakeNo].head.x]);

    //What will the head intersect?
    switch(state->board[newHead.y][newHead.x]){
        //Food SCORE :D
        case CELL_FOOD:
            ret = MOVE_SCORE;

        //Nothingness, go ahead
        case CELL_EMPTY:
            break;

        default:
            switch(state->board[newHead.y][newHead.x]){
                CELL_HORWALL:
                CELL_VERTWALL:
                CELL_CORNER:
                    ret = MOVE_DEAD | (snakeNo << 16);
                    break;
                default:
                    ret = MOVE_DEAD | (state->board[newHead.y][newHead.x] & 0xFFFF0000);
                    break;
            }
            break;
    }
    updateCell(state, newHead, state->board[state->snakes[snakeNo].head.y][state->snakes[snakeNo].head.x]);

    //Move the tail accordingly
    if(ret != MOVE_SCORE){
        cellPointTranslate(&newTail, state->board[state->snakes[snakeNo].tail.y][state->snakes[snakeNo].tail.x]);
        updateCell(state, state->snakes[snakeNo].tail, CELL_EMPTY);
    }

    //Commit the move
    state->snakes[snakeNo].head = newHead;
    state->snakes[snakeNo].tail = newTail;
    return ret;
}

static void sigint(int val){
    interrupt = true;
}

int main(int argc, char **argv){
    Input_Module inputModules[] = {Input_User1, Input_AI1, Input_AI4};
    GameState state;
    int i, j, inputNum, inputMax;

    //Validate
    if(argc < 2)
    {
        printf("Usage: %s <input mode> [random seed]\n", argv[0]);
        return 1;
    }

    inputNum = atoi(argv[1]);
    inputMax = sizeof(inputModules) / sizeof(Input_Module) - 1;
    if(inputNum > inputMax)
    {
        printf("Invalid input module\nOptions are from 0 to %d\n", inputMax);
        return 1;
    }

    //Prep dynamic functions
    Input_Module inputter = inputModules[inputNum];

    //Init stuff
    signal(SIGINT, sigint);
    state.score = 0;
    state.deadcount = 0;
    if(argc > 2)
        srand(atoi(argv[2]));
    else
        srand(getpid() + time(NULL));
    inputter.init();
    Output_init();

    //Initialize the board
    for(i = 0; i < HEIGHT; i++)
        for(j = 0; j < WIDTH; j++)
            state.board[i][j] = CELL_EMPTY;

    //Add boarders
    for(i = 0; i < WIDTH; i++){
        state.board[0][i] = CELL_HORWALL;
        state.board[HEIGHT - 1][i] = CELL_HORWALL;
    }
    for(i = 0; i < HEIGHT; i++){
        state.board[i][0] = CELL_VERTWALL;
        state.board[i][WIDTH - 1] = CELL_VERTWALL;
    }
    state.board[0][0] = CELL_CORNER;
    state.board[0][WIDTH - 1] = CELL_CORNER;
    state.board[HEIGHT - 1][0] = CELL_CORNER;
    state.board[HEIGHT - 1][WIDTH - 1] = CELL_CORNER;


    //play
    for(i = 0; i < FOODS; i++)
        state.food[i] = putRandCell(&state, CELL_FOOD);
    for(i = 0; i < SNAKES; i++){
        state.snakes[i].length = 1;
        state.snakes[i].mood = SNAKE_FINE;
        state.snakes[i].dead = false;
        state.snakes[i].head = state.snakes[i].tail = putRandCell(&state, rand() % 4);
    }
    Output_clear();
    Output_redraw(&state);
    while(!interrupt && state.deadcount != SNAKES && state.score < MAX_SCORE){
        usleep(STEP);
        for(i = 0; i < SNAKES; i++){
            if(state.snakes[i].dead)
                continue;
            Direction d = inputter.run(&state, i);
            MoveResult res = doMove(&state, i, d);
            switch(res & 0xFFFF){
                case MOVE_SCORE:
                    if(++state.score < MAX_SCORE)
                        for(j = 0; j < FOODS; j++)
                            if(POINT_CMP(state.food[j], state.snakes[i].head)){
                                state.food[j] = putRandCell(&state, CELL_FOOD);
                                if (DEBUG & 16)
                                    printf("\a");
                                break;
                            }
                    state.snakes[i].length++;
                    break;
                case MOVE_DEAD:
                    state.snakes[res >> 16].dead = true;
                    state.deadcount++;
                    interrupt = true;
                    break;
            }
        }
        Output_updateScore(state.score);
#if DEBUG & (1 | 2)
        Output_redraw(&state);
#else
        Output_flush(true);
#endif
    }
    inputter.finalize();
    Output_finalize();

    return 0;
}

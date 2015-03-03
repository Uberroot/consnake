#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <assert.h>

#include "input_ai4.h"
#include "output.h"
#include "snake.h"
#include "mempool.h"

typedef struct _DNode{
    struct _DNode *next;
    Point point;
    MemPoolObject *mpo;
} DNode;
typedef struct _PathNode{
    struct PathNode *next;
    Direction dir;
} PathNode;

static MemPool *dnodePool = NULL;

static inline DNode* makeDNode(){
    MemPoolObject *tmp = MemPool_Alloc(dnodePool);
    ((DNode*)tmp->data)->mpo = tmp;
    return (DNode*)tmp->data;
}

//Determines cost at each point to reach nearest target point
static void costEval(GameState *state, int16_t dboard[HEIGHT][WIDTH], int16_t pinned[HEIGHT][WIDTH], DNode *targets){
    int i, j;

    //Crank out values
    dboard[targets->point.y][targets->point.x] = 0;
    DNode *dCur = makeDNode();
    dCur->next = NULL;
    dCur->point.x = targets->point.x;
    dCur->point.y = targets->point.y;
    DNode *others = dCur;
    for(targets = targets->next; targets; targets = targets->next){
        dboard[targets->point.y][targets->point.x] = 0;
        others->next = makeDNode();
        others = others->next;
        others->next = NULL;
        others->point.x = targets->point.x;
        others->point.y = targets->point.y;
    }
    while(dCur != NULL){
        DNode *eHead = NULL;

        //Determine edges
        DNode *eCur = NULL;
        while(dCur != NULL){
            Point tests[4] = {{dCur->point.x + 1, dCur->point.y},
                              {dCur->point.x - 1, dCur->point.y},
                              {dCur->point.x, dCur->point.y + 1},
                              {dCur->point.x, dCur->point.y - 1}};
            for(i = 0; i < 4; i++)
            {
                if(dboard[tests[i].y][tests[i].x] == -1){
                    dboard[tests[i].y][tests[i].x] = -2;
                    DNode *edge = makeDNode();
                    edge->next = NULL;
                    edge->point.x = tests[i].x;
                    edge->point.y = tests[i].y;
                    
                    if(eHead == NULL){
                        eHead = edge;
                        eCur = edge;
                    }
                    else{
                        eCur->next = edge;
                        eCur = edge;
                    }
                }
            }
            DNode *tmp = dCur;
            dCur = dCur->next;
            MemPool_Free(dnodePool, tmp->mpo);
        }
        
        //Score the board
        for(eCur = eHead; eCur != NULL; eCur = eCur->next){
            Point tests[4] = {{eCur->point.x + 1, eCur->point.y},
                              {eCur->point.x - 1, eCur->point.y},
                              {eCur->point.x, eCur->point.y + 1},
                              {eCur->point.x, eCur->point.y - 1}};
            
            int16_t min = 0x7FFF;
            for(i = 0; i < 4; i++)
            {
                int16_t score = dboard[tests[i].y][tests[i].x];
                if(score != -1 && score != -2 && score < min)
                    min = score;
            }
            dboard[eCur->point.y][eCur->point.x] = min + 1 + pinned[eCur->point.y][eCur->point.x];
        }

        dCur = eHead;
    }
    //getchar();
}

static Direction ai4_run(GameState* state, uint16_t snakeNo){
    Direction ret;

    //Compute a new path
    Point cur;
    int i, j;
    int16_t dboard[HEIGHT][WIDTH];
    int16_t snakeboard[HEIGHT][WIDTH];

    //init the board map
    memset(dboard, -1, sizeof(dboard));
    memset(snakeboard, 0, sizeof(snakeboard));
    for(i = 0; i < HEIGHT; i++)
        dboard[i][0] = dboard[i][WIDTH - 1] = 0x7FFF;
    for(i = 0; i < WIDTH; i++)
        dboard[0][i] = dboard[HEIGHT - 1][i] = 0x7FFF;
    
    //Create costs differentials for the snake's body
    j = 1;
    for(i = 0; i < SNAKES; i++){
        cur = state->snakes[i].tail;
        while(!(cur.x == state->snakes[i].head.x && cur.y == state->snakes[i].head.y)){
            dboard[cur.y][cur.x] = 0x7FFF;
            snakeboard[cur.y][cur.x] = j++;
            cellPointTranslate(&cur, state->board[cur.y][cur.x]);
        }
        snakeboard[cur.y][cur.x] = j++;
        dboard[state->snakes[i].head.y][state->snakes[i].head.x] = 0x7FFF;
    }

    DNode *targets = NULL, *ctarget = NULL;
    for(i = 0; i < FOODS; i++){
        DNode *tmp = makeDNode();
        if(targets == NULL)
            ctarget = targets = tmp;
        ctarget->next = tmp;
        ctarget = tmp;
        ctarget->point.x = state->food[i].x;
        ctarget->point.y = state->food[i].y;
        ctarget->next = NULL;
    }
    costEval(state, dboard, snakeboard, targets);
    ctarget = targets;
    while(ctarget){
        DNode *tmp = ctarget->next;
        MemPool_Free(dnodePool, ctarget->mpo);
        ctarget = tmp;
    }
#if DEBUG & (1 | 2)
    setDebugMap(dboard);
#endif

    state->snakes[snakeNo].mood = SNAKE_FINE;

    //Check number of unreachable cells
    int unreachable = 0;
    for(i = 0; i < HEIGHT; i++)
        for(j = 0; j < WIDTH; j++)
            if(dboard[i][j] == -1){
                state->snakes[snakeNo].mood = SNAKE_FLUSTERED;
                unreachable++;
            }

    //Calculate the path
    Point current = state->snakes[snakeNo].head;
    PathNode *currentPath = NULL;
    while(dboard[current.y][current.x] != 0){
         Point tests[4] = {{current.x, current.y - 1},
                          {current.x, current.y + 1},
                          {current.x - 1, current.y},
                          {current.x + 1, current.y}};
                          
        int16_t min = 0x7FFF;
        for(i = 0; i < 4; i++){
            int16_t score = dboard[tests[i].y][tests[i].x];
            if(score < min && score >= 0)
                min = score;
        }
        if(min == 0x7FFF){//We're stuck. Maybe we can get unstuck
            //getchar();
            state->snakes[snakeNo].mood = SNAKE_FRENZIED;
#if DEBUG & (1 | 2)
            fprintf(stderr, "Oh shit...\n");
#endif
            //Look for a free cell
            for(i = 0; i < 4; i++)
                if(-1 == dboard[tests[i].y][tests[i].x]){
                    min = -1;
                    break;
                }
            if(unreachable < state->snakes[snakeNo].length || min == 0x7FFF){ //We're screwed
#if DEBUG & (1 | 2)
                fprintf(stderr, "I'm screwed\n");
#endif
                state->snakes[snakeNo].mood = SNAKE_FUCKED;
                if(min == 0x7FFF)
                    break;
            }
#if DEBUG & (1 | 2)
            //getchar();
#endif
        }

        while(true)
        {
            for(i = 0; i < 4; i++)
                if(dboard[tests[i].y][tests[i].x] == min && (rand() % 2)){ //randomly pick from optimal paths
                    current = tests[i];
                    return i;
                }
       }
       if(min == -1)
           break;         
    }

    return DIR_NONE;
}

static bool ai4_init()
{
    assert(dnodePool == NULL);
    dnodePool = MemPool_Create(HEIGHT * WIDTH, sizeof(DNode));
}

static void ai4_finalize()
{
    assert(dnodePool);
    free(dnodePool);
    dnodePool = NULL;
}

const Input_Module Input_AI4 = (Input_Module){&ai4_init, &ai4_run, &ai4_finalize};

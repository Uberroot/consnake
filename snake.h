#ifndef SNAKE_H
#define SNAKE_H

#include <stdbool.h>

//Full terminal width for debug 1
//#define WIDTH  47
//#define HEIGHT 63

//A nice playable board
#define WIDTH  80
#define HEIGHT 30

//A quarter board / good for debugs
//#define WIDTH  40
//#define HEIGHT 15

//Full terminal width for game (pretty :D)
//#define WIDTH  234
//#define HEIGHT 63

#define SNAKES 1
#define FOODS  3
#define STEP   20000

//DEBUG | 1 = Print single point distance solution board to stderr
//DEBUG | 2 = Overlay game with solution board
//DEBUG | 4 = Print exit stats
//DEBUG | 8 = Print \a character after each explicit flush
//DEBUG | 16 = Print \a character after each score
//#define DEBUG (1 | 2 | 4)
#define DEBUG (0)

#define MAX_SCORE ((WIDTH - 2) * (HEIGHT - 2) - 1)
#define PPOINT_CMP(a, b) (a->x == b->x && a->y == b->y)
#define POINT_CMP(a, b) (a.x == b.x && a.y == b.y)

typedef enum eCellState{ //High 2 bytes contain snake number
    CELL_EMPTY    = 10,
    CELL_HORWALL  = 11,
    CELL_VERTWALL = 12,
    CELL_CORNER   = 13,
    CELL_FOOD     = 14,
    CELL_UP       = 0,
    CELL_DOWN     = 1,
    CELL_LEFT     = 2,
    CELL_RIGHT    = 3
} CellState;

typedef enum eMoveResult{
    MOVE_LIVE,
    MOVE_SCORE,
    MOVE_DEAD  //High 2 bytes will be the original owner, if any
} MoveResult;

typedef enum eDirection{
    DIR_UP    = CELL_UP,
    DIR_DOWN  = CELL_DOWN,
    DIR_LEFT  = CELL_LEFT,
    DIR_RIGHT = CELL_RIGHT,
    DIR_NONE  = CELL_EMPTY
} Direction;

typedef enum eSnakeState{
    SNAKE_FINE,         //No loops - I know what I'm doing
    SNAKE_FLUSTERED,    //Loops, but not blocking food - I know what I'm doing, but I might get lost soon
    SNAKE_FRENZIED,     //Loops blocking food - I have no idea what I'm doing
    SNAKE_FUCKED        //Stuck in loop / less space to move than length of body - I have no idea what I'm doing and will probably lose
} SnakeState;

typedef struct sPoint{
    int x;
    int y;
} Point;

typedef struct sSnake{
    bool dead;
    Point head;
    Point tail;
    int length;
    SnakeState mood;
} Snake;

typedef struct sGameState{
    unsigned int deadcount;
    unsigned int score;
    Snake snakes[SNAKES];
    Point food[FOODS];
    CellState board[HEIGHT][WIDTH];
} GameState;

void cellPointTranslate(Point *p, CellState cell);
#endif

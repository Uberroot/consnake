#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "output.h"

#define RGB_TO_ANSI(r, g, b) (16 + 36 * r + 6 * g + b)
#define BUFFER_SIZE (HEIGHT * WIDTH)

typedef enum{
    COLOR_DEFAULT,
    COLOR_RED, 
    COLOR_GREEN,
    COLOR_YELLOW,
    COLOR_BLUE,
    COLOR_VIOLET,
    COLOR_CYAN,
    COLOR_BG_RED,
    COLOR_BG_GREEN,
    COLOR_BG_YELLOW,
    COLOR_BG_BLUE,
    COLOR_BG_VIOLET,
    COLOR_BG_CYAN
} ColorIdx;

char *colors[] = {"\033[0m",
                  "\033[31m",
                  "\033[32m",
                  "\033[33m",
                  "\033[34m",
                  "\033[35m",
                  "\033[36m",
                  "\033[101m",
                  "\033[102m",
                  "\033[103m",
                  "\033[104m",
                  "\033[105m",
                  "\033[106m"};

uint8_t chromaColors[] = {
                        RGB_TO_ANSI(5, 0, 0),
                        RGB_TO_ANSI(5, 1, 0),
                        RGB_TO_ANSI(5, 2, 0),
                        RGB_TO_ANSI(5, 3, 0),
                        RGB_TO_ANSI(5, 4, 0),
                        RGB_TO_ANSI(5, 5, 0),
                        RGB_TO_ANSI(4, 5, 0),
                        RGB_TO_ANSI(3, 5, 0),
                        RGB_TO_ANSI(2, 5, 0),
                        RGB_TO_ANSI(1, 5, 0),
                        RGB_TO_ANSI(0, 5, 0),
                        RGB_TO_ANSI(0, 5, 1),
                        RGB_TO_ANSI(0, 5, 2),
                        RGB_TO_ANSI(0, 5, 3),
                        RGB_TO_ANSI(0, 5, 4),
                        RGB_TO_ANSI(0, 5, 5),
                        RGB_TO_ANSI(0, 4, 5),
                        RGB_TO_ANSI(0, 3, 5),
                        RGB_TO_ANSI(0, 2, 5),
                        RGB_TO_ANSI(0, 1, 5),
                        RGB_TO_ANSI(0, 0, 5),
                        RGB_TO_ANSI(1, 0, 5),
                        RGB_TO_ANSI(2, 0, 5),
                        RGB_TO_ANSI(3, 0, 5),
                        RGB_TO_ANSI(4, 0, 5),
                        RGB_TO_ANSI(5, 0, 5),
                        RGB_TO_ANSI(5, 0, 4),
                        RGB_TO_ANSI(5, 0, 3),
                        RGB_TO_ANSI(5, 0, 2),
                        RGB_TO_ANSI(5, 0, 1)
                    };

static char *buffer;
static char *buffer_next;
static char *buffer_max;
static char *lastCol;
static Point lastPos;

#if DEBUG & 4
static unsigned int numFlushes = 0;
static unsigned int numAutoFlushes = 0;
static unsigned int numCharsPrinted = 0;
static unsigned int numRedraws = 0;
static unsigned int numClears = 0;
static unsigned int numUpdates = 0;
#endif

#if DEBUG & (1 | 2)
static int16_t dboard[HEIGHT][WIDTH];
static int16_t dboard_interval = 1;
void setDebugMap(int16_t board[HEIGHT][WIDTH]){
    int i, j;

    memcpy(&dboard, board, HEIGHT * WIDTH * sizeof(int16_t));
    dboard_interval = 1;
    for(i = 0; i < HEIGHT; i++)
        for(j = 0; j < WIDTH; j++)
            if(board[i][j] != 0x7FFF && board[i][j] > dboard_interval)
                dboard_interval = board[i][j];
    int16_t tmp = dboard_interval;
    i = 30;
    do{
        dboard_interval = tmp / i--;
    } while(!dboard_interval);
    dboard_interval++;
}
void drawDebug(){
    int i, j;
    char colmaker[13];
}
#endif

static inline void Output_putchar(char c){
    putchar(c);
#if DEBUG & 4
    numCharsPrinted++;
#endif
}
static inline void Output_printf(const char* fmt, ...){
    va_list args;
    va_start(args, fmt);
#if DEBUG & 4
    numCharsPrinted += vprintf(fmt, args);
#else
    vprintf(fmt, args);
#endif
    va_end(args);
}
static inline void Output_fprintf(FILE *f, const char* fmt, ...){
    va_list args;
    va_start(args, fmt);
#if DEBUG & 4
    numCharsPrinted += vfprintf(f, fmt, args);
#else
    vfprintf(f, fmt, args);
#endif
    va_end(args);
}

void Output_clear(){
    Output_printf("\033[2J\033[;H");

#if DEBUG & 4
    numClears++;
#endif
}

void Output_finalize(){
    free(buffer);
    Output_printf("\033[0m\033[%u;0H\033[?25h", HEIGHT + 2);

#if DEBUG & 4
    printf("========== Output ==========\n");
    printf("numFlushes: %u\n", numFlushes);
    printf("numAutoFlushes: %u\n", numAutoFlushes);
    printf("numCharsPrinted: %u\n", numCharsPrinted);
    printf("numRedraws: %u\n", numRedraws);
    printf("numClears: %u\n", numClears);
    printf("numUpdates: %u\n", numUpdates);
#endif
}

void Output_flush(bool ioflush){
    if(buffer_next != buffer){
        Output_printf(buffer);
        buffer_next = buffer;
        *buffer = 0;
        if(ioflush){
#if DEBUG & 8
        Output_printf("\a");
#endif
        fflush(stdout);
        }
    }
#if DEBUG & 4
    numFlushes++;
#endif 
}

void Output_init(){
    buffer = (char*)malloc(BUFFER_SIZE * sizeof(char));
    buffer_next = buffer;
    buffer_max = buffer + BUFFER_SIZE - 1;
    lastCol = "";
    lastPos = (Point){-1, -1};

#if DEBUG & 4
    numFlushes = 0;
    numAutoFlushes = 0;
    numCharsPrinted = 0;
    numRedraws = 0;
    numClears = 0;
    numUpdates = 0;
#endif
    Output_printf("\033[?25l"); //No cursor
}

void Output_redraw(GameState *state){
    int i, j;
    char colmaker[13];
    char *snakeCol;

    
    //Draw
    Output_clear();
#if DEBUG & 1
    Output_fprintf(stderr, "\033[0;0H");
#endif
    for(i = 0; i < HEIGHT; i++){
        for(j = 0; j < WIDTH; j++){
#if DEBUG & (1 | 2)
            char *color;
            switch(dboard[i][j]){
                case 0x7FFF:
                    color = colors[COLOR_BG_CYAN];
                    break;
                case -1:
                    color = "\033[0m";
                    break;
                default:
                    sprintf(colmaker, "\033[38;5;%dm", chromaColors[dboard[i][j] / dboard_interval]);
                    color = colmaker;
            }
#endif
#if DEBUG & 1
        Output_fprintf(stderr, "%s%4x\033[0m,", color, (unsigned short)dboard[i][j]);
#endif
#if DEBUG & 2
        colmaker[2] = '4';
        Output_printf("%s", color);
#endif
            switch(state->board[i][j] & 0xFFFF){
                case CELL_EMPTY:
                    Output_printf("%s ", lastCol == colors[COLOR_DEFAULT] ? "" : (lastCol = colors[COLOR_DEFAULT]));
                    break;
                case CELL_HORWALL:
                    Output_printf("%s-", lastCol == colors[COLOR_DEFAULT] ? "" : (lastCol = colors[COLOR_DEFAULT]));
                    break;
                case CELL_VERTWALL:
                    Output_printf("%s|", lastCol == colors[COLOR_DEFAULT] ? "" : (lastCol = colors[COLOR_DEFAULT]));
                    break;
                case CELL_CORNER:
                    Output_printf("%s+", lastCol == colors[COLOR_DEFAULT] ? "" : (lastCol = colors[COLOR_DEFAULT]));
                    break;
                case CELL_FOOD:
                    Output_printf("%s*", lastCol == colors[COLOR_VIOLET] ? "" : (lastCol = colors[COLOR_VIOLET]));
                    break;
                default:
                    switch(state->snakes[state->board[i][j] >> 16].mood){
                        case SNAKE_FINE:
                            snakeCol = colors[COLOR_GREEN];
                            break;
                        case SNAKE_FLUSTERED:
                            snakeCol = colors[COLOR_CYAN];
                            break;
                        case SNAKE_FRENZIED:
                            snakeCol = colors[COLOR_YELLOW];
                            break;
                        case SNAKE_FUCKED:
                            snakeCol = colors[COLOR_RED];
                            break;
                    }
                    switch(state->board[i][j] & 0xFFFF){
                        case CELL_UP:
                            Output_printf("%s^", lastCol == snakeCol ? "" : (lastCol = colors[COLOR_VIOLET]));
                            break;
                        case CELL_DOWN:
                            Output_printf("%sv", lastCol == snakeCol ? "" : (lastCol = colors[COLOR_VIOLET]));
                            break;
                        case CELL_LEFT:
                            Output_printf("%s<", lastCol == snakeCol ? "" : (lastCol = colors[COLOR_VIOLET]));
                            break;
                        case CELL_RIGHT:
                            Output_printf("%s>", lastCol == snakeCol ? "" : (lastCol = colors[COLOR_VIOLET]));
                            break;
                    }
                    break;
            }
#if DEBUG & (1 | 2)
             Output_printf("\033[0m");
#endif
        }
        Output_putchar('\n');
#if DEBUG & 1
        Output_fprintf(stderr, "\n");
#endif
    }
    Output_printf("Score: %d", state->score);
#if DEBUG & 4
    numRedraws++;
#endif
    Output_fprintf(stdout, "\033[0;0H");
    lastPos = (Point){0, 0};
}

void Output_update(GameState *state, Point p){
    char *snakeCol;
    char news[256];
    char newPos[32];

    //For raw output
    p.x++;
    p.y++;

    //Craft control codes to position cursor
    if(p.x == lastPos.x && p.y == lastPos.y)
        newPos[0] = '\0';
    else
    {
        if(p.x != lastPos.x && p.y == lastPos.y)
            sprintf(newPos, "\033[%dG", p.x);
        else if(p.x == lastPos.x && p.y != lastPos.y)
            sprintf(newPos, "\033[%dd", p.y);
        else
            sprintf(newPos, "\033[%u;%uH", p.y, p.x);
    }

    //Update lastPos
    lastPos.x = p.x + 1;
    lastPos.y = p.y;

    //For board access
    p.x--;
    p.y--;

    //Determine the change
    switch(state->board[p.y][p.x] & 0xFFFF){
        case CELL_EMPTY:
            snprintf(news, 256, "%s ", newPos);
            break;
        case CELL_HORWALL:
            snprintf(news, 256, "%s-", newPos);
            break;
        case CELL_VERTWALL:
            snprintf(news, 256, "%s|", newPos);
            break;
        case CELL_CORNER:
            snprintf(news, 256, "%s+", newPos);
            break;
        case CELL_FOOD:
            snprintf(news, 256, "%s%s*", newPos, lastCol == colors[COLOR_VIOLET] ? "" : (lastCol = colors[COLOR_VIOLET]));
            break;
        default:
            switch(state->snakes[state->board[p.y][p.x] >> 16].mood){
                case SNAKE_FINE:
                    snakeCol = colors[COLOR_GREEN];
                    break;
                case SNAKE_FLUSTERED:
                    snakeCol = colors[COLOR_CYAN];
                    break;
                case SNAKE_FRENZIED:
                    snakeCol = colors[COLOR_YELLOW];
                    break;
                case SNAKE_FUCKED:
                    snakeCol = colors[COLOR_RED];
                    break;
            }
            switch(state->board[p.y][p.x] & 0xFFFF){
                case CELL_UP:
                    snprintf(news, 256, "%s%s^", newPos, lastCol == snakeCol ? "" : (lastCol = snakeCol));
                    break;
                case CELL_DOWN:
                    snprintf(news, 256, "%s%sv", newPos, lastCol == snakeCol ? "" : (lastCol = snakeCol));
                    break;
                case CELL_LEFT:
                    snprintf(news, 256, "%s%s<", newPos, lastCol == snakeCol ? "" : (lastCol = snakeCol));
                    break;
                case CELL_RIGHT:
                    snprintf(news, 256, "%s%s>", newPos, lastCol == snakeCol ? "" : (lastCol = snakeCol));
                    break;
            }
            break;
    }

    //Commit the change
    if(buffer_next + strlen(news) > buffer_max)
    {
        Output_flush(false);
#if DEBUG & 4
        numAutoFlushes++;
#endif
    }
    strncpy(buffer_next, news, buffer_max - buffer_next - 1);
    buffer_next += strlen(news);

#if DEBUG & 4
    numUpdates++;
#endif
}

void Output_updateScore(uint32_t score){
    static uint32_t oldScore = 0;
    char news[256];

    //Do we need to do anything?
    if(oldScore == score)
        return;
    oldScore = score;

    //Determine the change
    snprintf(news, 256, "%s\033[%u;%uH%u", lastCol == colors[COLOR_DEFAULT] ? "" : (lastCol = colors[COLOR_DEFAULT]), HEIGHT + 1, 8, score); //strlen("Score: ") == 7
    lastPos = (Point){-1, -1};
    
    //Commit the change
    if(buffer_next + strlen(news) > buffer_max)
    {
        Output_flush(false);
#if DEBUG & 4
        numAutoFlushes++;
#endif
    }
    strncpy(buffer_next, news, buffer_max - buffer_next - 1);
    buffer_next += strlen(news);

#if DEBUG & 4
    numUpdates++;
#endif
}


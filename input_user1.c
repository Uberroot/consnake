#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <assert.h>

#include "input_user1.h"
#include "output.h"
#include "snake.h"
#include "mempool.h"

//Stolen from somewhere on stack overflow...
static int kbhit(void){
  struct termios oldt, newt;
  int ch;
  int oldf;
 
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
  ch = getchar();
 
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);
 
  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }
 
  return 0;
}

bool user1_init()
{
    return true;
}

Direction user1_run(GameState *state, uint16_t snakeNo){
    static int last = 0;
    Direction ret;

    system("stty raw");
    if(!kbhit())
        ret = DIR_NONE;
    else{
        int c;
        do
            c = getchar();
        while(c == last && kbhit());
        last = c;
        switch(c){
            case 0x41:
            case 'a':
                ret = DIR_LEFT;
                break;
            case 0x43:
            case 'd':
                ret = DIR_RIGHT;
                break;
            case 0x1b:
            case 'w':
                ret = DIR_UP;
                break;
            case 0x5b:
            case 's':
                ret = DIR_DOWN;
                break;
            default:
                ret = DIR_NONE;
        }
    }
    system("stty -raw");
    return ret;
}

void user1_finalize()
{
    return;
}

const Input_Module Input_User1 = (Input_Module){&user1_init, &user1_run, &user1_finalize};

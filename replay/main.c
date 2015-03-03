#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int main(int argc, char *argv[])
{
    assert(argc == 2);
    FILE *f = fopen(argv[1], "r");
    while(!feof(f)){
        char c = fgetc(f);
        if(c == '\a')
        {
            getchar();
            printf("\033[A");
            fflush(stdout);
        }
        else
            putchar(c);
        //usleep(1000);
    }
    fclose(f);
    return 0;
}

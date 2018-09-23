#include "unistd.h"
#include "stdio.h"

int main()
{
    int counter = 0;
    while (1)
    {
        printf("%d\n", counter);
        counter++;
        sleep(1);
    }

    return 0;
}

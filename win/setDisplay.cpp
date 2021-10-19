#include "display_roi.h"

int main(int argc, char **argv)
{
    if (argc > 1)
    {
        start(argv[1]);
    }
    else
        start("../IMG20210928132544.jpg");
    return 0;
}
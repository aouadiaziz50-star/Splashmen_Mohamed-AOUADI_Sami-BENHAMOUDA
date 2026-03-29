#include "actions.h"

#define P2_ROW_LEN 99

char get_action(void)
{
    static int row = 0;
    static int col_step = 0;
    static int going_right = 0;
    char ret = ACTION_MOVE_L;

    if (col_step < P2_ROW_LEN)
    {
        ret = going_right ? ACTION_MOVE_R : ACTION_MOVE_L;
        col_step++;
    }
    else
    {
        ret = ACTION_MOVE_D;
        col_step = 0;
        going_right = !going_right;
        row++;
        if (row >= 99)
            row = 0;
    }

    return ret;
}

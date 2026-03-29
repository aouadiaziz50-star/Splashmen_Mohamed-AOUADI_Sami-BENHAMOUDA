#include "actions.h"

#define P1_STEP_MAX 80

char get_action(void)
{
    static int step = 0;
    char ret = ACTION_MOVE_R;

    if (step < 20)
        ret = ACTION_MOVE_R;
    else if (step < 40)
        ret = ACTION_MOVE_D;
    else if (step < 60)
        ret = ACTION_MOVE_L;
    else
        ret = ACTION_MOVE_U;

    step++;
    if (step >= P1_STEP_MAX)
        step = 0;

    return ret;
}

#include "actions.h"

#define P4_STEP_MAX 120

char get_action(void)
{
    static int step = 0;
    char ret = ACTION_MOVE_R;

    if (step < 30)
        ret = ACTION_MOVE_R;
    else if (step < 60)
        ret = ACTION_MOVE_D;
    else if (step < 90)
        ret = ACTION_MOVE_L;
    else
        ret = ACTION_MOVE_U;

    step++;
    if (step >= P4_STEP_MAX)
        step = 0;

    return ret;
}
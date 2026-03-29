#include "actions.h"

char get_action(void)
{
    static int segment_length = 8;
    static int segment_done = 0;
    static int segment_repeat = 0;
    static int direction = 0;
    char ret = ACTION_MOVE_R;

    switch (direction)
    {
        case 0:
            ret = ACTION_MOVE_R;
            break;
        case 1:
            ret = ACTION_MOVE_D;
            break;
        case 2:
            ret = ACTION_MOVE_L;
            break;
        default:
            ret = ACTION_MOVE_U;
            break;
    }

    segment_done++;
    if (segment_done >= segment_length)
    {
        segment_done = 0;
        direction = (direction + 1) % 4;
        segment_repeat++;

        if (segment_repeat >= 2)
        {
            segment_repeat = 0;
            segment_length += 4;
            if (segment_length > 60)
                segment_length = 8;
        }
    }

    return ret;
}

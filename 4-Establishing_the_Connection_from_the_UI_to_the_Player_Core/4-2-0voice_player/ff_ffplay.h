#ifndef FF_FFPLAY_H
#define FF_FFPLAY_H

#include "ffmsg_queue.h"

class FFPlayer
{
public:
    FFPlayer();
    int ffp_create();
    int ffp_prepare_async_l(char *filename);

    MessageQueue *msg_queue_;

};

#endif // FF_FFPLAY_H

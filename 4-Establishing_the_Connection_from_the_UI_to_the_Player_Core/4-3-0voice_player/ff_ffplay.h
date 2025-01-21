#ifndef FF_FFPLAY_H
#define FF_FFPLAY_H
#include <thread>
#include "ffmsg_queue.h"

class FFPlayer
{
public:
    FFPlayer();
    int ffp_create();
    int ffp_prepare_async_l(char *file_name);
    int stream_open( const char *file_name);
    MessageQueue msg_queue_;
    char *input_filename_;
    int read_thread();

    std::thread *read_thread_;
};

inline static void ffp_notify_msg1(FFPlayer *ffp, int what) {
    msg_queue_put_simple3(&ffp->msg_queue_, what, 0, 0);
}

inline static void ffp_notify_msg2(FFPlayer *ffp, int what, int arg1) {
    msg_queue_put_simple3(&ffp->msg_queue_, what, arg1, 0);
}

inline static void ffp_notify_msg3(FFPlayer *ffp, int what, int arg1, int arg2) {
    msg_queue_put_simple3(&ffp->msg_queue_, what, arg1, arg2);
}

inline static void ffp_notify_msg4(FFPlayer *ffp, int what, int arg1, int arg2, void *obj, int obj_len) {
    msg_queue_put_simple4(&ffp->msg_queue_, what, arg1, arg2, obj, obj_len);
}

inline static void ffp_remove_msg(FFPlayer *ffp, int what) {
    msg_queue_remove(&ffp->msg_queue_, what);
}



#endif // FF_FFPLAY_H

#include "ffmsg_queue.h"
extern "C" {
#include "libavutil/avstring.h"
#include "libavutil/eval.h"
#include "libavutil/mathematics.h"
#include "libavutil/pixdesc.h"
#include "libavutil/imgutils.h"
#include "libavutil/dict.h"
#include "libavutil/parseutils.h"
#include "libavutil/samplefmt.h"
#include "libavutil/avassert.h"
#include "libavutil/time.h"
#include "libavformat/avformat.h"
#include "libavdevice/avdevice.h"
#include "libswscale/swscale.h"
#include "libavutil/opt.h"
#include "libavcodec/avfft.h"
#include "libswresample/swresample.h"
}

#include "ffmsg.h"
void msg_free_res(AVMessage *msg)
{
    if(!msg || !msg->obj)
        return;
    msg->free_l(msg->obj);
    msg->obj = NULL;
}

// 消息队列内部重新去构建 AVMessage（重新申请AVMessage，或者来自于recycle_msg）
// 新的消息插入到尾部
int msg_queue_put_private(MessageQueue *q, AVMessage *msg)
{
    AVMessage *msg1;

    if(q->abort_request)// 请求终止消息队列
        return -1;

    //1. 消息体使用回收的资源还是重新malloc
    msg1 = q->recycle_msg;
    if(msg1) {
        q->recycle_msg = msg1->next;
        q->recycle_count++;
    } else {
        q->alloc_count++;
        msg1 = (AVMessage *)av_malloc(sizeof(AVMessage));
    }

    *msg1 = *msg;//将传入的消息体进行赋值（浅拷贝）
    msg1->next = NULL;//注意消息队列最后一个指针指向空

    if(!q->first_msg) {//判断是不是队列第一个
        q->first_msg = msg1;
    } else {
        q->last_msg->next = msg1;
    }

    //更新队列
    q->last_msg = msg1;
    q->nb_messages++;
    SDL_CondSignal(q->cond);//于向指定的条件变量发送一个信号，以唤醒等待该条件变量的线程之一
    return 0;
}



int msg_queue_put(MessageQueue *q, AVMessage *msg)
{
    int ret;
    SDL_LockMutex(q->mutex);
    ret = msg_queue_put_private(q, msg);
    SDL_UnlockMutex(q->mutex);
    return ret;
}



void msg_init_msg(AVMessage *msg)
{
    memset(msg, 0, sizeof(AVMessage));
}


void msg_queue_put_simple1(MessageQueue *q, int what)
{
    AVMessage msg;
    msg_init_msg(&msg);
    msg.what = what;
    msg_queue_put(q, &msg);
}

void msg_queue_put_simple2(MessageQueue *q, int what, int arg1)
{

    AVMessage msg;
    msg_init_msg(&msg);
    msg.what = what;
    msg.arg1 = arg1;
    msg_queue_put(q, &msg);
}
// 插入简单消息，只带消息类型，带2个参数
void msg_queue_put_simple3(MessageQueue *q, int what, int arg1, int arg2)
{
    AVMessage msg;
    msg_init_msg(&msg);
    msg.what = what;
    msg.arg1 = arg1;
    msg.arg2 = arg2;
    msg_queue_put(q, &msg);
}
// 释放msg的obj资源
void msg_obj_free_l(void *obj)
{
    av_free(obj);
}
//插入消息，带消息类型，带2个参数，带obj
void msg_queue_put_simple4(MessageQueue *q, int what, int arg1, int arg2, void *obj, int obj_len)
{
    AVMessage msg;
    msg_init_msg(&msg);
    msg.what = what;
    msg.arg1 = arg1;
    msg.arg2 = arg2;
    msg.obj = av_malloc(obj_len);
    memcpy(msg.obj, obj, obj_len);
    msg.free_l = msg_obj_free_l;
    msg_queue_put(q, &msg);
}
// 消息队列初始化
void msg_queue_init(MessageQueue *q)
{
    memset(q, 0, sizeof(MessageQueue));
    q->mutex = SDL_CreateMutex();
    q->cond = SDL_CreateCond();
    q->abort_request = 1;
}
 // 消息队列flush，清空所有的消息
void msg_queue_flush(MessageQueue *q)
{
    AVMessage *msg, *msg1;

    SDL_LockMutex(q->mutex);
    for (msg = q->first_msg; msg != NULL; msg = msg1) { // 这个时候的obj没有清空？那会导致泄漏，实际是把消息对象暂存到了recycle_msg
        msg1 = msg->next;
        msg->next = q->recycle_msg;
        q->recycle_msg = msg;
    }
    q->last_msg = NULL;
    q->first_msg = NULL;
    q->nb_messages = 0;
    SDL_UnlockMutex(q->mutex);

}
// 消息销毁
void msg_queue_destroy(MessageQueue *q)
{
    msg_queue_flush(q);

   SDL_LockMutex(q->mutex);
   while(q->recycle_msg) {
       AVMessage *msg = q->recycle_msg;
       if (msg)
           q->recycle_msg = msg->next;
       msg_free_res(msg);
       av_freep(&msg);
   }
   SDL_UnlockMutex(q->mutex);

   SDL_DestroyMutex(q->mutex);
   SDL_DestroyCond(q->cond);
}
// 消息队列终止
void msg_queue_abort(MessageQueue *q)
{
    SDL_LockMutex(q->mutex);
    q->abort_request = 1;
    SDL_CondSignal(q->cond);
    SDL_UnlockMutex(q->mutex);
}
// 启用消息队列
void msg_queue_start(MessageQueue *q)
{
    SDL_LockMutex(q->mutex);
    q->abort_request = 0;
    // 插入一个消息
    AVMessage msg;
    msg_init_msg(&msg);
    msg.what = FFP_MSG_FLUSH;
    msg_queue_put_private(q, &msg);
    SDL_UnlockMutex(q->mutex);
}


// 从头部first_msg取消息
//return < 0 if aborted, 0 if no msg and > 0 if msg.
int msg_queue_get(MessageQueue *q, AVMessage *msg, int block)
{
    AVMessage *msg1;
    int ret;

    SDL_LockMutex(q->mutex);

    for(;;) {
        if(q->abort_request) {
            ret = -1;
            break;
        }
        //获取消息
        msg1 = q->first_msg;
        if(msg1) {
            q->first_msg = msg1->next;
            if(!q->first_msg)//如果被赋值的first为空，则下一个消息体指针也为空
                q->last_msg = NULL;
            q->nb_messages--;
            *msg = *msg1;//将获取到的信息体放入行数的形参
            msg1->obj = NULL;
            msg1->next = q->recycle_msg;
            q->recycle_msg = msg1;
            ret =1;
            break;      // 记得这里有个break的
        } else if (!block) {
            ret = 0;
            break;
        } else {
            SDL_CondWait(q->cond, q->mutex);
        }
    }
    SDL_UnlockMutex(q->mutex);
    return ret;
}
// 消息删除 把队列里同一消息类型的消息全删除掉
void msg_queue_remove(MessageQueue *q, int what)
{
    AVMessage **p_msg, *msg, *last_msg;
        SDL_LockMutex(q->mutex);

        last_msg = q->first_msg;//last_msg用于记录最后一个未被移除的消息，初始值为q->first_msg

        if (!q->abort_request && q->first_msg) {
            p_msg = &q->first_msg;//p_msg用于遍历消息队列，初始值为&q->first_msg
            /*使用while循环遍历消息队列，p_msg指向当前消息的前一个消息的next指针。
如果当前消息的类型与what匹配，则移除该消息并释放资源。
否则，更新last_msg和p_msg，继续遍历*/
            while (*p_msg) {
                msg = *p_msg;
                if (msg->what == what) {        // 同类型的消息全部删除
                    *p_msg = msg->next;
                    msg_free_res(msg);
                    msg->next = q->recycle_msg;     // 消息体回收
                    q->recycle_msg = msg;
                    q->nb_messages--;
                } else {
                    last_msg = msg;
                    p_msg = &msg->next;
                }
            }

            /*如果队列中还有消息，更新q->last_msg为最后一个未被移除的消息。
如果队列为空，将q->last_msg设为NULL*/
            if (q->first_msg) {
                q->last_msg = last_msg;
            } else {
                q->last_msg = NULL;
            }
        }

        SDL_UnlockMutex(q->mutex);
}

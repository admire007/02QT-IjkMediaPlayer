#include "ijkmediaplayer.h"

IjkMediaPlayer::IjkMediaPlayer()
{

}

int IjkMediaPlayer::ijkmp_create(std::function<int (void *)> msg_loop)
{
    int ret = 0;
    ffplayer_= new FFPlayer();
    if(!ffplayer_) {
        // std::cout<< " new FFPlayer() failed\n ";
        return -1;
    }
    msg_loop_ = msg_loop;

    ret = ffplayer_->ffp_create();

    if(ret < 0) {
        return -1;
    }
    return 0;
}

int IjkMediaPlayer::ijkmp_destroy()
{
    return 0;
}

int IjkMediaPlayer::ijkmp_set_data_source(const char *url)
{
    if(!url) {
        return -1;
    }
    data_source_ = strdup(url); // 分配内存+ 拷贝字符串
    return 0;
}

int IjkMediaPlayer::ijkmp_prepare_async()
{
    // 判断mp的状态
    // 正在准备中
    mp_state_ = MP_STATE_ASYNC_PREPARING;

    // 启用消息队列
    msg_queue_start(ffplayer_->msg_queue_);
    // 创建循环线程
    msg_thread_ = new std::thread(&IjkMediaPlayer::ijkmp_msg_loop, this, this);
    // 调用ffplayer
    int ret = ffplayer_->ffp_prepare_async_l(data_source_);
    if(ret < 0) {
        mp_state_ = MP_STATE_ERROR;
        return -1;
    }
    return 0;
}

int IjkMediaPlayer::ijkmp_msg_loop(void *arg)
{
    msg_loop_(arg);
    return 0;
}








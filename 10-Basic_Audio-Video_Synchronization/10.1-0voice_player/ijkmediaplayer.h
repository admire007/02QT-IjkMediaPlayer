#ifndef IJKMEDIAPLAYER_H
#define IJKMEDIAPLAYER_H

#include  <mutex>
#include <thread>
#include <functional>
#include "ff_ffplay_def.h"
#include "ff_ffplay.h"
#include "ffmsg_queue.h"

/*-
 MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_IDLE);
 MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_INITIALIZED);
 MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_ASYNC_PREPARING);
 MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_PREPARED);
 MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_STARTED);
 MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_PAUSED);
 MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_COMPLETED);
 MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_STOPPED);
 MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_ERROR);
 MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_END);
 */

/*-
 * ijkmp_set_data_source()  -> MP_STATE_INITIALIZED
 *
 * ijkmp_reset              -> self
 * ijkmp_release            -> MP_STATE_END
 */
#define MP_STATE_IDLE               0

/*-
 * ijkmp_prepare_async()    -> MP_STATE_ASYNC_PREPARING
 *
 * ijkmp_reset              -> MP_STATE_IDLE
 * ijkmp_release            -> MP_STATE_END
 */
#define MP_STATE_INITIALIZED        1

/*-
 *                   ...    -> MP_STATE_PREPARED
 *                   ...    -> MP_STATE_ERROR
 *
 * ijkmp_reset              -> MP_STATE_IDLE
 * ijkmp_release            -> MP_STATE_END
 */
#define MP_STATE_ASYNC_PREPARING    2

/*-
 * ijkmp_seek_to()          -> self
 * ijkmp_start()            -> MP_STATE_STARTED
 *
 * ijkmp_reset              -> MP_STATE_IDLE
 * ijkmp_release            -> MP_STATE_END
 */
#define MP_STATE_PREPARED           3

/*-
 * ijkmp_seek_to()          -> self
 * ijkmp_start()            -> self
 * ijkmp_pause()            -> MP_STATE_PAUSED
 * ijkmp_stop()             -> MP_STATE_STOPPED
 *                   ...    -> MP_STATE_COMPLETED
 *                   ...    -> MP_STATE_ERROR
 *
 * ijkmp_reset              -> MP_STATE_IDLE
 * ijkmp_release            -> MP_STATE_END
 */
#define MP_STATE_STARTED            4

/*-
 * ijkmp_seek_to()          -> self
 * ijkmp_start()            -> MP_STATE_STARTED
 * ijkmp_pause()            -> self
 * ijkmp_stop()             -> MP_STATE_STOPPED
 *
 * ijkmp_reset              -> MP_STATE_IDLE
 * ijkmp_release            -> MP_STATE_END
 */
#define MP_STATE_PAUSED             5

/*-
 * ijkmp_seek_to()          -> self
 * ijkmp_start()            -> MP_STATE_STARTED (from beginning)
 * ijkmp_pause()            -> self
 * ijkmp_stop()             -> MP_STATE_STOPPED
 *
 * ijkmp_reset              -> MP_STATE_IDLE
 * ijkmp_release            -> MP_STATE_END
 */
#define MP_STATE_COMPLETED          6

/*-
 * ijkmp_stop()             -> self
 * ijkmp_prepare_async()    -> MP_STATE_ASYNC_PREPARING
 *
 * ijkmp_reset              -> MP_STATE_IDLE
 * ijkmp_release            -> MP_STATE_END
 */
#define MP_STATE_STOPPED            7

/*-
 * ijkmp_reset              -> MP_STATE_IDLE
 * ijkmp_release            -> MP_STATE_END
 */
#define MP_STATE_ERROR              8

/*-
 * ijkmp_release            -> self
 */
#define MP_STATE_END                9



class IjkMediaPlayer
{
public:
    IjkMediaPlayer();
    ~IjkMediaPlayer();
    int ijkmp_create(std::function<int(void *)> msg_loop);
    int ijkmp_destroy();
    // 设置要播放的url
    int ijkmp_set_data_source(const char *url);
    // 准备播放
    int ijkmp_prepare_async();
    // 触发播放
    int ijkmp_start();
    // 停止
    int ijkmp_stop();
    // 暂停
    int ijkmp_pause();
    // seek到指定位置
    int ijkmp_seek_to(long msec);
    // 获取播放状态
    int ijkmp_get_state();
    // 是不是播放中
    bool ijkmp_is_playing();
    // 当前播放位置
    long ijkmp_get_current_position();
    // 总长度
    long ijkmp_get_duration();
    // 已经播放的长度
    long ijkmp_get_playable_duration();
    // 设置循环播放
    void ijkmp_set_loop(int loop);
    // 获取是否循环播放
    int  ijkmp_get_loop();
    // 读取消息
    int ijkmp_get_msg(AVMessage *msg, int block);
    // 设置音量
    void ijkmp_set_playback_volume(float volume);

    int ijkmp_msg_loop(void *arg);

    void AddVideoRefreshCallback(std::function<int(const Frame *)> callback);


private:
    // 互斥量
    std::mutex mutex_;
    // 真正的播放器
    FFPlayer *ffplayer_ = NULL;
    //函数指针, 指向创建的message_loop，即消息循环函数
    //    int (*msg_loop)(void*);
    std::function<int(void *)> msg_loop_ = NULL; // ui处理消息的循环
    //消息机制线程
    std::thread *msg_thread_; // 执行msg_loop
    //    SDL_Thread _msg_thread;
    //字符串，就是一个播放url
    char *data_source_;
    //播放器状态，例如prepared,resumed,error,completed等
    int mp_state_;  // 播放状态
};

#endif // IJKMEDIAPLAYER_H

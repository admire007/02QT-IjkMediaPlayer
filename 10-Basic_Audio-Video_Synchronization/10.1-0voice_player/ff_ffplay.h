#ifndef FF_FFPLAY_H
#define FF_FFPLAY_H
#include <thread>
#include "ffmsg_queue.h"
#include "ff_ffplay_def.h"
#include <functional>


class Decoder
{
public:
    AVPacket pkt_;
    PacketQueue	*queue_;         // 数据包队列
    AVCodecContext	*avctx_;     // 解码器上下文
    int		pkt_serial_;         // 包序列
    int		finished_;           // =0，解码器处于工作状态；=非0，解码器处于空闲状态
    std::thread *decoder_thread_ = NULL;
    Decoder();
    ~Decoder();
    void decoder_init(AVCodecContext *avctx, PacketQueue *queue);
    // 创建和启动线程
    int decoder_start(enum AVMediaType codec_type, const char *thread_name, void* arg);
    // 停止线程
    void decoder_abort(FrameQueue *fq);
    void decoder_destroy();
    int decoder_decode_frame(AVFrame *frame);
    int get_video_frame(AVFrame *frame);
    int queue_picture(FrameQueue *fq, AVFrame *src_frame, double pts,
                            double duration, int64_t pos, int serial);
    int audio_thread(void* arg);
    int video_thread(void* arg);

};
class FFPlayer
{
public:
    FFPlayer();
    int ffp_create();
    void ffp_destroy();
    int ffp_prepare_async_l(char *file_name);

    // 播放控制
    int       ffp_start_l();
    int       ffp_stop_l();
    int stream_open( const char *file_name);
    void stream_close();
    // 打开指定stream对应解码器、创建解码线程、以及初始化对应的输出
    int stream_component_open(int stream_index);
    // 关闭指定stream的解码线程，释放解码器资源
    void stream_component_close(int stream_index);

    int audio_open(int64_t wanted_channel_layout,
                          int wanted_nb_channels, int wanted_sample_rate,
                          struct AudioParams *audio_hw_params);
    void audio_close();
    MessageQueue msg_queue_;
    char *input_filename_;
    int read_thread();
    std::thread *read_thread_;

    int video_refresh_thread();
    void video_refresh(double *remaining_time);
    // 视频画面输出相关
    std::thread *video_refresh_thread_ = NULL;

    std::function<int(const Frame *)> video_refresh_callback_ = NULL;
    void AddVideoRefreshCallback(std::function<int(const Frame *)> callback);

    int get_master_sync_type();
    double get_master_clock();
    int av_sync_type = AV_SYNC_AUDIO_MASTER;           // 音视频同步类型, 默认audio master
    Clock	audclk;             // 音频时钟
    //Clock	vidclk;             // 视频时钟

    double			audio_clock = 0;            // 当前音频帧的PTS+当前帧Duration

    // 帧队列
    FrameQueue	pictq;          // 视频Frame队列
    FrameQueue	sampq;          // 采样Frame队列

    // 包队列
    PacketQueue		audioq;                 // 音频packet队列
    PacketQueue videoq;             // 视频队列
    int abort_request = 0;

    AVStream		*audio_st = NULL;              // 音频流
    AVStream		*video_st = NULL;              // 音频流
    int audio_stream = -1;
    int video_stream = -1;

    Decoder auddec;             // 音频解码器
    Decoder viddec;             // 视频解码器


    int eof = 0;
     AVFormatContext *ic = NULL;

     // 音频输出相关
     struct AudioParams audio_src;  // 保存最新解码的音频参数
     struct AudioParams audio_tgt;  // 保存SDL音频输出需要的参数
     struct SwrContext *swr_ctx = NULL;         // 音频重采样context
     int			audio_hw_buf_size = 0;          // SDL音频缓冲区的大小(字节为单位)
     // 指向待播放的一帧音频数据，指向的数据区将被拷入SDL音频缓冲区。若经过重采样则指向audio_buf1，
     // 否则指向frame中的音频
     uint8_t			*audio_buf = NULL;             // 指向需要重采样的数据
     uint8_t			*audio_buf1 = NULL;            // 指向重采样后的数据
     unsigned int		audio_buf_size = 0;     // 待播放的一帧音频数据(audio_buf指向)的大小
     unsigned int		audio_buf1_size = 0;    // 申请到的音频缓冲区audio_buf1的实际尺寸
     int			audio_buf_index = 0;            // 更新拷贝位置 当前音频帧中已拷入SDL音频缓冲区
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

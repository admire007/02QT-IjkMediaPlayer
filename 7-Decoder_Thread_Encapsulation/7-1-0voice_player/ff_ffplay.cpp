#include "ff_ffplay.h"
#include <iostream>
#include <string.h>
#include "ffmsg.h"


void print_error(const char *filename, int err)
{
    char errbuf[128];
    const char *errbuf_ptr = errbuf;

    if (av_strerror(err, errbuf, sizeof(errbuf)) < 0)
        errbuf_ptr = strerror(AVUNERROR(err));
    av_log(NULL, AV_LOG_ERROR, "%s: %s\n", filename, errbuf_ptr);
}

FFPlayer::FFPlayer()
{

}

int FFPlayer::ffp_create()
{
    std::cout << "ffp_create\n";
    msg_queue_init(&msg_queue_);
    return 0;
}

void FFPlayer::ffp_destroy()
{
    stream_close();

    // 销毁消息队列
    msg_queue_destroy(&msg_queue_);
}

int FFPlayer::ffp_prepare_async_l(char *file_name)
{
    //保存文件名
    input_filename_ =  strdup(file_name);

    int reval = stream_open(file_name);

    return reval;
}

int FFPlayer::ffp_start_l()
{
    // 触发播放
    std::cout << __FUNCTION__;
    return -1;
}

int FFPlayer::ffp_stop_l()
{
    abort_request = 1;  // 请求退出
    msg_queue_abort(&msg_queue_);  // 禁止再插入消息
    return -1;
}

int FFPlayer::stream_open(const char *file_name)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
    {
        av_log(NULL, AV_LOG_FATAL, "Could not initialize SDL - %s\n", SDL_GetError());
        av_log(NULL, AV_LOG_FATAL, "(Did you set the DISPLAY variable?)\n");
        return -1;
    }
    // 初始化Frame帧队列
    if (frame_queue_init(&pictq, &videoq, VIDEO_PICTURE_QUEUE_SIZE_DEFAULT) < 0)
        goto fail;
    if (frame_queue_init(&sampq, &audioq, SAMPLE_QUEUE_SIZE) < 0)
        goto fail;


    // 初始化Packet包队列
    if (packet_queue_init(&videoq) < 0 ||
            packet_queue_init(&audioq) < 0 )
        goto fail;


    // 初始化时钟

    // 初始化音量等

    // 创建解复用器读数据线程read_thread

    read_thread_ = new std::thread(&FFPlayer::read_thread, this);

    // 创建视频刷新线程
    return 0;
fail:
    stream_close();
    return -1;
}

void FFPlayer::stream_close()
{
    abort_request = 1; // 请求退出
    if(read_thread_ && read_thread_->joinable()) {
        read_thread_->join();       // 等待线程退出
    }
    /* close each stream */
    if (audio_stream >= 0)
        stream_component_close(audio_stream);  // 解码器线程请求abort的时候有调用 packet_queue_abort
    if (video_stream >= 0)
        stream_component_close(video_stream);


    // 关闭解复用器 avformat_close_input(&is->ic);
    // 释放packet队列
    packet_queue_destroy(&videoq);
    packet_queue_destroy(&audioq);
    // 释放frame队列
    frame_queue_destory(&pictq);
    frame_queue_destory(&sampq);

    if(input_filename_) {
        free(input_filename_);
        input_filename_ = NULL;
    }
}

// 如果想指定解码器怎么处理？
int FFPlayer::stream_component_open(int stream_index)
{
    AVCodecContext *avctx;
    AVCodec *codec;
    int sample_rate;
    int nb_channels;
    int64_t channel_layout;
    int ret = 0;

    // 判断stream_index是否合法
    if (stream_index < 0 || stream_index >= ic->nb_streams)
        return -1;
    /*  为解码器分配一个编解码器上下文结构体 */
    avctx = avcodec_alloc_context3(NULL);
    if (!avctx)
        return AVERROR(ENOMEM);

    /* 将码流中的编解码器信息拷贝到新分配的编解码器上下文结构体 */
    ret = avcodec_parameters_to_context(avctx, ic->streams[stream_index]->codecpar);
    if (ret < 0)
        goto fail;
    // 设置pkt_timebase
    avctx->pkt_timebase = ic->streams[stream_index]->time_base;

    /* 根据codec_id查找解码器 */
    codec = avcodec_find_decoder(avctx->codec_id);
    if (!codec) {
         av_log(NULL, AV_LOG_WARNING,
                                      "No decoder could be found for codec %s\n", avcodec_get_name(avctx->codec_id));
        ret = AVERROR(EINVAL);
        goto fail;
    }
    if ((ret = avcodec_open2(avctx, codec, NULL)) < 0) {
        goto fail;
    }
    switch (avctx->codec_type) {
    case AVMEDIA_TYPE_AUDIO:
        //从avctx(即AVCodecContext)中获取音频格式参数
        sample_rate = avctx->sample_rate;;  // 采样率
        nb_channels = avctx->channels;;    // 通道数
        channel_layout= avctx->channel_layout;; // 通道布局


        /* prepare audio output 准备音频输出*/

        audio_stream = stream_index;    // 获取audio的stream索引
        audio_st = ic->streams[stream_index];  // 获取audio的stream指针

         // 初始化ffplay封装的音频解码器, 并将解码器上下文 avctx和Decoder绑定
        auddec.decoder_init(avctx, &audioq);
        // 启动音频解码线程
        auddec.decoder_start(AVMEDIA_TYPE_AUDIO, "audio_thread", this);
        // 允许音频输出
        break;
    case AVMEDIA_TYPE_VIDEO:
        video_stream = stream_index;    // 获取video的stream索引
        video_st = ic->streams[stream_index];// 获取video的stream指针
//        // 初始化ffplay封装的视频解码器
       viddec.decoder_init(avctx, &videoq); //  is->continue_read_thread
//        // 启动视频频解码线程
        if ((ret = viddec.decoder_start(AVMEDIA_TYPE_VIDEO, "video_decoder", this)) < 0)
            goto out;
        break;
    default:
        break;
    }

    goto out;
fail:
    avcodec_free_context(&avctx);

out:
    return ret;
}

void FFPlayer::stream_component_close(int stream_index)
{
    AVCodecParameters *codecpar;

    if (stream_index < 0 || stream_index >= ic->nb_streams)
        return;
    codecpar = ic->streams[stream_index]->codecpar;

    switch (codecpar->codec_type) {
    case AVMEDIA_TYPE_AUDIO:
        std::cout << __FUNCTION__ << "  AVMEDIA_TYPE_AUDIO\n";
        // 请求终止解码器线程
        auddec.decoder_abort(&sampq);
        // 关闭音频设备
        // 销毁解码器
        auddec.decoder_destroy();
        // 释放重采样器
        // 释放audio buf
//        decoder_abort(&is->auddec, &is->sampq); // 解码器线程请求abort的时候有调用 packet_queue_abort
//        SDL_CloseAudioDevice(audio_dev);
//        decoder_destroy(&is->auddec);
//        swr_free(&is->swr_ctx);
//        av_freep(&is->audio_buf1);
//        is->audio_buf1_size = 0;
//        is->audio_buf = NULL;
        break;
    case AVMEDIA_TYPE_VIDEO:
        std::cout << __FUNCTION__ << "  AVMEDIA_TYPE_VIDEO\n";
        // 请求终止解码器线程
        // 关闭音频设备
        // 销毁解码器
        viddec.decoder_abort(&pictq);
        viddec.decoder_destroy();
        break;

    default:
        break;
    }

//    ic->streams[stream_index]->discard = AVDISCARD_ALL;  // 这个又有什么用?
    switch (codecpar->codec_type) {
    case AVMEDIA_TYPE_AUDIO:
        audio_st = NULL;
        audio_stream = -1;
        break;
    case AVMEDIA_TYPE_VIDEO:
        video_st = NULL;
        video_stream = -1;
        break;
    default:
        break;
    }
}

int FFPlayer::read_thread()
{
    int err, i, ret;
    int st_index[AVMEDIA_TYPE_NB];      // AVMEDIA_TYPE_VIDEO/ AVMEDIA_TYPE_AUDIO 等，用来保存stream index
    AVPacket pkt1;
    AVPacket *pkt = &pkt1;  //


     // 初始化为-1,如果一直为-1说明没相应steam
    memset(st_index, -1, sizeof(st_index));
    video_stream = -1;
    audio_stream = -1;
    eof = 0;

     // 1. 创建上下文结构体，这个结构体是最上层的结构体，表示输入上下文
    ic = avformat_alloc_context();
    if (!ic) {
        av_log(NULL, AV_LOG_FATAL, "Could not allocate context.\n");
        ret = AVERROR(ENOMEM);
        goto fail;
    }

    /* 3.打开文件，主要是探测协议类型，如果是网络文件则创建网络链接等 */
    err = avformat_open_input(&ic, input_filename_ , NULL, NULL);
    if (err < 0) {
        print_error(input_filename_, err);
        ret = -1;
        goto fail;
    }
    ffp_notify_msg1(this, FFP_MSG_OPEN_INPUT);
    std::cout << "read_thread FFP_MSG_OPEN_INPUT " << this << std::endl;

    /*
     * 4.探测媒体类型，可得到当前文件的封装格式，音视频编码参数等信息
     * 调用该函数后得多的参数信息会比只调用avformat_open_input更为详细，
     * 其本质上是去做了decdoe packet获取信息的工作
     * codecpar, filled by libavformat on stream creation or
     * in avformat_find_stream_info()
     */
    err = avformat_find_stream_info(ic, NULL);

    if (err < 0) {
        av_log(NULL, AV_LOG_WARNING,
               "%s: could not find codec parameters\n", input_filename_);
        ret = -1;
        goto fail;
    }

    ffp_notify_msg1(this, FFP_MSG_FIND_STREAM_INFO);
    std::cout << "read_thread FFP_MSG_FIND_STREAM_INFO " << this << std::endl;



    // 6.2 利用av_find_best_stream选择流，
    st_index[AVMEDIA_TYPE_VIDEO] =
            av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO,
                                st_index[AVMEDIA_TYPE_VIDEO], -1, NULL, 0);

    st_index[AVMEDIA_TYPE_AUDIO] =
            av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO,
                                st_index[AVMEDIA_TYPE_AUDIO],
                                st_index[AVMEDIA_TYPE_VIDEO],
                                NULL, 0);

    /* open the streams */
    /* 8. 打开视频、音频解码器。在此会打开相应解码器，并创建相应的解码线程。 */
    if (st_index[AVMEDIA_TYPE_AUDIO] >= 0) {// 如果有音频流则打开音频流
        stream_component_open(st_index[AVMEDIA_TYPE_AUDIO]);
    }

    ret = -1;
    if (st_index[AVMEDIA_TYPE_VIDEO] >= 0) { // 如果有视频流则打开视频流
        ret = stream_component_open( st_index[AVMEDIA_TYPE_VIDEO]);
    }


    ffp_notify_msg1(this, FFP_MSG_COMPONENT_OPEN);
    std::cout << "read_thread FFP_MSG_COMPONENT_OPEN " << this << std::endl;

    if (video_stream < 0 && audio_stream < 0) {
        av_log(NULL, AV_LOG_FATAL, "Failed to open file '%s' or configure filtergraph\n",
               input_filename_);
        ret = -1;
        goto fail;
    }


    ffp_notify_msg1(this, FFP_MSG_PREPARED);
    std::cout << "read_thread FFP_MSG_PREPARED " << this << std::endl;
    while (1) {
//        std::cout << "read_thread sleep, mp:" << this << std::endl;
        // 先模拟线程运行
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if(abort_request) {
            break;
        }
    }

    std::cout << __FUNCTION__ << " leave" << std::endl;

    return 0;
    fail:
    return -1;
}

Decoder::Decoder()
{
    av_init_packet(&pkt_);
}

Decoder::~Decoder()
{

}

void Decoder::decoder_init(AVCodecContext *avctx, PacketQueue *queue)
{
    avctx_ = avctx;
    queue_ = queue;
}

int Decoder::decoder_start(AVMediaType codec_type, const char *thread_name, void *arg)
{
    // 启用包队列
    packet_queue_start(queue_);
    // 创建线程
    if(AVMEDIA_TYPE_VIDEO == codec_type)
        decoder_thread_ = new std::thread(&Decoder::video_thread, this, arg);
    else if (AVMEDIA_TYPE_AUDIO == codec_type)
        decoder_thread_ = new std::thread(&Decoder::audio_thread, this, arg);
    else
        return -1;
    return 0;
}

void Decoder::decoder_abort(FrameQueue *fq)
{
    packet_queue_abort(queue_);     // 请求退出包队列
    frame_queue_signal(fq);     // 唤醒阻塞的帧队列
    if(decoder_thread_ && decoder_thread_->joinable()) {
        decoder_thread_->join(); // 等待解码线程退出
        delete decoder_thread_;
        decoder_thread_ = NULL;
    }
    packet_queue_flush(queue_);  // 情况packet队列，并释放数据
}

void Decoder::decoder_destroy()
{
    av_packet_unref(&pkt_);
    avcodec_free_context(&avctx_);
}

// 返回值-1: 请求退出
//       0: 解码已经结束了，不再有数据可以读取
//       1: 获取到解码后的frame
int Decoder::decoder_decode_frame(AVFrame *frame)
{
    int ret = AVERROR(EAGAIN);

    for (;;) {
        AVPacket pkt;
        do {  // 第一个循环 先把codec里的frame 全部读取

            if (queue_->abort_request)      // decoder_abort调用的时候 触发queue_->abort_request为1
                return -1;  // 是否请求退出
            switch (avctx_->codec_type) {
            case AVMEDIA_TYPE_VIDEO:
                ret = avcodec_receive_frame(avctx_, frame);
                //printf("frame pts:%ld, dts:%ld\n", frame->pts, frame->pkt_dts);
                if (ret >= 0) {
//                    if (decoder_reorder_pts == -1) {
//                        frame->pts = frame->best_effort_timestamp;
//                    } else if (!decoder_reorder_pts) {
//                        frame->pts = frame->pkt_dts;
//                    }
                } else {

                    char errStr[256] = { 0 };
                    av_strerror(ret, errStr, sizeof(errStr));
                    printf("video dec:%s\n", errStr);
                }
                break;
            case AVMEDIA_TYPE_AUDIO:
                ret = avcodec_receive_frame(avctx_, frame);
                if (ret >= 0) {
                    AVRational tb ={1, frame->sample_rate};    //
                    if (frame->pts != AV_NOPTS_VALUE) {
                        // 如果frame->pts正常则先将其从pkt_timebase转成{1, frame->sample_rate}
                        // pkt_timebase实质就是stream->time_base
                        frame->pts = av_rescale_q(frame->pts, avctx_->pkt_timebase, tb);
                    }
//                    else if (d->next_pts != AV_NOPTS_VALUE) {
//                        // 如果frame->pts不正常则使用上一帧更新的next_pts和next_pts_tb
//                        // 转成{1, frame->sample_rate}
//                        frame->pts = av_rescale_q(d->next_pts, d->next_pts_tb, tb);
//                    }
                }else {

                    char errStr[256] = { 0 };
                    av_strerror(ret, errStr, sizeof(errStr));
                    printf("audio dec:%s\n", errStr);
                }
                break;
            }
            // 1.3. 检查解码是否已经结束，解码结束返回0
            if (ret == AVERROR_EOF) {
                printf("avcodec_flush_buffers %s(%d)\n", __FUNCTION__, __LINE__);
                avcodec_flush_buffers(avctx_);
                return 0;
            }
            // 1.4. 正常解码返回1
            if (ret >= 0)
                return 1;       // 获取到一帧frame
        }while (ret != AVERROR(EAGAIN));   // 1.5 没帧可读时ret返回EAGIN，需要继续送packet

        //  在目前这个版本我们还不去检测播放序列的问题
        // 2 如果上面的循环获取到了frame这里不会被执行，第二个循环，主要是读取packet送给解码器
//        do { //  在目前这个版本我们还不去检测播放序列的问题

//        if (queue_->nb_packets == 0)  // 没有数据可读
//            SDL_CondSignal(d->empty_queue_cond);// 通知read_thread放入packet

        // 2.3 阻塞式读取packet
        if (packet_queue_get(queue_, &pkt, 1, &pkt_serial_) < 0)
            return -1;

//   } while (d->queue->serial != d->pkt_serial);// 如果不是同一播放序列(流不连续)则继续读取

        if (avcodec_send_packet(avctx_, &pkt) == AVERROR(EAGAIN)) {
            av_log(avctx_, AV_LOG_ERROR, "Receive_frame and send_packet both returned EAGAIN, which is an API violation.\n");
            // 先暂存这个pkt
        }
        av_packet_unref(&pkt);	// 一定要自己去释放音视频数据
    }
}

int Decoder::get_video_frame(AVFrame *frame)
{
    int got_picture;
    // 1. 获取解码后的视频帧
    if ((got_picture = decoder_decode_frame(frame)) < 0) {
        return -1; // 返回-1意味着要退出解码线程, 所以要分析decoder_decode_frame什么情况下返回-1
    }
    if (got_picture) {
        // 2. 分析获取到的该帧是否要drop掉, 该机制的目的是在放入帧队列前先drop掉过时的视频帧
//        frame->sample_aspect_ratio = av_guess_sample_aspect_ratio(is->ic, is->video_st, frame);
    }

    return got_picture;
}

int Decoder::queue_picture(FrameQueue *fq, AVFrame *src_frame, double pts, double duration, int64_t pos, int serial)
{
    Frame *vp;


    if (!(vp = frame_queue_peek_writable(fq))) // 检测队列是否有可写空间
        return -1;      // 请求退出则返回-1
    // 执行到这步说已经获取到了可写入的Frame
//    vp->sar = src_frame->sample_aspect_ratio;
//    vp->uploaded = 0;

    vp->width = src_frame->width;
    vp->height = src_frame->height;
    vp->format = src_frame->format;

    vp->pts = pts;
    vp->duration = duration;
//    vp->pos = pos;
//    vp->serial = serial;

    av_frame_move_ref(vp->frame, src_frame); // 将src中所有数据转移到dst中，并复位src。
    frame_queue_push(fq);   // 更新写索引位置
    return 0;
}

int Decoder::audio_thread(void *arg)
{
    std::cout << __FUNCTION__ <<  " into " << std::endl;
    FFPlayer *is = (FFPlayer *)arg;
    AVFrame *frame = av_frame_alloc();  // 分配解码帧
    Frame *af;
    int got_frame = 0;  // 是否读取到帧
    AVRational tb;      // timebase
    int ret = 0;

    if (!frame)
        return AVERROR(ENOMEM);

    do {
        // 1. 读取解码帧
        if ((got_frame = decoder_decode_frame(frame)) < 0)   // 是否获取到一帧数据
            goto the_end; // < =0 abort

        if (got_frame) {
            tb ={1, frame->sample_rate};   // 设置为sample_rate为timebase

            // 2. 获取可写Frame
            if (!(af = frame_queue_peek_writable(&is->sampq)))  // 获取可写帧
                goto the_end;
            // 3. 设置Frame并放入FrameQueue
            af->pts = (frame->pts == AV_NOPTS_VALUE) ? NAN : frame->pts * av_q2d(tb);  // 转换时间戳
//            af->pos = frame->pkt_pos;
//            af->serial = is->auddec.pkt_serial;
             af->duration = av_q2d({frame->nb_samples, frame->sample_rate});

            av_frame_move_ref(af->frame, frame);
            frame_queue_push(&is->sampq);  // 代表队列真正插入一帧数据

        }
    } while (ret >= 0 || ret == AVERROR(EAGAIN) || ret == AVERROR_EOF);
the_end:

    std::cout << __FUNCTION__ <<  " leave " << std::endl;
    av_frame_free(&frame);
    return ret;
}

int Decoder::video_thread(void *arg)
{
     std::cout << __FUNCTION__ <<  " into " << std::endl;
     FFPlayer *is = (FFPlayer *)arg;
     AVFrame *frame = av_frame_alloc();  // 分配解码帧
     double pts;                 // pts
     double duration;            // 帧持续时间
     int ret;
     //1 获取stream timebase
     AVRational tb = is->video_st->time_base; // 获取stream timebase
     //2 获取帧率，以便计算每帧picture的duration
     AVRational frame_rate = av_guess_frame_rate(is->ic, is->video_st, NULL);


     if (!frame)
         return AVERROR(ENOMEM);

     for (;;) {  // 循环取出视频解码的帧数据
         // 3 获取解码后的视频帧
         ret = get_video_frame(frame);
         if (ret < 0)
             goto the_end;   //解码结束, 什么时候会结束
         if (!ret)           //没有解码得到画面, 什么情况下会得不到解后的帧
             continue;

//           1/25 = 0.04秒
         // 4 计算帧持续时间和换算pts值为秒
         // 1/帧率 = duration 单位秒, 没有帧率时则设置为0, 有帧率帧计算出帧间隔
         duration = (frame_rate.num && frame_rate.den ? av_q2d({frame_rate.den, frame_rate.num}) : 0);
         // 根据AVStream timebase计算出pts值, 单位为秒
         pts = (frame->pts == AV_NOPTS_VALUE) ? NAN : frame->pts * av_q2d(tb);  // 单位为秒
         // 5 将解码后的视频帧插入队列   FrameQueue *fq, AVFrame *src_frame, double pts, double duration, int64_t pos, int serial
         ret = queue_picture(&is->pictq, frame, pts, duration, frame->pkt_pos, is->viddec.pkt_serial_);
         // 6 释放frame对应的数据
         av_frame_unref(frame);

         if (ret < 0) // 返回值小于0则退出线程
             goto the_end;
     }
 the_end:
     std::cout << __FUNCTION__ <<  " leave " << std::endl;
     av_frame_free(&frame);
     return 0;
}








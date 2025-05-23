#include "ff_ffplay.h"
#include <iostream>
#include <string.h>
#include "ffmsg.h"

/* Minimum SDL audio buffer size, in samples. */
#define SDL_AUDIO_MIN_BUFFER_SIZE 512
/* Calculate actual buffer size keeping in mind not cause too frequent audio callbacks */
#define SDL_AUDIO_MAX_CALLBACKS_PER_SEC 30


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
    return 0;
}

int FFPlayer::ffp_stop_l()
{
    abort_request = 1;  // 请求退出
    msg_queue_abort(&msg_queue_);  // 禁止再插入消息
    return 0;
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
    /*
     * 初始化时钟
     * 时钟序列->queue_serial，实际上指向的是is->videoq.serial
     */
	//init_clock(&vidclk);
    init_clock(&audclk);
    // 初始化音量等

    // 创建解复用器读数据线程read_thread

    read_thread_ = new std::thread(&FFPlayer::read_thread, this);

    // 创建视频刷新线程
    video_refresh_thread_ = new std::thread(&FFPlayer::video_refresh_thread, this);
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
        //调用audio_open打开sdl音频输出，实际打开的设备参数保存在audio_tgt，返回值表示输出设备的缓冲区大小
        if ((ret = audio_open( channel_layout, nb_channels, sample_rate, &audio_tgt)) < 0)
            goto fail;
        audio_hw_buf_size = ret;
        audio_src = audio_tgt;  //暂且将数据源参数等同于目标输出参数
        //初始化audio_buf相关参数
        audio_buf_size  = 0;
        audio_buf_index = 0;

        audio_stream = stream_index;    // 获取audio的stream索引
        audio_st = ic->streams[stream_index];  // 获取audio的stream指针

         // 初始化ffplay封装的音频解码器, 并将解码器上下文 avctx和Decoder绑定
        auddec.decoder_init(avctx, &audioq);
        // 启动音频解码线程
        auddec.decoder_start(AVMEDIA_TYPE_AUDIO, "audio_thread", this);
        // 允许音频输出
        //play audio
        SDL_PauseAudio(0);
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
        audio_close();
        // 销毁解码器
        auddec.decoder_destroy();
        // 释放重采样器

        swr_free(&swr_ctx);
        // 释放audio buf
        av_freep(&audio_buf1);
        audio_buf1_size = 0;
        audio_buf = NULL;
        break;
    case AVMEDIA_TYPE_VIDEO:
        // 请求退出视频画面刷新线程
        if(video_refresh_thread_ && video_refresh_thread_->joinable()) {
            video_refresh_thread_->join();  // 等待线程退出
        }

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

/**
 * Decode one audio frame and return its uncompressed size.
 *
 * The processed audio frame is decoded, converted if required, and
 * stored in is->audio_buf, with size in bytes given by the return
 * value.
 */
static int audio_decode_frame(FFPlayer *is)
{
    int data_size, resampled_data_size;
    int64_t dec_channel_layout;
    int wanted_nb_samples;
    Frame *af;
    int ret = 0;

    // 读取一帧数据
//    do {

        // 若队列头部可读，则由af指向可读帧
        if (!(af = frame_queue_peek_readable(&is->sampq)))
            return -1;

//    } while (af->serial != is->audioq.serial);



    // 根据frame中指定的音频参数获取缓冲区的大小 af->frame->channels * af->frame->nb_samples * 2
    data_size = av_samples_get_buffer_size(NULL,
                                           af->frame->channels,
                                           af->frame->nb_samples,
                                           (enum AVSampleFormat)af->frame->format, 1);
    // 获取声道布局
    dec_channel_layout =  (af->frame->channel_layout &&
             af->frame->channels == av_get_channel_layout_nb_channels(af->frame->channel_layout)) ?
                af->frame->channel_layout : av_get_default_channel_layout(af->frame->channels);
    // 获取样本数校正值：若同步时钟是音频，则不调整样本数；否则根据同步需要调整样本数
//    wanted_nb_samples = synchronize_audio(is, af->frame->nb_samples);  // 目前不考虑非音视频同步的是情况
    wanted_nb_samples = af->frame->nb_samples;


    // is->audio_tgt是SDL可接受的音频帧数，是audio_open()中取得的参数
    // 在audio_open()函数中又有"is->audio_src = is->audio_tgt""
    // 此处表示：如果frame中的音频参数 == is->audio_src == is->audio_tgt，
    // 那音频重采样的过程就免了(因此时is->swr_ctr是NULL)
    // 否则使用frame(源)和is->audio_tgt(目标)中的音频参数来设置is->swr_ctx，
    // 并使用frame中的音频参数来赋值is->audio_src
    if (af->frame->format           != is->audio_src.fmt            || // 采样格式
            dec_channel_layout      != is->audio_src.channel_layout || // 通道布局
            af->frame->sample_rate  != is->audio_src.freq          // 采样率
            ) {
        swr_free(&is->swr_ctx);
        is->swr_ctx = swr_alloc_set_opts(NULL,
                                         is->audio_tgt.channel_layout,  // 目标输出
                                         is->audio_tgt.fmt,
                                         is->audio_tgt.freq,
                                         dec_channel_layout,            // 数据源
                                         (enum AVSampleFormat)af->frame->format,
                                         af->frame->sample_rate,
                                         0, NULL);
        if (!is->swr_ctx || swr_init(is->swr_ctx) < 0) {
            av_log(NULL, AV_LOG_ERROR,
                   "Cannot create sample rate converter for conversion of %d Hz %s %d channels to %d Hz %s %d channels!\n",
                   af->frame->sample_rate, av_get_sample_fmt_name((enum AVSampleFormat)af->frame->format), af->frame->channels,
                   is->audio_tgt.freq, av_get_sample_fmt_name(is->audio_tgt.fmt), is->audio_tgt.channels);
            swr_free(&is->swr_ctx);
            ret = -1;
            goto fail;
        }
        is->audio_src.channel_layout = dec_channel_layout;
        is->audio_src.channels       = af->frame->channels;
        is->audio_src.freq = af->frame->sample_rate;
        is->audio_src.fmt = (enum AVSampleFormat)af->frame->format;
    }

    if (is->swr_ctx) {
        // 重采样输入参数1：输入音频样本数是af->frame->nb_samples
        // 重采样输入参数2：输入音频缓冲区
        const uint8_t **in = (const uint8_t **)af->frame->extended_data; // data[0] data[1]

        // 重采样输出参数1：输出音频缓冲区
        uint8_t **out = &is->audio_buf1; //真正分配缓存audio_buf1，指向是用audio_buf
        // 重采样输出参数2：输出音频缓冲区尺寸， 高采样率往低采样率转换时得到更少的样本数量，比如 96k->48k, wanted_nb_samples=1024
        // 则wanted_nb_samples * is->audio_tgt.freq / af->frame->sample_rate 为1024*48000/96000 = 512
        // +256 的目的是重采样内部是有一定的缓存，就存在上一次的重采样还缓存数据和这一次重采样一起输出的情况，所以目的是多分配输出buffer
        int out_count = (int64_t)wanted_nb_samples * is->audio_tgt.freq / af->frame->sample_rate
                + 256;
        // 计算对应的样本数 对应的采样格式 以及通道数，需要多少buffer空间
        int out_size  = av_samples_get_buffer_size(NULL, is->audio_tgt.channels,
                                                   out_count, is->audio_tgt.fmt, 0);
        int len2;
        if (out_size < 0) {
            av_log(NULL, AV_LOG_ERROR, "av_samples_get_buffer_size() failed\n");
            ret = -1;
            goto fail;
        }
        // if(audio_buf1_size < out_size) {重新分配out_size大小的缓存给audio_buf1, 并将audio_buf1_size设置为out_size }
        av_fast_malloc(&is->audio_buf1, &is->audio_buf1_size, out_size);
        if (!is->audio_buf1) {
            ret = AVERROR(ENOMEM);
            goto fail;
        }
        // 音频重采样：len2返回值是重采样后得到的音频数据中单个声道的样本数
        len2 = swr_convert(is->swr_ctx, out, out_count, in, af->frame->nb_samples);
        if (len2 < 0) {
            av_log(NULL, AV_LOG_ERROR, "swr_convert() failed\n");
            ret = -1;
            goto fail;
        }

        if (len2 == out_count) { // 这里的意思是我已经多分配了buffer，实际输出的样本数不应该超过我多分配的数量
            av_log(NULL, AV_LOG_WARNING, "audio buffer is probably too small\n");
            if (swr_init(is->swr_ctx) < 0)
                swr_free(&is->swr_ctx);
        }
        // 重采样返回的一帧音频数据大小(以字节为单位)
        is->audio_buf = is->audio_buf1;
        resampled_data_size = len2 * is->audio_tgt.channels * av_get_bytes_per_sample(is->audio_tgt.fmt);
    } else {
        // 未经重采样，则将指针指向frame中的音频数据
        is->audio_buf = af->frame->data[0]; // s16交错模式data[0], fltp data[0] data[1]
        resampled_data_size = data_size;
    }

    if (!isnan(af->pts))
        is->audio_clock = af->pts;   
    else
        is->audio_clock = NAN;

    frame_queue_next(&is->sampq);

    ret = resampled_data_size;

fail:
    return ret;
}



/* prepare a new audio buffer */
/**
 * @brief sdl_audio_callback
 * @param opaque    指向user的数据
 * @param stream    拷贝PCM的地址
 * @param len       需要拷贝的长度
 */
static void sdl_audio_callback(void *opaque, Uint8 *stream, int len)
{ // 2ch 2字节 1024 = 4096 -> 回调每次读取2帧数据
    FFPlayer *is = (FFPlayer *)opaque;
    int audio_size, len1;

    while (len > 0) {   // 循环读取，直到读取到足够的数据
        /* (1)如果is->audio_buf_index < is->audio_buf_size则说明上次拷贝还剩余一些数据，
         * 先拷贝到stream再调用audio_decode_frame
         * (2)如果audio_buf消耗完了，则调用audio_decode_frame重新填充audio_buf
         */
        if (is->audio_buf_index >= is->audio_buf_size) {
            audio_size = audio_decode_frame(is);
            if (audio_size < 0) {
                /* if error, just output silence */
                is->audio_buf = NULL;
                is->audio_buf_size = SDL_AUDIO_MIN_BUFFER_SIZE / is->audio_tgt.frame_size
                        * is->audio_tgt.frame_size;
            } else {

                is->audio_buf_size = audio_size; // 讲字节 多少字节
            }
            is->audio_buf_index = 0;
        }
        //根据缓冲区剩余大小量力而行
        len1 = is->audio_buf_size - is->audio_buf_index;
        if (len1 > len)  // len = 3000 < len1 4096
            len1 = len;

        if (is->audio_buf)
            memcpy(stream, (uint8_t *)is->audio_buf + is->audio_buf_index, len1);

        len -= len1;
        stream += len1;
        /* 更新is->audio_buf_index，指向audio_buf中未被拷贝到stream的数据（剩余数据）的起始位置 */
        is->audio_buf_index += len1;
    }

//    is->audio_write_buf_size = is->audio_buf_size - is->audio_buf_index;
    /* Let's assume the audio driver that is used by SDL has two periods. */
    if (!isnan(is->audio_clock)) {
        // 设置时钟
        set_clock(&is->audclk, is->audio_clock);
    }
}


// 先参考我们之前讲的06-sdl-pcm范例
int FFPlayer::audio_open(int64_t wanted_channel_layout, int wanted_nb_channels, int wanted_sample_rate, AudioParams *audio_hw_params)
{
    SDL_AudioSpec wanted_spec;
    // 音频参数设置SDL_AudioSpec
    wanted_spec.freq = wanted_sample_rate;          // 采样频率
    wanted_spec.format = AUDIO_S16SYS; // 采样点格式
    wanted_spec.channels = wanted_nb_channels;          // 2通道
    wanted_spec.silence = 0;
    wanted_spec.samples = 2048;       // 23.2ms -> 46.4ms 每次读取的采样数量，多久产生一次回调和 samples
    wanted_spec.callback = sdl_audio_callback; // 回调函数
    wanted_spec.userdata = this;

//    SDL_OpenAudioDevice
    //打开音频设备
    if(SDL_OpenAudio(&wanted_spec, NULL) != 0)
    {
        printf("Failed to open audio device, %s\n", SDL_GetError());
        return -1;
    }


    // wanted_spec是期望的参数，spec是实际的参数，wanted_spec和spec都是SDL中的结构。
    // 此处audio_hw_params是FFmpeg中的参数，输出参数供上级函数使用
    // audio_hw_params保存的参数，就是在做重采样的时候要转成的格式。
    audio_hw_params->fmt = AV_SAMPLE_FMT_S16;
    audio_hw_params->freq = wanted_spec.freq;
    audio_hw_params->channel_layout = wanted_channel_layout;
    audio_hw_params->channels =  wanted_spec.channels;
    /* audio_hw_params->frame_size这里只是计算一个采样点占用的字节数 */
    audio_hw_params->frame_size = av_samples_get_buffer_size(NULL, audio_hw_params->channels,
                                                                1,
                                                                audio_hw_params->fmt, 1);
    audio_hw_params->bytes_per_sec = av_samples_get_buffer_size(NULL, audio_hw_params->channels,
                                                                audio_hw_params->freq,
                                                                audio_hw_params->fmt, 1);
    if (audio_hw_params->bytes_per_sec <= 0 || audio_hw_params->frame_size <= 0) {
        av_log(NULL, AV_LOG_ERROR, "av_samples_get_buffer_size failed\n");
        return -1;
    }
    // 比如2帧数据，一帧就是1024个采样点， 1024*2*2 * 2 = 8192字节
    return wanted_spec.size;	/* SDL内部缓存的数据字节, samples * channels *byte_per_sample */
}

void FFPlayer::audio_close()
{
    SDL_CloseAudio();  // SDL_CloseAudioDevice
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
//        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if(abort_request) {
            break;
        }

        // 7.读取媒体数据，得到的是音视频分离后、解码前的数据
        ret = av_read_frame(ic, pkt); // 调用不会释放pkt的数据，需要我们自己去释放packet的数据
        if(ret < 0) { // 出错或者已经读取完毕了
            if ((ret == AVERROR_EOF || avio_feof(ic->pb)) && !eof) {        // 读取完毕了
                eof = 1;
            }
            if (ic->pb && ic->pb->error)  // io异常 // 退出循环
                break;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));     // 读取完数据了，这里可以使用timeout的方式休眠等待下一步的检测
            continue;		// 继续循环
        } else {
            eof = 0;
        }
        // 插入队列  先只处理音频包
        if (pkt->stream_index == audio_stream) {
            printf("audio ===== pkt pts:%ld, dts:%ld\n", pkt->pts/48, pkt->dts);
            packet_queue_put(&audioq, pkt);
        } else if (pkt->stream_index == video_stream) {
            printf("video ===== pkt pts:%ld, dts:%ld\n", pkt->pts/48, pkt->dts);
            packet_queue_put(&videoq, pkt);
        } else {
            av_packet_unref(pkt);// // 不入队列则直接释放数据
        }
    }

    std::cout << __FUNCTION__ << " leave" << std::endl;

fail:

    return 0;
}
/* polls for possible required screen refresh at least this often, should be less than 1/fps */
#define REFRESH_RATE 0.01  // 每帧休眠10ms
int FFPlayer::video_refresh_thread()
{
    double remaining_time = 0.0;
    while (!abort_request) {
        if (remaining_time > 0.0)
            av_usleep((int)(int64_t)(remaining_time * 1000000.0));
        remaining_time = REFRESH_RATE;
        video_refresh(&remaining_time);
    }
    std::cout << __FUNCTION__ << " leave" << std::endl;
    return 0;
}

void FFPlayer::video_refresh(double *remaining_time)
{
    Frame *vp = NULL;
    // 目前我们先是只有队列里面有视频帧可以播放，就先播放出来
    // 判断有没有视频画面
    if(video_st) {
        if (frame_queue_nb_remaining(&pictq) == 0) {
            // 什么都不用做，可以直接退出了
            return;
        }

        // 能跑到这里说明帧队列不为空，肯定有frame可以读取
        vp = frame_queue_peek(&pictq);  // 读取待显示帧

        // 对比audio的时间戳
        double diff = vp->pts - get_clock(&audclk);// get_master_clock();

         std::cout << __FUNCTION__ << "vp->pts:" << vp->pts << " - af->pts:" << get_clock(&audclk) << ", diff:" << diff << std::endl;

        if(diff > 0) {
            *remaining_time = FFMIN(*remaining_time, diff);
            return;
        }


        // 刷新显示
        if(video_refresh_callback_)
            video_refresh_callback_(vp);
        else
            std::cout << __FUNCTION__ << " video_refresh_callback_ NULL" << std::endl;

        frame_queue_next(&pictq);   // 当前vp帧出队列
    }
}

void FFPlayer::AddVideoRefreshCallback(
        std::function<int (const Frame *)> callback)
{
    video_refresh_callback_ = callback;
}

int FFPlayer::get_master_sync_type()
{
    if (av_sync_type == AV_SYNC_VIDEO_MASTER) {
        if (video_st)
            return AV_SYNC_VIDEO_MASTER;
        else
            return AV_SYNC_AUDIO_MASTER;	 /* 如果没有视频成分则使用 audio master */
    } else if (av_sync_type == AV_SYNC_AUDIO_MASTER) {
        if (audio_st)
            return AV_SYNC_AUDIO_MASTER;
        else if(video_st)
            return AV_SYNC_VIDEO_MASTER;        // 只有音频的存在
        else
            return AV_SYNC_UNKNOW_MASTER;
    }
}

double FFPlayer::get_master_clock()
{
    double val;

    switch (get_master_sync_type()) {
    case AV_SYNC_VIDEO_MASTER:
//        val = get_clock(&vidclk);
        break;
    case AV_SYNC_AUDIO_MASTER:
        val = get_clock(&audclk);
        break;
    default:
        val = get_clock(&audclk);
        break;
    }
    return val;
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
                    AVRational tb{1, frame->sample_rate};    //
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
                    printf("audio dec:%s, ret:%d,%d\n", errStr, ret, AVERROR(EAGAIN));
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
            tb = AVRational{1, frame->sample_rate};   // 设置为sample_rate为timebase

            // 2. 获取可写Frame
            if (!(af = frame_queue_peek_writable(&is->sampq)))  // 获取可写帧
                goto the_end;
            // 3. 设置Frame并放入FrameQueue
            af->pts = (frame->pts == AV_NOPTS_VALUE) ? NAN : frame->pts * av_q2d(tb);  // 转换时间戳
//            af->pos = frame->pkt_pos;
//            af->serial = is->auddec.pkt_serial;
           af->duration = av_q2d(AVRational{frame->nb_samples, frame->sample_rate});

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
         duration = (frame_rate.num && frame_rate.den) ? av_q2d(AVRational{frame_rate.den, frame_rate.num}) : 0;
         // 根据AVStream timebase计算出pts值, 单位为秒
         pts = (frame->pts == AV_NOPTS_VALUE) ? NAN : frame->pts * av_q2d(tb);  // 单位为秒
         // 5 将解码后的视频帧插入队列
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








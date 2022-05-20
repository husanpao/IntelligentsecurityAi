//
// Created by UMA-PC-001 on 2022/2/17.
//

#include "CameraPush.h"

CameraPush::~CameraPush() {
    if (sws_ctx) {
        sws_freeContext(sws_ctx);
    }
    if (outFrame) {
        av_frame_free(&outFrame);
    }
    if (encodecCtx) {
        avcodec_close(encodecCtx);
    }
    if (encodecCtx) {
        avcodec_free_context(&encodecCtx);
    }
    if (outCtx) {
        avformat_close_input(&outCtx);
    }
    if (outStream) {
        av_freep(outStream);
    }
    SPDLOG_INFO("[{}] ~FfmpegTool", this->cameraid);
}

CameraPush::CameraPush() {}

CameraPush::CameraPush(int width, int height, int fps, const char *url, const char *cameraid) : m_width(width),
                                                                                                m_height(height),
                                                                                                m_fps(fps),
                                                                                                m_url(url),
                                                                                                cameraid(cameraid) {
    sws_ctx = nullptr;
    outFrame = nullptr;
    encodecCtx = nullptr;
    outCtx = nullptr;
    codec = nullptr;
    outStream = nullptr;
}

bool CameraPush::Init() {
    bool flag = false;
    avformat_network_init();    //注册所有网络协议

//    this->init_RGB_to_YUV();
    flag = this->InitEncodeContext();
    if (!flag) { return flag; }
    flag = this->InitOutPutData();
    if (!flag) { return flag; }
    flag = this->CreatFormatContext();
    return flag;
}

bool CameraPush::InitOutPutData() {
    outFrame = av_frame_alloc();
    outFrame->format = *codec->pix_fmts;
    outFrame->height = m_height;
    outFrame->width = m_width;
    outFrame->pts = 0;

    int iRet = av_frame_get_buffer(outFrame, 0);
    if (iRet != 0) {
        char buf[1024] = {0};
        av_strerror(iRet, buf, sizeof(buf) - 1);
        SPDLOG_ERROR("[{}] av_frame_get_buffer {} code:{}", this->cameraid, buf, iRet);
        return false;
    }
    iRet = av_frame_make_writable(outFrame);
    if (iRet != 0) {
        char buf[1024] = {0};
        av_strerror(iRet, buf, sizeof(buf) - 1);
        SPDLOG_ERROR("[{}] av_frame_make_writable {}", this->cameraid, buf);
        return false;
    }
    return true;
}

bool CameraPush::InitEncodeContext() {
    codec = avcodec_find_encoder(AV_CODEC_ID_H264); //找到编码协议是H264
//    codec = avcodec_find_encoder_by_name("h264_nvenc"); //找到编码协议是H264

    if (!codec) {
        SPDLOG_ERROR("[{}]Can`t find h264 encoder!", cameraid);
        return false;
    }
    encodecCtx = avcodec_alloc_context3(codec); //创建编译器上下文

    if (!encodecCtx) {
        SPDLOG_ERROR("[{}]avcodec_alloc_context3 failed!", cameraid);
        return false;
    }

    //设置编码器参数
    encodecCtx->codec_id = codec->id;
    encodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER; //全局参数
    encodecCtx->thread_count = 1;

    encodecCtx->bit_rate = 50 * 1024 * 8;//压缩后每秒视频的bit位大小 50kB
    encodecCtx->width = m_width;
    encodecCtx->height = m_height;

    encodecCtx->time_base.num = 1;
    encodecCtx->time_base.den = m_fps;
    encodecCtx->framerate.den = m_fps;
    encodecCtx->framerate.num = 1;

    encodecCtx->qmin = 30;   //调节清晰度和编码速度 //这个值调节编码后输出数据量越大输出数据量越小，越大编码速度越快，清晰度越差
    encodecCtx->qmax = 51;

    encodecCtx->gop_size = 50;   //编码一旦有gopsize很大的时候或者用了opencodec，有些播放器会等待I帧，无形中增加延迟。
    encodecCtx->max_b_frames = 0;    //编码时如果有B帧会再解码时缓存很多帧数据才能解B帧，因此只留下I帧和P帧。
    encodecCtx->pix_fmt = *codec->pix_fmts;


    AVDictionary *param = 0;
    av_dict_set(&param, "preset", "superfast", 0);  //编码形式修改
    av_dict_set(&param, "tune", "zerolatency", 0);  //实时编码

    int ret = avcodec_open2(encodecCtx, codec, &param); //打开编码器上下文
    if (ret != 0) {
        char buf[1024] = {0};
        av_strerror(ret, buf, sizeof(buf) - 1);
        SPDLOG_ERROR(buf);
        return false;
    }
    SPDLOG_INFO("[{}]stream channel start success", cameraid);
    return true;
}

bool CameraPush::CreatFormatContext() {
    int ret = avformat_alloc_output_context2(&outCtx, 0, "flv", m_url); //rtmp 使用flv
    if (ret != 0) {
        char buf[1024] = {0};
        av_strerror(ret, buf, sizeof(buf) - 1);
        SPDLOG_ERROR("[{}]avformat_alloc_output_context2 error,{}", cameraid, buf);
        return false;
    }

    //添加视频流
    outStream = avformat_new_stream(outCtx, encodecCtx->codec);
    if (!outStream) {
        SPDLOG_ERROR("[{}]avformat_new_stream failed", cameraid);
        return false;
    }
    ret = avcodec_parameters_from_context(outStream->codecpar, encodecCtx);
    if (ret != 0) {
        char buf[1024] = {0};
        av_strerror(ret, buf, sizeof(buf) - 1);
        SPDLOG_ERROR("[{}]avcodec_parameters_from_context error,{}", cameraid, buf);
        return false;
    }

    outStream->time_base.num = 1;
    outStream->time_base.den = m_fps;

    //打开rtmp 的网络输出IO
    SPDLOG_INFO("[{}] push url:{}", this->cameraid, m_url);
    ret = avio_open(&outCtx->pb, m_url, AVIO_FLAG_WRITE);
    if (ret != 0) {
        char buf[1024] = {0};
        av_strerror(ret, buf, sizeof(buf) - 1);
        SPDLOG_ERROR("[{}]avio_open failed,{}", cameraid, buf);
        return false;
    }
    av_dump_format(outCtx, 0, m_url, 1);
    // AVDictionary* options = NULL;
    // av_dict_set(&options, "rtsp_transport", "tcp", 0);
    // av_dict_set(&options, "stimeout", "8000000", 0);  //设置超时时间
    //写入封装头
    ret = avformat_write_header(outCtx, NULL);
    if (ret != 0) {
        char buf[1024] = {0};
        av_strerror(ret, buf, sizeof(buf) - 1);
        SPDLOG_ERROR("[{}]avformat_write_header,{} ,ret:{}", cameraid, buf, ret);
        return false;
    }
    return true;
}

bool CameraPush::pusher(cv::Mat &frame, int frameCount) {
    AVPacket pkt;
    av_init_packet(&pkt);
    cvmatToAvframe(&frame, outFrame);
    int ret = avcodec_send_frame(encodecCtx, outFrame);
    if (ret != 0) {
        char buf[1024] = {0};
        av_strerror(ret, buf, sizeof(buf) - 1);
        SPDLOG_ERROR("[{}]avcodec_encode_video2 fail -------{}------ ret{}", cameraid, buf, ret);
        return false;
    }
    ret = avcodec_receive_packet(encodecCtx, &pkt);
    if (0 == ret) {
        pkt.stream_index = outStream->index;
        AVRational itime = encodecCtx->time_base;
        AVRational otime = outStream->time_base;

        pkt.pts = av_rescale_q_rnd(pkt.pts, itime, otime, (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, itime, otime, (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q_rnd(pkt.duration, itime, otime,
                                        (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.pos = -1;
        ret = av_interleaved_write_frame(outCtx, &pkt);
        if (ret == 0) {
            SPDLOG_DEBUG("[{}]push frame success!", cameraid);
            return true;
        } else {
            char buf[1024] = {0};
            av_strerror(ret, buf, sizeof(buf) - 1);
            SPDLOG_ERROR("[{}]push frame failed!,{}", cameraid, buf);
        }
    }
    return false;
}
//
// Created by UMA-PC-001 on 2022/2/17.
//

#ifndef WATCHAI_CAMERAPULL_H
#define WATCHAI_CAMERAPULL_H

#include "watchai.h"

using namespace std;
struct StreamInfo {
    double height;//视频高度
    double width;//视频宽度
    double fps;//视频fps
    string url;//视频地址
    string id;//视频对应的摄像头id
};

class CameraPull {
public:
    ~CameraPull();
    bool flag;
    CameraPull(string url, string id);
    int start();
    bool stop();
    cv::Mat get();
    StreamInfo info();

private:
    Queue queue;//存储最新视频数据
    string url;//视频地址
    string id;//视频对应摄像头id
    StreamInfo streaminfo;//视频信息结构体
    mutex m_mutex;//同步锁
    int height;//视频高
    int width;//视频宽
private:
    AVFormatContext *fmtContext;
    AVStream *video;
    const AVCodec *videoCodec;
    AVCodecContext *videoCodecContext;
    AVFrame *frame;
    AVPacket *videoPacket;
    int videoIdx;
    int ret;
};


#endif //WATCHAI_CAMERAPULL_H

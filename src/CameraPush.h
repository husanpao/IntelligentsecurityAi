//
// Created by UMA-PC-001 on 2022/2/17.
//

#ifndef WATCHAI_CAMERAPUSH_H
#define WATCHAI_CAMERAPUSH_H

#include "watchai.h"

class CameraPush {
public:
    ~CameraPush();

    CameraPush();

    CameraPush(int width, int height, int fps, const char *url, const char *cameraid);

    bool Init();

    bool InitOutPutData();  //初始化输出格式

    bool InitEncodeContext(); //初始化编码上下文

    bool CreatFormatContext();//创建封装器上下文

    bool pusher(cv::Mat &frame, int frameCount);//推流
private:
    SwsContext *sws_ctx;   //像素格式转换上下文
    AVFrame *outFrame;                  //输出数据结构
    AVCodecContext *encodecCtx;    //编码器上下文
    AVFormatContext *outCtx;    //rtmp flv 封装器
    const AVCodec *codec; //视频编码器
    AVStream *outStream;
    int m_width;    //视频宽度
    int m_height;   //视频高度
    int m_fps;      //视频帧数
    const char *m_url;    //URL地址
    const char *cameraid;    //摄像头id
};


#endif //WATCHAI_CAMERAPUSH_H

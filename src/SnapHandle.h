//
// Created by uma-pc001 on 2022/7/5.
//

#ifndef WATCHAI_SNAPHANDLE_H
#define WATCHAI_SNAPHANDLE_H

#include "watchai.h"
#include "YoloV5.h"
#include "CameraPull.h"

class SnapHandle {
public:
    ~SnapHandle();

    SnapHandle(AppConfig
               *appConfig,
               YoloV5 *yolo, string id, string url,
               set<string> algorithm_list);

    void startPrediction();
private:
    void drawImg(cv::Mat &frame, PredictionResult pr);

    string writeImg(cv::Mat &frame, string cameraid, string event);

    string getDayStr();

    void sendEventMsg();

    void Handle(cv::Mat frame);

private:
    string id;//摄像头id
    string url;//视频地址

    bool predictionFlag;//是否启用识别
    CameraPull *cameraPull;//拉取器
    YoloV5 *yolo;
    map<string, string> cameraPics;
    AppConfig *appConfig;//程序配置信息
    set<string> algorithm_list;//启用的算法列表
    map<string, string> result;
};


#endif //WATCHAI_SNAPHANDLE_H

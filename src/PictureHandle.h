//
// Created by uma-pc001 on 2022/5/19.
//

#ifndef WATCHAI_PICTUREHANDLE_H
#define WATCHAI_PICTUREHANDLE_H

#include "watchai.h"
#include "YoloV5.h"

class PictureHandle {

public:
    ~PictureHandle();

    PictureHandle(AppConfig *appConfig, YoloV5 *yolo, map<string, string> cameraPics, set<string> algorithm_list);

    void startPrediction();

private:
    void drawImg(cv::Mat &frame, PredictionResult pr);

    string writeImg(cv::Mat &frame, string cameraid, string event);

    string getDayStr();

    void sendEventMsg();

private:
    YoloV5 *yolo;
    map<string, string> cameraPics;
    AppConfig *appConfig;//程序配置信息
    set<string> algorithm_list;//启用的算法列表
    map<string, string> result;
};


#endif //WATCHAI_PICTUREHANDLE_H

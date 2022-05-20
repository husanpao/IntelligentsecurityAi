//
// Created by UMA-PC-001 on 2022/2/17.
//

#ifndef WATCHAI_CAMERAHANDLE_H
#define WATCHAI_CAMERAHANDLE_H

#include "watchai.h"
#include "CameraPull.h"
#include "CameraPush.h"
#include "YoloV5.h"
#include "ArcFace.h"
#include "EventCenter.h"

class CameraHandle {
public:
    ~CameraHandle();

    CameraHandle(AppConfig *appConfig, string url, string id, set<string> algorithm_list, YoloV5 *yolo,
                 EventCenter *eventCenter);

    void startPrediction();

    bool stopPrediction();

    bool startRender();

    bool stopRender();

    bool UpdateAlgorithm_list(set<string> algorithm_list);

public:
    ArcFace *facetool;
    bool predictionFlag;//是否启用识别
    bool renderFlag = false;//是否启用渲染
private:
    void Handle(cv::Mat frame);

    void initTimeEvent();

    void EndVideo();

    void DownloadVideo();

    void handleTimeEvent(cv::Mat frame);

    bool CheckSmoke(PredictionResult smoke);

    bool CheckWdAqd(PredictionResult wdaqd);

    bool CheckWdJyst(PredictionResult wdjyst);

    bool CheckDbxzr(PredictionResult &dbxzr);

    bool CheckRydd(PredictionResult rydd);

    bool CheckWdJz(PredictionResult wdjz);

    bool CheckWcgzf(PredictionResult wcgzf);

    bool CheckWithPerson(PredictionResult person);

    bool CheckWithWrft(PredictionResult wrft);


private:
    AppConfig *appConfig;//程序配置信息
    CameraPull *cameraPull;//拉取器
    CameraPush *cameraPush;//推送器
    string id;//摄像头id
    string url;//视频地址
    string pushUrl;//渲染推送地址
    int frameCount;//帧数
    int pushIdx = 0;//发送帧序列
    int showhold;//控制是否显示系数


    map<string, PDRV> allResultBoxes;//模型分类集合
    PDRV predictionsBoxes;//识别后的数据保存集合

    FSRV faceBoxes;//人脸数据集合
    set<string> algorithm_list;//启用的算法列表
    set<string> notRender;//不参与渲染的列表
    map<string, long long> timeEvents;
    YoloV5 *yolo;
    EventCenter *eventCenter;
    thread_pool *taskPool;
    Queue video;
};


#endif //WATCHAI_CAMERAHANDLE_H

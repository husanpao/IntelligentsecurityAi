//
// Created by UMA-PC-001 on 2022/2/17.
//
#pragma once
#ifndef WATCHAI_WATCHAI_H
#define WATCHAI_WATCHAI_H

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/freetype.hpp>
#include <spdlog/spdlog.h>
#include "mutex"
#include "hv/hv.h"
#include "hv/htime.h"
#include <rapidjson/document.h>
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "hv/http_client.h"
#include "hv/requests.h"
#include "thread_pool.hpp"
#include "seeta/Common/CStruct.h"

extern "C" {
#include "libavutil/avutil.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include <libavutil/imgutils.h>
}
using namespace std;
typedef std::queue<cv::Mat> Queue;

struct EventInfo {
    string label;
    float hold;
    int event;
    int left;
    int right;
    int top;
    int bottom;
};
struct PredictionResult {
    float hold;
    string event;
    EventInfo eventInfo;
};
typedef std::vector<PredictionResult> PDRV;

struct FaceResult {
    int64_t id;
    string name;
    string uuid;
    SeetaRect pos;
    float score;
};
typedef std::vector<FaceResult> FSRV;

cv::Mat avframeToCvmat(const AVFrame *frame, AVPixelFormat format, int width, int height);

AVFrame *cvmatToAvframe(cv::Mat *image, AVFrame *frame);

int httpDownload(const char *url, const char *filepath);

std::wstring Utf8ToUnicode(const std::string &strUTF8);

std::string WStringToString(const std::wstring &wstr);
std::string UnicodeToUtf8(const std::wstring &strUnicode);

std::wstring StringToWString(const std::string &str);
bool intersect(PredictionResult a, PredictionResult b);

float overlapRate(PredictionResult a, PredictionResult b);

struct AppConfig {
    string rtmp_server;
    string static_path;
    string static_host;
    string api_host;
    string live_server;
    string weight;
    string lables;
    char *faceKey;
    int interval;
    int time_interval;
    int maxstream;
    double facehold;
    int showhold;
    int frame_interval;
    map<string, string> mappers;
};
#endif //WATCHAI_WATCHAI_H


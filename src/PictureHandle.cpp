//
// Created by uma-pc001 on 2022/5/19.
//

#include "PictureHandle.h"

PictureHandle::~PictureHandle() {

}

PictureHandle::PictureHandle(AppConfig *appConfig, YoloV5 *yolo, map<string, string> cameraPics,
                             set<string> algorithm_list) : appConfig(appConfig), yolo(yolo), cameraPics(cameraPics),
                                                           algorithm_list(algorithm_list) {
    datetime_t now = datetime_now();
    string month = fmt::format("{}", now.month);
    if (now.month < 10) {
        month = fmt::format("0{}", now.month);
    }
    string day = fmt::format("{}", now.day);
    if (now.day < 10) {
        day = fmt::format("0{}", now.day);
    }
    string temp = fmt::format("{}/{}{}{}/", this->appConfig->static_path, now.year, month, day);
    if (access(temp.c_str(), F_OK) ==
        -1) {
        mkdir(temp.c_str());
    }
    if (access("temp", F_OK) ==
        -1) {
        mkdir("temp");
    }
}

void PictureHandle::drawImg(cv::Mat &frame, PredictionResult drawInfo) {

    string showLabel = drawInfo.eventInfo.label;

    if (this->appConfig->mappers.count(showLabel) != 0) {
        showLabel = this->appConfig->mappers[showLabel];
    }

    if (this->appConfig->showhold == 1) {
        showLabel = fmt::format("{0}_{1:.2f}", showLabel,
                                drawInfo.eventInfo.hold);
    }

    this->yolo->drawRectangle(frame, drawInfo.eventInfo.left,
                              drawInfo.eventInfo.top,
                              drawInfo.eventInfo.right, drawInfo.eventInfo.bottom,
                              showLabel);
}

string PictureHandle::getDayStr() {
    datetime_t now = datetime_now();
    string month = fmt::format("{}", now.month);
    if (now.month < 10) {
        month = fmt::format("0{}", now.month);
    }
    string day = fmt::format("{}", now.day);
    if (now.day < 10) {
        day = fmt::format("0{}", now.day);
    }
    return fmt::format("{}{}{}", now.year, month, day);
}

string PictureHandle::writeImg(cv::Mat &frame, string cameraid, string event) {
    long long time = gettimeofday_ms();
    string drawImgPath = fmt::format("{}\\{}\\{}_{}_{}.jpg", this->appConfig->static_path, getDayStr(),
                                     cameraid,
                                     event,
                                     time);
    cv::imwrite(drawImgPath, frame);
    return fmt::format("{}/{}_{}_{}.jpg", getDayStr(), cameraid, event, time);
}

void PictureHandle::startPrediction() {
    for (auto iter = this->cameraPics.begin(); iter != this->cameraPics.end(); ++iter) {
        string fullpath = hv::replace(this->appConfig->api_host, "/api", "");
        string temppic = fmt::format("temp/{}_{}.jpg", iter->first, gettimeofday_ms());
        fullpath.append(iter->second);
        int ret = httpDownload(fullpath.c_str(), temppic.c_str());
        SPDLOG_INFO("url:{} download:{} finish...", fullpath, temppic);
        if (ret != 0) {
            remove(temppic.c_str());
            continue;
        }
        auto srcFrame = cv::imread(temppic);
        if (srcFrame.empty()) {
            remove(temppic.c_str());
            continue;
        }
        int nRows = 1280;
        int nCols = srcFrame.cols * 1280 / srcFrame.rows;
        cv::Mat frame(nRows, nCols, srcFrame.type());
        resize(srcFrame, frame, frame.size(), 0, 0, cv::INTER_LINEAR);
        SPDLOG_INFO("{} read pic success", temppic);
        PDRV tempBoxs = this->yolo->prediction_my(frame, this->algorithm_list);
        SPDLOG_INFO("{} pred count...", temppic, tempBoxs.size());
        if (tempBoxs.size() > 0) {
            PDRV headHelmet;
            PDRV persons;
            for (PredictionResult drawInfo: tempBoxs) {
                if (drawInfo.event == "helmet" || drawInfo.event == "head") {
                    headHelmet.push_back(drawInfo);
                }
                if (drawInfo.event == "person") {
                    persons.push_back(drawInfo);
                }
            }
            bool flag = false;
            for (PredictionResult person: persons) {
                SPDLOG_INFO("{} find person...", temppic);
                for (PredictionResult head: headHelmet) {
                    if (intersect(head, person)) {
                        this->drawImg(frame, person);
                        flag = true;
                    }
                }
            }
            if (flag) {
                auto url = this->writeImg(frame, iter->first, "person");
                this->result.insert({iter->first, url});
            }
        }
        remove(temppic.c_str());
    }
    sendEventMsg();
}

void PictureHandle::sendEventMsg() {
    datetime_t now = datetime_now();
    rapidjson::StringBuffer buf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
    writer.StartObject();
    writer.Key("msgType");
    writer.Int(303);
    writer.Key("data");
    writer.StartArray();
    string datestr = fmt::format("{}-{}-{} {}:{}:{}", now.year, now.month, now.day, now.hour, now.min,
                                 now.sec);
    for (auto iter = this->result.begin(); iter != this->result.end(); ++iter) {
        writer.StartObject();
        writer.Key("ALGORITHM_ID");
        writer.String("person");
        writer.Key("CAMERA_ID");
        writer.String(iter->first.c_str());
        writer.Key("VIOLATION_TIME");
        writer.String(datestr.c_str());
        writer.Key("VIOLATION_PIC");
        writer.String(fmt::format("{}{}", this->appConfig->static_host, iter->second).c_str());
        writer.EndObject();
    }
    writer.EndArray();
    writer.EndObject();
    string result = buf.GetString();
    SPDLOG_INFO("PicEvent send body:{}", result);
    http_headers headers;
    headers.insert({"Content-Type", "application/json"});
    auto resp = requests::post(this->appConfig->api_host.c_str(), result, headers);
    if (resp == nullptr) {
        SPDLOG_ERROR("PicEvent request failed! body {}", result);
    } else {
        SPDLOG_INFO("PicEvent request body{} ", resp->body.c_str());
    }
}
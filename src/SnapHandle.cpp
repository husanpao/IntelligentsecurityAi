//
// Created by uma-pc001 on 2022/7/5.
//

#include "SnapHandle.h"

SnapHandle::~SnapHandle() {
    delete this->cameraPull;
    this->cameraPull = nullptr;
    SPDLOG_INFO("[{}] ~SnapHandle", this->id);
}

SnapHandle::SnapHandle(AppConfig *appConfig, YoloV5 *yolo, string id, string url, set<string> algorithm_list) : url(
        url), id(id) {
    this->appConfig = appConfig;
    this->algorithm_list = algorithm_list;
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
    this->cameraPull = new CameraPull(this->url, id);
    this->yolo = yolo;
    this->result.insert({this->id, ""});
}

void SnapHandle::startPrediction() {
    this->predictionFlag = true;
    thread *handle = new thread([this] {
        cv::waitKey(100);
        SPDLOG_INFO("[{}] Handle start ", this->id);
        while (this->predictionFlag) {
            auto frame = this->cameraPull->get();
            if (frame.empty()) {
                cv::waitKey(1);
                continue;
            }
            this->Handle(frame);
            this->cameraPull->stop();
            break;
        }
        SPDLOG_INFO("[{}] Handle end ", this->id);
    });
    handle->detach();
    SPDLOG_INFO("[{}] start prediction ", this->id);
    this->cameraPull->start();
    this->predictionFlag = false;
    delete handle;
    sendEventMsg();
    cv::waitKey(5);
    SPDLOG_INFO("[{}] end prediction ", this->id);
}

void SnapHandle::drawImg(cv::Mat &frame, PredictionResult drawInfo) {

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

string SnapHandle::writeImg(cv::Mat &frame, string cameraid, string event) {
    long long time = gettimeofday_ms();
    string drawImgPath = fmt::format("{}\\{}\\{}_{}_{}.jpg", this->appConfig->static_path, getDayStr(),
                                     cameraid,
                                     event,
                                     time);
    cv::imwrite(drawImgPath, frame);
    return fmt::format("{}/{}_{}_{}.jpg", getDayStr(), cameraid, event, time);
}

void SnapHandle::Handle(cv::Mat frame) {
    if (frame.empty()) {
        return;
    }
    PDRV tempBoxs = this->yolo->prediction_my(frame, this->algorithm_list);
    SPDLOG_INFO("{} pred count {}...", this->id, tempBoxs.size());
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
            for (PredictionResult head: headHelmet) {
                if (intersect(head, person)) {
                    this->drawImg(frame, person);
                    flag = true;
                    break;
                }
            }
        }
        if (flag) {
            auto picurl = this->writeImg(frame, this->id, "person");
            frame.release();
            this->result[this->id] = picurl;
        }
    }
}

string SnapHandle::getDayStr() {
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

void SnapHandle::sendEventMsg() {
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
//
// Created by UMA-PC-001 on 2022/2/17.
//

#include "CameraHandle.h"

CameraHandle::~CameraHandle() {

}

CameraHandle::CameraHandle(AppConfig *appConfig, string url, string id, set<string> algorithm_list, YoloV5 *yolo,
                           EventCenter *eventCenter) : url(
        url), id(id) {
    this->appConfig = appConfig;
    this->showhold = appConfig->showhold;
    this->algorithm_list = algorithm_list;
    initTimeEvent();
    if (hv::startswith(this->id, "offline")) {
        DownloadVideo();
    }
    this->cameraPull = new CameraPull(this->url, id);
    this->cameraPush = new CameraPush(1280, 720, 25,
                                      "",
                                      this->id.c_str());
    this->pushUrl = appConfig->rtmp_server;
    this->pushUrl.append(this->id);
    this->yolo = yolo;
    this->eventCenter = eventCenter;
//    this->eventCenter = new EventCenter(this->appConfig->showhold, yolo, this->appConfig->static_path,
//                                        this->appConfig->static_host, this->appConfig->api_host,
//                                        this->appConfig->mappers, this->appConfig->interval);
    this->taskPool = new thread_pool(1);
    this->notRender.insert("person");
//    this->notRender.insert("aqwl");
    this->notRender.insert("gzfzr");
    this->notRender.insert("zzfzr");
    this->notRender.insert("helmet");
    SPDLOG_INFO("[{}] PushUrl is {} ", this->id, this->pushUrl);
}

void CameraHandle::initTimeEvent() {
    timeEvents.clear();
    string timelabel = "aqwl,gzfzr,zzfzr";
    for (auto algorithm: this->algorithm_list) {
        if (hv::contains(timelabel, algorithm)) {
            if (algorithm == "gzfzr") {
                timeEvents["gzfzr"] = gettimeofday_ms();
                timeEvents["zzfzr"] = gettimeofday_ms();
            } else {
                timeEvents[algorithm.c_str()] = gettimeofday_ms();
            }
            SPDLOG_INFO("[{}] add timeEvent  {} ", this->id, algorithm);
        }
    }
}


void CameraHandle::DownloadVideo() {
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
    string video_str = fmt::format("{}{}.mp4", temp, this->id);
    string fullpath = hv::replace(this->appConfig->api_host, "/api", "");
    httpDownload(fullpath.append(this->url).c_str(), video_str.c_str());
    this->url = video_str;
}

bool CameraHandle::CheckSmoke(PredictionResult smoke) {
    bool flag = false;
    if (this->allResultBoxes.count("head") != 0) {
        for (auto head: this->allResultBoxes["head"]) {
            flag = intersect(smoke, head);
            if (flag) {
                int s_w = smoke.eventInfo.right - smoke.eventInfo.left;
                int h_w = head.eventInfo.right - head.eventInfo.left;
                float bs = (float) s_w / (float) h_w;

                if (bs > 0.75) {
                    return false;
                }
                float r = overlapRate(smoke, head);
                SPDLOG_INFO("{} rate with head is :{:03.2f}  hold: {:03.2f}",
                            smoke.event,
                            r,
                            smoke.hold);
                break;
            }
        }
    }
    if (this->allResultBoxes.count("helmet") != 0 && !flag) {
        for (const auto &helmet: this->allResultBoxes["helmet"]) {
            flag = intersect(smoke, helmet);
            if (flag) {
                int s_w = smoke.eventInfo.right - smoke.eventInfo.left;
                int h_w = helmet.eventInfo.right - helmet.eventInfo.left;
                float bs = (float) s_w / (float) h_w;
                if (bs > 0.75) {
                    return false;
                }
                float r = overlapRate(smoke, helmet);
                SPDLOG_INFO("{} rate with helmet is :{:03.2f}  hold: {:03.2f}",
                            smoke.event,
                            r,
                            smoke.hold);
                break;
            }
        }
    }
    return flag;
}

bool CameraHandle::CheckWdAqd(PredictionResult offground) {
    bool flag = false;
    if (offground.hold < 0.83) {
        return flag;
    }
    //判断是不是人
    if (this->allResultBoxes.count("person") != 0) {
        for (const auto &person: this->allResultBoxes["person"]) {
            flag = intersect(offground, person);
            if (flag) {
                break;
            }
        }
    }
    //不是人直接return
    if (!flag) {
        return flag;
    }
    flag = false;
    //判断存不存在未系安全带
    if (this->allResultBoxes.count("badsafebelt") != 0) {
        for (const auto &badsafebelt: this->allResultBoxes["badsafebelt"]) {
            flag = intersect(offground, badsafebelt);
            if (flag) {
                break;
            }
        }
    }
    //如果存在未系安全带直接跑出去
    if (flag) {
        return flag;
    }
    //判断是离地状态，有没有系安全带
    if (this->allResultBoxes.count("safebelt") != 0) {
        for (const auto &safebelt: this->allResultBoxes["safebelt"]) {
            flag = intersect(offground, safebelt);
            if (flag) {
                flag = false;
                break;
            } else {
                flag = true;
            }
        }
    } else {
        flag = true;
    }
    return flag;
}

bool CameraHandle::CheckWdJyst(PredictionResult predictionResult) {
    bool flag = false;
    if (this->allResultBoxes.count("gz") != 0) {
        for (auto gz: this->allResultBoxes["gz"]) {
            flag = intersect(predictionResult, gz);
            if (flag) {
                break;
            }
        }
    }
    if (this->allResultBoxes.count("onlyjyst") != 0 && flag) {
        for (const auto &refrent: this->allResultBoxes["onlyjyst"]) {
            flag = intersect(predictionResult, refrent);
            if (flag) {
                flag = false;
                break;
            }
        }
    }
    return flag;
}

bool CameraHandle::CheckDbxzr(PredictionResult &db) {
    bool flag = false;
    if (this->allResultBoxes.count("person") != 0) {
        int i = 0;
        for (const auto &person: this->allResultBoxes["person"]) {
            int width = db.eventInfo.right - db.eventInfo.left;
            int height = db.eventInfo.bottom - db.eventInfo.top;
            bool qflag = false;
            if (db.eventInfo.bottom < 420) {
                db.eventInfo.bottom = 420;
            }
            if (height > width && i++ == 0) {
                if (db.eventInfo.left > 640) {
                    db.eventInfo.left = db.eventInfo.left - db.eventInfo.bottom / 2;
                    if (db.eventInfo.left < 0) {
                        db.eventInfo.left = 10;
                    }
                } else {
                    db.eventInfo.right = db.eventInfo.right + db.eventInfo.bottom / 2;
                    if (db.eventInfo.right > 1280) {
                        db.eventInfo.right = 1270;
                    }
                }
                qflag = true;
            }

            width = db.eventInfo.right - db.eventInfo.left;
            int personwidth = person.eventInfo.right - person.eventInfo.left;
            int ps = width / personwidth;
            flag = intersect(db, person);
//            if (person.eventInfo.left > db.eventInfo.left &&
//                person.eventInfo.top > db.eventInfo.top &&
//                person.eventInfo.right < db.eventInfo.right &&
//                person.eventInfo.bottom < db.eventInfo.bottom) {
//                flag = true;
////                break;
//            }
//            SPDLOG_INFO("db width:{} person width:{} beisu:{} flag:{} qflag:{}", width, personwidth, ps, flag, qflag);
            if (flag && qflag ? ps < 25 && ps > 6 : ps < 11 && ps > 7) {
                break;
            } else {
                flag = false;
            }
//            flag = intersect(db, refrent);
//            if (flag) {
//                break;
//            }
//            if (refrent.eventInfo.left > predictionResult.eventInfo.left &&
//                refrent.eventInfo.top > predictionResult.eventInfo.top &&
//                refrent.eventInfo.right < predictionResult.eventInfo.right &&
//                refrent.eventInfo.bottom < predictionResult.eventInfo.bottom) {
//                this->predictionsBoxes.push_back(refrent);
//                flag = true;
//                break;
//            }
        }
    }
    return flag;
}

bool CameraHandle::CheckRydd(PredictionResult rydd) {
    int w = rydd.eventInfo.right - rydd.eventInfo.left;
    int h = rydd.eventInfo.bottom - rydd.eventInfo.top;
    float bs = (float) w / (float) h;
    if (bs < 1.3) {
        return false;
    }
    return CheckWithPerson(rydd, 0.2);
}

bool CameraHandle::CheckWcgzf(PredictionResult wcgzf) {
    bool flag = CheckWithPerson(wcgzf, 0.1);
    if (!flag) {
        return flag;
    }
    flag = false;
    if (this->allResultBoxes.count("head") != 0) {
        for (auto refrent: this->allResultBoxes["head"]) {
            flag = intersect(wcgzf, refrent);
            if (flag) {
                return false;
            }
        }
    }
    if (this->allResultBoxes.count("helmet") != 0) {
        for (const auto &refrent: this->allResultBoxes["helmet"]) {
            flag = intersect(wcgzf, refrent);
            if (flag) {
                return false;
            }
        }
    }
    if (this->allResultBoxes.count("onlyjyst") != 0) {
        for (const auto &onlyjyst: this->allResultBoxes["onlyjyst"]) {
            flag = intersect(wcgzf, onlyjyst);
            if (flag) {
                return false;
            }
        }
    }

    if (this->allResultBoxes.count("pifu") != 0) {
        for (const auto &pifu: this->allResultBoxes["pifu"]) {
            flag = intersect(wcgzf, pifu);
            if (flag) {
                return true;
            }
        }
    }
    return false;
}

bool CameraHandle::CheckWdJz(PredictionResult person) {
//    SPDLOG_INFO(" person:left:{} right:{} top:{} bottom:{}", person.eventInfo.left,
//                person.eventInfo.right, person.eventInfo.top, person.eventInfo.bottom);
    bool flag = false;
    //先判断是不是远距离
    flag = (person.eventInfo.right - person.eventInfo.left) > 130;
    if (!flag) {
//        SPDLOG_INFO("[{}] too yuan.....{}", this->id, person.eventInfo.right - person.eventInfo.left);
        return flag;
    }
    flag = false;
    //再判断是不是绝对正脸
    if (this->allResultBoxes.count("face") != 0) {
        for (const auto &face: this->allResultBoxes["face"]) {
            flag = intersect(person, face);
            if (flag) {
                break;
            }
        }
    }
    if (!flag) {
//        SPDLOG_INFO("[{}] no face.....", this->id);
        return flag;
    }
    //在判断是否识别到驾照
    if (this->allResultBoxes.count("ydjz") != 0) {
        for (const auto &jz: this->allResultBoxes["ydjz"]) {
            flag = intersect(person, jz);
//            SPDLOG_INFO("intersect person.....{}", flag);
            if (flag) {
                flag = false;
                break;
            } else {
                flag = true;
            }
        }
    }
    return flag;
}

bool CameraHandle::UpdateAlgorithm_list(set<string> algorithm_list) {
    this->algorithm_list = algorithm_list;
    initTimeEvent();
    return true;
}

bool CameraHandle::CheckWithPerson(PredictionResult predictionResult, float rate) {
    bool flag = false;
    if (this->allResultBoxes.count("person") != 0) {
        for (const auto &person: this->allResultBoxes["person"]) {
            flag = intersect(predictionResult, person);
            if (rate != 0 && flag) {
//                cv::Mat tempFrame = cv::imread("C:\\Users\\uma-pc001\\Desktop\\test.png");
//                this->yolo->drawRectangle(tempFrame, predictionResult.eventInfo.left,
//                                          predictionResult.eventInfo.top,
//                                          predictionResult.eventInfo.right, predictionResult.eventInfo.bottom,
//                                          predictionResult.event);
//                this->yolo->drawRectangle(tempFrame, person.eventInfo.left,
//                                          person.eventInfo.top,
//                                          person.eventInfo.right, person.eventInfo.bottom,
//                                          person.event);
//                cv::imshow("testwindow", tempFrame);
//                cv::waitKey(1);
                float r = overlapRate(predictionResult, person);
//                SPDLOG_INFO("{} rate with person is :{:03.2f} rate is:{:03.2f} hold: {:03.2f}", predictionResult.event,
//                            r, rate,
//                            predictionResult.hold);
                if (r > rate) {
                    return true;
                } else {
                    flag = false;
                }
            }
            if (flag) {
                break;
            }
        }
    }
    return flag;
}

bool CameraHandle::CheckWithWrft(PredictionResult wrft) {
    return CheckWithPerson(wrft);
}

bool CameraHandle::CheckWithHead(PredictionResult head) {
    bool flag = false;
    flag = CheckWithPerson(head, 0.03);
    if (!flag) {
        return false;
    }
    if (this->allResultBoxes.count("helmet") != 0) {
        for (const auto &helmet: this->allResultBoxes["helmet"]) {
            flag = intersect(head, helmet);
            if (flag) {
                float r = overlapRate(head, helmet);
                SPDLOG_INFO("{} rate with helmet is :{:03.2f} rate is:{:03.2f} hold: {:03.2f}", head.event,
                            r, 0.5,
                            head.hold);
                if (r > 0.5) {
                    return false;
                }
            }
        }
    }
    return true;
}


void CameraHandle::handleTimeEvent(cv::Mat frame) {
    map<string, long long>::iterator timeEventIterator;
    for (timeEventIterator = timeEvents.begin(); timeEventIterator != timeEvents.end(); timeEventIterator++) {
        if (gettimeofday_ms() - timeEventIterator->second > this->appConfig->time_interval) {
            auto time = gettimeofday_ms();
            cv::imwrite(fmt::format("{}\\{}\\{}_{}_{}.jpg", this->appConfig->static_path.c_str(),
                                    eventCenter->getDayStr(), this->id.c_str(),
                                    timeEventIterator->first, time), frame);
            eventCenter->sendEventMsg(timeEventIterator->first,
                                      fmt::format("{}/{}_{}_{}.jpg", eventCenter->getDayStr(),
                                                  this->id.c_str(),
                                                  timeEventIterator->first,
                                                  time), "", this->id.c_str());
            timeEventIterator->second = gettimeofday_ms();;
            SPDLOG_INFO("[{}] Send timeEvent {}", this->id.c_str(), timeEventIterator->first);
        }
    }
}


void CameraHandle::Handle(cv::Mat frame) {
    if (frame.empty()) {
        return;
    }
    bool flag = false;
    flag = this->frameCount++ % this->appConfig->frame_interval == 0;
    if (hv::startswith(this->id, "offline")) {
        flag = this->frameCount++ % 3 == 0;
    }
    if (flag) {
        //临时违章的容器
        //预测结果的容器
        //清空原先的预测shuju
        this->allResultBoxes.clear();
        this->predictionsBoxes.clear();
        PDRV tempBoxs = this->yolo->prediction_my(frame, this->algorithm_list);
        if (tempBoxs.size() > 0) {
            for (PredictionResult tempPredictionResult: tempBoxs) {
                if (this->allResultBoxes.count(tempPredictionResult.event) != 0) {
                    this->allResultBoxes[tempPredictionResult.event].push_back(tempPredictionResult);
                } else {
                    PDRV temp;
                    temp.push_back(tempPredictionResult);
                    this->allResultBoxes[tempPredictionResult.event] = temp;
                }
            }
            string label = "helmet,gz,onlyjyst,face,ydjz,safebelt,badsafebelt,pifu";
            map<string, PDRV>::iterator eventIterator;
            for (eventIterator = this->allResultBoxes.begin();
                 eventIterator != this->allResultBoxes.end(); eventIterator++) {
                if (hv::contains(label, eventIterator->first)) {
                    continue;
                }
                for (PredictionResult tempPredictionResult: eventIterator->second) {
                    flag = true;
                    if (tempPredictionResult.event == "head") {
                        flag = CheckWithHead(tempPredictionResult);
                    }
                    //判断吸烟
                    if (tempPredictionResult.event == "smoke") {
                        flag = CheckSmoke(tempPredictionResult);
                    }
                    //判定未戴绝缘手套
                    if (tempPredictionResult.event == "wdjyst") {
                        flag = CheckWdJyst(tempPredictionResult);
                    }
                    if (tempPredictionResult.event == "wrft") {
                        flag = CheckWithWrft(tempPredictionResult);
                    }
                    //判定人员倒地
                    if (tempPredictionResult.event == "rydd") {
                        flag = CheckRydd(tempPredictionResult);
                    }
                    if (tempPredictionResult.event == "gzfzr" || tempPredictionResult.event == "zzfzr") {
                        flag = CheckWithPerson(tempPredictionResult, 0.15);
                    }
                    //判定未穿工作服
                    if (tempPredictionResult.event == "wcgzf") {
                        flag = CheckWcgzf(tempPredictionResult);
                    }
                    //判断是否未系安全带
                    if (tempPredictionResult.event == "offground") {
                        flag = CheckWdAqd(tempPredictionResult);
                        if (flag) {
                            tempPredictionResult.event = "wjaqd";
                            tempPredictionResult.eventInfo.label = "wjaqd";
                        }
                    }
                    //判断驾照是否带
                    if (tempPredictionResult.event == "person" && algorithm_list.count("jzsb") != 0) {
                        flag = CheckWdJz(tempPredictionResult);
                        if (flag) {
                            tempPredictionResult.event = "wdjz";
                            tempPredictionResult.eventInfo.label = "wdjz";
                        }
                    }
                    //判断大型吊臂站人
                    if (tempPredictionResult.event == "dxdb") {
                        flag = CheckDbxzr(tempPredictionResult);
                        if (flag) {
                            tempPredictionResult.event = "dbxzr";
                            tempPredictionResult.eventInfo.label = "dbxzr";
                        }
                    }
                    if (tempPredictionResult.event == "person") {
                        flag = false;
                    }
                    if (flag) {
                        this->predictionsBoxes.push_back(tempPredictionResult);
                    }
                }
            }
        }
        auto face_frame = frame.clone();
        this->taskPool->push_task([this, face_frame]() {
            map<string, PDRV> events;
            this->faceBoxes = this->facetool->Run(face_frame);
            if (this->faceBoxes.size() > 0) {
                this->eventCenter->HandleEvent(events, faceBoxes, this->id, gettimeofday_ms(), face_frame);
            }
        });
        this->frameCount = (this->appConfig->frame_interval - 1) * -1;

        //处理定时任务
        if (timeEvents.size() > 0) {
            handleTimeEvent(frame.clone());
        }
        map<string, PDRV> events;
        for (PredictionResult result: this->predictionsBoxes) {
            if (timeEvents.count(result.event.c_str()) != 0) {
                timeEvents[result.event.c_str()] = gettimeofday_ms();
            } else {
                events[result.event].push_back(result);
            }
        }
        if (events.size() > 0) {
            this->eventCenter->HandleEvent(events, faceBoxes, this->id, gettimeofday_ms(), frame.clone());
        }
    }
    for (auto predictionResult: this->predictionsBoxes) {
        if (this->notRender.count(predictionResult.event) == 1) {
            continue;
        }
        string showLabel = predictionResult.eventInfo.label;
        if (eventChecks.count(showLabel) == 0) {
            eventChecks.insert({showLabel, {showLabel, 1, gettimeofday_ms()}});
        } else {
            long long timeX = gettimeofday_ms() - eventChecks[showLabel].lasttime;
            eventChecks[showLabel].lasttime = gettimeofday_ms();
            if (timeX > 1000) {
                eventChecks[showLabel].count = 1;
                continue;
            }
            if (eventChecks[showLabel].count == 3) {
                eventChecks[showLabel].count--;
            } else {
                eventChecks[showLabel].count++;
            }
        }

        if (this->appConfig->mappers.count(showLabel) != 0) {
            showLabel = this->appConfig->mappers[showLabel];
        }
        if (this->showhold == 1) {
            showLabel = fmt::format("{0}_{1:.2f}", showLabel,
                                    predictionResult.eventInfo.hold);
        }

        this->yolo->drawRectangle(frame, predictionResult.eventInfo.left,
                                  predictionResult.eventInfo.top,
                                  predictionResult.eventInfo.right, predictionResult.eventInfo.bottom,
                                  showLabel);
    }
    this->eventCenter->pushQueue(frame.clone(), this->id);
    if (this->renderFlag) {
        for (auto face: this->faceBoxes) {
            this->yolo->drawRectangleFace(frame, face.left, face.top, face.right, face.bottom, face.name);
        }
        this->cameraPush->pusher(frame, this->pushIdx++);
    }
//    for (auto predictionResult: this->predictionsBoxes) {
//        if (this->notRender.count(predictionResult.event) == 1) {
//            continue;
//        }
//        string showLabel = predictionResult.eventInfo.label;
//        if (this->appConfig->mappers.count(showLabel) != 0) {
//            showLabel = this->appConfig->mappers[showLabel];
//        }
//        if (this->showhold == 1) {
//            showLabel = fmt::format("{0}_{1:.2f}", showLabel,
//                                    predictionResult.eventInfo.hold);
//        }
//        this->yolo->drawRectangle(frame, predictionResult.eventInfo.left,
//                                  predictionResult.eventInfo.top,
//                                  predictionResult.eventInfo.right, predictionResult.eventInfo.bottom,
//                                  showLabel);
//    }
//    video.push(frame.clone());
}

void CameraHandle::startPrediction() {
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
    this->predictionFlag = true;
    thread handle([this] {
        cv::waitKey(100);
        SPDLOG_INFO("[{}] Handle start ", this->id);
        while (this->predictionFlag) {
            if (this->cameraPull->flag) {
                this->Handle(this->cameraPull->get());
            }
            cv::waitKey(1);
        }
        SPDLOG_INFO("[{}] Handle end ", this->id);
    });
    handle.detach();
    SPDLOG_INFO("[{}] start prediction ", this->id);
    this->cameraPull->start();
    this->predictionFlag = false;
    cv::waitKey(20000);
    if (hv::startswith(this->id, "offline")) {
        this->EndVideo();
    }
    this->taskPool->wait_for_tasks();
    SPDLOG_INFO("[{}] end prediction ", this->id);
    cv::destroyAllWindows();
    //注释。。
//    cv::VideoWriter outputVideo;
//    string videoPath = fmt::format("video\\{}_render.mp4",this->id);
//    outputVideo.open(videoPath, CV_FOURCC('a', 'v', 'c', '1'), 25,
//                     cv::Size(1280, 720));
//    while (!video.empty()) {
//        cv::Mat frame;
//        frame = video.front();
//        video.pop();
//        outputVideo.write(frame.clone());
//    }
//    outputVideo.release();
}


bool CameraHandle::stopPrediction() {
    this->cameraPull->stop();
    this->predictionFlag = false;
    return true;
}

void CameraHandle::EndVideo() {
    char body[1024];
    datetime_t now = datetime_now();
    sprintf(body, "{\n"
                  "    \"msgType\": 203,\n"
                  "    \"data\": {\n"
                  "        \"CAMERA_CODE\": \"%s\""
                  "    }\n"
                  "}", this->id.c_str());
    SPDLOG_INFO("[{}] end video event body:{}", this->id, body);
    http_headers headers;
    headers.insert({"Content-Type", "application/json"});
    auto resp = requests::post(this->appConfig->api_host.c_str(), body, headers);
    if (resp == nullptr) {
        SPDLOG_ERROR("[{}] end video request failed!", this->id);
    } else {
        SPDLOG_INFO("[{}] end video request body{}", this->id, resp->body.c_str());
    }
}

bool CameraHandle::startRender() {
    SPDLOG_INFO("[{}] start render ....", this->id);
    if (this->cameraPull->flag) {
        SPDLOG_INFO("[{}] init ffmpeg tool", this->id);
        delete this->cameraPush;
        this->cameraPush = new CameraPush(1280, 720, 25,
                                          this->pushUrl.c_str(),
                                          this->id.c_str());
        this->renderFlag = this->cameraPush->Init();
        SPDLOG_INFO("[{}] start render success", this->id);
    } else {
        SPDLOG_INFO("[{}] start render failed", this->id);
    }
    return this->renderFlag;
}

bool CameraHandle::stopRender() {
    this->renderFlag = false;
    return true;
}
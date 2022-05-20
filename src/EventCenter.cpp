//
// Created by UMA-PC-001 on 2022/2/17.
//

#include "EventCenter.h"

EventCenter::~EventCenter() {

}

EventCenter::EventCenter(int showhold, YoloV5 *yolo, string savepath, string static_host, string api_host,
                         map<string, string> mappers, int interval) {
    this->showhold = showhold;
    this->static_host = static_host;
    this->api_host = api_host;
    this->savepath = savepath;
    this->yolo = yolo;
    this->mappers = mappers;
    this->interval = interval;
    thread messageBus([this] {
        this->messageBus();
    });
    messageBus.detach();
//    thread eventMsgHandle([this] {
//        this->eventMsgHandle();
//    });
//    eventMsgHandle.detach();
//    eventMsgHandle.detach();
//
//    thread faceMsgHandle([this] {
//        this->faceMsgHandle();
//    });
//    faceMsgHandle.detach();

}

void EventCenter::removeCamera(string cameraid) {
    cameraEvent.erase(cameraid);
}

void EventCenter::HandleEvent(map<string, PDRV> events, FSRV faceBoxes, string cameraid, long long bad_evet_time,
                              cv::Mat frame) {
    m_mutex.lock();
    tempQueue.push(Event{cameraid, frame, events, faceBoxes, bad_evet_time});
    m_mutex.unlock();
}

string EventCenter::getDayStr() {
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

void EventCenter::messageBus() {
    while (1) {
        Event event;
        m_mutex.lock();
        int size = tempQueue.size();
        if (size > 100) {
            SPDLOG_INFO("task count:{}", size);
        }
        if (size > 0) {
            event = tempQueue.front();
            tempQueue.pop();
        }
        m_mutex.unlock();
        DealEvent(event);
    }
}

void EventCenter::faceMsgHandle(WaitSendFaceRecord waitSendFaceRecord) {
    SPDLOG_INFO("[{}] send face msg", waitSendFaceRecord.cameraid);
    try {
        for (FaceStruct face: waitSendFaceRecord.faceBoxes) {
            cv::Mat frame = waitSendFaceRecord.frame.clone();
            int left = face.left;
            int top = face.top;
            if (left > 20 && top > 20) {
                left = left - 20;
                top = top - 20;
            }
            cv::Rect rect(left, top, face.right - face.left + 40, face.bottom - face.top + 40);
            cv::Mat capture_img = frame(rect);
            long long time = gettimeofday_ms();
            cv::imwrite(fmt::format("{}\\{}\\{}_{}_face.jpg", savepath, getDayStr(), waitSendFaceRecord.cameraid,
                                    time), capture_img);
            this->sendFaceMsg(face.name, fmt::format("{}/{}_{}_face.jpg", getDayStr(),
                                                     waitSendFaceRecord.cameraid,
                                                     time), waitSendFaceRecord.cameraid);
        }
    } catch (cv::Exception &e) {
        SPDLOG_ERROR("[{}] opencv :{}", waitSendFaceRecord.cameraid, e.what());
    } catch (const std::exception &e) {
        SPDLOG_ERROR("[{}] execption :{}", waitSendFaceRecord.cameraid, e.what());
    }
    SPDLOG_INFO("[{}]send face msg end", waitSendFaceRecord.cameraid);
}

void EventCenter::sendFaceMsg(string name, string img, string cameraid) {
    try {
        char body[1024];
        datetime_t now = datetime_now();
        sprintf(body, "{\n"
                      "    \"msgType\": 302,\n"
                      "    \"data\": {\n"
                      "        \"FACE_ID\": \"%s\",\n"
                      "        \"CAMERA_ID\": \"%s\",\n"
                      "        \"FIND_TIME\": \"%d-%d-%d %d:%d:%d\",\n"
                      "        \"USER_PIC\": \"%s%s\",\n"
                      "    }\n"
                      "}", hv::split(name, '_')[0].c_str(), cameraid.c_str(), now.year, now.month, now.day, now.hour,
                now.min,
                now.sec, static_host.c_str(), img.c_str());
        SPDLOG_INFO("[{}] face send body:{}", body, cameraid);
        http_headers headers;
        headers.insert({"Content-Type", "application/json"});
        auto resp = requests::post(this->api_host.c_str(), body, headers);
        if (resp == nullptr) {
            SPDLOG_ERROR("[{}] face request failed!", cameraid);
        } else {
            SPDLOG_INFO("[{}] face request body{}", cameraid, resp->body.c_str());
        }
    } catch (cv::Exception &e) {
        SPDLOG_ERROR("[{}] opencv :{}", cameraid, e.what());
    } catch (const std::exception &e) {
        SPDLOG_ERROR("[{}] execption :{}", cameraid, e.what());
    }
}

void EventCenter::eventMsgHandle(WaitSendEventRecord waitSendRecord, queue<cv::Mat> frames) {
    SPDLOG_INFO("[{}] event:{} send event msg", waitSendRecord.cameraid, waitSendRecord.event);
    int i = 0;
    while (1) {
        try {
            if (cameraEvent.count(waitSendRecord.cameraid) == 0) {
                break;
            }
            m_mutex2.lock();
            Queue after = cameraVideo[waitSendRecord.cameraid].video2[waitSendRecord.afterid];
            m_mutex2.unlock();
            if (after.size() > 25 * 3 || i > 250) {
                long long time = gettimeofday_ms();
                cv::imwrite(fmt::format("{}\\{}\\{}_{}_{}_raw.jpg", savepath, getDayStr(), waitSendRecord.cameraid,
                                        waitSendRecord.event,
                                        time), waitSendRecord.frame);
                for (PredictionResult drawInfo: waitSendRecord.events) {
                    string showLabel = drawInfo.eventInfo.label;
                    if (this->mappers.count(showLabel) != 0) {
                        showLabel = this->mappers[showLabel];
                    }
                    if (this->showhold == 1) {
                        showLabel = fmt::format("{0}_{1:.2f}", showLabel,
                                                drawInfo.eventInfo.hold);
                    }
                    this->yolo->drawRectangle(waitSendRecord.frame, drawInfo.eventInfo.left,
                                              drawInfo.eventInfo.top,
                                              drawInfo.eventInfo.right, drawInfo.eventInfo.bottom,
                                              showLabel);
                }
                string drawImgPath = fmt::format("{}\\{}\\{}_{}_{}.jpg", savepath, getDayStr(),
                                                 waitSendRecord.cameraid,
                                                 waitSendRecord.event,
                                                 time);
                cv::imwrite(drawImgPath, waitSendRecord.frame);
                cv::VideoWriter outputVideo;
                string videoPath = fmt::format("{}\\{}\\{}_{}_{}.mp4", savepath, getDayStr(),
                                               waitSendRecord.cameraid,
                                               waitSendRecord.event,
                                               time);
                outputVideo.open(videoPath, CV_FOURCC('a', 'v', 'c', '1'), 25,
                                 cv::Size(1280, 720));
                while (!frames.empty()) {
                    cv::Mat frame;
                    frame = frames.front();
                    frames.pop();
                    outputVideo.write(frame.clone());
                }
                while (!after.empty()) {
                    cv::Mat frame;
                    frame = after.front();
                    after.pop();
                    outputVideo.write(frame.clone());
                }
                outputVideo.release();
                sendEventMsg(waitSendRecord.event, fmt::format("{}/{}_{}_{}.jpg", getDayStr(),
                                                               waitSendRecord.cameraid,
                                                               waitSendRecord.event,
                                                               time), fmt::format("{}/{}_{}_{}.mp4", getDayStr(),
                                                                                  waitSendRecord.cameraid,
                                                                                  waitSendRecord.event,
                                                                                  time), waitSendRecord.cameraid);
                SPDLOG_INFO("[{}] event:{} send event msg end", waitSendRecord.cameraid, waitSendRecord.event);
                m_mutex2.lock();
                cameraVideo[waitSendRecord.cameraid].video2.erase(waitSendRecord.afterid);
                m_mutex2.unlock();
                break;
            }
            cv::waitKey(1);
        } catch (cv::Exception &e) {
            SPDLOG_ERROR("[{}] opencv :{}", waitSendRecord.cameraid, e.what());
            break;
        } catch (const std::exception &e) {
            SPDLOG_ERROR("[{}] execption :{}", waitSendRecord.cameraid, e.what());
            break;
        }
        i++;
    }
}

void EventCenter::sendEventMsg(string event, string img, string video, string cameraid) {
    try {
        string label = "ydjz,safebelt";
        if (hv::contains(label, event)) {
            return;
        }
        char body[1024];
        datetime_t now = datetime_now();
        sprintf(body, "{\n"
                      "    \"msgType\": 301,\n"
                      "    \"data\": {\n"
                      "        \"ALGORITHM_ID\": \"%s\",\n"
                      "        \"CAMERA_ID\": \"%s\",\n"
                      "        \"VIOLATION_TIME\": \"%d-%d-%d %d:%d:%d\",\n"
                      "        \"VIOLATION_PIC\": \"%s%s\",\n"
                      "        \"VIOLATION_VIDEO\": \"%s%s\"\n"
                      "    }\n"
                      "}", event.c_str(), cameraid.c_str(), now.year, now.month, now.day, now.hour, now.min,
                now.sec, this->static_host.c_str(), img.c_str(), static_host.c_str(), video.c_str());
        SPDLOG_INFO("[{}] event send body:{}", cameraid, body);
        http_headers headers;
        headers.insert({"Content-Type", "application/json"});
        auto resp = requests::post(this->api_host.c_str(), body, headers);
        if (resp == nullptr) {
            SPDLOG_ERROR("[{}] event request failed! body {}", cameraid, body);
        } else {
            SPDLOG_INFO("[{}] event request body{} ", cameraid, resp->body.c_str());
        }
    } catch (cv::Exception &e) {
        SPDLOG_ERROR("[{}] opencv :{}", cameraid, e.what());
    } catch (const std::exception &e) {
        SPDLOG_ERROR("[{}] execption :{}", cameraid, e.what());
    }
}


void EventCenter::pushQueue(cv::Mat frame, string cameraid) {
    try {
        if (frame.empty()) {
            return;
        }
        cv::Mat temp = frame.clone();
        if (m_mutex2.try_lock()) {
            cameraVideo[cameraid].video1.push(temp.clone());
            if (cameraVideo[cameraid].video1.size() > 25 * 3) {
                cameraVideo[cameraid].video1.pop();
            }
            map<long long, Queue>::iterator videoIterator;
            for (videoIterator = cameraVideo[cameraid].video2.begin();
                 videoIterator != cameraVideo[cameraid].video2.end(); videoIterator++) {
                if (videoIterator->second.size() < 25 * 3 + 1) {
                    videoIterator->second.push(temp.clone());
                }
            }
            m_mutex2.unlock();
            temp.release();
        }

    } catch (cv::Exception &e) {
        SPDLOG_ERROR("[{}] opencv :{}", cameraid, e.what());
    } catch (const std::exception &e) {
        SPDLOG_ERROR("[{}] execption :{}", cameraid, e.what());
    }
}

void EventCenter::DealEvent(Event event) {
    if (event.cameraid == "" || event.frame.empty()) {
        return;
    }
    try {
//    SPDLOG_INFO("cache size:{} event size:{}", cameraEvent.size(), event.eventInfo.size());
        m_mutex2.lock();
        queue<cv::Mat> before(this->cameraVideo[event.cameraid].video1);
        m_mutex2.unlock();
        for (FaceStruct face: event.faceInfo) {
            if (cameraEvent[event.cameraid].faces.count(face.name) == 0) {
                WaitSendFaceRecord waitSendFaceRecord;
                waitSendFaceRecord.cameraid = event.cameraid;
                waitSendFaceRecord.frame = event.frame.clone();
                waitSendFaceRecord.faceBoxes = event.faceInfo;
                cameraEvent[event.cameraid].faces[face.name] = face;
                faceMsgHandle(waitSendFaceRecord);
            }
        }
        if (cameraEvent.count(event.cameraid) != 0) {
            //首先循环发送过来的事件
            map<string, PDRV>::iterator eventIterator;
            for (eventIterator = event.events.begin(); eventIterator != event.events.end(); eventIterator++) {
                //判断5分钟内有没有重复发送
                if (SendRecords.count(event.cameraid) != 0) {
                    if (SendRecords[event.cameraid].eventsRecord.count(eventIterator->first) != 0) {
                        if (gettimeofday_ms() - SendRecords[event.cameraid].eventsRecord[eventIterator->first] <
                            this->interval) {
                            continue;
                        }
                    }
                }
                //判断该类型事件有没有发生过
                if (cameraEvent[event.cameraid].events.count(eventIterator->first) != 0) {
                    long long timeX =
                            event.lasttime - cameraEvent[event.cameraid].events[eventIterator->first].lasttime;
                    //如果大于1秒直接丢弃该事件
                    if (timeX > 1000) {
                        SPDLOG_INFO("[{}] event:{} The time difference is too large drop frame {}", event.cameraid,
                                    eventIterator->first, timeX);
                        cameraEvent[event.cameraid].events.erase(eventIterator->first);
                    } else {
                        //如果间隔小于1秒，则更新最后违章事件发生时间
                        cameraEvent[event.cameraid].events[eventIterator->first].lasttime = event.lasttime;
                        //判断是不是连续发生大于2次
                        if (cameraEvent[event.cameraid].events[eventIterator->first].count > 3) {
                            //满足条件了，准备发送数据
                            SPDLOG_INFO("[{}] event:{} waitting send msg ...", event.cameraid, eventIterator->first);
                            //构建一个待发送数据
                            WaitSendEventRecord waitSendRecord;
//                            waitSendRecord.before = before;
                            waitSendRecord.cameraid = event.cameraid;
                            waitSendRecord.frame = event.frame.clone();
                            waitSendRecord.event = eventIterator->first;
                            waitSendRecord.events = eventIterator->second;
                            waitSendRecord.afterid = gettimeofday_ms();
                            m_mutex2.lock();
                            this->cameraVideo[event.cameraid].video2[waitSendRecord.afterid].push(event.frame);
                            m_mutex2.unlock();
                            //标志一下该事件已发送
                            SendRecords[event.cameraid].eventsRecord[eventIterator->first] = gettimeofday_ms();
                            cameraEvent[event.cameraid].events[eventIterator->first].count = 0;
                            cameraEvent[event.cameraid].events.erase(eventIterator->first);
                            eventMsgHandle(waitSendRecord, before);
//                        cameraEvent
                        } else {
                            //添加一次发生次数
                            cameraEvent[event.cameraid].events[eventIterator->first].count++;
                        }
                    }
                } else {
                    //给未出现的违章添加至容器
//                SPDLOG_INFO("add event");
                    MsgEvent msgEvent;
                    msgEvent.count = 1;
                    msgEvent.lasttime = event.lasttime;
                    cameraEvent[event.cameraid].events.insert({eventIterator->first, msgEvent});
                }
            }
        } else {
            SPDLOG_INFO("first init camera handle...");
            ResultCache camera;
            camera.cameraid = event.cameraid;
            map<string, PDRV>::iterator eventIterator;
            for (eventIterator = event.events.begin(); eventIterator != event.events.end(); eventIterator++) {
                MsgEvent msgEvent;
                msgEvent.count = 1;
                msgEvent.lasttime = event.lasttime;
                camera.events.insert({eventIterator->first, msgEvent});
            }
            cameraEvent.insert({event.cameraid, camera});
        }
        cameraEvent[event.cameraid].frame = event.frame.clone();
    } catch (cv::Exception &e) {
        SPDLOG_ERROR("[{}] opencv :{}", event.cameraid, e.what());
    } catch (const std::exception &e) {
        SPDLOG_ERROR("[{}] execption :{}", event.cameraid, e.what());
    }
}
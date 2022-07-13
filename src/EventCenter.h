//
// Created by UMA-PC-001 on 2022/2/17.
//

#ifndef WATCHAI_EVENTCENTER_H
#define WATCHAI_EVENTCENTER_H

#include "watchai.h"
#include "YoloV5.h"

struct Event {
    string cameraid;
    cv::Mat frame;
    map<string, PDRV> events;
    FSRV faceInfo;
    long long lasttime;
};
struct MsgEvent {
    long long lasttime;
    int count;
};
struct WaitSendEventRecord {
    string cameraid;
    string event;
    PDRV events;
    cv::Mat frame;
    Queue before;
    long long afterid;
};
struct WaitSendFaceRecord {
    string cameraid;
    cv::Mat frame;
    FSRV faceBoxes;
};
struct SendRecord {
    map<string, long long> eventsRecord;
};
struct ResultCache {
    string cameraid;
    cv::Mat frame;
    map<string, MsgEvent> events;
    map<string, FaceResult> faces;
};
struct CameraVideo {
    Queue video1;
    map<long long, Queue> video2;
};

class EventCenter {
public:
    ~EventCenter();

    EventCenter(int showhold, YoloV5 *yolo, string path, string static_host, string api_host,
                map<string, string> mappers, int interval);

    void HandleEvent(map<string, PDRV> events, FSRV faceBoxes, string cameraid, long long bad_evet_time, cv::Mat frame);

    void pushQueue(cv::Mat frame, string cameraid);

    void removeCamera(string cameraid);

    string getDayStr();

    void sendEventMsg(string event, string img, string video, string cameraid);

private:
    void messageBus();

    void DealEvent(Event event);

    void eventMsgHandle(WaitSendEventRecord waitSendRecord, queue<cv::Mat> frames);


    void faceMsgHandle(WaitSendFaceRecord waitSendFaceRecord);

    void sendFaceMsg(string uuid, string img, string cameraid);


private:
    int showhold;//控制是否显示系数
    string savepath;
    string static_host;
    string api_host;
    YoloV5 *yolo;
    map<string, string> mappers;
    map<string, ResultCache> cameraEvent;
    map<string, CameraVideo> cameraVideo;
    map<string, SendRecord> SendRecords;
//    std::queue<WaitSendEventRecord> waitSendEventRecords;
//    std::queue<WaitSendFaceRecord> waitSendFaceRecords;
    std::queue<Event> tempQueue;
    mutex m_mutex;
    mutex m_mutex2;
    int interval;
};


#endif //WATCHAI_EVENTCENTER_H

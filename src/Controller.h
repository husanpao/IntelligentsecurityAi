//
// Created by UMA-PC-001 on 2022/2/21.
//

#ifndef WATCHAI_CONTROLLER_H
#define WATCHAI_CONTROLLER_H

#include "watchai.h"
#include "CameraHandle.h"
#include "YoloV5.h"
#include "ArcFace.h"

struct Player {
    string cameraid;
    bool render;
    string liveUrl;
    CameraHandle *cameraHandle;
};

class Controller {
public:
    ~Controller();

    Controller(AppConfig *appconfig);

    string MAVideo(rapidjson::Value &data);

    string SubVideo(rapidjson::Value &data);

    string AddFace(rapidjson::Value &data);

    string LookPic(rapidjson::Value &data);
    
    string SnapPic(rapidjson::Value &data);

private:
    void initLables();

public:
    std::map<string, Player *> players;
    AppConfig *appconfig;
    YoloV5 *yolov5;
    ArcFace *facetool;
    EventCenter *eventCenter;
};


#endif //WATCHAI_CONTROLLER_H

//
// Created by UMA-PC-001 on 2022/2/21.
//

#include "Controller.h"
#include "PictureHandle.h"
#include "SnapHandle.h"

Controller::~Controller() {}

Controller::Controller(AppConfig *appconfig) {
    this->appconfig = appconfig;
    initLables();
    //人脸识别SDK初始化
    SeetaFace *face = new SeetaFace("./seeta", appconfig->facehold);
    face->Load();
    this->seetaFace = face;
    bool isCuda = torch::cuda::is_available();
    this->yolov5 = new YoloV5(this->appconfig->weight, isCuda);
    this->yolov5->prediction(torch::rand({1, 3, 640, 640}));
    this->eventCenter = new EventCenter(this->appconfig->showhold, yolov5, this->appconfig->static_path,
                                        this->appconfig->static_host, this->appconfig->api_host,
                                        this->appconfig->mappers, this->appconfig->interval);
}

void Controller::initLables() {
    rapidjson::Document dom;
    if (!dom.Parse(this->appconfig->lables.c_str()).HasParseError()) {
        if (dom.IsArray()) {
            rapidjson::Value &labels = dom.GetArray();
            size_t len = labels.Size();
            for (size_t i = 0; i < len; i++) {
                const rapidjson::Value &label = labels[i];
                if (label.IsObject()) {
                    if (label.HasMember("key") && label.HasMember("value")) {
                        string key = label["key"].GetString();
                        string value = label["value"].GetString();
                        this->appconfig->mappers.insert({key, value});
                    }
                }
            }
        }
    }
}

string Controller::AddFace(rapidjson::Value &data) {
    string result = "";
    if (data.HasMember("USERID") && data.HasMember("NAME") && data.HasMember("FACEIMAGE") &&
        data.HasMember("STATE")) {
        string userid = data["USERID"].GetString();
        string name = data["NAME"].GetString();
        string faceimage = data["FACEIMAGE"].GetString();
        int state = data["STATE"].GetInt();
        string nname = WStringToString(Utf8ToUnicode(name));
        faceimage = WStringToString(Utf8ToUnicode(faceimage));
        string fullpath = hv::replace(this->appconfig->api_host, "/api", "");
        string temp_img = fmt::format("./faces/{}.jpg", userid);
        int ret = httpDownload(fullpath.append(faceimage).c_str(),
                               temp_img.c_str());
        result = (ret == 0 ? "" : "Add Face Feature Error");
        if (result == "") {
            this->seetaFace->Register(temp_img, nname, userid);
        }
    }
    rapidjson::StringBuffer buf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
    writer.StartObject();
    writer.Key("code");
    writer.Int(200);
    writer.Key("message");
    writer.String("success");
    writer.Key("data");
    writer.String(result.c_str());
    writer.EndObject();
    return buf.GetString();
}

string Controller::MAVideo(rapidjson::Value &data) {
    vector<string> add;
    vector<string> remove;
    if (data.HasMember("CAMERA_ADD") && data["CAMERA_ADD"].IsObject()) {
        rapidjson::Value &camera_add = data["CAMERA_ADD"];
        if (camera_add.IsObject()) {
            if (camera_add.HasMember("CAMERA_CODE") && camera_add.HasMember("STREAM")) {
                string cameraid = camera_add["CAMERA_CODE"].GetString();
                string camera_url = camera_add["STREAM"].GetString();
                if (camera_url != "" && cameraid != "") {
                    set<string> algorithm_list;
                    if (data.HasMember("ALGORITHM_LIST") && data["ALGORITHM_LIST"].IsString()) {
                        string algorithmstr = data["ALGORITHM_LIST"].GetString();
                        hv::StringList temp = hv::split(algorithmstr, ',');
                        for (auto algorithm: temp) {
                            algorithm_list.insert(algorithm);
                        }
                    }
                    Player *player = nullptr;
                    //判断当前摄像机是否开启，如果开启则取缓存数据，否则new新对象
                    if (players.count(cameraid) == 1) {
                        player = players[cameraid];
                    } else {
                        if (players.size() > this->appconfig->maxstream) {
                            return "{\"code\":200,\"message\":\"The maximum value is reached\",\"data\":{\"ADDED_CAMERA\":[\"\"],\"REMOVED_CAMERA\":[],\"ONLINE_CAMERA\":[]}}";
                        }
                        player = new Player();
                        player->cameraid = cameraid;
                        players.insert({cameraid, player});
                    }
                    //判断模型是否在工作
                    if (!player->cameraHandle) {
                        SPDLOG_INFO("[ADD]cameraid:{},cameraurl:{}", cameraid, camera_url);
                        thread t([this, camera_url, player, algorithm_list]() {
                            //构建渲染后预览地址
                            string temp_live_server(this->appconfig->live_server);
                            player->liveUrl = temp_live_server.append(
                                    player->cameraid);
                            //构建预测模型控制器
                            CameraHandle *cameraHandle = new CameraHandle(this->appconfig, camera_url, player->cameraid,
                                                                          algorithm_list, this->yolov5,
                                                                          this->eventCenter);

                            cameraHandle->seetaFace = this->seetaFace;
                            player->cameraHandle = cameraHandle;
                            cameraHandle->startPrediction();
                            delete cameraHandle;
                            //停止预测后移除会话信息
                            players.erase(player->cameraid);
                            eventCenter->removeCamera(player->cameraid);
                        });
                        t.detach();
                    } else {
                        player->cameraHandle->UpdateAlgorithm_list(algorithm_list);
                        SPDLOG_INFO("[ADD]cameraid:{},cameraurl:{} is prediction!", cameraid, camera_url);
                    }
                    add.push_back(cameraid);
                }
            }
        }
    }
    if (data.HasMember("CAMERA_REMOVE")) {
        string camera_remove = data["CAMERA_REMOVE"].GetString();
        Player *player = nullptr;
        if (players.count(camera_remove) == 1) {
            player = players[camera_remove];
            if (player->cameraHandle != nullptr) {
                player->cameraHandle->stopPrediction();
            }
            SPDLOG_INFO("[REMOVE]cameraid:{}", camera_remove);
            remove.push_back(camera_remove);
        }
    }
    rapidjson::StringBuffer buf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
    writer.StartObject();
    writer.Key("code");
    writer.Int(200);
    writer.Key("message");
    writer.String("success");
    writer.Key("data");
    writer.StartObject();
    writer.Key("ADDED_CAMERA");
    writer.StartArray();
    for (string cameraid: add) {
        writer.String(cameraid.c_str());
    }
    writer.EndArray();
    writer.Key("REMOVED_CAMERA");
    writer.StartArray();
    for (string cameraid: remove) {
        writer.String(cameraid.c_str());
    }
    writer.EndArray();
    writer.Key("ONLINE_CAMERA");
    writer.StartArray();
    for (auto m = players.begin(); m != players.end(); ++m) {
        if (m->second->cameraHandle) {
            if (m->second->cameraHandle->predictionFlag) {
                writer.String(m->first.c_str());
            }
        }
    }
    writer.EndArray();
    writer.EndObject();
    writer.EndObject();
    string result = buf.GetString();
    return result;
}

string Controller::SnapPic(rapidjson::Value &data) {
    string result = "";
    if (data.IsObject()) {
        const rapidjson::Value &obj = data;
        if (obj.HasMember("STREAM") && obj.HasMember("CAMERA_CODE")) {
            set<string> algorithm_list = {"helmet"};
            string stream = obj["STREAM"].GetString();
            string camera_code = obj["CAMERA_CODE"].GetString();
            thread t([this, stream, camera_code, algorithm_list]() {
                SnapHandle *snapHandle = new SnapHandle(this->appconfig, this->yolov5, camera_code, stream,
                                                        algorithm_list);
                snapHandle->startPrediction();
                delete snapHandle;
                SPDLOG_INFO("[{}] delete snapHandle ", camera_code);
            });
            t.detach();
        }
    }
    rapidjson::StringBuffer buf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
    writer.StartObject();
    writer.Key("code");
    writer.Int(200);
    writer.Key("message");
    writer.String("success");
    writer.Key("data");
    writer.String(result.c_str());
    writer.EndObject();
    return buf.GetString();
}

string Controller::LookPic(rapidjson::Value &data) {
    string result = "";
    if (data.IsArray()) {
        rapidjson::Value &objs = data.GetArray();
        size_t len = objs.Size();
        map<string, string> cameraPics;
        for (size_t i = 0; i < len; i++) {
            const rapidjson::Value &obj = objs[i];
            if (obj.IsObject()) {
                if (obj.HasMember("STREAM") && obj.HasMember("CAMERA_CODE")) {
                    string stream = obj["STREAM"].GetString();
                    string camera_code = obj["CAMERA_CODE"].GetString();
                    cameraPics.insert({camera_code, stream});
                }
            }
        }
        set<string> algorithm_list = {"helmet"};
        thread t([this, cameraPics, algorithm_list]() {
            PictureHandle *pictureHandle = new PictureHandle(this->appconfig, this->yolov5, cameraPics, algorithm_list);
            pictureHandle->startPrediction();
        });
        t.detach();
    } else {
        result = "data is not array";
    }
    rapidjson::StringBuffer buf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
    writer.StartObject();
    writer.Key("code");
    writer.Int(200);
    writer.Key("message");
    writer.String("success");
    writer.Key("data");
    writer.String(result.c_str());
    writer.EndObject();
    return buf.GetString();
}

string Controller::SubVideo(rapidjson::Value &data) {
    string result = "";
    if (data.HasMember("CAMERA_CODE")) {
        string cameraid = data["CAMERA_CODE"].GetString();
        int event = data["EVENT_TYPE"].GetInt();
        if (players.count(cameraid) == 1) {
            Player *player = players[cameraid];
            if (player->cameraHandle != nullptr) {
                result = "";
                if (player->cameraHandle->predictionFlag && event == 1 && !player->render) {
                    player->render = player->cameraHandle->startRender();
                    if (player->render) {
                        result = player->liveUrl;
                    } else {
                        result = "can't subvideo";
                    }
                } else if (player->cameraHandle->predictionFlag && event == 0 && player->render) {
                    player->cameraHandle->stopRender();
                    player->render = false;
                } else {
                    result = player->liveUrl;
                }
            }
        } else {
            result = "camera no pull stream";
        }
    }
    rapidjson::StringBuffer buf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
    writer.StartObject();
    writer.Key("code");
    writer.Int(200);
    writer.Key("message");
    writer.String("success");
    writer.Key("data");
    writer.String(result.c_str());
    writer.EndObject();
    return buf.GetString();
}
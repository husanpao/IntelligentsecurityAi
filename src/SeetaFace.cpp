//
// Created by uma-pc001 on 2022/7/12.
//


#include "SeetaFace.h"
#include "enjoy/enjoy_all.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"

SeetaFace::SeetaFace(char *mp, float ts) {
    this->modelpath = mp;
    this->threshold = ts;
    this->fd = new_fd();
    this->fl = new_fl();
    this->fb = new_fb();
}

SeetaFace::~SeetaFace() {
    delete this->fd;
    delete this->fl;
    delete this->fb;
}

FaceRecognizer *SeetaFace::new_fr() {
    ModelSetting setting;
    setting.set_id(0);
    setting.set_device(SEETA_DEVICE_GPU);
    setting.append(modelpath + "/face_recognizer.csta");
    return new FaceRecognizer(setting);
}

FaceLandmarker *SeetaFace::new_fl() {
    ModelSetting setting;
    setting.set_id(0);
    setting.set_device(SEETA_DEVICE_GPU);
    setting.append(modelpath + "/face_landmarker_pts5.csta");
    return new FaceLandmarker(setting);
}

FaceDetector *SeetaFace::new_fd() {
    ModelSetting setting;
    setting.set_id(0);
    setting.set_device(SEETA_DEVICE_GPU);
    setting.append(modelpath + "/face_detector.csta");
    return new FaceDetector(setting);
}

FaceDatabase *SeetaFace::new_fb() {
    ModelSetting setting;
    setting.set_id(0);
    setting.set_device(SEETA_DEVICE_GPU);
    setting.append(modelpath + "/face_recognizer.csta");
    return new FaceDatabase(setting);
}

int64_t SeetaFace::extract_feature(Mat img) {
    SeetaImageData simg;
    simg.height = img.rows;
    simg.width = img.cols;
    simg.channels = img.channels();
    simg.data = img.data;
    auto faces = this->fd->detect(simg);
    if (faces.size <= 0) {
        SPDLOG_ERROR("no face detected");
        return false;
    }
    SeetaPointF points[5];
    this->fl->mark(simg, faces.data[0].pos, points);
    return this->fb->Register(simg, points);
}

bool SeetaFace::Register(Mat img, string name, string uuid) {
    if (img.empty()) {
        return false;
    }
    this->infos[extract_feature(img)] = {name, uuid};
    if (this->Save()) {
        SPDLOG_INFO("save face_feature success.");
    } else {
        SPDLOG_INFO("save face_feature error.");
    }
    SPDLOG_INFO("current face count:{}", this->fb->Count());
    return true;
}

bool SeetaFace::Register(string path, string name, string uuid) {
    Mat img = imread(path);
    return Register(img, name, uuid);
}

bool SeetaFace::Delete(char *name) {
    return false;
}

bool SeetaFace::Save() {
    this->fb->Save((this->modelpath + "/face_feature.dat").c_str());
    rapidjson::StringBuffer buf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
    writer.StartArray();
    for (auto o: this->infos) {
        writer.StartObject();
        writer.Key("id");
        writer.Int(o.first);
        writer.Key("name");
        writer.String(o.second.name.c_str());
        writer.Key("uuid");
        writer.String(o.second.uuid.c_str());
        writer.EndObject();
    }
    writer.EndArray();

    rapidjson::Document docu;
    docu.Parse(buf.GetString());
    FILE *fp = fopen((this->modelpath + "/face_n.dat").c_str(), "wb");
    char writeBuffer[65535];
    rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
    rapidjson::PrettyWriter<rapidjson::FileWriteStream> fw(os);
    docu.Accept(fw);
    fclose(fp);
    return true;
}

bool SeetaFace::Load() {
    this->fb->Load((this->modelpath + "/face_feature.dat").c_str());
    SPDLOG_INFO("load faceinfo count:{}", this->fb->Count());
    if (Enjoy::Kit::FileUtil::exist(this->modelpath + "/face_n.dat")) {
        FILE *fp = fopen((this->modelpath + "/face_n.dat").c_str(), "rb");
        char readBuffer[65536];
        rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
        rapidjson::Document doc;
        doc.ParseStream(is);
        fclose(fp);
        if (doc.IsArray()) {
            rapidjson::Value &objs = doc.GetArray();
            size_t len = objs.Size();
            for (size_t i = 0; i < len; i++) {
                const rapidjson::Value &obj = objs[i];
                if (obj.IsObject()) {
                    int64_t id = obj["id"].GetInt64();
                    string name = obj["name"].GetString();
                    string uuid = obj["uuid"].GetString();
                    this->infos[id] = {name, uuid};
                }
            }
        }
        SPDLOG_INFO("load facestruct count:{}", this->infos.size());
    } else {
        SPDLOG_WARN("can not find face_n.dat.");
    }
    return false;
}

vector<FaceResult> SeetaFace::Compare(Mat frame) {
    SeetaImageData image;
    image.height = frame.rows;
    image.width = frame.cols;
    image.channels = frame.channels();
    image.data = frame.data;
    auto faces = this->fd->detect(image);
    vector<FaceResult> rs;
    for (int i = 0; i < faces.size; i++) {
        //----人脸----
        auto face = faces.data[i].pos;
        //----关键点检测----
        vector<SeetaPointF> points(this->fl->number());
        this->fl->mark(image, face, points.data());
        float sim = 0;
        int64_t id = this->fb->Query(image, points.data(), &sim);
        if (sim > this->threshold) {
            rs.push_back({id, this->infos[id].name, this->infos[id].uuid, face, sim});
        }
    }
    return rs;
}



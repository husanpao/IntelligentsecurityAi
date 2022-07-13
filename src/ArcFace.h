//
// Created by UMA-PC-001 on 2022/2/17.
//

#ifndef WATCHAI_ARCFACE_H
#define WATCHAI_ARCFACE_H
#include "watchai.h"
#include "asface/amcomdef.h"
#include "asface/merror.h"
#include "asface/arcsoft_face_sdk.h"
class ArcFace {
public:
    ~ArcFace();

    ArcFace();

    ArcFace(char *activekey, double facehold);

    void Init();

    std::vector<FaceStruct> Run(cv::Mat img);

    void registerSingle(string filename);

private:
    void getSdkRelatedInfo();

    MInt32 faceNum = 0;

    bool Recognition();

    void timestampToTime(char *timeStamp, char *dateTime, int dateTimeSize);

    void registerFaceFeature();


private:
    bool status = false;
    bool initEnd = false;
    char *activekey;
    double facehold;
    MHandle engineHandle;
    list<ASF_FaceFeature> features;
    mutex m_mutex;
};


#endif //WATCHAI_ARCFACE_H

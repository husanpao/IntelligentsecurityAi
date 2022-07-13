//
// Created by uma-pc001 on 2022/7/12.
//

#ifndef SEETAFACE_DEMO_SEETAFACE_H
#define SEETAFACE_DEMO_SEETAFACE_H

#include "seeta/FaceRecognizer.h"
#include "seeta/FaceDetector.h"
#include "seeta/FaceLandmarker.h"
#include "seeta/FaceDatabase.h"
#include "watchai.h"
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace seeta;
using namespace std;
using namespace cv;

struct Person {
    string name;
    string uuid;
};

class SeetaFace {

public:
    SeetaFace(char *modelpath = ".", float threshold = 0.6);

    virtual ~SeetaFace();

    bool Register(Mat img, string name, string uuid);

    bool Register(string path, string name, string uuid);

    bool Delete(char *id);

    bool Save();

    bool Load();

    vector<FaceResult> Compare(Mat img);

private:
    FaceRecognizer *new_fr();

    FaceLandmarker *new_fl();

    FaceDetector *new_fd();

    FaceDatabase *new_fb();

    int64_t extract_feature(Mat img);

private:
    FaceDetector *fd;
    FaceLandmarker *fl;
    FaceDatabase *fb;
    string modelpath;
    map<int64_t, Person> infos;
    float threshold;
};


#endif //SEETAFACE_DEMO_SEETAFACE_H

//
// Created by UMA-PC-001 on 2022/2/17.
//

#include "ArcFace.h"

void ArcFace::Init() {
    initEnd = false;
    if (access("faces", F_OK) == -1) {
        mkdir("faces");
    }
    getSdkRelatedInfo();
    status = Recognition();
    if (status) {
        registerFaceFeature();
    }
    initEnd = true;
}

std::vector<FaceStruct> ArcFace::Run(cv::Mat img) {
    std::vector<FaceStruct> facelist;
    if (m_mutex.try_lock()) {
        if (!status || !initEnd) {
            m_mutex.unlock();
            return facelist;
        }
        ASF_MultiFaceInfo detectedFaces = {0};
        MRESULT res = ASFDetectFaces(engineHandle, img.cols, img.rows, ASVL_PAF_RGB24_B8G8R8,
                                     (MUInt8 *) img.ptr<uchar>(0),
                                     &detectedFaces);
        if (MOK == res) {
            SPDLOG_DEBUG("detectedFaces faceNum:{}", detectedFaces.faceNum);
            for (int i = 0; i < detectedFaces.faceNum; i++) {
                SPDLOG_DEBUG("Face Id: {}", detectedFaces.faceID[i]);
                SPDLOG_DEBUG("Face Orient:{}", detectedFaces.faceOrient[i]);
                SPDLOG_DEBUG("Face Rect: {},{},{},{}",
                             detectedFaces.faceRect[i].left, detectedFaces.faceRect[i].top,
                             detectedFaces.faceRect[i].right,
                             detectedFaces.faceRect[i].bottom);
                ASF_FaceFeature feature = {0};
                ASF_SingleFaceInfo singleDetectedFaces = {0};
                singleDetectedFaces.faceRect.left = detectedFaces.faceRect[i].left;
                singleDetectedFaces.faceRect.top = detectedFaces.faceRect[i].top;
                singleDetectedFaces.faceRect.right = detectedFaces.faceRect[i].right;
                singleDetectedFaces.faceRect.bottom = detectedFaces.faceRect[i].bottom;
                singleDetectedFaces.faceOrient = detectedFaces.faceOrient[i];
                singleDetectedFaces.faceDataInfo = detectedFaces.faceDataInfoList[i];
                MRESULT res = ASFFaceFeatureExtract(engineHandle, img.cols, img.rows,
                                                    ASVL_PAF_RGB24_B8G8R8, (MUInt8 *) img.ptr<uchar>(0),
                                                    &singleDetectedFaces, ASF_REGISTER, 0,
                                                    &feature);
                if (MOK == res) {
                    MFloat confidenceLevel;
                    ASF_FaceFeatureInfo faceFeatureInfo = {0};
                    res = ASFFaceFeatureCompare_Search(engineHandle, &feature, &confidenceLevel, &faceFeatureInfo);
                    if (res != MOK) {
                        SPDLOG_DEBUG("ASFFaceFeatureCompare failed : {}", res);
                    } else {
                        FaceStruct faceStruct;
                        faceStruct.name = "Unknow";
                        faceStruct.seq = detectedFaces.faceID[i];
                        faceStruct.left = detectedFaces.faceRect[i].left;
                        faceStruct.top = detectedFaces.faceRect[i].top;
                        faceStruct.right = detectedFaces.faceRect[i].right;
                        faceStruct.bottom = detectedFaces.faceRect[i].bottom;
                        if (confidenceLevel >= facehold) {
                            SPDLOG_DEBUG("ASFFaceFeatureCompare sucessed: {},{},{}", confidenceLevel,
                                         faceFeatureInfo.searchId,
                                         faceFeatureInfo.tag);
                            faceStruct.name = faceFeatureInfo.tag;
                            faceStruct.idcode = faceFeatureInfo.searchId;
                            facelist.push_back(faceStruct);
                        }

                    }
                }
            }
        }
        m_mutex.unlock();
    }
    return facelist;
}

ArcFace::ArcFace() {

}

ArcFace::ArcFace(char *activekey, double facehold) : activekey(activekey), facehold(facehold) {

}

void CutIplImage(IplImage *src, IplImage *dst, int x, int y) {
    CvSize size = cvSize(dst->width, dst->height);//区域大小
    cvSetImageROI(src, cvRect(x, y, size.width, size.height));//设置源图像ROI
    cvCopy(src, dst); //复制图像
    cvResetImageROI(src);//源图像用完后，清空ROI
}


std::string UnicodeToUtf8(const std::wstring &strUnicode) {
    int len = WideCharToMultiByte(CP_UTF8, 0, strUnicode.c_str(), -1, NULL, 0, NULL, NULL);
    if (len == 0) {
        return "";
    }

    char *pRes = new char[len];
    if (pRes == NULL) {
        return "";
    }

    WideCharToMultiByte(CP_UTF8, 0, strUnicode.c_str(), -1, pRes, len, NULL, NULL);
    pRes[len - 1] = '\0';
    std::string result = pRes;
    delete[] pRes;

    return result;
}

std::wstring StringToWString(const std::string &str) {
    int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
    if (len == 0) {
        return L"";
    }

    wchar_t *pRes = new wchar_t[len];
    if (pRes == NULL) {
        return L"";
    }

    MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, pRes, len);
    pRes[len - 1] = L'\0';
    std::wstring result = pRes;
    delete[] pRes;

    return result;
}


void ArcFace::registerSingle(string filename) {
    string name = UnicodeToUtf8(StringToWString(filename));

    string path = fmt::format("faces/{}.jpg", filename);
    IplImage *originalImg = cvLoadImage(path.c_str());
    if (originalImg == NULL) {
        return;
    }
    IplImage *img = cvCreateImage(cvSize(originalImg->width - originalImg->width %
                                                              4, originalImg->height), IPL_DEPTH_8U,
                                  originalImg->nChannels);
    CutIplImage(originalImg, img, 0, 0);
    ASF_MultiFaceInfo detectedFaces = {0};
    MRESULT res = ASFDetectFaces(engineHandle, img->width, img->height, ASVL_PAF_RGB24_B8G8R8,
                                 (MUInt8 *) img->imageData,
                                 &detectedFaces);
    if (MOK == res) {
        if (detectedFaces.faceNum == 0) {
            SPDLOG_ERROR("filename:{} has no face", name);
            return;
        }
        ASF_FaceFeature feature = {0};
        ASF_SingleFaceInfo singleDetectedFaces = {0};
        singleDetectedFaces.faceRect.left = detectedFaces.faceRect[0].left;
        singleDetectedFaces.faceRect.top = detectedFaces.faceRect[0].top;
        singleDetectedFaces.faceRect.right = detectedFaces.faceRect[0].right;
        singleDetectedFaces.faceRect.bottom = detectedFaces.faceRect[0].bottom;
        singleDetectedFaces.faceOrient = detectedFaces.faceOrient[0];
        singleDetectedFaces.faceDataInfo = detectedFaces.faceDataInfoList[0];
        res = ASFFaceFeatureExtract(engineHandle, img->width, img->height,
                                    ASVL_PAF_RGB24_B8G8R8, (MUInt8 *) img->imageData,
                                    &singleDetectedFaces, ASF_REGISTER, 0,
                                    &feature);
        if (MOK == res) {
            ASF_FaceFeatureInfo faceFeatureInfo = {0};
            faceFeatureInfo.searchId = faceNum;
            faceFeatureInfo.feature = &feature;
            faceFeatureInfo.tag = name.c_str();
            ASFRegisterFaceFeature(engineHandle, &faceFeatureInfo, 1);
            cvReleaseImage(&img);
            cvReleaseImage(&originalImg);
            faceNum++;
        }
    }
}

void ArcFace::registerFaceFeature() {
    list<hdir_t> dirs;
    int size = listdir("faces", dirs);
    SPDLOG_INFO("start load face feature waiting...");
    for (hdir_t file: dirs) {
        if (file.type == 100) {
            continue;
        }
        registerSingle(hv::replace(file.name, ".jpg", ""));
        if (faceNum % 100 == 0) {
            SPDLOG_INFO("load face feature count:{}", faceNum);
        }
    }
    MRESULT res = ASFGetFaceCount(engineHandle, &faceNum);
    if (res == MOK) {
        SPDLOG_INFO("load face end count:{}", faceNum);
    }
}


bool ArcFace::Recognition() {
    MRESULT res = MOK;
//    res = ASFOnlineActivation(appid, sdkkey, activekey);
    res = ASFOfflineActivation(activekey);
    if (MOK != res && MERR_ASF_ALREADY_ACTIVATED != res) {
        SPDLOG_ERROR("ASFOnlineActivation fail: {}", res);
        return false;
    }

    MInt32 mask = ASF_FACE_DETECT | ASF_FACERECOGNITION;
    res = ASFInitEngine(ASF_DETECT_MODE_VIDEO, ASF_OP_0_ONLY, ASF_MAX_DETECTFACENUM, mask, &engineHandle);
    if (res != MOK) {
        SPDLOG_ERROR("ASFInitEngine fail: {}", res);
        return false;
    }
    return true;
}


void ArcFace::getSdkRelatedInfo() {
    MRESULT res = MOK;
    //采集当前设备信息，用于离线激活
    char *deviceInfo = NULL;
    res = ASFGetActiveDeviceInfo(&deviceInfo);
    if (res != MOK) {
        SPDLOG_DEBUG("ASFGetActiveDeviceInfo fail: {}", res);
    } else {
        SPDLOG_DEBUG("ASFGetActiveDeviceInfo sucess: {}", deviceInfo);
        HFile h;
        h.open("FaceRegisterCode.txt", "w+");
        h.write(deviceInfo);
        h.close();
        h.flush();
    }
    //获取激活文件信息
    ASF_ActiveFileInfo activeFileInfo = {0};
    res = ASFGetActiveFileInfo(&activeFileInfo);
    if (res != MOK) {
        SPDLOG_ERROR("ASFGetActiveFileInfo fail: {}", res);
    } else {
        //这里仅获取了有效期时间，还需要其他信息直接打印即可
        char startDateTime[32];
        timestampToTime(activeFileInfo.startTime, startDateTime, 32);
        SPDLOG_DEBUG("startTime: {}", startDateTime);
        char endDateTime[32];
        timestampToTime(activeFileInfo.endTime, endDateTime, 32);
        SPDLOG_DEBUG("endTime: {}", endDateTime);
        SPDLOG_DEBUG("ASFGetActiveFileInfo success: {}", res);
    }
}

void ArcFace::timestampToTime(char *timeStamp, char *dateTime, int dateTimeSize) {

}
//
// Created by UMA-PC-001 on 2022/2/19.
//
#include "watchai.h"
#include<cmath>

cv::Mat avframeToCvmat(const AVFrame *frame, AVPixelFormat format, int width, int height) {
    cv::Mat image(height, width, CV_8UC3);
    int cvLinesizes[1];
    cvLinesizes[0] = image.step1();
    switch (format) {
        case AV_PIX_FMT_YUVJ420P :
            format = AV_PIX_FMT_YUV420P;
            break;
        case AV_PIX_FMT_YUVJ422P  :
            format = AV_PIX_FMT_YUV422P;
            break;
        case AV_PIX_FMT_YUVJ444P   :
            format = AV_PIX_FMT_YUV444P;
            break;
        case AV_PIX_FMT_YUVJ440P :
            format = AV_PIX_FMT_YUV440P;
            break;
        default:
            break;
    }
    SwsContext *conversion = sws_getContext(frame->width, frame->height, format, width, height,
                                            AVPixelFormat::AV_PIX_FMT_BGR24, SWS_FAST_BILINEAR, NULL, NULL, NULL);
    sws_scale(conversion, frame->data, frame->linesize, 0, frame->height, &image.data, cvLinesizes);
    sws_freeContext(conversion);
    return image;
}

AVFrame *cvmatToAvframe(cv::Mat *image, AVFrame *frame) {
    int width = image->cols;
    int height = image->rows;
    int cvLinesizes[1];
    cvLinesizes[0] = image->step1();
    if (frame == NULL) {
        frame = av_frame_alloc();
        av_image_alloc(frame->data, frame->linesize, width, height, AVPixelFormat::AV_PIX_FMT_YUV420P, 1);
    }
    SwsContext *conversion = sws_getContext(width, height, AVPixelFormat::AV_PIX_FMT_BGR24, width, height,
                                            (AVPixelFormat) frame->format, SWS_FAST_BILINEAR, NULL, NULL, NULL);
    sws_scale(conversion, &image->data, cvLinesizes, 0, height, frame->data, frame->linesize);
    sws_freeContext(conversion);
    return frame;
}

std::wstring Utf8ToUnicode(const std::string &strUTF8) {
    int len = MultiByteToWideChar(CP_UTF8, 0, strUTF8.c_str(), -1, NULL, 0);
    if (len == 0) {
        return L"";
    }

    wchar_t *pRes = new wchar_t[len];
    if (pRes == NULL) {
        return L"";
    }

    MultiByteToWideChar(CP_UTF8, 0, strUTF8.c_str(), -1, pRes, len);
    pRes[len - 1] = L'\0';
    std::wstring result = pRes;
    delete[] pRes;

    return result;
}

std::string WStringToString(const std::wstring &wstr) {
    int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    if (len == 0) {
        return "";
    }

    char *pRes = new char[len];
    if (pRes == NULL) {
        return "";
    }

    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, pRes, len, NULL, NULL);
    pRes[len - 1] = '\0';
    std::string result = pRes;
    delete[] pRes;

    return result;
}

bool intersect(PredictionResult a, PredictionResult b) {
//    SPDLOG_INFO("[{},{},{},{}] p[{},{},{},{}]", a.eventInfo.left,
//                a.eventInfo.top,
//                a.eventInfo.right, a.eventInfo.bottom, b.eventInfo.left,
//                b.eventInfo.top,
//                b.eventInfo.right, b.eventInfo.bottom);

//    auto verti_dis = abs(a.eventInfo.left - b.eventInfo.left);
//    auto horiz_dis = abs(a.eventInfo.top - b.eventInfo.top);
//    // 高度和宽度之和的一半
//    auto half_w = (a.eventInfo.right - a.eventInfo.left + b.eventInfo.right - b.eventInfo.left) / 2;
//    auto half_y = (a.eventInfo.bottom - a.eventInfo.top + a.eventInfo.bottom - a.eventInfo.top) / 2;
//    if (verti_dis < half_y && horiz_dis < half_w) {
//        return true;
//    }
//    return false;
    if (abs((a.eventInfo.left + a.eventInfo.right) / 2 - (b.eventInfo.left + b.eventInfo.right) / 2) <
        ((a.eventInfo.right + b.eventInfo.right - a.eventInfo.left - b.eventInfo.left) / 2)
        && abs((a.eventInfo.top + a.eventInfo.bottom) / 2 - (b.eventInfo.top + b.eventInfo.bottom) / 2) <
           ((a.eventInfo.bottom + b.eventInfo.bottom - a.eventInfo.top - b.eventInfo.top) / 2))
        return true;
    return false;
}


int httpDownload(const char *url, const char *filepath) {
    HFile file;
    if (file.open(filepath, "wb") != 0) {
        SPDLOG_ERROR("Failed to open file {}", filepath);
        return -20;
    }
    // HEAD
    auto resp = requests::head(url);
    if (resp == NULL) {
        SPDLOG_ERROR("request failed!");
        return -1;
    }
    if (resp->status_code == HTTP_STATUS_NOT_FOUND) {
        SPDLOG_ERROR("404 Not Found");
        return -1;
    }
    bool use_range = false;
    int range_bytes = 1 << 20; // 1M
    std::string accept_ranges = resp->GetHeader("Accept-Ranges");
    size_t content_length = hv::from_string<size_t>(resp->GetHeader("Content-Length"));
    // use Range if server accept_ranges and content_length > 1M
    if (resp->status_code == 200 &&
        accept_ranges == "bytes" &&
        content_length > range_bytes) {
        use_range = true;
    }
    // GET
    if (!use_range) {
        resp = requests::get(url);
        if (resp == NULL) {
            SPDLOG_ERROR("request failed!");
            return -1;
        }
        file.write(resp->body.data(), resp->body.size());
        return 0;
    }
    long from = 0, to = 0;
    int last_progress = 0;
    http_client_t *cli = http_client_new();
    HttpRequestPtr req(new HttpRequest);
    req->method = HTTP_GET;
    req->url = url;
    while (from < content_length) {
        to = from + range_bytes - 1;
        if (to >= content_length) to = content_length - 1;
        req->SetRange(from, to);
        int ret = http_client_send(cli, req.get(), resp.get());
        if (ret != 0) {
            SPDLOG_ERROR("request failed!");
            return -1;
        }
        file.write(resp->body.data(), resp->body.size());
        from = to + 1;
        int cur_progress = from * 100 / content_length;
        if (cur_progress > last_progress) {
            last_progress = cur_progress;
        }
    }
    http_client_del(cli);
    return 0;
}
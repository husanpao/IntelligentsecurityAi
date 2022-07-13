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

float overlapRate(PredictionResult a, PredictionResult b) {
    cv::Rect rc1(a.eventInfo.left, a.eventInfo.top, a.eventInfo.right - a.eventInfo.left,
                 a.eventInfo.bottom - a.eventInfo.top);
    cv::Rect rc2(b.eventInfo.left, b.eventInfo.top, b.eventInfo.right - b.eventInfo.left,
                 b.eventInfo.bottom - b.eventInfo.top);

    CvPoint p1, p2;                 //p1为相交位置的左上角坐标，p2为相交位置的右下角坐标
    p1.x = std::max(rc1.x, rc2.x);
    p1.y = std::max(rc1.y, rc2.y);

    p1.x = std::max(rc1.x, rc2.x);
    p1.y = std::max(rc1.y, rc2.y);

    p2.x = std::min(rc1.x + rc1.width, rc2.x + rc2.width);
    p2.y = std::min(rc1.y + rc1.height, rc2.y + rc2.height);

    float AJoin = 0;
    if (p2.x > p1.x && p2.y > p1.y)            //判断是否相交
    {
        AJoin = (p2.x - p1.x) * (p2.y - p1.y);    //求出相交面积
    }
    float A1 = rc1.width * rc1.height;
    float A2 = rc2.width * rc2.height;
    float AUnion = A1 > A2 ? A1 : A2;                 //两者组合的面积
    if (AUnion > 0)
        return (AJoin / AUnion);                  //相交面积与组合面积的比例
    else
        return 0;
}

//float overlapRate(PredictionResult a, PredictionResult b) {
//    SPDLOG_INFO("a:[{},{},{},{}] p[{},{},{},{}]", a.eventInfo.left,
//                a.eventInfo.top,
//                a.eventInfo.right, a.eventInfo.bottom, b.eventInfo.left,
//                b.eventInfo.top,
//                b.eventInfo.right, b.eventInfo.bottom);
//    int x1 = a.eventInfo.left;
//    int y1 = a.eventInfo.bottom;
//    int h1 = a.eventInfo.bottom - a.eventInfo.top;
//    int w1 = a.eventInfo.right - a.eventInfo.left;
//
//    int x2 = b.eventInfo.left;
//    int y2 = b.eventInfo.bottom;
//    int h2 = b.eventInfo.bottom - b.eventInfo.top;
//    int w2 = b.eventInfo.right - b.eventInfo.left;
//    int endx = max(x1 + w1, x2 + w2);
//    int startx = min(x1, x2);
//    int width = w1 + w2 - (endx - startx);  // 重叠部分宽
//    int endy = max(y1 + h1, y2 + h2);
//    int starty = min(y1, y2);
//    int height = h1 + h2 - (endy - starty);  // 重叠部分高
//    if (width > 0 && height > 0) {
//        int area = width * height;  // 重叠部分面积
//        int area1 = w1 * h1;
//        int area2 = w2 * h2;
//        float ratio = (float) area / (area1 + area2 - area);
//        return ratio;
//    } else {
//        // 不重叠：算出来的width或height小于等于0
//        return 0.0;
//    }
//}


int httpDownload(const char *url, const char *filepath) {
    HFile file;
    if (file.open(filepath, "wb") != 0) {
        SPDLOG_ERROR("Failed to open file {}", filepath);
        return -20;
    }
    if (strlen(url) < 10) {
        SPDLOG_ERROR("url is valid:{}", url);
        return -1;
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
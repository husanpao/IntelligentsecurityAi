//
// Created by UMA-PC-001 on 2022/2/17.
//

#ifndef WATCHAI_YOLOV5_H
#define WATCHAI_YOLOV5_H
#include "watchai.h"
#include <torch/torch.h>
#include <torch/script.h>
class ImageResizeData {
public:
    // 添加处理过后的图片
    void setImg(cv::Mat img);

    // 获取处理过后的图片
    cv::Mat getImg();

    // 当原始图片宽高比大于处理过后图片宽高比时此函数返回 true
    bool isW();

    // 当原始图片高宽比大于处理过后图片高宽比时此函数返回 true
    bool isH();

    // 添加处理之后图片的宽
    void setWidth(int width);

    // 获取处理之后图片的宽
    int getWidth();

    // 添加处理之后图片的高
    void setHeight(int height);

    // 获取处理之后图片的高
    int getHeight();

    // 添加原始图片的宽
    void setW(int w);

    // 获取原始图片的宽
    int getW();

    // 添加原始图片的高
    void setH(int h);

    // 获取原始图片的高
    int getH();

    // 添加从原始图片到处理过后图片所添加黑边大小
    void setBorder(int border);

    // 获取从原始图片到处理过后图片所添加黑边大小
    int getBorder();

private:
    // 处理过后图片高
    int height;
    // 处理过后图片宽
    int width;
    // 原始图片宽
    int w;
    // 原始图片高
    int h;
    // 从原始图片到处理图片所添加的黑边大小
    int border;
    // 处理过后的图片
    cv::Mat img;
};
struct ModelInfo {
    string name;
    double similarity;
    bool enable;
    torch::jit::script::Module model;
    std::map<int, std::string> labels;
};

class YoloV5 {
public:
    /**
     * 构造函数
     * @param ptFile yoloV5 pt文件路径
     * @param isCuda 是否使用 cuda 默认不起用
     * @param height yoloV5 训练时图片的高
     * @param width yoloV5 训练时图片的宽
     * @param confThres 非极大值抑制中的 scoreThresh
     * @param iouThres 非极大值抑制中的 iouThresh
     */
    YoloV5(std::string ptFile, bool isCuda = false, int height = 640, int width = 640,
           float confThres = 0.25,
           float iouThres = 0.45);

    /**
     * 预测函数
     * @param data 语言预测的数据格式 (batch, rgb, height, width)
     */
    std::vector<torch::Tensor> prediction(torch::Tensor data);

    std::vector<PredictionResult> prediction_my(cv::Mat img, set<string> algorithm_list);

    /**
     * 预测函数
     * @param filePath 需要预测的图片路径
     */
    std::vector<torch::Tensor> prediction(std::string filePath);

    /**
     * 预测函数
     * @param img 需要预测的图片
     */
    std::vector<torch::Tensor> prediction(cv::Mat img);


    /**
     * 预测函数
     * @param imgs 需要预测的图片集合
     */
    std::vector<torch::Tensor> prediction(std::vector<cv::Mat> imgs);

    /**
     * 改变图片大小的函数
     * @param img 原始图片
     * @param height 要处理成的图片的高
     * @param width 要处理成的图片的宽
     * @return 封装好的处理过后图片数据结构
     */
    static ImageResizeData resize(cv::Mat img, int height, int width);

    /**
     * 改变图片大小的函数
     * @param img 原始图片
     * @return 封装好的处理过后图片数据结构
     */
    ImageResizeData resize(cv::Mat img);

    /**
     * 改变图片大小的函数
     * @param imgs 原始图片集合
     * @param height 要处理成的图片的高
     * @param width 要处理成的图片的宽
     * @return 封装好的处理过后图片数据结构
     */
    static std::vector<ImageResizeData> resize(std::vector<cv::Mat> imgs, int height, int width);

    /**
     * 改变图片大小的函数
     * @param imgs 原始图片集合
     * @return 封装好的处理过后图片数据结构
     */
    std::vector<ImageResizeData> resize(std::vector<cv::Mat> imgs);

    /**
     * 根据输出结果在给定图片中画出框
     * @param imgs 原始图片集合
     * @param rectangles 通过预测函数处理好的结果
     * @param labels 类别标签
     * @param thickness 线宽
     * @return 画好框的图片
     */
    std::vector<cv::Mat>
    drawRectangle(std::vector<cv::Mat> imgs, std::vector<torch::Tensor> rectangles,
                  std::map<int, std::string> labels,
                  int thickness = 2);

    /**
     * 根据输出结果在给定图片中画出框
     * @param imgs 原始图片集合
     * @param rectangles 通过预测函数处理好的结果
     * @param thickness 线宽
     * @return 画好框的图片
     */
    std::vector<cv::Mat>
    drawRectangle(std::vector<cv::Mat> imgs, std::vector<torch::Tensor> rectangles, int thickness = 2);

    /**
     * 根据输出结果在给定图片中画出框
     * @param imgs 原始图片集合
     * @param rectangles 通过预测函数处理好的结果
     * @param colors 每种类型对应颜色
     * @param labels 类别标签
     * @return 画好框的图片
     */
    std::vector<cv::Mat>
    drawRectangle(std::vector<cv::Mat> imgs, std::vector<torch::Tensor> rectangles,
                  std::map<int, cv::Scalar> colors,
                  std::map<int, std::string> labels, int thickness = 2);

    /**
     * 根据输出结果在给定图片中画出框
     * @param img 原始图片
     * @param rectangle 通过预测函数处理好的结果
     * @param thickness 线宽
     * @return 画好框的图片
     */
    cv::Mat drawRectangle(cv::Mat img, torch::Tensor rectangle, int thickness = 2);

    /**
     * 根据输出结果在给定图片中画出框
     * @param img 原始图片
     * @param faceStruct 人脸信息
     * @param thickness 线宽
     * @return 画好框的图片
     */

    void
    drawRectangleFace(cv::Mat img, int left, int top, int right, int bottom, string label, int thickness = 2);

    void drawRectangle(cv::Mat img, int left, int top, int right, int bottom, string label,
                       int thickness = 2);


    cv::Mat
    drawRectangle(cv::Mat img, torch::Tensor rectangle, std::map<int, std::string> labels, int thickness = 2);

    /**
     * 根据输出结果在给定图片中画出框
     * @param img 原始图片
     * @param rectangle 通过预测函数处理好的结果
     * @param colos 每种类型对应颜色
     * @param labels 类别标签
     * @param thickness 线宽
     * @return 画好框的图片
     */
    cv::Mat drawRectangle(cv::Mat img, torch::Tensor rectangle, std::map<int, cv::Scalar> colors,
                          std::map<int, std::string> labels, int thickness = 2);

    /**
     * 用于判断给定数据是否存在预测
     * @param clazz 通过预测函数处理好的结果
     * @return 如果图片中存在给定某一种分类返回 true
     */
    bool existencePrediction(torch::Tensor clazz);

    /**
     * 用于判断给定数据是否存在预测
     * @param classs 通过预测函数处理好的结果
     * @return 如果图片集合中存在给定某一种分类返回 true
     */
    bool existencePrediction(std::vector<torch::Tensor> classs);

    // 随机获取一种颜色
    cv::Scalar getRandScalar();

private:
    // 是否启用 cuda
    bool isCuda;
    mutex m_mutexDraw;
    mutex m_mutexPred;
    // 非极大值抑制中的第一步数据清理
    float confThres;
    // 非极大值抑制中 iou
    float iouThres;
    // 模型所需要的图片的高
    float height;
    // 模型所需要的图片的宽
    float width;
    // 画框颜色 map
    std::map<int, cv::Scalar> mainColors;
    // 模型
//    torch::jit::script::Module model;
    list<ModelInfo> models;

    // 图片通道转换为 rgb
    cv::Mat img2RGB(cv::Mat img);

    // 图片变为 Tensor
    torch::Tensor img2Tensor(cv::Mat img);

    // (center_x center_y w h) to (left, top, right, bottom)
    torch::Tensor xywh2xyxy(torch::Tensor x);

    // 非极大值抑制算法
    torch::Tensor nms(torch::Tensor bboxes, torch::Tensor scores, float thresh);

    // 预测出来的框根据原始图片还原算法
    std::vector<torch::Tensor> sizeOriginal(std::vector<torch::Tensor> result, std::vector<ImageResizeData> imgRDs);

    // 非极大值抑制算法整体
    std::vector<torch::Tensor>
    non_max_suppression(torch::Tensor preds, float confThres = 0.25, float iouThres = 0.45);
};


#endif //WATCHAI_YOLOV5_H

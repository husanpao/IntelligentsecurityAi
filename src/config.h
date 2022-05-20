//
// Created by UMA-PC-001 on 2022/2/20.
//

#ifndef WATCHAI_CONFIG_H
#define WATCHAI_CONFIG_H
#include "watchai.h"
struct conf_item {
    std::string title = {""};
    std::vector<std::pair<std::string, std::string>> items = {};
};

class config {
private:
    std::vector<struct conf_item> m_data;
    std::vector<std::string> m_filelist; //用于临时保存配置文件列表
    std::string m_file;

    void parse_conf();

    void m_readfile(std::string fname);

    void trim(std::string &s); //清除前后的空格


public:
    //给定一个配置文件的文件名,立刻获取配置的内容
    config(std::string filename);

    // title 标题部分
    // key 要提取的关键项
    // def 如果找不到关键项的值,提供的默认值
    char *readStr(std::string title, std::string key, std::string def = "");

    int readInt(std::string title, std::string key, std::string def = "");

    double readDouble(std::string title, std::string key, std::string def = "");

    int size();

    ~config() = default;
};


#endif //WATCHAI_CONFIG_H

//
// Created by UMA-PC-001 on 2022/2/20.
//

#include "config.h"

config::config(std::string filename) : m_file(filename) {
    m_filelist.clear();
    m_data.clear();
    m_filelist.push_back(m_file);
    parse_conf();
}

char *config::readStr(std::string title, std::string key, std::string def) {
    for (auto &&i: m_data) {
        if (i.title == title) {
            //auto&&或函数参数类型的自动推导的T&&是一个未定的引用类型，它可能是左值引用，也可能是右值引用，取决于初始化的值类型
            for (auto &&b: i.items) {
                if (b.first == key) {
                    return const_cast<char *>(b.second.c_str());
                }
            }
        }
    }
    return "def";
}

int config::readInt(std::string title, std::string key, std::string def) {
    const string &value = readStr(title, key);
    if (!value.empty()) {
        return stoi(value);
    } else {
        return 0;
    }
}

double config::readDouble(std::string title, std::string key, std::string def) {
    const string &value = readStr(title, key);
    if (!value.empty()) {
        return stod(value);
    } else {
        return 0;
    }
}


void config::trim(std::string &s) {
    s.erase(s.find_last_not_of(" ") + 1);
    s.erase(0, s.find_first_not_of(" "));
    for (int i = 0; i < strlen(s.data()); i++) {
        if (s[i] == '\n' || s[i] == '\r')
            s[i] = '\0';
    }
}

int config::size() {
    return m_data.size();
}


void config::m_readfile(std::string fname) {
    if (fname.empty()) {
        return;
    }
    std::ifstream f(fname, std::ios::in);
    std::string line;
    if (!f.is_open()) //判断是否打开
    {
        spdlog::throw_spdlog_ex("config.conf file not found");
        return;
    }
    //读取这个文件的一行
    conf_item *item = nullptr;
    while (getline(f, line)) //读取一行,直到读完
    {

        trim(line);     //清空前后空格
        if (line == "" || line.find_first_of(";") == 0 || line.find_first_of("#") == 0) //如果是空字符串
        {
            continue;
        }//如果是标题
        if (line.find_first_of("[") == 0 && line.find_last_of("]") == strlen(line.data()) - 1) {
            trim(line);
            line = line.substr(1, strlen(line.data()) - 2);
            if (item != nullptr) {
                m_data.push_back(*item);
                delete item;
            }

            item = new conf_item();
            item->title = line;
            continue;
        } else {
            int pos = line.find_first_of("=");
            if (pos == -1) {
                continue;
            }
            std::string key = line.substr(0, pos);
            trim(key);
            std::string val = line.substr(pos + 1);
            trim(val);
            SPDLOG_DEBUG("{}.{}={}", item->title, key, val);
            item->items.push_back(std::pair<std::string, std::string>(key, val));
            continue;
        }

    }
    if (item != nullptr) {
        m_data.push_back(*item);
        delete item;
    }
}

void config::parse_conf() {
    int index = 0;
    for (index = 0; index < m_filelist.size(); index++) {
        m_readfile(m_filelist.at(index));
    }
}
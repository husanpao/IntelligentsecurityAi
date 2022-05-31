
#include "hv/HttpServer.h"
#include "CameraPull.h"
#include "config.h"
#include "CameraHandle.h"
#include "Controller.h"
#include <DbgHelp.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/daily_file_sink.h>
#include "hv/EventLoop.h"

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE//必须定义这个宏,才能输出文件名和行号
using namespace hv;

int GenerateMiniDump(PEXCEPTION_POINTERS pExceptionPointers) {
    // 定义函数指针
    typedef BOOL(WINAPI *MiniDumpWriteDumpT)(
            HANDLE,
            DWORD,
            HANDLE,
            MINIDUMP_TYPE,
            PMINIDUMP_EXCEPTION_INFORMATION,
            PMINIDUMP_USER_STREAM_INFORMATION,
            PMINIDUMP_CALLBACK_INFORMATION
    );
    // 从 "DbgHelp.dll" 库中获取 "MiniDumpWriteDump" 函数
    MiniDumpWriteDumpT pfnMiniDumpWriteDump = NULL;
    HMODULE hDbgHelp = LoadLibrary("DbgHelp.dll");
    if (NULL == hDbgHelp) {
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    pfnMiniDumpWriteDump = (MiniDumpWriteDumpT) GetProcAddress(hDbgHelp, "MiniDumpWriteDump");

    if (NULL == pfnMiniDumpWriteDump) {
        FreeLibrary(hDbgHelp);
        return EXCEPTION_CONTINUE_EXECUTION;
    }

    HANDLE hDumpFile = CreateFile("app.dmp", GENERIC_READ | GENERIC_WRITE,
                                  FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
    if (INVALID_HANDLE_VALUE == hDumpFile) {
        FreeLibrary(hDbgHelp);
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    // 写入 dmp 文件
    MINIDUMP_EXCEPTION_INFORMATION expParam;
    expParam.ThreadId = GetCurrentThreadId();
    expParam.ExceptionPointers = pExceptionPointers;
    expParam.ClientPointers = FALSE;
    pfnMiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
                         hDumpFile, MiniDumpWithDataSegs, (pExceptionPointers ? &expParam : NULL), NULL, NULL);
    // 释放文件
    CloseHandle(hDumpFile);
    FreeLibrary(hDbgHelp);
    return EXCEPTION_EXECUTE_HANDLER;
}

LONG WINAPI ExceptionFilter(LPEXCEPTION_POINTERS lpExceptionInfo) {
    // 这里做一些异常的过滤或提示
    if (IsDebuggerPresent()) {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    return GenerateMiniDump(lpExceptionInfo);
}

void FFMPEG_LOG(void *ptr, int level, const char *fmt, va_list vl) {
    if (level < AV_LOG_WARNING) {
        char line[1024];
        static int print_prefix = 1;
        va_list vl2;
        va_copy(vl2, vl);
        av_log_format_line(ptr, level, fmt, vl2, line, sizeof(line), &print_prefix);
                va_end(vl2);
        SPDLOG_ERROR("FFMPEG :{}", line);
    }
}

void initconfig(string level, int port) {
    //设置libhv日志配置
    hlog_set_file(fmt::format("logs/network_{}", port).c_str());
    hlog_set_remain_days(1);
    //设置ffmpeg
//    av_log_set_level(AV_LOG_ERROR); //设置日志级别
    av_log_set_callback(FFMPEG_LOG);  // 设置自定义的日志输出方法
    //设置程序日志配置
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_st>());
    sinks.push_back(
            std::make_shared<spdlog::sinks::daily_file_sink_st>(fmt::format("logs/znajgkpt_{}.log", port), 00, 01));
    auto logger = std::make_shared<spdlog::logger>("name", begin(sinks), end(sinks));
    spdlog::register_logger(logger);
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] %^[%l]%$ [%s:%#] %v ");
//    logger->set_pattern("[%Y-%m-%d %T.%e] [%l] %v");
    if (level == "debug") {
        logger->set_level(spdlog::level::debug);
        spdlog::flush_on(spdlog::level::debug);
    } else if (level == "warn") {
        logger->set_level(spdlog::level::warn);
        spdlog::flush_on(spdlog::level::warn);
    } else if (level == "error") {
        logger->set_level(spdlog::level::err);
        spdlog::flush_on(spdlog::level::err);
    } else {
        logger->set_level(spdlog::level::info);
        spdlog::flush_on(spdlog::level::info);
    }
    spdlog::set_default_logger(logger);
}

void sig_handler(int sig) {
    if (sig == SIGINT) {
        SPDLOG_INFO("Terminated by Ctrl+C signal.");
        exit(0);
    }
}

static void onTimer(TimerID timerID, string url, int port) {
    auto resp = requests::get(fmt::format("{}?port={}", url, port).c_str());
    if (resp == NULL) {
        SPDLOG_INFO("Datacontrol is offline...");
    }
}

int main(int argc, char *argv[]) {
    SetUnhandledExceptionFilter(ExceptionFilter);
    SetConsoleTitle("智能安监管控平台-20220527"); // 设置窗口标题
    config *c = new config("./config.conf");
    int port = c->readInt("APP", "port", "1360");
    if (argc > 1) {
        port = atoi(argv[1]);
    }
    initconfig(c->readStr("LOG", "level"), port);
    AppConfig *config = new AppConfig();
    config->rtmp_server = c->readStr("MEDIA", "rtmp_server");
    config->live_server = c->readStr("MEDIA", "live_server");
    config->static_path = c->readStr("APP", "static_path");
    config->faceKey = c->readStr("FACE", "key", "facekey.txt");
    config->interval = c->readInt("APP", "interval", "300") * 1000;
    config->time_interval = c->readInt("APP", "time_interval", "600") * 1000;
    config->maxstream = c->readInt("APP", "maxstream", "5");
    config->facehold = c->readDouble("APP", "facehold", "0.8");
    config->frame_interval = c->readDouble("APP", "frame_interval", "10");
    config->static_host = c->readStr("MEDIA", "static_host");
    config->api_host = c->readStr("APP", "api_host");
    config->weight = c->readStr("YOLOV5", "weight");
    config->lables = c->readStr("YOLOV5", "labels");
    config->showhold = c->readInt("APP", "showhold", "0");
    EventLoopPtr loop(new EventLoop);
    loop->setInterval(10000, std::bind(onTimer, std::placeholders::_1, c->readStr("APP", "datacontrol"), port));
    thread t([loop]() {
        loop->run();
    });
    t.detach();
    Controller *control = new Controller(config);
    HttpService router;
    router.GET("/ping", [](HttpRequest *req, HttpResponse *resp) {
        return resp->String("pong");
    });
    router.GET("/getplayers", [control, port](HttpRequest *req, HttpResponse *resp) {
        rapidjson::StringBuffer buf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
        writer.StartObject();
        writer.Key("code");
        writer.Int(200);
        writer.Key("client");
        writer.Int(port);
        writer.Key("data");
        writer.StartArray();
        for (auto player: control->players) {
            writer.String(player.first.c_str());
        }
        writer.EndArray();
        writer.EndObject();
        return resp->String(buf.GetString());
    });
    router.POST("/media/api", [control](const HttpContextPtr &ctx) {
        rapidjson::Document dom;
        SPDLOG_INFO("request body:{}", ctx->body());
        if (!dom.Parse(ctx->body().c_str()).HasParseError()) {
            if (dom.HasMember("msgType") && dom["msgType"].IsInt()) {
                int msgType = dom["msgType"].GetInt();
                string msgText = "UNKnow";
                string result = "";
                if (msgType == 101) {
                    msgText = "ADD FACE MSG";
                    result = control->AddFace(dom["data"]);
                } else if (msgType == 201) {
                    msgText = "MAVideo MSG";
                    result = control->MAVideo(dom["data"]);
                } else if (msgType == 202) {
                    result = control->SubVideo(dom["data"]);
                    msgText = "SubVideo MSG";
                } else if (msgType == 205) {
                    result = control->LookPic(dom["data"]);
                    msgText = "LookPic MSG";
                } else {
                    result = "{\"code\":500,\"message\":\"error\",\"data\":\"UNKnow msgType\"}";
                }
                SPDLOG_INFO("msgType:{},msgText:{},result:{}", msgType, msgText, result);
                return ctx->send(result, ctx->type());
            }
        } else {
            return ctx->send(
                    "{\"code\":500,\"message\":\"error\",\"data\":\"body not valid\"}",
                    ctx->type());
        }
        return ctx->send(ctx->body(), ctx->type());
    });

    http_server_t server;
    server.port = port;
    server.service = &router;
    SPDLOG_INFO("load environment success. service started on {} .", port);
    signal(SIGINT, sig_handler);
    http_server_run(&server);
    return 0;
}

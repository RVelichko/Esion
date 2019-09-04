/**
 * \brief  Вебсокет Сервер для организации работы счётчиков.
 * \author Величко Ростислав
 * \date   16.09.2018
 */

#include <dlfcn.h>
#include <unistd.h>

#include <iostream>
#include <memory>
#include <functional>
#include <thread>
#include <sstream>
#include <atomic>

#include "Log.hpp"
#include "json.hpp"
#include "SignalDispatcher.hpp"
#include "http_server.h"
#include "http_headers.h"
#include "http_content_type.h"


static const size_t DEFAULT_SERVER_PORT = 50000;
static const char DEFAULT_PAGE_POINT[]  = "^/config?$";
static const char DEFAULT_SRV_ADDRESS[] = "0.0.0.0";
static std::uint16_t DEFAULT_SRV_THREADS = std::thread::hardware_concurrency() + 1;
static const char DEFAULT_ROOT_DIR[] = "./content";
static const char DEFAULT_PAGE[] = "index.html";


struct GlobalArgs {
    int port;               ///< параметр -p
    const char* page_point; ///< параметр -w
    const char* address;    ///< параметр -a
    int threads;            ///< параметр -t
    const char* root_dir;   ///< параметр -r
    const char* page;       ///< параметр -i
} __gargs;


static const char *__opt_string = "p:w:a:t:r:i:h?";
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void HelpMessage() {
    std::cout << "  Use:\n\t#server -p " << DEFAULT_SERVER_PORT << "\n"
              << "  Args:\n"
              << "\t[-p]\t Web socket server port.\t[" << DEFAULT_SERVER_PORT << "]\n"
              << "\t[-w]\t Web page connection point.\t[" << DEFAULT_PAGE_POINT << "]\n"
              << "\t[-a]\t Server address.\t[" << DEFAULT_SRV_ADDRESS << "]\n"
              << "\t[-t]\t Server threads.\t[" << DEFAULT_SRV_THREADS << "]\n"
              << "\t[-r]\t Root contents directory.\t[" << DEFAULT_ROOT_DIR << "]\n"
              << "\t[-i]\t Web page.\t[" << DEFAULT_PAGE << "]\n"
              << "__________________________________________________________________\n\n";
    exit(EXIT_FAILURE);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


typedef utils::SignalDispatcher SignalDispatcher;
typedef std::shared_ptr<SignalDispatcher> PSignalDispatcher;
typedef std::lock_guard<std::mutex> LockQuard;
typedef nlohmann::json Json;

int main(int argc_, char **argv_) {
    /// Инициализация globalArgs до начала работы с ней.
    __gargs.port       = DEFAULT_SERVER_PORT;
    __gargs.page_point = DEFAULT_PAGE_POINT;
    __gargs.address    = DEFAULT_SRV_ADDRESS;
    __gargs.threads    = DEFAULT_SRV_THREADS;
    __gargs.root_dir   = DEFAULT_ROOT_DIR;
    __gargs.page       = DEFAULT_PAGE;

    /// Обработка входных опций.
    int opt = getopt(argc_, argv_, __opt_string);
    while(opt not_eq -1) {
        switch(opt) {
            case 'p':{
                    int value = strtol(optarg, (char**)nullptr, 10);
                    if (value <= 1000) {
                        HelpMessage();
                    }
                    __gargs.port = value;
                } break;
            case 'w':
                __gargs.page_point = optarg;
                break;
            case 'a':
                __gargs.address = optarg;
                break;
            case 't': {
                    int value = strtol(optarg, (char**)nullptr, 10);
                    if (100 < value) {
                        HelpMessage();
                    }
                    __gargs.threads = value;
                } break;
            case 'r':
                __gargs.root_dir = optarg;
                break;
            case 'i':
                __gargs.page = optarg;
                break;

            case 'h':
            case '?':
                HelpMessage();
                break;

            default: break;
        }
        opt = getopt(argc_, argv_, __opt_string);
    }
    LOG_TO_STDOUT;
    LOG(DEBUG) << __gargs.port;
    LOG(DEBUG) << __gargs.page_point;
    LOG(DEBUG) << __gargs.address;
    LOG(DEBUG) << __gargs.threads;
    LOG(DEBUG) << __gargs.root_dir;
    LOG(DEBUG) << __gargs.page;
    try {
        using namespace Network;
        std::atomic_bool is_runing{true};
        HttpServer srv(__gargs.address, __gargs.port, __gargs.threads, [&](IHttpRequestPtr req) {
            std::string path = req->GetPath();
            LOG(TRACE) << "Input: \"" << path << "\"";
            auto set_req_fn = [&](const Json& js) {
                req->SetResponseAttr("Access-Control-Allow-Origin", "*");
                req->SetResponseAttr("Access-Control-Allow-Methods", "*");
                req->SetResponseAttr("Access-Control-Allow-Headers", "*");
                req->SetResponseAttr("Content-Type", "application/json;charset=UTF-8");
                req->SetResponseString(js.dump());
            };
            if (path == "/app_info") {
                set_req_fn({
                  { "dev_id", "1234567890" },
                  { "power_percent", 99 }
                });
            } else if (path == "/settings_info") {
                set_req_fn({
                    {"cloud_url", "esion.ru"},
                    {"ssid",      "wifi"},
                    {"pswd",      "wifipassword"},
                    {"user",      "Test"},
                    {"address",   "Test address"},
                    {"desc",      "Тестовая конфигурация."},
                    { "power", "bat_4aa" }
                });
            } else if (path == "/counter_0") {
                set_req_fn({
                    {"type", "debug 1"},
                    {"ser_num", "001"},
                    {"unit", "литр"},
                    {"unit_impl", 1},
                    {"desc", "Тестовый счётчик 1"},
                    {"mcubs", 0.456}
                });
            } else if (path == "/counter_1") {
                set_req_fn({
                    {"type", "debug 2"},
                    {"ser_num", "002"},
                    {"unit", "литр"},
                    {"unit_impl", 10},
                    {"desc", "Тестовый счётчик 2"},
                    {"mcubs", 0.123}
                });
            } else if (path == "/counter_2" or path == "/counter_3") {
                set_req_fn({
                    {"type", "none"}
                });
            } else if (path == "/apply") {
                size_t len = req->GetContentSize();
                std::vector<uint8_t> buf(len, '\0');
                req->GetContent(&buf[0], len, false);
                LOG(DEBUG) << std::string(buf.begin(), buf.end());
                set_req_fn({
                    {"status", "ok"}
                });
            } else if (path == "/exit") {
                LOG(TRACE) << "EXIT";
                set_req_fn({
                    {"status", "ok"}
                });
            } else {
                path = __gargs.root_dir + path + (path == "/" ? __gargs.page : std::string());
                std::stringstream Io;
                Io << "Path: " << path << std::endl
                   << Http::Request::Header::Host::Name << ": "
                   << req->GetHeaderAttr(Http::Request::Header::Host::Value) << std::endl
                   << Http::Request::Header::Referer::Name << ": "
                   << req->GetHeaderAttr(Http::Request::Header::Referer::Value) << std::endl;
                LOG(DEBUG) << "Io: \"" << Io.str() << "\"";
                req->SetResponseAttr(Http::Response::Header::Server::Value, "MyTestServer");
                req->SetResponseAttr(Http::Response::Header::ContentType::Value, Http::Content::TypeFromFileName(path));
                req->SetResponseFile(path);
            }
        });
        PSignalDispatcher sd = std::make_shared<SignalDispatcher>([&] {
            LOG(DEBUG) << "Server is stopped."; //is_runing = false;
        });
    } catch (std::exception const &e) {
      LOG(ERROR) << e.what();
      return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

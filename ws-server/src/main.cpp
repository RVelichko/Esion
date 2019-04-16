/**
 * \brief  Вебсокет Сервер для организации работы счётчиков.
 * \author Величко Ростислав
 * \date   16.09.2018
 */

// docker run -i -t -v /home/rostislav/Develop/esion/ws-server:/ws-server ubuntu14 /bin/bash
// cmake -DCMAKE_C_COMPILER=gcc-4.9 -DCMAKE_CXX_COMPILER=g++-4.9 ..
// sudo systemctl status esion-srv.service

#include <iostream>
#include <memory>
#include <tuple>
#include <functional>

#include "DevicePeerWorker.hpp"
#include "OperatorPeerWorker.hpp"
#include "SignalDispatcher.hpp"
#include "WebSocketServer.hpp"
#include "DbFacade.hpp"
#include "Log.hpp"


static const size_t DEFAULT_SERVER_PORT = 20000;
static char DEFAULT_SSL[] = "";
static char DEFAULT_DEVICE_POINT[] = "^/device?$";
static char DEFAULT_PAGE_POINT[]   = "^/info?$";

static char DEFAULT_DB_ADDRESS[]  = "94.127.68.132";
static char DEFAULT_DB_NAME[]     = "devices";
static char DEFAULT_DB_LOGIN[]    = "esion";
static char DEFAULT_DB_PASSWORD[] = "esionpassword";


struct GlobalArgs {
    int port;           ///< параметр -p
    char* ssl_crt;      ///< параметр -c
    char* ssl_key;      ///< параметр -k
    char* device_point; ///< параметр -e
    char* page_point;   ///< параметр -w
    char* db_addr;      ///< параметр -a
    char* db_name;      ///< параметр -n
    char* db_ligin;     ///< параметр -l
    char* db_pswd;      ///< параметр -s
} __global_args;


static const char *__opt_string = "p:k:c:e:w:a:l:n:s:h?";
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void HelpMessage() {
    std::cout << "  Use:\n\t#ws-server -p " << DEFAULT_SERVER_PORT << "\n"
              << "  Args:\n"
              << "\t[-p]\t Web socket server port. Default: " << DEFAULT_SERVER_PORT << "\n"
              << "\t[-c]\t SSL sertificate path.\n"
              << "\t[-k]\t SSL key path.\n"
              << "\t[-e]\t Device connection point. Default: " << DEFAULT_DEVICE_POINT << "\n"
              << "\t[-w]\t Web page connection point. Default: " << DEFAULT_PAGE_POINT << "\n"
              << "\t[-a]\t DB address. Default: " << DEFAULT_DB_ADDRESS << "\n"
              << "\t[-n]\t DB name. Default: " << DEFAULT_DB_NAME << "\n"
              << "\t[-l]\t DB login. Default: " << DEFAULT_DB_LOGIN << "\n"
              << "\t[-s]\t DB password. Default: " << DEFAULT_DB_PASSWORD << "\n"
              << "__________________________________________________________________\n\n";
    exit(EXIT_FAILURE);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


typedef utils::SignalDispatcher SignalDispatcher;
typedef std::shared_ptr<SignalDispatcher> PSignalDispatcher;
typedef server::DbFacade DbFacade;
typedef server::PDbFacade PDbFacade;
typedef server::DevicePeerWorker DevicePeerWorker;
typedef server::OperatorPeerWorker OperatorPeerWorker;
typedef std::shared_ptr<DevicePeerWorker> PDevicePeerWorker;
typedef std::shared_ptr<OperatorPeerWorker> POperatorPeerWorker;
typedef wsocket::WSServer WSServer;


int main(int argc_, char **argv_) {
    /// Инициализация globalArgs до начала работы с ней.
    __global_args.port         = DEFAULT_SERVER_PORT;    
    __global_args.ssl_crt      = DEFAULT_SSL;
    __global_args.ssl_key      = DEFAULT_SSL;
    __global_args.device_point = DEFAULT_DEVICE_POINT;
    __global_args.page_point   = DEFAULT_PAGE_POINT;
    __global_args.db_addr      = DEFAULT_DB_ADDRESS;
    __global_args.db_name      = DEFAULT_DB_NAME;
    __global_args.db_ligin     = DEFAULT_DB_LOGIN;
    __global_args.db_pswd      = DEFAULT_DB_PASSWORD;

    /// Обработка входных опций.
    int opt = getopt(argc_, argv_, __opt_string);
    while(opt not_eq -1) {
        switch(opt) {
            case 'p':{
                    int value = strtol(optarg, (char**)nullptr, 10);
                    if (value <= 1000) {
                        HelpMessage();
                    }
                    __global_args.port = value;
                } break;
            case 'c':
                __global_args.ssl_crt = optarg;
                break;
            case 'k':
                __global_args.ssl_key = optarg;
                break;
            case 'e':
                __global_args.device_point = optarg;
                break;
            case 'w':
                __global_args.page_point = optarg;
                break;
            case 'a':
                __global_args.db_addr = optarg;
                break;
            case 'n':
                __global_args.db_name = optarg;
                break;
            case 'l':
                __global_args.db_ligin = optarg;
                break;
            case 's':
                __global_args.db_pswd = optarg;
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
    /// Доступ к БД.
    PDbFacade db(new DbFacade());
    if (db->connect(__global_args.db_addr, __global_args.db_name, __global_args.db_ligin, __global_args.db_pswd)) {
        std::mutex mutex; ///< Объект синхронизации доступа к общим объектам.
        /// Точка подключения робота - источника видеочений
        PDevicePeerWorker device_pw = std::make_shared<DevicePeerWorker>(mutex, db);
        /// Точка подключения оператора - потребителя видео
        POperatorPeerWorker oper_pw = std::make_shared<OperatorPeerWorker>(mutex, db);
        /// Конструирование сервера
        WSServer p2p(__global_args.port, 
                     __global_args.ssl_crt, 
                     __global_args.ssl_key,
                     std::make_pair(__global_args.device_point, device_pw),
                     std::make_pair(__global_args.page_point, oper_pw));
        return EXIT_SUCCESS;
    }
    LOG(ERROR) << "Check Database parameters.";
    return EXIT_FAILURE;
}

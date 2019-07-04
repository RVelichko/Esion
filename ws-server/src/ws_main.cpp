/**
 * \brief  Вебсокет Сервер для организации работы счётчиков.
 * \author Величко Ростислав
 * \date   16.09.2018
 */

// docker run -i -t -v /home/rostislav/Develop/esion/ws-server:/ws-server ubuntu14 /bin/bash
// cmake -DCMAKE_C_COMPILER=gcc-4.9 -DCMAKE_CXX_COMPILER=g++-4.9 ..
// sudo cp esion-srv.service /etc/systemd/system/
// sudo systemctl start esion-srv
// sudo systemctl status esion-srv

#include <iostream>
#include <memory>
#include <tuple>
#include <functional>

#include "DevicePeerWorker.hpp"
#include "OperatorPeerWorker.hpp"
#include "SignalDispatcher.hpp"
#include "WebSocketServer.hpp"
#include "DbFacade.hpp"
#include "OperatorCommands.hpp"
#include "IndexDbFacade.hpp"
#include "Log.hpp"


static const size_t DEFAULT_SERVER_PORT = 20000;
static char DEFAULT_SSL[] = "";
static char DEFAULT_SRV_OPERATOR_LOGIN[] = "esion_operator";
static char DEFAULT_SRV_OPERATOR_PSWD[]  = "esion_operatorpassowrd";
static char DEFAULT_DEVICE_POINT[]       = "^/device?$";
static char DEFAULT_PAGE_POINT[]         = "^/info?$";

static char DEFAULT_DB_ADDRESS[]  = "127.0.0.1";
static char DEFAULT_DB_NAME[]     = "devices";
static char DEFAULT_DB_LOGIN[]    = "esion";
static char DEFAULT_DB_PASSWORD[] = "esionpassword";

static char DEFAULT_INDEX_SERVER_URL[] = "127.0.0.1:3000/index";
static char DEFAULT_INDEX_DB_LOGIN[] = "index";
static char DEFAULT_INDEX_DB_PSWD[] = "Vishen";

static char DEFAULT_YMAP_API_KEY[] = "ad12ff63-587b-42c7-b1e1-e8b8b0913cda";

static char DEFAULT_INDEX_PATH[] = "index";


struct GlobalArgs {
    int port;             ///< параметр -p
    char* ssl_crt;        ///< параметр -c
    char* ssl_key;        ///< параметр -k
    char* srv_ligin;      ///< параметр -q
    char* srv_pswd;       ///< параметр -r
    char* device_point;   ///< параметр -e
    char* page_point;     ///< параметр -w
    char* db_addr;        ///< параметр -a
    char* db_name;        ///< параметр -n
    char* db_ligin;       ///< параметр -l
    char* db_pswd;        ///< параметр -s
    char* yamap_api_key;  ///< параметр -y
    char* index_url;      ///< параметр -u
    char* index_db_login; ///< параметр -m
    char* index_db_pswd;  ///< параметр -t
    char* index_path;   ///< параметр -i
} __global_args;


static const char *__opt_string = "p:k:q:r:c:e:w:a:l:n:s:y:u:m:t:i:h?";
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void HelpMessage() {
    std::cout << "  Use:\n\t#ws-server -p " << DEFAULT_SERVER_PORT << "\n"
              << "  Args:\n"
              << "\t[-p]\t Web socket server port. [" << DEFAULT_SERVER_PORT << "\n"
              << "\t[-c]\t SSL sertificate path.\n"
              << "\t[-k]\t SSL key path.\n"
              << "\t[-q]\t Server operator Login.[" << DEFAULT_SRV_OPERATOR_LOGIN << "]\n"
              << "\t[-r]\t Server operator Password.[" << DEFAULT_SRV_OPERATOR_PSWD << "]\n"
              << "\t[-e]\t Device connection point. [" << DEFAULT_DEVICE_POINT << "]\n"
              << "\t[-w]\t Web page connection point. [" << DEFAULT_PAGE_POINT << "]\n"
              << "\t[-a]\t DB address. [" << DEFAULT_DB_ADDRESS << "]\n"
              << "\t[-n]\t DB name. [" << DEFAULT_DB_NAME << "]\n"
              << "\t[-l]\t DB login. [" << DEFAULT_DB_LOGIN << "]\n"
              << "\t[-s]\t DB password. [" << DEFAULT_DB_PASSWORD << "]\n"
              << "\t[-y]\t Yandex map API key. [" << DEFAULT_YMAP_API_KEY << "]\n"
              << "\t[-u]\t Index server URL. [" << DEFAULT_INDEX_SERVER_URL << "]\n"
              << "\t[-m]\t Index DB login. [" << DEFAULT_INDEX_DB_LOGIN << "]\n"
              << "\t[-t]\t Index DB pswd. [" << DEFAULT_INDEX_DB_PSWD << "]\n"
              << "\t[-i]\t Index DB path. [" << DEFAULT_INDEX_PATH << "]\n"
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
typedef sindex::IndexDbFacade IndexDbFacade;
typedef std::shared_ptr<IndexDbFacade> PIndexDbFacade;
typedef std::shared_ptr<DevicePeerWorker> PDevicePeerWorker;
typedef std::shared_ptr<OperatorPeerWorker> POperatorPeerWorker;
typedef server::BaseCommand BaseCommand;
typedef wsocket::UniWsServer UniWsServer;


int main(int argc_, char **argv_) {
    /// Инициализация globalArgs до начала работы с ней.
    __global_args.port           = DEFAULT_SERVER_PORT;
    __global_args.ssl_crt        = DEFAULT_SSL;
    __global_args.ssl_key        = DEFAULT_SSL;
    __global_args.device_point   = DEFAULT_DEVICE_POINT;
    __global_args.page_point     = DEFAULT_PAGE_POINT;
    __global_args.db_addr        = DEFAULT_DB_ADDRESS;
    __global_args.db_name        = DEFAULT_DB_NAME;
    __global_args.db_ligin       = DEFAULT_DB_LOGIN;
    __global_args.db_pswd        = DEFAULT_DB_PASSWORD;
    __global_args.yamap_api_key  = DEFAULT_YMAP_API_KEY;
    __global_args.index_url      = DEFAULT_INDEX_SERVER_URL;
    __global_args.index_db_login = DEFAULT_INDEX_DB_LOGIN;
    __global_args.index_db_pswd  = DEFAULT_INDEX_DB_PSWD;
    __global_args.index_path     = DEFAULT_INDEX_PATH;

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
            case 'y':
                __global_args.yamap_api_key = optarg;
                break;
            case 'u':
                __global_args.index_url = optarg;
                break;
            case 'm':
                __global_args.index_db_login = optarg;
                break;
            case 't':
                __global_args.index_db_pswd = optarg;
                break;
            case 'i':
                __global_args.index_path = optarg;
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
        ///// Доступ к индексной БД.
        PIndexDbFacade xdb = std::make_shared<IndexDbFacade>(db);
        xdb->init(__global_args.index_path);
        /// Объект синхронизации доступа к общим объектам.
        std::mutex mutex;
        /// Точка подключения устройства.
        PDevicePeerWorker device_pw = std::make_shared<DevicePeerWorker>(mutex, db, __global_args.yamap_api_key);
        /// Точка подключения операторской страницы.
        //BaseCommand::_si_url = __global_args.index_url;
        POperatorPeerWorker oper_pw = std::make_shared<OperatorPeerWorker>(mutex, db, xdb);
        /// Конструирование сервера
        UniWsServer p2p(__global_args.port,
                     __global_args.ssl_crt, 
                     __global_args.ssl_key,
                     std::make_pair(__global_args.device_point, device_pw),
                     std::make_pair(__global_args.page_point, oper_pw));
        return EXIT_SUCCESS;
    }
    LOG(ERROR) << "Check Database parameters.";
    return EXIT_FAILURE;
}

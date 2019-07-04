/**
 * \brief  Вебсокет Сервер для организации работы с поисковым индексом.
 * \author Величко Ростислав
 * \date   24.06.2019
 */

// sudo cp esion-index-srv.service /etc/systemd/system/
// sudo systemctl start esion-index-srv
// sudo systemctl status esion-index-srv

#include <iostream>
#include <memory>
#include <tuple>
#include <functional>

#include "SearchIndexPeerWorker.hpp"
#include "SignalDispatcher.hpp"
#include "WebSocketServer.hpp"
#include "DbFacade.hpp"
#include "IndexDbFacade.hpp"
#include "SearchIndexCommands.hpp"
#include "Log.hpp"


static const size_t DEFAULT_SERVER_PORT = 30000;
static char DEFAULT_INDEX_POINT[] = "^/index?$";
static char DEFAULT_INDEX_PATH[] = "index";
static char DEFAULT_INDEX_DB_LOGIN[] = "index";
static char DEFAULT_INDEX_DB_PSWD[] = "Vishen";

static char DEFAULT_DB_ADDRESS[]  = "127.0.0.1";
static char DEFAULT_DB_NAME[]     = "devices";
static char DEFAULT_DB_LOGIN[]    = "esion";
static char DEFAULT_DB_PSWD[] = "esionpassword";


struct GlobalArgs {
    int port;             ///< параметр -p
    char* index_point;    ///< параметр -i
    char* index_path;     ///< параметр -x
    char* index_db_login; ///< параметр -m
    char* index_db_pswd;  ///< параметр -t
    char* db_addr;        ///< параметр -a
    char* db_name;        ///< параметр -n
    char* db_login;       ///< параметр -l
    char* db_pswd;        ///< параметр -s
} __global_args;


static const char *__opt_string = "p:i:x:m:t:a:n:l:s:h?";
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void HelpMessage() {
    std::cout << "  Use:\n\t#si-server -p " << DEFAULT_SERVER_PORT << "\n"
              << "  Args:\n"
              << "\t[-p]\t Web socket server port. [" << DEFAULT_SERVER_PORT << "\n"
              << "\t[-i]\t Index connection point. [" << DEFAULT_INDEX_POINT << "]\n"
              << "\t[-m]\t Index DB path. [" << DEFAULT_INDEX_PATH << "]\n"
              << "\t[-t]\t Index DB login. [" << DEFAULT_INDEX_DB_LOGIN << "]\n"
              << "\t[-x]\t Index DB pswd. [" << DEFAULT_INDEX_DB_PSWD << "]\n"
              << "\t[-a]\t DB address. [" << DEFAULT_DB_ADDRESS << "]\n"
              << "\t[-n]\t DB name. [" << DEFAULT_DB_NAME << "]\n"
              << "\t[-l]\t DB login. [" << DEFAULT_DB_LOGIN << "]\n"
              << "\t[-s]\t DB password. [" << DEFAULT_DB_PSWD << "]\n"
              << "__________________________________________________________________\n\n";
    exit(EXIT_FAILURE);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


typedef utils::SignalDispatcher SignalDispatcher;
typedef std::shared_ptr<SignalDispatcher> PSignalDispatcher;
typedef server::DbFacade DbFacade;
typedef sindex::PDbFacade PDbFacade;
typedef sindex::BaseCommand BaseCommand;
typedef sindex::IndexDbFacade IndexDbFacade;
typedef sindex::PIndexDbFacade PIndexDbFacade;
typedef sindex::SearchIndexPeerWorker SearchIndexPeerWorker;
typedef std::shared_ptr<SearchIndexPeerWorker> PSearchIndexPeerWorker;
typedef wsocket::UniWsServer UniWsServer;


int main(int argc_, char **argv_) {
    /// Инициализация globalArgs до начала работы с ней.
    __global_args.port           = DEFAULT_SERVER_PORT;
    __global_args.index_point    = DEFAULT_INDEX_POINT;
    __global_args.index_path     = DEFAULT_INDEX_PATH;
    __global_args.index_db_login = DEFAULT_INDEX_DB_LOGIN;
    __global_args.index_db_pswd  = DEFAULT_INDEX_DB_PSWD;
    __global_args.db_addr        = DEFAULT_DB_ADDRESS;
    __global_args.db_name        = DEFAULT_DB_NAME;
    __global_args.db_login       = DEFAULT_DB_LOGIN;
    __global_args.db_pswd        = DEFAULT_DB_PSWD;

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
            case 'i':
                __global_args.index_point = optarg;
                break;
            case 'x':
                __global_args.index_path = optarg;
                break;
            case 'm':
                __global_args.index_db_login = optarg;
                break;
            case 't':
                __global_args.index_db_pswd = optarg;
                break;
            case 'a':
                __global_args.db_addr = optarg;
                break;
            case 'n':
                __global_args.db_name = optarg;
                break;
            case 'l':
                __global_args.db_login = optarg;
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
    PDbFacade db = std::make_shared<DbFacade>();
    if (db->connect(__global_args.db_addr, __global_args.db_name, __global_args.db_login, __global_args.db_pswd)) {
        /// Доступ к индексной БД.
        PIndexDbFacade xdb = std::make_shared<IndexDbFacade>(db);
        xdb->init(__global_args.index_path);
        /// Объект синхронизации доступа к общим объектам.
        std::mutex mutex;
        /// Точка подключения для запросов индексов.
        BaseCommand::_login = __global_args.index_db_login;
        BaseCommand::_pswd = __global_args.index_db_pswd;
        PSearchIndexPeerWorker index_pw = std::make_shared<SearchIndexPeerWorker>(mutex, xdb);
        /// Конструирование сервера
        UniWsServer p2p(__global_args.port, "", "", std::make_pair(__global_args.index_point, index_pw));
        return EXIT_SUCCESS;
    }
    LOG(ERROR) << "Check Database parameters.";
    return EXIT_FAILURE;
}

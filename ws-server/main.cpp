/**
 * \brief  Вебсокет Сервер для организации работы счётчиков.
 * \author Величко Ростислав
 * \date   16.09.2018
 */

#include <iostream>
#include <memory>
#include <tuple>
#include <functional>

#include "ws_server.hpp"
#include "SignalDispatcher.hpp"
#include "RoomController.hpp"
#include "WebSocketServer.hpp"
#include "Log.hpp"


static const size_t DEFAULT_SERVER_PORT = 20000;
static char DEFAULT_SSL[] = "";
static char DEFAULT_DEVICE_POINT[] = "^/rest/device?$";
static char DEFAULT_PAGE_POINT[] = "^/rest/page?$";


struct GlobalArgs {
    int port;       /// параметр -p
    char* ssl_crt;  /// параметр -c
    char* ssl_key;  /// параметр -k
    char* device_point; /// параметр -e
    char* page_point;   /// параметр -w
} __global_args;


static const char *__opt_string = "p:k:c:e:w:h?";
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



void HelpMessage() {
    std::cout << "  Use:\n\t#ws-server -p " << DEFAULT_SERVER_PORT << "\n"
              << "  Args:\n"
              << "\t[-p]\t Web socket server port. Default: " << DEFAULT_SERVER_PORT << "\n"
              << "\t[-c]\t SSL sertificate path.\n"
              << "\t[-k]\t SSL key path.\n"
              << "\t[-e]\t Device connection point. Default: " << DEFAULT_DEVICE_POINT << "\n"
              << "\t[-w]\t Web page connection point. Default: " << DEFAULT_PAGE_POINT << "\n"
              << "__________________________________________________________________\n\n";
    exit(EXIT_FAILURE);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


typedef utils::SignalDispatcher SignalDispatcher;
typedef std::shared_ptr<SignalDispatcher> PSignalDispatcher;
typedef device::SingleRoomController SingleRoomController;
typedef device::PSingleRoomController PSingleRoomController;
typedef device::DevicePeerWorker DevicePeerWorker;
typedef device::ClientPagePeerWorker ClientPagePeerWorker;
typedef std::shared_ptr<DevicePeerWorker> PDevicePeerWorker;
typedef std::shared_ptr<ClientPagePeerWorker> PClientPagePeerWorker;
typedef wsocket::WSServer WSServer;


int main(int argc_, char **argv_) {
    /// Инициализация globalArgs до начала работы с ней.
    __global_args.port = DEFAULT_SERVER_PORT;    
    __global_args.ssl_crt = DEFAULT_SSL;
    __global_args.ssl_key = DEFAULT_SSL;
    __global_args.device_point = DEFAULT_DEVICE_POINT;
    __global_args.page_point = DEFAULT_PAGE_POINT;

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

            case 'h':
            case '?':
                HelpMessage();
                break;

            default: break;
        }
        opt = getopt(argc_, argv_, __opt_string);
    }
    LOG_TO_STDOUT;
    PSingleRoomController room_controller = std::make_shared<SingleRoomController>(); ///< Контроллер комнат
    std::mutex mutex; ///< Объект синхронизации доступа к общим объектам из разных воркеров и подклю1
    
    /// Точка подключения робота - источника видеочений
    PDevicePeerWorker device_pw = std::make_shared<DevicePeerWorker>(mutex, room_controller);
    /// Точка подключения оператора - потребителя видео
    PClientPagePeerWorker page_pw = std::make_shared<ClientPagePeerWorker>(mutex, room_controller);

    /// Конструирование сервера
    WSServer p2p(__global_args.port, 
                 __global_args.ssl_crt, 
                 __global_args.ssl_key,
                 std::make_pair(__global_args.device_point, device_pw),
                 std::make_pair(__global_args.page_point, page_pw));

    return EXIT_SUCCESS;
}

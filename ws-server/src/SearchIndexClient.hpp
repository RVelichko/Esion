/**
 * \brief  Клиент для подключения к выделенному серверу поискового индекса.
 * \author R.N.Velichko rostislav.vel@gmail.com
 * \date   24.06.2019
 */

#pragma once

#include <string>
#include <memory>
#include <thread>

#include "client_ws.hpp"


namespace sindex {

typedef SimpleWeb::SocketClient<SimpleWeb::WS> WsClient;
typedef WsClient::Connection Connection;
typedef WsClient::Message Message;
typedef WsClient::SendStream SendStream;


struct ClientWorker {
    typedef std::function<void(const std::string&)>               OnMessageFn;
    typedef std::function<void()>                                 OnOpenFn;
    typedef std::function<void(const boost::system::error_code&)> OnErrorFn;
    typedef std::function<void(int, const std::string&)>          OnCloseFn;

    OnMessageFn on_msg_fn;
    OnOpenFn    on_open_fn;
    OnErrorFn   on_err_fn;
    OnCloseFn   on_close_fn;
};



class SearchIndexClient {
    typedef std::shared_ptr<WsClient> PWsClient;
    typedef std::shared_ptr<Connection> PConnection;
    typedef std::shared_ptr<Message> PMessage;
    typedef std::shared_ptr<SendStream> PSendStream;
    typedef std::thread Thread;
    typedef std::unique_ptr<Thread, std::function<void(Thread*)>> PThread;

    PConnection _connection;
    PWsClient _client;
    PThread _thread;
    ClientWorker _cw;

public:
    SearchIndexClient(const ClientWorker &cw,  const std::string &srv_url);
    virtual ~SearchIndexClient();

    void send(const std::string &msg);
};
} /// sindex

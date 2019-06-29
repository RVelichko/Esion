#include <functional>

#include "Log.hpp"
#include "OperatorCommands.hpp"
#include "OperatorPeerWorker.hpp"

using namespace server;
namespace ph = std::placeholders;

typedef std::lock_guard<std::mutex> LockQuard;


bool OperatorPeerWorker::parseMessage(const std::string &msg, const ConcreteFn &fn) try {
    static std::mutex m;
    Json json;
    { ///< LOCK Розобрать полученную строку в json.
        LockQuard l(m);
        json = Json::parse(msg);
    };
    fn(json);
    return true;
} catch(std::exception &e) {
    LOG(ERROR) << "Can`t parse recieved json: " << e.what();
    return false;
}


PConnectionValue OperatorPeerWorker::firstMessage(size_t connection_id, const std::string &msg) {
    LOG(DEBUG) << "conid = " << connection_id << "; " << msg;
    PConnectionValue con_val;
    parseMessage(msg, [&](const Json &json) {
        con_val = std::make_shared<ConnectionValue>();
        auto jcmd = json.value("cmd", Json());
        if (not jcmd.empty() and jcmd.is_object()) {
            auto snd_fn = std::bind(_msg_fn, connection_id, ph::_1, WS_STRING_MESSAGE);
            auto auth = AuthorizeCommand(connection_id, jcmd, _mutex, _db, snd_fn);
            if (auth) {
                auth.execute();
            } else {
                LOG(ERROR) << "First command is`t \"auth\"!";
            }
        } else {
            LOG(ERROR) << "First message is`t command! Must by \"auth\" command!";
        }
    });
    return con_val;
}


bool OperatorPeerWorker::lastMessage(const ConnectionValuesIter &iter, const std::string &msg) {
    size_t connection_id = iter->first;
    LOG(DEBUG) << "conid = " << connection_id << "; " << msg;
    parseMessage(msg, [=](const Json &json) {
        /// Обработка запроса проверки подключения.
        auto ping = json.find("ping");
        if (ping not_eq json.end()) {
            Json jsnd = {{"pong", "pong"}};
            _msg_fn(connection_id, jsnd.dump(), WS_STRING_MESSAGE);
            LOG(DEBUG) << jsnd;
        } else {
            auto jcmd = json.value("cmd", Json());
            if (not jcmd.empty() and jcmd.is_object()) {
                auto snd_fn = std::bind(_msg_fn, connection_id, ph::_1, WS_STRING_MESSAGE);
                if (not BaseCommand::executeByName(connection_id, jcmd, _mutex, _db, snd_fn)) {
                    LOG(ERROR) << "Command is`t exequted!";
                }
            }
        }
    });
    return false;
    //return true; ///< Завершить работу данного оператора.
}


void OperatorPeerWorker::sendClose(size_t connection_id) {
    Json j = {{"cmd", "close"}};
    _msg_fn(connection_id, j.dump(), WS_STRING_MESSAGE);
}


OperatorPeerWorker::OperatorPeerWorker(std::mutex &mutex, const PDbFacade& db)
    : BaseWorker(mutex, db)
{}


OperatorPeerWorker::~OperatorPeerWorker() 
{}

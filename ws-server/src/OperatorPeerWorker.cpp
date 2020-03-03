#include <functional>

#include "Log.hpp"
#include "ReportGenerator.hpp"
#include "OperatorPeerWorker.hpp"


using namespace server;

namespace ph = std::placeholders;

typedef std::lock_guard<std::mutex> LockQuard;


bool OperatorPeerWorker::parseMessage(const std::string &msg, const ConcreteFn &fn) try {
    static std::mutex m;
    Json json;
    if (not msg.empty()) {
        { ///< LOCK Розобрать полученную строку в json.
            LockQuard l(m);
            json = Json::parse(msg);
        };
    } else {
        LOG(TRACE) << "Empty message.";
    }
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
        if (not json.empty()) {
            auto jcmd = json.value("cmd", Json());
            if (not jcmd.empty() and jcmd.is_object()) {
                auto snd_fn = std::bind(_msg_fn, connection_id, ph::_1, WS_STRING_MESSAGE);
                if (not OperatorBaseCommand::executeByName(jcmd, _mutex, snd_fn)) {
                    LOG(ERROR) << "Command is`t exequted!";
                }
            } else {
                LOG(ERROR) << "First message is`t command! Must by \"auth\" command!";
            }
        }
    });
    return con_val;
}


bool OperatorPeerWorker::lastMessage(const ConnectionValuesIter &iter, const std::string &msg) {
    size_t connection_id = iter->first;
    LOG(DEBUG) << "conid = " << connection_id << "; " << msg;
    parseMessage(msg, [=](const Json &json) {
        /// Обработка запроса проверки подключения.
        if (not json.empty()) {
            auto ping = json.find("ping");
            if (ping not_eq json.end()) {
                Json jsnd = {
                    {"pong", "pong"}
                };
                _msg_fn(connection_id, jsnd.dump(), WS_STRING_MESSAGE);
                LOG(DEBUG) << jsnd;
            } else {
                auto jcmd = json.value("cmd", Json());
                if (not jcmd.empty() and jcmd.is_object()) {
                    auto snd_fn = std::bind(_msg_fn, connection_id, ph::_1, WS_STRING_MESSAGE);
                    if (not OperatorBaseCommand::executeByName(jcmd, _mutex, snd_fn)) {
                        LOG(ERROR) << "Command is`t exequted!";
                    }
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


OperatorPeerWorker::OperatorPeerWorker(std::mutex &mutex, const PDbFacade& db, const std::string& reports_path, size_t garb_timer)
    : BaseWorker(mutex) {
    OperatorBaseCommand::_garb_timer = garb_timer;
    OperatorBaseCommand::_db = db;
    ReportGenerator::_reports_path = reports_path;
}


OperatorPeerWorker::~OperatorPeerWorker() 
{}

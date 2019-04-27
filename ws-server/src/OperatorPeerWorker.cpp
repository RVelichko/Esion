#include "Log.hpp"
#include "OperatorPeerWorker.hpp"

using namespace server;

typedef std::lock_guard<std::mutex> LockQuard;


std::string GetVerify(const std::string& login, const std::string& pswd) {
    return login + "." + pswd;
}


bool OperatorPeerWorker::parseMessage(const std::string &msg, const ConcreteFn &fn) try {
    Json json;
    { ///< LOCK Розобрать полученную строку в json.
        LockQuard l(_mutex);
        json = Json::parse(msg);
    };
    std::string login = json.value("login", "");
    std::string pswd = json.value("pswd", "");
    std::string coll = json.value("coll", "");
    if (not coll.empty()) {
        _db->setCollection(coll);
    }
    fn(GetVerify(login, pswd), json);
    return true;
} catch(std::exception &e) {
    LOG(ERROR) << "Can`t parse recieved json: " << e.what();
    return false;
}


PConnectionValue OperatorPeerWorker::firstMessage(size_t connection_id, const std::string &msg) {
    LOG(DEBUG) << "conid = " << connection_id << "; " << msg;
    PConnectionValue con_val;
    parseMessage(msg, [&](const std::string &verify, const Json &json) {
        con_val = std::make_shared<ConnectionValue>();
        /// Если оператор уже подключён - оправить ему команду на отключение
        size_t old_operator_id = BaseWorker::getOperatorConnectionId();
        if (old_operator_id) {
            LOG(DEBUG) << "send to OLD operator: " << old_operator_id << "; \"CLOSE\"";
            Json j  = {{"cmd", "close_old"}};
            _msg_fn(old_operator_id, j.dump(), WS_STRING_MESSAGE);
        }
        BaseWorker::setOperatorConnectionId(connection_id);
        Json j  = {{"msg", "connected"}};
        _msg_fn(connection_id, j.dump(), WS_STRING_MESSAGE);
        LOG(DEBUG) << "send to operator: " << connection_id << ", " << j.dump();
    });
    return con_val;
}


bool OperatorPeerWorker::lastMessage(const ConnectionValuesIter &iter, const std::string &msg) {
    size_t connection_id = iter->first;
    size_t cur_operator_id = BaseWorker::getOperatorConnectionId();
    if (connection_id == cur_operator_id) {
        LOG(DEBUG) << "conid = " << connection_id << "; " << msg;
        parseMessage(msg, [=](const std::string &verify, const Json &json) {
            /// Обработка запроса проверки подключения.
            auto ping = json.value("ping", "");
            if (not ping.empty()) {
                Json jsnd = {{"pong", "pong"}};
                _msg_fn(connection_id, jsnd.dump(), WS_STRING_MESSAGE);
                LOG(DEBUG) << jsnd;
            }
            auto jcmd = json.value("cmd", Json());
            if (not jcmd.empty() and jcmd.is_object()) {
                /// Обработка команды запроса списка устройств из базы.
                auto jget_list = jcmd.value("get_list", Json());
                if (not jget_list.empty() and jget_list.is_object()) {
                    auto jnum = jget_list.value("num", Json());
                    auto jskip = jget_list.value("skip", Json());
                    if (not jnum.empty() and not jskip.empty()) {
                        uint8_t num = static_cast<uint8_t>(jget_list.value("num", 10));
                        uint8_t skip = static_cast<uint8_t>(jget_list.value("skip", 0));
                        LockQuard l(_mutex);
                        if (_db) {
                            auto devs = _db->getDevices(num, skip);
                            Json jsnd = {{"devs", devs}};
                            _msg_fn(connection_id, jsnd.dump(), WS_STRING_MESSAGE);
                            LOG(DEBUG) << jsnd;
                        } else {
                            LOG(FATAL) << "DB is NULL!";
                        }
                    } else {
                        LOG(WARNING) << "Incorrect cmd: [get_list] \"" << jget_list.dump() << "\"";
                    }
                }
            }
        });
        return false;
    }
    return true; ///< Завершить работу данного оператора.
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

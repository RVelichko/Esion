#include <functional>

#include "Log.hpp"
#include "SearchIndexCommands.hpp"
#include "SearchIndexPeerWorker.hpp"

using namespace sindex;
namespace ph = std::placeholders;

typedef std::lock_guard<std::mutex> LockQuard;
typedef sindex::FindDevicesCommand FindDevicesCommand;


bool SearchIndexPeerWorker::parseMessage(const std::string &msg, const ConcreteFn &fn) try {
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


PConnectionValue SearchIndexPeerWorker::firstMessage(size_t connection_id, const std::string &msg) {
    LOG(DEBUG) << "conid = " << connection_id << "; " << msg;
    PConnectionValue con_val;
    parseMessage(msg, [&](const Json &json) {
        con_val = std::make_shared<ConnectionValue>();
        auto jcmd = json.value("cmd", Json());
        if (not jcmd.empty() and jcmd.is_object()) {
            auto snd_fn = std::bind(_msg_fn, connection_id, ph::_1, WS_STRING_MESSAGE);
            auto auth = AuthorizeCommand(jcmd, _xdb, snd_fn);
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


bool SearchIndexPeerWorker::lastMessage(const ConnectionValuesIter &iter, const std::string &msg) {
    size_t connection_id = iter->first;
    LOG(DEBUG) << "conid = " << connection_id << "; " << msg;
    parseMessage(msg, [=](const Json &json) {
        auto jcmd = json.value("cmd", Json());
        if (not jcmd.empty() and jcmd.is_object()) {
            auto snd_fn = std::bind(_msg_fn, connection_id, ph::_1, WS_STRING_MESSAGE);
            if (not BaseCommand::executeByName(jcmd, _xdb, snd_fn)) {
                LOG(ERROR) << "Command is`t exequted!";
            }
        }

    });
    return false;
}


void SearchIndexPeerWorker::sendClose(size_t connection_id) {
    Json j = {{"cmd", "close"}};
    _msg_fn(connection_id, j.dump(), WS_STRING_MESSAGE);
}


SearchIndexPeerWorker::SearchIndexPeerWorker(std::mutex &mutex, const PIndexDbFacade& xdb)
    : Worker(mutex)
    , _xdb(xdb) {
    LOG(DEBUG);
}


SearchIndexPeerWorker::~SearchIndexPeerWorker() {
    LOG(DEBUG);
}

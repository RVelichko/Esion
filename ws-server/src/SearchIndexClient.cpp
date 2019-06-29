#include "Log.hpp"
#include "SearchIndexClient.hpp"


using namespace sindex;


SearchIndexClient::SearchIndexClient(const ClientWorker &cw, const std::string &srv_url)
    : _cw(cw) {
    LOG(DEBUG);
    _client = PWsClient(new WsClient(srv_url));
    _client->onmessage = [&,this](PMessage message) {
        if (_cw.on_msg_fn) {
            _cw.on_msg_fn(message->string());
        }
    };
    _client->onopen = [this]() {
        if (_cw.on_open_fn) {
            _cw.on_open_fn();
        }
    };
    _client->onclose = [this](int status, const std::string &reason) {
        if (_cw.on_close_fn) {
            _cw.on_close_fn(status, reason);
        }
    };
    _client->onerror = [this](const boost::system::error_code &ec) {
        _cw.on_err_fn(ec);
    };
    _thread = PThread(new Thread([&]() {
        _client->start();
    }), [this](Thread *p) {
        if (p) {
            _client->stop();
            p->join();
        }
    });
}


SearchIndexClient::~SearchIndexClient() {
    if (_client) {
        _client->send_close(1000);
    }
    LOG(DEBUG);
}


void SearchIndexClient::send(const std::string &msg) {
    if (_client) {
        auto snd_strm = std::make_shared<SendStream>();
        *snd_strm << msg;
        _client->send(snd_strm, _cw.on_err_fn);
        LOG(DEBUG) << "SND: \"" << msg << "\"";
    }
}


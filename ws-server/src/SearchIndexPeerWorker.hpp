/**
 * \brief  Простой websocket обработчик подключения за индекстыми данными.
 * \author R.N.Velichko rostislav.vel@gmail.com
 * \date   24.06.2019
 */

#pragma once

#include "json.hpp"

#include "WebSocketServer.hpp"
#include "IndexDbFacade.hpp"

namespace sindex {

typedef nlohmann::json Json;
typedef server::DbFacade DbFacade;
typedef std::shared_ptr<DbFacade> PDbFacade;

typedef wsocket::Worker Worker; ///< Класс реализующий необходимые функции обработки данным сервером.
typedef wsocket::ConnectionValue ConnectionValue;
typedef wsocket::PConnectionValue PConnectionValue;
typedef wsocket::ConnectionValuesIter ConnectionValuesIter;
typedef std::shared_ptr<IndexDbFacade> PIndexDbFacade;


/**
 * Клас, обрабатывающий подключения клиентских страниц.
 */
class SearchIndexPeerWorker : public Worker {
    typedef std::function<void(const Json&)> ConcreteFn;

    PIndexDbFacade _xdb;

    virtual bool parseMessage(const std::string &msg, const ConcreteFn &func);
    virtual PConnectionValue firstMessage(size_t connection_id, const std::string &msg);
    virtual bool lastMessage(const ConnectionValuesIter &iter, const std::string &msg);
    virtual void sendClose(size_t connection_id);

public:
    SearchIndexPeerWorker(std::mutex &mutex, const PIndexDbFacade& xdb);
    virtual ~SearchIndexPeerWorker();
};
} /// sindex

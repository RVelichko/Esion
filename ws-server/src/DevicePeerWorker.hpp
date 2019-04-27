/**
 * \brief  Простой websocket обработчик подключения устройства к комнате 1x1 связи контроллера с веб страницей состояний счётчиков.
 * \author R.N.Velichko rostislav.vel@gmail.com
 * \date   17.09.2018
 */

#pragma once

#include "BaseWorker.hpp"

namespace server {


/**
 * Клас, обрабатывающий подключения от устройства.
 */
class DevicePeerWorker : public BaseWorker {
    virtual bool parseMessage(const std::string &msg, const ConcreteFn &func);
    virtual PConnectionValue firstMessage(size_t connection_id, const std::string &msg);
    virtual bool lastMessage(const ConnectionValuesIter &iter, const std::string &msg);
    virtual void sendClose(size_t connection_id);

public:
    DevicePeerWorker(std::mutex &mutex, const PDbFacade& db);
    virtual ~DevicePeerWorker();
};
} /// server

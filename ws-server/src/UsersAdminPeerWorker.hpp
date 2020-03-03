/**
 * \brief  Websocket обработчик подключения админстратора пользователей сервиса обслуживания устройств.
 * \author R.N.Velichko rostislav.vel@gmail.com
 * \date   12.09.2019
 */

#pragma once

#include "UsersAdminCommands.hpp"
#include "BaseWorker.hpp"

namespace server {

/**
 * Клас, обрабатывающий подключения для управления владельцами групп устройств.
 */
class UsersAdminPeerWorker : public BaseWorker {
    virtual bool parseMessage(const std::string &msg, const ConcreteFn &func);
    virtual PConnectionValue firstMessage(size_t connection_id, const std::string &msg);
    virtual bool lastMessage(const ConnectionValuesIter &iter, const std::string &msg);
    virtual void sendClose(size_t connection_id);

public:
    UsersAdminPeerWorker(std::mutex &mutex, const PDbFacade& db, size_t garb_timer);
    virtual ~UsersAdminPeerWorker();
};
} /// server

/**
 * \brief  Базовый класс websocket сервера для обслуживания комнаты 1x1 связи контроллера с веб страницей состояний счётчиков.
 * \author R.N.Velichko rostislav.vel@gmail.com
 * \date   17.09.2018
 */

#pragma once


#include "json.hpp"
#include "WebSocketServer.hpp"

namespace server {

typedef nlohmann::json Json;

typedef wsocket::Worker Worker; ///< Класс реализующий необходимые функции обработки данным сервером.
typedef wsocket::ConnectionValue ConnectionValue;
typedef wsocket::PConnectionValue PConnectionValue;
typedef wsocket::ConnectionValuesIter ConnectionValuesIter;

/**
 * Базовый клас, предоставляющий обобщённый набор функций
 */
class BaseWorker : public Worker {
protected:
    typedef std::function<void(const Json&)> ConcreteFn;
    
    virtual bool parseMessage(const std::string &msg, const ConcreteFn &func) = 0;
    virtual void sendClose(size_t connection_id) = 0;

    virtual void onError(size_t connection_id, const boost::system::error_code &ec);
    virtual void onClose(size_t connection_id, int status, const std::string &reason);

    //void setOperatorConnectionId(size_t connection_id);
    //size_t getOperatorConnectionId();

public:
    explicit BaseWorker(std::mutex &mutex);
    virtual ~BaseWorker();
};
} /// server

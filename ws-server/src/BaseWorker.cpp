#include "BaseWorker.hpp"
#include "Log.hpp"


using namespace server;

typedef std::lock_guard<std::mutex> LockQuard;


size_t BaseWorker::_operator_connection_id = 0;


void BaseWorker::setOperatorConnectionId(size_t connection_id) {
    { ///< LOCK
        LockQuard l(_mutex);
        BaseWorker::_operator_connection_id = connection_id;
    }
}


size_t BaseWorker::getOperatorConnectionId() {
    size_t operator_connection_id = 0;
    { ///< LOCK
        LockQuard l(_mutex);
        operator_connection_id = BaseWorker::_operator_connection_id;
    }
    return operator_connection_id;
}


void BaseWorker::onError(size_t connection_id, const boost::system::error_code &ec) {
    sendClose(connection_id);
    Worker::onError(connection_id, ec);
}


void BaseWorker::onClose(size_t connection_id, int status, const std::string &reason) {
    sendClose(connection_id);
    Worker::onClose(connection_id, status, reason);
}


BaseWorker::BaseWorker(std::mutex &mutex, const PDbFacade& db)
    : Worker(mutex)
    , _db(db)
{}


BaseWorker::~BaseWorker() 
{}

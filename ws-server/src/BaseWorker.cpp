#include "BaseWorker.hpp"
#include "Log.hpp"


using namespace server;

typedef std::lock_guard<std::mutex> LockQuard;


void BaseWorker::onError(size_t connection_id, const boost::system::error_code &ec) {
    sendClose(connection_id);
    Worker::onError(connection_id, ec);
}


void BaseWorker::onClose(size_t connection_id, int status, const std::string &reason) {
    sendClose(connection_id);
    Worker::onClose(connection_id, status, reason);
}


BaseWorker::BaseWorker(std::mutex &mutex)
    : Worker(mutex)
{}


BaseWorker::~BaseWorker() 
{}

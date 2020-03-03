#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>
#include <thread>
#include <string>

#include "exceptions.hpp"
#include "non_copyable.h"
#include "srv_tools.h"
#include "http_request.h"

namespace Network
{
  
  DECLARE_RUNTIME_EXCEPTION(HttpServer)

  class HttpServer final : private utils::NonCopyable<HttpServer> {
  public:
    typedef std::vector<IHttpRequest::Type> MethodPool;
    typedef std::function<void (IHttpRequestPtr)> OnRequestFunc;

    enum {
        MaxHeaderSize = static_cast<std::size_t>(-1),
        MaxBodySize = MaxHeaderSize
    };

    HttpServer(std::string const &address, std::uint16_t port,
               std::uint16_t threadCount, OnRequestFunc const &onRequest,
               MethodPool const &allowedMethods = {IHttpRequest::Type::GET },
               std::size_t maxHeadersSize = MaxHeaderSize,
               std::size_t maxBodySize = MaxBodySize);

  private:
    typedef std::function<void(std::thread*)> ThreadDeleter;
    typedef std::unique_ptr<std::thread, ThreadDeleter> ThreadPtr;
    typedef std::vector<ThreadPtr> ThreadPool;

    volatile bool IsRun = true;
    ThreadPool Threads;
    Common::BoolFlagInvertor RunFlag;
  };
  
}

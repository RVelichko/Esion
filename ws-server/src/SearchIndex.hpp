/**
 * \brief  Клиент для подключения к выделенному серверу поискового индекса.
 * \author R.N.Velichko rostislav.vel@gmail.com
 * \date   24.06.2019
 */

#pragma once

#include <string>
#include <memory>
#include <thread>
#include <functional>
#include <atomic>
#include <mutex>


namespace sindex {


class SearchIndex {
    typedef std::thread Thread;
    typedef std::unique_ptr<Thread, std::function<void(Thread*)>> PThread;

    std::mutex _m;
    PThread _th;

public:
    SearchIndex(const std::string &index_path);
    virtual ~SearchIndex();

    void start();
    void stop();

};
} /// sindex

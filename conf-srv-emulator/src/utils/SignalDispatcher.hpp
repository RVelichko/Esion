/*!
 * \brief  Диспетчер сигналов для управления подписавшимися объектами.
 * \author R.N.Velichko rostislav.vel@gmail.com
 * \date   18.10.2013
 */

#pragma once

#include <string>
#include <cstddef>
#include <memory>
#include <functional>
#include <condition_variable>
#include <csignal>
#include <vector>


namespace utils {

class SignalDispatcherResources {
public:
    typedef std::function<void()> Handle;
    typedef std::vector<Handle> Handlers;
    typedef std::condition_variable Condition;
    typedef std::mutex Mutex;

protected:
    static Handlers _resources;
    static Condition _wait_condition;
    static Mutex _wait_mutex;
};


class SignalDispatcher
    : public SignalDispatcherResources
    , public std::enable_shared_from_this<SignalDispatcher> {
    //typedef std::function<void(int)> SignalHandle;

    static void onSignal(int) {
        _wait_condition.notify_all();
        for (auto res : _resources) {
            res();
        }
    }

public:
    template<class ... Args>
    SignalDispatcher(const Args&... args) {
        _resources = Handlers({args...});
        std::signal(SIGINT, &SignalDispatcher::onSignal);
        std::unique_lock<Mutex> lock(_wait_mutex);
        _wait_condition.wait(lock);
    }
    
    virtual ~SignalDispatcher() {
        stop();
    }
    
    void stop() {
        std::raise(SIGTERM);
    }
};
} // utils

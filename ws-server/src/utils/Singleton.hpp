/**
 * \brief  Потокозащищённая обёртка для уникального объекта.
 * \author R.N.Velichko rostislav.vel@gmail.com
 * \date   02.03.2013
 */

#pragma once

#include <memory>
#include <thread>
#include <mutex>

#include "NonCopyable.hpp"


namespace utils {

template<class Type>
class Singleton : private NonCopyable<Singleton<Type>>
{
public:
  static Type* get() {
    static std::shared_ptr<Type> _only_one_instance;
    static std::mutex _mutex;

    if (not _only_one_instance.get()) {
      std::lock_guard<std::mutex> lock(_mutex);

      if (not _only_one_instance) {
        _only_one_instance.reset(new Type());
      }
    }
    return _only_one_instance.get();
  }
};
} /// utils

/**
 * \brief  Идеоматический класс, запрещающий копирование.
 * \author R.N.Velichko rostislav.vel@gmail.com
 * \date   16.09.2018
 */

/// Пример использования: class CantCopy : private NonCopyable<CantCopy> {};


namespace utils {

template<class T>
class NonCopyable {
protected:
    NonCopyable() {}
    ~NonCopyable() {} /// Protected non-virtual destructor
    
private: 
    NonCopyable (const NonCopyable &);
    NonCopyable & operator = (const NonCopyable &);
};
} /// utils

#include <sys/stat.h>
#include <dirent.h>

#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <thread>

#ifdef _WIN32
#include <sys/timeb.h>
#include <sys/types.h>
#include <winsock2.h>

#include <WinBase.h>
#include <stddef.h>

/// from linux's sys/times.h
#define __need_clock_t
#include <time.h>

/// Structure describing CPU time used by a process and its children.
struct tms {
    clock_t tms_utime;  ///< User CPU time.
    clock_t tms_stime;  ///< System CPU time.
    clock_t tms_cutime; ///< User CPU time of dead children.
    clock_t tms_cstime; ///< System CPU time of dead children.
};

typedef long long suseconds_t;

int gettimeofday(struct timeval* t, void* timezone) {
    struct _timeb timebuffer;
    _ftime(&timebuffer);
    t->tv_sec = static_cast<long>(timebuffer.time);
    t->tv_usec = 1000 * timebuffer.millitm;
    return 0;
}

/**
 * Store the CPU time used by this process and all its
 * dead children (and their dead children) in BUFFER.
 * Return the elapsed real time, or (clock_t) -1 for errors.
 * All times are in CLK_TCKths of a second.
 */
clock_t times(struct tms* __buffer) {
    __buffer->tms_utime = clock();
    __buffer->tms_stime = 0;
    __buffer->tms_cstime = 0;
    __buffer->tms_cutime = 0;
    return __buffer->tms_utime;
}
#else
#include "sys/time.h"
#endif


#include "Log.hpp"


typedef struct timeval Timeval;

using namespace utils;

#ifndef LOG_MAX_QUEUE_SIZE
#define LOG_MAX_QUEUE_SIZE (std::numeric_limits<std::uint8_t>::max)()
#endif


#ifndef LOG_FILE_PATH
#define LOG_FILE_PATH "out"
#endif


namespace utils {
struct TimevalWrap {
    Timeval tv;
};
} // namespace utils

typedef utils::TimevalWrap TimevalWrap;
typedef utils::PTimevalWrap PTimevalWrap;
typedef std::unique_lock<std::mutex> UniqueLock;
typedef std::lock_guard<std::mutex> LockQuard;


std::string level_name(const Log::Level& value) {
    switch (value) {
    case TEST:
        return "TEST";
    case DEBUG:
        return "DEBUG";
    case TRACE:
        return "TRACE";
    case INFO:
        return "INFO";
    case WARNING:
        return "WARNING";
    case ERROR:
        return "ERROR";
    case FATAL:
        return "FATAL";
    default:
        throw std::runtime_error("LOD ERROR: Udeclared level");
    }
}


static std::string TimevalToStr(PTimevalWrap tvw, const std::string& format_ = "%d.%m.%Y-%H:%M:%S",
                                bool out_usecs = true) {
    const time_t t = tvw->tv.tv_sec;
    struct tm* ptm = localtime(&t);
    char buf[32] = {0};
    /// Format: Mo, 15.06.2009 20:20:00
    strftime(buf, 32, format_.c_str(), ptm);
    if (out_usecs) {
        return std::string(buf) + "." + std::to_string(tvw->tv.tv_usec / 1000);
    }
    return std::string(buf);
}


static std::string LogFileName(size_t i) {
    PTimevalWrap tvw = std::make_shared<TimevalWrap>();
    gettimeofday(&tvw->tv, NULL);
    return (LOG_FILE_PATH + TimevalToStr(tvw, "-[%d.%m.%Y-%H.%M]-", false) + std::to_string(i) + ".log");
}


Log::Log()
    : _file_number(0)
    , _file_size((std::numeric_limits<size_t>::max)())
    , _file_line_number(0)
    , _is_run(false)
    , _is_log_out(false)
    , _is_log_out_file(true)
    , _log_file_depth(LOG_FILE_DEPTH) {
    for (size_t l = 0; l < static_cast<size_t>(Level::_quantity); ++l) {
        _toggle_levels[l] = true;
    }
    start();
}


Log::~Log() {
    stop();
    std::cout << "Log is stopped.\n" << std::flush;
    _ext_out_func = std::move(std::function<void(const std::string&)>());
}


void Log::init(bool is_log_out, bool is_log_out_file, size_t log_file_depth) {
    _is_log_out = is_log_out;
    _is_log_out_file = is_log_out_file;
    _log_file_depth = log_file_depth;
}


void Log::initExtFunc(ExtOutFunc&& ext_out_func_, bool full_out_) {
    _ext_out_func = std::move(ext_out_func_);
    _full_out_to_ext_func = full_out_;
}


void Log::execute() {
    while (true) {
        QueueTask task;
        { ///< LOCK
            UniqueLock l(_mutex);
            while (_queue.empty()) {
                if (not _is_run) {
                    close();
                    return;
                }
                _cond.wait(l);
            }
            task = _queue.front();
            _queue.pop();
        }
        Level level = std::get<0>(task);
        std::string module = std::get<1>(task);
        std::string message = std::get<2>(task);
        PTimevalWrap tvw = std::get<3>(task);
        std::stringstream ss;
        ss << (++_file_line_number) << ". [" << TimevalToStr(tvw) << "]"
           << " [" << level_name(level) << "]"
           << " [" << module << "]";
        if (_is_log_out) {
            if (_ext_out_func) {
                if (_full_out_to_ext_func) {
                    _ext_out_func((ss.str() + " " + message).c_str());
                } else {
                    _ext_out_func(("[" + level_name(level) + "] " + message).c_str());
                }
            }
#ifdef _WIN32
            ::OutputDebugStringA((ss.str() + " " + message).c_str());
#else
            std::cout << ss.str() << " " << message << "\n" << std::flush;
#endif
        }
        if (_is_log_out_file) {
            if (_file_size >= _log_file_depth) {
                ++_file_number;
                open();
            }
            if (_file.is_open()) {
                _file << ss.str() << " " << message << "\n" << std::flush;
                _file_size = static_cast<size_t>(_file.tellp());
            }
        }
    }
}


void Log::handleCancel() {
    _cond.notify_one();
}


void Log::open() {
    if (_file.is_open()) {
        close();
    }
    std::string file_name = LogFileName(_file_number);
    _file.open(file_name.c_str());
}


void Log::close() {
    if (_file.is_open()) {
        _file.close();

        auto now = std::chrono::system_clock::now();
        struct dirent* epdf;
        DIR* dpdf = opendir("./");

        if (NULL not_eq dpdf) {
            epdf = readdir(dpdf);
            while (epdf) {
                if (DT_REG == epdf->d_type) {
                    std::string name = epdf->d_name;
                    std::string ext = name.substr();
                    auto pos = name.find(".");

                    if ((pos not_eq std::string::npos) and (".log" == ext)) {
                        struct stat t_stat;
                        stat(name.c_str(), &t_stat);
                        struct tm* file_time = localtime(&t_stat.st_ctime);
                        auto mod = std::chrono::system_clock::from_time_t(mktime(file_time));

                        if (mod + std::chrono::hours(_log_file_depth) < now) {
                            if (0 not_eq remove(name.c_str())) {
                                std::runtime_error("LOG ERROR: can`t remove file `" + name + "`.");
                            }
                        }
                    }
                }
                epdf = readdir(dpdf);
            }
        }
        ++_file_number;
    }
}


void Log::print(const Log::Level& level_, const std::string& module_, const std::string& message_) {
    LockQuard l(_mutex);
    if (_is_run and _toggle_levels[static_cast<size_t>(level_)]) {
        PTimevalWrap tvw = std::make_shared<TimevalWrap>();
        gettimeofday(&tvw->tv, NULL);

        if (_queue.size() < LOG_MAX_QUEUE_SIZE) {
            _queue.push(std::make_tuple(level_, module_, message_, tvw)); // millis));
        } else {
            std::string err_msg
                = "Log max queue size exceeded! " + std::to_string(_queue.size()) + " messages were dropped.";
            Queue q;
            _queue.swap(q);
            _queue.push(std::make_tuple(ERROR, "LOG::print", err_msg, tvw)); // millis));
        }
        _cond.notify_one();
    }
}


void Log::start() {
    if (_thread) {
        stop();
    }
    _is_run = true;
    _thread = std::make_shared<std::thread>(std::bind(&Log::execute, this));
}


void Log::stop() {
    _is_run = false;
    _cond.notify_one();

    if (_thread) {
        _thread->join();
        _thread.reset();
    }
}


void Log::toggle(const Level& level_, bool is_on_) {
    LockQuard lock(_mutex);
    _toggle_levels[static_cast<size_t>(level_)] = is_on_;
}


bool Log::isLogOutFile() const {
    return _is_log_out_file;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


LogSequence::Head::Next::Next(const Next& next_)
    : _stream(next_._stream) {}


LogSequence::Head::Head(const Head& head_)
    : _stream(head_._stream)
    , _level(head_._level)
    , _module(head_._module) {}


LogSequence::LogSequence(const Log::Level& level_, const std::string& module_)
    : _level(level_)
    , _module(module_) {}

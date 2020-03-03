#include "JsonCommand.hpp"

using namespace utils;


std::string JsonCommand::TimeToStr(time_t rawtime) {
    struct tm* tm_info = localtime(&rawtime);
    char buf[TIME_STRING_BUFFER_LEN] = {0};
    strftime(buf, TIME_STRING_BUFFER_LEN, "%d.%m.%Y-%H:%M:%S", tm_info);
    return std::string(buf);
}


size_t JsonCommand::ToNumber(const Json& j, const std::string& k) {
    if (j[k].is_number()) {
        return static_cast<size_t>(j.value(k, 0));
    } else if (j[k].is_string()) {
        if (j[k] == "null") {
            return 0;
        } else {
            return static_cast<size_t>(std::stoul(j.value(k, "0")));
        }
    }
    return static_cast<size_t>(0);
}


std::string JsonCommand::ToString(const Json& j, const std::string& k) {
    if (j[k].is_number_integer()) {
        return std::to_string(j.value(k, 0));
    } else if (j[k].is_number_float()) {
        return std::to_string(j.value(k, 0.0));
    } else if (j[k].is_string()) {
        if (j[k] == "null") {
            return "";
        } else {
            return j.value(k, "");
        }
    }
    return std::string();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


JsonCommand::JsonCommand(const std::string& name)
    : _name(name)
    , _is_corrected(false)
{}


JsonCommand::JsonCommand(const std::string& name, const Json& jsn)
    : _name(name) {
    _is_corrected = set(jsn);
}


JsonCommand::~JsonCommand()
{}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


std::string JsonCommand::getName() {
    return _name;
}


Json JsonCommand::getData() {
    return _jdata;
}


bool JsonCommand::setData(const Json& jdata) {
    if (not jdata.empty()) {
        _jdata = jdata;
        return true;
    }
    return false;
}


Json JsonCommand::get() {
    Json jsn = {
        {"cmd", _name}
    };
    if (not _jdata.empty()) {
        jsn["data"] = _jdata;
    }
    return jsn;
}


bool JsonCommand::set(const Json& jsn) {
    std::string name = jsn.value("name", "");
    if (not name.empty() and _name == name) {
        Json jdata = jsn.value("data", Json());
        if (not jdata.empty()) {
            _jdata = jdata;
        }
        return true;
    }
    return false;
}


JsonCommand::operator bool() {
    return _is_corrected;
}

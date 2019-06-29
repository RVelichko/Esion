#include "JsonCommand.hpp"

using namespace utils;


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

#include <string.h>

#include <cstdlib>
#include <sstream>
#include <fstream>
#include <ctime>
#include <set>
#include <locale>

#include <boost/filesystem.hpp>

#include "Log.hpp"
#include "ReportGenerator.hpp"


using namespace server;
namespace bfs = boost::filesystem;


std::string ReportGenerator::_reports_path;


void EraseOldFiles(time_t timeout) {
    auto bpath = bfs::absolute(bfs::path(ReportGenerator::_reports_path));
    if (bfs::exists(bpath)) {
        std::vector<bfs::path> dels;
        for (bfs::directory_iterator itr(bpath); itr not_eq bfs::directory_iterator(); ++itr) {
            if (bfs::is_regular_file(itr->status())) {
                bfs::path path = itr->path();
                time_t lwt = bfs::last_write_time(path);
                time_t old = std::time(nullptr) - timeout;
                if (lwt < old and path.string().find(".csv") not_eq std::string::npos) {
                    dels.push_back(path);
                }
            }
        }
        for (auto d : dels) {
            bfs::remove(d);
            LOG(DEBUG) << "Remove: " << d.string();
        }
    }
}


std::string TimeToStr(time_t rawtime) {
    struct tm* tm_info = localtime(&rawtime);
    char buf[TIME_STRING_BUFFER_LEN] = {0};
    strftime(buf, TIME_STRING_BUFFER_LEN, "%d.%m.%Y-%H:%M:%S", tm_info);
    return std::string(buf);
}


size_t ToNumber(const Json& j, const std::string& k) {
    if (j[k].is_number()) {
        return static_cast<size_t>(j.value(k, 0));
    } else if (j[k].is_string()) {
        return static_cast<size_t>(std::stoul(j.value(k, "0")));
    }
    return static_cast<size_t>(0);
}


std::string ToString(const Json& j, const std::string& k) {
    if (j[k].is_number()) {
        return std::to_string(j.value(k, 0));
    } else if (j[k].is_number_float()) {
        return std::to_string(j.value(k, 0.0));
    } else if (j[k].is_string()) {
        return j.value(k, "");
    }
    return std::string();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


ReportGenerator::ReportGenerator(const std::string& enc)
    : _encoding(enc) {
    LOG(DEBUG);
}


ReportGenerator::~ReportGenerator() {
    LOG(DEBUG);
}


ReportGenerator::operator bool () {
    return not _result_path.empty();
}


ReportGenerator::operator std::string () {
    return _result_path;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


DevicesReportGenerator::DevicesReportGenerator(const Json& jdevs, const std::string& enc)
    : ReportGenerator(enc) {
    EraseOldFiles(60 * 60 * 12);
    try {
        if (not jdevs.empty() and jdevs.is_array()) {
            if (not _reports_path.empty()) {
                /// Создать имя файла отчёта.
                auto bpath = bfs::absolute(bfs::path(_reports_path));
                if (not bfs::exists(bpath)) {
                    bfs::create_directory(bpath);
                }
                time_t rawtime;
                time(&rawtime);
                std::locale loc;
                std::string lo_enc;
                for(auto elem : enc) {
                   lo_enc += std::tolower(elem, loc);
                }
                std::string file_name = "Devices_" + TimeToStr(rawtime) + "_" + lo_enc + REPORT_FILE_EXTENTION;
                std::string file_name_utf8 = "Devices_" + TimeToStr(rawtime) + "_utf8" + REPORT_FILE_EXTENTION;
                std::string path = bpath.string() + "/" + file_name;
                std::string path_utf8 = bpath.string() + "/" + file_name_utf8;
                std::ofstream ofs(path_utf8.c_str());
                if (ofs.is_open()) {
                    /// Записать отчёт в файл.
                    ofs << "N%,Номер устройства,Адрес,Владелец,GEO,Дата последнего обновления,"
                        << "Тип питания,Напряжение,Описание,Статус,";
                    for (size_t i = 1; i <= 4; ++i) {
                         ofs << "Тип " << i << ",Кубометров " << i << ",Число импульсов " << i << ",Название " << i
                             << ",Единица измерения " << i << "," << "Цена импульса " << i << ",Серийный номер " << i
                             << ",Дата поверки " << i << ",Описание " << i << ",";
                    }
                    ofs << "\n";
                    size_t line_num = 0;
                    for (auto jdev : jdevs) {
                        double lo = 0.0;
                        double la = 0.0;
                        auto jgeo = jdev.find("geo");
                        if (jgeo not_eq jdev.end() and jgeo->is_array() and jgeo->size() == 2) {
                            lo = (*jgeo)[0];
                            la = (*jgeo)[1];
                        } else {
                            LOG(ERROR) << "Incorrect geo.";
                        }
                        ofs << ++line_num << ","
                            << jdev["dev_id"].get<std::string>() << ","
                            << jdev["coll"].get<std::string>() << ","
                            << jdev["user"].get<std::string>() << ","
                            << lo << " " << la << ","
                            << TimeToStr(ToNumber(jdev, "update_time")) << ","
                            << jdev["power_type"].get<std::string>() << ","
                            << ToString(jdev, "voltage") << ","
                            << jdev["desc"].get<std::string>() << ","
                            << jdev["status"].get<std::string>() << ",";
                        auto jcounts = jdev.find("counters");
                        if (jcounts not_eq jdev.end() and jcounts->is_array() and jcounts->size() == 4) {
                            for (size_t i = 0; i < 4; ++i) {
                                size_t count = ToNumber((*jcounts)[i], "count");
                                size_t unit_count = ToNumber((*jcounts)[i], "unit_count");
                                ofs << (*jcounts)[i]["type"].get<std::string>() << ","
                                    << count * unit_count << ","
                                    << count << ","
                                    << (*jcounts)[i]["unit"].get<std::string>() << ","
                                    << (*jcounts)[i]["unit_type"].get<std::string>() << ","
                                    << unit_count << ","
                                    << ToString((*jcounts)[i], "serial") << ","
                                    << ToNumber((*jcounts)[i], "verify_date") << ","
                                    << (*jcounts)[i]["desc"].get<std::string>() << ",";
                            }
                        }
                        ofs << "\n";
                    }
                    if (lo_enc not_eq "utf8" and lo_enc not_eq "utf-8") {
                        std::string sys_cmd = "iconv -f \"UTF8\" -t \"" + _encoding + "\" -o " + path + " " + path_utf8;
                        std::system(sys_cmd.c_str());
                        if (bfs::exists(bfs::path(path))) {
                            bfs::remove(path_utf8);
                            _result_path = file_name;
                        } else {
                            _result_path = file_name_utf8;
                            LOG(ERROR) << "Can`t create report for encoding to \"" << _encoding << "\"";
                        }
                    } else {
                        _result_path = file_name;
                    }
                    LOG(DEBUG) << "Create report file: \"" << _result_path << "\"";
                } else {
                    LOG(ERROR) << "Can`t open report file \"" << path << "\"";
                }
            } else {
                LOG(ERROR) << "Reports path is not set!";
            }
        } else {
            LOG(ERROR) << "Devices list is empty!";
        }
    } catch (const std::exception& e) {
        LOG(ERROR) << "Can`t create report. " << e.what();
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


EventsReportGenerator::EventsReportGenerator(const Json& jevs, const std::string& enc)
    : ReportGenerator(enc) {
    EraseOldFiles(60 * 60 * 24);
    try {
        if (not jevs.empty() and jevs.is_array()) {
            if (not _reports_path.empty()) {
                /// Создать имя файла отчёта.
                auto bpath = bfs::absolute(bfs::path(_reports_path));
                if (not bfs::exists(bpath)) {
                    bfs::create_directory(bpath);
                }
                time_t rawtime;
                time(&rawtime);
                std::locale loc;
                std::string lo_enc;
                for(auto elem : enc) {
                   lo_enc += std::tolower(elem, loc);
                }
                std::string file_name = "Events_" + TimeToStr(rawtime) + "_" + lo_enc + REPORT_FILE_EXTENTION;
                std::string file_name_utf8 = "Events_" + TimeToStr(rawtime) + "_utf8" + REPORT_FILE_EXTENTION;
                std::string path = bpath.string() + "/" + file_name;
                std::string path_utf8 = bpath.string() + "/" + file_name_utf8;
                std::ofstream ofs(path_utf8.c_str());
                if (ofs.is_open()) {
                    /// Записать отчёт в файл.
                    ofs << "N%,Номер события,Номер устройства,Адрес,Владелец,GEO,Дата события,"
                        << "Приоритет,Описание,";
                    ofs << "\n";
                    size_t line_num = 0;
                    for (auto jev : jevs) {
                        double lo = 0.0;
                        double la = 0.0;
                        auto jgeo = jev.find("geo");
                        if (jgeo not_eq jev.end() and jgeo->is_array() and jgeo->size() == 2) {
                            lo = (*jgeo)[0];
                            la = (*jgeo)[1];
                        } else {
                            LOG(ERROR) << "Incorrect geo.";
                        }
                        ofs << ++line_num << ","
                            << jev["ev_id"].get<std::string>() << ","
                            << jev["dev_id"].get<std::string>() << ","
                            << jev["coll"].get<std::string>() << ","
                            << jev["user"].get<std::string>() << ","
                            << lo << " " << la << ","
                            << TimeToStr(ToNumber(jev, "time")) << ","
                            << jev["priority"].get<std::string>() << ","
                            << jev["desc"].get<std::string>() << ",";
                        ofs << "\n";
                    }
                    if (lo_enc not_eq "utf8" and lo_enc not_eq "utf-8") {
                        std::string sys_cmd = "iconv -f \"UTF8\" -t \"" + _encoding + "\" -o " + path + " " + path_utf8;
                        std::system(sys_cmd.c_str());
                        if (bfs::exists(bfs::path(path))) {
                            bfs::remove(path_utf8);
                            _result_path = file_name;
                        } else {
                            _result_path = file_name_utf8;
                            LOG(ERROR) << "Can`t create report for encoding to \"" << _encoding << "\"";
                        }
                    } else {
                        _result_path = file_name_utf8;
                    }
                    LOG(DEBUG) << "Create report file: \"" << _result_path << "\"";
                } else {
                    LOG(ERROR) << "Can`t open report file \"" << path << "\"";
                }
            } else {
                LOG(ERROR) << "Reports path is not set!";
            }
        } else {
            LOG(ERROR) << "Events list is empty!";
        }
    } catch (const std::exception& e) {
        LOG(ERROR) << "Can`t create report. " << e.what();
    }
}

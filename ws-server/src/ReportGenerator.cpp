#include <string.h>

#include <cstdlib>
#include <sstream>
#include <fstream>
#include <ctime>
#include <set>
#include <locale>
#include <algorithm>

#include <boost/filesystem.hpp>

#include "Log.hpp"
#include "ReportGenerator.hpp"


using namespace server;
namespace bfs = boost::filesystem;

typedef utils::JsonCommand Jcom;


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


std::string GetProgressResponce(const std::string &cmd_name, double progress) {
    Json resp = {
        {"resp", {
            {"name", cmd_name},
            {"status", "ok"},
            {"progress", progress}
        }}
    };
    return resp.dump();
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


DevicesReportGenerator::DevicesReportGenerator(const Json& jdevs, const std::string& enc, const SendFn& snd_fn)
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
                std::string file_name = "Devices_" + Jcom::TimeToStr(rawtime) + "_" + lo_enc + REPORT_FILE_EXTENTION;
                std::string file_name_utf8 = "Devices_" + Jcom::TimeToStr(rawtime) + "_utf8" + REPORT_FILE_EXTENTION;
                std::string path = bpath.string() + "/" + file_name;
                std::string path_utf8 = bpath.string() + "/" + file_name_utf8;
                std::ofstream ofs(path_utf8.c_str());
                if (ofs.is_open()) {
                    /// Записать отчёт в файл.
                    ofs << "N%;Идентификатор устройства;Адрес;Квартира;Владелец;Дата запуска;Дата обновления;"
                        << "Тип питания;Напряжение;Описание;Статус;";
                    ofs << "Канал N%;Тип;Кубометров;Число импульсов;Название;Единица измерения;"
                        << "Цена импульса;Серийный номер;Дата поверки;Описание;";
                    ofs << "\n";
                    size_t line_num = 0;
                    for (size_t n = 0; n < jdevs.size(); ++n) {
                        auto jdev = jdevs[n];
                        auto desc = jdev["desc"].get<std::string>();
                        std::replace(desc.begin(), desc.end(), ';', '|');
                        std::replace(desc.begin(), desc.end(), '"', '`');
                        ofs << ++line_num << ";"
                            << jdev["dev_id"].get<std::string>() << ";"
                            << jdev["coll"].get<std::string>() << ";"
                            << Jcom::ToNumber(jdev, "apmt") << ";"
                            << jdev["user"].get<std::string>() << ";"
                            << Jcom::TimeToStr(Jcom::ToNumber(jdev, "start_time")) << ";"
                            << Jcom::TimeToStr(Jcom::ToNumber(jdev, "update_time")) << ";"
                            << jdev["power_type"].get<std::string>() << ";"
                            << Jcom::ToString(jdev, "voltage") << ";"
                            << jdev["status"].get<std::string>() << ";"
                            << desc << ";";
                        auto jcounts = jdev.find("counters");
                        if (jcounts not_eq jdev.end() and jcounts->is_array() and jcounts->size() == 4) {
                            auto fill_count_fn = [&](size_t i) {
                                ofs << i + 1 << ";";
                                bool is_none = (*jcounts)[i]["type"].get<std::string>() == "none";
                                if (is_none) {
                                    ofs << "none;;;;;;;;;\n";
                                } else {
                                    size_t count = Jcom::ToNumber((*jcounts)[i], "count");
                                    size_t unit_count = Jcom::ToNumber((*jcounts)[i], "unit_count");
                                    ofs << (*jcounts)[i]["type"].get<std::string>() << ";"
                                        << count * unit_count << ";"
                                        << count << ";"
                                        << (*jcounts)[i]["unit"].get<std::string>() << ";"
                                        << (*jcounts)[i]["unit_type"].get<std::string>() << ";"
                                        << unit_count << ";"
                                        << Jcom::ToString((*jcounts)[i], "serial") << ";"
                                        << Jcom::TimeToStr(Jcom::ToNumber((*jcounts)[i], "verify_date")) << ";"
                                        << (*jcounts)[i]["desc"].get<std::string>() << ";"
                                        << "\n";
                                }
                            };
                            fill_count_fn(0);
                            for (size_t i = 1; i < 4; ++i) {
                                ofs << ";;;;;;;;;;;";
                                fill_count_fn(i);
                            }
                        } else {
                            ofs << ";;;;;;;;;;\n";
                        }
                        //if (snd_fn) {
                        //    double p = (100.0 * static_cast<double>(n)) / static_cast<double>(jdevs.size() - 1);
                        //    snd_fn(GetProgressResponce("get_devices_report", p));
                        //}
                    }
                    if (lo_enc not_eq "utf8" and lo_enc not_eq "utf-8") {
                        std::string sys_cmd = "iconv -f utf8 -t " + _encoding + " -o " + path + " " + path_utf8;
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


EventsReportGenerator::EventsReportGenerator(const Json& jevs, const std::string& enc, const SendFn& snd_fn)
    : ReportGenerator(enc) {
    EraseOldFiles(60 * 60 * 24);
    try {
        if (not jevs.empty() and jevs.is_array()) {
            if (not _reports_path.empty()) {
                if (jevs.size()) {
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
                    std::string file_name = "Events_" + Jcom::TimeToStr(rawtime) + "_" + lo_enc + REPORT_FILE_EXTENTION;
                    std::string file_name_utf8 = "Events_" + Jcom::TimeToStr(rawtime) + "_utf8" + REPORT_FILE_EXTENTION;
                    std::string path = bpath.string() + "/" + file_name;
                    std::string path_utf8 = bpath.string() + "/" + file_name_utf8;
                    std::ofstream ofs(path_utf8.c_str());
                    if (ofs.is_open()) {
                        /// Записать отчёт в файл.
                        ofs << "N%;Идентификатор события;Идентификатор устройства;Адрес;Квартира;Владелец;Дата события;Приоритет;Описание;"
                            << "\n" << std::flush;
                        size_t line_num = 0;
                        for (size_t n =0; n < jevs.size(); ++n) {
                            auto jev = jevs[n];
                            auto desc = jev["desc"].get<std::string>();
                            std::replace(desc.begin(), desc.end(), ';', '|');
                            std::replace(desc.begin(), desc.end(), '"', '`');
                            ofs << ++line_num << ","
                                << jev["ev_id"].get<std::string>() << ";"
                                << jev["dev_id"].get<std::string>() << ";"
                                << jev["coll"].get<std::string>() << ";"
                                << Jcom::ToNumber(jev, "apmt") << ";"
                                << jev["user"].get<std::string>() << ";"
                                << Jcom::TimeToStr(Jcom::ToNumber(jev, "time")) << ";"
                                << jev["priority"].get<std::string>() << ";"
                                << desc << ";";
                            ofs << "\n" << std::flush;
                            //if (snd_fn) {
                            //    double p = (100.0 * static_cast<double>(n)) / static_cast<double>(jevs.size() - 1);
                            //    snd_fn(GetProgressResponce("get_events_report", p));
                            //}
                        }
                        if (lo_enc not_eq "utf8" and lo_enc not_eq "utf-8") {
                            std::string sys_cmd = "iconv -f utf8 -t " + _encoding + " -o " + path + " " + path_utf8;
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
                    LOG(ERROR) << "Empty events array.";
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

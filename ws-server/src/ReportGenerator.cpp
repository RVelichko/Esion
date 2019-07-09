#include <fstream>
#include <ctime>

#include <boost/filesystem.hpp>

#include "Log.hpp"
#include "ReportGenerator.hpp"


using namespace server;
namespace bfs = boost::filesystem;


std::string ReportGenerator::_reports_path;


std::string TimeToStr(time_t rawtime) {
    struct tm* tm_info = localtime(&rawtime);
    char buf[TIME_STRING_BUFFER_LEN] = {0};
    strftime(buf, TIME_STRING_BUFFER_LEN, "%d.%m.%Y-%H:%M:%S", tm_info);
    return std::string(buf);
}


ReportGenerator::ReportGenerator(const Json& jdevs) try {
    LOG(DEBUG);
    if (not jdevs.empty() and jdevs.is_array()) {
        if (not _reports_path.empty()) {
            /// Создать имя файла отчёта.
            auto bpath = bfs::absolute(bfs::path(_reports_path));
            if (not bfs::exists(bpath)) {
                bfs::create_directory(bpath);
            }
            time_t rawtime;
            time(&rawtime);
            std::string file_name = TimeToStr(rawtime) + REPORT_FILE_EXTENTION;
            std::string path = bpath.string() + "/" + file_name;
            /// Записать отчёт в файл.
            std::ofstream ofs(path.c_str());
            ofs << "Id,dev_id,coll,user,geo,update_time,power_type,voltage,desc,status,";
            for (size_t i = 1; i <= 4; ++i) {
                ofs << "type_" << i << ",count_" << i << ",unit_" << i << ",unit_type_" << i << ","
                    << "unit_count_" << i << ",serial_" << i << ",verify_date_" << i << ",desc_" << i << ",";
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
                    << jdev["dev_id"] << ","
                    << jdev["coll"] << ","
                    << jdev["user"] << ","
                    << lo << " " << la << ","
                    << TimeToStr(jdev["update_time"]) << ","
                    << jdev["power_type"] << ","
                    << jdev["voltage"] << ","
                    << jdev["desc"] << ","
                    << jdev["status"] << ",";
                auto jcounts = jdev.find("counters");
                if (jcounts not_eq jdev.end() and
                    jcounts->is_array() and jcounts->size() == 4) {
                    for (size_t i = 0; i < 4; ++i) {
                        ofs << (*jcounts)[i]["type"] << ","
                            << (*jcounts)[i]["count"] << ","
                            << (*jcounts)[i]["unit"] << ","
                            << (*jcounts)[i]["unit_type"] << ","
                            << (*jcounts)[i]["unit_count"] << ","
                            << (*jcounts)[i]["serial"] << ","
                            << (*jcounts)[i]["verify_date"] << ","
                            << (*jcounts)[i]["desc"] << ",";
                    }
                }
                ofs << "\n";
            }
            _result_path = file_name;
            LOG(DEBUG) << "Create report file: \"" << _result_path << "\"";
        } else {
            LOG(ERROR) << "Reports path is not set!";
        }
    }
} catch (const std::exception& e) {
    LOG(ERROR) << "Can`t create report. Bad JSON format.";
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

#include <fstream>
#include <ctime>

#include <boost/filesystem.hpp>

#include "Log.hpp"
#include "ReportGenerator.hpp"


using namespace server;


std::string ReportGenerator::_report_path;


ReportGenerator::ReportGenerator(const Json& jdevs) try {
    LOG(DEBUG);
    if (not jdevs.empty() and jdevs.is_array()) {
        /// Создать имя файла отчёта.
        time_t rawtime;
        time(&rawtime);
        struct tm* tm_info = localtime(&rawtime);
        char buf[TIME_STRING_BUFFER_LEN] = {0};
        strftime(buf, TIME_STRING_BUFFER_LEN, "%d.%m.%Y-%H:%M:%S", tm_info);
        _result_path = _report_path + "/" + buf + REPORT_FILE_EXTENTION;
        /// Записать отчёт в файл.
        std::ofstream ofs(_result_path.c_str());
        ofs << "Id;dev_id;coll;user;geo;update_time;power_type;voltage;desc;status;";
        for (size_t i = 0; i < 4; ++i) {
            ofs << "type_" << i << ";count_" << i << ";unit_" << i << ";unit_type_" << i << ";"
                << "unit_count_" << i << ";serial_" << i << ";verify_date_" << i << ";desc_" << i << ";";
        }
        ofs << "\n";
        size_t line_num = 0;
        for (auto jdev : jdevs) {
            auto dev_id = jdev.value("dev_id", "");
            auto coll   = jdev.value("coll", "");
            auto user   = jdev.value("user", "");
            double lo = 0.0;
            double la = 0.0;
            auto jgeo = jdev.value("geo", Json());
            if (jgeo.is_array() and jgeo.size() == 2) {
                lo = jgeo[0];
                la = jgeo[1];
            }
            auto update_time = jdev.value("update_time", "");
            auto power_type  = jdev.value("power_type", "");
            auto voltage     = jdev.value("voltage", "");
            auto desc        = jdev.value("desc", "");
            auto status      = jdev.value("status", "");
            auto jcounters    = jdev.value("counters", Json());
            ofs << line_num << ";" << dev_id << ";" << coll << ";" << user << ";" << lo << " " << la << ";"
                << update_time << ";" << power_type << ";" << voltage << ";" << desc << ";" << status << ";";
            for (size_t i = 0; i < 4; ++i) {
                std::string type;
                std::string count;
                std::string unit;
                std::string unit_type;
                std::string unit_count;
                std::string serial;
                std::string verify_date;
                std::string desc;
                if (jcounters.is_array() and jcounters.size() == 4) {
                    type        = jcounters[i].value("type", "");
                    count       = jcounters[i].value("count", "");
                    unit        = jcounters[i].value("unit", "");
                    unit_type   = jcounters[i].value("unit_type", "");
                    unit_count  = jcounters[i].value("unit_count", "");
                    serial      = jcounters[i].value("serial", "");
                    verify_date = jcounters[i].value("verify_date", "");
                    desc        = jcounters[i].value("desc", "");
                }
                ofs << type << ";" << count << ";" << unit << ";" << unit_type << ";" << unit_count << ";"
                    << serial << ";" << verify_date << ";" << desc << ";";
            }
            ofs << "\n";
        }
        LOG(DEBUG) << "Create report file: \"" << _result_path << "\"";
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

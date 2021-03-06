/**
 * \brief  Генератор отчётов.
 * \author R.N.Velichko rostislav.vel@gmail.com
 * \date   04.07.2019
 */

#include "json.hpp"
#include "JsonCommand.hpp"

namespace server {

typedef nlohmann::json Json;
typedef std::function<void(const std::string&)> SendFn;

static const char REPORT_FILE_EXTENTION[] = ".csv";


/*!
 * \brief Класс генерации отчёта.
 */
class ReportGenerator {
protected:
    std::string _encoding;
    std::string _result_path;

public:
    static std::string _reports_path;

    ReportGenerator(const std::string& enc);
    virtual ~ReportGenerator();

    /*!
     * \brief Оператор, возвращающий флаг удачного заврешения операции.
     */
    operator bool ();

    /*!
     * \brief Оператор, возвращающий полный путь к новому файлу отчёта.
     */
    operator std::string ();
};


/*!
 * \brief Класс генерации отчёта по устройствам учёта.
 */
class DevicesReportGenerator : public ReportGenerator {
public:
    DevicesReportGenerator(const Json& jdevs, const std::string& enc, const SendFn& snd_fn);
};


/*!
 * \brief Класс генерации отчёта по событиям на устройствах учёта.
 */
class EventsReportGenerator : public ReportGenerator {
public:
    EventsReportGenerator(const Json& jdevs, const std::string& enc, const SendFn& snd_fn);
};
} /// server

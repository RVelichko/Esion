/**
 * \brief  Генератор отчётов.
 * \author R.N.Velichko rostislav.vel@gmail.com
 * \date   04.07.2019
 */


#include "json.hpp"

namespace server {

typedef nlohmann::json Json;

static const char REPORT_FILE_EXTENTION[] = ".csv";
static const size_t TIME_STRING_BUFFER_LEN = 80;


class ReportGenerator {
    std::string _result_path;

public:
    static std::string _report_path;

    ReportGenerator(const Json& jdevs);
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
} /// server

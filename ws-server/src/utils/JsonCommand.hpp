/*!
 * \brief  Класс реализующий паттерн Command в json представлении.
 * \author R.N.Velichko rostislav.vel@gmail.com
 * \date   14.06.2019
 */

#pragma once


#include <string>

#include "json.hpp"


namespace utils {

typedef nlohmann::json Json;


class JsonCommand {
protected:
    std::string _name;
    bool _is_corrected;
    Json _jdata;

public:
    JsonCommand(const std::string& name);
    explicit JsonCommand(const std::string& name, const Json& jsn);
    virtual ~JsonCommand();

    /**
     * Абстрактный метод выполнения команды.
     * \return  Возвращает json c результатом обработки.
     */
    virtual Json execute() = 0;

    /**
     * Метод, возвращающий текущее имя команды.
     * \return  Nмя команды.
     */
    virtual std::string getName();

    /**
     * Метод, возвращающий данные команды в формате json.
     * \return  Данные команды в формате json.
     */
    virtual Json getData();

    /**
     * Метод, добавляет/меняет данные команды в формате json.
     * \param  data  Данные в формате json. "data": < "cmd data str" | {...} >
     * \return  TRUE - в случае успешного выполнения, FALSE - в случае ошибки или некорректоного формата.
     */
    virtual bool setData(const Json& data);

    /**
     * Метод, выполняет сериализацию команды в json представление.
     * \return  Команды в формате json.
     */
    virtual Json get();

    /**
     * Метод, выполняет парсинг команды из json представления.
     * \param  jsn  Команда в формате json. {"cmd": < "cmd name" | {"name":"cmd_name", "data": < "cmd data str" | {...} > } > }
     * \return  TRUE - в случае успешного выполнения, FALSE - в случае ошибки или некорректоного формата.
     */
    virtual bool set(const Json& jsn);

    /**
     * Оператор возвращающий флаг корректной первичной обработки команды.
     * \return  TRUE - в случае корректного определения команды, FALSE - в случае ошибки или некорректоного формата.
     */
    operator bool();
};
} /// utils

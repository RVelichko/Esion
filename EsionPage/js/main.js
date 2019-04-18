test_json = {
    "id":12345678,
    "power_type": "6V",
    "power": "5.9999",
    "counters": [
        {
            "count": 100,
            "type": "test type1",
            "unit": "liter",
            "units_count": 1,
            "max_value": 1000000000,
            "serial_num": 001,
            "desc": "Контроль потребления холодной воды."
        },
        {
            "count": 50,
            "type": "test type2",
            "unit": "liter",
            "units_count": 10,
            "max_value": 1000000000,
            "serial_num": 002,
            "desc": "Контроль потребления горячей воды."
        },
        {"type":"none"},
        {"type":"none"}
    ],
    "desc": "Тестовое описание устройства."
}


/**
 * \brief Глобальные переменные.
 */
window.config = {
    "name": "Control info page",
	"service": {
		"address": "127.0.0.1",
        "_address": "94.127.68.132",
		"port": 20000,
		"rest": "/info",
		"room_id": "2370053276"
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * \brief Функция выполняет подключение к сервису связи с устройствойм.
 */
function ConnectToService() {

}


/**
 * \brief Функция динамически корректирует высоту скролируемой области лога.
 */
function SetScrollLogHeight() {
    var h = 100;
    if ($('#devices_list_container').css('display') == 'none') {
        h = $('#settings_container').height() - $('#log_label').height() - 50;
    } else {
        h = $('#devices_list_container').height() - $('#log_label').height() - 50;
    }
    $('#esion_log').css('max-height', h);
    //console.log('# ' + h);
}


/**
 * \brief Функция динамически корректирует высоту лога.
 */
function SetLogHeight() {
    var h = 100;
    if ($('#devices_list_container').css('display') == 'none') {
        h = $('#settings_container').height();
    } else {
        h = $('#devices_list_container').height();
    }
    $('#log_container').css('height', h);
    //console.log('# ' + h);
}


/**
 * \brief Функция выполняет обновление поля лога.
 */
function UpdateLog() {
    if ($('#log .msg').length > window.log_lines_max_num) {
        $('#log .msg:last').remove();
    }
}


/**
 * \brief Функция добавляет запись, прижатую к левому крфю поля лога.
 */
function AddLeftLog(msg) {
    var tops = {
        year: 'numeric', month: 'numeric', day: 'numeric',
        hour: 'numeric', minute: 'numeric', second: 'numeric'
    };
    $('#log').prepend('<div class=\'msg msg-left\'>' + msg + '</div>');
    UpdateLog();
    SetScrollLogHeight();
}


/**
 * \brief Функция добавляет запись, прижатую к правому крфю поля лога.
 */
function AddRightLog(msg) {
    var tops = {
        year: 'numeric', month: 'numeric', day: 'numeric',
        hour: 'numeric', minute: 'numeric', second: 'numeric'
    };
    var t = new Date();
    $('#log').prepend('<div class=\'msg msg-right\'>' + t.toLocaleString('ru-RU', tops) + " | " + msg + '</div>');
    UpdateLog();
    SetScrollLogHeight();
}
///////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * \brief Функция выполняет заполнение поля счётчика.
 */
function FillCencor(num, jcount) {
    if (jcount !== null) {
        $('#censor_' + num).remove();
        $('#censors').append('<div id=\'censor_' + num + '\' class=\'col-xs-12\'></div>');
        var type = 'none';
        if (typeof jcount['type'] !== 'undefined') {
            type = jcount.type;
        }
        if (type !== 'none') {
            var count = 0;
            if (typeof jcount['count'] !== 'undefined') {
                count = jcount.count;
            }
            var unit = 0;
            if (typeof jcount['unit'] !== 'undefined') {
                unit = jcount.unit;
            }
            var units_count = 0;
            if (typeof jcount['units_count'] !== 'undefined') {
                units_count = jcount.units_count;
            }
            var serial_num = '000';
            if (typeof jcount['serial_num'] !== 'undefined') {
                serial_num = jcount.serial_num;
            }
            var desc = 'Счётчик потребления воды.';
            if (typeof jcount['desc'] !== 'undefined') {
                desc = jcount.desc;
            }
            $('#censor_' + num).append(
                '<div class=\'row col-xs-12\'>' +
                    '<div class=\'row col-xs-4\'>' +
                        '<label style=\'color: green;\'>№ </label>' +
                        '<label style=\'font-size:150%;\'>' + num + '.</label>' +
                        '<label style=\'color: green;\'>[Ипл.]: </label>' +
                        '<label>' + count + '</label>' +
                    '</div>' +
                    '<div class=\'row col-xs-4\'>' +
                        '<label style=\'color: green;\'>[Юнит]: </label>' +
                        '<label>' + unit + '</label>' +
                    '</div>' +
                    '<div class=\'row col-xs-4\'>' +
                        '<label style=\'color: green;\'>[Юнит/Импл.]: </label>' +
                        '<label>' + units_count + '</label>' +
                    '</div>' +
                '</div>' +
                '<div class=\'row col-xs-12\'>' +
                    '<div class=\'row col-xs-3\'' +
                        '<label style=\'color: green;\'>Тип: </label>' +
                        '<label>' + type + '</label>' +
                    '</div>' +
                    '<div class=\'row col-xs-6\'>' +
                        '<label style=\'color: green;\'>Серийный номер:</label>' +
                        '<label>' + serial_num + '</label>' +
                    '</div>' +
                '</div>' +
                '<div class=\'row col-xs-12\'>' +
                    '<label style=\'color: green;\'>Описание:</label>' +
                    '<label>' + desc + '</label>' +
                '</div>');
        } else {
            $('#censor_' + num).append('<label style=\'color: green;\'>№ </label><label >' + num + '.</label><label>Не подключён</label>');
        }
    }
}


function FillCencors(jcensors) {
    if (jcensors.length === 4) {
        FillCencor(1, jcensors[0]);
        $('#cens_line_01').remove();
        $('#censors').append('<div id=\'cens_line_01\' class=\'row col-xs-12\'><hr></div>');
        FillCencor(2, jcensors[1]);
        $('#cens_line_12').remove();
        $('#censors').append('<div id=\'cens_line_12\' class=\'row col-xs-12\'><hr></div>');
        FillCencor(3, jcensors[2]);
        $('#cens_line_23').remove();
        $('#censors').append('<div id=\'cens_line_23\' class=\'row col-xs-12\'><hr></div>');
        FillCencor(4, jcensors[3]);
    }
}


/**
 * \brief Функция выполняет сброс поля счётчика.
 */
function ClearCencor(num) {
    $('#censor_' + num).remove();
    $('#censors').append('<div id=\'censor_' + num + '\' class=\'col-xs-12\'></div>');
    $('#censor_' + num).append('<label style=\'color: green;\'>№ </label><label >' + num + '.</label><label>Не подключён</label>');
}


function ClearCencors() {
    ClearCencor(1);
    $('#cens_line_01').remove();
    $('#censors').append('<div id=\'cens_line_01\' class=\'row col-xs-12\'><hr></div>');
    ClearCencor(2);
    $('#cens_line_12').remove();
    $('#censors').append('<div id=\'cens_line_12\' class=\'row col-xs-12\'><hr></div>');
    ClearCencor(3);
    $('#cens_line_23').remove();
    $('#censors').append('<div id=\'cens_line_23\' class=\'row col-xs-12\'><hr></div>');
    ClearCencor(4);
}


function ParseData(date) {
    var day = date.getDate();
    var month = date.getMonth();
    var year = date.getFullYear();
    var hours = date.getHours();
    var minutes = date.getMinutes();
    var seconds = date.getSeconds();
    var hstr = hours.toString();
    if (hours < 10) {
        hstr = '0' + hours.toString();
    }
    var mstr = minutes.toString();
    if (minutes < 10) {
        var mstr = '0' + minutes.toString();
    }
    var sstr = minutes.toString();
    if (seconds < 10) {
        var sstr = '0' + seconds.toString();
    }
    return day + '.' + month + '.' + year + ' ' + hstr + ':' + mstr + ':' + sstr;
}


/**
 * \brief Функция выполняет добавление краткой забиси об активном устройстве.
 */
function ShowDevice(json) {
    console.log(json);
    if (typeof json['id'] !== 'undefined') {
        $('#device_id_label').text(json.id);
    } else {
        $('#device_id_label').text('000000000000');
    }
    if (typeof json['time'] !== 'undefined') {
        var date = new Date();
        date.setTime(parseInt(json.time) * 1000);
        $('#update_time_label').text(ParseData(date));
    } else {
        $('#update_time_label').text('00.00.00 00:00:00');
    }
    if (typeof json['power_type'] !== 'undefined') {
        $('#battery_type_label').text(json.power_type);
    } else {
        $('#battery_type_label').text('none');
    }
    if (typeof json['power'] !== 'undefined') {
        $('#battery_label').text(json.power + 'V');
    } else {
        $('#battery_label').text('0V');
    }
    if (typeof json['counters'] !== 'undefined') {
        FillCencors(json.counters);
    } else {
        ClearCencors();
    }
    $('#devices_list_container').hide();
    $('#settings_container').show();
    SetLogHeight();
    SetScrollLogHeight();
}


function AddDeviceListLine(json) {
    console.log(JSON.stringify(json));
    var desc = 'Контроллер без адреса.';
    if (typeof json['desc'] !== 'undefined') {
        desc = json['desc'];
    };
    if (typeof json['id'] !== 'undefined') {
        var id = json['id'];
        $('#devices_list').append(
            '<div id=\'show_device_' + id + '\' class=\'row col-xs-12\' json=' + json + '>' +
                '<div class=\'row col-xs-1 vcenter\'>' +
                    '<button id=\'show_' + id + '_bt\' class=\'btn btn-success\' onclick=\'ShowDevice(' + JSON.stringify(json) + ')\'>></button>' +
                '</div>' +
                '<div class=\'row col-xs-2 vcenter\'>' +
                    //'<label style=\'color: green;\'>№ </label>' +
                    '<label style=\'font-size:120%;\'>' + id + '</label>' +
                '</div>' +
                '<div class=\'row col-xs-9 vcenter\'>' +
                    '<label style=\'color: green;\'>:</label>' +
                    '<label>' + desc + '</label>' +
                '</div>' +
            '</div>');
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * \brief Функция выполняет обновление редактируемых полей и меняет конфигураци для заданного поля.
 */
function СhangeSetting(id) {
    var v = $(id).val();
    if (v) {
        $(id).attr('placeholder', v);
        $(id).val('');
    } else {
        v = $(id).attr('placeholder');
    }
    return v;
}


/**
 * \brief Функция выполняет отправку запроса на получение списка устройств.
 */
function GetList(num, skip) {
    if (typeof window.websock !== 'undefined' && window.websock.readyState === 1) {
        var service_login = СhangeSetting('#service_login');
        var service_pswd = СhangeSetting('#service_pswd');
        var collection = СhangeSetting('#collection_name');
        var get_list_cmd = {
            login: service_login,
            pswd: service_pswd,
            coll: collection,
            cmd: {
                get_list: {
                    num: num,
                    skip: skip
                }
            }
        };
        var jstr = JSON.stringify(get_list_cmd);
        window.websock.send(jstr);
        AddRightLog('Snd GET LIST');
        console.log("Send to server: " + jstr);
        window.list_num  = window.list_num + 10;
        window.list_skip = window.list_skip + 10;
    } else {
        console.log('ERR: WebSocket is not opened.');
    }
}

/**
 * \brief Функция инициализирует обслуживание настроек и кнопок.
 */
function HandleSettings() {
    window.list_num = 10;
    window.list_skip = 0;
    /// Установка параметров из конфига.
    var url = window.config.service.address + ":" +
              window.config.service.port +
              window.config.service.rest;
    $("#service_url").attr('placeholder', url);
    var room_id = window.config.service.room_id;
    $("#room_id").attr('placeholder', room_id);

    /// Инициализация обработки нажатия кнопки CONNECT.
    $("#connect").click(function(e) {
        /// Деактивировать кнопку.
        $("#connect").hide();
        var url = "ws://" + СhangeSetting("#service_url");
        AddRightLog("Connecting to url: '" + url + '"');
        console.log("URL: " + url);
        /// Выполнить подключение к сервису.
        window.websock = new WebSocket(url);

        window.websock.onopen = function() {
            var now = new Date();
            var service_login = СhangeSetting('#service_login');
            var service_pswd = СhangeSetting('#service_pswd');
            var collection = СhangeSetting('#collection_name');
            var connect = {
                login: service_login,
                pswd: service_pswd,
                coll: collection,
                msg: "Подключился оператор: " + window.config.name
            };
            var jstr = JSON.stringify(connect);
            window.websock.send(jstr);
            console.log("Send to server: " + jstr);
        };

        window.websock.onerr = function(e) {
            AddLeftLog('ERR: ' + e.message);
            $("#connect").show();
            console.log(e.message);
        };

        window.websock.onmessage = function(e) {
            console.log('recv: ' + e.data);
            var json = JSON.parse(e.data);
            if (json === null) {
                AddLeftLog('Device is not connected yet.');
            } else if (typeof json['cmd'] !== 'undefined') { ///< Обработка команд.
                var cmd = json.cmd;
                if (cmd === 'close_old') {
                    AddLeftLog('Recv CLOSE OLD');
                    $("#connect").show();
                }
            } else if (typeof json['msg'] !== 'undefined') { ///< Обработка сообщений.
                var msg = json.msg;
                if (msg === 'connected') {
                    AddLeftLog('Recv MSG: CONNECTED');
                    GetList(window.list_num, window.list_skip);
                }
            } else if (typeof json['devs'] !== 'undefined') { ///< Обработка списка устройств.
                var devs = json.devs;
                if (devs !== null) {
                    devs.forEach(function(dev) { AddDeviceListLine(dev); });
                    SetLogHeight();
                    SetScrollLogHeight();
                } else {
                    if (10 < window.list_num) {
                        window.list_num  = window.list_num - 10;
                        window.list_skip = window.list_skip - 10;
                    }
                }
            } else if (typeof json['id'] !== 'undefined' && typeof json['counters'] !== 'undefined') { ///< Обработка обновлений с устройств.
                AddLeftLog("Recv device: " + JSON.stringify(json));
                if (typeof json['time'] !== 'undefined') {
                    $("#time_label").text(msg.time);
                }
                if (typeof json['bat'] !== 'undefined') {
                    $("#battery_label").text(msg.bat);
                }
                if (typeof json['counts'] !== 'undefined') {
                    $.each(msg.counts, function(key, count) {
                        $("#sensor_" + key + "_label").text(count);
                    });
                }
            }
        };

        window.websock.onclose = function(e) {
            console.log(e.message);
            AddRightLog('WebSocket is Closed.');
            $("#connect").show();
        };
    });

    $('#next_bt').click(function(e) {
        GetList(window.list_num, window.list_skip);
    });

    $('#show_all_bt').click(function(e) {
        $('#devices_list_container').show();
        $('#settings_container').hide();
        SetLogHeight();
        SetScrollLogHeight();
    });
}
///////////////////////////////////////////////////////////////////////////////////////////////////


$(window).resize(function() {
    SetLogHeight();
    SetScrollLogHeight();
});


$(document).ready(function() {
    /// Инициализация обработки кнопки и полей с параметрами.
    HandleSettings();

    /// Подключение к сервису для логирования текущих данных от устройства по websocket.
    ConnectToService();
    /// Сбросить счётчики.
    //var json = JSON.parse(test_json);
    FillCencors(test_json);

    /// Обновить высоту лога и Скрола для лога.
    SetLogHeight();
    SetScrollLogHeight();
});

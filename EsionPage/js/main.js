/**
 * \brief Глобальные переменные.
 */

window.config = {
    "name": "debug page",
	"service": {
		"address": "127.0.0.1",
		"port": 20000,
		"rest": "/rest/page",
		"room_id": "esion_1"
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
    var h = $('#settings_container').height() - $('#log_label').height() - 50;
    $('#esion_log').css('max-height', h);
    //console.log('# ' + h);
}


/**
 * \brief Функция динамически корректирует высоту лога.
 */
function SetLogHeight() {
    var h = $('#settings_container').height();
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
 * \brief Функция инициализирует обслуживание настроек и кнопок.
 */
function HandleSettings() {
    /// Установка параметров из конфига.
    var url = window.config.service.address + ":" +
              window.config.service.port +
              window.config.service.rest;
    $("#service_url").attr('placeholder', url);
    var room_id = window.config.service.room_id;
    $("#room_id").attr('placeholder', room_id);

    /// Инициализация обработки нажатия кнопки CONNECT.
    $("#connect").click(function() {
        var url = "ws://" + СhangeSetting("#service_url");
        var room_id = СhangeSetting("#room_id");
        AddRightLog("Connecting to url: '" + url + "'; room: '" + window.config.service.room_id + "'");
        console.log("URL: " + url);
        /// Выполнить подключение к сервису.
        window.websock = new WebSocket(url);

        window.websock.onopen = function() {
            var now = new Date();
            var connect = {
                room_id: window.config.service.room_id,
                msg: "Подключился оператор: " + window.config.name
            };
            var jstr = JSON.stringify(connect);
            console.log("Send to server: " + jstr);
            window.websock.send(jstr);
        };

        window.websock.onerr = function(e) {
            console.log(e.message);
        };

        window.websock.onmessage = function(e) {
            console.log("recv: " + e.data);
            var json = JSON.parse(e.data);
            if ("msg" in json) {
                var msg = json.msg;
                /// Обновить информацию об подключённом устройстве.
                AddLeftLog("Recv device: " + JSON.stringify(msg));
                if ("time" in msg) {
                    var tops = {
                        year: 'numeric', month: 'numeric', day: 'numeric',
                        hour: 'numeric', minute: 'numeric', second: 'numeric'
                    };
                    var dt = new Date(msg.time);
                    $("#time_label").text(dt.toLocaleString('ru-RU', tops));
                }
                if ("bat" in msg) {
                    $("#battery_label").text(msg.bat);
                }
                if ("counts" in msg) {
                    $.each(msg.counts, function(key, count) {
                        $("#sensor_" + key + "_label").text(count);
                    });
                }
                /// Проинформировать об отключении устройства.
                if ("disconnected" in msg) {
                    AddLeftLog("Device is disconnected: " + msg.disconnected);
                }
            }
            console.log("Recv: " + JSON.stringify(json));
        };
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

    /// Обновить высоту лога и Скрола для лога.
    SetLogHeight();
    SetScrollLogHeight();
});

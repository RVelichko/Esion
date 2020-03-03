/**
 * \brief Глобальные переменные.
 */
window.config = {
    "name": "Index debugger page",
	"service": {
		"address": "127.0.0.1",
		"port": 30000,
		"rest": "/index",
        "login":"index",
        "pswd":"Vishen"
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////


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


function addDevice() {
    if (typeof window.websock !== 'undefined' && window.websock.readyState === 1) {
        var qstr = СhangeSetting('#id_add_dev');
        var _id = СhangeSetting('#id_add_dev_id');
        var jadd_devs = {
            cmd: {
                name:"add_dev",
                data: {
                    qstr: qstr,
                    _id: _id,
                    token: window.token
                }
            }
        };
        var jstr = JSON.stringify(jadd_devs);
        window.websock.send(jstr);
        AddLeftLog("Send to server: " + jstr);
        console.log("Send to server: " + jstr);
    } else {
        console.log('ERR: WebSocket is not opened.');
    }
}


function findDevices() {
    if (typeof window.websock !== 'undefined' && window.websock.readyState === 1) {
        var qstr = СhangeSetting('#id_find_devs');
        var jfind_devs = {
            cmd: {
                name:"find_devs",
                data: {
                    qstr: qstr,
                    offset: 0,
                    max: 100,
                    token: window.token
                }
            }
        };
        var jstr = JSON.stringify(jfind_devs);
        window.websock.send(jstr);
        AddLeftLog("Send to server: " + jstr);
        console.log("Send to server: " + jstr);
        //window.list_num  = window.list_num + 10;
        //window.list_skip = window.list_skip + 10;
    } else {
        console.log('ERR: WebSocket is not opened.');
    }
}


function addEvent() {
    if (typeof window.websock !== 'undefined' && window.websock.readyState === 1) {
        var qstr = СhangeSetting('#id_add_ev');
        var _id = СhangeSetting('#id_add_ev_id');
        var jadd_evs = {
            cmd: {
                name:"add_ev",
                data: {
                    qstr: qstr,
                    _id: _id,
                    token: window.token
                }
            }
        };
        var jstr = JSON.stringify(jadd_evs);
        window.websock.send(jstr);
        AddLeftLog("Send to server: " + jstr);
        console.log("Send to server: " + jstr);
    } else {
        console.log('ERR: WebSocket is not opened.');
    }
}


function findEvents() {
    if (typeof window.websock !== 'undefined' && window.websock.readyState === 1) {
        var qstr = СhangeSetting('#id_find_evs');
        var jfind_evs = {
            cmd: {
                name:"find_evs",
                data: {
                    qstr: qstr,
                    offest: 0,
                    max: 100,
                    token: window.token
                }
            }
        };
        var jstr = JSON.stringify(jfind_evs);
        window.websock.send(jstr);
        AddLeftLog("Send to server: " + jstr);
        console.log("Send to server: " + jstr);
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

    var url = "ws://" + window.config.service.address + ':' + window.config.service.port + window.config.service.rest;
    AddLeftLog("Start connecting to url: '" + url + '"');
    console.log("URL: " + url);
    /// Выполнить подключение к сервису.
    window.websock = new WebSocket(url);

    window.websock.onopen = function() {
        var now = new Date();
        var auth = {
            cmd: {
                name: "auth",
                data: {
                    login: window.config.service.login,
                    pswd: window.config.service.pswd
                }
            }
        };
        var jstr = JSON.stringify(auth);
        window.websock.send(jstr);
        AddLeftLog('Send to server: ' + jstr);
        console.log('Send to server: ' + jstr);
    };

    window.websock.onerr = function(e) {
        AddLeftLog('ERR: ' + e.message);
        console.log(e.message);
        //clearInterval(window.ping_loop);
    };

    window.websock.onmessage = function(e) {
        console.log('recv: ' + e.data);
        var json = JSON.parse(e.data);
        if (json === null) {
            AddLeftLog('ERR: Responce is NULL.');
        } else if (typeof json['resp'] !== 'undefined') { ///< Обработка команд.
            var resp = json.resp;
            if (typeof resp["status"] !== 'undefined') {
                if (resp.status === 'ok') {
                    if (typeof resp.name !== 'undefined') {
                        var cmd_name = resp.name;
                        if (cmd_name === 'auth') {
                            if (typeof resp['token'] !== 'undefined') {
                                window.token = resp.token;
                                AddRightLog('Connecting to Search Index server! Token is ' + resp.token);
                            } else {
                                AddRightLog('ERR: Reseave undeclared token!');
                            }
                        }
                        if (cmd_name === 'add_dev') {
                            AddRightLog('Device is added to index!');
                        }
                        if (cmd_name === 'find_devs') {
                            if (typeof resp['indexes'] !== 'unefined') {
                                var indexes = resp.indexes;
                                AddRightLog('Devices Indexes: ' + indexes);
                            } else {
                                AddRightLog('ERR: Devices Indexes tag is undeclared!');
                            }
                        }
                        if (cmd_name === 'add_ev') {
                            AddRightLog('Event is added to Index!');
                        }
                        if (cmd_name === 'find_evs') {
                            if (typeof resp['indexes'] !== 'unefined') {
                                var indexes = resp.indexes;
                                AddRightLog('Devices Indexes: ' + indexes);
                            } else {
                                AddRightLog('ERR: Devices Indexes tag is undeclared!');
                            }
                        }
                    } else {
                        AddRightLog('ERR: Undeclared responce command name.');
                    }
                } else if (typeof resp["desc"] !== 'undefined') {
                    if (typeof resp.name !== 'undefined') {
                        var cmd_name = resp.name;
                        AddRightLog('ERR: cmd[' + cmd_name + ']: ' + resp.desc);
                    } else {
                        AddRightLog('ERR: [undefined cmd name]: ' + resp.desc);
                    }
                } else {
                    AddRightLog('ERR: Undefined "decs".');
                }
            } else {
                AddRightLog('ERR: Undefined responce.');
            }
        }
    };

    window.websock.onclose = function(e) {
        console.log(e.message);
        AddRightLog('WebSocket is Closed.');
        $("#connect").show();
    };
}
///////////////////////////////////////////////////////////////////////////////////////////////////


$(window).resize(function() {
    SetLogHeight();
    SetScrollLogHeight();
});


$(document).ready(function() {
    /// Инициализация обработки кнопки и полей с параметрами.
    HandleSettings();

    /// Обновить высоту лога и Скрола для лога.
    SetLogHeight();
    SetScrollLogHeight();
});

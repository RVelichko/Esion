/**
 * \brief Глобальные переменные.
 */
window.config = {
    "name": "Index debugger page",
	"service": {
		"address": "127.0.0.1",
		"port": 20000,
		"rest": "/info",
        "login":"Debug",
        "pswd":"Debug"
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


function getDevices(is_filter) {
    if (typeof window.websock !== 'undefined' && window.websock.readyState === 1) {
        var skip = СhangeSetting('#id_get_devs_skip');
        var num = СhangeSetting('#id_get_devs_num');
        if (is_filter) {
            var filter = СhangeSetting('#id_get_devs_filter');
            var jget_devs = {
                cmd: {
                    name:"get_devs",
                    data: {
                        filter: filter,
                        skip: Number.parseInt(skip),
                        num: Number.parseInt(num),
                        token: window.token
                    }
                }
            };
        } else {
            var jget_devs = {
                cmd: {
                    name:"get_devs",
                    data: {
                        skip: Number.parseInt(skip),
                        num: Number.parseInt(num),
                        token: window.token
                    }
                }
            };
        }
        var jstr = JSON.stringify(jget_devs);
        window.websock.send(jstr);
        AddLeftLog("Send to server: " + jstr);
        console.log("Send to server: " + jstr);
    } else {
        console.log('ERR: WebSocket is not opened.');
    }
}


function getDevicesGeo() {
    if (typeof window.websock !== 'undefined' && window.websock.readyState === 1) {
        var geo_lo = СhangeSetting('#id_get_devs_lo');
        var geo_la = СhangeSetting('#id_get_devs_la');
        var geo_r = СhangeSetting('#id_get_devs_r');
        var skip = СhangeSetting('#id_get_devs_geo_skip');
        var num = СhangeSetting('#id_get_devs_geo_num');
        var jget_devs_geo = {
            cmd: {
                name:"get_devs",
                data: {
                    geo: [Number.parseFloat(geo_lo), Number.parseFloat(geo_la)],
                    radius: Number.parseFloat(geo_r),
                    skip: Number.parseInt(skip),
                    num: Number.parseInt(num),
                    token: window.token
                }
            }
        };
        var jstr = JSON.stringify(jget_devs_geo);
        window.websock.send(jstr);
        AddLeftLog("Send to server: " + jstr);
        console.log("Send to server: " + jstr);
        //window.list_num  = window.list_num + 10;
        //window.list_skip = window.list_skip + 10;
    } else {
        console.log('ERR: WebSocket is not opened.');
    }
}


function getDevicesPoly() {
    if (typeof window.websock !== 'undefined' && window.websock.readyState === 1) {
        var geo_x = СhangeSetting('#id_get_devs_x');
        var geo_y = СhangeSetting('#id_get_devs_y');
        var geo_w = СhangeSetting('#id_get_devs_w');
        var geo_h = СhangeSetting('#id_get_devs_h');
        var skip = СhangeSetting('#id_get_devs_geo_skip');
        var num = СhangeSetting('#id_get_devs_geo_num');
        var jget_devs_poly = {
            cmd: {
                name:"get_devs",
                data: {
                    geo_poly: [Number.parseFloat(geo_x), Number.parseFloat(geo_y), Number.parseFloat(geo_w), Number.parseFloat(geo_h)],
                    skip: Number.parseInt(skip),
                    num: Number.parseInt(num),
                    token: window.token
                }
            }
        };
        var jstr = JSON.stringify(jget_devs_poly);
        window.websock.send(jstr);
        AddLeftLog("Send to server: " + jstr);
        console.log("Send to server: " + jstr);
    } else {
        console.log('ERR: WebSocket is not opened.');
    }
}


function setDevStatus() {
    if (typeof window.websock !== 'undefined' && window.websock.readyState === 1) {
        var dev_id = СhangeSetting('#id_set_dev_status_id');
        var status = СhangeSetting('#id_set_dev_status');
        var jset_status = {
            cmd: {
                name:"set_status",
                data: {
                    dev_id: _id,
                    status: status,
                    token: window.token
                }
            }
        };
        var jstr = JSON.stringify(jset_status);
        window.websock.send(jstr);
        AddLeftLog("Send to server: " + jstr);
        console.log("Send to server: " + jstr);
    } else {
        console.log('ERR: WebSocket is not opened.');
    }
}


function getReport() {
    if (typeof window.websock !== 'undefined' && window.websock.readyState === 1) {
        var get_report = СhangeSetting('#id_get_report');
        var jget_report = {
            cmd: {
                name:"get_report",
                data: {
                    coll: get_report,
                    token: window.token
                }
            }
        };
        var jstr = JSON.stringify(jget_report);
        window.websock.send(jstr);
        AddLeftLog("Send to server: " + jstr);
        console.log("Send to server: " + jstr);
    } else {
        console.log('ERR: WebSocket is not opened.');
    }
}


function getEvents(is_filter) {
    if (typeof window.websock !== 'undefined' && window.websock.readyState === 1) {
        var skip = СhangeSetting('#id_get_evs_skip');
        var num = СhangeSetting('#id_get_evs_num');
        if (is_filter) {
            var filter = СhangeSetting('#id_get_evs_filter');
            var jget_evs = {
                cmd: {
                    name:"get_events",
                    data: {
                        filter: filter,
                        skip: Number.parseInt(skip),
                        num: Number.parseInt(num),
                        token: window.token
                    }
                }
            };
        } else {
            var jget_evs = {
                cmd: {
                    name:"get_events",
                    data: {
                        skip: Number.parseInt(skip),
                        num: Number.parseInt(num),
                        token: window.token
                    }
                }
            };
        }
        var jstr = JSON.stringify(jget_evs);
        window.websock.send(jstr);
        AddLeftLog("Send to server: " + jstr);
        console.log("Send to server: " + jstr);
    } else {
        console.log('ERR: WebSocket is not opened.');
    }
}


function setDevStatus() {
    if (typeof window.websock !== 'undefined' && window.websock.readyState === 1) {
        var dev_id = СhangeSetting('#id_set_dev_status_id');
        var status = СhangeSetting('#id_set_dev_status');
        var jset_dev_status = {
            cmd: {
                name:"set_dev_status",
                data: {
                    dev_id: dev_id,
                    status: status,
                    token: window.token
                }
            }
        };
        var jstr = JSON.stringify(jset_dev_status);
        window.websock.send(jstr);
        AddLeftLog("Send to server: " + jstr);
        console.log("Send to server: " + jstr);
    } else {
        console.log('ERR: WebSocket is not opened.');
    }
}


function logout() {
    var now = new Date();
    var auth = {
        cmd: {
            name: "logout"
        }
    };
    var jstr = JSON.stringify(auth);
    window.websock.send(jstr);
    AddLeftLog('Send to server: ' + jstr);
    console.log('Send to server: ' + jstr);
}


function login() {
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
        login();
    };

    window.websock.onerr = function(e) {
        AddLeftLog('ERR: ' + e.message);
        console.log(e.message);
        //clearInterval(window.ping_loop);
        logout();
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
                                $('#id_token').text(resp.token);
                                $('#bt_log').attr('onclick', 'logout()');
                                $('#bt_log').text('logout');
                                AddRightLog('Connecting to Search Index server! Token is ' + resp.token);
                            } else {
                                AddRightLog('ERR: Reseave undeclared token!');
                            }
                        }
                        if (cmd_name === 'logout') {
                            $('#id_token').text('');
                            $('#bt_log').attr('onclick', 'login()');
                            $('#bt_log').text('login');
                            AddRightLog('Logout complette!');
                        }
                        if (cmd_name === 'get_devs') {
                            if (typeof resp['data'] !== 'unefined') {
                                var data = JSON.stringify(resp.data);
                                AddRightLog('Devices: ' + data);
                            } else {
                                AddRightLog('ERR: Data tag is undeclared!');
                            }
                        }
                        if (cmd_name === 'get_events') {
                            if (typeof resp['data'] !== 'unefined') {
                                var data = JSON.stringify(resp.data);
                                AddRightLog('Events: ' + data);
                            } else {
                                AddRightLog('ERR: Data tag is undeclared!');
                            }
                        }
                        if (cmd_name === 'set_dev_status') {
                            AddRightLog('Device status is set!');
                        }
                        if (cmd_name === 'get_report') {
                            if (typeof resp['report_url'] !== 'unefined') {
                                var report_url = resp.report_url;
                                AddRightLog('Report URL: ' + report_url);
                            } else {
                                AddRightLog('ERR: Can`t find tag of report URL!');
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
        logout();
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

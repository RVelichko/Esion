class Api {
    constructor () {
        this.token = null;
        this.socket = null;
    }

    connect (ws) {
        return new Promise((resolve, reject) => {
            const socket = new WebSocket(ws || '');
            socket.onopen = function() {
                resolve(socket);
            };

            socket.onerror = function(error) {
                reject(error);
            };

            // socket.onclose = function(event) {
            //     if (event.wasClean) {
            //         console.log('Соединение закрыто чисто');
            //     } else {
            //         console.log('Обрыв соединения');
            //     }
            //     console.log('Код: ' + event.code + '. Причина: ' + event.reason);
            // };

            this.socket = socket;
        });
    }

    login ({ user, pswd }) {
        const sendData = {
            cmd: {
                name: 'auth',
                data: { user, pswd }
            }
        };

        this.socket.send(JSON.stringify(sendData));
    }

    getDevices ({ token, skip, num, filter }) {
        const sendData = {
            cmd: {
                name: 'get_devs',
                data: { token, skip, num, filter }
            }
        };

        this.socket.send(JSON.stringify(sendData));
    }

    setToken (token) {
        this.token = token;
    }

    removeToken() {
        this.token = null;
    }
}

export default new Api();

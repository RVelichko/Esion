class WsClient {
    constructor () {
        this._cachedSend = [];
        this._openCallbacks = [];
        this._messageCallbacks = [];
        this._errorCallbacks = [];
        this._closeCallbacks = [];
    }

    connect (url, options) {
        this._url = url;
        this._reconnect = options && options.reconnect || 1000;
        this._debug = options && options.debug;

        const socket = new WebSocket(this._url || '');

        socket.addEventListener('open', () => {
            if (this._debug) {
                console.log('Socket connected');
            }
            this._openCallbacks.forEach(callback => callback());
            const loop = () => {
                if (this._cachedSend.length) {
                    this.send(this._cachedSend.pop());
                    loop();
                }
            };
            loop();
        });

        socket.addEventListener('error', event => {
            if (this._debug) {
                console.log('Socket error', event);
            }
            this._errorCallbacks.forEach(callback => callback(event));
        });

        socket.addEventListener('close', event => {
            if (this._debug) {
                console.log('Socket closed', event);
                console.log('Socket reconnecting...');
            }
            this._closeCallbacks.forEach(callback => callback(event));
            setTimeout(() => { this.connect(url, options); }, this._reconnect);
        });

        socket.addEventListener('message', event => {
            if (this._debug) {
                console.log('Socket message: ' + event.data);
            }
            this._messageCallbacks.forEach(callback => callback(event));
        });

        this.socket = socket;
    }

    send (data) {
        if (this.socket.readyState === WebSocket.OPEN) {
            if (this._debug) {
                console.log('Socket send: ' + data);
            }
            this.socket.send(data);
        } else {
            this._cachedSend.push(data);
        }
    }

    onopen (callback) {
        this._openCallbacks.push(callback);
    }

    onmessage (callback) {
        this._messageCallbacks.push(callback);
    }

    onclose (callback) {
        this._closeCallbacks.push(callback);
    }

    onerror (callback) {
        this._errorCallbacks.push(callback);
    }
}

export default new WsClient();

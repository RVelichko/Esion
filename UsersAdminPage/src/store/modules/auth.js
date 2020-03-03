import socket from '../../api/ws-client'

export default {
    namespaced: true,
    state: {
        token: localStorage.getItem('token'),
        status: '', // loading, success, error
        message: '',
        user: {
            name: localStorage.getItem('user'),
            description: localStorage.getItem('userDescription')
        }
    },
    mutations: {
        setToken (state, token) {
            state.token = token;
        },
        removeToken (state) {
            state.token = '';
        },
        setStatus (state, { status, message }) {
            state.status = status || '';
            state.message = message || '';
        },
        setUser (state, user) {
            state.user = user;
        },
        removeUser (state) {
            state.user.name = '';
            state.user.description = '';
        }
    },
    actions: {
        login ({ commit }, { username, password }) {
            const sendData = {
                cmd: {
                    name: 'auth',
                    data: {
                        login: username,
                        pswd: password
                    }
                }
            };
            socket.send(JSON.stringify(sendData));
            commit('setStatus', { status: 'loading' });
            this._loginTimer = setTimeout(() => {
                commit('setStatus', { status: 'error', message: 'Request timeout' });
            }, 3000);
        },
        logout ({ commit, state }) {
            const sendData = {
                cmd: {
                    name: 'logout',
                    data: {
                        token: state.token
                    }
                }
            };
            socket.send(JSON.stringify(sendData));
            commit('removeToken');
            commit('setStatus', '');
            commit('removeUser');
            localStorage.removeItem('token');
            localStorage.removeItem('user');
            localStorage.removeItem('userDescription');
        },
        onLoginResponse ({ commit }, data) {
            clearTimeout(this._loginTimer);
            if (data.status === 'ok') {
                commit('setToken', data.token);
                commit('setStatus', { status: 'success' });
                let userInfo = data.user_info || {};
                commit('setUser', {
                    name: userInfo.person || '-',
                    description: userInfo.desc || '-'
                });
                localStorage.setItem('token', data.token);
                localStorage.setItem('user', userInfo.person || '-');
                localStorage.setItem('userDescription', userInfo.desc || '-');
            } else {
                commit('removeToken');
                commit('setStatus', { status: 'error', message: data.desc });
                localStorage.removeItem('token');
                localStorage.removeItem('user');
                localStorage.removeItem('userDescription');
            }
        },
        onLogoutResponse ({ commit }, data) {}
    }
}

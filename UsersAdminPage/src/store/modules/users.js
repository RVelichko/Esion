import socket from '../../api/ws-client'
import moment from 'moment';

export default {
    namespaced: true,
    state: {
        search: '',
        users: []
    },
    mutations: {
        setSearch (state, search) {
            state.search = search;
        },
        setUsers (state, users) {
            state.users = users;
        }
    },
    actions: {
        getUsers({ commit, state, rootState }, { filter, skip, num, sort, dateFrom, dateTo, dateType }) {
           const sendData = {
                cmd: {
                    name: 'get_users',
                    data: {
                        token: rootState.auth.token,
                        skip: skip || 0,
                        num: num || 100,
                        sort: {
                            field: sort.field,
                            direction: sort.order
                        }
                    }
                }
            };
            if (filter) {
                sendData.cmd.data.filter = filter;
            }
            if (dateFrom) {
                dateFrom.setHours(0, 0, 0);
                sendData.cmd.data.date_time_from = moment(dateFrom).unix();
            }
            if (dateTo) {
                dateTo.setHours(23, 59, 59);
                sendData.cmd.data.date_time_to = moment(dateTo).unix();
            }
            if ((dateFrom || dateTo) && dateType) {
                sendData.cmd.data.date_type = dateType;
            }
            socket.send(JSON.stringify(sendData));
        },
        getUser({ commit, state, rootState }, { filter, skip, num, sort, dateFrom, dateTo, dateType }) {},
        addUser({ commit, state, rootState }, { filter, skip, num, sort, dateFrom, dateTo, dateType }) {},
        updateUser({ commit, state, rootState }, { filter, skip, num, sort, dateFrom, dateTo, dateType }) {},
        getAddrs({ commit, state, rootState }, { filter, skip, num, sort, dateFrom, dateTo, dateType }) {},
        onGetUsersMessage({ commit, state, rootState }, data) {
            if (data.status === 'ok') {
                commit('setUsers', data.data);
            }
        },
        onGetUserMessage({ commit, state, rootState }, data) {},
        onAddUserMessage({ commit, state, rootState }, data) {},
        onUpdateUserMessage({ commit, state, rootState }, data) {},
        onGetAddrsMessage({ commit, state, rootState }, data) {}
    }
}

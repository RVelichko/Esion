import socket from '../../api/ws-client'
import moment from 'moment';

export default {
    namespaced: true,
    state: {
        search: '',
    },
    mutations: {
        setSearch (state, search) {
            state.search = search;
        },
    },
    actions: {
        getUsersMessage({ commit, state, rootState }, { filter, skip, num, sort, dateFrom, dateTo, dateType }) {},
        getUserMessage({ commit, state, rootState }, { filter, skip, num, sort, dateFrom, dateTo, dateType }) {},
        addUserMessage({ commit, state, rootState }, { filter, skip, num, sort, dateFrom, dateTo, dateType }) {},
        updateUserMessage({ commit, state, rootState }, { filter, skip, num, sort, dateFrom, dateTo, dateType }) {},
        getAddrsMessage({ commit, state, rootState }, { filter, skip, num, sort, dateFrom, dateTo, dateType }) {},
        onGetUsersMessage({ commit, state, rootState }, data) {},
        onGetUserMessage({ commit, state, rootState }, data) {},
        onAddUserMessage({ commit, state, rootState }, data) {},
        onUpdateUserMessage({ commit, state, rootState }, data) {},
        onGetAddrsMessage({ commit, state, rootState }, data) {}
    }
}

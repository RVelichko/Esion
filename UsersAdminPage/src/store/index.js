import Vue from 'vue';
import Vuex from 'vuex';

// modules
import auth from "./modules/auth";
import config from "./modules/config";
import users from "./modules/users";

Vue.use(Vuex);

export default new Vuex.Store({
  modules: {
    auth,
    config,
    users
  },
  state: {
    socket: {
      connected: false
    },
    menuExpanded: false
  },
  mutations: {
    setSocketConnected (state, connected) {
        state.connected = connected;
    },
    setMenuExpanded (state, expanded) {
        state.menuExpanded = expanded;
    }
  },
  actions: {
    onSocketMessage({ dispatch, commit, state }, data) {
      switch (data.name) {
        case 'auth': dispatch('auth/onLoginResponse', data); break;
        case 'get_users': dispatch('users/onGetUsersMessage', data); break;
        case 'get_user': dispatch('users/onGetUserMessage', data); break;
        case 'add_user': dispatch('users/onAddUserMessage', data); break;
        case 'update_user': dispatch('users/onUpdateUserMessage', data); break;
        case 'get_addrs': dispatch('users/onGetAddrsMessage', data); break;
        case 'logout': dispatch('auth/onLogoutResponse', data); break;
        default: console.warn('Unknown response', data);
      }
    }
  }
})

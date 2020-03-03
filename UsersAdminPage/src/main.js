import Vue from 'vue';
import VueMaterial from 'vue-material';
import App from './App.vue';
import router from './router';
import store from './store';
import 'vue-material/dist/vue-material.min.css';
import 'vue-material/dist/theme/default.css';
import wsock from './api/ws-client';

Vue.use(VueMaterial);

Vue.config.productionTip = false;


(async function() {
  try {
    const config = await store.dispatch('config/loadConfig');
    // socket init
    wsock.connect(config.ws, { debug: config.debug });
    wsock.onopen(() => store.commit('setSocketConnected', true));
    wsock.onclose(() => store.commit('setSocketConnected', false));
    wsock.onmessage(event => {
        const data = JSON.parse(event.data);
        if (data.resp.status === 'err' && data.resp.desc.includes('Invalid token')) {
            store.commit('auth/removeToken');
            localStorage.removeItem('token');
        } else {
            return store.dispatch('onSocketMessage', data.resp || {});
        }
    });

    const vm = new Vue({
      router,
      store,
      render: h => h(App)
    }).$mount('#app');

    //vm.$material.locale = locale
  } catch (e) {
    console.error(e);
  }
})();

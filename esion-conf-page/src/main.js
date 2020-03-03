import Vue from 'vue';
import App from './App.vue';

import VueMaterial from 'vue-material';
//import { MdButton, MdContent } from 'vue-material/dist/components'
import 'vue-material/dist/vue-material.min.css';
import 'vue-material/dist/theme/default.css';

//Vue.use(MdButton)
//Vue.use(MdContent)


Vue.use(VueMaterial);

(async function() {
    try {
        const vm = new Vue({
            render: h => h(App)
        }).$mount('#app');
    } catch (e) {
        console.error(e);
    }
})();

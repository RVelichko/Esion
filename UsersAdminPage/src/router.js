import Vue from 'vue'
import Router from 'vue-router'
import Users from './views/Users.vue'

Vue.use(Router)

export default new Router({
  routes: [
    {
        path: '/',
        name: 'users',
        component: Users
    },
    {
        path: '/login',
        name: 'login',
        meta: {
          layout: 'plain'
        },
        // route level code-splitting
        // this generates a separate chunk (about.[hash].js) for this route
        // which is lazy-loaded when the route is visited.
        component: () => import(/* webpackChunkName: "login" */ './views/Login.vue')
    },
    {
        path: '*',
        meta: {
          layout: 'plain'
        },
        component: () => import(/* webpackChunkName: "404" */ './views/404.vue')
  }
  ]
})

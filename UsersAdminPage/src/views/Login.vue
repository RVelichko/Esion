<template>
    <div class="login" @keyup.enter="login">
        <md-content class="md-elevation-3">
            <div class="logo">
                <img src="logo.svg" :alt="title">
            </div>

            <div class="title">
                <div class="md-title" style="margin-bottom: 20px;">{{ title }}</div>
                <div class="md-body-1">{{ description }}</div>
            </div>

            <div class="form">
                <md-field>
                    <label>Логин</label>
                    <md-input v-model="username" autofocus></md-input>
                </md-field>

                <md-field md-has-password>
                    <label>Пароль</label>
                    <md-input v-model="password" type="password"></md-input>
                </md-field>

                <div class="error" v-if="status === 'error'">{{ message }}</div>
            </div>

            <div class="actions md-layout md-alignment-center-center">
                <md-button class="md-raised md-primary" @click="login">Войти</md-button>
            </div>

            <div class="loading-overlay" v-if="status === 'loading'">
                <md-progress-spinner md-mode="indeterminate" :md-stroke="3"></md-progress-spinner>
            </div>
        </md-content>
    </div>
</template>

<script>
    import store from '../store';
    import { mapState } from 'vuex';

    export default {
        name: 'Login',
        beforeRouteEnter: function (to, from, next) {
            if (!store.state.auth.token) {
                next();
            } else {
                next('/');
            }
        },
        data() {
            return {
                username: '',
                password: ''
            };
        },
        watch: {
            status (status) {
                if (status === 'success') {
                    this.$router.push('/');
                }
            }
        },
        computed: {
            ...mapState('config', {
                title: state => state.title,
                description: state => state.description
            }),
            ...mapState('auth', {
                status: state => state.status,
                message: state => state.message
            }),
        },
        methods: {
            login () {
                return this.$store.dispatch('auth/login', {
                    username: this.username,
                    password: this.password,
                });
            }
        }
    };
</script>

<style lang="scss">
    .login {
        display: flex;
        align-items: center;
        justify-content: center;
        position: relative;
        height: 100vh;

        .logo {
            display: block;
            text-align: center;
            margin-bottom: 20px;
        }

        .error {
            font-size: 15px;
            color: #d32f2f;
            text-align: center;
        }

        .title {
            text-align: center;
            margin-bottom: 30px;

            img {
                margin-bottom: 14px;
                max-width: 80px;
            }
        }

        .actions {
            .md-button {
                margin: 0;
            }
        }

        .form {
            margin-bottom: 42px;
        }

        .md-content {
            z-index: 1;
            padding: 40px;
            width: 100%;
            max-width: 400px;
            position: relative;
        }

        .loading-overlay {
            z-index: 10;
            top: 0;
            left: 0;
            right: 0;
            position: absolute;
            width: 100%;
            height: 100%;
            background: rgba(255, 255, 255, 0.9);
            display: flex;
            align-items: center;
            justify-content: center;
        }
    }
</style>

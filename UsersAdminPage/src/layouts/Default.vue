<template>
    <div class="default-layout">
        <md-app>
            <md-app-toolbar class="md-primary" md-elevation="0">
                <md-button class="md-icon-button" @click="toggleMenu" v-if="!menuExpanded">
                    <md-icon>menu</md-icon>
                </md-button>
                <span class="md-title">{{ title }}</span>
            </md-app-toolbar>

            <md-app-drawer :md-active.sync="menuExpanded" md-persistent="mini">
                <md-toolbar class="md-transparent" md-elevation="0">
                    <span>МЕНЮ</span>

                    <div class="md-toolbar-section-end">
                        <md-button class="md-icon-button md-dense" @click="toggleMenu">
                            <md-icon>keyboard_arrow_left</md-icon>
                        </md-button>
                    </div>
                </md-toolbar>

                <md-list id="nav">
                    <md-list-item>
                        <md-icon>person</md-icon>
                        <div class="md-list-item-text">
                            <span>{{ user.name }}</span>
                            <span style="font-size: 12px; color: #999;">{{ user.description }}</span>
                        </div>
                    </md-list-item>

                    <md-list-item to="/" exact>
                        <md-icon>vertical_split</md-icon>
                        <span class="md-list-item-text">Пользователи</span>
                    </md-list-item>

                    <md-list-item @click="logout">
                        <md-icon>exit_to_app</md-icon>
                        <span class="md-list-item-text">Выход</span>
                    </md-list-item>
                </md-list>
            </md-app-drawer>

            <md-app-content>
                <slot />
            </md-app-content>
        </md-app>
    </div>
</template>

<script>
    import { mapState } from 'vuex';

    export default {
        name: 'default',
        mounted () {
            return this.$store.dispatch('users/getUsers', {
                filter: ''
            });
        },
        computed: {
            ...mapState('config', {
                title: state => state.title
            }),
            ...mapState('auth', {
                user: state => state.user
            }),
            menuExpanded: {
                get () {
                    return this.$store.state.menuExpanded;
                },
                set (value) {
                    this.$store.commit('setMenuExpanded', value);
                }
            }
        },
        methods: {
            toggleMenu () {
                this.menuExpanded = !this.menuExpanded
            },
            logout () {
                return this.$store.dispatch('auth/logout').then(() => {
                    this.$router.push('/login');
                });
            }
        }
    }
</script>

<style lang="scss">
    .default-layout {
        .router-link-active {
            background-color: #E1E1E1;

            .md-icon {
                color: #448AFF !important;
            }

            .md-list-item-text {
                font-weight: bold;
                color: #333;
            }
        }

        .md-content {
            background: url(../assets/background.svg) !important;
            background-size: cover !important;;
        }

        .md-app-container {
            overflow: hidden;
        }

        .md-app {
            height: 100vh;
        }

        .md-drawer {
            width: 280px;
            max-width: calc(100vw - 125px);
        }
    }
</style>

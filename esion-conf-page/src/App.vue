<template>
  <div class="page-container md-layout-row">
    <md-app>
      <md-app-toolbar class="md-primary">
        <span class="md-title">ЭСИОН</span>
        <h3 class="md-toolbar-section-end">v 3.1</h3>
      </md-app-toolbar>
      <md-app-drawer md-permanent="full">
        <md-list class="md-double-line md-dense">
            <md-list-item>
            <!-- <md-icon>bookmark</md-icon> -->
            <span class="md-title">ID</span>
            <span class="md-title">{{dev_id}}</span>
            </md-list-item>
        </md-list>
        <md-list>
            <md-list-item @click='showSettings'>
                <!-- <md-icon>settings</md-icon> -->
                <span class="md-list-item-text">Общие настройки</span>
            </md-list-item>
            <md-list-item @click='showCounters'>
                <!-- <md-icon>settings_input_component</md-icon> -->
                <span class="md-list-item-text">Cчётчики</span>
            </md-list-item>
            <!-- <md-list-item @click='showSysSettings'> -->
                <!-- <md-icon>settings_input_component</md-icon> -->
                <!-- <span class="md-list-item-text">Системные настройки</span> -->
            <!-- </md-list-item> -->
            <md-list-item @click='apply'>
                <!-- <md-icon class='md-accent'>done_all</md-icon> -->
                <span class="md-list-item-text">Применить</span>
            </md-list-item>
            <md-list-item @click='exit'>
                <!-- <md-icon>exit_to_app</md-icon> -->
                <span class="md-list-item-text">Выйти</span>
            </md-list-item>
        </md-list>
        <md-list class="md-double-line md-dense">
            <md-list-item>
                <!-- <md-icon>power</md-icon> -->
                <span class="md-title">POWER</span>
                <span class="md-title">{{power_percent}}%</span>
            </md-list-item>
        </md-list>
      </md-app-drawer>
      <md-app-content v-if=!is_exit>
          <settings-tab v-if=toggle_settings v-model='settings_data' @settings-data-event='settingsEvent' />
          <counters-tab v-if=toggle_counters v-model='counters_data' @count-card-event='countEvent' @erase-count-event='eraseCountEvent' />
          <!-- <sys-settings-tab v-if=toggle_sys_settings v-model='sys_settings_data' @sys-settings-event='sysSettingsEvent' @erase-sys-settings-event='eraseSysSettingsEvent' /> -->
          <span class="md-caption">ООО "ИнфоТек-Сервис" (812)34-77-99-8</span>
      </md-app-content>
      <md-app-content v-if=is_exit>
          <span class="md-title">Выход выполнен. Для входа запустите на устройстве конфигурирование и перезагрузите страницу.</span>
      </md-app-content>
    </md-app>
  </div>
</template>

<script>
    import axios from 'axios';
    import CountersTab from './components/CountersTab.vue';
    import SettingsTab from './components/SettingsTab.vue';
    // import SysSettingsTab from './components/SysSettingsTab.vue';
    export default {
        name: 'Esion',
        data() {
            return {
                is_exit: false,
                dev_id: '0000000000',
                toggle_settings: true,
                toggle_counters: false,
                //toggle_sys_settings: false,
                power_percent: 0,
                settings_data: {},
                counters_data: [],
                //sys_settings_data: {}
            };
        },
        mounted: function() {
            axios.get('/app_info').then((resp) => {
                var js = resp.data;
                if (js !== null) {
                    if (typeof js['dev_id'] !== 'undefined') {
                        this.dev_id = js.dev_id;
                    }
                    if (typeof js['power_percent'] !== 'undefined') {
                        this.power_percent = js.power_percent;
                    }
                }
            });
        },
        components: {
            CountersTab,
            SettingsTab,
            // SysSettingsTab
        },
        methods: {
            showSettings() {
                this.toggle_settings = true;
                this.toggle_counters = false;
                //this.toggle_sys_settings = false;
            },
            showCounters() {
                this.toggle_settings = false;
                this.toggle_counters = true;
                //this.toggle_sys_settings = false;
            },
            showSysSettings() {
                this.toggle_settings = false;
                this.toggle_counters = false;
                //this.toggle_sys_settings = true;
            },
            apply() {
                const payload = {
                  settings: this.settings_data,
                  counters: this.counters_data
                };
                const url = '/apply';
                axios.post(url, JSON.stringify(payload));
                console.log('APPLY: ' + url + ': ' + JSON.stringify(payload));
            },
            exit() {
                axios.get('/exit');
                this.is_exit = true;
            },
            settingsEvent(val) {
                this.settings_data[val.k] = val.v;
                console.log('#' + JSON.stringify(this.settings_data));
            },
            countEvent(val) {
                var count = this.counters_data[val.id];
                if (typeof count === 'undefined') {
                    count = {};
                }
                count[val.k] = val.v;
                this.counters_data[val.id] = count;
                console.log('#' + JSON.stringify(this.counters_data));
            },
            //sysSettingsEvent(val) {
            //    this.settings_data[val.k] = val.v;
            //    console.log('#' + JSON.stringify(this.sys_settings_data));
            //},
            eraseCountEvent(val) {
                this.counters_data[val] = {};
                console.log('# erase ' + JSON.stringify(this.counters_data));
            },
            //eraseSysSettingsEvent(val) {
            //    this.sys_settings_data[val] = {};
            //    console.log('# erase ' + JSON.stringify(this.sys_settings_data));
            //}
        }
    }
</script>

<style lang="scss" scoped>
    @font-face {
        font-family: 'Material Icons';
        font-style: normal;
        font-weight: 400;
        src: url(../public/fonts/KFOlCnqEu92Fr1MmEU9fBBc4.woff2) format('woff2');
    }
    @font-face {
        font-family: 'Material Icons';
        font-style: normal;
        font-weight: 400;
        src: url(../public/fonts/KFOlCnqEu92Fr1MmWUlfBBc4.woff2) format('woff2');
    }
    @font-face {
        font-family: 'Material Icons';
        font-style: normal;
        font-weight: 400;
        src: url(../public/fonts/KFOmCnqEu92Fr1Mu4mxK.woff2) format('woff2');
    }
    @font-face {
        font-family: 'Material Icons';
        font-style: normal;
        font-weight: 400;
        src: url(../public/fonts/KFOmCnqEu92Fr1Mu5mxKOzY.woff2) format('woff2');
    }
    @font-face {
        font-family: 'Material Icons';
        font-style: normal;
        font-weight: 400;
        src: url(../public/fonts/flUhRq6tzZclQEJ-Vdg-IuiaDsNc.woff2) format('woff2');
    }
    .material-icons {
        font-family: 'Material Icons';
        font-weight: normal;
        font-style: normal;
        font-size: 24px;
        line-height: 1;
        letter-spacing: normal;
        text-transform: none;
        display: inline-block;
        white-space: nowrap;
        word-wrap: normal;
        direction: ltr;
        -webkit-font-feature-settings: 'liga';
        -webkit-font-smoothing: antialiased;
    }
  .md-app {
    min-height: 350px;
    border: 1px solid rgba(#000, .12);
  }
  .md-drawer {
    width: 230px;
    max-width: calc(100vw - 125px);
  }
  .md-list {
    width: 320px;
    max-width: 100%;
    display: inline-block;
    vertical-align: top;
    border: 1px solid rgba(#000, .12);
  }
</style>

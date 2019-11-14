<template>
    <md-card class="md-layout-item md-size-100 md-small-size-100">
        <!-- <md-list class="md-double-line md-dense">
            <md-list-item>
                <div class="md-layout-item md-size-20">
                    <md-list-item>
                        <label><h2>Дата запуска</h2></label>
                    </md-list-item>
                </div>
                <div class="md-layout-item md-size-80">
                    <md-list-item>
                        <md-datepicker v-model="start_date" v-on:md-closed="$emit('settings-data-event',{k:'start_date',v:(start_date && Number(start_date))/1000 })" />
                    </md-list-item>
                </div>
            </md-list-item>
        </md-list> -->

        <md-list class="md-double-line md-dense">
            <md-list-item>
                <md-field>
                    <label>Тип питания</label>
                    <md-select v-model='power' name='power' id='power' v-on:input="$emit('settings-data-event', {k:'power', v:$event})">
                        <md-option :selected='power === bat_lion' value='bat_lion'>LiOn battary [ 3.8 V ]</md-option>
                        <md-option :selected='power === bat_4aa' value='bat_4aa'>4AA battaries [ 6 V ]</md-option>
                    </md-select>
                    <span class="md-helper-text">Выберите тип питания</span>
                </md-field>
            </md-list-item>
        </md-list>

        <md-list class="md-double-line md-dense">
            <md-list-item>
                <md-field md-clearable>
                    <label>Идентификатор</label>
                    <md-input v-model="wifi_ssid" v-on:input="$emit('settings-data-event', {k:'ssid', v:$event})"></md-input>
                    <span class="md-helper-text">Укажите имя WIFI сети</span>
                </md-field>
            </md-list-item>
        <md-list class="md-double-line md-dense">

        </md-list>
            <md-list-item>
                <md-field>
                    <label>Пароль</label>
                    <md-input v-model="wifi_password" type="password" v-on:input="$emit('settings-data-event', {k:'pswd', v:$event})"></md-input>
                    <span class="md-helper-text">Укажите WIFI пароль</span>
                </md-field>
            </md-list-item>
        </md-list>

        <md-list class="md-double-line md-dense">
            <md-list-item>
                <md-field md-clearable>
                    <label>URL</label>
                    <md-input v-model='cloud_url' v-on:input="$emit('settings-data-event', {k:'url', v:$event})"></md-input>
                    <span class="md-helper-text">Укажите URL сервера обслуживания</span>
                </md-field>
            </md-list-item>
        </md-list>

        <md-list class="md-double-line md-dense">
            <md-list-item>
                <md-field md-clearable>
                    <label>Владелец устройства</label>
                    <md-input v-model="user" v-on:input="$emit('settings-data-event', {k:'user', v:$event})"></md-input>
                    <span class='md-helper-text'>Укажите ФИО</span>
                </md-field>
            </md-list-item>
        </md-list>

        <md-list class="md-double-line md-dense">
            <md-list-item>
                <md-field md-clearable>
                    <label>Идентификатор УК</label>
                    <md-input v-model="coll_id" v-on:input="$emit('settings-data-event', {k:'coll_id', v:$event})"></md-input>
                    <span class='md-helper-text'>Укажите идентификатор управляющей компании</span>
                </md-field>
            </md-list-item>
        </md-list>

        <md-list class="md-double-line md-dense">
            <md-list-item>
                <div class="md-layout-item md-size-29">
                    <md-list-item>
                        <md-field>
                            <label>Квартира</label>
                            <md-input v-model='apmt' type="number" min=1 max=1000 v-on:input="$emit('settings-data-event', {k:'apmt', v:$event})"></md-input>
                            <span class="md-helper-text">Укажите номер квартиры</span>
                        </md-field>
                    </md-list-item>
                </div>
                <div class="md-layout-item md-size-70">
                    <md-list-item>
                        <md-field md-clearable>
                            <label>Адрес</label>
                            <md-input v-model='address' v-on:input="$emit('settings-data-event', {k:'address', v:$event})"></md-input>
                            <span class="md-helper-text">Укажите полный адрес расположения</span>
                        </md-field>
                    </md-list-item>
                </div>
            </md-list-item>
        </md-list>

        <md-field>
            <md-divider></md-divider>
            <label>Описание</label>
            <md-textarea v-model='description' v-on:input="$emit('settings-data-event', {k:'desc', v:$event})"></md-textarea>
        </md-field>

        <md-list class="md-double-line md-dense">
            <md-list-item>
                <md-field md-clearable>
                    <label>Таймаут контрольной отправки [час.]</label>
                    <md-input v-model="ctrl_timeout" type="number" min=10 max=200 v-on:input="$emit('settings-data-event', {k:'ctrl_timeout', v:$event})"></md-input>
                    <span class='md-helper-text'>Укажите количество часов м/у контрольными отправками</span>
                </md-field>
            </md-list-item>
        </md-list>

        <md-list class="md-double-line md-dense">
            <md-list-item>
                <md-field md-clearable>
                    <label>Минимальное количество кубометров для фиксации</label>
                    <md-input v-model="min_mcubs" type="number" min=1 max=100 v-on:input="$emit('settings-data-event', {k:'min_mcubs', v:$event})"></md-input>
                    <span class='md-helper-text'>Укажите минимальное количество кубометров для фиксации на сервере</span>
                </md-field>
            </md-list-item>
        </md-list>

        <md-list class="md-double-line md-dense">
            <md-list-item>
                <md-field md-clearable>
                    <label>Задержка перед отправкой [сек.]</label>
                    <md-input v-model="snd_timeout" type="number" min=1 max=100 v-on:input="$emit('settings-data-event', {k:'snd_timeout', v:$event})"></md-input>
                    <span class='md-helper-text'>Укажите задержку в секундах перед отправкой даных</span>
                </md-field>
            </md-list-item>
        </md-list>

        <md-list class="md-double-line md-dense">
            <md-list-item>
                <md-field md-clearable>
                    <label>Продолжительность конфигурирования [мин.]</label>
                    <md-input v-model="cfg_time" type="number" min=5 max=60 v-on:input="$emit('settings-data-event', {k:'cfg_time', v:$event})"></md-input>
                    <span class='md-helper-text'>Укажите максимальную продолжительность режима конфигурирования в минутах</span>
                </md-field>
            </md-list-item>
        </md-list>
    </md-card>
</template>

<script>
    import axios from 'axios';

    export default {
        name: 'SettingsTab',
        data: () => ({
            power:'',
            cloud_url: 'esion.ru',
            wifi_ssid: '',
            wifi_password: '',
            user: '',
            coll_id: 'Debug collection',
            address: '', // Полный адрес.
            apmt: 0, // Номер квартиры.
            //start_date: 0,
            description: '',
            ctrl_timeout: 72,
            min_mcubs: 1,
            snd_timeout: 10,
            cfg_time: 20
        }),
        mounted: function() {
            axios.get('/settings_info').then((resp) => {
                var js = resp.data;
                if (js !== null) {
                    if (typeof js['cloud_url'] !== 'undefined') {
                        this.cloud_url = js.cloud_url;
                    }
                    if (typeof js['ssid'] !== 'undefined') {
                        this.wifi_ssid = js.ssid;
                    }
                    if (typeof js['pswd'] !== 'undefined') {
                        this.wifi_password = js.pswd;
                    }
                    if (typeof js['user'] !== 'undefined') {
                        this.user = js.user;
                    }
                    if (typeof js['coll_id'] !== 'undefined') {
                        this.coll_id = js.coll_id;
                    }
                    if (typeof js['address'] !== 'undefined') {
                        this.address = js.address;
                    }
                    if (typeof js['apmt'] !== 'undefined') {
                        this.apmt = js.apmt;
                    }
                    if (typeof js['desc'] !== 'undefined') {
                        this.description = js.desc;
                    }
                    if (typeof js['power'] !== 'undefined') {
                        this.power = js.power;
                    }
                    if (typeof js['ctrl_timeout'] !== 'undefined') {
                        this.ctrl_timeout = js.ctrl_timeout;
                    }
                    if (typeof js['min_mcubs'] !== 'undefined') {
                        this.min_mcubs = js.min_mcubs;
                    }
                    if (typeof js['snd_timeout'] !== 'undefined') {
                        this.snd_timeout = js.snd_timeout;
                    }
                    if (typeof js['cfg_time'] !== 'undefined') {
                        this.cfg_time = js.cfg_time;
                    }
                }
            });
        },
    }
</script>

<template>
  <div class='users_information'>
    <md-table v-model="users" md-card class="md-layout-item md-size-100 md-small-size-100">
      <md-table-toolbar>
        <h1 class="md-title">Пользователи сервиса обслуживания устройств</h1>
      </md-table-toolbar>
      <md-table-row slot="md-table-row" slot-scope="{ item }">
        <md-table-cell md-label="ID" md-sort-by="id" md-numeric>{{ item.id }}</md-table-cell>
        <md-table-cell md-label="Название" md-sort-by="name">{{ item.comp_name }}</md-table-cell>
        <md-table-cell md-label="ФИО" md-sort-by="email">{{ item.user_name }}</md-table-cell>
        <md-table-cell md-label="Логин" md-sort-by="gender">{{ item.user_login }}</md-table-cell>
        <md-table-cell md-label="Телефон" md-sort-by="title">{{ item.tephone }}</md-table-cell>
        <md-table-cell md-label="Описание" md-sort-by="title">{{ item.desc }}</md-table-cell>
        <md-table-cell md-label="Создан" md-sort-by="title">{{ item.date_from }}</md-table-cell>
        <md-table-cell md-label="Обновлён" md-sort-by="title">{{ item.date_to }}</md-table-cell>
        <md-button v-if=!old_inited class="md-icon-button md-raised md-accent" @click='delUser' >
            <md-icon>clear</md-icon>
        </md-button>
      </md-table-row>
    </md-table>
  </div>
</template>

<script>
    import store from '../store';
    import moment from 'moment';
    import { mapState } from 'vuex';

    export default {
        name: 'UsersInforamtion',
        data: () => ({
            search: '',
            sort: {
                field: 'coll_id',
                direction: 'asc'
            },
            users_inf: []
        }),
        computed: {
            ...mapState('users', {
                users: state => state.users,
            })
        },
        created () {
            this.$store.dispatch('users/getUsers', {
                       filter: this.search, // this.search ? `"${ this.search }"` : '',
                       skip: this.$store.state.config.skip,
                       num: this.$store.state.config.skip,
                       sort: this.sort,
                       dateFrom: this.dateFilter.date_from,
                       dateTo: this.dateFilter.date_to,
                       dateType: this.dateFilter.date_type
                   }).then();
        },
        methods: {
            delUser() {
                console.log('ADD USER');
            }
        }
    }
</script>

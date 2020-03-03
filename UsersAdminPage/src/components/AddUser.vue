<template>
    <div class="add_user">
        <md-card class="md-layout-item md-size-100 md-small-size-100">
        <md-card-header>
            <div class="md-layout-item md-small-size-100">
                <md-list class="md-double-line md-dense">
                    <md-list-item>
                        <span class="md-title">Добавить ответственного пользователя от УК</span>
                        <md-button class="md-fab md-mini md-primary" @click='addUser' >
                            <md-icon>add</md-icon>
                        </md-button>
                    </md-list-item>
                </md-list>
            </div>
        </md-card-header>
        <md-card-content>
            <div class="md-layout md-gutter">
                <div class="md-layout-item md-small-size-100">
                    <md-list class="md-double-line md-dense">
                        <md-list-item>
                            <md-field>
                                <label for='company_name'>Название</label>
                                <md-input type="company_name" min=1 max=100 name='company_name' id='company_name' />
                                <span class="md-helper-text">Укажите название УК</span>
                            </md-field>
                        </md-list-item>
                    </md-list>
                </div>
                <div class="md-layout-item md-small-size-100">
                    <md-list class="md-double-line md-dense">
                        <md-list-item>
                            <md-field md-clearable>
                                <label for='user_name'>ФИО</label>
                                <md-input name='user_name' id='user_name' v-model='user_name' />
                                <span class="md-helper-text">Укажите ФИО ответственного лица</span>
                            </md-field>
                        </md-list-item>
                    </md-list>
                </div>
                <div class="md-layout-item md-small-size-100">
                    <md-list class="md-double-line md-dense">
                        <md-list-item>
                            <md-field md-clearable>
                                <label for='user_login'>Логин</label>
                                <md-input name='user_login' id='user_login' v-model='user_login' />
                                <span class="md-helper-text">Укажите логин для доступа к сервису</span>
                            </md-field>
                        </md-list-item>
                    </md-list>
                </div>
            </div>
            <div class="md-layout md-gutter">
                <div class="md-layout-item md-size-30">
                    <md-list class="md-double-line md-dense">
                        <md-list-item>
                            <md-field md-clearable>
                                <label for='telephone_num'>Телефон</label>
                                <md-input name='telephone_num' id='telephone_num' v-model='telephone_num'  />
                                <span class="md-helper-text">Укажите телехон для связи</span>
                            </md-field>
                        </md-list-item>
                    </md-list>
                </div>
                <div class="md-layout-item md-size-70">
                    <md-list class="md-double-line md-dense">
                        <md-list-item>
                            <md-field>
                                <label>Описание | Адрес | Контакты </label>
                                <md-textarea v-model='description' ></md-textarea>
                                <md-icon>description</md-icon>
                            </md-field>
                        </md-list-item>
                    </md-list>
                </div>
            </div>
        </md-card-content>
    </md-card>
  </div>
</template>


<script>
    import axios from 'axios';

    export default {
      name: 'AddUser',
      props: {
          count_id: {
              type: Number
          }
      },
      data: () => ({
          company_name: '',
          user_name: '',
          user_login: '',
          telephone_num: '',
          description: '',
          date_from: '',
          date_to: ''
      }),
      mounted: function() {
      },
      methods: {
          addUser() {
              console.log('ADD USER');
          }
      }
    }
</script>

<style lang="scss" scoped>
  .md-card {
    width: 320px;
    margin: 4px;
    display: inline-block;
    vertical-align: top;
  }
</style>

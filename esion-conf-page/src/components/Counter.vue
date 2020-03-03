<template>
  <div>
    <md-card v-if=active class="md-layout-item md-size-100 md-small-size-100">
        <md-card-header>
            <div class="md-layout md-gutter">
                <div class="md-layout-item md-size-25">
                    <md-list class="md-double-line md-dense">
                        <md-list-item>
                            <span class="md-title">Счётчик N {{count_id}}</span>
                        </md-list-item>
                    </md-list>
                </div>
                <div class="md-layout-item md-size-33">
                    <md-list class="md-double-line md-dense">
                        <md-list-item>
                            <md-field md-clearable>
                                <label for='count_type'>Начальное значение</label>
                                <md-input type="number" name='start_mcubs' id='count_type' v-model='start_mcubs' v-on:input="$emit('count-data-event',{id:count_id-1,k:'start_mcubs',v:$event})" />
                                <span class="md-helper-text">Укажите начальное значение счётчика</span>
                            </md-field>
                        </md-list-item>
                    </md-list>
                </div>
                <div class="md-layout-item md-size-33">
                    <md-list class="md-double-line md-dense">
                        <md-list-item>
                            <span class="md-title">{{mcubs}} [ м.куб. ]</span>
                        </md-list-item>
                    </md-list>
                </div>
                <md-button v-if=!old_inited class="md-icon-button md-raised md-accent" @click='erase' >
                    <!-- <md-icon >clear</md-icon> -->
                    <md-icon class="" md-src="icons/close-24px.svg" />
                </md-button>
            </div>
        </md-card-header>
        <md-card-content>
            <div class="md-layout md-gutter">
                <div class="md-layout-item md-small-size-100">
                    <md-list class="md-double-line md-dense">
                        <md-list-item>
                            <md-field md-clearable>
                                <label for='count_type'>Тип</label>
                                <md-input name='count_type' id='count_type' v-model='count_type' v-on:input="$emit('count-data-event',{id:count_id-1,k:'count_type',v:$event})" />
                                <span class="md-helper-text">Укажите тип счётчика</span>
                            </md-field>
                        </md-list-item>
                    </md-list>
                </div>
                <div class="md-layout-item md-small-size-100">
                    <md-list class="md-double-line md-dense">
                        <md-list-item>
                            <md-field md-clearable>
                                <label for='unit'>Юнит</label>
                                <md-input name='unit' id='unit' v-model='unit' v-on:input="$emit('count-data-event',{id:count_id-1,k:'unit',v:$event})" />
                                <span class="md-helper-text">Укажите единицу измерения</span>
                            </md-field>
                        </md-list-item>
                    </md-list>
                </div>
                <div class="md-layout-item md-small-size-100">
                    <md-list class="md-double-line md-dense">
                        <md-list-item>
                            <md-field>
                                <label for='unit_impl'>Юнит / Импульс</label>
                                <md-input type="number" min=1 max=100 name='unit_impl' id='unit_impl' v-model='unit_impl' v-on:input="$emit('count-data-event',{id:count_id-1,k:'unit_impl',v:$event})" />
                                <span class="md-helper-text">Укажите цену импульса</span>
                            </md-field>
                        </md-list-item>
                    </md-list>
                </div>
            </div>
            <div class="md-layout md-gutter">
                <div class="md-layout-item md-size-20">
                    <md-list class="md-double-line md-dense">
                        <md-list-item>
                            <md-field md-clearable>
                                <label for='ser_num'>Сер. Номер</label>
                                <md-input name='ser_num' id='ser_num' v-model='ser_num' v-on:input="$emit('count-data-event',{id:count_id-1, k:'ser_num',v:$event})" />
                                <span class="md-helper-text">Укажите серийный номер</span>
                            </md-field>
                        </md-list-item>
                    </md-list>
                </div>
                <!-- <div class="md-layout-item md-size-30">
                    <md-list class="md-double-line md-dense">
                        <md-list-item>
                            <md-datepicker v-model="verify_date" v-on:md-closed="$emit('count-data-event',{id:count_id-1,k:'verify_date',v:verify_date})" />
                        </md-list-item>
                    </md-list>
                </div> -->
                <div class="md-layout-item md-size-80">
                    <md-list class="md-double-line md-dense">
                        <md-list-item>
                            <md-field>
                                <label>Описание</label>
                                <md-textarea v-model='description' v-on:input="$emit('count-data-event',{id:count_id-1,k:'desc',v:$event})" ></md-textarea>
                            </md-field>
                        </md-list-item>
                    </md-list>
                </div>
            </div>
        </md-card-content>
    </md-card>
    <md-card v-if=!active class="md-layout-item md-size-100 md-small-size-100">
        <md-card-header>
            <div class="md-layout-item md-small-size-100">
                <md-list class="md-double-line md-dense">
                    <md-list-item>
                        <span class="md-title">Счётчик N {{count_id}}</span>
                        <md-list class="md-double-line md-dense">
                            <md-list-item>
                                <md-button class='md-dense md-raised md-primary' @click='setup'>Настроить</md-button>
                            </md-list-item>
                        </md-list>
                    </md-list-item>
                </md-list>
            </div>
        </md-card-header>
    </md-card>
  </div>
</template>

<script>
    import axios from 'axios';

    export default {
      name: 'CounterCard',
      props: {
          count_id: {
              type: Number
          }
      },
      data: () => ({
          old_inited: false,
          active: false,
          count_type: 'none',
          ser_num: '',
          unit: 'литр',
          unit_impl: 1,
          description: '',
          //verify_date: 0, //Number(0),
          start_mcubs: 0.0,
          mcubs: 0.0
      }),
      computed: {
          mdType() {
              return Number;
          }
      },
      mounted: function() {
          var id = this.count_id - 1;
          axios.get('/counter_' + id).then((resp) => {
              var js = resp.data;
              if (js !== null) {
                  if (typeof js['type'] !== 'undefined') {
                      this.count_type = js.type;
                      if (js.type !== 'none') {
                          if (typeof js['ser_num'] !== 'undefined') {
                              this.ser_num = js.ser_num;
                          }
                          if (typeof js['unit'] !== 'undefined') {
                              this.unit = js.unit;
                          }
                          if (typeof js['unit_impl'] !== 'undefined') {
                              this.unit_impl = js.unit_impl;
                          }
                          if (typeof js['desc'] !== 'undefined') {
                              this.description = js.desc;
                          }
                          // if (typeof js['verify_date'] !== 'undefined') {
                          //     this.verify_date = moment.unix(js.verify_date).format('DD.MM.YYYY HH:mm:ss');
                          // }
                          if (typeof js['start_mcubs'] !== 'undefined') {
                              this.start_mcubs = js.start_mcubs;
                          }
                          if (typeof js['mcubs'] !== 'undefined') {
                              this.mcubs = js.mcubs;
                          }
                          this.active = true;
                          this.old_inited = true;
                      }
                  }
              }
          });
      },
      methods: {
          setup() {
              this.active = true;
          },
          erase() {
              this.active = false;
              this.ser_num = '';
              this.unit = 'литр';
              this.unit_impl = 1;
              this.count_type = 'none';
              this.description = '';
              this.start_mcubs = 0.0;
              this.mcubs = 0.0;
              this.$emit('erase-count-event', this.count_id);
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
    // @font-face {
    //     font-family: 'Material Icons';
    //     font-style: normal;
    //     font-weight: 400;
    //     src: url(../../public/fonts/KFOlCnqEu92Fr1MmEU9fBBc4.woff2) format('woff2');
    // }
    // @font-face {
    //     font-family: 'Material Icons';
    //     font-style: normal;
    //     font-weight: 400;
    //     src: url(../../public/fonts/KFOlCnqEu92Fr1MmWUlfBBc4.woff2) format('woff2');
    // }
    // @font-face {
    //     font-family: 'Material Icons';
    //     font-style: normal;
    //     font-weight: 400;
    //     src: url(../../public/fonts/KFOmCnqEu92Fr1Mu4mxK.woff2) format('woff2');
    // }
    // @font-face {
    //     font-family: 'Material Icons';
    //     font-style: normal;
    //     font-weight: 400;
    //     src: url(../../public/fonts/KFOmCnqEu92Fr1Mu5mxKOzY.woff2) format('woff2');
    // }
    // @font-face {
    //     font-family: 'Material Icons';
    //     font-style: normal;
    //     font-weight: 400;
    //     src: url(../../public/fonts/flUhRq6tzZclQEJ-Vdg-IuiaDsNc.woff2) format('woff2');
    // }
</style>

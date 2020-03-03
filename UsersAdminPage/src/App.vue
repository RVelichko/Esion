<template>
  <div id="app">
    <component :is="layout">
      <router-view />
    </component>
  </div>
</template>

<script>
  import Default from './layouts/Default'
  import Plain from './layouts/Plain'

  export default {
    computed: {
      layout() {
        return this.$route.meta.layout || 'default';
      }
    },
    components: {
      Default,
      Plain
    }
  }
</script>

<style lang="scss">
  #app {
    font-family: Roboto, Helvetica, Arial, sans-serif;
    font-size: 16px;
    -webkit-font-smoothing: antialiased;
    -moz-osx-font-smoothing: grayscale;
    text-align: center;
    color: #2c3e50;
  }
</style>

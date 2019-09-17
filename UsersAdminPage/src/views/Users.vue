<template>
  <div class="users">
    <img alt="Vue logo" src="../assets/logo.png">
    <UsersInformation msg="Welcome to Your Vue.js App"/>
  </div>
</template>

<script>
// @ is an alias to /src
import UsersInformation from '@/components/UsersInformation.vue'

export default {
    name: 'users',
    components: {
        UsersInformation
    }
}
</script>

<template>
  <div class="users">
      <AddUser />
      <UsersInformation />
  </div>
</template>

<script>
// @ is an alias to /src
import AddUser from '@/components/AddUser.vue'
import UsersInformation from '@/components/UsersInformation.vue'

export default {
    name: 'users',
    components: {
        AddUser,
        UsersInformation
    }
}
</script>

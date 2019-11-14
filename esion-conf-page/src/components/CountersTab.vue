<template>
  <div>
    <form class="md-layout-item md-size-100 md-small-size-100">
        <counter-card :count_id=1 @count-data-event="$emit('count-card-event', $event)" @erase-count-event="$emit('erase-count-event', $event)" />
        <counter-card :count_id=2 @count-data-event="$emit('count-card-event', $event)" @erase-count-event="$emit('erase-count-event', $event)" />
        <counter-card :count_id=3 @count-data-event="$emit('count-card-event', $event)" @erase-count-event="$emit('erase-count-event', $event)" />
        <counter-card :count_id=4 @count-data-event="$emit('count-card-event', $event)" @erase-count-event="$emit('erase-count-event', $event)" />
    </form>
  </div>
</template>

<script>
    import CounterCard from './Counter.vue';

    export default {
      name: 'CountersTab',
      components: {
          CounterCard
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

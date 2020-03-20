<template lang="pug">
b-card(title="Lucky nums" class="mb-2")
  b-card-text
    div
      ul
        li(v-for="num in prevLuckyNums") {{ num }}
</template>

<script>
export default {
  name: "PrevLuckyNums",
  props: {
    prevLuckyNums: Array,
  },
}
</script>

<style scoped>
ul {
  list-style-type: none;
  padding: 0;
}
li {
  display: inline-block;
  margin: 0 10px;
}
</style>

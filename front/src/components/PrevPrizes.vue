<template lang="pug">
b-card(title="Prizes" class="mb-2")
  b-card-text
    div
      ul
        li(v-for="prize in prevPrizes") GR${{ prize }}
</template>

<script>
export default {
  name: "PrevPrizes",
  props: {
    prevPrizes: Array,
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

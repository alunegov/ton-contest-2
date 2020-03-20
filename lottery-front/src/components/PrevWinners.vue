<template lang="pug">
b-card(title="Winners" class="mb-2")
  b-card-text
    div
      ul
        li(v-for="winner in prevWinners") {{ winner.addr }} - GR${{ winner.prize }}
  a(href="#" class="card-link") View all winners
</template>

<script>
export default {
  name: 'PrevWinners',
  data () {
    return {
      prevWinners: [
        { addr: 'sadfwqerkl;dfjkvlvbasdfasdf', prize: 23 },
        { addr: 'xvcbcvm,3490829641112348980920_349', prize: 45},
      ],
    };
  },
}
</script>

<style scoped>
ul {
  list-style-type: none;
  padding: 0;
}
li {
  margin: 0 10px;
}
</style>

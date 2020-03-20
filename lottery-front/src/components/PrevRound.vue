<template lang="pug">
#prev
  b-container(fluid="md")
    b-row(class="pt-2")
      b-col
        h3 Prev round
    b-row
      b-col
        b-card-group(deck)
          PrevLuckyNums(:prev-lucky-nums="prev_lucky_nums")
          PrevPrizes(:prev-prizes="prev_prizes")
    b-row
      b-col
        PrevWinners(v-if="false")
</template>

<script>
import { mapState } from 'vuex';

import PrevLuckyNums from './PrevLuckyNums.vue';
import PrevPrizes from './PrevPrizes.vue';
import PrevWinners from './PrevWinners.vue';

export default {
  name: 'PrevRound',
  components: {
    PrevLuckyNums,
    PrevPrizes,
    PrevWinners,
  },
  computed: mapState([
    'prev_lucky_nums',
    'prev_prizes',
  ]),
  mounted() {
    this.$store.dispatch('load_prev_round');
  },
}
</script>

<style scoped>
#prev {
  background-color: #0bb814a6;
  min-height: 200px;
}
</style>

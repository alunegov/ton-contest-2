<template lang="pug">
#current
  b-container(fluid="md" class="pb-4")
    b-row(class="pt-2")
      b-col
        h1 "3 of 13" lottery smc
    b-row(class="pt-2")
      b-col
        h3 Current round
    b-row
      b-col
        b-card-group(deck)
          CurrentPrizeFund(:prize-fund="prize_fund")
          CurrentParticipants(:participants-count="participants_count")
</template>

<script>
import { mapState } from 'vuex';

import CurrentPrizeFund from './CurrentPrizeFund.vue';
import CurrentParticipants from './CurrentParticipants.vue';

export default {
  name: 'CurrentRound',
  components: {
    CurrentPrizeFund,
    CurrentParticipants,
  },
  computed: mapState([
    'prize_fund',
    'participants_count',
  ]),
  mounted() {
    this.$store.dispatch('load_current_round');
  },
}
</script>

<style scoped>
#current {
  background-color: #950bb88f;
  min-height: 200px;
}
</style>

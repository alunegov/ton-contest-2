<template lang='pug'>
#my-prize
  b-container(fluid="md")
    b-row(class="pt-2 mb-2")
      b-col
        h3 Check your prize
    b-row
      b-col(sm="2")
        p Address
      b-col(sm="8" class="mb-2")
        b-form-input(placeholder='Enter your wallet smc address' v-model='addr')
      b-col(sm="2")
        b-button(variant="primary" @click='check_is_winner(addr)') Check
    b-row
      b-col
        p(v-if="is_winner_checked") Prize for {{ is_winner_addr }} is GR${{ is_winner_prize }}
</template>

<script>
import { mapState, mapActions } from 'vuex';

export default {
  name: 'MyPrize',
  data () {
    return {
      addr: '',
    };
  },
  computed: mapState([
    'is_winner_checked',
    'is_winner_addr',
    'is_winner_prize',
  ]),
  methods: mapActions([
    'check_is_winner',
  ]),
}
</script>

<style scoped>
#my-prize {
  background-color: #0b79b8af;
  min-height: 200px;
}
</style>

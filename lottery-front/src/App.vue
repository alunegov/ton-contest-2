<template lang="pug">
#app
  CurrentRound
  Rules
  PrevRound
  MyPrize
</template>

<script>
import CurrentRound from './components/CurrentRound.vue';
import Rules from './components/Rules.vue';
import PrevRound from './components/PrevRound.vue';
import MyPrize from './components/MyPrize.vue';

export default {
  name: 'App',
  components: {
    CurrentRound,
    Rules,
    PrevRound,
    MyPrize,
  },
}
</script>

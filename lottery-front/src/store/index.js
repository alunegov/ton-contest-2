import Vue from 'vue';
import Vuex from 'vuex';
import axios from 'axios';

Vue.use(Vuex);

export default new Vuex.Store({
  state: {
    API: 'http://localhost:8080',
    prize_fund: 0,
    participants_count: 0,
    prev_lucky_nums: [],
    prev_prizes: [0, 0, 0],
    is_winner_checked: false,
    is_winner_addr: '',
    is_winner_prize: 0,
  },

  mutations: {
    set_prize_fund (state, prize_fund) {
      state.prize_fund = prize_fund;
    },
    set_participants_count (state, participants_count) {
      state.participants_count = participants_count;
    },
    set_lucky_nums (state, lucky_nums) {
      state.prev_lucky_nums = [...lucky_nums];
    },
    set_prizes (state, prizes) {
      state.prev_prizes = [...prizes];
    },
    set_is_winner (state, { addr, prize }) {
      state.is_winner_checked = true;
      state.is_winner_addr = addr;
      state.is_winner_prize = prize;
    },
  },

  actions: {
    load_current_round: async ({ commit, state }) => {
      const resp = await axios.get(`${state.API}/current`);
      //console.log(resp);
      commit('set_prize_fund', resp.data.prize_fund);
      commit('set_participants_count', resp.data.participants_count);
    },
    load_prev_round: async ({ commit, state }) => {
      const resp = await axios.get(`${state.API}/prev`);
      commit('set_lucky_nums', resp.data.lucky_nums);
      commit('set_prizes', resp.data.prizes);
    },
    check_is_winner: async ({ commit, state }, addr) => {
      const resp = await axios.get(`${state.API}/is_winner/${addr}`);
      commit('set_is_winner', { addr: addr, prize: resp.data.prize });
    },
  },

  modules: {
  },
})

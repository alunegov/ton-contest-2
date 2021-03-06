import Vue from 'vue';
import Vuex from 'vuex';
import axios from 'axios';

Vue.use(Vuex);

function to_grams(nanograms) {
  return nanograms / 1000000000.0
}

export default new Vuex.Store({
  state: {
    API: process.env.VUE_APP_API,
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
      const resp_prize_fund = await axios.get(`${state.API}/prize_fund`);
      const resp_participants = await axios.get(`${state.API}/participants`);
      commit('set_prize_fund', to_grams(resp_prize_fund.data.prize_fund));
      commit('set_participants_count', resp_participants.data.participants.length);
    },
    load_prev_round: async ({ commit, state }) => {
      const resp_lucky_nums = await axios.get(`${state.API}/lucky_nums`);
      const resp_prizes = await axios.get(`${state.API}/prizes`);
      commit('set_lucky_nums', resp_lucky_nums.data.lucky_nums);
      commit('set_prizes', [to_grams(resp_prizes.data.p1), to_grams(resp_prizes.data.p2), to_grams(resp_prizes.data.p3)]);
    },
    check_is_winner: async ({ commit, state }, addr) => {
      const resp = await axios.get(`${state.API}/is_winner?addr=${encodeURIComponent(addr)}`);
      commit('set_is_winner', { addr: addr, prize: to_grams(resp.data.prize) });
    },
  },

  modules: {
  },
})

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
      const resp_prize_fund = await axios.get(`${state.API}/prize_fund`);
      const resp_participants = await axios.get(`${state.API}/participants`);
      commit('set_prize_fund', resp_prize_fund.data.prize_fund);
      commit('set_participants_count', resp_participants.data.participants.length);
    },
    load_prev_round: async ({ commit, state }) => {
      const resp_lucky_nums = await axios.get(`${state.API}/lucky_nums`);
      const resp_prizes = await axios.get(`${state.API}/prizes`);
      commit('set_lucky_nums', resp_lucky_nums.data.lucky_nums);
      commit('set_prizes', resp_prizes.data.prizes);
    },
    check_is_winner: async ({ commit, state }, addr) => {
      // TODO: sanitize addr?
      const resp = await axios.get(`${state.API}/is_winner/${addr}`);
      commit('set_is_winner', { addr: addr, prize: resp.data.prize });
    },
  },

  modules: {
  },
})

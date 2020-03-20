<template lang="pug">
#rules
  b-container(fluid="md")
    b-row(class="pt-2")
      b-col
        h3 Rules
    b-row
      b-col
        b-card(class="mb-2")
          ul
            li Participant choose 3 arbitrary unique numbers in range from 0 to 99 and "buys" a ticket
            li Prize fund of current round is a sum of all "sold" tickets plus "unused" fund from previos round minus commision to owner
            li Sponsors can increase current round prize fund by any amount of GR$
            li Owner of smc conducts a lottery round (say once per day):
              ul
                li Generate 13 unique random numbers in range from 0 to 99 and determines the winners and their prize among participants
                li Guess all 3 numbers - 60% of prize fund, 2 numbers - 22% and 1 number - 15%
                li Charge smc commision - 3% of prize fund
            li Results of previous round can be fetched with lucky_nums, prizes and is_winner methods
            li Winners of previous rounds can withdraw their prizes
          <b-alert show>Buying a ticket and withdrawing a prize can be performed with official Gram Wallet app</b-alert>
</template>

<script>
export default {
  name: 'Rules',
}
</script>

<style scoped>
#rules {
  background-color: #ffc973;
  min-height: 200px;
}
ul {
  padding: 0;
}
li {
  margin: 0 30px;
}
</style>

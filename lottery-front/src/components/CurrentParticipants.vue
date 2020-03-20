<template lang="pug">
b-card(title="Participants" class="mb-2")
  b-card-text Number of participants are {{ participantsCount }}
  a(href="#" class="card-link" v-if="false") View participants
</template>

<script>
export default {
  name: "CurrentParticipants",
  props: {
    participantsCount: Number,
  },
}
</script>

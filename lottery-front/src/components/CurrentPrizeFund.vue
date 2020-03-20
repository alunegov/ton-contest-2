<template lang="pug">
b-card(title="Prize fund" class="mb-2")
  b-card-text Prize fund is GR${{ prizeFund }}
</template>

<script>
export default {
  name: "CurrentPrizeFund",
  props: {
    prizeFund: Number,
  },
}
</script>

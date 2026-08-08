<script setup lang="ts">
import { computed } from 'vue'
import { useData, withBase } from 'vitepress'
import { archived, latest } from '../versions'

const { page } = useData()

const version = computed(() =>
  archived.find((v) => page.value.relativePath.startsWith(`${v}/`)),
)

/** The same page in the latest docs, so the link does not dump the reader on the home page. */
const samePageInLatest = computed(() =>
  withBase('/' + page.value.relativePath.replace(/^[^/]+\//, '').replace(/\.md$/, '')),
)
</script>

<template>
  <div v-if="version" class="version-banner">
    <strong>You are reading the {{ version }} documentation.</strong>
    The current version is {{ latest }}.
    <a :href="samePageInLatest">Go to this page in {{ latest }}</a>.
  </div>
</template>

<style scoped>
.version-banner {
  margin: 0 0 24px;
  padding: 12px 16px;
  border: 1px solid var(--vp-c-warning-1);
  border-radius: 8px;
  background: var(--vp-c-warning-soft);
  font-size: 14px;
  line-height: 1.6;
}

.version-banner a {
  color: var(--vp-c-brand-1);
  font-weight: 500;
  text-decoration: underline;
  text-underline-offset: 2px;
}
</style>

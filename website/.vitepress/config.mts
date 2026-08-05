import { defineConfig } from 'vitepress'

const repo = 'https://github.com/he4rt/scylladb-php-driver'

export default defineConfig({
  title: 'ScyllaDB PHP Driver',
  description:
    'A high-performance PHP extension for ScyllaDB and Apache Cassandra, built on the native CQL binary protocol.',
  lang: 'en-US',
  base: '/scylladb-php-driver/',
  cleanUrls: true,
  lastUpdated: true,
  appearance: 'dark',
  ignoreDeadLinks: false,

  head: [
    ['link', { rel: 'icon', type: 'image/svg+xml', href: '/scylladb-php-driver/logo.svg' }],
    ['meta', { name: 'theme-color', content: '#6D4AFF' }],
    ['meta', { property: 'og:type', content: 'website' }],
    ['meta', { property: 'og:title', content: 'ScyllaDB PHP Driver' }],
    [
      'meta',
      {
        property: 'og:description',
        content: 'A high-performance PHP extension for ScyllaDB and Apache Cassandra.',
      },
    ],
  ],

  markdown: {
    theme: { light: 'github-light', dark: 'houston' },
    lineNumbers: false,
  },

  sitemap: {
    hostname: 'https://he4rt.github.io/scylladb-php-driver/',
  },

  themeConfig: {
    logo: '/logo.svg',
    siteTitle: 'ScyllaDB PHP',

    nav: [
      { text: 'Guide', link: '/guide/introduction', activeMatch: '/guide/' },
      { text: 'Reference', link: '/reference/cluster-builder', activeMatch: '/reference/' },
      {
        text: 'Resources',
        items: [
          { text: 'Packagist', link: 'https://packagist.org/packages/codelieutenant/scylla-driver' },
          { text: 'Changelog', link: `${repo}/blob/trunk/CHANGELOG.md` },
          { text: 'Contributing', link: `${repo}/blob/trunk/CONTRIBUTING.md` },
          { text: 'Discord', link: 'https://discord.gg/B6rutCXvgp' },
        ],
      },
    ],

    sidebar: {
      '/guide/': [
        {
          text: 'Getting started',
          collapsed: false,
          items: [
            { text: 'Introduction', link: '/guide/introduction' },
            { text: 'Installation', link: '/guide/installation' },
            { text: 'Quick start', link: '/guide/quickstart' },
          ],
        },
        {
          text: 'Connecting',
          collapsed: false,
          items: [
            { text: 'Clusters and sessions', link: '/guide/connecting' },
            { text: 'Authentication', link: '/guide/authentication' },
            { text: 'TLS and SSL', link: '/guide/tls' },
            { text: 'Load balancing and routing', link: '/guide/load-balancing' },
            { text: 'Connection pool and timeouts', link: '/guide/connection-tuning' },
            { text: 'Retry policies', link: '/guide/retry-policies' },
          ],
        },
        {
          text: 'Working with data',
          collapsed: false,
          items: [
            { text: 'Queries and statements', link: '/guide/queries' },
            { text: 'Results and paging', link: '/guide/results' },
            { text: 'Data types', link: '/guide/data-types' },
            { text: 'Collections and UDTs', link: '/guide/collections' },
            { text: 'Batches', link: '/guide/batches' },
            { text: 'Asynchronous queries', link: '/guide/async' },
            { text: 'Schema metadata', link: '/guide/schema-metadata' },
          ],
        },
        {
          text: 'Operating',
          collapsed: false,
          items: [
            { text: 'Error handling', link: '/guide/error-handling' },
            { text: 'Performance', link: '/guide/performance' },
            { text: 'Metrics and logging', link: '/guide/observability' },
            { text: 'Troubleshooting', link: '/guide/troubleshooting' },
          ],
        },
      ],
      '/reference/': [
        {
          text: 'API reference',
          collapsed: false,
          items: [
            { text: 'Cluster\\Builder', link: '/reference/cluster-builder' },
            { text: 'Cluster and Session', link: '/reference/session' },
            { text: 'Statements', link: '/reference/statements' },
            { text: 'Rows and Futures', link: '/reference/rows-futures' },
            { text: 'Value classes', link: '/reference/values' },
            { text: 'Type factory', link: '/reference/types' },
            { text: 'Constants', link: '/reference/constants' },
            { text: 'Exceptions', link: '/reference/exceptions' },
          ],
        },
      ],
    },

    socialLinks: [{ icon: 'github', link: repo }],

    editLink: {
      pattern: `${repo}/edit/trunk/website/:path`,
      text: 'Edit this page on GitHub',
    },

    search: {
      provider: 'local',
    },

    outline: { level: [2, 3], label: 'On this page' },

    footer: {
      message: 'Released under the Apache License 2.0.',
      copyright: 'Copyright © DataStax, Inc. and contributors.',
    },
  },
})

import { defineConfig } from 'vitepress'
import { allSidebars, isArchived, latest, versionNav } from './versions'

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

  // An archived version keeps its own copy of every page. Without this, a
  // search for "withPort" returns one hit per version.
  transformPageData(pageData) {
    if (isArchived(pageData.relativePath)) {
      pageData.frontmatter.search = false
    }
  },

  themeConfig: {
    logo: '/logo.svg',
    siteTitle: 'ScyllaDB PHP',

    nav: [
      { text: 'Guide', link: '/guide/introduction', activeMatch: '^/guide/' },
      { text: 'Reference', link: '/reference/cluster-builder', activeMatch: '^/reference/' },
      {
        text: 'Resources',
        items: [
          { text: 'Packagist', link: 'https://packagist.org/packages/codelieutenant/scylla-driver' },
          { text: 'Changelog', link: `${repo}/blob/trunk/CHANGELOG.md` },
          { text: 'Contributing', link: `${repo}/blob/trunk/CONTRIBUTING.md` },
          { text: 'Discord', link: 'https://discord.gg/B6rutCXvgp' },
        ],
      },
      versionNav(),
    ],

    sidebar: allSidebars(),

    socialLinks: [{ icon: 'github', link: repo }],

    editLink: {
      pattern: `${repo}/edit/trunk/website/:path`,
      text: 'Edit this page on GitHub',
    },

    search: {
      provider: 'local',
      options: {
        // Belt and braces: transformPageData already sets search: false, but a
        // future page added under an archived directory must never leak in.
        _render(src, env, md) {
          if (isArchived(env.relativePath)) return ''
          return md.render(src, env)
        },
      },
    },

    outline: { level: [2, 3], label: 'On this page' },

    footer: {
      message: `Version ${latest}. Released under the Apache License 2.0.`,
      copyright: 'Copyright © DataStax, Inc. and contributors.',
    },
  },
})

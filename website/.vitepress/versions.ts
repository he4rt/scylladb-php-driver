import versions from '../versions.json'

export const latest: string = versions.latest
export const archived: string[] = versions.archived

/** '' for the latest docs at the site root, '/1.4' for an archived version. */
export function basePath(version: string): string {
  return version === latest ? '' : `/${version}`
}

/**
 * The guide and reference sidebars, rooted at `base`.
 * An archived version gets the same tree under its own prefix.
 */
export function sidebar(base = '') {
  return {
    [`${base}/guide/`]: [
      {
        text: 'Getting started',
        collapsed: false,
        items: [
          { text: 'Introduction', link: `${base}/guide/introduction` },
          { text: 'Installation', link: `${base}/guide/installation` },
          { text: 'Quick start', link: `${base}/guide/quickstart` },
        ],
      },
      {
        text: 'Connecting',
        collapsed: false,
        items: [
          { text: 'Clusters and sessions', link: `${base}/guide/connecting` },
          { text: 'Authentication', link: `${base}/guide/authentication` },
          { text: 'TLS and SSL', link: `${base}/guide/tls` },
          { text: 'Load balancing and routing', link: `${base}/guide/load-balancing` },
          { text: 'Connection pool and timeouts', link: `${base}/guide/connection-tuning` },
          { text: 'Retry policies', link: `${base}/guide/retry-policies` },
        ],
      },
      {
        text: 'Working with data',
        collapsed: false,
        items: [
          { text: 'Queries and statements', link: `${base}/guide/queries` },
          { text: 'Results and paging', link: `${base}/guide/results` },
          { text: 'Data types', link: `${base}/guide/data-types` },
          { text: 'Collections and UDTs', link: `${base}/guide/collections` },
          { text: 'Batches', link: `${base}/guide/batches` },
          { text: 'Asynchronous queries', link: `${base}/guide/async` },
          { text: 'Event loops', link: `${base}/guide/event-loops` },
          { text: 'Schema metadata', link: `${base}/guide/schema-metadata` },
        ],
      },
      {
        text: 'Operating',
        collapsed: false,
        items: [
          { text: 'Error handling', link: `${base}/guide/error-handling` },
          { text: 'Performance', link: `${base}/guide/performance` },
          { text: 'Metrics and logging', link: `${base}/guide/observability` },
          { text: 'Troubleshooting', link: `${base}/guide/troubleshooting` },
        ],
      },
    ],
    [`${base}/reference/`]: [
      {
        text: 'API reference',
        collapsed: false,
        items: [
          { text: 'Cluster\\Builder', link: `${base}/reference/cluster-builder` },
          { text: 'Cluster and Session', link: `${base}/reference/session` },
          { text: 'Statements', link: `${base}/reference/statements` },
          { text: 'Rows and Futures', link: `${base}/reference/rows-futures` },
          { text: 'Async and event loops', link: `${base}/reference/async` },
          { text: 'Value classes', link: `${base}/reference/values` },
          { text: 'Type factory', link: `${base}/reference/types` },
          { text: 'Constants', link: `${base}/reference/constants` },
          { text: 'Exceptions', link: `${base}/reference/exceptions` },
        ],
      },
    ],
  }
}

/** One sidebar map covering the latest docs and every archived version. */
export function allSidebars() {
  return archived.reduce(
    (acc, version) => ({ ...acc, ...sidebar(`/${version}`) }),
    sidebar(),
  )
}

/** The version dropdown. Latest first, then archives, newest to oldest. */
export function versionNav() {
  return {
    text: latest,
    items: [
      { text: `${latest} (latest)`, link: '/' },
      ...archived.map((version) => ({ text: version, link: `/${version}/` })),
    ],
  }
}

/** True for a path that belongs to an archived version. */
export function isArchived(relativePath: string): boolean {
  return archived.some((version) => relativePath.startsWith(`${version}/`))
}

#!/usr/bin/env node

/**
 * Freeze the docs at the site root as an archived version, then point the root
 * at a new version number.
 *
 *   node scripts/version.mjs 1.5
 *
 * Run it when you START writing the docs for a new release, not when you
 * finish. The root always describes `latest`, so the content has to be copied
 * away before you change it.
 */

import { cp, mkdir, readFile, readdir, stat, writeFile } from 'node:fs/promises'
import { existsSync } from 'node:fs'
import { dirname, join, resolve } from 'node:path'
import { fileURLToPath } from 'node:url'

const root = resolve(dirname(fileURLToPath(import.meta.url)), '..')
const versionsFile = join(root, 'versions.json')

/** Copied into the archive. Everything else is shared across versions. */
const CONTENT = ['guide', 'reference', 'index.md']

const next = process.argv[2]

if (!next) {
  fail('Usage: node scripts/version.mjs <new-version>   (for example 1.5)')
}

if (!/^\d+\.\d+$/.test(next)) {
  fail(`Version must be MAJOR.MINOR, for example 1.5 or 2.0. Got "${next}".`)
}

const versions = JSON.parse(await readFile(versionsFile, 'utf8'))
const { latest, archived } = versions

if (next === latest) {
  fail(`The root already describes ${next}.`)
}

if (archived.includes(next)) {
  fail(`${next} is already archived. Pick a version number that is not in use.`)
}

const target = join(root, latest)

if (existsSync(target)) {
  fail(`${latest}/ already exists. Remove it, or check versions.json.`)
}

// 1. Copy the root content into the archive directory.
await mkdir(target, { recursive: true })

for (const entry of CONTENT) {
  await cp(join(root, entry), join(target, entry), { recursive: true })
}

// 2. Rewrite in-site links so an archived page links inside its own version.
//    Every /guide/ and /reference/ in the docs is a link target, so a plain
//    replace is safe. A build with ignoreDeadLinks: false catches a miss.
let rewritten = 0

for await (const file of markdownFiles(target)) {
  const before = await readFile(file, 'utf8')
  const after = before
    .replaceAll('/guide/', `/${latest}/guide/`)
    .replaceAll('/reference/', `/${latest}/reference/`)

  if (after !== before) {
    await writeFile(file, after)
    rewritten++
  }
}

// 3. Register the archive and move the root to the new version.
versions.latest = next
versions.archived = [latest, ...archived]

await writeFile(versionsFile, `${JSON.stringify(versions, null, 2)}\n`)

console.log(`Archived ${latest} into website/${latest}/ (${rewritten} files rewritten).`)
console.log(`The site root now describes ${next}.`)
console.log('')
console.log('Next:')
console.log(`  1. npm run build          # confirms every link in ${latest}/ still resolves`)
console.log(`  2. Edit guide/ and reference/ for ${next}`)
console.log(`  3. Drop any "Known defect in ${latest}.x" note that ${next} fixes`)

async function* markdownFiles(dir) {
  for (const entry of await readdir(dir)) {
    const full = join(dir, entry)
    if ((await stat(full)).isDirectory()) {
      yield* markdownFiles(full)
    } else if (entry.endsWith('.md')) {
      yield full
    }
  }
}

function fail(message) {
  console.error(message)
  process.exit(1)
}

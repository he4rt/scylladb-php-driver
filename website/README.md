# Documentation site

The user documentation, published to GitHub Pages at
<https://he4rt.github.io/scylladb-php-driver/>.

Built with [VitePress](https://vitepress.dev).

## Develop

```bash
cd website
npm install
npm run dev      # http://localhost:5173/scylladb-php-driver/
```

## Build

```bash
npm run build    # output in .vitepress/dist
npm run preview  # serve the built site
```

The build fails on a dead internal link, because `ignoreDeadLinks` is `false`. Run
`npm run build` before you open a pull request.

## Structure

```
website/
├── .vitepress/
│   ├── config.mts          # site metadata, nav, search
│   ├── versions.ts         # version list, sidebar generator
│   └── theme/
│       ├── index.ts        # extends the default theme
│       ├── VersionBanner.vue  # "you are reading the 1.5 docs" bar
│       └── style.css       # palette and layout overrides
├── scripts/version.mjs     # cuts a new version
├── versions.json           # which version the root is, which are archived
├── public/logo.svg
├── index.md                # home page
├── guide/                  # task-oriented pages, the latest version
├── reference/              # API reference pages, the latest version
└── 1.5/                    # an archived version, once one exists
    ├── guide/
    └── reference/
```

## Add a page

1. Create the Markdown file under `guide/` or `reference/`.
2. Add it to the matching array in `.vitepress/versions.ts`, in the `sidebar()`
   function. Use the `${base}` prefix, so archived versions keep working.
3. Run `npm run build` to check the links.

## Versioning

The site root always documents the version named in `versions.json`. Older
versions are frozen copies under `website/<version>/`, reachable from the
version dropdown in the nav.

```json
{
  "latest": "1.5",
  "archived": []
}
```

### Cut a new version

Run this when you START writing the docs for a release, not when you finish.
The root content has to be copied away before you change it.

```bash
npm run docs:version 1.6
```

That does three things:

1. Copies `guide/`, `reference/`, and `index.md` into `website/1.5/`.
2. Rewrites every `/guide/` and `/reference/` link in the copy to `/1.5/guide/`
   and `/1.5/reference/`, so an archived page links inside its own version.
3. Sets `latest` to `1.6` and adds `1.5` to `archived`.

Then:

```bash
npm run build     # confirms every rewritten link still resolves
```

Now edit `guide/` and `reference/` for 1.6. Remove any "Known defect in 1.5.x"
note that 1.6 fixes. The 1.5 copy keeps the old text.

### What an archived version gets

- Its own sidebar, generated from the same `sidebar()` function.
- A banner at the top of every page, linking to the same page in the latest
  version.
- Exclusion from the search index, so one query does not return the same page
  once per version.

Archived pages are never edited again. Fix a typo in an old version only if it
is wrong in a way that misleads someone pinned to that release.

## Publish

`.github/workflows/docs.yml` builds every push to `trunk` that touches `website/`, then deploys to
GitHub Pages. Pull requests build but do not deploy.

The repository needs **Settings → Pages → Build and deployment → Source: GitHub Actions** for the
deployment step to work.

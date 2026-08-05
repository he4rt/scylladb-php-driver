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
│   ├── config.mts        # nav, sidebar, search, site metadata
│   └── theme/
│       ├── index.ts      # extends the default theme
│       └── style.css     # palette and layout overrides
├── public/logo.svg
├── index.md              # home page
├── guide/                # task-oriented pages
└── reference/            # API reference pages
```

## Add a page

1. Create the Markdown file under `guide/` or `reference/`.
2. Add it to the matching `sidebar` array in `.vitepress/config.mts`.
3. Run `npm run build` to check the links.

## Publish

`.github/workflows/docs.yml` builds every push to `trunk` that touches `website/`, then deploys to
GitHub Pages. Pull requests build but do not deploy.

The repository needs **Settings → Pages → Build and deployment → Source: GitHub Actions** for the
deployment step to work.

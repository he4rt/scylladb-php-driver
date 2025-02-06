import {themes as prismThemes} from 'prism-react-renderer';
import type {Config} from '@docusaurus/types';
import type * as Preset from '@docusaurus/preset-classic';

// This runs in Node.js - Don't use client-side code here (browser APIs, JSX...)

const docs_env = process.env.DOCS_ENV || 'local';
const isProd = docs_env === 'production';
const url = isProd
        ? 'https://he4rt.github.io'
        : 'http://localhost:8000';

const baseUrl = isProd
        ? '/scylladb-php-driver/'
        : '/';

const fullUrl = `${url}${baseUrl}`;


const config: Config = {
    title: 'ScyllaDB PHP Driver',
    tagline: 'PHP Driver for the highly performant NoSQL database',
    favicon: 'img/favicon.ico',

    // Set the production url of your site here
    url: url,
    // Set the /<baseUrl>/ pathname under which your site is served
    // For GitHub pages deployment, it is often '/<projectName>/'
    baseUrl: baseUrl,

    // GitHub pages deployment config.
    // If you aren't using GitHub pages, you don't need these.
    organizationName: 'He4rt Developers', // Usually your GitHub org/user name.
    projectName: 'scylladb-php-driver', // Usually your repo name.

    onBrokenLinks: 'warn',
    onBrokenMarkdownLinks: 'warn',

    // Even if you don't use internationalization, you can use this field to set
    // useful metadata like html lang. For example, if your site is Chinese, you
    // may want to replace "en" with "zh-Hans".


    presets: [
        [
            'classic',
            {
                docs: {
                    sidebarPath: './sidebars.ts',
                },
                blog: {
                    showReadingTime: false,
                    feedOptions: {
                        type: ['rss', 'atom'],
                        xslt: true,
                    },
                    // Please change this to your repo.
                    // Remove this to remove the "edit this page" links.
                    // Useful options to enforce blogging best practices
                    onInlineTags: 'warn',
                    onInlineAuthors: 'warn',
                    onUntruncatedBlogPosts: 'warn',
                },
                theme: {
                    customCss: './src/css/custom.css',
                },
            } satisfies Preset.Options,
        ],
    ],

    themeConfig: {
        // Replace with your project's social card
        image: 'img/docusaurus-social-card.jpg',
        navbar: {
            title: 'ScyllaDB PHP Driver Docs',
            logo: {
                alt: 'ScyllaDB PHP Driver Docs',
                src: 'img/logo.png',
            },
            items: [
                {
                    type: 'docSidebar',
                    sidebarId: 'tutorialSidebar',
                    position: 'left',
                    label: 'Getting Started',
                },
                {
                    href: fullUrl + '/api/',
                    label: 'API Documentation',
                    position: 'left',
                },
                {
                    href: 'https://github.com/he4rt/scylladb-php-driver',
                    label: 'GitHub',
                    position: 'right',
                },
            ],
        },
        footer: {
            style: 'dark',
            links: [
                {
                    title: 'Docs',
                    items: [
                        {
                            label: 'Tutorial',
                            to: '/docs/intro',
                        },
                    ],
                },
                {
                    title: 'Community',
                    items: [
                        {
                            label: 'Discord',
                            href: 'https://discord.gg/M4wQRRBT',
                        },
                    ],
                },
                {
                    title: 'More',
                    items: [
                        {
                            label: 'GitHub',
                            href: 'https://github.com/he4rt/scylladb-php-driver',
                        },
                    ],
                },
            ],
            copyright: `Copyright © ${new Date().getFullYear()} Basement Developers, Inc. Built with Docusaurus.`,
        },
        prism: {
            theme: prismThemes.jettwaveLight,
            darkTheme: prismThemes.jettwaveDark,
            additionalLanguages: ['php', 'bash'],
        },
    } satisfies Preset.ThemeConfig,
};

export default config;

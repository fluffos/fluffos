import { themes as prismThemes } from 'prism-react-renderer';
import type { Config } from '@docusaurus/types';
import type * as Preset from '@docusaurus/preset-classic';
import { createRequire } from 'node:module';

const require = createRequire(import.meta.url);
const lpcSyntaxPlugin = require('../tools/lpc-syntax/docusaurus-plugin.cjs');

const config: Config = {
  title: 'FluffOS Docs',
  tagline: 'LPC driver — from zero to a running mud',
  favicon: 'img/favicon.svg',
  url: 'https://www.fluffos.info',
  baseUrl: '/',
  organizationName: 'fluffos',
  projectName: 'fluffos',
  onBrokenLinks: 'throw',

  future: {
    // Needed for faster.ssgWorkerThreads; do not set v4: true (that
    // turns off MDX v1 compat, which this corpus still needs).
    v4: { removeLegacyPostBuildHeadAttribute: true },
    faster: true,
  },

  i18n: {
    defaultLocale: 'en',
    locales: ['en', 'zh-CN'],
    localeConfigs: {
      en: { label: 'English' },
      'zh-CN': { label: '简体中文' },
    },
  },

  markdown: {
    // treat .md files as standard Markdown (not MDX) so existing docs don't need conversion
    format: 'detect',
    mermaid: true,
    hooks: {
      onBrokenMarkdownLinks: 'warn',
    },
  },

  presets: [
    [
      'classic',
      {
        docs: {
          sidebarPath: './sidebars.ts',
          path: '.',
          routeBasePath: '/',
          editUrl: 'https://github.com/fluffos/fluffos/edit/master/docs/',
          showLastUpdateTime: true,
          showLastUpdateAuthor: true,
          // ```c fences on language / efun / apply pages are LPC, not C.
          // Real C stays ```c under docs/driver/ and build-wasm; use ```cpp
          // for C samples elsewhere (see concepts/general/lpc.md).
          remarkPlugins: [lpcSyntaxPlugin],
          exclude: [
            '**/node_modules/**',
            '**/archive/**',
            // the docs source dir is the site dir itself, so keep the
            // translated corpus (i18n/) out of the default-locale glob
            'i18n/**',
            'CLAUDE.md',
            'README.md',
          ],
        },
        blog: false,
        sitemap: {
          lastmod: 'datetime',
          changefreq: 'weekly',
          priority: 0.5,
          filename: 'sitemap.xml',
        },
        theme: {
          customCss: './src/css/custom.css',
        },
      } satisfies Preset.Options,
    ],
  ],

  plugins: [
    [
      '@docusaurus/plugin-client-redirects',
      {
        // Jekyll / VitePress leftover: /build.html → /build
        fromExtensions: ['html'],
        redirects: [
          { from: '/llms', to: '/llm' },
          { from: '/getting-started', to: '/start' },
          { from: '/agents', to: '/mudlib-agents' },
          { from: '/build_v2017', to: '/build' },
          { from: '/reporting-bugs', to: '/bug' },
          { from: '/dev-environment', to: '/lpc/dev-environment' },
          { from: '/dev-setup', to: '/lpc/dev-environment' },
        ],
      },
    ],
    'docusaurus-plugin-image-zoom',
  ],

  themes: [
    '@docusaurus/theme-mermaid',
    [
      '@easyops-cn/docusaurus-search-local',
      {
        // offline/local search — no external service required
        hashed: true,
        indexBlog: false,
        // docs live in this directory and are served from the site root
        docsDir: '.',
        docsRouteBasePath: '/',
        // English docs plus the zh-CN pages
        language: ['en', 'zh'],
        highlightSearchTermsOnTargetPage: true,
        explicitSearchResultPath: true,
      },
    ],
  ],

  themeConfig: {
    image: 'img/favicon.svg',
    metadata: [
      {
        name: 'description',
        content:
          'FluffOS documentation: build the driver, boot a mud, LPC language, efuns, applies, and the fluffos/* ecosystem.',
      },
    ],
    mermaid: {
      theme: { light: 'neutral', dark: 'dark' },
    },
    zoom: {
      selector: '.markdown :not(em) > img',
      background: {
        light: 'rgb(255, 255, 255)',
        dark: 'rgb(26, 35, 50)',
      },
    },
    navbar: {
      title: 'FluffOS',
      logo: {
        alt: 'FluffOS',
        src: 'img/favicon.svg',
      },
      items: [
        { to: '/start', label: 'Start', position: 'left' },
        { to: '/llm', label: 'LLM', position: 'left' },
        { to: '/lpc/', label: 'LPC', position: 'left' },
        { to: '/efun/', label: 'Efuns', position: 'left' },
        { to: '/ecosystem', label: 'Ecosystem', position: 'left' },
        {
          type: 'docSidebar',
          sidebarId: 'docs',
          position: 'left',
          label: 'Docs',
        },
        {
          type: 'localeDropdown',
          position: 'right',
        },
        {
          href: 'https://github.com/fluffos/fluffos',
          label: 'GitHub',
          position: 'right',
        },
      ],
    },
    footer: {
      style: 'dark',
      links: [
        {
          title: 'Start',
          items: [
            { label: 'From zero to a running mud', to: '/start' },
            { label: 'LLM onboarding', to: '/llm' },
            { label: 'Mudlib AGENTS.md', to: '/mudlib-agents' },
            { label: 'Dev environment', to: '/lpc/dev-environment' },
            { label: 'Build from Source', to: '/build' },
            { label: 'Troubleshooting', to: '/bug' },
          ],
        },
        {
          title: 'Reference',
          items: [
            { label: 'LPC', to: '/lpc/' },
            { label: 'Efuns', to: '/efun/' },
            { label: 'Applies', to: '/apply/' },
            { label: 'Ecosystem', to: '/ecosystem' },
          ],
        },
        {
          title: 'Community',
          items: [
            { label: 'GitHub org', href: 'https://github.com/fluffos' },
            { label: 'Driver repo', href: 'https://github.com/fluffos/fluffos' },
            { label: 'Releases', href: 'https://github.com/fluffos/fluffos/releases' },
            { label: 'Forum', href: 'https://forum.fluffos.info' },
            { label: 'License', to: '/license' },
          ],
        },
      ],
      copyright: `Copyright © ${new Date().getFullYear()} FluffOS Contributors.`,
    },
    prism: {
      theme: prismThemes.github,
      darkTheme: prismThemes.dracula,
      additionalLanguages: ['c', 'cpp', 'bash', 'json', 'cmake', 'ini', 'diff'],
    },
    // mermaid + image-zoom keys are not on Preset.ThemeConfig
  } as Preset.ThemeConfig,
};

export default config;

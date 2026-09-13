import { themes as prismThemes } from 'prism-react-renderer';
import type { Config } from '@docusaurus/types';
import type * as Preset from '@docusaurus/preset-classic';

const config: Config = {
  title: 'FluffOS Docs',
  tagline: 'LPC driver — from zero to a running mud',
  favicon: 'img/favicon.svg',
  url: 'https://www.fluffos.info',
  baseUrl: '/',
  organizationName: 'fluffos',
  projectName: 'fluffos',
  onBrokenLinks: 'throw',

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
        theme: {
          customCss: './src/css/custom.css',
        },
      } satisfies Preset.Options,
    ],
  ],

  themes: [
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
    },
  } satisfies Preset.ThemeConfig,
};

export default config;

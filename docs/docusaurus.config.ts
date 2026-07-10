import { themes as prismThemes } from 'prism-react-renderer';
import type { Config } from '@docusaurus/types';
import type * as Preset from '@docusaurus/preset-classic';

const config: Config = {
  title: 'FluffOS Docs',
  tagline: 'A high-performance LPMUD game engine',
  favicon: undefined,
  url: 'https://www.fluffos.info',
  baseUrl: '/',
  organizationName: 'fluffos',
  projectName: 'fluffos',
  onBrokenLinks: 'throw',

  i18n: {
    defaultLocale: 'en',
    locales: ['en'],
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
    navbar: {
      title: 'FluffOS',
      items: [
        {
          type: 'docSidebar',
          sidebarId: 'docs',
          position: 'left',
          label: 'Documentation',
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
          title: 'Documentation',
          items: [
            { label: 'Getting Started', to: '/build' },
            { label: 'Efuns', to: '/efun/' },
            { label: 'Applies', to: '/apply/' },
          ],
        },
        {
          title: 'Community',
          items: [
            {
              label: 'GitHub',
              href: 'https://github.com/fluffos/fluffos',
            },
            {
              label: 'Issues',
              href: 'https://github.com/fluffos/fluffos/issues',
            },
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

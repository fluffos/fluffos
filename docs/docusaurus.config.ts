import { themes as prismThemes } from 'prism-react-renderer';
import type { Config } from '@docusaurus/types';
import type * as Preset from '@docusaurus/preset-classic';
import { createRequire } from 'node:module';

const require = createRequire(import.meta.url);
const lpcSyntaxPlugin = require('../tools/lpc-syntax/docusaurus-plugin.cjs');

const siteUrl = 'https://www.fluffos.info';

const jsonLd = {
  '@context': 'https://schema.org',
  '@graph': [
    {
      '@type': 'Organization',
      '@id': `${siteUrl}/#org`,
      name: 'FluffOS',
      url: siteUrl,
      logo: `${siteUrl}/img/favicon.svg`,
      sameAs: [
        'https://github.com/fluffos',
        'https://github.com/fluffos/fluffos',
        'https://forum.fluffos.info',
      ],
    },
    {
      '@type': 'WebSite',
      '@id': `${siteUrl}/#website`,
      url: siteUrl,
      name: 'FluffOS',
      alternateName: [
        'FluffOS Docs',
        'MudOS',
        'MudOS successor',
        'LPMUD',
        'LPMud',
        'LPC',
        'LPC language',
      ],
      description:
        'Documentation for FluffOS, the actively maintained MudOS successor: an LPMUD driver with an LPC compiler and VM.',
      publisher: { '@id': `${siteUrl}/#org` },
      inLanguage: ['en', 'zh-CN'],
    },
    {
      '@type': 'SoftwareApplication',
      name: 'FluffOS',
      alternateName: ['MudOS', 'FluffOS LPMUD driver'],
      applicationCategory: 'GameEngine',
      operatingSystem: 'Linux, macOS, Windows, WebAssembly',
      programmingLanguage: ['LPC', 'C++'],
      url: siteUrl,
      downloadUrl: 'https://github.com/fluffos/fluffos',
      softwareHelp: `${siteUrl}/lpmud`,
      license: `${siteUrl}/license`,
      isAccessibleForFree: true,
      offers: { '@type': 'Offer', price: '0', priceCurrency: 'USD' },
      description:
        'Actively maintained MudOS successor. LPMUD driver: LPC compiler, virtual machine, Telnet, WebSocket, and TLS.',
    },
    {
      '@type': 'FAQPage',
      '@id': `${siteUrl}/lpmud#faq`,
      url: `${siteUrl}/lpmud`,
      mainEntity: [
        {
          '@type': 'Question',
          name: 'What is MudOS?',
          acceptedAnswer: {
            '@type': 'Answer',
            text: 'MudOS is an LPMUD driver (LPC compiler and virtual machine) from the 1990s. It is no longer maintained. FluffOS is the actively maintained MudOS successor and runs existing MudOS mudlibs.',
          },
        },
        {
          '@type': 'Question',
          name: 'What is LPMUD?',
          acceptedAnswer: {
            '@type': 'Answer',
            text: 'LPMUD (also spelled LPMud) is a multiplayer text-world architecture that splits the game (the mudlib, written in LPC) from the engine (the driver). FluffOS is a current LPMUD driver.',
          },
        },
        {
          '@type': 'Question',
          name: 'What is LPC?',
          acceptedAnswer: {
            '@type': 'Answer',
            text: 'LPC (Lars Pensjö C) is the object-oriented, C-like language used to write LPMUD games. FluffOS compiles and runs LPC. It is not C, Pike, or a general-purpose compiler.',
          },
        },
        {
          '@type': 'Question',
          name: 'Is FluffOS compatible with MudOS?',
          acceptedAnswer: {
            '@type': 'Answer',
            text: 'Yes. FluffOS is backward-compatible with MudOS mudlibs. Use current master, not the unsupported v2017 branch.',
          },
        },
      ],
    },
  ],
};

const config: Config = {
  title: 'FluffOS',
  tagline: 'MudOS successor · LPMUD driver · LPC language',
  favicon: 'img/favicon.svg',
  url: siteUrl,
  baseUrl: '/',
  organizationName: 'fluffos',
  projectName: 'fluffos',
  onBrokenLinks: 'throw',
  headTags: [
    {
      tagName: 'script',
      attributes: { type: 'application/ld+json' },
      innerHTML: JSON.stringify(jsonLd),
    },
  ],

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
          // Real C stays ```c under docs/driver/. LPC on build-wasm and
          // new pages should use ```lpc. Use ```cpp for C samples
          // elsewhere (see concepts/general/lpc.md).
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
          createSitemapItems: async (params) => {
            const { defaultCreateSitemapItems, ...rest } = params;
            const items = await defaultCreateSitemapItems(rest);
            const bump = (path: string, priority: number) => {
              for (const item of items) {
                const u = item.url.replace(/\/$/, '') || siteUrl;
                if (u === `${siteUrl}${path}` || u === `${siteUrl}/zh-CN${path}`) {
                  item.priority = priority;
                  item.changefreq = 'weekly';
                }
              }
            };
            bump('', 1);
            bump('/lpmud', 0.9);
            bump('/lpc', 0.9);
            bump('/start', 0.8);
            bump('/efun', 0.7);
            bump('/apply', 0.7);
            return items;
          },
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
        // Alias → canonical. createRedirects also emits /zh-CN/… variants
        // so locale-prefixed bookmarks stay in zh-CN.
        createRedirects(existingPath: string) {
          const aliases: Record<string, string[]> = {
            '/llm': ['/llms'],
            '/start': ['/getting-started'],
            '/mudlib-agents': ['/agents'],
            '/build': ['/build_v2017'],
            '/bug': ['/reporting-bugs'],
            '/lpc/dev-environment': ['/dev-environment', '/dev-setup'],
            '/lpmud': [
              '/mudos',
              '/lpmud-driver',
              '/lp-mud',
              '/lpc-language',
              '/what-is-lpc',
              '/what-is-mudos',
            ],
          };
          const localePrefix = existingPath.startsWith('/zh-CN/')
            ? '/zh-CN'
            : '';
          const unprefixed = localePrefix
            ? existingPath.slice(localePrefix.length)
            : existingPath;
          const extra = aliases[unprefixed];
          return extra ? extra.map((from) => localePrefix + from) : undefined;
        },
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
    image: 'img/og.png',
    metadata: [
      {
        name: 'description',
        content:
          'FluffOS is the actively maintained MudOS successor: an LPMUD driver with an LPC compiler and VM. Language reference, efuns, mudlibs, and how to boot a game.',
      },
      {
        name: 'keywords',
        content:
          'FluffOS, MudOS, LPMUD, LPMud, LPC, LPC language, LPMUD driver, MudOS successor, Lars Pensjö C, mudlib',
      },
      { name: 'twitter:card', content: 'summary_large_image' },
      { property: 'og:type', content: 'website' },
      { property: 'og:site_name', content: 'FluffOS' },
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
            { label: 'MudOS, LPMUD, LPC', to: '/lpmud' },
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

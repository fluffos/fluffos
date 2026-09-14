import type { SidebarsConfig } from '@docusaurus/plugin-content-docs';
import generated from './sidebars.generated.json';

// The reference trees (efun/, apply/, stdlib/, concepts/, driver/, cli/,
// zh-CN/) are generated into sidebars.generated.json by gen_sidebar.py,
// driven by the curated labels/descriptions in sidebar_meta.json.
// Run `./gen_sidebar.py` after adding, removing, or moving pages in those
// trees; CI verifies freshness with `./gen_sidebar.py --check`.
//
// The lpc/ tree and the top-level pages below are hand-authored.
const gen = generated as Record<string, any>;

const sidebars: SidebarsConfig = {
  docs: [
    {
      type: 'category',
      label: 'Getting Started',
      collapsed: false,
      items: [
        { type: 'doc', id: 'index', label: 'Overview' },
        { type: 'doc', id: 'lpmud', label: 'MudOS, LPMUD, LPC' },
        { type: 'doc', id: 'start', label: 'From zero to a running mud' },
        { type: 'doc', id: 'lpc/dev-environment', label: 'Dev environment', key: 'getting-started-dev-env' },
        { type: 'doc', id: 'llm', label: 'LLM onboarding' },
        { type: 'doc', id: 'mudlib-agents', label: 'Mudlib AGENTS.md' },
        { type: 'doc', id: 'ecosystem', label: 'Ecosystem' },
        { type: 'doc', id: 'build', label: 'Build from Source' },
        { type: 'doc', id: 'build-wasm', label: 'Build for WebAssembly' },
        { type: 'doc', id: 'bug', label: 'Troubleshooting' },
        { type: 'doc', id: 'license', label: 'License' },
      ],
    },
    {
      type: 'category',
      label: 'LPC Language',
      link: { type: 'doc', id: 'lpc/index' },
      items: [
        { type: 'doc', id: 'lpc/source-files', label: 'Source Files & Object Names' },
        { type: 'doc', id: 'lpc/diagnostics', label: 'Compiler Diagnostics' },
        { type: 'doc', id: 'lpc/dev-environment', label: 'Dev environment', key: 'lpc-dev-env' },
        { type: 'doc', id: 'lpc/style-guide', label: 'Style Guide' },
        { type: 'doc', id: 'lpc/formatter', label: 'Formatter' },
        {
          type: 'category',
          label: 'Types',
          link: { type: 'doc', id: 'lpc/types/index' },
          items: [
            'lpc/types/general',
            'lpc/types/array',
            'lpc/types/buffer',
            'lpc/types/classes',
            'lpc/types/float',
            'lpc/types/function',
            'lpc/types/mappings',
            'lpc/types/promise',
            'lpc/types/strings',
            'lpc/types/substructures',
          ],
        },
        {
          type: 'category',
          label: 'Constructs',
          link: { type: 'doc', id: 'lpc/constructs/index' },
          items: [
            'lpc/constructs/async',
            'lpc/constructs/catch',
            'lpc/constructs/do-while',
            'lpc/constructs/for',
            'lpc/constructs/foreach',
            'lpc/constructs/function',
            'lpc/constructs/if',
            'lpc/constructs/include',
            'lpc/constructs/inherit',
            'lpc/constructs/modifiers',
            'lpc/constructs/operators',
            'lpc/constructs/prototypes',
            'lpc/constructs/ref',
            'lpc/constructs/shadowing',
            'lpc/constructs/switch',
            'lpc/constructs/text_blocks',
            'lpc/constructs/while',
          ],
        },
        {
          type: 'category',
          label: 'Preprocessor',
          link: { type: 'doc', id: 'lpc/preprocessor/index' },
          items: [
            'lpc/preprocessor/define',
            'lpc/preprocessor/include',
            'lpc/preprocessor/conditionals',
            'lpc/preprocessor/pragma',
          ],
        },
      ],
    },
    gen['efun'],
    gen['apply'],
    gen['stdlib'],
    gen['concepts'],
    gen['driver'],
    gen['cli'],
  ],
};

export default sidebars;

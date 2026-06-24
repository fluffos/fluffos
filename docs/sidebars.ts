import type { SidebarsConfig } from '@docusaurus/plugin-content-docs';

const sidebars: SidebarsConfig = {
  docs: [
    {
      type: 'category',
      label: 'Introduction',
      collapsed: false,
      items: ['index', 'build', 'bug'],
    },
    {
      type: 'category',
      label: 'CLI',
      collapsed: false,
      items: [
        'cli/driver',
        'cli/lpcc',
        'cli/o2json',
        'cli/json2o',
      ],
    },
    {
      type: 'category',
      label: 'LPC',
      items: [
        { type: 'doc', id: 'lpc/index', label: 'LPC Language' },
        { type: 'doc', id: 'driver/index', label: 'Driver' },
      ],
    },
    {
      type: 'category',
      label: 'Reference',
      items: [
        {
          type: 'category',
          label: 'Apply',
          link: { type: 'doc', id: 'apply/index' },
          items: [
            { type: 'doc', id: 'apply/master/index', label: 'Master' },
            { type: 'doc', id: 'apply/interactive/index', label: 'Interactive' },
            { type: 'doc', id: 'apply/object/index', label: 'Object' },
          ],
        },
        { type: 'doc', id: 'efun/index', label: 'EFUN' },
        { type: 'doc', id: 'stdlib/index', label: 'LPC Library' },
      ],
    },
    {
      type: 'category',
      label: 'Driver Internal',
      items: [
        { type: 'doc', id: 'concepts/index', label: 'Concepts' },
        { type: 'doc', id: 'driver/index', label: 'Notes' },
      ],
    },
    {
      type: 'category',
      label: '中文文档',
      items: [
        { type: 'doc', id: 'zh-CN/build/index', label: '编译文档' },
        { type: 'doc', id: 'zh-CN/apply/index', label: 'Apply 文档' },
        { type: 'doc', id: 'zh-CN/efun/index', label: 'EFUN 文档' },
      ],
    },
    {
      type: 'category',
      label: 'Historical',
      items: ['build_v2017'],
    },
  ],
};

export default sidebars;

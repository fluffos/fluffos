const SIDEBAR = [
    {
        text: 'Introduction',
        collapsed: false,
        items: [
            {text: 'FluffOS', link: '/'},
            {text: 'Getting Started', link: '/build.html'},
            {text: 'Troubleshooting', link: '/bug.html'}
        ]
    },
    {
        text: 'CLI',
        collapsed: false,
        items: [
            {text: 'driver', link: '/cli/driver.html'},
            {text: 'lpcc', link: '/cli/lpcc.html'},
            {text: 'o2json', link: '/cli/o2json.html'},
            {text: 'json2o', link: '/cli/json2o.html'}
        ]
    },
    {
        text: 'LPC',
        items: [
            {text: 'LPC Language', link: '/lpc/'},
            {text: 'Driver', link: '/driver/'},
        ]
    },
    {
        text: 'Reference',
        items: [
            {
                text: 'Apply',
                link: '/apply/',
                collapsed: true,
                items: [
                    {text: 'Master', link: '/apply/master/'},
                    {text: 'Interactive', link: '/apply/interactive/'},
                    {text: 'Object', link: '/apply/object/'},
                ]
            },
            {text: 'EFUN', link: '/efun/'},
            {text: 'LPC Library', link: '/stdlib/'}
        ]
    },
    {
        text: 'Driver Internal',
        items: [
            {text: 'Concepts', link: '/concepts/'},
            {text: 'Notes', link: '/driver/'},
        ]
    },
    {
        text: '中文文档',
        items: [
            {text: '编译文档', link: '/zh-CN/build/'},
            {text: 'Apply 文档', link: '/zh-CN/apply/'},
            {text: 'EFUN 文档', link: '/zh-CN/efun/'},
        ]
    },
    {
        text: 'Historical',
        items: [
            {text: 'Build (v2017)', link: '/build_v2017.html'}
        ]
    },
]

export default SIDEBAR;
---
title: Mudlib AGENTS.md
slug: /mudlib-agents
description: FluffOS mudlib 的 AGENTS.md 起步模板。拷进 mudlib 根目录并填空。
---

# Mudlib `AGENTS.md`

本页是**模板**。用户选定 mudlib 后，把下面内容拷到该树根目录的 `AGENTS.md`
（若已有，例如 fluffos/mudlibs 的长篇说明，只合并端口和启动行，不要整文件覆盖）。
填完所有 `TODO`。

完整可拷贝英文模板见 [英文页](https://www.fluffos.info/mudlib-agents)。不要把
`fluffos/fluffos` 驱动仓库的 `AGENTS.md` 贴进 mudlib。

助手应先读 **mudlib** 的 `AGENTS.md`，再读
[https://www.fluffos.info/llm](https://www.fluffos.info/llm)。

要点（写入文件时请改成该 lib 的事实）：

- 这是 **mudlib**（游戏），驱动是另一个程序
- 启动命令、配置文件、`master file`、include、端口
- 管理员/第一次登录怎么做
- 不要臆造 efun；apply 签名看 https://www.fluffos.info/apply/
- 不要跑驱动的 `testsuite/`，除非用户在改 FluffOS
- 编码：默认 UTF-8；GBK 树只有在用户要求时才用 [gbk2utf8](https://github.com/fluffos/gbk2utf8)

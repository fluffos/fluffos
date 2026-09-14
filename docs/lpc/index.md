---
title: LPC language (Lars Pensjö C)
description: LPC is the C-like language LPMUD games use. FluffOS — the MudOS successor — compiles and runs this LPC dialect.
keywords:
  - LPC
  - Lars Pensjö C
  - LPMUD
  - MudOS
  - FluffOS
---

# The LPC language

**LPC** (Lars Pensjö C) is the language **LPMUD** games are written in.
**FluffOS** (the **MudOS** successor) compiles and runs it. Every room,
item, NPC, and player is an LPC object.

Glossary: [MudOS, LPMUD, and LPC](../lpmud).

* [source files & object names](source-files)
* [compiler diagnostics](diagnostics)
* [development environment](dev-environment) — editor plugin, docs highlighter, formatter
* [style guide](style-guide)
* [formatter](formatter)

## constructs

* [async, await, acatch](constructs/async)
* [catch / throw](constructs/catch)
* [do-while](constructs/do-while)
* [for](constructs/for)
* [foreach](constructs/foreach)
* [function](constructs/function)
* [if](constructs/if)
* [include](constructs/include)
* [inherit](constructs/inherit)
* [modifiers](constructs/modifiers)
* [operators](constructs/operators)
* [prototypes](constructs/prototypes)
* [ref](constructs/ref)
* [shadowing](constructs/shadowing)
* [switch](constructs/switch)
* [text_blocks](constructs/text_blocks)
* [while](constructs/while)

## preprocessor

* [overview & directives](preprocessor/)
* [define](preprocessor/define)
* [include](preprocessor/include)
* [conditionals](preprocessor/conditionals)
* [pragma](preprocessor/pragma)

## types

* [array](types/array)
* [buffer](types/buffer)
* [classes](types/classes)
* [float](types/float)
* [function](types/function)
* [general](types/general)
* [mappings](types/mappings)
* [promise](types/promise)
* [strings](types/strings)
* [substructures](types/substructures)

Concept-level overview: [The LPC Language](../concepts/general/lpc).
Async in depth: [Async and Promises](../concepts/general/async).

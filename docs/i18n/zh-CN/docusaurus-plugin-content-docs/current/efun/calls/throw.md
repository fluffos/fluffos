---
title: calls / throw
---
# throw

### 名称

    throw() - 把一个值交给最近的 catch()

### 语法

    void throw(mixed value);

### 描述

    立即中止当前正在执行的代码，并把 `value` 交给当前调用之上最近的
    catch(3)。该 catch() 会返回 `value`，而不是通常的 0。

    throw() 不会返回，所以同一个函数中写在它后面的代码永远不会执行。

    任何类型的值都可以抛出，catch() 会把它原样交回 —— 字符串、数组、
    映射、类、对象都可以。值不会被转换成文本，也不会被添加任何内容。
    因此 throw() 的用途不止于错误消息：你可以把一个完整的结构化结果
    交给准备处理它的代码。

    抛出 0 是无法被检测到的。catch() 本来就用返回 0 表示“没有出错”，
    所以 throw(0) 和主体正常执行完毕看起来完全一样。请抛出 0 以外的值。

    只有最内层的 catch() 会收到这个值，它不会自动继续向外传递。如果
    捕获到它的代码想把它继续传下去，必须再次 throw()。

### 传播范围

    抛出的值可以沿着任意多层普通调用向上传递。不管嵌套多深，也不管你
    是怎么进入这层调用的：同一对象内的函数调用、通过 `ob->func()` 的
    调用、继承来的函数、用 evaluate() 调用的函数指针，以及由 filter()、
    sort_array() 这类外部函数执行的回调，都会把抛出的值原样传给上面的
    catch()。甚至在 `catch(load_object(...))` 中载入对象时，其 create()
    抛出的值也能正常被捕获。

    抛出的值唯一无法逃出的，是由驱动程序自己发起的调用。call_out()、
    input_to() 的回调，以及驱动程序在你的对象上调用的各种 apply ——
    logon()、net_dead()、process_input()、reset()、clean_up()、
    on_destruct()，还有 telnet 和 GMCP 相关的 apply —— 每一个都会开始
    一条全新的调用链，它们之上没有任何 catch()。即使当初安排这个
    call_out() 的代码本身正处在一个 catch() 之中也是如此：等到回调真正
    执行时，那个 catch() 早就已经返回了。在这些地方使用 throw() 会引发
    错误（见下面的“错误”一节）。如果回调中需要处理失败，请把 catch()
    写在回调内部。

### 抛出的值不是驱动程序错误

    通过 throw() 到达 catch() 的值完全没有经过驱动程序的错误处理机制。
    不会打印调用栈回溯，不会写入调试日志，也不会调用主控对象中的
    error_handler() apply。

    抛出的字符串同样不会带上驱动程序给自己的错误消息加的前导 `*`，
    而这个 `*` 正是处理代码区分两者的依据：

    - `"*Bad argument 1 to move()\n"` 来自驱动程序。
    - `"took too long\n"` 来自 mudlib 中某处的 throw()。

    用捕获到的值拼接新消息时要注意这一点。如果 `err` 来自驱动程序，
    写成 `throw("context: " + err)` 会把 `*` 埋到新字符串的中间，任何
    检查 `err[0] == '*'` 的代码都不会再把它识别为驱动程序错误。

### 错误

    如果当前调用链上没有任何 catch()，throw() 会引发一个真正的运行时
    错误 `*Throw with no catch.` —— 带调用栈回溯，也会走错误处理流程
    —— 而你抛出的值会被丢弃。

### 示例

```c
// 把失败继续向上传递，同时补充更多上下文。
void move_or_fail(object ob, object dest) {
    mixed err = catch(ob->move(dest));

    if (err) {
        // 先去掉驱动程序加的 '*'，添加文字后再补回去，
        // 这样结果读起来仍然像一条驱动程序风格的错误消息。
        if (stringp(err) && err[0] == '*')
            err = err[1..];

        throw("*move_or_fail(): " + err);
    }
}
```

```c
// 抛出一个值而不是一条消息，这样调用方可以直接根据细节做判断，
// 而不必去拆解字符串。
private void charge(object who, int amount) {
    int have = who->query_coins();

    if (have < amount)
        throw(({ "insufficient_funds", amount - have }));

    who->add_coins(-amount);
}

void buy(object who, int price) {
    mixed err = catch(charge(who, price));

    if (arrayp(err) && err[0] == "insufficient_funds")
        write("你还差 " + err[1] + " 个金币。\n");
    else if (err)
        throw(err);     // 不该由我们处理 —— 继续往上传
}
```

### 参考

    catch(3), error(3), error_handler(4)

### 翻译

    雪风(i@mud.ren)

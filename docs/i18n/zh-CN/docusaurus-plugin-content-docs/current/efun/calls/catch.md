---
title: calls / catch
---
# catch

### 名称

    catch - 捕获运行时错误

### 语法

    mixed catch( expr );
    mixed catch { statements }

### 描述

    catch 是一个语言结构（编译器关键字），而不是普通的外部函数。
    它接受两种主体写法，与 time_expression 共享同一语法：

    - 括号表达式形式：`catch(expr)` 对 <expr> 求值。
    - 语句块形式：`catch { statements }` 执行该语句块。

    如果没有错误会返回 0，如果有标准错误，会返回一个以 `*` 开头的
    包括错误信息的字符串。主体表达式本身的值会被丢弃。

    外部函数 throw() 也可以用来立即返回任意值，可以和 catch 配合使用。
    唯一的例外是 0：它并不会被拒绝，只是无法被检测到 —— catch() 本来
    就用返回 0 表示“没有出错”，所以 throw(0) 和主体正常执行完毕无法
    区分。

    在 catch 语句块中使用 `break` 或 `continue` 跳出是编译期错误。

    catch() 比较消耗资源，请不要随意使用，建议只用在出错时可能会造成严重问题的地方。

### 示例

    ```c
    void example1() {
        object ob ;
        mixed err ;

        err = catch( ob = load_object("/obj/weapon/sword") ) ;
        if(err) throw("加载指定文件时出错。") ;
    }

    void example2() {
        mixed err = catch {
            string file, *files = ({
                "/u/g/gesslar/one",     // 正常文件
                "/u/g/gesslar/two",     // 有问题的文件
                "/u/g/gesslar/three",   // 正常文件
            }) ;

            foreach(file in files) load_object(file) ;
        } ;

        if(err) printf("ERR: %O", err) ;
    }

    // ERR: "*Error in loading object '/u/g/gesslar/two'"

    // 捕获到的值不一定是错误消息。throw() 会把收到的值原样交回，
    // 因此用一个类可以表示一种结构清晰的失败，捕获方能逐个字段
    // 检查它，而不必去解析字符串。
    class failure {
        string kind ;
        mixed detail ;
    }

    private void charge(object who, int amount) {
        int have = who->query_coins() ;

        if(have < amount)
            throw(new(class failure,
                      kind: "insufficient_funds",
                      detail: amount - have)) ;

        who->add_coins(-amount) ;
    }

    void example3(object who, int price) {
        mixed err = catch( charge(who, price) ) ;

        // 必须先判断 classp()：err 同样有可能是驱动程序的错误字符串，
        // 这种情况下 && 会在这里短路。
        if(classp(err) && err.kind == "insufficient_funds")
            write("你还差 " + err.detail + " 个金币。\n") ;
        else if(err)
            throw(err) ;    // 不该由我们处理 —— 继续往上传
    }
    ```

### 参考

    error(3), throw(3), error_handler(4), time_expression(3)

### 翻译 ###

    雪风(i@mud.ren)

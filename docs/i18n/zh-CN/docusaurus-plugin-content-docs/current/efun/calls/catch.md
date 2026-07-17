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

    外部函数 throw() 用来马上抛出一个错误并返回非零值，可以和 catch
    配合使用。

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
    ```

### 参考

    error(3), throw(3), error_handler(4), time_expression(3)

### 翻译 ###

    雪风(i@mud.ren)

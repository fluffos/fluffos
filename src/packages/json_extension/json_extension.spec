/**
 * 将 LPC 数据结构编码为 JSON 字符串
 *
 * @example
 * // 基础类型编码
 * string json = json_encode(42);                    // "42"
 * json = json_encode(3.14);                         // "3.14"
 * json = json_encode("hello");                      // "hello"
 * json = json_encode(({1, 2, 3}));                 // "[1,2,3]"
 * json = json_encode((["a": 1, "b": 2]));          // "{"a":1,"b":2}"
 *
 * @example
 * // 复杂嵌套结构
 * mixed data = ([
 *     "name": "张三",
 *     "level": 50,
 *     "skills": ({ "剑法", "内功", "轻功" }),
 *     "inventory": ([
 *         "金创药": 10,
 *         "小还丹": 5
 *     ])
 * ]);
 * string json = json_encode(data);
 *
 * @example
 * // 对象编码
 * json = json_encode(this_object());  // "path/to/object"
 *
 * @example
 * // 循环引用检测
 * mapping m = (["a": 1]);
 * m["self"] = m;
 * json = json_encode(m);  // 值为 "<circular_ref_mapping>"
 *
 * @param data 要编码的 LPC 数据，支持以下类型：
 *             - int/float: 编码为 JSON 数字
 *             - string: 编码为 JSON 字符串
 *             - array: 编码为 JSON 数组
 *             - mapping: 编码为 JSON 对象（键必须是 string 或 int）
 *             - object: 编码为对象文件名字符串
 *             - 其他类型: 编码为 JSON null
 *
 * @return JSON 格式的字符串，失败返回 "0"
 *
 * @note 编码限制（安全保护）：
 *       - 最大嵌套深度: 128 层
 *       - 最大数组元素: 10,000,000 个
 *       - 最大 mapping 条目: 5,000,000 个
 *       - 超过限制时会截断并输出调试信息
 *
 * @note 特殊行为：
 *       - 检测并标记循环引用为 "<circular_ref_*>" 字符串
 *       - mapping 的键支持 string 和 int 类型
 *       - destructed 对象编码为 null
 *
 * @see json_decode
 * @see write_json
 */
string json_encode(mixed data);

/**
 * 将 JSON 字符串解码为 LPC 数据结构
 *
 * @example
 * // 基础类型解码
 * mixed data = json_decode("42");          // 42
 * data = json_decode("3.14");              // 3.14
 * data = json_decode("\"hello\"");         // "hello"
 * data = json_decode("true");              // 1
 * data = json_decode("false");             // 0
 * data = json_decode("null");              // 0
 *
 * @example
 * // 数组解码
 * data = json_decode("[1, 2, 3]");         // ({1, 2, 3})
 * data = json_decode("[1, "mixed", 3.14]"); // ({1, "mixed", 3.14})
 *
 * @example
 * // 对象解码
 * data = json_decode("{\"a\":1,\"b\":2}"); // (["a": 1, "b": 2])
 *
 * @example
 * // 复杂嵌套结构
 * string json = '{
 *     "name": "李四",
 *     "age": 30,
 *     "items": [{"name": "剑", "damage": 100}]
 * }';
 * data = json_decode(json);
 *
 * @example
 * // 错误处理
 * data = json_decode("");                  // 0
 * data = json_decode("invalid json");      // 0
 * data = json_decode("{\"broken\"");       // 0 (语法错误)
 *
 * @param json_str 要解码的 JSON 格式字符串
 *
 * @return 解码后的 LPC 数据，失败返回 0：
 *         - JSON null/布尔值 → LPC int (0/1)
 *         - JSON 数字 → LPC int 或 float
 *         - JSON 字符串 → LPC string
 *         - JSON 数组 → LPC array
 *         - JSON 对象 → LPC mapping
 *
 * @note 解码限制（安全保护）：
 *       - 最大嵌套深度: 128 层
 *       - 最大数组元素: 10,000,000 个
 *       - 最大对象属性: 5,000,000 个
 *       - 最大字符串长度: 64 MB
 *       - 超过限制时会截断并输出调试信息
 *
 * @note JSON 对象的键始终解码为 LPC string
 *
 * @see json_encode
 * @see read_json
 */
mixed json_decode(string json_str);

/**
 * 将 LPC 数据结构以 JSON 格式写入文件
 *
 * @example
 * // 写入简单数据
 * int success = write_json("/data/player.json", ([
 *     "name": "王五",
 *     "level": 60,
 *     "hp": 1000
 * ]));
 * if (success) {
 *     write("数据保存成功\n");
 * }
 *
 * @example
 * // 写入复杂嵌套结构
 * mixed guild_data = ([
 *     "guild_name": "丐帮",
 *     "members": ({
 *         (["name": "洪七公", "generation": 18]),
 *         (["name": "黄蓉", "generation": 19])
 *     }),
 *     "skills": (["打狗棒法": 100, "降龙十八掌": 200])
 * ]);
 * write_json("/data/guild.json", guild_data);
 *
 * @example
 * // 错误处理
 * success = write_json("/invalid/path/file.json", data);  // 返回 0
 *
 * @param filename 文件路径（相对于 MUD 根目录或绝对路径）
 * @param data 要写入的 LPC 数据结构（支持类型见 json_encode）
 *
 * @return 成功返回 1，失败返回 0：
 *         - 路径无效: 返回 0
 *         - 权限不足: 返回 0
 *         - 编码失败: 返回 0
 *         - 写入成功: 返回 1
 *
 * @note 文件以二进制模式写入，确保 UTF-8 编码正确
 * @note 会覆盖同名文件（如果存在）
 * @note 路径会经过 check_valid_path 安全检查
 *
 * @see json_encode
 * @see read_json
 */
int write_json(string filename, mixed data);

/**
 * 从文件读取 JSON 并解码为 LPC 数据结构
 *
 * @example
 * // 读取玩家数据
 * mixed player_data = read_json("/data/player.json");
 * if (mappingp(player_data)) {
 *     printf("玩家: %s, 等级: %d\n",
 *            player_data["name"], player_data["level"]);
 * }
 *
 * @example
 * // 读取配置文件
 * mixed config = read_json("/etc/config.json");
 * if (config) {
 *     foreach (string key, mixed value in config) {
 *         printf("%s: %O\n", key, value);
 *     }
 * }
 *
 * @example
 * // 错误处理
 * mixed data = read_json("/nonexistent/file.json");  // 返回 0
 * data = read_json("/data/invalid.json");            // 返回 0（解析失败）
 *
 * @param filename 文件路径（相对于 MUD 根目录或绝对路径）
 *
 * @return 解码后的 LPC 数据，失败返回 0：
 *         - 文件不存在: 返回 0
 *         - 路径无效: 返回 0
 *         - 权限不足: 返回 0
 *         - 文件过大: 返回 0
 *         - JSON 格式错误: 返回 0
 *         - 成功: 返回解码后的数据
 *
 * @note 文件读取限制：
 *       - 最大文件大小: 256 MB
 *       - 文件以二进制模式读取
 *       - 使用 INSITU 模式优化性能
 *
 * @note 路径会经过 check_valid_path 安全检查
 * @note 解码限制见 json_decode 函数说明
 *
 * @see json_decode
 * @see write_json
 */
mixed read_json(string filename);

/**
 * 按 mapping 的值进行排序，返回排序后的键数组（通用版本）
 *
 * 根据值的大小对 mapping 进行排序，支持浮点数、整数字符串数值。
 * 排序是确定性的，相同值的键会按内容排序。
 *
 * @example
 * // 基础升序排序（默认）
 * mapping m = (["sword": 100, "axe": 50, "bow": 75]);
 * mixed *keys = sort_mapping(m);
 * // 结果: ({"axe", "bow", "sword"})  按值 50, 75, 100
 *
 * @example
 * // 降序排序
 * keys = sort_mapping(m, 1);
 * // 结果: ({"sword", "bow", "axe"})  按值 100, 75, 50
 *
 * @example
 * // 浮点数排序
 * m = (["player_a": 99.5, "player_b": 87.3, "player_c": 95.8]);
 * keys = sort_mapping(m);  // 按分数升序
 *
 * @example
 * // 字符串数值排序（可解析为数字的字符串）
 * m = (["a": "100", "b": "50.5", "c": "abc"]);
 * keys = sort_mapping(m);
 * // "abc" 无法解析为数字，权重为 -INFINITY，排在最前
 * // 结果: ({"c", "b", "a"})  按权重 -∞, 50.5, 100
 *
 * @example
 * // 排行榜应用
 * mapping rankings = ([
 *     "张三": 1580,
 *     "李四": 1850,
 *     "王五": 1720
 * ]);
 * mixed *top_players = sort_mapping(rankings, 1)[0..2];
 * // 取前三名: ({"李四", "王五", "张三"})
 *
 * @param map 要排序的 mapping
 * @param flag 排序方向（可选参数）
 *             - 0 或省略: 升序（从小到大）
 *             - 1: 降序（从大到小）
 *
 * @return 排序后的键数组（array），类型为 mixed*
 *
 * @note 值类型支持：
 *       - int: 直接比较
 *       - float: 直接比较
 *       - string: 尝试解析为数字（strtod），失败则为 -INFINITY
 *       - 其他类型: 权重为 -INFINITY
 *
 * @note 确定性排序：
 *       - 相同权重的键会按键的内容进行二次排序
 *       - 确保多次调用结果一致
 *
 * @note 性能：时间复杂度 O(n log n)，n 为 mapping 的条目数
 *
 * @see sort_mapping_int（仅支持整数，性能稍优）
 */
mixed *sort_mapping(mapping map, int flag | void);

/**
 * 按 mapping 的整数值进行排序，返回排序后的键数组（极速整数版本）
 *
 * 专为整数优化的排序函数，只支持值为整数的 mapping。
 * 相比 sort_mapping 性能更优，适合超大规模数据排序。
 *
 * @example
 * // 基础升序排序（默认）
 * mapping scores = (["a": 100, "b": 50, "c": 75]);
 * mixed *keys = sort_mapping_int(scores);
 * // 结果: ({"b", "c", "a"})  按值 50, 75, 100
 *
 * @example
 * // 降序排序
 * keys = sort_mapping_int(scores, 1);
 * // 结果: ({"a", "c", "b"})  按值 100, 75, 50
 *
 * @example
 * // 玩家等级排序
 * mapping players = ([
 *     "张三": 85,
 *     "李四": 92,
 *     "王五": 78,
 *     "赵六": 88
 * ]);
 * mixed *sorted = sort_mapping_int(players, 1);
 * // 降序: ({"李四", "赵六", "张三", "王五"})
 *
 * @example
 * // 非整数值处理
 * mapping m = (["a": 100, "b": "text", "c": 50.5]);
 * sorted = sort_mapping_int(m);
 * // "b" 和 "c" 的值非整数，视为 0
 * // 结果取决于各键的字母顺序
 *
 * @param map 要排序的 mapping（值应为整数）
 * @param flag 排序方向（可选参数）
 *             - 0 或省略: 升序（从小到大）
 *             - 1: 降序（从大到小）
 *
 * @return 排序后的键数组（array），类型为 mixed*
 *
 * @note 值类型限制：
 *       - 只支持 T_NUMBER 类型的值
 *       - 非整数类型的值会被视为 0
 *       - 浮点数会被截断为整数（非预期行为，建议值是整数时使用）
 *
 * @note 性能优势：
 *       - 避免浮点数解析和比较
 *       - 整数比较更快
 *       - 适合超大规模数据（百万级条目）
 *
 * @note 确定性排序：相同值的按键的内容进行二次排序
 *
 * @note 如果值包含浮点数或字符串数值，请使用 sort_mapping
 *
 * @see sort_mapping（支持浮点数和字符串数值）
 */
mixed *sort_mapping_int(mapping map, int flag | void);
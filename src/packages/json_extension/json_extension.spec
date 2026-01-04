// 安全编码 JSON
string json_encode(mixed);
// 安全解码 JSON
mixed json_decode(string);
// 安全写入 JSON 
int write_json(string, mixed);
// 安全读取 JSON
mixed read_json(string);
// 通用排序（支持 string/float/int）
// flag: 0=升序（默认）, 1=降序
mixed *sort_mapping(mapping, int | void);
// 极速整数排序（仅支持 int，用于超大规模数据）
// flag: 0=升序（默认）, 1=降序
mixed *sort_mapping_int(mapping, int | void);
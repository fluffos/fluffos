/**
 * json.c
 *
 * LPC support functions for JSON serialization and deserialization.
 * Attempts to be compatible with reasonably current FluffOS and LDMud
 * drivers, with at least a gesture or two toward compatibility with
 * older drivers.
 *
 *
 * mixed json_decode(string text)
 *     Deserializes JSON into an LPC value.
 *
 * string json_encode(mixed value)
 *     Serializes an LPC value into JSON text.
 *
 * v1.0: initial release
 * v1.0.1: fix for handling of \uXXXX on FLUFFOS
 * v1.0.2: define array keyword for LDMud & use it consistently
 * v1.0.3: fix for empty data structures
 * v1.0.4: Removed array keyword. (Yucong Sun)
 * v1.0.5: Fix decoding number 0.
 *
 * LICENSE
 *
 * The MIT License (MIT)
 *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __STD_JSON_H
#define __STD_JSON_H

#define to_string(x)                ("" + (x))

#define JSON_DECODE_PARSE_TEXT      0
#define JSON_DECODE_PARSE_POS       1
#define JSON_DECODE_PARSE_LINE      2
#define JSON_DECODE_PARSE_CHAR      3
#define JSON_DECODE_PARSE_FIELDS    4

private mixed json_decode_parse_value(mixed* parse);
private varargs mixed json_decode_parse_string(mixed* parse, int initiator_checked);

private void json_decode_parse_next_char(mixed* parse) {
    parse[JSON_DECODE_PARSE_POS]++;
    parse[JSON_DECODE_PARSE_CHAR]++;
}

private void json_decode_parse_next_chars(mixed* parse, int num) {
    parse[JSON_DECODE_PARSE_POS] += num;
    parse[JSON_DECODE_PARSE_CHAR] += num;
}

private void json_decode_parse_next_line(mixed* parse) {
    parse[JSON_DECODE_PARSE_POS]++;
    parse[JSON_DECODE_PARSE_LINE]++;
    parse[JSON_DECODE_PARSE_CHAR] = 1;
}

private void json_decode_skip_whitespaces(mixed* parse) {
    int ch;
    while(1) {
      json_decode_parse_next_char(parse);
      ch = parse[JSON_DECODE_PARSE_TEXT][parse[JSON_DECODE_PARSE_POS]];
      if (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t') {
        continue;
      } else {
        return ;
      }
    }
}

private int json_decode_hexdigit(int ch) {
    switch(ch) {
    case '0'    :
        return 0;
    case '1'    :
    case '2'    :
    case '3'    :
    case '4'    :
    case '5'    :
    case '6'    :
    case '7'    :
    case '8'    :
    case '9'    :
        return ch - '0';
    case 'a'    :
    case 'A'    :
        return 10;
    case 'b'    :
    case 'B'    :
        return 11;
    case 'c'    :
    case 'C'    :
        return 12;
    case 'd'    :
    case 'D'    :
        return 13;
    case 'e'    :
    case 'E'    :
        return 14;
    case 'f'    :
    case 'F'    :
        return 15;
    }
    return -1;
}

private varargs int json_decode_parse_at_token(mixed* parse, string token, int start) {
    int i, j;
    for(i = start, j = strlen(token); i < j; i++)
        if(parse[JSON_DECODE_PARSE_TEXT][parse[JSON_DECODE_PARSE_POS] + i] != token[i])
            return 0;
    return 1;
}

private varargs void json_decode_parse_error(mixed* parse, string msg, int ch) {
    if(ch)
        msg = sprintf("%s, '%c'", msg, ch);
    msg = sprintf("%s @ line %d char %d\n", msg, parse[JSON_DECODE_PARSE_LINE], parse[JSON_DECODE_PARSE_CHAR]);
    error(msg);
}

// Inline whitespace skipping for better performance
private void json_decode_skip_ws(mixed* parse) {
    int ch;
    int pos = parse[JSON_DECODE_PARSE_POS];

    while(1) {
        ch = parse[JSON_DECODE_PARSE_TEXT][pos];
        switch(ch) {
        case ' ':
        case '\t':
        case '\r':
            pos++;
            parse[JSON_DECODE_PARSE_CHAR]++;
            break;
        case '\n':
            pos++;
            parse[JSON_DECODE_PARSE_LINE]++;
            parse[JSON_DECODE_PARSE_CHAR] = 1;
            break;
        case 0x0c:
            pos++;
            parse[JSON_DECODE_PARSE_LINE]++;
            parse[JSON_DECODE_PARSE_CHAR] = 1;
            break;
        default:
            parse[JSON_DECODE_PARSE_POS] = pos;
            return;
        }
    }
}

private mixed json_decode_parse_object(mixed* parse) {
    mapping out = ([]);
    mixed key, value;
    int ch;
    int pos;

    // Skip opening brace
    pos = parse[JSON_DECODE_PARSE_POS] + 1;
    parse[JSON_DECODE_PARSE_POS] = pos;
    parse[JSON_DECODE_PARSE_CHAR]++;

    // Check for empty object
    json_decode_skip_ws(parse);
    if(parse[JSON_DECODE_PARSE_TEXT][parse[JSON_DECODE_PARSE_POS]] == '}') {
        parse[JSON_DECODE_PARSE_POS]++;
        parse[JSON_DECODE_PARSE_CHAR]++;
        return out;
    }

    while(1) {
        // Skip whitespace before key
        json_decode_skip_ws(parse);

        // Parse key
        key = json_decode_parse_string(parse);

        // Skip whitespace and find colon
        json_decode_skip_ws(parse);
        ch = parse[JSON_DECODE_PARSE_TEXT][parse[JSON_DECODE_PARSE_POS]];
        if(ch != ':') {
            if(ch == 0)
                json_decode_parse_error(parse, "Unexpected end of data");
            json_decode_parse_error(parse, "Expected ':' after object key", ch);
        }
        parse[JSON_DECODE_PARSE_POS]++;
        parse[JSON_DECODE_PARSE_CHAR]++;

        // Parse value
        value = json_decode_parse_value(parse);
        out[key] = value;

        // Skip whitespace and check for comma or closing brace
        json_decode_skip_ws(parse);
        ch = parse[JSON_DECODE_PARSE_TEXT][parse[JSON_DECODE_PARSE_POS]];

        if(ch == '}') {
            parse[JSON_DECODE_PARSE_POS]++;
            parse[JSON_DECODE_PARSE_CHAR]++;
            return out;
        }

        if(ch == ',') {
            parse[JSON_DECODE_PARSE_POS]++;
            parse[JSON_DECODE_PARSE_CHAR]++;
            continue;
        }

        if(ch == 0)
            json_decode_parse_error(parse, "Unexpected end of data");
        json_decode_parse_error(parse, "Expected ',' or '}' in object", ch);
    }
}

private mixed json_decode_parse_array(mixed* parse) {
    mixed* out;
    mixed* values = ({});
    mixed value;
    int ch;
    int count = 0;

    // Skip opening bracket
    parse[JSON_DECODE_PARSE_POS]++;
    parse[JSON_DECODE_PARSE_CHAR]++;

    // Check for empty array
    json_decode_skip_ws(parse);
    if(parse[JSON_DECODE_PARSE_TEXT][parse[JSON_DECODE_PARSE_POS]] == ']') {
        parse[JSON_DECODE_PARSE_POS]++;
        parse[JSON_DECODE_PARSE_CHAR]++;
        return ({});
    }

    // Pre-allocate with reasonable initial capacity
    values = allocate(16);

    while(1) {
        // Parse value
        value = json_decode_parse_value(parse);

        // Store value, growing array if needed
        if(count >= sizeof(values)) {
            // Double the capacity
            values = values + allocate(sizeof(values));
        }
        values[count++] = value;

        // Skip whitespace and check for comma or closing bracket
        json_decode_skip_ws(parse);
        ch = parse[JSON_DECODE_PARSE_TEXT][parse[JSON_DECODE_PARSE_POS]];

        if(ch == ']') {
            parse[JSON_DECODE_PARSE_POS]++;
            parse[JSON_DECODE_PARSE_CHAR]++;
            // Trim to actual size
            if(count < sizeof(values)) {
                return values[0..count-1];
            }
            return values;
        }

        if(ch == ',') {
            parse[JSON_DECODE_PARSE_POS]++;
            parse[JSON_DECODE_PARSE_CHAR]++;
            continue;
        }

        if(ch == 0)
            json_decode_parse_error(parse, "Unexpected end of data");
        json_decode_parse_error(parse, "Expected ',' or ']' in array", ch);
    }
}

private varargs mixed json_decode_parse_string(mixed* parse, int initiator_checked) {
    int pos, ch;
    buffer result;
    int result_len = 0;
    int has_escapes = 0;

    if(!initiator_checked) {
        ch = parse[JSON_DECODE_PARSE_TEXT][parse[JSON_DECODE_PARSE_POS]];
        if(!ch)
            json_decode_parse_error(parse, "Unexpected end of data");
        if(ch != '"')
            json_decode_parse_error(parse, "Unexpected character", ch);
    }
    json_decode_parse_next_char(parse);

    // Allocate initial buffer - will grow if needed
    result = allocate_buffer(256);
    pos = parse[JSON_DECODE_PARSE_POS];

    while(1) {
        ch = parse[JSON_DECODE_PARSE_TEXT][pos];

        switch(ch) {
        case 0:
            json_decode_parse_error(parse, "Unexpected end of data");

        case '"':
            // End of string
            parse[JSON_DECODE_PARSE_POS] = pos;
            json_decode_parse_next_char(parse);

            if(result_len == 0) {
                return "";
            }

            // Resize buffer to exact size and decode
            if(result_len < sizeof(result)) {
                result = result[0..result_len-1];
            }
            return string_decode(result, "utf-8");

        case '\\':
            // Escape sequence
            has_escapes = 1;
            pos++;
            ch = parse[JSON_DECODE_PARSE_TEXT][pos];

            switch(ch) {
            case 0:
                json_decode_parse_error(parse, "Unexpected end of data");
            case '"':
                result[result_len++] = '"';
                pos++;
                break;
            case '\\':
                result[result_len++] = '\\';
                pos++;
                break;
            case '/':
                result[result_len++] = '/';
                pos++;
                break;
            case 'b':
                result[result_len++] = '\b';
                pos++;
                break;
            case 'f':
                result[result_len++] = 0x0c;
                pos++;
                break;
            case 'n':
                result[result_len++] = '\n';
                pos++;
                break;
            case 'r':
                result[result_len++] = '\r';
                pos++;
                break;
            case 't':
                result[result_len++] = '\t';
                pos++;
                break;
            case 'u':
                // Unicode escape \uXXXX
                pos++;
                {
                    int code = 0;
                    int digit;

                    for(int i = 0; i < 4; i++) {
                        ch = parse[JSON_DECODE_PARSE_TEXT][pos++];
                        digit = json_decode_hexdigit(ch);
                        if(digit == -1)
                            json_decode_parse_error(parse, "Invalid hex digit", ch);
                        code = (code << 4) | digit;
                    }

                    // Check for UTF-16 surrogate pair
                    if((code & 0xfffff800) == 0xd800) {
                        // High surrogate, need low surrogate
                        int high = code;
                        int low;

                        if(parse[JSON_DECODE_PARSE_TEXT][pos] != '\\' ||
                           parse[JSON_DECODE_PARSE_TEXT][pos+1] != 'u') {
                            json_decode_parse_error(parse, "Invalid string, missing surrogate pair");
                        }

                        pos += 2; // Skip \u
                        low = 0;
                        for(int i = 0; i < 4; i++) {
                            ch = parse[JSON_DECODE_PARSE_TEXT][pos++];
                            digit = json_decode_hexdigit(ch);
                            if(digit == -1)
                                json_decode_parse_error(parse, "Invalid hex digit", ch);
                            low = (low << 4) | digit;
                        }

                        code = 0x10000 + (high - 0xd800) * 0x400 + (low - 0xDC00);
                    }

                    // Use sprintf("%c") which properly handles Unicode via ICU
                    // Then convert the UTF-8 string to raw bytes for the buffer
                    {
                        string utf_str;
                        buffer utf_bytes;
                        int i, len;

                        // sprintf handles Unicode codepoints correctly
                        utf_str = sprintf("%c", code);

                        // Convert UTF-8 string to raw bytes
                        utf_bytes = string_encode(utf_str, "utf-8");
                        len = sizeof(utf_bytes);

                        // Ensure buffer has enough space
                        if(result_len + len > sizeof(result))
                            result = result + allocate_buffer(256);

                        // Copy UTF-8 bytes to result buffer
                        for(i = 0; i < len; i++)
                            result[result_len++] = utf_bytes[i];
                    }
                }
                break;
            default:
                json_decode_parse_error(parse, "Invalid escape sequence", ch);
            }
            break;

        default:
            // Regular character
            if(result_len >= sizeof(result)) {
                result = result + allocate_buffer(256);
            }
            result[result_len++] = ch;
            pos++;
            break;
        }
    }
}

private mixed json_decode_parse_number(mixed* parse) {
    int pos = parse[JSON_DECODE_PARSE_POS];
    int from = pos;
    int has_dot = 0;
    int has_exp = 0;
    int ch, next_ch;
    buffer num_buf;
    int num_len = 0;

    // Allocate buffer for number
    num_buf = allocate_buffer(32);

    ch = parse[JSON_DECODE_PARSE_TEXT][pos];

    // Handle negative sign
    if (ch == '-') {
        num_buf[num_len++] = ch;
        pos++;
        next_ch = parse[JSON_DECODE_PARSE_TEXT][pos];
        if(!next_ch) json_decode_parse_error(parse, "Unexpected end of data");
        if(next_ch < '0' || next_ch > '9')
            json_decode_parse_error(parse, "Unexpected character", next_ch);
        ch = next_ch;
    }

    // Handle leading zero
    if (ch == '0') {
        num_buf[num_len++] = ch;
        pos++;
        next_ch = parse[JSON_DECODE_PARSE_TEXT][pos];

        if(next_ch == 0) {
            parse[JSON_DECODE_PARSE_POS] = pos;
            return 0;
        }

        if (next_ch == '.' || next_ch == 'e' || next_ch == 'E') {
            ch = next_ch;
        } else {
            // Check if followed by valid separator
            if((next_ch >= '0' && next_ch <= '9') || next_ch == '-')
                json_decode_parse_error(parse, "Unexpected character", next_ch);
            parse[JSON_DECODE_PARSE_POS] = pos;
            return 0;
        }
    }

    // Parse digits, decimal point, and exponent
    while(1) {
        ch = parse[JSON_DECODE_PARSE_TEXT][pos];

        switch(ch) {
        case '.':
            if(has_dot || has_exp)
                json_decode_parse_error(parse, "Unexpected character", ch);
            has_dot = 1;
            num_buf[num_len++] = ch;
            pos++;
            break;

        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            if(num_len >= sizeof(num_buf))
                num_buf = num_buf + allocate_buffer(32);
            num_buf[num_len++] = ch;
            pos++;
            break;

        case 'e':
        case 'E':
            if(has_exp)
                json_decode_parse_error(parse, "Unexpected character", ch);
            has_exp = 1;
            num_buf[num_len++] = ch;
            pos++;

            // Check for optional +/- after exponent
            ch = parse[JSON_DECODE_PARSE_TEXT][pos];
            if(ch == '+' || ch == '-') {
                num_buf[num_len++] = ch;
                pos++;
            }
            break;

        case '-':
        case '+':
            json_decode_parse_error(parse, "Unexpected character", ch);

        default:
            // End of number
            if(num_len == 0 || num_len == 1 && (num_buf[0] == '-' || num_buf[0] == '.'))
                json_decode_parse_error(parse, "Invalid number");

            parse[JSON_DECODE_PARSE_POS] = pos;

            // Trim buffer to actual size
            if(num_len < sizeof(num_buf))
                num_buf = num_buf[0..num_len-1];

            // Convert to int or float
            if(has_dot || has_exp)
                return to_float(string_decode(num_buf, "utf-8"));
            else
                return to_int(string_decode(num_buf, "utf-8"));
        }
    }
}

private mixed json_decode_parse_value(mixed* parse) {
    int ch;
    int pos;

    // Skip leading whitespace
    json_decode_skip_ws(parse);

    ch = parse[JSON_DECODE_PARSE_TEXT][parse[JSON_DECODE_PARSE_POS]];
    switch(ch) {
    case 0:
        json_decode_parse_error(parse, "Unexpected end of data");
    case '{':
        return json_decode_parse_object(parse);
    case '[':
        return json_decode_parse_array(parse);
    case '"':
        return json_decode_parse_string(parse, 1);
    case '-':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        return json_decode_parse_number(parse);
    case 't':
        // Parse "true"
        pos = parse[JSON_DECODE_PARSE_POS];
        if(parse[JSON_DECODE_PARSE_TEXT][pos] == 't' &&
           parse[JSON_DECODE_PARSE_TEXT][pos+1] == 'r' &&
           parse[JSON_DECODE_PARSE_TEXT][pos+2] == 'u' &&
           parse[JSON_DECODE_PARSE_TEXT][pos+3] == 'e') {
            parse[JSON_DECODE_PARSE_POS] = pos + 4;
            parse[JSON_DECODE_PARSE_CHAR] += 4;
            return 1;
        }
        json_decode_parse_error(parse, "Invalid token starting with 't'", ch);
    case 'f':
        // Parse "false"
        pos = parse[JSON_DECODE_PARSE_POS];
        if(parse[JSON_DECODE_PARSE_TEXT][pos] == 'f' &&
           parse[JSON_DECODE_PARSE_TEXT][pos+1] == 'a' &&
           parse[JSON_DECODE_PARSE_TEXT][pos+2] == 'l' &&
           parse[JSON_DECODE_PARSE_TEXT][pos+3] == 's' &&
           parse[JSON_DECODE_PARSE_TEXT][pos+4] == 'e') {
            parse[JSON_DECODE_PARSE_POS] = pos + 5;
            parse[JSON_DECODE_PARSE_CHAR] += 5;
            return 0;
        }
        json_decode_parse_error(parse, "Invalid token starting with 'f'", ch);
    case 'n':
        // Parse "null"
        pos = parse[JSON_DECODE_PARSE_POS];
        if(parse[JSON_DECODE_PARSE_TEXT][pos] == 'n' &&
           parse[JSON_DECODE_PARSE_TEXT][pos+1] == 'u' &&
           parse[JSON_DECODE_PARSE_TEXT][pos+2] == 'l' &&
           parse[JSON_DECODE_PARSE_TEXT][pos+3] == 'l') {
            parse[JSON_DECODE_PARSE_POS] = pos + 4;
            parse[JSON_DECODE_PARSE_CHAR] += 4;
            return 0;
        }
        json_decode_parse_error(parse, "Invalid token starting with 'n'", ch);
    default:
        json_decode_parse_error(parse, "Unexpected character", ch);
    }
}

private mixed json_decode_parse(mixed* parse) {
    mixed out = json_decode_parse_value(parse);

    // Skip trailing whitespace
    json_decode_skip_ws(parse);

    // Should be at end of input
    if(parse[JSON_DECODE_PARSE_TEXT][parse[JSON_DECODE_PARSE_POS]] != 0) {
        json_decode_parse_error(parse, "Unexpected character after value",
                                 parse[JSON_DECODE_PARSE_TEXT][parse[JSON_DECODE_PARSE_POS]]);
    }

    return out;
}

mixed json_decode(string text) {
    mixed* parse;
    buffer endl = allocate_buffer(1);
    endl[0] = 0;

    if(!text) {
      return 0;
    }

    parse = allocate(JSON_DECODE_PARSE_FIELDS);
    parse[JSON_DECODE_PARSE_TEXT] = string_encode(text, "utf-8") + endl;
    parse[JSON_DECODE_PARSE_POS] = 0;
    parse[JSON_DECODE_PARSE_CHAR] = 1;
    parse[JSON_DECODE_PARSE_LINE] = 1;
    return json_decode_parse(parse);
}

private string json_encode_string(string value) {
    buffer result;
    int result_len = 0;
    int len = strlen(value);
    int i, ch;

    // Allocate buffer with some headroom for escapes
    result = allocate_buffer(len * 2 + 2);
    result[result_len++] = '"';

    for(i = 0; i < len; i++) {
        ch = value[i];

        switch(ch) {
        case '"':
            if(result_len + 2 >= sizeof(result))
                result = result + allocate_buffer(256);
            result[result_len++] = '\\';
            result[result_len++] = '"';
            break;
        case '\\':
            if(result_len + 2 >= sizeof(result))
                result = result + allocate_buffer(256);
            result[result_len++] = '\\';
            result[result_len++] = '\\';
            break;
        case '/':
            if(result_len + 2 >= sizeof(result))
                result = result + allocate_buffer(256);
            result[result_len++] = '\\';
            result[result_len++] = '/';
            break;
        case '\b':
            if(result_len + 2 >= sizeof(result))
                result = result + allocate_buffer(256);
            result[result_len++] = '\\';
            result[result_len++] = 'b';
            break;
        case 0x0c:
            if(result_len + 2 >= sizeof(result))
                result = result + allocate_buffer(256);
            result[result_len++] = '\\';
            result[result_len++] = 'f';
            break;
        case '\n':
            if(result_len + 2 >= sizeof(result))
                result = result + allocate_buffer(256);
            result[result_len++] = '\\';
            result[result_len++] = 'n';
            break;
        case '\r':
            if(result_len + 2 >= sizeof(result))
                result = result + allocate_buffer(256);
            result[result_len++] = '\\';
            result[result_len++] = 'r';
            break;
        case '\t':
            if(result_len + 2 >= sizeof(result))
                result = result + allocate_buffer(256);
            result[result_len++] = '\\';
            result[result_len++] = 't';
            break;
        case 0x1b:
            if(result_len + 6 >= sizeof(result))
                result = result + allocate_buffer(256);
            result[result_len++] = '\\';
            result[result_len++] = 'u';
            result[result_len++] = '0';
            result[result_len++] = '0';
            result[result_len++] = '1';
            result[result_len++] = 'b';
            break;
        default:
            if(result_len + 1 >= sizeof(result))
                result = result + allocate_buffer(256);
            result[result_len++] = ch;
            break;
        }
    }

    result[result_len++] = '"';

    // Trim to actual size
    if(result_len < sizeof(result))
        result = result[0..result_len-1];

    return string_decode(result, "utf-8");
}

varargs string json_encode(mixed value, mixed* pointers) {
    if(undefinedp(value))
        return "null";
    if(intp(value) || floatp(value))
        return to_string(value);
    if(stringp(value)) {
        return json_encode_string(value);
    }
    if(mapp(value)) {
        string* parts = ({});
        int count = 0;
        string result;

        if(pointers) {
            // Don't recurse into circular data structures, output null for
            // their interior reference
            if(member_array(value, pointers) != -1)
                return "null";
            pointers += ({ value });
        } else {
            pointers = ({ value });
        }

        foreach(mixed k, mixed v in value) {
            // Non-string keys are skipped because the JSON spec requires that
            // object field names be strings.
            if(!stringp(k))
                continue;
            parts += ({ sprintf("%s:%s", json_encode_string(k), json_encode(v, pointers)) });
            count++;
        }

        if(count == 0)
            return "{}";

        return sprintf("{%s}", implode(parts, ","));
    }
    if(arrayp(value))
    {
        if(sizeof(value)) {
            string* parts = ({});

            if(pointers) {
                // Don't recurse into circular data structures, output null for
                // their interior reference
                if(member_array(value, pointers) != -1)
                    return "null";
                pointers += ({ value });
            } else {
                pointers = ({ value });
            }

            foreach(mixed v in value) {
                parts += ({ json_encode(v, pointers) });
            }

            return sprintf("[%s]", implode(parts, ","));
        } else {
            return "[]";
        }
    }
    // Values that cannot be represented in JSON are replaced by nulls.
    return "null";
}

#endif /* __STD_JSON_H */

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
 * v1.1.0: Adding unicode support for complex emoji, etc
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

/**
 * @function json_decode_parse_next_char
 * @description Advances the parse position by one character.
 * @param {mixed[]} parse - The parse state array.
 */
private void json_decode_parse_next_char(mixed* parse) {
    parse[JSON_DECODE_PARSE_POS]++;
    parse[JSON_DECODE_PARSE_CHAR]++;
}

/**
 * @function json_decode_parse_next_chars
 * @description Advances the parse position by the specified number of characters.
 * @param {mixed[]} parse - The parse state array.
 * @param {int} num - The number of characters to advance.
 */
private void json_decode_parse_next_chars(mixed* parse, int num) {
    parse[JSON_DECODE_PARSE_POS] += num;
    parse[JSON_DECODE_PARSE_CHAR] += num;
}

/**
 * @function json_decode_parse_next_line
 * @description Advances the parse position to the next line.
 * @param {mixed[]} parse - The parse state array.
 */
private void json_decode_parse_next_line(mixed* parse) {
    parse[JSON_DECODE_PARSE_POS]++;
    parse[JSON_DECODE_PARSE_LINE]++;
    parse[JSON_DECODE_PARSE_CHAR] = 1;
}

/**
 * @function json_decode_skip_whitespaces
 * @description Skips whitespace characters in the parse state.
 * @param {mixed[]} parse - The parse state array.
 */
private void json_decode_skip_whitespaces(mixed* parse) {
    int ch;
    while(1) {
      json_decode_parse_next_char(parse);
      ch = parse[JSON_DECODE_PARSE_TEXT][parse[JSON_DECODE_PARSE_POS]];
      if(ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t') {
        continue;
      } else {
        return ;
      }
    }
}

/**
 * @function json_decode_hexdigit
 * @description Converts a hexadecimal character to its integer value.
 * @param {int} ch - The hexadecimal character.
 * @returns {int} - The integer value of the hexadecimal character.
 */
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

/**
 * @function json_decode_parse_at_token
 * @description Checks if the parse position matches the specified token.
 * @param {mixed[]} parse - The parse state array.
 * @param {string} token - The token to check for.
 * @param {int} [start=0] - The starting position to check from.
 * @returns {int} - 1 if the token matches, otherwise 0.
 */
private varargs int json_decode_parse_at_token(mixed* parse, string token, int start) {
    int i, j;
    for(i = start, j = strlen(token); i < j; i++)
        if(parse[JSON_DECODE_PARSE_TEXT][parse[JSON_DECODE_PARSE_POS] + i] != token[i])
            return 0;
    return 1;
}

/**
 * @function json_decode_parse_error
 * @description Raises a parse error with the specified message and character.
 * @param {mixed[]} parse - The parse state array.
 * @param {string} msg - The error message.
 * @param {int} [ch] - The character causing the error.
 */
private varargs void json_decode_parse_error(mixed* parse, string msg, int ch) {
    if(ch)
        msg = sprintf("%s, '%c'", msg, ch);
    msg = sprintf("%s @ line %d char %d\n", msg, parse[JSON_DECODE_PARSE_LINE], parse[JSON_DECODE_PARSE_CHAR]);
    error(msg);
}

/**
 * @function json_decode_parse_object
 * @description Parses a JSON object from the given parse state.
 * @param {mixed[]} parse - The parse state array.
 * @returns {mapping} - The parsed JSON object.
 */
private mixed json_decode_parse_object(mixed* parse) {
    mapping out = ([]);
    int done = 0;
    mixed key, value;
    int found_non_whitespace, found_sep, found_comma;
    json_decode_parse_next_char(parse);
    if(parse[JSON_DECODE_PARSE_TEXT][parse[JSON_DECODE_PARSE_POS]] == '}') {
        done = 1;
        json_decode_parse_next_char(parse);
    }
    while(!done) {
        found_non_whitespace = 0;
        while(!found_non_whitespace) {
            switch(parse[JSON_DECODE_PARSE_TEXT][parse[JSON_DECODE_PARSE_POS]]) {
            case 0      :
                json_decode_parse_error(parse, "Unexpected end of data");
            case ' '    :
            case '\t'   :
            case '\r'   :
                json_decode_parse_next_char(parse);
                break;
            case 0x0c   :
            case '\n'   :
                json_decode_parse_next_line(parse);
                break;
            default     :
                found_non_whitespace = 1;
                break;
            }
        }
        key = json_decode_parse_string(parse);
        found_sep = 0;
        while(!found_sep) {
            int ch = parse[JSON_DECODE_PARSE_TEXT][parse[JSON_DECODE_PARSE_POS]];
            switch(ch) {
            case 0      :
                json_decode_parse_error(parse, "Unexpected end of data");
            case ':'    :
                found_sep = 1;
                json_decode_parse_next_char(parse);
                break;
            case ' '    :
            case '\t'   :
            case '\r'   :
                json_decode_parse_next_char(parse);
                break;
            case 0x0c   :
            case '\n'   :
                json_decode_parse_next_line(parse);
                break;
            default     :
                json_decode_parse_error(parse, "Unexpected character", ch);
            }
        }
        value = json_decode_parse_value(parse);
        found_comma = 0;
        while(!found_comma && !done) {
            int ch = parse[JSON_DECODE_PARSE_TEXT][parse[JSON_DECODE_PARSE_POS]];
            switch(ch) {
            case 0      :
                json_decode_parse_error(parse, "Unexpected end of data");
            case ','    :
                found_comma = 1;
                json_decode_parse_next_char(parse);
                break;
            case '}'    :
                done = 1;
                json_decode_parse_next_char(parse);
                break;
            case ' '    :
            case '\t'   :
            case '\r'   :
                json_decode_parse_next_char(parse);
                break;
            case 0x0c   :
            case '\n'   :
                json_decode_parse_next_line(parse);
                break;
            default     :
                json_decode_parse_error(parse, "Unexpected character", ch);
            }
        }
        out[key] = value;
    }
    return out;
}

/**
 * @function json_decode_parse_array
 * @description Parses a JSON array from the given parse state.
 * @param {mixed[]} parse - The parse state array.
 * @returns {mixed[]} - The parsed JSON array.
 */
private mixed json_decode_parse_array(mixed* parse) {
    mixed* out = ({});
    int done = 0;
    int found_comma;
    json_decode_parse_next_char(parse);
    if(parse[JSON_DECODE_PARSE_TEXT][parse[JSON_DECODE_PARSE_POS]] == ']') {
        done = 1;
        json_decode_parse_next_char(parse);
    }
    while(!done) {
        mixed value = json_decode_parse_value(parse);
        found_comma = 0;
        while(!found_comma && !done) {
            int ch = parse[JSON_DECODE_PARSE_TEXT][parse[JSON_DECODE_PARSE_POS]];
            switch(ch) {
            case 0      :
                json_decode_parse_error(parse, "Unexpected end of data");
            case ','    :
                found_comma = 1;
                json_decode_parse_next_char(parse);
                break;
            case ']'    :
                done = 1;
                json_decode_parse_next_char(parse);
                break;
            case ' '    :
            case '\t'   :
            case '\r'   :
                json_decode_parse_next_char(parse);
                break;
            case 0x0c   :
            case '\n'   :
                json_decode_parse_next_line(parse);
                break;
            default     :
                json_decode_parse_error(parse, "Unexpected character", ch);
            }
        }
        out += ({ value });
    }
    return out;
}

/**
 * @function json_decode_parse_string
 * @description Parses a JSON string from the given parse state.
 * @param {mixed[]} parse - The parse state array.
 * @param {int} [initiator_checked=0] - Whether the initiator has been checked.
 * @returns {string} - The parsed JSON string.
 */
private varargs mixed json_decode_parse_string(mixed* parse, int initiator_checked) {
    int from, to, esc_state, esc_active;
    string out;
    if(!initiator_checked) {
        int ch = parse[JSON_DECODE_PARSE_TEXT][parse[JSON_DECODE_PARSE_POS]];
        if(!ch)
            json_decode_parse_error(parse, "Unexpected end of data");
        if(ch != '"')
            json_decode_parse_error(parse, "Unexpected character", ch);
    }
    json_decode_parse_next_char(parse);
    from = parse[JSON_DECODE_PARSE_POS];
    to = -1;
    esc_state = 0;
    esc_active = 0;
    while(to == -1) {
        switch(parse[JSON_DECODE_PARSE_TEXT][parse[JSON_DECODE_PARSE_POS]]) {
        case 0          :
            json_decode_parse_error(parse, "Unexpected end of data");
        case '\\'       :
            esc_state = !esc_state;
            break;
        case '"'        :
            if(esc_state) {
                esc_state = 0;
                esc_active++;
            } else {
                to = parse[JSON_DECODE_PARSE_POS] - 1;
            }
            break;
        default         :
            if(esc_state) {
                esc_state = 0;
                esc_active++;
            }
            break;
        }
        json_decode_parse_next_char(parse);
    }
    out = string_decode(parse[JSON_DECODE_PARSE_TEXT][from .. to], "utf-8");
    if(esc_active) {
        if(member_array('"', out) != -1)
            out = replace_string(out, "\\\"", "\"");
        if(strsrch(out, "\\b") != -1)
            out = replace_string(out, "\\b", "\b");
        if(strsrch(out, "\\f") != -1)
            out = replace_string(out, "\\f", "\x0c");
        if(strsrch(out, "\\n") != -1)
            out = replace_string(out, "\\n", "\n");
        if(strsrch(out, "\\r") != -1)
            out = replace_string(out, "\\r", "\r");
        if(strsrch(out, "\\t") != -1)
            out = replace_string(out, "\\t", "\t");
        if(strsrch(out, "\\u") != -1) {
int i, k, character, next_character;
buffer utf8_buf;
string unicode_char;

i = 0;
while(i < strlen(out)) {
    if(out[i] == '\\' && i + 1 < strlen(out) && out[i + 1] == 'u') {
        if(i + 5 >= strlen(out)) {
            break;
        }

        sscanf(out[i+2..i+5], "%x", character);

        if(character >= 0xD800 && character <= 0xDBFF) {
            if(i + 11 < strlen(out) && out[i + 6] == '\\' && out[i + 7] == 'u') {
                sscanf(out[i+8..i+11], "%x", next_character);
                if(next_character >= 0xDC00 && next_character <= 0xDFFF) {
                    int codepoint = 0x10000 + ((character - 0xD800) << 10) + (next_character - 0xDC00);
                    utf8_buf = string_encode(sprintf("%c", codepoint), "UTF-8");
                    unicode_char = string_decode(utf8_buf, "UTF-8");
                    out = out[0..i-1] + unicode_char + out[i+12..];
                    i += strlen(unicode_char) - 1;
                } else {
                    utf8_buf = string_encode(sprintf("%c", character), "UTF-8");
                    unicode_char = string_decode(utf8_buf, "UTF-8");
                    out = out[0..i-1] + unicode_char + out[i+6..];
                    i += strlen(unicode_char) - 1;
                }
            } else {
                utf8_buf = string_encode(sprintf("%c", character), "UTF-8");
                unicode_char = string_decode(utf8_buf, "UTF-8");
                out = out[0..i-1] + unicode_char + out[i+6..];
                i += strlen(unicode_char) - 1;
            }
        } else {
            utf8_buf = string_encode(sprintf("%c", character), "UTF-8");
            unicode_char = string_decode(utf8_buf, "UTF-8");
            out = out[0..i-1] + unicode_char + out[i+6..];
            i += strlen(unicode_char) - 1;
        }
    }
    i++;
}
        }
        if(member_array('/', out) != -1)
            out = replace_string(out, "\\/", "/");
        if(member_array('\\', out) != -1)
            out = replace_string(out, "\\\\", "\\");
    }
    return out;
}

/**
 * @function json_decode_parse_number
 * @description Parses a JSON number from the given parse state.
 * @param {mixed[]} parse - The parse state array.
 * @returns {mixed} - The parsed JSON number.
 */
private mixed json_decode_parse_number(mixed* parse) {
    int from = parse[JSON_DECODE_PARSE_POS];
    int to = -1;
    int dot = -1;
    int exp = -1;
    int ch;
    int next_ch;
    string number;

    ch = parse[JSON_DECODE_PARSE_TEXT][parse[JSON_DECODE_PARSE_POS]];
    if(ch == '-') {
        next_ch = parse[JSON_DECODE_PARSE_TEXT][parse[JSON_DECODE_PARSE_POS] + 1];
        if(!next_ch) json_decode_parse_error(parse, "Unexpected end of data");
        if(next_ch < '0' || next_ch > '9')
            json_decode_parse_error(parse, "Unexpected character", next_ch);
        json_decode_parse_next_char(parse);
    }

    ch = parse[JSON_DECODE_PARSE_TEXT][parse[JSON_DECODE_PARSE_POS]];
    if(ch == '0') {
        // 0 can only either be an direct int value 0, or 0e or 0E
        next_ch = parse[JSON_DECODE_PARSE_TEXT][parse[JSON_DECODE_PARSE_POS] + 1];
        // 0 before EOF
        if(next_ch == 0) {
          json_decode_parse_next_char(parse);
          return 0;
        }
        // only valid char here are .eE, continue parse
        if(next_ch == '.' || next_ch == 'e' || next_ch == 'E') {
          json_decode_parse_next_char(parse);
        } else {
          // consume until next non-whitespace
          json_decode_skip_whitespaces(parse);
          next_ch = parse[JSON_DECODE_PARSE_TEXT][parse[JSON_DECODE_PARSE_POS]];
          // cannot continue to be number.
          if((next_ch >= '0' && next_ch <= '9') || next_ch == '-') json_decode_parse_error(parse, "Unexpected character", next_ch);
          return 0;
        }
    }
    while(to == -1) {
        ch = parse[JSON_DECODE_PARSE_TEXT][parse[JSON_DECODE_PARSE_POS]];
        switch(ch) {
        case '.'        :
            if(dot != -1 || exp != -1)
                json_decode_parse_error(parse, "Unexpected character", ch);
            dot = parse[JSON_DECODE_PARSE_POS];
            json_decode_parse_next_char(parse);
            break;
        case '0'        :
        case '1'        :
        case '2'        :
        case '3'        :
        case '4'        :
        case '5'        :
        case '6'        :
        case '7'        :
        case '8'        :
        case '9'        :
            json_decode_parse_next_char(parse);
            break;
        case 'e'        :
        case 'E'        :
            if(exp != -1)
                json_decode_parse_error(parse, "Unexpected character", ch);
            exp = parse[JSON_DECODE_PARSE_POS];
            json_decode_parse_next_char(parse);
            break;
        case '-'        :
        case '+'        :
            if(exp == parse[JSON_DECODE_PARSE_POS] - 1) {
                json_decode_parse_next_char(parse);
                break;
            }
            // Fallthrough
        default         :
            to = parse[JSON_DECODE_PARSE_POS] - 1;
            if(dot == to || to < from)
                json_decode_parse_error(parse, "Unexpected character", ch);
            break;
        }
    }
    number = string_decode(parse[JSON_DECODE_PARSE_TEXT][from .. to], "utf-8");
    if(dot != -1 || exp != -1)
        return to_float(number);
    else
        return to_int(number);
}

/**
 * @function json_decode_parse_value
 * @description Parses a JSON value from the given parse state.
 * @param {mixed[]} parse - The parse state array.
 * @returns {mixed} - The parsed JSON value.
 */
private mixed json_decode_parse_value(mixed* parse) {
    for(;;) {
        int ch;
        ch = parse[JSON_DECODE_PARSE_TEXT][parse[JSON_DECODE_PARSE_POS]];
        switch(ch) {
        case 0          :
            json_decode_parse_error(parse, "Unexpected end of data");
        case '{'        :
            return json_decode_parse_object(parse);
        case '['        :
            return json_decode_parse_array(parse);
        case '"'        :
            return json_decode_parse_string(parse, 1);
        case '-'        :
        case '0'        :
        case '1'        :
        case '2'        :
        case '3'        :
        case '4'        :
        case '5'        :
        case '6'        :
        case '7'        :
        case '8'        :
        case '9'        :
            return json_decode_parse_number(parse);
        case ' '        :
        case '\t'       :
        case '\r'       :
            json_decode_parse_next_char(parse);
            break;
        case 0x0c       :
        case '\n'       :
            json_decode_parse_next_line(parse);
            break;
        case 't'        :
            if(json_decode_parse_at_token(parse, "true", 1)) {
                json_decode_parse_next_chars(parse, 4);
                return 1;
            } else {
                json_decode_parse_error(parse, "Unexpected character", ch);
            }
        case 'f'        :
            if(json_decode_parse_at_token(parse, "false", 1)) {
                json_decode_parse_next_chars(parse, 5);
                return 0;
            } else {
                json_decode_parse_error(parse, "Unexpected character", ch);
            }
        case 'n'        :
            if(json_decode_parse_at_token(parse, "null", 1)) {
                json_decode_parse_next_chars(parse, 4);
                return 0;
            } else {
                json_decode_parse_error(parse, "Unexpected character", ch);
            }
        default         :
            json_decode_parse_error(parse, "Unexpected character", ch);
        }
    }
}

/**
 * @function json_decode_parse
 * @description Parses a JSON value from the given parse state.
 * @param {mixed[]} parse - The parse state array.
 * @returns {mixed} - The parsed JSON value.
 */
private mixed json_decode_parse(mixed* parse) {
    mixed out = json_decode_parse_value(parse);
    for(;;) {
        int ch = parse[JSON_DECODE_PARSE_TEXT][parse[JSON_DECODE_PARSE_POS]];
        switch(ch) {
        case 0          :
            return out;
        case ' '        :
        case '\t'       :
        case '\r'       :
            json_decode_parse_next_char(parse);
            break;
        case 0x0c       :
        case '\n'       :
            json_decode_parse_next_line(parse);
            break;
        default         :
            json_decode_parse_error(parse, "Unexpected character", ch);
        }
    }
    return 0;
}

/**
 * @simul_efun json_decode
 * @description Deserializes a JSON string into an LPC value.
 * @param {string} text - The JSON string to deserialize.
 * @returns {mixed} - The deserialized LPC value.
 */
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

private nosave nomask string unicode_pattern = "([^\\x00-\\x7F])" ;

/**
 * @simul_efun json_encode
 * @description Serializes an LPC value into a JSON string.
 * @param {mixed} value - The LPC value to serialize.
 * @param {mixed[]} [pointers] - An optional array of pointers to handle
 *                               circular references.
 * @returns {string} - The JSON string representation of the LPC value.
 */
varargs string json_encode(mixed value, mixed* pointers) {


    if(undefinedp(value))
        return "null";
    if(intp(value) || floatp(value))
        return to_string(value);
    if(stringp(value)) {
        if(member_array('"', value) != -1)
            value = replace_string(value, "\"", "\\\"");
        value = sprintf("\"%s\"", value);
        if(member_array('\\', value) != -1) {
            value = replace_string(value, "\\", "\\\\");
            if(strsrch(value, "\\\"") != -1)
                value = replace_string(value, "\\\"", "\"");
        }
        if(member_array('\b', value) != -1)
            value = replace_string(value, "\b", "\\b");
        if(member_array(0x0c, value) != -1)
            value = replace_string(value, "\x0c", "\\f");
        if(member_array('\n', value) != -1)
            value = replace_string(value, "\n", "\\n");
        if(member_array('\r', value) != -1)
            value = replace_string(value, "\r", "\\r");
        if(member_array('\t', value) != -1)
            value = replace_string(value, "\t", "\\t");
        if(member_array(0x1b, value) != -1)
          value = replace_string(value, "\x1b", "\\u001b");

        // Unicode handling
        while(pcre_match(value, unicode_pattern)) {
            int codepoint;
            string char_to_replace = pcre_extract(value, unicode_pattern)[0];
            buffer utf8_buf = string_encode(char_to_replace, "UTF-8");

            // Get the codepoint from the UTF-8 bytes
            if (sizeof(utf8_buf) == 1) {
                codepoint = utf8_buf[0];
            } else if (sizeof(utf8_buf) == 2) {
                codepoint = ((utf8_buf[0] & 0x1F) << 6) | (utf8_buf[1] & 0x3F);
            } else if (sizeof(utf8_buf) == 3) {
                codepoint = ((utf8_buf[0] & 0x0F) << 12) | ((utf8_buf[1] & 0x3F) << 6) | (utf8_buf[2] & 0x3F);
            } else if (sizeof(utf8_buf) == 4) {
                codepoint = ((utf8_buf[0] & 0x07) << 18) | ((utf8_buf[1] & 0x3F) << 12) |
                            ((utf8_buf[2] & 0x3F) << 6) | (utf8_buf[3] & 0x3F);
            }

            if (codepoint <= 0xFFFF) {
                value = pcre_replace(value, unicode_pattern, ({ sprintf("\\u%04X", codepoint) }));
            } else {
                // Encode as surrogate pair
                int high = 0xD800 + ((codepoint - 0x10000) >> 10);
                int low = 0xDC00 + ((codepoint - 0x10000) & 0x3FF);
                value = pcre_replace(value, unicode_pattern, ({ sprintf("\\u%04X\\u%04X", high, low) }));
            }
        }

        return value;
    }

    if(mapp(value)) {
        string out;
        int ix = 0;
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
            if(ix++)
                out = sprintf("%s,%s:%s", out, json_encode(k, pointers), json_encode(v, pointers));
            else
                out = sprintf("%s:%s", json_encode(k, pointers), json_encode(v, pointers));
        }
        if(!out || out == "")
            return "{}";
        return sprintf("{%s}", out);
    }
    if(arrayp(value))
    {
        if(sizeof(value)) {
            string out;
            int ix = 0;
            if(pointers) {
                // Don't recurse into circular data structures, output null for
                // their interior reference
                if(member_array(value, pointers) != -1)
                    return "null";
                pointers += ({ value });
            } else {
                pointers = ({ value });
            }
            foreach(mixed v in value)
                if(ix++)
                    out = sprintf("%s,%s", out, json_encode(v, pointers));
                else
                    out = json_encode(v, pointers);

            if(!out || out == "")
                return "[]";
            return sprintf("[%s]", out);
        } else {
            return "[]";
        }
    }
    // Values that cannot be represented in JSON are replaced by nulls.
    return "null";
}

#endif /* __STD_JSON_H */

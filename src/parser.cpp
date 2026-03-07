//
// Created by luolonghua on 2026/2/18.
//

#include "parser.h"

// 辅助函数，用于将 raw 字符串里的转义序列转换为实际字符
static std::string unescape_string(std::string_view raw) {
    std::string result;
    result.reserve(raw.size());

    for (size_t i = 0; i < raw.size(); ++i) {
        if (raw[i] == '\\' && i + 1 < raw.size()) {
            i++; // 跳过 '\'
            switch (raw[i]) {
                case '"':  result += '"';  break;
                case '\\': result += '\\'; break;
                case '/':  result += '/';  break;
                case 'b':  result += '\b'; break;
                case 'f':  result += '\f'; break;
                case 'n':  result += '\n'; break;
                case 'r':  result += '\r'; break;
                case 't':  result += '\t'; break;
                case 'u': {
                    auto parse_hex4 = [&](size_t pos) -> int {
                        if (pos + 4 > raw.size()) return -1;
                        int val = 0;
                        for (int j = 0; j < 4; ++j) {
                            char c = raw[pos + j];
                            val <<= 4;
                            if (c >= '0' && c <= '9') val |= (c - '0');
                            else if (c >= 'a' && c <= 'f') val |= (c - 'a' + 10);
                            else if (c >= 'A' && c <= 'F') val |= (c - 'A' + 10);
                            else return -1;
                        }
                        return val;
                    };

                    int cp = parse_hex4(i + 1);
                    if (cp != -1) {
                        i += 4;
                        // 处理 UTF-16 代理对
                        if (cp >= 0xD800 && cp <= 0xDBFF) {
                            if (i + 2 < raw.size() && raw[i + 1] == '\\' && raw[i + 2] == 'u') {
                                int cp2 = parse_hex4(i + 3);
                                if (cp2 >= 0xDC00 && cp2 <= 0xDFFF) {
                                    cp = (((cp - 0xD800) << 10) | (cp2 - 0xDC00)) + 0x10000;
                                    i += 6;
                                }
                            }
                        }
                        // UTF-8 编码转换 (支持 1-4 字节)
                        if (cp <= 0x7F) {
                            result += static_cast<char>(cp);
                        } else if (cp <= 0x7FF) {
                            result += static_cast<char>(0xC0 | (cp >> 6));
                            result += static_cast<char>(0x80 | (cp & 0x3F));
                        } else if (cp <= 0xFFFF) {
                            result += static_cast<char>(0xE0 | (cp >> 12));
                            result += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
                            result += static_cast<char>(0x80 | (cp & 0x3F));
                        } else {
                            result += static_cast<char>(0xF0 | (cp >> 18));
                            result += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
                            result += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
                            result += static_cast<char>(0x80 | (cp & 0x3F));
                        }
                    }
                    break;
                }
                default: result += raw[i]; break;
            }
        } else {
            result += raw[i];
        }
    }
    return result;
}


// 消费掉当前字符，并返回它，同时为下一次做好准备
// current 应该是 Scanner 刚刚吃掉的字符
char Scanner::advance() {
    char current = src[cursor];
    if (current == '\n' || current == '\r') {
        line++;
        column = 1;
    } else {
        column++;
    }
    cursor++;
    return current;
}

Token Scanner::next_token() {
    skip_whitespace(); // 自动跳过空格、换行
    if (cursor >= src.size()) return {TokenType::EndInp, "",line,column};

    char c = src[cursor];
    switch (c) {
        case '{': {
            int l = line;
            int col = column;
            advance();
            return {TokenType::BeginObject, "{", l, col};
        }
        case '}': {
            int l = line;
            int col = column;
            advance();
            return {TokenType::EndObject, "}", l, col};
        }
        case '[': {
            int l = line;
            int col = column;
            advance();
            return {TokenType::BeginArray, "[", l, col};
        }
        case ']': {
            int l = line;
            int col = column;
            advance();
            return {TokenType::EndArray, "]", l, col};
        }
        case ':': {
            int l = line;
            int col = column;
            advance();
            return {TokenType::Colon, ":", l, col};
        }
        case ',': {
            int l = line;
            int col = column;
            advance();
            return {TokenType::Comma, ",", l, col};
        }
    }


    // 字符串
    if (c == '"') {
        int l = line;          // 保存起始行列
        int col = column;
        advance();             // 消耗开头的 "

        size_t start = cursor; // 字符串内容开始位置
        while (cursor < src.size()) {
            if (src[cursor] == '\\') {
                advance(); // 消耗'\'
                if (cursor < src.size()) {
                    advance(); //消耗被转义的字符
                }
            }else if (src[cursor] == '"') {
                break; //未被转义的结束双引号，跳出
            }else {
                advance();//普通字符
            }
        }
        if (cursor >= src.size()) {
            // 这里可以用 l, col 报错，因为错误发生在开头的 " 处
            throw std::runtime_error(
               "Line " + std::to_string(l) +
               ", Col " + std::to_string(col) +
               ": Unterminated string"
           );
        }
        // 注意：这里拿到的 val 还是带有 '\' 和 'n' 两个字符的原始字符串
        std::string_view val = src.substr(start, cursor - start);
        advance();             // 消耗结束的 "
        return {TokenType::String, val, l, col};
    }

    // 数字
    if (std::isdigit(c) || c == '-' || c == '+') {
        int l = line;          // 保存起始行列
        int col = column;
        size_t start = cursor; // 记录起始索引
        advance();             // 消耗第一个字符

        while (cursor < src.size() && (std::isdigit(src[cursor]) || src[cursor] == '.')) {
            advance();
        }
        std::string_view val = src.substr(start, cursor - start);
        return {TokenType::Number, val, l, col};
    }

    // true
    if (src.substr(cursor, 4) == "true") {
        int l = line;
        int col = column;
        for (int i = 0; i < 4; ++i) advance(); // 用 advance 逐个消耗
        return {TokenType::True, "true", l, col};
    }

    // false
    if (src.substr(cursor, 5) == "false") {
        int l = line;
        int col = column;
        for (int i = 0; i < 5; ++i) advance();
        return {TokenType::False, "false", l, col};
    }

    // null
    if (src.substr(cursor, 4) == "null") {
        int l = line;
        int col = column;
        for (int i = 0; i < 4; ++i) advance();
        return {TokenType::Null, "null", l, col};
    }

    // 其他错误
    // 对于无法识别的字符，保存当前位置再抛出，方便定位
    int l = line;
    int col = column;
    advance(); // 也可以不 advance，但通常我们会消耗掉这个非法字符以避免无限循环
    throw std::runtime_error("Line " + std::to_string(l) +
                ", Col " + std::to_string(col) +
                ":Unexpected character" );
}


void Scanner::skip_whitespace() {
    while (cursor < src.size() && (src[cursor] == ' ' || src[cursor] == '\n' || src[cursor] == '\r' || src[cursor] == '\t')) {
        advance();
    }
}

// 核心入口
Json Parser::parse() {
    return parse_value();
}

void Parser::consume() {
    lookahead = scanner.next_token();
}

Json Parser::parse_array() {
    Json::array_type arr;
    consume(); // 吃掉 '['
    while (lookahead.type != TokenType::EndArray) {
        arr.push_back(parse_value());
        if (lookahead.type == TokenType::Comma) {
            consume(); // 跳过 ','
        }
    }
    consume(); // 吃掉 ']'
    return Json(arr);
}

Json Parser::parse_string() {
    //修改：使用 decode_string
    std::string val = unescape_string(lookahead.value);
    consume();// 消费掉这个 String Token
    return Json(val);
}

Json Parser::parse_number() {
    // 将 string_view 转为 string 再用 stod(string to double)  转为 double .
    double val = std::stod(std::string(lookahead.value));
    consume();
    return Json(val);
}

Json Parser::parse_value() {
    switch (lookahead.type) {
        case TokenType::BeginObject: return parse_object();
        case TokenType::BeginArray:  return parse_array();
        case TokenType::String:      return parse_string();
        case TokenType::Number:      return parse_number();
        case TokenType::True:        consume(); return Json(true);
        case TokenType::False:       consume(); return Json(false);
        case TokenType::Null:        consume(); return Json(nullptr);
        default: throw std::runtime_error("Line " + std::to_string(lookahead.line) +
                ", Col " + std::to_string(lookahead.column) +
                ": Unexpected token");
    }
}

// 解析对象 { "key" : value }
Json Parser::parse_object() {
    Json::object_type obj;
    consume(); // 吃掉 '{'

    while (lookahead.type != TokenType::EndObject) {
        // 1. 解析 Key
        if (lookahead.type != TokenType::String) {
            throw std::runtime_error(
                "Line " + std::to_string(lookahead.line) +
                ", Col " + std::to_string(lookahead.column) +
                ": Key must be a string"
                );
        }
        std::string key = unescape_string(lookahead.value);
        consume(); // 吃掉 Key

        // 2. 吃掉 ':'
        if (lookahead.type != TokenType::Colon) {
            throw std::runtime_error("Line " + std::to_string(lookahead.line) +
                ", Col " + std::to_string(lookahead.column) +
                ": Expected ':' after key");
        }
        consume();

        // 3. 递归解析 Value (这里就是最神奇的地方！)
        obj[key] = parse_value();

        // 4. 处理逗号
        if (lookahead.type == TokenType::Comma) {
            consume();
        }
    }
    consume(); // 吃掉 '}'
    return Json(obj);
}
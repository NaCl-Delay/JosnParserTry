#include <iostream>
#include <string>
#include "json.h"
#include <cassert>
#include <iomanip>
//
//
// // 辅助函数：以十六进制查看字符串内容，用于调试转义是否真正成功
// void print_hex(const std::string& s) {
//     for (unsigned char c : s) {
//         std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)c << " ";
//     }
//     std::cout << std::dec << std::endl;
// }
//
// void test_complex_escapes() {
//     std::cout << "--- Starting Complex Escape Test ---" << std::endl;
//
//     // 1. 定义一个极其复杂的 JSON 字符串
//     // 包含：
//     // - 标准转义：\n, \t, \r, \b, \f
//     // - 符号转义：\" , \\, \/
//     // - Unicode (中文): \u4e2d\u6587
//     // - Unicode (代理对/Emoji): \uD83D\uDE80 (火箭)
//     // - 复杂的对象嵌套
//     std::string complex_json = R"({
//         "status": "Success",
//         "controls": "Line1\nLine2\tTabbed\rCarriage",
//         "path": "C:\\Windows\\System32\\drivers",
//         "quote": "He said, \"JSON is awesome/cool\"",
//         "international": {
//             "chinese": "\u4e2d\u6587",
//             "emoji": "\uD83D\uDE80",
//             "mixed": "Hello \u4e2d\u6587 \uD83D\uDE00"
//         },
//         "array": ["\bBackspace", "\fFormfeed", "\\\\ Double Backslash"]
//     })";
//
//     try {
//         // --- 步骤 A: 解析原始 JSON ---
//         Json j = Json::parse(complex_json);
//
//         // --- 步骤 B: 验证解析后的内存值 ---
//
//         // 验证换行和制表符
//         std::string controls = j["controls"].as_string();
//         assert(controls.find('\n') != std::string::npos);
//         assert(controls.find('\t') != std::string::npos);
//
//         // 验证反斜杠（内存中应该是单个 \）
//         std::string path = j["path"].as_string();
//         assert(path == "C:\\Windows\\System32\\drivers");
//
//         // 验证中文解析 (UTF-8)
//         std::string chinese = j["international"]["chinese"].as_string();
//         // "中文" 的 UTF-8 编码是 E4 B8 AD E6 96 87
//         assert(chinese == "中文");
//
//         // 验证 Emoji (代理对组合)
//         std::string emoji = j["international"]["emoji"].as_string();
//         // 🚀 的 UTF-8 是 F0 9F 9A 80
//         assert(emoji == "🚀");
//
//         std::cout << "A. Parse Check: SUCCESS" << std::endl;
//         std::cout << "   - Decoded Emoji: " << emoji << std::endl;
//         std::cout << "   - Decoded Chinese: " << chinese << std::endl;
//
//         // --- 步骤 C: 测试序列化 (Dump) ---
//         // 序列化后，内存中的换行符应该重新变回字符 \n
//         std::string dumped = j.dump(true); // 使用 pretty print
//
//         // 检查生成的字符串里是否包含转义序列文本
//         assert(dumped.find("\\n") != std::string::npos);
//         assert(dumped.find("\\\"") != std::string::npos);
//         assert(dumped.find("\\\\") != std::string::npos);
//
//         std::cout << "B. Dump Check: SUCCESS" << std::endl;
//
//         // --- 步骤 D: 再次解析 (Round Trip) ---
//         // 确保 dump 出来的东西还能被正确解析回去，且内容一致
//         Json j_second = Json::parse(dumped);
//         assert(j_second["international"]["emoji"].as_string() == "🚀");
//         assert(j_second["path"].as_string() == j["path"].as_string());
//
//         std::cout << "C. Round Trip Check: SUCCESS" << std::endl;
//
//     } catch (const std::exception& e) {
//         std::cerr << "Test FAILED: " << e.what() << std::endl;
//         exit(1);
//     }
//
//     std::cout << "--- All Complex Escape Tests Passed! ---\n" << std::endl;
// }
//
// int main() {
// #ifdef _WIN32
//     system("chcp 65001"); // 将控制台设置为 UTF-8 编码
// #endif
//     test_complex_escapes();
//     return 0;
// }


int main() {
#ifdef _WIN32
system("chcp 65001"); // 将控制台设置为 UTF-8 编码
#endif


    // 1. 准备一个复杂的 JSON 字符串
    std::string raw_json = R"(
  {
        "name": "Gem\nini",
        "version": 3.1,
        "is_active": true,
        "features": ["parsing", "serialization", "recursive"],
        "metadat\na": {
            "author" : "NaCl-Delay",
            "year": 2026
        },
        "nil_val": null
    }
)";

    try {
        // 2. 调用解析接口
         Json data = Json::parse(raw_json);

        // 3. 验证数据访问
        std::cout << "--- 基础字段访问 ---" << std::endl;
        std::cout << "Name: " << data["name"].as_string() << std::endl;
        std::cout << "Version: " << data["version"].as_double() << std::endl;
        std::cout << "Is Active: " << (data["is_active"].as_bool() ? "Yes" : "No") << std::endl;

        std::cout << "\n--- 数组访问 ---" << std::endl;
        // 使用 operator[] 访问数组元素
        std::cout << "First Feature: " << data["features"][0].as_string() << std::endl;

        std::cout << "\n--- 嵌套对象访问 ---" << std::endl;
        // 链式访问 data["key"]["subkey"]
        std::cout << "Author: " << data["metadat\na"]["author"].as_string() << std::endl;

        std::cout << "\n--- 序列化测试 (dump) ---" << std::endl;
        // 测试我们刚刚写好的 dump 函数
        std::cout << data.dump(4) << std::endl;

    } catch (const std::exception& e) {
        // 捕获 Scanner 或 Parser 抛出的异常
        std::cerr << "解析出错: " << e.what() << std::endl;
    }

    return 0;
}
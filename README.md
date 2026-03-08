# SAST2026 - C++ 组寒假大作战项目：JSON 解析器

一个用 C++17 实现的轻量级 JSON 解析器，支持标准 JSON 的解析、数据访问和序列化。

本项目作为寒假 woc 任务，旨在深入理解 JSON 语法规范及 C++ 现代特性的实践应用。

---

## 试题要求

### 题目描述
实现一个 JSON 解析器，能够解析标准 JSON 格式的字符串，并提供便捷的数据访问接口。
### 功能要求

#### 【基础要求】

支持解析 JSON 的六种基本类型： null boolean (true/false) number (整数和浮点数) string (包含转义字符处理) array object

提供友好的数据访问 API

解析失败时提供有意义的错误信息（包含行号/列号）

支持将解析后的对象序列化回 JSON 字符串

#### 【进阶要求】

支持 JSON 注释（// 和 /* */）

支持尾随逗号

支持 JSON Pointer (RFC 6901) 查询

支持从文件读取解析

 实现迭代器支持，可用于范围 for 循环

【挑战要求】

 实现 SAX 风格的流式解析接口

性能优化：使用 string_view 减少拷贝

支持 JSON Schema 验证

#### 技术要求
可以参考第三方 JSON 库（如 nlohmann/json, rapidjson 等）

可以使用 STL 标准库

建议使用递归下降解析法

---

## 功能列表


### 基础功能
- 支持 JSON 六种基本类型：`null`、`boolean`、`number`、`string`、`array`、`object`
- 完整的转义字符处理：`\n`、`\t`、`\"`、`\\`、`\uXXXX`（含 Unicode 代理对）
- 解析失败时提供包含**行号/列号**的错误信息
- 支持将解析结果序列化回 JSON 字符串（紧凑格式 / 缩进格式）

### 数据访问 API
| 方法 | 说明 |
|------|------|
| `as_string()` | 取字符串值 |
| `as_int()` | 取整数值 |
| `as_double()` | 取浮点数值 |
| `as_bool()` | 取布尔值 |
| `as_array()` | 取数组 |
| `as_object()` | 取对象 |
| `operator[](size_t)` | 按下标访问数组元素 |
| `operator[](string)` | 按键名访问对象成员 |
| `is_null()` / `is_bool()` 等 | 类型判断 |
| `dump(int indent)` | 序列化，indent 为缩进空格数（-1 为紧凑格式）|

---

## 项目结构

```
woc-JsonParser/ 
├── README.md
├── CMakeLists.txt
├── .gitignore
├── include/
│   ├── json.h        # Json 类定义
│   └── parser.h      # Scanner / Parser / Token 定义
├── src/
│   ├── json.cpp      # Json 类实现（访问、序列化）
│   ├── parser.cpp    # 词法分析 + 递归下降解析
│   └── main.cpp      # 使用示例
└── test/
    └── text.cpp      # 测试用例（40 个）
```

---

## 依赖

- C++17 或更高标准
- CMake 3.10+
- 无第三方库依赖，仅使用 STL

---

##  开发环境与构建

本项目使用 **CMake** 作为构建系统，开发与调试均在 **CLion** 环境下完成。

* **C++ 标准**：C++17 (深度使用了 `std::variant`, `std::optional` 等特性以保证类型安全)
* **编译器推荐**：GCC 8+ / Clang 7+ / MSVC 19.14+

### 使用 CMake（推荐）

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### 运行主程序（使用示例）

```bash
./JsonParserTry
```

### 运行测试

```bash
./JsonTest
```


## 使用示例

```cpp
Json json = Json::parse(R"({
    "name": "张三",
    "age": 20,
    "scores": [95, 87, 92],
    "address": {
        "city": "北京",
        "street": "中关村大街"
    }
})");

std::string name = json["name"].as_string();       // "张三"
int age          = json["age"].as_int();            // 20
double score     = json["scores"][0].as_double();   // 95.0
std::string city = json["address"]["city"].as_string(); // "北京"

// 序列化（4 空格缩进）
std::string output = json.dump(4);
```

### 错误处理

```cpp
try {
    Json::parse("{bad json}");
} catch (const std::exception& e) {
    // 输出：Line 1, Col 2: Unexpected character
    std::cout << e.what() << std::endl;
}
```

---

## 设计说明

- **词法分析（Scanner）**：逐字符扫描，将输入切分为 Token，每个 Token 记录行号和列号
- **语法分析（Parser）**：递归下降解析，按照 JSON 语法规则递归构建 Json 对象
- **数据存储**：使用 `std::variant` 存储六种类型，避免手动内存管理
- **性能优化**：Token 的 `value` 字段使用 `std::string_view` 直接指向原始字符串，减少拷贝

---
## 声明

这个项目的完成使用了 Gemini , DeepSeek , Claude 这些 AI 的网页版 
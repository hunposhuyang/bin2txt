#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <iomanip> // For std::hex and std::setw

struct stringTable {
    uint32_t offset; // 字符串的偏移量
    std::string string; // 字符串内容
};

class Bin {
public:
    Bin(std::string filename);
    ~Bin();

    void initData();
    void crateOffsetFile();

private:
    uint32_t Size; // 文件大小
    uint8_t* Data; // 文件数据
    std::string filename; // 文件名
    std::vector<stringTable> stringTables; // 存储字符串表的动态数组
    std::string output = "";
    bool isValidString(const std::string& str);
};

Bin::Bin(std::string filename) {
    this->filename = filename;
    initData();
}

Bin::~Bin() {
    delete[] Data; // 释放内存
}

void Bin::initData() {
    std::ifstream fs;
    uintmax_t fileSize = std::filesystem::file_size(filename);

    if (fileSize > static_cast<uintmax_t>(std::numeric_limits<int>::max())) {
        throw std::overflow_error("File size exceeds maximum int value");
    }

    int size = static_cast<int>(fileSize);

    fs.open(filename, std::ios::binary);
    if (!fs.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    uint8_t* data = new uint8_t[size];
    fs.read(reinterpret_cast<char*>(data), size);
    this->Data = data;
    this->Size = size;
    fs.close();
}

// 辅助函数：验证字符串是否为有效UTF-8编码
bool isValidUTF8(const std::string& str) {
    const unsigned char* data = (const unsigned char*)str.data();
    size_t len = str.size();
    size_t i = 0;
    while (i < len) {
        unsigned char c = data[i];
        if (c < 0x80) {
            i++;
        } else if ((c & 0xE0) == 0xC0) { // 2字节
            if (i + 1 >= len || (data[i+1] & 0xC0) != 0x80) return false;
            i += 2;
        } else if ((c & 0xF0) == 0xE0) { // 3字节
            if (i + 2 >= len || 
                (data[i+1] & 0xC0) != 0x80 || 
                (data[i+2] & 0xC0) != 0x80) return false;
            i += 3;
        } else if ((c & 0xF8) == 0xF0) { // 4字节
            if (i + 3 >= len || 
                (data[i+1] & 0xC0) != 0x80 || 
                (data[i+2] & 0xC0) != 0x80 || 
                (data[i+3] & 0xC0) != 0x80) return false;
            i += 4;
        } else {
            return false; // 非法起始字节
        }
    }
    return true;
}

bool Bin::isValidString(const std::string& str) {
    if (str.empty()) return false;

    // 首先验证整个字符串是有效的UTF-8
    if (!isValidUTF8(str)) return false;

    for (size_t i = 0; i < str.size();) {
        unsigned char c = str[i];
        if (c < 0x80) { // ASCII字符
            // 排除控制字符（0x00-0x1F）和DEL（0x7F）
            if (c < 0x20 || c == 0x7F) return false;
            i++;
        } else { // 非ASCII字符（UTF-8多字节）
            // 已通过isValidUTF8验证，直接跳过该字符
            int len = 0;
            if ((c & 0xE0) == 0xC0) len = 2;
            else if ((c & 0xF0) == 0xE0) len = 3;
            else if ((c & 0xF8) == 0xF0) len = 4;
            else return false; // 不应触发，因已验证UTF-8有效性
            i += len;
        }
    }
    return true;
}

void Bin::crateOffsetFile() {
    size_t startOffset = 0; // 当前字符串的起始偏移量

    for (size_t i = 0; i < Size; ++i) {
        if (Data[i] == 0x00) { // 遇到 0x00
            // 提取从 startOffset 到 i 的子数组作为字符串
            std::string str(reinterpret_cast<const char*>(&Data[startOffset]), i - startOffset);

			if (isValidString(str)) {
                // 将字符串及其偏移量存储到 stringTables
                stringTables.push_back({ static_cast<uint32_t>(startOffset), str });
			}


            // 更新起始偏移量
            startOffset = i + 1;
        }
    }

    // 处理末尾没有 0x00 的情况
    if (startOffset < Size) {
        std::string str(reinterpret_cast<const char*>(&Data[startOffset]), Size - startOffset);
        stringTables.push_back({ static_cast<uint32_t>(startOffset), str });
    }

    // 创建一个文件并将字符串表写入文件,编码为utf-8
    output = filename + ".txt";
    std::ofstream offsetFile(output);
    if (!offsetFile.is_open()) {
        std::cerr << "Failed to create offset file" << std::endl;
        return;
    }
    // 写入 UTF-8 BOM (EF BB BF)
    unsigned char bom[] = { 0xEF, 0xBB, 0xBF };
    offsetFile.write(reinterpret_cast<char*>(bom), sizeof(bom));

    // 设置输出格式为十六进制，前面补0，8位宽度
    offsetFile << std::hex << std::setfill('0');

    for (const auto& table : stringTables) {
        offsetFile << "0x" << std::setw(8) << table.offset << "\t" << table.string << std::endl;
    }

    offsetFile.close();
    std::cout << output << " created successfully." << std::endl;
}
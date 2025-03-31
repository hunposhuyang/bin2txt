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

bool Bin::isValidString(const std::string& str) {
    if (str.empty()) {
        return false;
    }

    int printable_count = 0;
    bool has_utf8 = false;
    size_t i = 0;

    while (i < str.size()) {
        unsigned char c = str[i];

        // 处理UTF-8多字节字符
        if (c >= 0x80) {  // UTF-8字符开始
            int follow_bytes = 0;
            if ((c & 0xE0) == 0xC0) follow_bytes = 1;      // 2字节字符
            else if ((c & 0xF0) == 0xE0) follow_bytes = 2; // 3字节字符
            else if ((c & 0xF8) == 0xF0) follow_bytes = 3; // 4字节字符

            // 验证后续字节是否符合格式(10xxxxxx)
            bool valid = true;
            for (int j = 0; j < follow_bytes; ++j) {
                if (++i >= str.size() || (str[i] & 0xC0) != 0x80) {
                    valid = false;
                    break;
                }
            }

            if (valid) {
                printable_count += follow_bytes + 1;  // 整个UTF-8字符计为可打印
                has_utf8 = true;
            }
        }
        // ASCII可打印字符（排除控制字符）
        else if (c >= 0x20 && c != 0x7F) {  // 0x20-0x7E为可打印ASCII
            printable_count++;
        }
        i++;
    }

    // 调整判定条件：包含UTF-8字符时降低比例要求
    const int threshold = has_utf8 ? 50 : 70;
    return (printable_count * 100 / str.size() >= threshold);
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
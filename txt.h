#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

struct MemoryOperation {
    uintptr_t address;
    std::string content;
};

class TXT
{
public:
    std::string filename = "";
    std::string output = "";
    uintptr_t base_offset = NULL;

    TXT(const char* strfilepath, const char* baseOffset, const char* output);
    ~TXT();
    void writeTxt2Bin();
    std::vector<MemoryOperation> process_file(std::string& filename, uintptr_t base_address);


};

TXT::TXT(const char* strfilepath, const char* output, const char* baseOffset)
{
    this->filename = strfilepath;
    this->output = output;
	this->base_offset = std::stoull(baseOffset, nullptr, 0);
}

TXT::~TXT()
{
}

void TXT::writeTxt2Bin()
{
	std::vector<MemoryOperation> operations = process_file(filename, base_offset);
	// 打开输出文件，使用二进制模式和追加模式
	std::ofstream outfile(output, std::ios::binary | std::ios::in | std::ios::out);
	if (!outfile.is_open()) {
		// 如果文件不存在，则创建文件
		outfile.open(output, std::ios::binary | std::ios::out);
		if (!outfile.is_open()) {
			throw std::runtime_error("Failed to open output file");
		}
	}
	// 将每个操作的内容写入到指定的偏移位置
	for (const auto& op : operations) {
		uintptr_t relative_address = op.address;
        outfile.seekp(static_cast<std::streamoff>(relative_address));
        outfile.write(op.content.c_str(), static_cast<std::streamsize>(op.content.size()));
	}
	outfile.close();
}




std::vector<MemoryOperation> TXT::process_file(std::string& file_name, uintptr_t base_address) {
    std::ifstream file(file_name);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file");
    }

    std::vector<MemoryOperation> operations;
    std::string line;

    while (std::getline(file, line)) {
        const size_t tab_pos = line.find('\t');
        if (tab_pos == std::string::npos) continue;

        const std::string offset_str = line.substr(0, tab_pos);
        std::string content = line.substr(tab_pos + 1);

        try {
            uintptr_t offset = std::stoull(offset_str, nullptr, 0);
            operations.push_back({
                base_address + offset,
                content + '\0'  // 添加字符串结束符
                });
        }
        catch (const std::invalid_argument&) {
            continue;
        }
        catch (const std::out_of_range&) {
            continue;
        }
    }

    file.close();
    return operations;
}

#include <iostream>
#include <fstream>
#include "txt.h"
#include "bin.h"


int main(int argc, const char* argv[]) {

	if (std::string(argv[1]) == "-c" && argc > 3) 
	{
		if (argc == 4)
		{
			TXT txt = TXT(argv[2], argv[3], 0);
			txt.writeTxt2Bin();
		}
		else if (argc == 5)
		{
			TXT txt = TXT(argv[2], argv[3], argv[4]);
			txt.writeTxt2Bin();
		}
		
		
	}
	else if (argc == 3 && std::string(argv[1]) == "-e")
	{
		Bin bin = Bin(argv[2]);
		bin.crateOffsetFile();
	}
	else
	{
		std::cerr << "Usage: \n" << "txt2bin: " << argv[0] << " -c <string_file_path> <bin_file_path> <base_offset（default=0）>" << std::endl
								 << "bin2txt: " << argv[0] << " -e <bin_file_path>" << std::endl;
	}


	return 0;
}


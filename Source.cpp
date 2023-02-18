#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <stdint.h>
#include <windows.h>
#include <vector>
#include <map>
#include <algorithm>
#include <bitset>
#include <cmath>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <math.h>

using namespace std;

int write_bchain(bool& func_flag, unsigned char byte, FILE* w_file, std::vector<unsigned char>& byte_chain) {

	if (byte_chain.size() > 128) {

		switch (func_flag)
		{
			unsigned char flag_byte;
		case 0:
			flag_byte = 127;
			fputc(flag_byte, w_file);
			for (int i = 0; i < 128; i++) {
				fputc(byte_chain[0], w_file);
				byte_chain.erase(byte_chain.begin());
			}
			write_bchain(func_flag, byte, w_file, byte_chain);
			break;

		case 1:
			flag_byte = 255;
			fputc(flag_byte, w_file);
			fputc(byte_chain.back(), w_file);
			for (int i = 0; i < 129; i++) {
				byte_chain.erase(byte_chain.begin());
			}
			write_bchain(func_flag, byte, w_file, byte_chain);
			break;
		}
	}
	else {
		switch (func_flag)
		{
			unsigned char flag_byte;

		case 0:
			flag_byte = byte_chain.size() - 1;
			fputc(flag_byte, w_file);
			for (auto iter = byte_chain.begin(); iter != byte_chain.end(); ++iter) {
				fputc(*iter, w_file);
			}
			byte_chain.clear();
			byte_chain.push_back(byte);
			byte_chain.push_back(byte);
			break;
		case 1:
			flag_byte = byte_chain.size() - 2;
			flag_byte |= (1 << 7);
			fputc(flag_byte, w_file);
			fputc(byte_chain.back(), w_file);
			byte_chain.clear();
			byte_chain.push_back(byte);
			break;
		}
		func_flag = !func_flag;
	}

	return 0;
};

void zip(char file_name[40]) {
	FILE* file = fopen(file_name, "rb");
	FILE* archive = fopen("archive.imi", "wb");

	uint8_t signature[6] = { 0x69, 0x69, 0x6D, 0x61, 0x69, 0x61 };

	//write signature to archive
	for (int i = 0; i < 6; i++) {
		fputc(signature[i], archive);
	}

	unsigned char byte;

	std::vector<unsigned char> byte_chain;

	bool func_flag = 0;

	while (!feof(file)) {

		byte = fgetc(file);

		if (!feof(file)) {

			switch (func_flag) {

			case 0:
				if (byte_chain.size() == 0 || byte != byte_chain.back()) {
					byte_chain.push_back(byte);
				}
				else if (byte_chain.size() > 1) {
					byte_chain.pop_back();
					write_bchain(func_flag, byte, archive, byte_chain);
				}
				else {
					byte_chain.push_back(byte);
					func_flag = !func_flag;
				}
				break;
			case 1:
				if (byte == byte_chain.back()) {
					byte_chain.push_back(byte);
				}
				else {
					write_bchain(func_flag, byte, archive, byte_chain);
				}
				break;
			}
		}
	}

	if (!byte_chain.empty()) {
		write_bchain(func_flag, 0, archive, byte_chain);
	}
	fclose(file);
	fclose(archive);
};

void unzip() {
	FILE* archive = fopen("archive.imi", "rb");
	FILE* file = fopen("unzip", "wb");

	//signature
	uint8_t signature[6] = { 0x69, 0x69, 0x6D, 0x61, 0x69, 0x61 };

	//check archive signature
	for (int i = 0; i < 6; i++) {
		uint8_t current;
		fread(&current, 1, 1, archive);

		if (current != signature[i]) {
			std::cout << "error" << std::endl;
			return;
		}
	}

	unsigned char byte;
	int chain_size;

	while (!feof(archive)) {

		byte = fgetc(archive);

		if (!feof(archive)) {
			if ((int)byte <= 127) {
				chain_size = (int)byte + 1;
				for (int i = 0; i < chain_size; ++i) {
					byte = fgetc(archive);
					//std::cout << (int)byte << " ";
					fputc(byte, file);
				}
			}
			else {
				byte &= ~(1 << 7);
				chain_size = (int)byte + 2;
				byte = fgetc(archive);
				for (int i = 0; i < chain_size; ++i) {
					fputc(byte, file);
					//std::cout << (int)byte << " ";
				}
			}
		}
	}
	fclose(file);
	fclose(archive);
};

size_t byte_of_file(char file_name[40]) {
	std::ifstream file(file_name); 
	size_t s = 0; 
	if (!(file.is_open())) {
		std::cout << "File not found" << std::endl;
	}
	while (!file.eof()) { 
		file.get();       
		s++;             
	}

	file.close(); 

	s--; 

	return s; 
}

void smart_archive(char file_name[40]) {
	zip(file_name);
	size_t file_s = byte_of_file(file_name);
	char archive_name[40] = "archive.imi";
	size_t archive_s = byte_of_file(archive_name);

	if (archive_s > file_s) {
		FILE* file = fopen(file_name, "rb");
		FILE* archive = fopen("archive.imi", "wb");
		while (!feof(file)) {
			unsigned char byte = fgetc(file);
			if (!feof(file)) {
				fputc(byte, archive);
			}
		}
	}
	else {
		unzip();
	}
}

int main() {

	std::cout << "Input filename: ";
	char file_name[40];
	std::cin >> file_name;

	smart_archive(file_name);

	return 0;
}

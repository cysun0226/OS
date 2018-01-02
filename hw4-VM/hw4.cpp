// Student ID : 0416045
// Name       : cysun
// Date       : 2018.01.02

#include <iostream>
#include <fstream>
using namespace std;

#define TLB_SIZE 16
#define PAGE_TABLE_SIZE 256
#define PAGE_SIZE 256
#define FRAME_SIZE 256
#define FRAME_NUM 256
#define PHYSICAL_MEMORY 65536

int main(int argc, char* argv[])
{
	fstream backing_file, addr_file;
	backing_file.open(argv[1], ios::in);
	addr_file.open(argv[2], ios::in);

	unsigned short log_addr;
	for (size_t i = 0; i < 10; i++) {
		addr_file >> log_addr;
		cout << log_addr << endl;
	}
}

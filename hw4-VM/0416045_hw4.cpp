// Student ID : 0416045
// Name       : cysun
// Date       : 2018.01.02

#include <iostream>
#include <cstdio>
#include <fstream>
#include <bitset>
#include <vector>
#include <map>
#include <algorithm>
#include <cstdint>

using namespace std;

#define TLB_SIZE 16
#define PAGE_TABLE_SIZE 256
#define PAGE_SIZE 256
#define FRAME_SIZE 256
#define FRAME_NUM 256
#define PHYSICAL_MEMORY 65536

typedef unsigned char Page[PAGE_SIZE];
// typedef unsigned char Frame[FRAME_SIZE];

std::vector<char*> physical_memory;

map<int, int> TLB; // page number, frame number
map<int, int>::iterator TLBiter;
std::vector<int> LRU;
std::vector<int>::iterator LRUiter;

int page_table[PAGE_TABLE_SIZE];

void page_table_init(int* ptab)
{
	for (size_t i = 0; i < PAGE_TABLE_SIZE; i++) {
		page_table[i] = -1;
	}
}

int main(int argc, char* argv[])
{
	fstream backing_file, addr_file;
	backing_file.open(argv[1], ios::in);
	addr_file.open(argv[2], ios::in);

	int TLB_hit = 0, page_flt = 0;
	page_table_init(page_table);

	int addr_num;
	addr_file >> addr_num;
	unsigned short log_addr;
	for (size_t i = 0; i < addr_num; i++)
	{
		unsigned char page_number, offset;
		addr_file >> log_addr;
		offset = log_addr & 0b11111111;
		page_number = log_addr >> 8;

		// update LRU
		LRUiter = find(LRU.begin(), LRU.end(), page_number);
		if (LRUiter != LRU.end()) { // exist in LRU, pop the original one and push again
			LRU.erase(LRUiter);
			LRU.push_back(page_number);
		}
  	else
    	LRU.push_back(page_number);

		int phy_addr = 0;

		/* check TLB */
		TLBiter = TLB.find(page_number);
		// TLB hit
		if(TLBiter != TLB.end()) {
			// cout << "TLB hit, frame number = " << bitset<8>(TLBiter->second) << endl;
			phy_addr = ((unsigned char)TLBiter->second << 8) + offset;
			cout << int(phy_addr) << ' ' << int(physical_memory[TLBiter->second][offset]) << endl;
			TLB_hit++;
		}
		// TLB miss
		else {
			if (page_table[page_number] != -1) {
				// cout << "find in page table, frame number = " << page_table[page_number] << endl;
				phy_addr = ((unsigned char)page_table[page_number] << 8) + offset;
				cout << int(phy_addr) << ' ' << int(physical_memory[page_table[page_number]][offset]) << endl;
			}
			// page fault
			else {
				// cout << "page fault!" << endl;
				page_flt++;
				// Update TLB and page table
				// move data from backing store to physical memory
				char* frame;
				frame = new char[FRAME_SIZE];
				FILE *bs;
				bs = fopen(argv[1], "rb");
				fseek(bs, 0, SEEK_SET); // front
				fseek(bs, page_number*PAGE_SIZE, SEEK_CUR);
				fread(frame, sizeof(unsigned char), FRAME_SIZE, bs);

				// *f = 0;
				physical_memory.push_back(frame);
				int frame_number = physical_memory.size()-1;
				page_table[page_number] = frame_number;
				if(TLB.size() < TLB_SIZE)
					TLB[page_number] = frame_number;
				else {
					// remove the bottom of LRU
					TLB.erase(LRU[0]);
					TLB[page_number] = frame_number;
					LRU.erase(LRU.begin());
				}

				phy_addr = ((unsigned char)frame_number << 8) + offset;
				cout << int(phy_addr) << ' ' << int(frame[offset]) << endl;
			}
		}
		// cout << "addr = " << log_addr << endl;
		// cout << bitset<8>(page_number) << ' ' << bitset<8>(offset) << endl << endl;
	}

	cout << "TLB hits: " << TLB_hit << endl;
	cout << "Page faults: " << page_flt << endl;

	return 0;
}

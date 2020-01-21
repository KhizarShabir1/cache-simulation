#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <string.h>
#include <iomanip>
#include <fstream>
#include <math.h>
#include <list>
#include <algorithm>
#include "funcs.h"

using namespace std;

//variables
int hit = 0, miss = 0;
int write_back = 0;
int write_through = 0;
int write_allocate = 0;
int write_no_allocate = 0;
int Data_READ=0;
int Inst_READ=0;
int Total_ACCESS=0;
int Data_Miss=0;
int Inst_Hit=0;
int Data_Write=0;
int DataWR_Hits=0;
int DataWR_Miss=0;
int Data_Hit=0;
int Inst_Miss=0;

class Block
{
public:
	int valid_bit;
	int dirty_bit;
	int tag;
	unsigned int *data;
	int size_word;

	//functions
	Block()
	{
		valid_bit = 0;
		dirty_bit = 0;
		tag = 0;
		data = NULL;
	}
	void block_initialize(int block_size_word)
	{
		size_word = block_size_word;
		data = new unsigned int [block_size_word];
	}
};

class Set
{
public:
	int blocksPerSet;
	Block *cache_row;
	list<int> lru;
	
	Set()
	{
		cache_row = NULL;
	}
	void set_declaration(int size, int block_size_word)
	{
		blocksPerSet = size;
		cache_row = new  Block [size];
		for (int i=0;i<size;++i)
		{
			cache_row[i].block_initialize(block_size_word);			
		}
	}
	~Set()
	{
		delete [] cache_row;
	}	
};

class Cache
{
public:
	int capacity;
	int block_size;		//in bytes
	int associativity;
	int total_sets;
	Set *setArray;
	
	Cache()
	{
		capacity = 0;
		block_size = 0;
		associativity = 0;
		total_sets = 0;
		setArray = NULL;
	}
	void initialize(int capacity , int block_Size , int associativity)
	{
		this->capacity = capacity;
		this->block_size = block_Size;
		this->associativity = associativity;
		total_sets = (capacity/block_Size)/associativity;
		setArray = new Set[total_sets];
		for (int i =0;i<total_sets;++i)
		{
			setArray[i].set_declaration(associativity, block_Size/4);
		}
	}
	void calculate_tibw(int decimal_address, int &tag, int &index, int &wordOffset)
	{
		int temp = decimal_address;
		/*byteOffset = temp % 4;*/
		temp = temp / 4;
		wordOffset = temp % (block_size/4);
		temp = temp / (block_size/4);
		index = temp % total_sets;
		temp = temp / total_sets;
		tag = temp;
	}
	unsigned int calculate_address(const int &tag, const int &index)
	{
		return ((tag * total_sets) + index) * (block_size/4) * 4;
	}
	int dataLoad(int decimal_address)		//return 1 for hit, 0 for miss
	{
		int tag =0, index = 0, wordOffset = 0 /*, byteOffset = 0*/;
		calculate_tibw(decimal_address, tag, index, wordOffset);
		bool exist = false;

		for(int i = 0;i<setArray[index].blocksPerSet ;++i)
		{
			if (setArray[index].cache_row[i].tag == tag && setArray[index].cache_row[i].valid_bit == 1)
			{
				list<int> &tempLru = setArray[index].lru; 

				list<int>::iterator itr = find(tempLru.begin(),tempLru.end(),i); 
				if ( itr != tempLru.end())
				{
					tempLru.erase(itr);
				}
				tempLru.push_back(i);
				exist = true;
				break;
			}
		}
		if(exist == true)
		{
			++hit;
			return 1;
		}
		else
		{
			++miss;
			return 0;
		}
	}
	unsigned int *dataStoreMiss(int decimal_address, int* data)
	{
		int tag =0, index = 0, wordOffset = 0 /*, byteOffset = 0*/;
		calculate_tibw(decimal_address, tag, index, wordOffset);
		int dirty = 0;
		unsigned int *dataWordArray = NULL; 

		int dataIndex = -1;
		for(int i = 0; i<setArray[index].blocksPerSet; ++i)
		{
			if (setArray[index].cache_row[i].valid_bit == 0)
			{
				dataIndex = i;
				break;
			}
		}
		
		if(dataIndex == -1)
		{
			dataIndex = setArray[index].lru.front();
			if (setArray[index].cache_row[dataIndex].dirty_bit == 1)
			{
				dirty = 1;
				dataWordArray = setArray[index].cache_row[dataIndex].data;
			}
		}

		setArray[index].cache_row[dataIndex].valid_bit = 1;
		setArray[index].cache_row[dataIndex].tag = tag;
		for (int j =0; j<(block_size/4); ++j)
		{
			setArray[index].cache_row[dataIndex].data[j] = data[j];
		}

		list<int> &tempLru = setArray[index].lru; 
		list<int>::iterator itr = find(tempLru.begin(),tempLru.end(),dataIndex); 
		if ( itr != tempLru.end())
		{
			tempLru.erase(itr);
		}
		tempLru.push_back(dataIndex);

		if (dirty == 1)
		{
			return dataWordArray;
		}
		return NULL;
	}
	int dataStore(int decimal_address, unsigned int dataWord)
	{
		int tag =0, index = 0, wordOffset = 0 /*, byteOffset = 0*/;
		calculate_tibw(decimal_address, tag, index, wordOffset);
		bool exist = false;

		for(int i = 0; i<setArray[index].blocksPerSet; ++i)
		{
			if (setArray[index].cache_row[i].tag == tag && setArray[index].cache_row[i].valid_bit == 1)
			{
				setArray[index].cache_row[i].data[wordOffset] = dataWord;
				if (write_back)
				{
					setArray[index].cache_row[i].dirty_bit = 1;
				}

				list<int> &tempLru = setArray[index].lru; 

				list<int>::iterator itr = find(tempLru.begin(),tempLru.end(),i); 
				if ( itr != tempLru.end())
				{
					tempLru.erase(itr);
				}
				tempLru.push_back(i);
				exist = true;
				break;
			}
		}
		if(exist == true)
		{
			++hit;
			return 1;
		}
		else
		{
			++miss;
			return 0;
		}
	}
	~Cache()
	{
		delete [] setArray;
	}
};

class MainMemory
{
public:
	int sizeWord;
	int *array;

	MainMemory(int sizeBytes)
	{
		sizeWord = sizeBytes/4;
		array = new int [sizeWord];	//word
		for(int i =0;i<(sizeWord);++i)
		{
			array[i] = i*4;
		}
	}
};

//functions
void dataMemory(MainMemory &mainMemory, Cache &cache, int decimal_address, int instruction, unsigned int data = 0);
void printCache(Cache &l1);
void printMemory(MainMemory &ram);
void cacheDataToMem(MainMemory &ram, Cache &l1);


int main (int argc, char *argv[ ])
{
	//the values that we get from the 
	//command line will be put into these
	//variables. we can use them later in the program
	//like for checking if they have valid values
	int cache_capacity = 0;
	int cache_blocksize = 0;
	int cache_associativity = 0;
	int cache_split = 0;
	if(!parseParams(argc, argv, cache_capacity, cache_blocksize, cache_associativity, cache_split, write_back, write_through, write_allocate, write_no_allocate))
	{
		exit(2);
	}

	if(write_through == 0)
	{
		write_back = 1;
	}
	if(write_no_allocate == 0)
	{
		write_allocate = 1;
	}

	cout << "Cache Capacity: " << cache_capacity << endl;
	cout << "Cache BlockSize: " << cache_blocksize << endl;
	cout << "Cache Associativity: " << cache_associativity << endl;
	cout << "Cache Split: " << cache_split<<endl;
	cout << "Write Back: " << write_back<<endl;
	cout << "Write Through: " << write_through<<endl;
	cout << "Write Allocate: " << write_allocate<<endl;
	cout << "Write no-allocate: " << write_no_allocate<<endl;

	Cache *l1;
	if(cache_split == 0)		//unified
	{
		cache_capacity = cache_capacity * 1024;		//in KB's
		l1 = new Cache;
		l1[0].initialize(cache_capacity, cache_blocksize, cache_associativity);
	}
	else						//split
	{
		cache_capacity = (cache_capacity * 1024) / 2;
		l1 = new Cache [2];
		l1[0].initialize(cache_capacity, cache_blocksize, cache_associativity);			//l1D
		l1[1].initialize(cache_capacity, cache_blocksize, cache_associativity);			//l1I
	}

	int memory_size = 16*1024*1024;		//16MB
	MainMemory ram(memory_size);

	//file reading
	char filename[50];
	ifstream file;
	cout<<"Enter filename : ";
	cin>>filename;

	while(1)
	{		
		file.open(filename,ios::in);
		if (file.fail())
		{
			cout<<"File not found.";
			cout<<"Enter again : ";
			cin>>filename;
			continue;
		}
		break;
	}

	string str;	
	int decimal_address;
	unsigned int dataWord;	
	while(file >> str)
	{
		if(str == "0")		//data load
		{
			Total_ACCESS++;
			Data_READ++;
			file >> hex >> decimal_address;
			dataMemory(ram, l1[0], decimal_address, 0);
		}
		else if (str == "1")
		{
			Total_ACCESS++;
			Data_Write++;
			file >> hex >> decimal_address;
			file >> hex >> dataWord;
			dataMemory(ram, l1[0], decimal_address, 1, dataWord);
		}
		else if (str == "2")
		{
			Total_ACCESS++;
			Inst_READ++;
			file >> hex >> decimal_address;
			if(cache_split == 0)		//unified
			{
				dataMemory(ram, l1[0], decimal_address, 2);
			}
			else						//split
			{
				dataMemory(ram, l1[1], decimal_address, 2);
			}
		}
	}
	file.close();
	if(cache_split == 0)
	{
		cacheDataToMem(ram,l1[0]);
	}
	else
	{
		cacheDataToMem(ram,l1[0]);
		cacheDataToMem(ram,l1[1]);
	}


	if(cache_split == 0)
	{
	cout<<endl<<"< STATISTICS >"<<endl;
	cout<<"Total Memory Access"<<miss<<endl;
	cout<<endl<<"L1 CACHE"<<endl;	
	cout<<"Hit : "<< dec <<hit<<endl;
	cout<<"Miss : "<< dec <<miss<<endl;
	cout<<"Total Accesses : "<<Total_ACCESS<<endl;
	cout<<"Data Read : "<<Data_READ<<endl;
	cout<<"Data Writes : "<<Data_Write<<endl;
	cout<<"Instrution Access : "<<Inst_READ<<endl;
	cout<<"Hit Rate : "<<double(hit)/double(Total_ACCESS)<<endl;
	cout<<"Miss Rate : "<<(double(miss)/double(Total_ACCESS))<<endl;
	printCache(l1[0]);
	}

	else
	{
		
		cout<<endl<<"< STATISTICS >"<<endl;
		
		cout<<endl;
		
		cout<<"Memory Accesses : "<<DataWR_Miss+Data_Miss+Inst_Miss<<endl;
		cout<<"L1 DATA CACHE : "<<endl;
		cout<<"Total Data Access : "<<Data_READ<<endl;
		cout<<"Total Data Writes : "<<Data_Write<<endl;

		cout<<"Data Read Hits : "<<Data_Hit<<endl;
		cout<<"Data Read Miss : "<<Data_Miss<<endl;
		cout<<"Data Write Hits : "<<DataWR_Hits<<endl;
		cout<<"Data Write Miss : "<<DataWR_Miss<<endl;
		cout<<"Data read hitrate : "<<(double)Data_Hit/(double)Data_READ<<endl;
		cout<<"Data write hitrate : "<<(double)DataWR_Hits/(double)Data_Write<<endl;
		
	cout<<"L1 INSTRUCTION CACHE : "<<endl;
	cout<<"Instruction  Access : "<<dec<<Inst_READ<<endl;
	cout<<"Instruction Hits : "<<dec<<Inst_Hit<<endl;
	cout<<"Instruction Miss : "<<dec<<Inst_Miss<<endl; 
	cout<<"Instruction hitrate is : "<<(double)(Inst_Hit)/(double)(Inst_READ)<<endl;
		printCache(l1[0]);
		cout<<endl;

	cout<<"L1 INSTRUCTION CACHE : "<<endl;
	cout<<"Instruction  Access : "<<dec<<Inst_READ<<endl;
	cout<<"Instruction Hits : "<<dec<<Inst_Hit<<endl;
	cout<<"Instruction Miss : "<<dec<<Inst_Miss<<endl; 
	
		printCache(l1[1]);
	}
	
	printMemory(ram);

	return 0;
}

void dataMemory(MainMemory &ram, Cache &l1, int decimal_address, int instruction, unsigned int dataWord)
{
	if((decimal_address/4) > ram.sizeWord)
	{
		cout<<"Segmentation fault"<<endl;
		return;
	}

	unsigned int *data;
	if(instruction == 0)
	{
		if(!l1.dataLoad(decimal_address))		//if miss
		{
			Data_Miss++;
			if(decimal_address/4 > (ram.sizeWord-(l1.block_size/4)))
			{
				data = l1.dataStoreMiss(decimal_address, ram.array+(ram.sizeWord-(l1.block_size/4)));
			}
			else
			{
				data = l1.dataStoreMiss(decimal_address, ram.array+(decimal_address/4));
			}
		}
		else
		{
			Data_Hit++;
		}
	}
	else if (instruction == 1)
	{
		//if hit it will automatically modify cache
		if(!l1.dataStore(decimal_address, dataWord))		//if miss 
		{
			DataWR_Miss++;
			if(write_allocate)
			{
				if(decimal_address/4 > (ram.sizeWord-(l1.block_size/4)))
				{
					data = l1.dataStoreMiss(decimal_address, ram.array+(ram.sizeWord-(l1.block_size/4)));
				}
				else
				{
					data = l1.dataStoreMiss(decimal_address, ram.array+(decimal_address/4));
				}
				l1.dataStore(decimal_address, dataWord);
				--hit;		//because this time data is in cache
			
			}
			else if(write_no_allocate)
			{
				ram.array[decimal_address/4] = dataWord;
			}
		}
		else		//hit
		{
				DataWR_Hits++;
			if(write_through)
			{
			
				ram.array[decimal_address/4] = dataWord;
			}
		}
	}
	else if (instruction == 2)
	{
		if(!l1.dataLoad(decimal_address))		//if miss
		{
			Inst_Miss++;
			if(decimal_address/4 > (ram.sizeWord-(l1.block_size/4)))
			{
				data = l1.dataStoreMiss(decimal_address, ram.array+(ram.sizeWord-(l1.block_size/4)));
			}
			else
			{
				data = l1.dataStoreMiss(decimal_address, ram.array+(decimal_address/4));
			}
		}
		else{
			Inst_Hit++;
		}

	}

	if (data != NULL)
	{
		for (int i = 0; i < (l1.block_size/4);++i)
		{
			ram.array[(decimal_address/4)+i] = data[i];
		}
	}
}

void printCache(Cache &l1)
{
	//cout<<"CACHE CONTENT"<<endl;
	cout<<"Set"<<'\t'<<"V"<<'\t'<<"Tag"<<'\t'<<"        Dirty"<<"   Words"<<endl;

	for (int i =0; i<l1.total_sets; ++i)
	{
		for (int j =0;j<l1.setArray[i].blocksPerSet;++j)
		{
					cout<< dec <<i<<'\t'<<l1.setArray[i].cache_row[j].valid_bit<<'\t' << std::hex<< std::setfill('0') << std::setw(8) 
					<<l1.setArray[i].cache_row[j].tag<<'\t';
					
					cout<<setw(1)<<l1.setArray[i].cache_row[j].dirty_bit<<'\t';
			for (int k =0;k<(l1.block_size)/4;++k)
			{
					std::cout << std::hex << std::setfill('0') << std::setw(8)
		 <<l1.setArray[i].cache_row[j].data[k]<<"\t";
			}
			cout<<endl;
		}
	}
}

void printMemory(MainMemory &ram)
{
	cout<<endl<<"MEMORY CONTENT"<<endl;
	cout<<"Address"<<"\t\t"<<"Words"<<endl;
	int k = 0, l = 0;
	unsigned int start = 4242572;
	for (unsigned int i = (start/4); i<((start/4)+(1024/(8))); ++i)
	{
		cout<< hex <<(start+(l*32))<<"\t";
		++l;
		for(int j =0; j<8; ++j)
		{
			std::cout << std::hex 	<< std::setfill('0') << std::setw(8) << std::hex
					<< ram.array[(start/4)+k] << "\t";
			++k;
		}
		cout<<endl;
	}
}

void cacheDataToMem(MainMemory &ram, Cache &l1)
{	
	for (int i =0; i<l1.total_sets; ++i)
	{
		for (int j =0; j<l1.setArray[i].blocksPerSet; ++j)
		{
			if(l1.setArray[i].cache_row[j].dirty_bit == 1)
			{
				l1.setArray[i].cache_row[j].dirty_bit = 0;
				unsigned int address = l1.calculate_address(l1.setArray[i].cache_row[j].tag, i);
				unsigned int *data = l1.setArray[i].cache_row[j].data;
				for (int k = 0; k<(l1.block_size/4); ++k)
				{
					ram.array[(address/4)+k] = data[k]; 
				}
			}
		}
	}
}

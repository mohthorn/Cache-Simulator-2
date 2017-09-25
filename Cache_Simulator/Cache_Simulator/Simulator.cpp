#include<iostream>
#include<fstream>
#include <cstdlib>
using namespace::std;
#define VALID 1
#define INVALID 0

class Cache_line {
public:
	int tag;
	int count;
	int flag;
	//static int min;
//	static int max;
};

class Output {
public:
	int totalcount=0;
	int readcount=0;
	int writecount=0;
	int totalmiss=0;
	int readmiss=0;
	int writemiss=0;
};

class Cache_Sim{

	int nk;
	int assoc;
	int blocksize;
	char repl;
	int set_num;
	int offset_l;
	int index_l;
	Cache_line *caches;

public:
	Output output;

public: 
	int checkHit(long long addr ,int base)
	{
		for (int i = 0; i < assoc; i++)
		{
			if (caches[base+i].flag==VALID&&caches[base+i].tag == ((addr >> (offset_l + index_l))))
				return base+i;
		}
		return -1;
	}

public:
	void cacheOp(long long addr,char op)
	{
		int set= (addr >> offset_l) % set_num; //there is a function to get the set index number
		int base = set*assoc;	//where the target set starts
		int index = checkHit(addr, base);	//the block address
		if (index >= 0)		//hit
		{
			if (op == 'r')
			{
				output.readcount++;
			}
			else
			{
				output.writecount++;
				//write
			}
		}
		else
		{
			if (op == 'r')
			{
				output.readmiss++;
				output.readcount++;
			}
			else
			{
				output.writemiss++;
				output.writecount++;
			}
			index = getFree(base);
			caches[index].flag = VALID;
			caches[index].tag = addr >> (offset_l + index_l);
		}
		caches[index].count = getRecent(base)+1;

	}

public:
	int getFree(int base)
	{
		for (int i = 0; i < assoc; i++)
		{
			if (caches[base + i].flag != VALID)
				return base + i;
		}
		return LRU(base);

	}

public:
	int LRU(int base)
	{
		int min = caches[base].count;
		//int max = caches[base].count;
		int target=base;
		for (int i = 0; i < assoc; i++)
		{
			if (caches[base + i].count < min)
			{
				min = caches[base + i].count;
				target = base + i;
			}
		}
		return target;
	}

public:
	int getRecent(int base)
	{
		int max = caches[base].count;
		for (int i = 0; i < assoc; i++)
		{
			if (caches[base + i].count > max)
			{
				max = caches[base + i].count;
			}

		}
		return max;
	}

public:
	Cache_Sim(int nk, int assoc, int blocksize, char repl)
	{
		this->nk = nk;
		this->assoc = assoc;
		this->blocksize = blocksize;
		this->repl = repl;
		set_num = nk * 1024 / assoc / blocksize;
		offset_l = log(blocksize) / log(2);
		index_l = log(set_num) / log(2);
		caches = new Cache_line[nk * 1024 / blocksize];
	}

};

class Result {
	int total_miss;
	int total_percent;
	int read_miss;
	int read_percent;
	int write_miss;
	int write_percent;
};

int main(int argc, char*argv[])
{
	if (argc == 5)
	{
		ifstream trace;
		trace.open("429.mcf-184B.trace.txt", ios::in);
		char op;
		long long addr;
		Cache_Sim cs(atoi(argv[1]),atoi(argv[2]),atoi(argv[3]),argv[4][0]);
		while (!trace.eof())
		{
			trace >> op;
			trace >> hex >> addr;
			cs.cacheOp(addr, op);
		}
		cs.output.totalcount = cs.output.readcount + cs.output.writecount;
		cs.output.totalmiss = cs.output.readmiss + cs.output.writemiss;
		cout << cs.output.totalmiss << endl;
		cout << cs.output.readmiss << endl;
		cout << cs.output.writemiss << endl;
	}
	

	return 0;
}
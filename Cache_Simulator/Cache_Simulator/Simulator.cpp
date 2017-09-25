#include<iostream>
#include<fstream>
#include <cstdlib>
#include<ctime>
using namespace::std;
#define VALID 1
#define INVALID 0

class Cache_line {		//one block in cache, the program does simulates what is happening inside the block
public:					//thus only a flag signaling if the block is initialized, a tag as in addr tag, and a count
	int tag;			//serving as a time of life count
	int count=0;
	int flag=INVALID;
};

class Output {			//store and print the count and miss numbers
public:
	int totalcount=0;
	int readcount=0;
	int writecount=0;
	int totalmiss=0;
	int readmiss=0;
	int writemiss=0;

};

ostream& operator<<(ostream& os, const Output& out)  //Overload ofstream to make the output nicer
{
	os.precision(6);
	os.setf(ios::fixed);
	os << out.totalmiss << ' ';

	os << 1.0*out.totalmiss / out.totalcount * 100 << "\% ";
	os << out.readmiss << ' ';
	if (out.readcount == 0)
		os << "0.000000% ";
	else
		os << 1.0*out.readmiss / out.readcount * 100 << "\% ";
	os << out.writemiss << ' ';
	if (out.writecount == 0)
		os << "0.000000% ";
	else
		os << 1.0*out.writemiss / out.writecount * 100 << "\%"<<endl;
	return os;
}

class Cache_Sim{			//The class doing most of the functions including initializing the cache
							//checking hit, fetching blocks and else.
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
	int checkHit(long long addr ,int base)		//Check if the tag is in a set, use tag to see if it matches
	{
		for (int i = 0; i < assoc; i++)
		{
			if (caches[base+i].flag==VALID&&caches[base+i].tag == ((addr >> (offset_l + index_l))))
				return base+i;
		}			//The tag should be the highest bits after the bit shift. Offset and Index length was calculated
		return -1;		//earlier
	}

public:
	void cacheOp(long long addr,char op)
	{
		int set= (addr >> offset_l) % set_num; //there is a function to get the set index number
		int base = set*assoc;	//use index number to find where the target set starts
		int index = checkHit(addr, base);	//find the block address
		if (index >= 0)		//hit
		{
			if (op == 'r')			//Read and write instructions.They are treated the same in the simulator
			{
				output.readcount++;
			}
			else
			{
				output.writecount++;
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
			index = getFree(base);				//find a free position in set or switch a block out
			caches[index].flag = VALID;			//now there is a block in the space
			caches[index].tag = addr >> (offset_l + index_l);	//set the tag
		}
		caches[index].count = getRecent(base)+1;	//make the block the newest in timeline

	}

public:
	int getFree(int base)
	{
		for (int i = 0; i < assoc; i++)			//free space first
		{
			if (caches[base + i].flag != VALID)
				return base + i;
		}
		if (repl == 'l')						//if no free space, use LRU or Random
			return LRU(base);
		else
			return base + rand()%assoc;

	}

public:
	int LRU(int base)
	{
		int min = caches[base].count;			//the min in count means the oldest in timeline
		int target=base;
		for (int i = 0; i < assoc; i++)				//traverse and get the oldest block
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
	int getRecent(int base)							//traverse and get the newest count in timeline
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
	Cache_Sim(int nk, int assoc, int blocksize, char repl)	//initialize the cache
	{
		this->nk = nk;
		this->assoc = assoc;
		this->blocksize = blocksize;
		this->repl = repl;
		set_num = nk * 1024 / assoc / blocksize;
		offset_l = log(blocksize) / log(2);					// get the length of offset and index
		index_l = log(set_num) / log(2);
		caches = new Cache_line[nk * 1024 / blocksize];    //shoud be blocks, 
	}														//allocate the block objects needed in memory

};


int main(int argc, char*argv[])
{
	srand((unsigned)time(0));							//use current time as the seed of rand()
	if (argc == 5)											//check if the input is valid
	{
		ifstream trace;
		trace.open("429.mcf-184B.trace.txt", ios::in);
		if (trace.fail())
		{
			cout << "file not found" << endl;
			return 0;
		}
		char op;
		long long addr;							//I think it is only place where long long is needed in the examples
		Cache_Sim cs(atoi(argv[1]),atoi(argv[2]),atoi(argv[3]),argv[4][0]);	//cache initialization
		while (!trace.eof())
		{
			trace >> op;
			trace >> hex >> addr;				//read in a hexadecimal number
			if (trace.fail())					//to prevent the trap of reading the last line twice
				break;
			cs.cacheOp(addr, op);
		}
		cs.output.totalcount = cs.output.readcount + cs.output.writecount;
		cs.output.totalmiss = cs.output.readmiss + cs.output.writemiss;
		cout << cs.output << endl;
	}
	else
	{
		cout << "usage error" << endl;
	}

	return 0;
}
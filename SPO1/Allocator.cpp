#include "StdAfx.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <vector>

using namespace std;

struct tag {
	bool free;
	size_t size;
	size_t prevSize;
};

struct Item {
	void *ptr;
	int checksum;
};

class Allocator
{
public:
	Allocator(size_t mem_size) {
		this->mem_size = mem_size;
		mem_start = malloc(mem_size);
		struct tag* header = (tag*)mem_start;
		header->free = false;
		header->prevSize = 0;
		header->size = 0;
		header = (tag*)((size_t)header + header->size + sizeof(tag));
		header->free = true;
		header->prevSize = 0;
		header->size = mem_size - sizeof(tag) * 3;
		header = (tag*)((size_t)header + header->size + sizeof(tag));
		header->free = false;
		header->prevSize = mem_size - sizeof(tag) * 3;
		header->size = 0;
	}

	~Allocator() {
		delete mem_start;
	}

	void mem_test(int count) {
		//srand((unsigned)time(NULL));
		int size = 1;
		Item *p =(Item*)malloc(sizeof(Item));
		vector<Item> ptrs;
		ptrs.reserve((size_t)mem_size/sizeof(tag));
		void* *vector= new void*[10];
		p->ptr = (void*)((size_t)mem_start + sizeof(tag));
		p->checksum = 0;
		ptrs.push_back(*p);
		for (int i = 0; i < count; i++) {
			cout << i<<endl;
			if (i == 700) {
				cout << i << endl;
				//mem_dump();
			}
			//if (size < (mem_size / sizeof(tag))) {
			int num = 0;
			if (size - 1 != 0) {
				num = rand() % (size - 1);
			}
				void *addr = ptrs[num].ptr;
				if (((tag*)addr)->free) {
					int size_block = rand() % ((tag*)addr)->size+1;
					void *ptr = mem_alloc(size_block);
					mem_dump();
					header_dump(ptrs,size);
				}
				else {
					bool op = (bool)(rand() % 2);
					if (op) {
						int size_block = rand() % ((tag*)addr)->size + 1;
						flag = false;
						size_t preSize = ((tag*)addr)->size;
						void* new_header = mem_realloc(addr, size_block);
						mem_dump();
						header_dump(ptrs, size);
					}
					else {
						mem_free(addr);
						mem_dump();
						header_dump(ptrs, size);
					}
					//header_dump(ptrs,size);
				}
			//}

				//mem_dump();
		}
	}
	

	void mem_dump()
	{
		cout << " #\t| Free\t| Size\t| Pointer\n";
		cout << "**********************\n";
		struct tag* curr_header = (tag*)mem_start;
		make_string(0, curr_header);
		curr_header = (tag*)((size_t)curr_header+sizeof(tag));
		int i = 1;
		while (!(curr_header->free==false&&curr_header->size==0))
		{
			make_string(i, curr_header);
			curr_header = (tag*)((size_t)curr_header + curr_header->size + sizeof(tag));
			i++;
		}
		make_string(i, curr_header);
		cout << "\n";
		//curr_header = (tag*)mem_start;
		//if (flag) {
		//	void *List = malloc(sizeof(void*)*i);
		//	for (int j = 0; j < i; j++) {
		//		List = &curr_header;
		//		curr_header = (tag*)((size_t)curr_header + curr_header->size + sizeof(tag));
		//	}
		//	return List;
		//}
	}

	void *mem_alloc(size_t size) {
		//size_t size_block = size;
		//if (size_block % 4) {
		//	size_block = (size_block / 4 + 1) * 4;
		//}
		//int* ptr = new int;
		//*ptr = NULL;
		struct tag* curr_header = (tag*)((size_t)mem_start + sizeof(tag));
		//struct tag* next_header = (tag*)curr_header+curr_header->size;
		while (!((curr_header->free == false) && (curr_header->size == 0))) {
			if (curr_header->free&&((curr_header->size == size)|| (curr_header->size > (size+sizeof(tag))))) {
				if (curr_header->size == size) {
					curr_header->free = false;
					return curr_header;
				}
				else 
				{
					curr_header->size -= size + sizeof(tag);
					size_t prevSize = curr_header->size;
					curr_header = (tag*)((size_t)curr_header + curr_header->size+ sizeof(tag));
					curr_header->free = false;
					curr_header->prevSize = prevSize;
					curr_header->size = size;
					prevSize = size;
					curr_header = (tag*)((size_t)curr_header + curr_header->size+ sizeof(tag));
					curr_header->prevSize = prevSize;
					curr_header = (tag*)((size_t)curr_header - curr_header->prevSize - sizeof(tag));
					return curr_header;
				}
			}
			curr_header = (tag*)((size_t)curr_header + curr_header->size + sizeof(tag));
		}
		return NULL;
	}

	void *mem_realloc(void *addr, size_t size) {
		size_t prevSize = ((tag*)addr)->size;
		struct tag* curr_header = (tag*)addr;
		if (curr_header->size == 0 || curr_header->size==size) {
			return NULL;
		}
		if (size > prevSize) {
			struct tag* prev_header = (tag*)((size_t)curr_header - curr_header->prevSize - sizeof(tag));
			struct tag* next_header = (tag*)((size_t)curr_header + curr_header->size + sizeof(tag));
			if ((prev_header->free&&next_header->free)&&((prev_header->size+next_header->size)>(size + curr_header->size))) {
				mem_free(prev_header);
				mem_free(next_header);
				size_t free_size = prev_header->size+next_header->size+curr_header->size+sizeof(tag)-size;
				void* buff = malloc(prevSize);
				memcpy(buff, (void*)((size_t)curr_header + sizeof(tag)), prevSize);
				next_header = (tag*)((size_t)next_header+next_header->size + sizeof(tag));
				next_header->prevSize = size;
				prev_header->free = true;
				prev_header->size = free_size;
				curr_header = (tag*)((size_t)prev_header + prev_header->size + sizeof(tag));
				curr_header->size = size;
				curr_header->free = false;
				curr_header->prevSize = free_size;
				memcpy((void*)((size_t)curr_header + sizeof(tag)), buff, prevSize);
				delete buff;
				return curr_header;
			}
			else if (prev_header->free && (prev_header->size>(size + curr_header->size))) {
				mem_free(prev_header);
				size_t free_size = prev_header->size + curr_header->size - size;
				void* buff = malloc(prevSize);
				memcpy(buff, (void*)((size_t)curr_header + sizeof(tag)), prevSize);
				prev_header->size = free_size;
				prev_header->free = true;
				curr_header = (tag*)((size_t)prev_header + prev_header->size + sizeof(tag));
				curr_header->size = size;
				curr_header->prevSize = free_size;
				curr_header->free = false;
				next_header->prevSize = size;
				memcpy((void*)((size_t)curr_header + sizeof(tag)), buff, prevSize);
				delete buff;
				return curr_header;
			}
			else if (next_header->free && (next_header->size >(size + curr_header->size))) {
				mem_free(next_header);
				size_t free_size = next_header->size + curr_header->size - size;
				curr_header->size = free_size;
				next_header = (tag*)((size_t)curr_header + curr_header->size + sizeof(tag));
				next_header->prevSize = free_size;
				next_header->size = size;
				next_header->free = false;
				memcpy((void*)((size_t)next_header + sizeof(tag)), (void*)((size_t)curr_header + sizeof(tag)), size);
				next_header = (tag*)((size_t)next_header + next_header->size + sizeof(tag));
				next_header->prevSize = size;
				next_header = (tag*)((size_t)next_header - next_header->prevSize - sizeof(tag));
				return next_header;
			}
			else
			{
				struct tag* new_header = (tag*)mem_alloc(size);
				memcpy((void*)((size_t)new_header + sizeof(tag)), (void*)((size_t)curr_header + sizeof(tag)), curr_header->size);
				mem_free(curr_header);
				next_header = (tag*)((size_t)new_header + new_header->size + sizeof(tag));
				next_header->prevSize = size;
				flag = true;
				return new_header;
			}
		}
		else {
			size_t free_size = 0;
			if (curr_header->size > size + sizeof(tag)) {
				free_size = curr_header->size - size - sizeof(tag);
			}
			if (free_size > sizeof(tag)) {/////////////////////
				curr_header->size = size;
				bool free = false;
				struct tag* prev_header = (tag*)((size_t)curr_header - curr_header->prevSize - sizeof(tag));
				if (prev_header->free) {
					mem_free(prev_header);
					free = true;
				}
				struct tag* next_header = (tag*)((size_t)curr_header + curr_header->size + sizeof(tag));
				next_header->free = true;
				next_header->size = free_size;
				next_header->prevSize = size;
				mem_free(next_header);
				void* buff = malloc(size);
				memcpy(buff, (void*)((size_t)curr_header + sizeof(tag)), size);
				mem_free(curr_header);
				if (!free) {
					prev_header = curr_header;
				}
				prev_header->size -= size+sizeof(tag);
				struct tag* new_header = (tag*)((size_t)prev_header + prev_header->size +sizeof(tag));
				new_header->free = false;
				new_header->size = size;
				new_header->prevSize = prev_header->size;
				next_header = (tag*)((size_t)new_header+new_header->size + sizeof(tag));
				next_header->prevSize = size;
				return new_header;
			}
			else {
				struct tag* new_header = (tag*)mem_alloc(size);
				if (new_header != NULL) {
					memcpy((void*)((size_t)new_header + sizeof(tag)), (void*)((size_t)curr_header + sizeof(tag)), size);
					mem_free(curr_header);
					struct tag* next_header = (tag*)((size_t)new_header + new_header->size + sizeof(tag));
					next_header->prevSize = size;
					flag = true;
				}
				return new_header;
			}
		}
	}

	void mem_free(void *addr) {
		struct tag* curr_header = (tag*)addr;
		if (curr_header->size == 0) {
			return;
		}
		curr_header->free = true;
		struct tag* prev_header = (tag*)((size_t)curr_header - curr_header->prevSize-sizeof(tag));
		struct tag* next_header = (tag*)((size_t)curr_header + curr_header->size + sizeof(tag));
		while (prev_header->free) {
			prev_header->size += curr_header->size + sizeof(tag);
			next_header->prevSize = prev_header->size;
			curr_header = prev_header;
			prev_header = (tag*)((size_t)curr_header - curr_header->prevSize - sizeof(tag));
		}
		while (next_header->free) {
			curr_header->size += next_header->size + sizeof(tag);
			next_header = (tag*)((size_t)curr_header + curr_header->size + sizeof(tag));
			next_header->prevSize = curr_header->size;
		}
	}

private:

	size_t mem_size;
	void *mem_start;
	bool flag;

	void header_dump(vector<Item> &ptrs, int &size) {
		void* curr_header = (void*)((size_t)mem_start+sizeof(tag));
		int i = 0;
		Item *p;
		while (!(((tag*)curr_header)->free == false && ((tag*)curr_header)->size == 0)) {
			if (i > size - 1) {
				p = new Item;
				p->ptr = curr_header;
				p->checksum = 0;
				ptrs.insert(ptrs.begin() + i, *p);
				size++;
				//curr_header = (void*)((size_t)curr_header + ((tag*)curr_header)->size + sizeof(tag));
			}
			else if (curr_header!=ptrs[i].ptr) {
				if(curr_header<ptrs[i].ptr){
					while (curr_header < ptrs[i].ptr) {
						p = new Item;
						p->ptr = curr_header;
						p->checksum = 0;
						ptrs.insert(ptrs.begin() + i, *p);
						size++;
						i++;
						curr_header = (void*)((size_t)curr_header + ((tag*)curr_header)->size + sizeof(tag));
					}
					while (curr_header > ptrs[i].ptr) {
						ptrs.erase(ptrs.begin() + i);
						size--;
					}
					if (curr_header != ptrs[i].ptr) {
						p = new Item;
						p->ptr = curr_header;
						p->checksum = 0;
						ptrs.insert(ptrs.begin() + i, *p);
						size++;
					}
				}
				else {
					while (curr_header > ptrs[i].ptr) {
						ptrs.erase(ptrs.begin() + i);
						size--;
					}
					if (curr_header != ptrs[i].ptr) {
						p = new Item;
						p->ptr = curr_header;
						p->checksum = 0;
						ptrs.insert(ptrs.begin() + i, *p);
						size++;
					}
				}
			}
			i++;
			curr_header = (void*)((size_t)curr_header+((tag*)curr_header)->size + sizeof(tag));
		}
	}

	void make_string(int num,tag *addr)
	{
		cout << " "<<num<< "\t| ";
		if (addr->free) {
			cout << "free\t| ";
		}
		else 
		{
			cout << "busy\t| ";
		}
		cout << addr->size<<"\t| "<<addr<<"\n";
	}

};


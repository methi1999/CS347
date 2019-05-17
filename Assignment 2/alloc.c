#include "alloc.h"
// #include <iostream>

/* Code to allocate page of 4KB size with mmap() call and
* initialization of other necessary data structures.
* return 0 on success and 1 on failure (e.g if mmap() fails)
*/
int init()
{
	// Write your code below
	//Initialise array to 0 - free, 1 - full
	for (int i = 0; i < NCHUNKS; ++i){
		array[i] = 0;
		size_array[i] = 0;
	}

	void * page = mmap(
	NULL,								//addr
	PAGESIZE,							//create one page
	PROT_READ|PROT_WRITE|PROT_EXEC,		//Permissions - read, write and execute
	MAP_SHARED|MAP_ANONYMOUS,-1,0);

	if (page == MAP_FAILED) {
    	// std::cout << "Could not mmap";
    	return 1;
  	}
  	region = (char *)page;
  	// std::cout << "Done" << std::endl << std::endl;

  	return 0;

}

/* optional cleanup with munmap() call
* return 0 on success and 1 on failure (if munmap() fails)
*/
int cleanup()
{

	// Write your code below
	int delete_page = munmap((void *)region, PAGESIZE);
	if (delete_page == 0)
		return 0;
	else
		return -1;
}

/* Function to allocate memory of given size
* argument: bufSize - size of the buffer
* return value: on success - returns pointer to starting address of allocated memory
*			    on failure (not able to allocate) - returns NULL
*/
char *alloc(int bufSize)
{
	// write your code below
	if (bufSize % 8 != 0)
		return NULL;
	int chunks_required = bufSize/8;
	int start = 0, end = 0, found = 0;
	int found_buffer = 0;

	while (end<NCHUNKS){
		
		if(array[end] == 0){
			// std::cout << "Found free at " << end << std::endl;
			found++;
			end++;
		}

		else{
			// std::cout << "Not free at " << end << std::endl;
			end++;
			start = end;
			found = 0;
		}

		if(found == chunks_required){
			found_buffer = 1;
			break;
		}
	}
	if (found_buffer == 0)
		return NULL;
	else {
		for (int i = start; i < end; ++i){
			array[i] = 1;
			size_array[i] = chunks_required;
		}
	}
	
	// printf("%i %i\n",start,end-1);
	// printf("%p\n",(void *)(region+8*start));

	return (region+start*8);

}


/* Function to free the memory
* argument: takes the starting address of an allocated buffer
*/
void dealloc(char *memAddr)
{
	// write your code below
	int index = (memAddr-region)/8;
	int size_of_chunk = size_array[index];
	// printf("Deallocating from %i to %i",index,index+size_array[index]-1);
	for (int i = index; i < index+size_of_chunk; ++i)
	{
		*(region+i) = '\0';
		array[i] = 0;
	}

	for (int i = index; i < size_of_chunk; ++i)
		size_array[i] = 0;

}



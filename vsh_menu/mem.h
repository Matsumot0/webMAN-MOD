#ifndef __MEM_H__
#define __MEM_H__


#define MB(x)      ((x)*1024*1024)


void create_heap(int32_t size);
void destroy_heap(void);
void *mem_alloc(uint32_t size);


#endif // __MEM_H__ 

#include <sys/memory.h>

#include "mem.h"
#include "inc/vsh_exports.h"
#include "network.h"            // debug


static sys_memory_container_t mc_app = (sys_memory_container_t)-1;
static sys_addr_t heap_mem = 0;
static uint32_t prx_heap = 0;



/***********************************************************************
* create prx heap from memory container 1("app")
***********************************************************************/
void create_heap(int32_t size)
{
  mc_app = vsh_memory_container_by_id(1);
  sys_memory_allocate_from_container(MB(size), mc_app, SYS_MEMORY_PAGE_SIZE_1M, &heap_mem);
  prx_heap = (uint32_t)heap_mem;
}

/***********************************************************************
* 
***********************************************************************/
void destroy_heap(void)
{
  sys_memory_free((sys_addr_t)heap_mem);
  prx_heap = 0;
  mc_app = (sys_memory_container_t)-1;
}

/***********************************************************************
* 
***********************************************************************/
void *mem_alloc(uint32_t size)
{
	uint32_t add = prx_heap;
	prx_heap += size;
  
	return (void*)add;
}





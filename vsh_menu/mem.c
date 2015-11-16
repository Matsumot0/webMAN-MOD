#include <sys/memory.h>

#include "include/mem.h"
#include "include/vsh_exports.h"
//#include "include/network.h"	// debug


static sys_memory_container_t mc_app = (sys_memory_container_t)-1;
static sys_addr_t heap_mem = 0;
static uint32_t prx_heap = 0;



/***********************************************************************
* create prx heap from memory container 1("app")
***********************************************************************/
int32_t create_heap(int32_t size)
{
  mc_app = vsh_memory_container_by_id(1);
  if (!mc_app) return(-1);

  sys_memory_allocate_from_container(MB(size), mc_app, SYS_MEMORY_PAGE_SIZE_1M, &heap_mem);
  if (!heap_mem) return(-1);

  prx_heap = (uint32_t)heap_mem;
  return(0);
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

/***********************************************************************
*
***********************************************************************/
int32_t mem_free(uint32_t size)
{
	if(prx_heap>=size) prx_heap -= size; else return (-1);
	return(0);
}

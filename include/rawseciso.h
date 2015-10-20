#ifdef COBRA_ONLY

enum STORAGE_COMMAND
{
	CMD_READ_ISO,
	CMD_READ_DISC,
	CMD_READ_CD_ISO_2352,
	CMD_FAKE_STORAGE_EVENT,
	CMD_GET_PSX_VIDEO_MODE
};

typedef struct
{
	uint64_t device;
        uint32_t emu_mode;
        uint32_t num_sections;
        uint32_t num_tracks;
} __attribute__((packed)) rawseciso_args;

static uint32_t handle = -1;

static sys_device_info_t disc_info;
static uint32_t *sections, *sections_size;
static uint32_t num_sections;
uint64_t sector_size = 0x200;
static sys_event_queue_t command_queue_ntfs = -1;
static u8 rawseciso_loaded=0;

#define CD_CACHE_SIZE			(64)

#define CD_SECTOR_SIZE_2048   2048
static u32 CD_SECTOR_SIZE_2352 = 2352;

static uint64_t discsize=0;
static int is_cd2352=0;
static uint8_t *cd_cache=0;
static uint32_t cached_cd_sector=0x80000000;

static inline void get_next_read(uint64_t discoffset, uint64_t bufsize, uint64_t *offset, uint64_t *readsize, int *idx)
{
	uint64_t base = 0;
	*idx = -1;
	*readsize = bufsize;
	*offset = 0;

	for(uint32_t i = 0; i < num_sections; i++)
	{
		uint64_t last = base + ((uint64_t)sections_size[i] * sector_size);

		if(discoffset >= base && discoffset < last)
		{
			uint64_t maxfileread = last-discoffset;

			if(bufsize > maxfileread)
				*readsize = maxfileread;
			else
				*readsize = bufsize;

			*idx = i;
			*offset = discoffset-base;
			return;
		}

		base += ((uint64_t)sections_size[i] * sector_size);
	}

	// We can be here on video blu-ray
	//DPRINTF("Offset or size out of range  %lx%08lx   %lx!!!!!!!!\n", discoffset>>32, discoffset, bufsize);
}

static int process_read_iso_cmd_iso(uint8_t *buf, uint64_t offset, uint64_t size)
{
	uint64_t remaining;

	//DPRINTF("read iso: %p %lx %lx\n", buf, offset, size);
	remaining = size;

	while(remaining > 0)
	{
		uint64_t pos, readsize;
		int idx;
		int ret;
		uint8_t tmp[sector_size];
		uint32_t sector;
		uint32_t r;

		get_next_read(offset, remaining, &pos, &readsize, &idx);

		if(idx == -1 || sections[idx] == 0xFFFFFFFF)
		{
			memset(buf, 0, readsize);
			buf += readsize;
			offset += readsize;
			remaining -= readsize;
			continue;
		}

		if(pos & (sector_size-1))
		{
			uint64_t csize;

			sector = sections[idx] + pos/sector_size;
			ret = sys_storage_read(handle, 0, sector, 1, tmp, &r, 0);
			if(ret != 0 || r != 1)
			{
				//DPRINTF("sys_storage_read failed: %x 1 -> %x\n", sector, ret);
				return FAILED;
			}

			csize = sector_size-(pos&(sector_size-1));

			if(csize > readsize)
				csize = readsize;

			memcpy(buf, tmp+(pos&(sector_size-1)), csize);
			buf += csize;
			offset += csize;
			pos += csize;
			remaining -= csize;
			readsize -= csize;
		}

		if(readsize > 0)
		{
			uint32_t n = readsize / sector_size;

			if(n > 0)
			{
				uint64_t s;

				sector = sections[idx] + pos/sector_size;
				ret = sys_storage_read(handle, 0, sector, n, buf, &r, 0);
				if(ret != 0 || r != n)
				{
					//DPRINTF("sys_storage_read failed: %x %x -> %x\n", sector, n, ret);
					return FAILED;
				}

				s = n * sector_size;
				buf += s;
				offset += s;
				pos += s;
				remaining -= s;
				readsize -= s;
			}

			if(readsize > 0)
			{
				sector = sections[idx] + pos/sector_size;
				ret = sys_storage_read(handle, 0, sector, 1, tmp, &r, 0);
				if(ret != 0 || r != 1)
				{
					//DPRINTF("sys_storage_read failed: %x 1 -> %x\n", sector, ret);
					return FAILED;
				}

				memcpy(buf, tmp, readsize);
				buf += readsize;
				offset += readsize;
				remaining -= readsize;
			}
		}
	}

	return 0;
}

static inline void my_memcpy(uint8_t *dst, uint8_t *src, uint16_t size)
{
	for(uint16_t i = 0; i < size; i++)
		dst[i] = src[i];
}

static int process_read_cd_2048_cmd_iso(uint8_t *buf, uint32_t start_sector, uint32_t sector_count)
{
	uint32_t capacity = sector_count * CD_SECTOR_SIZE_2048;
	uint32_t fit = capacity / CD_SECTOR_SIZE_2352;
	uint32_t rem = (sector_count-fit);
	uint32_t i;
	uint8_t *in = buf;
	uint8_t *out = buf;

	if(fit > 0)
	{
		process_read_iso_cmd_iso(buf, start_sector * CD_SECTOR_SIZE_2352, fit * CD_SECTOR_SIZE_2352);

		for(i = 0; i < fit; i++)
		{
			my_memcpy(out, in+24, CD_SECTOR_SIZE_2048);
			in  += CD_SECTOR_SIZE_2352;
			out += CD_SECTOR_SIZE_2048;
			start_sector++;
		}
	}

	for(i = 0; i < rem; i++)
	{
		process_read_iso_cmd_iso(out, (start_sector * CD_SECTOR_SIZE_2352) + 24, CD_SECTOR_SIZE_2048);
		out += CD_SECTOR_SIZE_2048;
		start_sector++;
	}

	return 0;
}

static int process_read_cd_2352_cmd_iso(uint8_t *buf, uint32_t sector, uint32_t remaining)
{
	int cache = 0;

	if(remaining <= CD_CACHE_SIZE)
	{
		int dif = (int)cached_cd_sector-sector;

		if(ABS(dif) < CD_CACHE_SIZE)
		{
			uint8_t *copy_ptr = NULL;
			uint32_t copy_offset = 0;
			uint32_t copy_size = 0;

			if(dif > 0)
			{
				if(dif < (int)remaining)
				{
					copy_ptr = cd_cache;
					copy_offset = dif;
					copy_size = remaining-dif;
				}
			}
			else
			{
				copy_ptr = cd_cache+((-dif) * CD_SECTOR_SIZE_2352);
				copy_size = MIN((int)remaining, CD_CACHE_SIZE+dif);
			}

			if(copy_ptr)
			{
				memcpy(buf+(copy_offset * CD_SECTOR_SIZE_2352), copy_ptr, copy_size * CD_SECTOR_SIZE_2352);

				if(remaining == copy_size)
				{
					return 0;
				}

				remaining -= copy_size;

				if(dif <= 0)
				{
					uint32_t newsector = cached_cd_sector + CD_CACHE_SIZE;
					buf += ((newsector-sector) * CD_SECTOR_SIZE_2352);
					sector = newsector;
				}
			}
		}

		cache = 1;
	}

	if(!cache)
	{
		return process_read_iso_cmd_iso(buf, sector * CD_SECTOR_SIZE_2352, remaining * CD_SECTOR_SIZE_2352);
	}

	if(!cd_cache)
	{
		sys_addr_t addr=0;

		int ret = sys_memory_allocate(_192KB_, SYS_MEMORY_PAGE_SIZE_64K, &addr);
		if(ret != 0)
		{
			//DPRINTF("sys_memory_allocate failed: %x\n", ret);
			return ret;
		}

		cd_cache = (uint8_t *)addr;
	}

	if(process_read_iso_cmd_iso(cd_cache, sector * CD_SECTOR_SIZE_2352, CD_CACHE_SIZE * CD_SECTOR_SIZE_2352) != 0)
		return FAILED;

	memcpy(buf, cd_cache, remaining * CD_SECTOR_SIZE_2352);
	cached_cd_sector = sector;
	return 0;
}

static int sys_storage_ext_mount_discfile_proxy(sys_event_port_t result_port, sys_event_queue_t command_queue, int emu_type, uint64_t disc_size_bytes, uint32_t read_size, unsigned int trackscount, ScsiTrackDescriptor *tracks)
{
	system_call_8(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_MOUNT_DISCFILE_PROXY, result_port, command_queue, emu_type, disc_size_bytes, read_size, trackscount, (uint64_t)(uint32_t)tracks);
	return (int)p1;
}

static void rawseciso_stop_thread(uint64_t arg)
{
	uint64_t exit_code;
	rawseciso_loaded = 1;

	if(command_queue_ntfs != (sys_event_queue_t)-1)
	{
		if(sys_event_queue_destroy(command_queue_ntfs, SYS_EVENT_QUEUE_DESTROY_FORCE) != 0)
		{
			//DPRINTF("Failed in destroying command_queue\n");
		}
	}

	if(thread_id_ntfs != (sys_ppu_thread_t)-1)
	{
		sys_ppu_thread_join(thread_id_ntfs, &exit_code);
	}

	rawseciso_loaded = 0;
	sys_ppu_thread_exit(0);
}

static void rawseciso_thread(uint64_t arg)
{
	rawseciso_args *args;
	sys_event_port_t result_port;
	sys_event_queue_attribute_t queue_attr;
	unsigned int real_disctype;
	int ret;
	ScsiTrackDescriptor *tracks;
	int emu_mode, num_tracks;
	int cd_sector_size_param = 0;

	CD_SECTOR_SIZE_2352 = 2352;

	args = (rawseciso_args *)(uint32_t)arg;

	num_sections = args->num_sections;
	sections = (uint32_t *)(args+1);
	sections_size = sections + num_sections;

	sector_size = 0x200;
	if(args->device!=0)
	{
		for(u8 retry = 0; retry < 16; retry++)
		{
			if(sys_storage_get_device_info(args->device, &disc_info) == 0) {sector_size = (uint32_t) disc_info.sector_size; break;}
			sys_timer_usleep(500000);
		}
	}

	discsize = 0;

	for(uint32_t i = 0; i < num_sections; i++)
	{
		discsize += sections_size[i];
	}

	discsize = discsize * sector_size;

	emu_mode = args->emu_mode & 0xF;
	if(emu_mode == EMU_PSX)
	{
		num_tracks = args->num_tracks;

        if(num_tracks > 100)
        {
            CD_SECTOR_SIZE_2352 = (num_tracks & 0xffff00)>>4;
            if(CD_SECTOR_SIZE_2352 != 2352 && CD_SECTOR_SIZE_2352 != 2048 && CD_SECTOR_SIZE_2352 != 2336 && CD_SECTOR_SIZE_2352 != 2448) CD_SECTOR_SIZE_2352 = 2352;
        }

        if(CD_SECTOR_SIZE_2352 != 2352) cd_sector_size_param = CD_SECTOR_SIZE_2352<<4;

		num_tracks &= 0xff;

		tracks = (ScsiTrackDescriptor *)(sections_size + num_sections);
		is_cd2352 = 1;
	}
	else
	{
		num_tracks = 0;
		tracks = NULL;
		is_cd2352 = 0;
	}

	//DPRINTF("discsize = %lx%08lx\n", discsize>>32, discsize);

	ret = sys_storage_open(args->device, 0, &handle, 0);
	if(ret != 0)
	{
		//DPRINTF("sys_storage_open failed: %x\n", ret);
		sys_memory_free((sys_addr_t)args);
		sys_ppu_thread_exit(ret);
	}

	ret = sys_event_port_create(&result_port, 1, SYS_EVENT_PORT_NO_NAME);
	if(ret != 0)
	{
		//DPRINTF("sys_event_port_create failed: %x\n", ret);
		sys_storage_close(handle);
		sys_memory_free((sys_addr_t)args);
		sys_ppu_thread_exit(ret);
	}

	sys_event_queue_attribute_initialize(queue_attr);
	ret = sys_event_queue_create(&command_queue_ntfs, &queue_attr, 0, 1);
	if(ret != 0)
	{
		//DPRINTF("sys_event_queue_create failed: %x\n", ret);
		sys_event_port_destroy(result_port);
		sys_storage_close(handle);
		sys_memory_free((sys_addr_t)args);
		sys_ppu_thread_exit(ret);
	}

	sys_storage_ext_get_disc_type(&real_disctype, NULL, NULL);

	if(real_disctype != 0)
	{
		fake_eject_event(BDVD_DRIVE);
	}

	if(is_cd2352)
	{
		if(discsize % CD_SECTOR_SIZE_2352)
		{
			discsize = discsize - (discsize % CD_SECTOR_SIZE_2352);
		}
	}

	ret = sys_storage_ext_mount_discfile_proxy(result_port, command_queue_ntfs, emu_mode, discsize, _256KB_, (num_tracks | cd_sector_size_param), tracks);
	//DPRINTF("mount = %x\n", ret);


	fake_insert_event(BDVD_DRIVE, real_disctype);

	if(ret != 0)
	{
		sys_event_port_disconnect(result_port);
		// Queue destroyed in stop thread sys_event_queue_destroy(command_queue_ntfs);
		sys_event_port_destroy(result_port);
		sys_storage_close(handle);

		sys_memory_free((sys_addr_t)args);
		sys_ppu_thread_exit(0);
	}

	rawseciso_loaded=1;
	while(rawseciso_loaded)
	{
		sys_event_t event;

		ret = sys_event_queue_receive(command_queue_ntfs, &event, 0);
		if(ret != 0)
		{
			//DPRINTF("sys_event_queue_receive failed: %x\n", ret);
			break;
		}

		void *buf = (void *)(uint32_t)(event.data3>>32ULL);
		uint64_t offset = event.data2;
		uint32_t size = event.data3&0xFFFFFFFF;

		switch(event.data1)
		{
			case CMD_READ_ISO:
			{
				if(is_cd2352)
				{
					ret = process_read_cd_2048_cmd_iso(buf, offset / CD_SECTOR_SIZE_2048, size / CD_SECTOR_SIZE_2048);
				}
				else
				{
					ret = process_read_iso_cmd_iso(buf, offset, size);
				}
			}
			break;

			case CMD_READ_CD_ISO_2352:
			{
				ret = process_read_cd_2352_cmd_iso(buf, offset / CD_SECTOR_SIZE_2352, size / CD_SECTOR_SIZE_2352);
			}
			break;
		}

		ret = sys_event_port_send(result_port, ret, 0, 0);
		if(ret != 0)
		{
			//DPRINTF("sys_event_port_send failed: %x\n", ret);
			break;
		}
	}

	sys_storage_ext_get_disc_type(&real_disctype, NULL, NULL);
	fake_eject_event(BDVD_DRIVE);
	sys_storage_ext_umount_discfile();

	if(real_disctype != 0)
	{
		fake_insert_event(BDVD_DRIVE, real_disctype);
	}

	if(cd_cache)
	{
		sys_memory_free((sys_addr_t)cd_cache);
	}

	sys_storage_close(handle);

	sys_event_port_disconnect(result_port);
	if(sys_event_port_destroy(result_port) != 0)
	{
		//DPRINTF("Error destroyng result_port\n");
	}

	// queue destroyed in stop thread

	//DPRINTF("Exiting main thread!\n");
	rawseciso_loaded = 0;

	sys_memory_free((sys_addr_t)args);
	sys_ppu_thread_exit(0);
}

#endif
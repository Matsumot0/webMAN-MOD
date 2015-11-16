#ifndef LITE_EDITION
typedef struct
{
	char server[0x40];
	char path[0x420];
	uint32_t emu_mode;
	uint32_t num_tracks;
	uint16_t port;
	uint8_t pad[6];
	ScsiTrackDescriptor tracks[1];
} __attribute__((packed)) netiso_args;

static int g_socket = -1;
static sys_event_queue_t command_queue = -1;

int connect_to_remote_server(u8 server_id);

static u8 netiso_loaded=0;

static int remote_stat(int s, char *path, int *is_directory, int64_t *file_size, uint64_t *mtime, uint64_t *ctime, uint64_t *atime, int *abort_connection);

static int remote_stat(int s, char *path, int *is_directory, int64_t *file_size, uint64_t *mtime, uint64_t *ctime, uint64_t *atime, int *abort_connection)
{
	netiso_stat_cmd cmd;
	netiso_stat_result res;
	int len;

	*abort_connection = 1;

	len = strlen(path);
	memset(&cmd, 0, sizeof(cmd));
	cmd.opcode = (NETISO_CMD_STAT_FILE);
	cmd.fp_len = (len);

	if(send(s, &cmd, sizeof(cmd), 0) != sizeof(cmd))
	{
		//DPRINTF("send failed (remote_stat) (errno=%d)!\n", get_network_error());
		return FAILED;
	}

	if(send(s, path, len, 0) != len)
	{
		//DPRINTF("send failed (remote_stat) (errno=%d)!\n", get_network_error());
		return FAILED;
	}

	if(recv(s, &res, sizeof(res), MSG_WAITALL) != sizeof(res))
	{
		//DPRINTF("recv failed (remote_stat) (errno=%d)!\n", get_network_error());
		return FAILED;
	}

	*abort_connection = 0;

	*file_size = (res.file_size);
	if(*file_size == -1)
		return FAILED;

	*is_directory = res.is_directory;
	*mtime = (res.mtime);
	*ctime = (res.ctime);
	*atime = (res.atime);

	return 0;
}

static int read_remote_file(int s, void *buf, uint64_t offset, uint32_t size, int *abort_connection)
{
	netiso_read_file_cmd cmd;
	netiso_read_file_result res;

	*abort_connection = 1;

	memset(&cmd, 0, sizeof(cmd));
	cmd.opcode = (NETISO_CMD_READ_FILE);
	cmd.offset = (offset);
	cmd.num_bytes = (size);

	if(send(s, &cmd, sizeof(cmd), 0) != sizeof(cmd))
	{
		//DPRINTF("send failed (read_remote_file) (errno=%d)!\n", get_network_error());
		return FAILED;
	}

	if(recv(s, &res, sizeof(res), MSG_WAITALL) != sizeof(res))
	{
		//DPRINTF("recv failed (read_remote_file) (errno=%d)!\n", get_network_error());
		return FAILED;
	}

	*abort_connection = 0;

	int bytes_read = (res.bytes_read);
	if(bytes_read <= 0)
		return bytes_read;

	if(recv(s, buf, bytes_read, MSG_WAITALL) != bytes_read)
	{
		//DPRINTF("recv failed (read_remote_file) (errno=%d)!\n", get_network_error());
		*abort_connection = 1;
		return FAILED;
	}

	return bytes_read;
}

static int64_t open_remote_file_2(int s, char *path, int *abort_connection)
{
	netiso_open_cmd cmd;
	netiso_open_result res;
	int len;

	*abort_connection = 1;

	len = strlen(path);
	memset(&cmd, 0, sizeof(cmd));
	cmd.opcode = BE16(NETISO_CMD_OPEN_FILE);
	cmd.fp_len = BE16(len);

	if(send(s, &cmd, sizeof(cmd), 0) != sizeof(cmd))
	{
		//DPRINTF("send failed (open_remote_file) (errno=%d)!\n", get_network_error());
		return FAILED;
	}

	if(send(s, path, len, 0) != len)
	{
		//DPRINTF("send failed (open_remote_file) (errno=%d)!\n", get_network_error());
		return FAILED;
	}

	if(recv(s, &res, sizeof(res), MSG_WAITALL) != sizeof(res))
	{
		//DPRINTF("recv failed (open_remote_file) (errno=%d)!\n", get_network_error());
		return FAILED;
	}

	*abort_connection = 0;

	return (res.file_size);
}

static int64_t open_remote_file(char *path)
{
	netiso_open_cmd cmd;
	netiso_open_result res;
	int len;

	len = strlen(path);

	memset(&cmd, 0, sizeof(cmd));
	cmd.opcode = NETISO_CMD_OPEN_FILE;
	cmd.fp_len = len;

	if(send(g_socket, &cmd, sizeof(cmd), 0) != sizeof(cmd))
	{
		//DPRINTF("send failed (open_file) (errno=%d)!\n", sys_net_errno);
		return FAILED;
	}

	if(send(g_socket, path, len, 0) != len)
	{
		//DPRINTF("send failed (open_file) (errno=%d)!\n", sys_net_errno);
		return FAILED;
	}

	if(recv(g_socket, &res, sizeof(res), MSG_WAITALL) != sizeof(res))
	{
		//DPRINTF("recv failed (open_file) (errno=%d)!\n", sys_net_errno);
		return FAILED;
	}

	if(res.file_size == -1)
	{
		//DPRINTF("Remote file %s doesn't exist!\n", path);
		return FAILED;
	}

	//DPRINTF("Remote file %s opened. Size = %x%08lx bytes\n", path, (res.file_size>>32), res.file_size&0xFFFFFFFF);
	return res.file_size;
}

static int read_remote_file_critical(uint64_t offset, void *buf, uint32_t size)
{
	netiso_read_file_critical_cmd cmd;

	memset(&cmd, 0, sizeof(cmd));
	cmd.opcode = NETISO_CMD_READ_FILE_CRITICAL;
	cmd.num_bytes = size;
	cmd.offset = offset;

	if(send(g_socket, &cmd, sizeof(cmd), 0) != sizeof(cmd))
	{
		//DPRINTF("send failed (read file) (errno=%d)!\n", sys_net_errno);
		return FAILED;
	}

	if(recv(g_socket, buf, size, MSG_WAITALL) != (int)size)
	{
		//DPRINTF("recv failed (recv file)  (errno=%d)!\n", sys_net_errno);
		return FAILED;
	}

	return 0;
}

static int process_read_cd_2048_cmd(uint8_t *buf, uint32_t start_sector, uint32_t sector_count)
{
	netiso_read_cd_2048_critical_cmd cmd;

	memset(&cmd, 0, sizeof(cmd));
	cmd.opcode = NETISO_CMD_READ_CD_2048_CRITICAL;
    //cmd.pad = CD_SECTOR_SIZE_2352;
	cmd.start_sector = start_sector;
	cmd.sector_count = sector_count;

	if(send(g_socket, &cmd, sizeof(cmd), 0) != sizeof(cmd))
	{
		//DPRINTF("send failed (read 2048) (errno=%d)!\n", sys_net_errno);
		return FAILED;
	}

	if(recv(g_socket, buf, sector_count * CD_SECTOR_SIZE_2048, MSG_WAITALL) != (int)(sector_count * CD_SECTOR_SIZE_2048))
	{
		//DPRINTF("recv failed (read 2048)  (errno=%d)!\n", sys_net_errno);
		return FAILED;
	}

	return 0;
}

static int process_read_iso_cmd(uint8_t *buf, uint64_t offset, uint32_t size)
{
	uint64_t read_end;

	//DPRINTF("read iso: %p %lx %x\n", buf, offset, size);
	read_end = offset + size;

	if(read_end >= discsize)
	{
		//DPRINTF("Read beyond limits: %llx %x (discsize=%llx)!\n", offset, size, discsize);

		if(offset >= discsize)
		{
			memset(buf, 0, size);
			return 0;
		}

		memset(buf+(discsize-offset), 0, read_end-discsize);
		size = discsize-offset;
	}

	return read_remote_file_critical(offset, buf, size);
}

static int process_read_cd_2352_cmd(uint8_t *buf, uint32_t sector, uint32_t remaining)
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
		return process_read_iso_cmd(buf, sector * CD_SECTOR_SIZE_2352, remaining * CD_SECTOR_SIZE_2352);
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

	if(process_read_iso_cmd(cd_cache, sector * CD_SECTOR_SIZE_2352, CD_CACHE_SIZE * CD_SECTOR_SIZE_2352) != 0)
		return FAILED;

	memcpy(buf, cd_cache, remaining * CD_SECTOR_SIZE_2352);
	cached_cd_sector = sector;
	return 0;
}

static void netiso_thread(uint64_t arg)
{
	netiso_args *args;
	sys_event_port_t result_port;
	sys_event_queue_attribute_t queue_attr;
	unsigned int real_disctype;
	int64_t ret64;
	int ret;
	ScsiTrackDescriptor *tracks;
	int emu_mode, num_tracks;
	unsigned int cd_sector_size_param = 0;

	args = (netiso_args *)(uint32_t)arg;

	//DPRINTF("Hello VSH\n");

	g_socket = connect_to_server(args->server, args->port);
	if(g_socket < 0)
	{
		sys_memory_free((sys_addr_t)args);
		sys_ppu_thread_exit(0);
	}

	ret64 = open_remote_file(args->path);
	if(ret64 < 0)
	{
		sys_memory_free((sys_addr_t)args);
		sys_ppu_thread_exit(0);
	}

	discsize = (uint64_t)ret64;

	ret = sys_event_port_create(&result_port, 1, SYS_EVENT_PORT_NO_NAME);
	if(ret != 0)
	{
		//DPRINTF("sys_event_port_create failed: %x\n", ret);
		sys_memory_free((sys_addr_t)args);
		sys_ppu_thread_exit(ret);
	}

	sys_event_queue_attribute_initialize(queue_attr);
	ret = sys_event_queue_create(&command_queue, &queue_attr, 0, 1);
	if(ret != 0)
	{
		//DPRINTF("sys_event_queue_create failed: %x\n", ret);
		sys_memory_free((sys_addr_t)args);
		sys_ppu_thread_exit(ret);
	}

	CD_SECTOR_SIZE_2352 = 2352;

	emu_mode = args->emu_mode & 0xF;
	if(emu_mode == EMU_PSX)
	{
		num_tracks = args->num_tracks;
		tracks = &args->tracks[0];
		is_cd2352 = 1;
	}
	else
	{
		num_tracks = 0;
		tracks = NULL;
		is_cd2352 = 0;
	}

	sys_memory_free((sys_addr_t)args);
	sys_storage_ext_get_disc_type(&real_disctype, NULL, NULL);

	if(real_disctype != 0)
	{
		fake_eject_event(BDVD_DRIVE);
	}

	if(discsize % CD_SECTOR_SIZE_2352)
	{
		discsize = discsize - (discsize % CD_SECTOR_SIZE_2352);
	}

	ret = sys_storage_ext_mount_discfile_proxy(result_port, command_queue, emu_mode, discsize, _256KB_, (num_tracks | cd_sector_size_param), tracks);
	//DPRINTF("mount = %x\n", ret);

	fake_insert_event(BDVD_DRIVE, real_disctype);

	if(ret != 0)
	{
		sys_event_port_destroy(result_port);
		sys_ppu_thread_exit(0);
	}

	netiso_loaded=1;
	while(netiso_loaded)
	{
		sys_event_t event;

		ret = sys_event_queue_receive(command_queue, &event, 0);
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
					ret = process_read_cd_2048_cmd(buf, offset / CD_SECTOR_SIZE_2048, size / CD_SECTOR_SIZE_2048);
				}
				else
				{
					ret = process_read_iso_cmd(buf, offset, size);
				}
			}
			break;

			case CMD_READ_CD_ISO_2352:
			{
				ret = process_read_cd_2352_cmd(buf, offset / CD_SECTOR_SIZE_2352, size / CD_SECTOR_SIZE_2352);
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

	if(g_socket >= 0)
	{
		shutdown(g_socket, SHUT_RDWR);
		socketclose(g_socket);
		g_socket = -1;
	}

	sys_event_port_disconnect(result_port);
	if(sys_event_port_destroy(result_port) != 0)
	{
		//DPRINTF("Error destroyng result_port\n");
	}

	//DPRINTF("Exiting main thread!\n");
	netiso_loaded = 0;
	sys_ppu_thread_exit(0);
}

static void netiso_stop_thread(uint64_t arg)
{
	uint64_t exit_code;
	netiso_loaded = 1;

	if(g_socket >= 0)
	{
		shutdown(g_socket, SHUT_RDWR);
		socketclose(g_socket);
		g_socket = -1;
	}

	if(command_queue != (sys_event_queue_t)-1)
	{
		if(sys_event_queue_destroy(command_queue, SYS_EVENT_QUEUE_DESTROY_FORCE) != 0)
		{
			//DPRINTF("Failed in destroying command_queue\n");
		}
	}

	if(thread_id_net != (sys_ppu_thread_t)-1)
	{
		sys_ppu_thread_join(thread_id_net, &exit_code);
	}

	netiso_loaded = 0;
	sys_ppu_thread_exit(0);
}

int connect_to_remote_server(u8 server_id)
{
	int ns=FAILED;
	if( (server_id==0 && webman_config->netp0 && webman_config->neth0[0]) ||
		(server_id==1 && webman_config->netp1 && webman_config->neth1[0]) ||
		(server_id==2 && webman_config->netp2 && webman_config->neth2[0]) )
	{
		if(server_id==1 && strcmp(webman_config->neth0, webman_config->neth1)==0 && webman_config->netp0==webman_config->netp1 && webman_config->netd0==webman_config->netd1) return FAILED;
		if(server_id==2 && strcmp(webman_config->neth0, webman_config->neth2)==0 && webman_config->netp0==webman_config->netp2 && webman_config->netd0==webman_config->netd2) return FAILED;
		if(server_id==2 && strcmp(webman_config->neth1, webman_config->neth2)==0 && webman_config->netp1==webman_config->netp2 && webman_config->netd1==webman_config->netd2) return FAILED;

		u8 retries=0;
	reconnect:
		if(server_id==0) ns=connect_to_server(webman_config->neth0, webman_config->netp0); else
		if(server_id==1) ns=connect_to_server(webman_config->neth1, webman_config->netp1); else
		if(server_id==2) ns=connect_to_server(webman_config->neth2, webman_config->netp2);

		if(ns<0)
		{
			if(retries<3)
			{
				retries++;
				sys_timer_sleep(1);
				goto reconnect;
			}
		}
	}
	return ns;
}

static int open_remote_dir(int s, char *path, int *abort_connection)
{
	netiso_open_dir_cmd cmd;
	netiso_open_dir_result res;
	int len;

	*abort_connection = 0;

	len = strlen(path);
	memset(&cmd, 0, sizeof(cmd));
	cmd.opcode = (NETISO_CMD_OPEN_DIR);
	cmd.dp_len = (len);

	if(send(s, &cmd, sizeof(cmd), 0) != sizeof(cmd))
	{
		//DPRINTF("send failed (open_remote_dir) (errno=%d)!\n", get_network_error());
		*abort_connection = 1;
		return FAILED;
	}

	if(send(s, path, len, 0) != len)
	{
		//DPRINTF("send failed (open_remote_dir) (errno=%d)!\n", get_network_error());
		*abort_connection = 1;
		return FAILED;
	}

	if(recv(s, &res, sizeof(res), MSG_WAITALL) != sizeof(res))
	{
		//DPRINTF("recv failed (open_remote_dir) (errno=%d)!\n", get_network_error());
		*abort_connection = 1;
		return FAILED;
	}

	return (res.open_result);
}

static int read_remote_dir(int s, sys_addr_t *data /*netiso_read_dir_result_data **data*/, int *abort_connection)
{
	netiso_read_dir_entry_cmd cmd;
	netiso_read_dir_result res;
	int len;

	*abort_connection = 1;

	memset(&cmd, 0, sizeof(cmd));
	cmd.opcode = (NETISO_CMD_READ_DIR);

 //MM_LOG("Sending request...(%i) ", s);
	if(send(s, &cmd, sizeof(cmd), 0) != sizeof(cmd))
	{
 //MM_LOG("FAILED!\n");
		return FAILED;
	}
 //MM_LOG("Receiving response...");
	if(recv(s, &res, sizeof(res), MSG_WAITALL) != sizeof(res))
	{
 //MM_LOG("FAILED!\n");
		return FAILED;
	}

 //MM_LOG("OK (%i entries)\n", res.dir_size );
	if(res.dir_size > 0)
	{
		sys_addr_t data1=0;
		for(int64_t retry=16; retry>0; retry--)
		{
			if(res.dir_size>retry*123) res.dir_size=retry*123;

			len = (sizeof(netiso_read_dir_result_data)*res.dir_size);
			int len2= ((len+_64KB_)/_64KB_)*_64KB_;
			if(sys_memory_allocate(len2, SYS_MEMORY_PAGE_SIZE_64K, &data1)==0)
			{
				*data=data1;
				u8 *data2=(u8*)data1;

				if(recv(s, data2, len, MSG_WAITALL) != len)
				{
					sys_memory_free(data1);
					*data=NULL;
					return FAILED;
				}
				break;
			}
			else
				*data=NULL;
		}
	}
	else
		*data=NULL;

	*abort_connection = 0;

	return (res.dir_size);
}


/*
u64 ssssocket(void)
{
	system_call_3(713, 2, 1, 0);			//socket
	return_to_user_prog(uint64_t);
}


static int connect_to_server2(char *server, uint16_t port)
{
	struct sockaddr_in sin;
	unsigned int temp;
	int s;

	if((temp = inet_addr(server)) != (unsigned int)-1)
	{
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = temp;
	}
	else {
		struct hostent *hp;

		if((hp = gethostbyname(server)) == NULL)
		{
			return FAILED;
		}

		sin.sin_family = hp->h_addrtype;
		memcpy(&sin.sin_addr, hp->h_addr, hp->h_length);
	}

	sin.sin_port = htons(port);

	memcpy(&sin_global, &sin, 16);

	sprintf(netbuf, "Empty buffer...\r\n");

	pokeq(0x80000000007FD0A0ULL, (u64)&sin_global);
	pokeq(0x80000000007FD0A8ULL, (u64)&netbuf);
	pokeq(0x80000000007FD0B0ULL, (u64)&statbuf);
	pokeq(0x80000000007FD0B8ULL, (u64)&direntry);

	//peek(0x80000000007FD608): 0x0002947AC0A864D1 | ...z..d.

	s = ssssocket();
	{system_call_3(702, s, (u64)&sin, sizeof(sin));}		//connect
	{system_call_6(710, s, (u64)&sin, sizeof(sin), 0, (u64)&sin, sizeof(sin));}	//sendto
	//{system_call_6(707, s, (u64)&sin, sizeof(sin), 8, (u64)&sin, sizeof(sin));}	//recvfrom
	{system_call_1(714, s);}				//close
	{system_call_2(712, s, 2);}				//shutdown

	return 0;
}
*/

#endif //#ifndef LITE_EDITION

static void poll_thread(uint64_t poll)
{
	/*u8 d0[157];
	u8 d1[157];
	u8 d0t[157];
	u8 d1t[157];
	int u0=0, u1=0;
	char un[128];

	while(working)
	{
		if(u0<128)
		{
			int fd;
			u64 read_e;
			bool toupd0, toupd1;
			cellFsOpen((char*)"/dev_hdd0/vsh/task/00000001/d0.pdb", CELL_FS_O_RDONLY, &fd, NULL, 0);
			cellFsRead(fd, (void *)&d0, 157, &read_e); cellFsClose(fd);
			cellFsOpen((char*)"/dev_hdd0/vsh/task/00000001/d1.pdb", CELL_FS_O_RDONLY, &fd, NULL, 0);
			cellFsRead(fd, (void *)&d1, 157, &read_e); cellFsClose(fd);
			toupd0=0;
			toupd1=0;
			for(u8 b=0;b<157;b++)
			{
				if(d0[b]!=d0t[b]) toupd0=1;
				if(d1[b]!=d1t[b]) toupd1=1;
				d0t[b]=d0[b];
				d1t[b]=d1[b];
			}
			if(toupd0)
			{
				u0++;
				sprintf(un, "/dev_hdd0/tmp/d0-%03i.bin", u0);
				cellFsOpen(un, CELL_FS_O_CREAT|CELL_FS_O_WRONLY, &fd, NULL, 0);
				cellFsWrite(fd, (void *)d0, 157, &read_e);
				cellFsClose(fd);
			}
			if(toupd1)
			{
				u1++;
				sprintf(un, "/dev_hdd0/tmp/d1-%03i.bin", u1);
				cellFsOpen(un, CELL_FS_O_CREAT|CELL_FS_O_WRONLY, &fd, NULL, 0);
				cellFsWrite(fd, (void *)d1, 157, &read_e);
				cellFsClose(fd);
			}
		}
	}
	*/

	u8 to=0;
	u8 sec=0;
	u32 t1=0, t2=0;
	u8 lasttemp=0;
	u8 stall=0;
	const u8 step=3;
	const u8 step_up=5;
	//u8 step_down=2;
	u8 smoothstep=0;
	int delta=0;
	char msg[200];

	old_fan=0;
	while(working)
	{
		if(fan_ps2_mode) /* skip dynamic fan control */; else

		// dynamic fan control
		if(max_temp)
		{
			t1=0;
			get_temperature(0, &t1); // 3E030000 -> 3E.03°C -> 62.(03/256)°C
			t2=t1;
			sys_timer_usleep(300000);

			get_temperature(1, &t2); // 3E030000 -> 3E.03°C -> 62.(03/256)°C
			sys_timer_usleep(200000);

			if(!max_temp || fan_ps2_mode) continue; // if fan mode was changed to manual by another thread while doing usleep

			t1=t1>>24;
			t2=t2>>24;

			if(t2>t1) t1=t2;

			if(!lasttemp) lasttemp=t1;

			delta=(lasttemp-t1);

			lasttemp=t1;

			if(t1>=max_temp || t1>84)
			{
				if(delta< 0) fan_speed+=2;
				if(delta==0 && t1!=(max_temp-1)) fan_speed++;
				if(delta==0 && t1>=(max_temp+1)) fan_speed+=(2+(t1-max_temp));
				if(delta> 0)
				{
					smoothstep++;
					if(smoothstep>1)
					{
						fan_speed--;
						smoothstep=0;
					}
				}
				if(t1>84)	 fan_speed+=step_up;
				if(delta< 0 && (t1-max_temp)>=2) fan_speed+=step_up;
			}
			else
			{
				if(delta< 0 && t1>=(max_temp-1)) fan_speed+=2;
				if(delta==0 && t1<=(max_temp-2))
				{
					smoothstep++;
					if(smoothstep>1)
					{
						fan_speed--;
						if(t1<=(max_temp-3)) {fan_speed--; if(fan_speed>0xA8) fan_speed--;} // 66%
						if(t1<=(max_temp-5)) {fan_speed--; if(fan_speed>0x80) fan_speed--;} // 50%
						smoothstep=0;
					}
				}
				//if(delta==0 && t1>=(max_temp-1)) fan_speed++;
				if(delta> 0)
				{
					//smoothstep++;
					//if(smoothstep)
					{
						fan_speed--;
						if(t1<=(max_temp-3)) {fan_speed--; if(fan_speed>0xA8) fan_speed--;} // 66%
						if(t1<=(max_temp-5)) {fan_speed--; if(fan_speed>0x80) fan_speed--;} // 50%
						smoothstep=0;
					}
				}
			}

			if(t1>76 && old_fan<0x43) fan_speed++; // <26%
			if(t1>=MAX_FANSPEED && fan_speed<0xB0) {old_fan=0; fan_speed=0xB0;} // <69%

			if(fan_speed<((webman_config->minfan*255)/100)) fan_speed=(webman_config->minfan*255)/100;
			if(fan_speed>MAX_FANSPEED) fan_speed=MAX_FANSPEED;

			//sprintf(debug, "OFAN: %x | CFAN: %x | TEMP: %i | STALL: %i\r\n", old_fan, fan_speed, t1, stall);	ssend(data_s, mytxt);
			//if(abs(old_fan-fan_speed)>=0x0F || stall>35 || (abs(old_fan-fan_speed) /*&& webman_config->aggr*/))
			if(old_fan!=fan_speed || stall>35)
			{
				//if(t1>76 && fan_speed<0x50) fan_speed=0x50;
				//if(t1>77 && fan_speed<0x58) fan_speed=0x58;
				if(t1>78 && fan_speed<0x50) fan_speed+=2; // <31%
				if(old_fan!=fan_speed)
				{
				old_fan=fan_speed;
				fan_control(fan_speed, 1);
				//sprintf(debug, "OFAN: %x | CFAN: %x | TEMP: %i | SPEED APPLIED!\r\n", old_fan, fan_speed, t1); ssend(data_s, mytxt);
				stall=0;
				}
			}
			else
				if( old_fan>fan_speed && (old_fan-fan_speed)>8 && t1<(max_temp-3) )
					stall++;
		}

		#include "combos.h"

		// Overheat control (over 83°C)
		to++;
		if(to==20)
		{
			get_temperature(0, &t1);
			get_temperature(1, &t2);
			t1>>=24; t2>>=24;
			if(t1>(MAX_TEMPERATURE-2) || t2>(MAX_TEMPERATURE-2))
			{
				if(!webman_config->warn)
				{
					sprintf((char*) msg, "%s\r\n CPU: %i°C   RSX: %i°C", STR_OVERHEAT, t1, t2);
					show_msg((char*) msg);
					sys_timer_sleep(2);
				}
				if(t1>MAX_TEMPERATURE || t2>MAX_TEMPERATURE)
				{
					if(!max_temp) max_temp=(MAX_TEMPERATURE-3);
					if(fan_speed<0xB0) fan_speed=0xB0; // 69%
					else
						if(fan_speed<MAX_FANSPEED) fan_speed+=8;

					old_fan=fan_speed;
					fan_control(fan_speed, 0);
					show_msg((char*)STR_OVERHEAT2);
				}
			}
		}
		if(to>40) to=0;

		// detect aprox. time when a game is launched
		if((sec % 10)==0) {if(View_Find("game_plugin")==0) gTick=rTick; else if(gTick.tick==rTick.tick) cellRtcGetCurrentTick(&gTick);}

		// USB Polling
		if(poll==0 && sec>=120) // check USB drives each 120 seconds
		{
			uint8_t tmp[2048];
			uint32_t usb_handle = -1, r;

			for(u8 f0=0; f0<8; f0++)
			{
				if(sys_storage_open(((f0<6)?USB_MASS_STORAGE_1(f0):USB_MASS_STORAGE_2(f0)), 0, &usb_handle, 0)==0)
				{
					sys_storage_read(usb_handle, 0, to, 1, tmp, &r, 0);
					sys_storage_close(usb_handle);
					//sprintf(tmp, "/dev_usb00%i: Read %i sectors @ %i offset", f0, r, to); show_msg((char*)tmp);
				}
			}
			sec=0;
		}
		sec+=step;

#ifdef WM_REQUEST
		// Poll requests via local file
		if((sec & 1) && (gTick.tick>rTick.tick)) continue; // slowdown polling if ingame
		if(FileExists("/dev_hdd0/tmp/wm_request"))
		{
			loading_html++;
			sys_ppu_thread_t id;
			if(working) sys_ppu_thread_create(&id, handleclient, WM_FILE_REQUEST, -0x1d8, 0x20000, 0, "wwwd");
		}
#endif
	}

	sys_ppu_thread_exit(0);
}

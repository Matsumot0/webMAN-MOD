#define SC_FS_DISK_FREE		840

static void add_list_entry(char *tempstr, bool is_dir, char *ename, char *templn, char *name, char *fsize, CellRtcDateTime rDate, u16 flen, unsigned long long sz, char *sf, u8 is_net)
{
	unsigned long long sbytes = sz;

	if(sz<10240) sprintf(sf, "%s", STR_BYTE);
	else if(sz<0x200000ULL) {sprintf(sf, "%s", STR_KILOBYTE); sz>>=10;}
	else if(sz<0xC00000000ULL) {sprintf(sf, "%s", STR_MEGABYTE); sz>>=20;}
	else {sprintf(sf, "%s", STR_GIGABYTE); sz>>=30;}

	urlenc(tempstr, templn, 1); strncpy(templn, tempstr, MAX_LINE_LEN);

	{strcpy(tempstr, name); htmlenc(name, tempstr, 0);}

	flen=strlen(name);

	if(is_dir)
	{
		bool show_play = ((flen == 8) && !strcmp(name, "dev_bdvd") && View_Find("game_plugin")==0);

		if(name[0]=='.')
			sprintf(fsize, "<a href=\"%s\">%s</a>", templn, HTML_DIR);
		else if(flen == 9 && !strcmp(name, "dev_blind"))
			sprintf(fsize, "<a href=\"%s?0\">%s</a>", templn, HTML_DIR);
		else if(show_play && (isDir("/dev_bdvd/PS3_GAME") || file_exists("/dev_bdvd/SYSTEM.CNF")))
			sprintf(fsize, "<a href=\"play.ps3\">&lt;%s></a>", "Play");
		else if(show_play && isDir("/dev_bdvd/BDMV"))
			sprintf(fsize, "<a href=\"play.ps3\">&lt;%s></a>", "BDV");
		else if(show_play && isDir("/dev_bdvd/VIDEO_TS"))
			sprintf(fsize, "<a href=\"play.ps3\">&lt;%s></a>", "DVD");
		else if(show_play && isDir("/dev_bdvd/AVCHD"))
			sprintf(fsize, "<a href=\"play.ps3\">&lt;%s></a>", "AVCHD");
#ifdef FIX_GAME
		else if(flen == 9 && strstr(templn, HDD0_GAME_DIR))
			sprintf(fsize, "<a href=\"/fixgame.ps3%s\">%s</a>", templn, HTML_DIR);
#endif
#ifdef COPY_PS3
		else if(!is_net && ( (flen = 5 && (strcasestr(name, "VIDEO") || strcasestr(name, "music"))) || (flen = 6 && strcasestr(name, "covers")) || strstr(name, "savedata") || strstr(name, "exdata") || strstr(name, "home") ))
			sprintf(fsize, "<a href=\"/copy.ps3%s\" title=\"copy to %s\">%s</a>", templn, strstr(templn, "/dev_hdd0") ? "/dev_usb000" : "/dev_hdd0", HTML_DIR);
#endif
		else
		if(strlen(templn)<=11 && strstr(templn, "/dev_"))
		{
			uint64_t freeSize, devSize;
			system_call_3(SC_FS_DISK_FREE, (uint64_t)templn, (uint64_t)&devSize,  (uint64_t)&freeSize);

			sprintf(fsize, "<a href=\"/mount.ps3%s\" title=\"%'llu %s (%'llu %s)\">%'8llu %s</a>", templn, (unsigned long long)((devSize)>>20), STR_MEGABYTE, devSize, STR_BYTE, (unsigned long long)((freeSize)>>20), STR_MEGABYTE);
		}
		else
#ifdef PS2_DISC
			sprintf(fsize, "<a href=\"/mount%s%s\">%s</a>", strstr(name, "[PS2")?".ps2":".ps3", templn, HTML_DIR);
#else
			sprintf(fsize, "<a href=\"/mount.ps3%s\">%s</a>", templn, HTML_DIR);
#endif
	}


#ifdef COBRA_ONLY
	else if( (flen > 4 && name[flen-4]=='.' && strcasestr(ISO_EXTENSIONS, name+flen-4)) || (!is_net && ( strstr(name, ".ntfs[") || !extcmp(name, ".BIN.ENC", 8) )) )
	{
		if( strcasestr(name, ".iso.") && extcasecmp(name, ".iso.0", 6) )
			sprintf(fsize, "<label title=\"%'llu %s\"> %'llu %s</label>", sbytes, STR_BYTE, sz, sf);
		else
			sprintf(fsize, "<a href=\"/mount.ps3%s\" title=\"%'llu %s\">%'llu %s</a>", templn, sbytes, STR_BYTE, sz, sf);
	}
#endif


#ifdef COPY_PS3
	else if(!is_net && (
			   !extcmp(name, ".pkg", 4) || !extcmp(name, ".edat", 5) || !extcmp(name, ".p3t", 4)
			|| !extcmp(name, ".rco", 4) || !extcmp(name, ".qrc", 4)  || !memcmp(name, "coldboot", 8)
			|| !memcmp(name, "webftp_server", 13) || !memcmp(name, "boot_plugins_", 13)
			|| strcasestr(name, ".jpg") || strcasestr(name, ".png") || strstr(name, ".bmp")
			|| strcasestr(name, ".mp4") || strcasestr(name, ".mkv") || strcasestr(name, ".avi")
			|| strcasestr(name, ".mp3")
 #ifdef SWAP_KERNEL
			|| !memcmp(name, "lv2_kernel", 10)
 #endif
			))
    		sprintf(fsize, "<a href=\"/copy.ps3%s\" title=\"%'llu %s copy to %s\">%'llu %s</a>", templn, sbytes, STR_BYTE, strstr(templn, "/dev_hdd0") ? "/dev_usb000" : "/dev_hdd0", sz, sf);
#endif //#ifdef COPY_PS3


#ifdef LOAD_PRX
	else if(!is_net && ( !extcmp(name, ".sprx", 5)))
		sprintf(fsize, "<a href=\"/loadprx.ps3?slot=6&prx=%s\">%'llu %s</a>", templn, sz, sf);
#endif
	else
		sprintf(fsize, "<label title=\"%'llu %s\"> %'llu %s</label>", sbytes, STR_BYTE, sz, sf);

	snprintf(ename, 6, "%s    ", name); if(!strstr(templn, ":")) urlenc(templn, tempstr, 1);

	sprintf(tempstr, "%c%c%c%c%c%c<tr>"
                     "<td><a %shref=\"%s\">%s</a></td>",
	is_dir ? '0' : '1', ename[0], ename[1], ename[2], ename[3], ename[4],
	is_dir ? "class=\"d\" " : "class=\"w\" ", templn, name);

	flen=strlen(tempstr);
	if(flen>=LINELEN)
	{
		if(is_dir) sprintf(fsize, HTML_DIR); else sprintf(fsize, "%llu %s", sz, sf);

		sprintf(tempstr, "%c%c%c%c%c%c<tr>"
                         "<td><a %shref=\"%s\">%s</a></td>",
		is_dir ? '0' : '1', ename[0], ename[1], ename[2], ename[3], ename[4],
		is_dir ? "class=\"d\" " : "class=\"w\" ", templn, name);

		flen=strlen(tempstr);
		if(flen>=LINELEN)
		{
			if(is_dir) sprintf(fsize, HTML_DIR); else sprintf(fsize, "%llu %s", sz, sf);

			sprintf(tempstr, "%c%c%c%c%c%c<tr>"
                             "<td>%s</td>",
			is_dir ? '0' : '1', ename[0], ename[1], ename[2], ename[3], ename[4],
			name);
		}
	}

	sprintf(templn, "<td> %s &nbsp; </td>"
					"<td>%02i-%s-%04i %02i:%02i</td></tr>",
					fsize,
					rDate.day, smonth[rDate.month-1], rDate.year, rDate.hour, rDate.minute);
	strcat(tempstr, templn);

	flen=strlen(tempstr);
	if(flen>=LINELEN) {flen=0; tempstr[0]=0;} //ignore file if it is still too long
}

static bool folder_listing(char *buffer, char *templn, char *param, int conn_s, char *tempstr, char *header)
{
	struct CellFsStat buf;
	int fd;

	CellRtcDateTime rDate;

	if(strstr(param, "/dev_blind?"))
	{
		if(strstr(param, "?1")) enable_dev_blind(NULL);
		if(strstr(param, "?0")) {system_call_3(SC_FS_UMOUNT, (u64)(char*)"/dev_blind", 0, 1);}

		sprintf(templn, "/dev_blind: %s", isDir("/dev_blind")?STR_ENABLED:STR_DISABLED); strcat(buffer, templn); return true; //goto send_response;
	}

	absPath(templn, param, "/"); // auto mount /dev_blind

	u8 is_net = (param[1]=='n');

	if(copy_aborted) strcat(buffer, STR_CPYABORT);    //  /copy.ps3$abort
	else
	if(fix_aborted)  strcat(buffer, "Fix aborted!");  //  /fixgame.ps3$abort

	if(copy_aborted | fix_aborted)  strcat(buffer, "<p>");

	if(is_net || cellFsOpendir(param, &fd) == CELL_FS_SUCCEEDED)
	{
		CellFsDirent entry;
		u64 read_e;
		unsigned long long sz=0, dir_size=0;
		char sf[8];
		char fsize[LINELEN];
		char ename[8];
		char swap[MAX_PATH_LEN];
		u16 idx=0, dirs=0, flen; bool is_dir;
		u32 tlen=strlen(buffer); buffer[tlen]=0;
		char *sysmem_html=buffer+_8KB_;

		typedef struct
		{
			char 	path[LINELEN];
		}
		t_line_entries;
		t_line_entries *line_entry = (t_line_entries *)sysmem_html;
		u16 max_entries=(((BUFFER_SIZE_ALL-_8KB_))/MAX_LINE_LEN)-1; tlen=0;

		strcat(buffer, "<table class=\"propfont\"><tr><td>");

		// breadcrumb trail //
		strcpy(templn, param);
		while(strchr(templn+1, '/'))
        {
			templn[strchr(templn+1, '/')-templn]=0;
			tlen+=strlen(templn)+1;

			strcpy(swap, param);
			swap[tlen]=0;

			strcat(buffer, "<a class=\"f\" href=\"");
			urlenc(fsize, swap, 0);
			strcat(buffer, fsize);

			htmlenc(fsize, templn, 1);
			sprintf(swap, "\">%s</a>/", templn);
			strcat(buffer, swap);

			strcpy(templn, param+tlen);
		}

		if(!param[1]) sprintf(swap, "/"); else {urlenc(line_entry[0].path, param, 0); htmlenc(line_entry[1].path, templn, 0); sprintf(swap, "<a href=\"/mount.ps3%s\">%s</a>", line_entry[0].path, line_entry[1].path);} strcat(buffer, swap);
		////

		if((param[7]=='v' || param[7]=='m') && View_Find("game_plugin")==0 && (isDir("/dev_bdvd/PS3_GAME") || file_exists("/dev_bdvd/SYSTEM.CNF") || isDir("/dev_bdvd/BDMV") || isDir("/dev_bdvd/VIDEO_TS") || isDir("/dev_bdvd/AVCHD")))
			strcat(buffer, ":</td><td width=90><a href=\"/play.ps3\">&lt;Play>&nbsp;</a>");
		else
			strcat(buffer, ":</td><td width=90>&nbsp;");

		strcat(buffer, "</td><td></td></tr>");

		tlen=0;

 #ifdef COBRA_ONLY
  #ifndef LITE_EDITION
		if(is_net)
		{
			int ns=FAILED;
			int abort_connection=0;
			if(param[4]=='0') ns=connect_to_server(webman_config->neth0, webman_config->netp0); else
			if(param[4]=='1') ns=connect_to_server(webman_config->neth1, webman_config->netp1); else
			if(param[4]=='2') ns=connect_to_server(webman_config->neth2, webman_config->netp2);
			if(ns>=0)
			{
				strcat(param, "/");
				if(open_remote_dir(ns, param+5, &abort_connection)>=0)
				{
					strcpy(templn, param); if(templn[strlen(templn)-1]=='/') templn[strlen(templn)-1]=0;
					if(strrchr(templn, '/')) templn[strrchr(templn, '/')-templn]=0; if(strlen(templn)<6 && strlen(param)<8) {templn[0]='/'; templn[1]=0;}

					urlenc(swap, templn, 0);
					sprintf(tempstr, "!00000<tr><td><a class=\"f\" href=\"%s\">..</a></td><td> <a href=\"%s\">%s</a> &nbsp; </td><td>11-Nov-2006 11:11</td></tr>", swap, swap, HTML_DIR);

					if(strlen(tempstr)>MAX_LINE_LEN) return false; //ignore lines too long
					strncpy(line_entry[idx].path, tempstr, LINELEN); idx++; dirs++;
					tlen+=strlen(tempstr);

					sys_addr_t data2=0;
					netiso_read_dir_result_data *data=NULL;
					int v3_entries=0;
					v3_entries = read_remote_dir(ns, &data2, &abort_connection);
					if(data2!=NULL)
					{
						data=(netiso_read_dir_result_data*)data2;

						for(int n=0;n<v3_entries;n++)
						{
							if(data[n].name[0]=='.' && data[n].name[1]==0) continue;
							if(tlen>(BUFFER_SIZE-1024)) break;
							if(idx>=(max_entries-3)) break;

							if(param[1]==0)
								sprintf(templn, "/%s", data[n].name);
							else
							{
								sprintf(templn, "%s%s", param, data[n].name);
							}
							flen=strlen(templn)-1; if(templn[flen]=='/') templn[flen]=0;

							cellRtcSetTime_t(&rDate, data[n].mtime);

							sz=(unsigned long long)data[n].file_size; dir_size+=sz;

							is_dir=data[n].is_directory; if(is_dir) dirs++;

							add_list_entry(tempstr, is_dir, ename, templn, data[n].name, fsize, rDate, flen, sz, sf, true);

							if(strlen(tempstr)>MAX_LINE_LEN) continue; //ignore lines too long
							strncpy(line_entry[idx].path, tempstr, LINELEN); idx++;
							tlen+=strlen(tempstr);

							if(!working) break;
						}
						sys_memory_free(data2);
					}
				}
				else //may be a file
				{
					flen=strlen(param)-1; if(param[flen]=='/') param[flen]=0;

					int is_directory=0;
					int64_t file_size;
					u64 mtime, ctime, atime;
					if(remote_stat(ns, param+5, &is_directory, &file_size, &mtime, &ctime, &atime, &abort_connection)==0)
					{
						if(file_size && !is_directory)
						{
							if(open_remote_file_2(ns, param+5, &abort_connection)>0)
							{
								prepare_header(header, param, 1);
								sprintf(templn, "Content-Length: %llu\r\n\r\n", (unsigned long long)file_size); strcat(header, templn);

								ssend(conn_s, header);
								int bytes_read, boff=0;
								while(boff<file_size)
								{
									bytes_read = read_remote_file(ns, (char*)tempstr, boff, 4000, &abort_connection);
									if(bytes_read)
									{
										if(send(conn_s, tempstr, bytes_read, 0)<0) break;
									}
									boff+=bytes_read;
									if(bytes_read<4000 || boff>=file_size) break;
								}
								open_remote_file_2(ns, (char*)"/CLOSEFILE", &abort_connection);
								shutdown(ns, SHUT_RDWR); socketclose(ns);
								sclose(&conn_s);
								return false;
							}
						}
					}
				}
				shutdown(ns, SHUT_RDWR); socketclose(ns);
			}
		}
		else
  #endif
 #endif
		{
			while(cellFsReaddir(fd, &entry, &read_e) == 0 && read_e > 0)
			{
				if(entry.d_name[0]=='.' && entry.d_name[1]==0) continue;
				if(tlen>(BUFFER_SIZE-1024)) break;
				if(idx>=(max_entries-3)) break;

				if(param[1]==0)
					sprintf(templn, "/%s", entry.d_name);
				else
				{
					sprintf(templn, "%s/%s", param, entry.d_name);
				}
				flen = strlen(templn)-1; if(templn[flen]=='/') templn[flen]=0;

				cellFsStat(templn, &buf);
				cellRtcSetTime_t(&rDate, buf.st_mtime);

				sz=(unsigned long long)buf.st_size; dir_size+=sz;

				is_dir=(buf.st_mode & S_IFDIR); if(is_dir) dirs++;

				add_list_entry(tempstr, is_dir, ename, templn, entry.d_name, fsize, rDate, flen, sz, sf, false);

				if(strlen(tempstr)>MAX_LINE_LEN) continue; //ignore lines too long
				strncpy(line_entry[idx].path, tempstr, LINELEN); idx++;
				tlen+=flen;

				if(!working) break;
			}
			cellFsClosedir(fd);
		}

		if(strlen(param)<4)
		{
			for(u8 n=0; n<3; n++)
			if( (n==0 && (webman_config->netd0 && webman_config->neth0[0] && webman_config->netp0)) ||
				(n==1 && (webman_config->netd1 && webman_config->neth1[0] && webman_config->netp1)) ||
				(n==2 && (webman_config->netd2 && webman_config->neth2[0] && webman_config->netp2)) )
			{
				sprintf(tempstr, "0net%i <tr>"
										"<td><a class=\"d\" href=\"/net%i\">net%i (%s:%i)</a></td>"
										"<td> <a href=\"/mount.ps3/net%i\">%s</a> &nbsp; </td><td>11-Nov-2006 11:11</td>"
										"</tr>", n, n, n, n==1 ? webman_config->neth1 : n==2 ? webman_config->neth2 : webman_config->neth0,
														  n==1 ? webman_config->netp1 : n==2 ? webman_config->netp2 : webman_config->netp0, n, HTML_DIR);
				strncpy(line_entry[idx].path, tempstr, LINELEN); idx++;
				tlen+=strlen(tempstr);
			}
		}

		//sclose(&data_s);
		if(idx)
		{   // sort html file entries
			u16 n, m;
			for(n=0; n<(idx-1); n++)
				for(m=(n+1); m<idx; m++)
					if(strcasecmp(line_entry[n].path, line_entry[m].path)>0)
					{
						strcpy(swap, line_entry[n].path);
						strcpy(line_entry[n].path, line_entry[m].path);
						strcpy(line_entry[m].path, swap);
					}
		}

		for(u16 m=0;m<idx;m++)
		{
			strcat(buffer, (line_entry[m].path)+6);
			if(strlen(buffer)>(BUFFER_SIZE-1024)) break;
		}

		//if(sysmem_html) sys_memory_free(sysmem_html);
		strcat(buffer, "</table>");

		if(strlen(param)>4)
		{
			///////////
			bool show_icon = false;

			if(is_net && strstr(param, "/GAMES/"))
			{
				char *p = strchr(param + 12, '/'); if(p) p[0]=0; sprintf(templn, "%s/PS3_GAME/ICON0.PNG", param); show_icon = true;
			}

			if(!show_icon)
			{
				sprintf(templn, "%s/ICON0.PNG", param); show_icon = file_exists(templn);                    // current folder
				if(!show_icon) sprintf(templn, "%s/ICON2.PNG", param); show_icon = file_exists(templn);     // ps3_extra folder
				if(!show_icon)
				{
					char *p = strchr(param + 18, '/'); if(p) p[0]=0;
					sprintf(templn, "%s/PS3_GAME/ICON0.PNG", param); show_icon = file_exists(templn);       // dev_bdvd or jb folder
					if(!show_icon) sprintf(templn, "%s/ICON0.PNG", param); show_icon = file_exists(templn); // game dir
				}
			}

			unsigned int real_disctype, effective_disctype = 1, iso_disctype;
			if(!show_icon && strstr(param, "/dev_bdvd"))
			{
#if COBRA_ONLY
				cobra_get_disc_type(&real_disctype, &effective_disctype, &iso_disctype);
				if(iso_disctype==DISC_TYPE_PSX_CD) sprintf(templn, "%s", wm_icons[6]); else
				if(iso_disctype==DISC_TYPE_PS2_DVD || iso_disctype==DISC_TYPE_PS2_CD) sprintf(templn, "%s", wm_icons[7]); else
				if(iso_disctype==DISC_TYPE_DVD) sprintf(templn, "%s", wm_icons[9]); else
#endif
				sprintf(templn, "%s", wm_icons[5]); show_icon = true;
			}

			if(show_icon)
				{for(u16 m=idx;m<7;m++) {strcat(buffer, "<BR>");} urlenc(swap, templn, 0); sprintf(templn, "<img src=\"%s\" style=\"position:absolute;top:118px;right:10px;z-index:-1\" onerror=\"this.style.display='none';\">", swap); strcat(buffer, templn);}
			///////////

			memset(tempstr, 0, MAX_PATH_LEN);
			if(effective_disctype != DISC_TYPE_NONE && !strcmp(param, "/dev_bdvd") && file_exists(WMTMP "/last_game.txt"))
			{
				int fd=0;

				if(cellFsOpen(WMTMP "/last_game.txt", CELL_FS_O_RDONLY, &fd, NULL, 0) == CELL_FS_SUCCEEDED)
				{
					cellFsRead(fd, (void *)tempstr, MAX_PATH_LEN, NULL);
					cellFsClose(fd);

					if(strlen(tempstr)>10) {urlenc(swap, tempstr, 0); htmlenc(templn, tempstr, 0); sprintf(tempstr, "<span style=\"position:absolute;right:8px\"><font size=2><a href=\"%s\">%s</a></span>", swap, templn);}
				}
			}

			///////////
			uint32_t blockSize;
			uint64_t freeSize;
			if(strchr(param+1, '/'))
				param[strchr(param+1, '/')-param]=0;

			if(param[1]=='n')
				sprintf(templn, "<hr>"
								"<b><a href=\"%s\">%s</a>:", param, param);
			else
			{
				cellFsGetFreeSize(param, &blockSize, &freeSize);
				sprintf(templn, "<hr>"
								"<b><a href=\"%s\">%s</a>: %'d %s",
								param, param, (int)((blockSize*freeSize)>>20), STR_MBFREE);
			}
			strcat(buffer, templn);

			sprintf(templn, "</b> &nbsp; <font color=\"#707070\">%'i Dir(s) %'d %s %'d %s</font>%s",
							(dirs-1), (idx-dirs), STR_FILES,
							dir_size<(_1MB_)?(int)(dir_size>>10):(int)(dir_size>>20),
							dir_size<(_1MB_)?STR_KILOBYTE:STR_MEGABYTE, tempstr);
			strcat(buffer, templn);
			///////////
		}
		else
			strcat(buffer,  HTML_BLU_SEPARATOR
							"webMAN - Simple Web Server" EDITION "<br>");
	}
	return true;
}

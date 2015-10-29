///////////// PS3MAPI BEGIN //////////////

#define SYSCALL8_OPCODE_PS3MAPI						0x7777

#define PS3MAPI_SERVER_VERSION						0x0120
#define PS3MAPI_SERVER_MINVERSION					0x0120

#define PS3MAPI_WEBUI_VERSION						0x0121
#define PS3MAPI_WEBUI_MINVERSION					0x0120

#define PS3MAPI_CORE_MINVERSION						0x0111

#define PS3MAPI_OPCODE_GET_CORE_VERSION				0x0011
#define PS3MAPI_OPCODE_GET_CORE_MINVERSION			0x0012
#define PS3MAPI_OPCODE_GET_FW_TYPE					0x0013
#define PS3MAPI_OPCODE_GET_FW_VERSION				0x0014
#define PS3MAPI_OPCODE_GET_ALL_PROC_PID				0x0021
#define PS3MAPI_OPCODE_GET_PROC_NAME_BY_PID			0x0022
#define PS3MAPI_OPCODE_GET_PROC_BY_PID				0x0023
#define PS3MAPI_OPCODE_GET_CURRENT_PROC				0x0024
#define PS3MAPI_OPCODE_GET_CURRENT_PROC_CRIT		0x0025
#define PS3MAPI_OPCODE_GET_PROC_MEM					0x0031
#define PS3MAPI_OPCODE_SET_PROC_MEM					0x0032
#define PS3MAPI_OPCODE_GET_ALL_PROC_MODULE_PID		0x0041
#define PS3MAPI_OPCODE_GET_PROC_MODULE_NAME			0x0042
#define PS3MAPI_OPCODE_GET_PROC_MODULE_FILENAME		0x0043
#define PS3MAPI_OPCODE_LOAD_PROC_MODULE				0x0044
#define PS3MAPI_OPCODE_UNLOAD_PROC_MODULE			0x0045
#define PS3MAPI_OPCODE_UNLOAD_VSH_PLUGIN			0x0046
#define PS3MAPI_OPCODE_GET_VSH_PLUGIN_INFO			0x0047
#define PS3MAPI_OPCODE_GET_IDPS 					0x0081
#define PS3MAPI_OPCODE_SET_IDPS 					0x0082
#define PS3MAPI_OPCODE_GET_PSID 					0x0083
#define PS3MAPI_OPCODE_SET_PSID						0x0084
#define PS3MAPI_OPCODE_CHECK_SYSCALL				0x0091
#define PS3MAPI_OPCODE_DISABLE_SYSCALL				0x0092
#define PS3MAPI_OPCODE_PDISABLE_SYSCALL8 			0x0093
#define PS3MAPI_OPCODE_PCHECK_SYSCALL8 				0x0094
#define PS3MAPI_OPCODE_REMOVE_HOOK					0x0101

#define PS3MAPI_OPCODE_SUPPORT_SC8_PEEK_POKE		0x1000
#define PS3MAPI_OPCODE_LV2_PEEK						0x1006
#define PS3MAPI_OPCODE_LV2_POKE						0x1007
#define PS3MAPI_OPCODE_LV1_PEEK						0x1008
#define PS3MAPI_OPCODE_LV1_POKE						0x1009

#define PS3MAPI_OPCODE_SET_ACCESS_KEY				0x2000
#define PS3MAPI_OPCODE_REQUEST_ACCESS				0x2001

///////////// PS3MAPI END //////////////

#ifdef PS3MAPI

static int Char2Int(char input);
static void Hex2Bin(const char* src, char* target);

static int Char2Int(char input)
{
  if(input >= '0' && input <= '9')
    return input - '0';
  if(input >= 'A' && input <= 'F')
    return input - 'A' + 10;
  if(input >= 'a' && input <= 'f')
    return input - 'a' + 10;
  return NULL;
}

static void Hex2Bin(const char* src, char* target)
{
  while(*src && src[1])
  {
    *(target++) = Char2Int(*src)*16 + Char2Int(src[1]);
    src += 2;
  }
}

static void ps3mapi_buzzer(char *buffer, char *templn, char *param);
static void ps3mapi_led(char *buffer, char *templn, char *param);
static void ps3mapi_notify(char *buffer, char *templn, char *param);
static void ps3mapi_syscall(char *buffer, char *templn, char *param);
static void ps3mapi_syscall8(char *buffer, char *templn, char *param);
static void ps3mapi_setidps(char *buffer, char *templn, char *param);
static void ps3mapi_getmem(char *buffer, char *templn, char *param);
static void ps3mapi_setmem(char *buffer, char *templn, char *param);
static void ps3mapi_vshplugin(char *buffer, char *templn, char *param);
static void ps3mapi_gameplugin(char *buffer, char *templn, char *param);

static void ps3mapi_home(char *buffer, char *templn)
{
	int syscall8_state = -1;
	{system_call_2(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_PCHECK_SYSCALL8); syscall8_state = (int)p1;}
	int version = 0;
	if(syscall8_state>=0) {system_call_2(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_CORE_VERSION); version = (int)(p1);}
	int versionfw = 0;
	if(syscall8_state>=0) {system_call_2(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_FW_VERSION); versionfw = (int)(p1);}
	char fwtype[32]; memset(fwtype, 0, 32);
	if(syscall8_state>=0) {system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_FW_TYPE, (u64)(u32)fwtype);}

	//---------------------------------------------
	//PS3 Commands---------------------------------
	//---------------------------------------------
	sprintf(templn, "<b>%s</b>"
					"<hr color=\"#0099FF\"/>"
					"<table width=\"800\" border=\"0\" cellspacing=\"2\" cellpadding=\"0\"><tr>", "PS3 Commands");
	strcat(buffer, templn);

	//RingBuzzer
	ps3mapi_buzzer(buffer, templn, (char*)" ");

	//LedRed
	ps3mapi_led(buffer, templn, (char*)" ");

	sprintf(templn, "%s", "</td>");
	strcat(buffer, templn);

	//Notify
    ps3mapi_notify(buffer, templn, (char*)" ");

	if (syscall8_state >= 0 && syscall8_state < 3)
	{
        ps3mapi_syscall(buffer, templn, (char*)" ");
	}
	if (syscall8_state >= 0)
	{
		//Syscall8
		ps3mapi_syscall8(buffer, templn, (char*)" ");
	}
	if (syscall8_state >= 0 && syscall8_state < 3)
	{
		//IDPS/PSID
		if(version >= 0x0120)
		{
            ps3mapi_setidps(buffer, templn, (char*)" ");
		}
		else
		{
			sprintf(templn, "%s", "</table><br>");
			strcat(buffer, templn);
		}
		//---------------------------------------------
		//Process Commands-----------------------------
		//---------------------------------------------
		//GetMem
		ps3mapi_getmem(buffer, templn, (char*)" ");

		//SetMem
		ps3mapi_setmem(buffer, templn, (char*)" ");

		//---------------------------------------------
		//VSH Plugin-----------------------------------
		//---------------------------------------------
		ps3mapi_vshplugin(buffer, templn, (char*)" ");

		//---------------------------------------------
		//Game Plugin-----------------------------------
		//---------------------------------------------
		ps3mapi_gameplugin(buffer, templn, (char*)" ");

		sprintf(templn, "<hr color=\"#FF0000\"/>"
						"Firmware: %X %s | PS3MAPI: webUI v%X, Server v%X, Core v%X | By NzV, modified by OsirisX", versionfw, fwtype, PS3MAPI_WEBUI_VERSION, PS3MAPI_SERVER_VERSION, version);
		strcat(buffer, templn);
	}
	else
	{
		sprintf(templn, "</table><br><hr color=\"#FF0000\"/>[SYSCALL8 %sDISABLED] | PS3MAPI: webUI v%X, Server v%X | By NzV", (syscall8_state==3)?"PARTIALY ":"", PS3MAPI_WEBUI_VERSION, PS3MAPI_SERVER_VERSION);
		strcat(buffer, templn);
	}
}

static void ps3mapi_buzzer(char *buffer, char *templn, char *param)
{
	bool is_ps3mapi_home = (param[0] == ' ');

	if(strstr(param, "buzzer.ps3mapi?"))
	{
		if(strstr(param, "mode=1")) { BEEP1 } else
		if(strstr(param, "mode=2")) { BEEP2 } else
		if(strstr(param, "mode=3")) { BEEP3 }
	}

	if(!is_ps3mapi_home)
		sprintf(templn, "<b>%s --> %s --> %s</b>"
						"<hr color=\"#0099FF\"/>", "PS3MAPI", "PS3 Commands", "Buzzer");
    else
		sprintf(templn, "<td width=\"260\" style=\"text-align:left; float:left;\"><u>%s:</u><br>", "Buzzer");
	strcat(buffer, templn);

	sprintf(templn, "<form id=\"buzzer\" action=\"/buzzer.ps3mapi\" method=\"get\" enctype=\"application/x-www-form-urlencoded\" target=\"_self\"><br>"
					"<b>%s:</b>  <select name=\"mode\">", "Mode");
	strcat(buffer, templn);
	add_option_item("1" , "Simple", strstr(param, "mode=1"), buffer);
	add_option_item("2" , "Double", strstr(param, "mode=2"), buffer);
	add_option_item("3" , "Triple", strstr(param, "mode=3"), buffer);
	sprintf(templn, "</select>   <input type=\"submit\" value=\" %s \"/></td></form><br>", "Ring");
	if(!is_ps3mapi_home) strcat(templn, "<hr color=\"#FF0000\"/>"); else strcat(templn, "</td>");
	strcat(buffer, templn);
}

static void ps3mapi_led(char *buffer, char *templn, char *param)
{
	bool is_ps3mapi_home = (param[0] == ' ');

	if(strstr(param, "led.ps3mapi?"))
	{
		int color=0, mode=OFF;
		if(strstr(param, "color=0")) color = RED; else
		if(strstr(param, "color=1")) color = GREEN; else
		if(strstr(param, "color=2")) color = RED+GREEN; //YELLOW

		if(strstr(param, "mode=1")) mode = ON; else
		if(strstr(param, "mode=2")) mode = BLINK_FAST; else
		if(strstr(param, "mode=3")) mode = BLINK_SLOW;

		if(color & RED)   { system_call_2(SC_SYS_CONTROL_LED, RED  , mode); }
		if(color & GREEN) { system_call_2(SC_SYS_CONTROL_LED, GREEN, mode); }
	}

	if(!is_ps3mapi_home)
		sprintf(templn, "<b>%s --> %s --> %s</b>"
						"<hr color=\"#0099FF\"/>", "PS3MAPI", "PS3 Commands", "Led");
    else
		sprintf(templn, "<td width=\"260\" style=\"text-align:left; float:left;\"><u>%s:</u><br>", "Led");

	strcat(buffer, templn);

	sprintf(templn, "<form id=\"led\" action=\"/led.ps3mapi\" method=\"get\" enctype=\"application/x-www-form-urlencoded\" target=\"_self\"><br>"
					"<b>%s:</b>  <select name=\"color\">",
					"Color");
	strcat(buffer, templn);

	add_option_item("0" , "Red", strstr(param, "color=0"), buffer);
	add_option_item("1" , "Green", strstr(param, "color=1"), buffer);
	add_option_item("2" , "Yellow (Red+Green)", strstr(param, "color=2"), buffer);
	sprintf(templn, "</select>   <b>%s:</b>  <select name=\"mode\">", "Mode");
	strcat(buffer, templn);
	add_option_item("0" , "Off", strstr(param, "mode=0"), buffer);
	add_option_item("1" , "On", strstr(param, "mode=1"), buffer);
	add_option_item("2" , "Blink fast", strstr(param, "mode=2"), buffer);
	add_option_item("3" , "Blink slow", strstr(param, "mode=3"), buffer);
	sprintf(templn, "</select>   <input type=\"submit\" value=\" %s \"/></form><br>", "Set");
	if(!is_ps3mapi_home) strcat(templn, "<hr color=\"#FF0000\"/>"); else strcat(templn, "</table></td>");
	strcat(buffer, templn);
}

static void ps3mapi_notify(char *buffer, char *templn, char *param)
{
	bool is_ps3mapi_home = (param[0] == ' ');

	char msg[200] = "Hello :)";
	if(strstr(param, "notify.ps3mapi?"))
	{
		char *pos;
		pos=strstr(param, "msg=");
		if(pos)
		{
			get_value(msg, pos + 4, 199);
			show_msg(msg);
		}
	}

	if(!is_ps3mapi_home)
		sprintf(templn, "<b>%s --> %s --> %s</b>"
						"<hr color=\"#0099FF\"/>", "PS3MAPI", "PS3 Commands", "Notify");
	else
		sprintf(templn, "<tr><td style=\"text-align:left; float:left;\"><br><u>%s:</u><br><br>", "Notify");

	strcat(buffer, templn);

	sprintf(templn, "<form action=\"/notify.ps3mapi\" method=\"get\" enctype=\"application/x-www-form-urlencoded\" target=\"_self\">"
					"<table width=\"800\" border=\"0\" cellspacing=\"2\" cellpadding=\"0\"><tr><td style=\"text-align:left; float:left;\">"
					"<textarea name=\"msg\" cols=\"111\" rows=\"2\" maxlength=\"199\">%s</textarea></td></tr>"
					"<tr><td style=\"text-align:right; float:right;\"><br><input type=\"submit\" value=\" %s \"/></td></tr></table></form>", msg, "Send");

	if(!is_ps3mapi_home) strcat(templn, "<hr color=\"#FF0000\"/>"); else strcat(templn, "</td>");
	strcat(buffer, templn);
}

static void ps3mapi_syscall(char *buffer, char *templn, char *param)
{
	bool is_ps3mapi_home = (param[0] == ' ');

	if(strstr(param, "syscall.ps3mapi?"))
	{
		if(strstr(param, "sc9=1"))  { system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_DISABLE_SYSCALL, 9);  }
		if(strstr(param, "sc10=1")) { system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_DISABLE_SYSCALL, 10); }
		if(strstr(param, "sc11=1")) { system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_DISABLE_SYSCALL, 11); }
		if(strstr(param, "sc35=1")) { system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_DISABLE_SYSCALL, 35); }
		if(strstr(param, "sc36=1")) { system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_DISABLE_SYSCALL, 36); }
		if(strstr(param, "sc6=1"))  { system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_DISABLE_SYSCALL, 6);  }
		if(strstr(param, "sc7=1"))  { system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_DISABLE_SYSCALL, 7);  }
	}

	if(!is_ps3mapi_home)
		sprintf(templn, "<b>%s --> %s --> %s</b>"
						"<hr color=\"#0099FF\"/>"
						"<table width=\"800\" border=\"0\" cellspacing=\"2\" cellpadding=\"0\">", "PS3MAPI", "PS3 Commands", "CFW syscall");
	else
		sprintf(templn, "<table width=\"800\" border=\"0\" cellspacing=\"2\" cellpadding=\"0\">"
						"<tr><td style=\"text-align:left; float:left;\"><u>%s:</u><br><br></td></tr>", "CFW syscall");

	strcat(buffer, templn);

	sprintf(templn, "<form id=\"syscall\" action=\"/syscall.ps3mapi\" method=\"get\" enctype=\"application/x-www-form-urlencoded\" target=\"_self\">"
					"<br><tr><td width=\"260\" style=\"text-align:left; float:left;\">");
	strcat(buffer, templn);

	int ret_val = -1;

    { system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_CHECK_SYSCALL, 6); ret_val = (int)p1;}
	if( ret_val != 0 )  add_check_box("sc6", "1\" disabled=\"disabled", "[6]LV2 Peek", NULL, true, buffer);
	else {ret_val = -1; add_check_box("sc6", "1", "[6]LV2 Peek", NULL, false, buffer);}

    { system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_CHECK_SYSCALL, 7); ret_val = (int)p1;}
	if( ret_val != 0 )  add_check_box("sc7", "1\" disabled=\"disabled", "[7]LV2 Poke", NULL, true, buffer);
	else {ret_val = -1; add_check_box("sc7", "1", "[7]LV2 Poke", NULL, false, buffer);}

    strcat(buffer, "</td><td  width=\"260\"  valign=\"top\" style=\"text-align:left; float:left;\">");

    { system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_CHECK_SYSCALL, 9); ret_val = (int)p1;}
	if( ret_val != 0 )  add_check_box("sc9", "1\" disabled=\"disabled", "[9]LV1 Poke", NULL, true, buffer);
	else {ret_val = -1; add_check_box("sc9", "1", "[9]LV1 Poke", NULL, false, buffer);}

    { system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_CHECK_SYSCALL, 10); ret_val = (int)p1;}
	if( ret_val != 0 )  add_check_box("sc10", "1\" disabled=\"disabled", "[10]LV1 Call", NULL, true, buffer);
	else {ret_val = -1; add_check_box("sc10", "1", "[10]LV1 Call", NULL, false, buffer);}

    { system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_CHECK_SYSCALL, 11); ret_val = (int)p1;}
	if( ret_val != 0 )  add_check_box("sc11", "1\" disabled=\"disabled", "[11]LV1 Peek", NULL, true, buffer);
	else {ret_val = -1; add_check_box("sc11", "1", "[11]LV1 Peek", NULL, false, buffer);}

    strcat(buffer, "</td><td  width=\"260\"  valign=\"top\" style=\"text-align:left; float:left;\">");

    { system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_CHECK_SYSCALL, 35); ret_val = (int)p1;}
	if( ret_val != 0 )  add_check_box("sc35", "1\" disabled=\"disabled", "[35]Map Path", NULL, true, buffer);
	else {ret_val = -1; add_check_box("sc35", "1", "[35]Map Path", NULL, false, buffer);}

	{ system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_CHECK_SYSCALL, 36); ret_val = (int)p1;}
	if( ret_val != 0 )  add_check_box("sc36", "1\" disabled=\"disabled", "[36]Map Game", NULL, true, buffer);
	else {ret_val = -1; add_check_box("sc36", "1", "[36]Map Game", NULL, false, buffer);}

	sprintf(templn, "</td></tr><tr><td style=\"text-align:right; float:right;\"><br><input type=\"submit\" value=\" %s \"/></td></tr></form></table><br>", "Disable");
	if(!is_ps3mapi_home) strcat(templn, "<hr color=\"#FF0000\"/>");
	strcat(buffer, templn);
}

static void ps3mapi_syscall8(char *buffer, char *templn, char *param)
{
	bool is_ps3mapi_home = (param[0] == ' ');

	if(strstr(param, "syscall8.ps3mapi?"))
	{
		if(strstr(param, "mode=0")) {{ system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_PDISABLE_SYSCALL8, 0); }}
		if(strstr(param, "mode=1")) {{ system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_PDISABLE_SYSCALL8, 1); }}
		if(strstr(param, "mode=2")) {{ system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_PDISABLE_SYSCALL8, 2); }}
		if(strstr(param, "mode=3")) {{ system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_PDISABLE_SYSCALL8, 3); }}
		if(strstr(param, "mode=4")) {{ system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_DISABLE_SYSCALL, 8); }}
	}

	sprintf(templn, "<b>%s%s --> %s</b>"
					"<hr color=\"#0099FF\"/>"
					"<table width=\"800\" border=\"0\" cellspacing=\"2\" cellpadding=\"0\">"
					"<form id=\"syscall8\" action=\"/syscall8.ps3mapi\" method=\"get\" enctype=\"application/x-www-form-urlencoded\" target=\"_self\">"
					"<br><tr><td style=\"text-align:left; float:left;\">",
					is_ps3mapi_home ? "" : "PS3MAPI --> ", "PS3 Commands", "CFW syscall 8");
	strcat(buffer, templn);
	int ret_val = -1;
	{ system_call_2(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_PCHECK_SYSCALL8); ret_val = (int)p1;}
	if(ret_val < 0)
	{
		add_radio_button("mode\" disabled=\"disabled", "0", "sc8_0", "Fully enabled", NULL, false, buffer);
		add_radio_button("mode\" disabled=\"disabled", "1", "sc8_1", "Partially disabled : Keep only COBRA/MAMBA/PS3MAPI features", NULL, false, buffer);
		add_radio_button("mode\" disabled=\"disabled", "2", "sc8_2", "Partially disabled : Keep only PS3MAPI features", NULL, false, buffer);
		add_radio_button("mode\" disabled=\"disabled", "3", "sc8_3", "Fake disabled (can be re-enabled)", NULL, false, buffer);
		add_radio_button("mode\" disabled=\"disabled", "4", "sc8_4", "Fully disabled (cant be re-enabled)", NULL, true, buffer);
	}
	else
	{
		add_radio_button("mode", "0", "sc8_0", "Fully enabled", NULL, (ret_val == 0), buffer);
		add_radio_button("mode", "1", "sc8_1", "Partially disabled : Keep only COBRA/MAMBA/PS3MAPI features", NULL, (ret_val == 1), buffer);
		add_radio_button("mode", "2", "sc8_2", "Partially disabled : Keep only PS3MAPI features", NULL, (ret_val == 2), buffer);
		add_radio_button("mode", "3", "sc8_3", "Fake disabled (can be re-enabled)", NULL, (ret_val == 3), buffer);
		add_radio_button("mode", "4", "sc8_4", "Fully disabled (cant be re-enabled)", NULL, false, buffer);
	}
	sprintf(templn, "</td></tr><tr><td style=\"text-align:right; float:right;\"><br><input type=\"submit\" value=\" %s \"/></td></tr></form></table><br>", "Set");
	if(!is_ps3mapi_home) strcat(templn, "<hr color=\"#FF0000\"/>");
	strcat(buffer, templn);
}

static void ps3mapi_getmem(char *buffer, char *templn, char *param)
{
	bool is_ps3mapi_home = (param[0] == ' ');

	u32 pid = 0;
	u64 address = 0;
	int length = 0;
	if(strstr(param, "getmem.ps3mapi?"))
	{
		char *pos;
		pos=strstr(param, "addr=");
		if(pos)
		{
			char addr_tmp[40];
			get_value(addr_tmp, pos + 5, 39);
			address = convertH(addr_tmp);
		}
		else goto getmem_err_arg;
		pos=strstr(param, "len=");
		if(pos)
		{
			char len_tmp[20];
			get_value(len_tmp, pos + 4, 19);
			length = val(len_tmp);
			length = RANGE(length, 0, 2048);
		}
		else goto getmem_err_arg;
		pos=strstr(param, "proc=");
		if(pos)
		{
			char pid_tmp[20];
			get_value(pid_tmp, pos + 5, 19);
			pid = val(pid_tmp);
		}
		else goto getmem_err_arg;
	}

 getmem_err_arg:
	if(!is_ps3mapi_home)
		sprintf(templn, "<b>%s --> %s --> %s</b>"
						"<hr color=\"#0099FF\"/>",
						"PS3MAPI", "Processes Commands", "Get process memory");
	else
		sprintf(templn, "<b>%s</b><hr color=\"#0099FF\"/>", "Processes Commands");

	strcat(buffer, templn);

	sprintf(templn, "<form action=\"/getmem.ps3mapi\" method=\"get\" enctype=\"application/x-www-form-urlencoded\" target=\"_self\"><br>"
					"<b><u>%s:</u></b>  ", "Process");
	strcat(buffer, templn); memset(templn, 0, MAX_LINE_LEN);
	if(pid == 0 )
	{
		strcat(buffer, "<select name=\"proc\">");
		char pid_str[32];
		u32 pid_list[16];
		{system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_ALL_PROC_PID, (u64)(u32)pid_list); }
		for(int i = 0; i < 16; i++)
		{
			if(1 < pid_list[i])
			{
				memset(templn, 0, MAX_LINE_LEN);
				memset(pid_str, 0, sizeof(pid_str));
				{system_call_4(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_PROC_NAME_BY_PID, (u64)(u32)pid_list[i], (u64)(u32)templn); }
				sprintf(pid_str, "%i", pid_list[i]);
				if(1 < strlen(templn))add_option_item(pid_str , templn, true, buffer);
			}
		}
		strcat(buffer, "</select>   ");
	}
	else
	{
		memset(templn, 0, MAX_LINE_LEN);
		{system_call_4(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_PROC_NAME_BY_PID, (u64)pid, (u64)(u32)templn); }
		strcat(buffer, templn);
		sprintf(templn, "<input name=\"proc\" type=\"hidden\" value=\"%u\"><br><br>", pid);
		strcat(buffer, templn);
	}
	sprintf(templn, "<b><u>%s:</u></b> " HTML_INPUT("addr", "%llX", "16", "18")
					"   <b><u>%s:</u></b> <input name=\"len\" type=\"number\" value=\"%i\" min=\"1\" max=\"2048\">"
					"   <input type=\"submit\" value=\" %s \"/></form>", "Address", address, "Length", length, "Get");
	strcat(buffer, templn);

	if(pid != 0  && length != 0)
	{
		sprintf(templn, "<br><br><b><u>%s:</u></b><br><br><textarea name=\"output\" cols=\"111\" rows=\"10\" readonly=\"true\">", "Output");
	    strcat(buffer, templn);
		char buffer_tmp[length + 1];
		memset(buffer_tmp, 0, sizeof(buffer_tmp));
		int retval = -1;
		{system_call_6(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_PROC_MEM, (u64)pid, (u64)address, (u64)(u32)buffer_tmp, (u64)length); retval = (int)p1;}
		if(0 <= retval)
		{
			for(int i = 0; i < length; i++)
			{
				sprintf(templn, "%02X", (uint8_t)buffer_tmp[i]);
				strcat(buffer, templn);
			}
		}
		else {sprintf(templn, "%s: %i", "Error", retval); strcat(buffer, templn);}
		strcat(buffer, "</textarea><br>");
	}

	if(!is_ps3mapi_home) strcat(buffer, "<br><hr color=\"#FF0000\"/>"); else strcat(buffer, "<br>");
#ifdef DEBUG_MEM
	strcat(buffer, "Dump: [<a href=\"/dump.ps3?mem\">Full Memory</a>] [<a href=\"/dump.ps3?lv1\">LV1</a>] [<a href=\"/dump.ps3?lv2\">LV2</a>]");
	if(!is_ps3mapi_home) {sprintf(templn, " [<a href=\"/dump.ps3?%llx\">LV1 Dump 0x%llx</a>] [<a href=\"/peek.lv1?%llx\">LV1 Peek 0x%llx</a>]", address, address, address, address); strcat(buffer, templn);}
#endif
	strcat(buffer, "<p>");
}

static void ps3mapi_setmem(char *buffer, char *templn, char *param)
{
	bool is_ps3mapi_home = (param[0] == ' ');

	u32 pid = 0;
	u64 address = 0;
	int length = 0;
	char value[130];
	char val_tmp[260];
	if(strstr(param, "setmem.ps3mapi?"))
	{
		char *pos;
		pos=strstr(param, "addr=");
		if(pos)
		{
			char addr_tmp[40];
			get_value(addr_tmp, pos + 5, 39);
			address = convertH(addr_tmp);
		}
		else goto setmem_err_arg;
		pos=strstr(param, "val=");
		if(pos)
		{
			get_value(val_tmp, pos + 4, 259);
			length = (strlen(val_tmp) / 2);
			Hex2Bin(val_tmp, value);
		}
		else goto setmem_err_arg;
		pos=strstr(param, "proc=");
		if(pos)
		{
			char pid_tmp[20];
			get_value(pid_tmp, pos + 5, 19);
			pid = val(pid_tmp);
		}
		else goto setmem_err_arg;
	}

 setmem_err_arg:
	if(!is_ps3mapi_home)
		sprintf(templn, "<b>%s --> %s --> %s</b>"
						"<hr color=\"#0099FF\"/>",
						"PS3MAPI", "Processes Commands", "Set process memory");
	else
		sprintf(templn, "<u>%s:</u>", "Set process memory");

	strcat(buffer, templn);

	sprintf(templn, "<form action=\"/setmem.ps3mapi\" method=\"get\" enctype=\"application/x-www-form-urlencoded\" target=\"_self\">"
					"<b><u>%s:</u></b>  ", "Process");
	strcat(buffer, templn); memset(templn, 0, MAX_LINE_LEN);
	if(pid == 0 )
	{
		strcat(buffer, "<select name=\"proc\">");
		char pid_str[32];
		u32 pid_list[16];
		{system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_ALL_PROC_PID, (u64)(u32)pid_list); }
		for(int i = 0; i < 16; i++)
		{
			if(1 < pid_list[i])
			{
				memset(templn, 0, MAX_LINE_LEN);
				memset(pid_str, 0, sizeof(pid_str));
				{system_call_4(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_PROC_NAME_BY_PID, (u64)pid_list[i], (u64)(u32)templn); }
				sprintf(pid_str, "%i", pid_list[i]);
				if(1 < strlen(templn))add_option_item(pid_str , templn, true, buffer);
			}
		}
		strcat(buffer, "</select>   ");
	}
	else
	{
		memset(templn, 0, MAX_LINE_LEN);
		{system_call_4(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_PROC_NAME_BY_PID, (u64)pid, (u64)(u32)templn); }
		strcat(buffer, templn);
		sprintf(templn, "<input name=\"proc\" type=\"hidden\" value=\"%u\"><br><br>", pid);
		strcat(buffer, templn);
	}
	if(strlen(val_tmp) < 1) sprintf(val_tmp, "%02X", 0);
	sprintf(templn, "<b><u>%s:</u></b> "  HTML_INPUT("addr", "%llX", "16", "18")
					"<br><br><b><u>%s:</u></b><br><br>"
					"<table width=\"800\" border=\"0\" cellspacing=\"2\" cellpadding=\"0\">"
					"<tr><td style=\"text-align:left; float:left;\">"
					"<textarea name=\"val\" cols=\"111\" rows=\"3\" maxlength=\"199\">%s</textarea></td></tr>"
					"<tr><td style=\"text-align:right; float:right;\"><br>"
					"<input type=\"submit\" value=\" %s \"/></td></tr></table></form>", "Address", address, "Value", val_tmp, "Set");
	strcat(buffer, templn);
	if(pid != 0 && length != 0)
	{
		int retval = -1;
		{system_call_6(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_SET_PROC_MEM, (u64)pid, (u64)address, (u64)(u32)value, (u64)length); retval = (int)p1;}
		if(0 <= retval) sprintf(templn, "<br><b><u>%s!</u></b>", "Done");
		else sprintf(templn, "<br><b><u>%s: %i</u></b>", "Error", retval);
		strcat(buffer, templn);
	}

	if(!is_ps3mapi_home) strcat(buffer, "<br><hr color=\"#FF0000\"/>"); else strcat(buffer, "<br>");
}

static void ps3mapi_setidps(char *buffer, char *templn, char *param)
{
	bool is_ps3mapi_home = (param[0] == ' ');

	{system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_IDPS, (u64)IDPS);}
	{system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_PSID, (u64)PSID);}
	u64 _new_IDPS[2] = { IDPS[0], IDPS[1]};
	u64 _new_PSID[2] = { PSID[0], PSID[1]};

	if(strstr(param, "setidps.ps3mapi?"))
	{
		char *pos;
		pos=strstr(param, "idps1=");
		if(pos)
		{
			char idps1_tmp[17];
			get_value(idps1_tmp, pos + 6, 16);
			_new_IDPS[0] = convertH(idps1_tmp);
		}
		else goto setidps_err_arg1;
		pos=strstr(param, "idps2=");
		if(pos)
		{
			char idps2_tmp[17];
			get_value(idps2_tmp, pos + 6, 16);
			_new_IDPS[1] = convertH(idps2_tmp);
		}
		else goto setidps_err_arg1;
		{system_call_4(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_SET_IDPS, (u64)_new_IDPS[0], (u64)_new_IDPS[1]);}

	 setidps_err_arg1:

		pos=strstr(param, "psid1=");
		if(pos)
		{
			char psid1_tmp[17];
			get_value(psid1_tmp, pos + 6, 16);
			_new_PSID[0] = convertH(psid1_tmp);
		}
		else goto setidps_err_arg2;
		pos=strstr(param, "psid2=");
		if(pos)
		{
			char psid2_tmp[17];
			get_value(psid2_tmp, pos + 6, 16);
			_new_PSID[1] = convertH(psid2_tmp);
		}
		else goto setidps_err_arg2;
		{system_call_4(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_SET_PSID, (u64)_new_PSID[0], (u64)_new_PSID[1]);}
	}

 setidps_err_arg2:

	sprintf(templn, "<b>%s%s --> %s</b>"
					"<hr color=\"#0099FF\"/>"
					"<form action=\"/setidps.ps3mapi\" method=\"get\" enctype=\"application/x-www-form-urlencoded\" target=\"_self\"><br>"
					"<table width=\"800\" border=\"0\" cellspacing=\"2\" cellpadding=\"0\"><tr><td width=\"400\" style=\"text-align:left; float:left;\">"
	                "<br><b><u>%s:</u></b><br>" HTML_INPUT("idps1", "%016llX", "16", "18") HTML_INPUT("idps2", "%016llX", "16", "18") "</td>"
					"<td style=\"text-align:left; float:left;\">"
	                "<br><b><u>%s:</u></b><br>" HTML_INPUT("psid1", "%016llX", "16", "18") HTML_INPUT("psid2", "%016llX", "16", "18") "</td></tr>"
	                "<tr><td style=\"text-align:right; float:right;\"><br><input type=\"submit\" value=\" %s \"/></td></tr></table></form><br>",
					is_ps3mapi_home ? "" : "PS3MAPI --> ", "PS3 Commands", "Set IDPS/PSID", "IDPS", _new_IDPS[0], _new_IDPS[1], "PSID", _new_PSID[0], _new_PSID[1], "Set");
	if(!is_ps3mapi_home) strcat(templn, "<hr color=\"#FF0000\"/>");
	strcat(buffer, templn);
}

static void add_plugins_list(char *buffer, char *templn)
{
	if(!strstr(buffer, "<datalist id=\"plugins\">"))
	{
		strcat(buffer, "<div style=\"display:none\"><datalist id=\"plugins\">");
		int fd, cnt = 0; char paths[10][48] = {"/dev_hdd0", "/dev_hdd0/plugins", "/dev_hdd0/plugins/ps3xpad", "/dev_hdd0/plugins/ps3_menu", "/dev_usb000", "/dev_usb001", "/dev_hdd0/game/UPDWEBMOD/USRDIR", "/dev_hdd0/game/UPDWEBMOD/USRDIR/official", "/dev_hdd0/tmp"};

		for(u8 i = 0; i < 9; i++)
		if(cellFsOpendir(paths[i], &fd) == CELL_FS_SUCCEEDED)
		{
			CellFsDirent dir; u64 read = sizeof(CellFsDirent);

			while(!cellFsReaddir(fd, &dir, &read))
			{
				if(!read || cnt>50) break;
				if(strstr(dir.d_name, ".sprx"))
				{
					sprintf(templn, "<option>%s/%s</option>", paths[i], dir.d_name); strcat(buffer, templn); cnt++;
				}
			}
			cellFsClosedir(fd);
		}

		strcat(buffer, "</datalist></div>");
	}
}

static void ps3mapi_vshplugin(char *buffer, char *templn, char *param)
{
	bool is_ps3mapi_home = (param[0] == ' ');

	char tmp_name[30];
	char tmp_filename[256];

	if(strstr(param, "vshplugin.ps3mapi?"))
	{
		char *pos;
		unsigned int uslot = 99;

		pos=strstr(param, "?s=");
		if(pos)
		{
			pos+=3;
			switch (pos[0])
			{
				case '1': sprintf(tmp_filename, "/dev_hdd0/mamba_plugins.txt"); break;
				case '2': sprintf(tmp_filename, "/dev_hdd0/prx_plugins.txt");   break;
				case '3': sprintf(tmp_filename, "/dev_hdd0/game/PRXLOADER/USRDIR/plugins.txt"); break;
				default : sprintf(tmp_filename, "/dev_hdd0/boot_plugins.txt");
			}

			u64 written; int fdwm=0;
			if(cellFsOpen(tmp_filename, CELL_FS_O_CREAT|CELL_FS_O_WRONLY|CELL_FS_O_TRUNC, &fdwm, NULL, 0) == CELL_FS_SUCCEEDED)
			{
				sprintf(templn, "<p><a href=\"%s\" style=\"padding:8px;background:#900;border-radius:8px;\">%s</a><p>", tmp_filename, tmp_filename); strcat(buffer, templn);

				for (unsigned int slot = 1; slot < 7; slot++)
				{
					memset(tmp_name, 0, sizeof(tmp_name));
					memset(tmp_filename, 0, sizeof(tmp_filename));
					{system_call_5(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_VSH_PLUGIN_INFO, (u64)slot, (u64)(u32)tmp_name, (u64)(u32)tmp_filename); }
					if(strlen(tmp_filename) > 0)
					{
						sprintf(templn, "%s\n", tmp_filename);
						cellFsWrite(fdwm, (void *)templn, strlen(templn), &written);
					}
				}
				cellFsClose(fdwm);
			}
		}
		else
		{
			pos=strstr(param, "unload_slot=");
			if(pos)
			{
				char uslot_str[3];
				get_value(uslot_str, pos + 12, 2);
				uslot = val(uslot_str);
				if ( uslot ) {{system_call_2(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_UNLOAD_VSH_PLUGIN, (u64)uslot);}}
			}
			else
			{
				pos=strstr(param, "load_slot=");
				if(pos)
				{
					char uslot_str[3];
					get_value(uslot_str, pos + 10, 2);
					uslot = val(uslot_str);
				}
				else goto loadvshplug_err_arg;
				pos=strstr(param, "prx=");
				if(pos)
				{
					char prx_path[256];
					get_value(prx_path, pos + 4, 256);
					if ( uslot ) {{system_call_5(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_LOAD_VSH_PLUGIN, (u64)uslot, (u64)(u32)prx_path, NULL, 0);}}
				}
			}
		}
	}

loadvshplug_err_arg:

	sprintf(templn, "<b>%s%s</b>"
					"<hr color=\"#0099FF\"/><br>"
					"<table border=\"0\" cellspacing=\"2\" cellpadding=\"0\">"
					"<tr><td width=\"75\" style=\"text-align:left; float:left;\">%s</td>"
					"<td width=\"120\" style=\"text-align:left; float:left;\">%s</td>"
					"<td width=\"500\" style=\"text-align:left; float:left;\">%s</td>"
					"<td width=\"125\" style=\"text-align:right; float:right;\"> </td></tr>",
					is_ps3mapi_home ? "" : "PS3MAPI --> ", "VSH Plugins", "Slot", "Name", "File name");

	strcat(buffer, templn);
	for (unsigned int slot = 0; slot < 7; slot++)
	{
		memset(tmp_name, 0, sizeof(tmp_name));
		memset(tmp_filename, 0, sizeof(tmp_filename));
		{system_call_5(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_VSH_PLUGIN_INFO, (u64)slot, (u64)(u32)tmp_name, (u64)(u32)tmp_filename); }
		if(strlen(tmp_filename) > 0)
		{
			sprintf(templn, "<tr><td width=\"75\" style=\"text-align:left; float:left;\">%i</td>"
							"<td width=\"120\" style=\"text-align:left; float:left;\">%s</td>"
							"<td width=\"500\" style=\"text-align:left; float:left;\">%s</td>"
							"<td width=\"100\" style=\"text-align:right; float:right;\">"
							"<form action=\"/vshplugin.ps3mapi\" method=\"get\" enctype=\"application/x-www-form-urlencoded\" target=\"_self\">"
							"<input name=\"unload_slot\" type=\"hidden\" value=\"%i\"><input type=\"submit\" %s/></form></td></tr>",
							slot, tmp_name, tmp_filename, slot, (slot) ? "value=\" Unload \"" : "value=\" Reserved \" disabled=\"disabled\"" );
		}
		else
 		{
			sprintf(templn, "<tr><td width=\"75\" style=\"text-align:left; float:left;\">%i</td>"
							"<td width=\"120\" style=\"text-align:left; float:left;\">%s</td>"
							"<form action=\"/vshplugin.ps3mapi\" method=\"get\" enctype=\"application/x-www-form-urlencoded\" target=\"_self\">"
							"<td width=\"500\" style=\"text-align:left; float:left;\">"
							HTML_INPUT("prx\" style=\"width:555px\" list=\"plugins", "", "128", "75") "<input name=\"load_slot\" type=\"hidden\" value=\"%i\"></td>"
							"<td width=\"100\" style=\"text-align:right; float:right;\"><input type=\"submit\" %s/></td></form></tr>",
							slot, "NULL", slot, (slot) ? "value=\" Load \"" : "value=\" Reserved \" disabled=\"disabled\"" );
		}
			strcat(buffer, templn);
	}

	sprintf(templn, "<tr><td colspan=4><p>%s > "	HTML_BUTTON_FMT
													HTML_BUTTON_FMT
													HTML_BUTTON_FMT
													HTML_BUTTON_FMT "</td></tr>", STR_SAVE,
		HTML_BUTTON, "boot_plugins.txt" , HTML_ONCLICK, "/vshplugin.ps3mapi?s=0",
		HTML_BUTTON, "mamba_plugins.txt", HTML_ONCLICK, "/vshplugin.ps3mapi?s=1",
		HTML_BUTTON, "prx_plugins.txt"  , HTML_ONCLICK, "/vshplugin.ps3mapi?s=2",
		HTML_BUTTON, "plugins.txt"      , HTML_ONCLICK, "/vshplugin.ps3mapi?s=3"); strcat(buffer, templn);

	add_plugins_list(buffer, templn);

	sprintf(templn, "%s", "</table><br>");
	if(!is_ps3mapi_home) strcat(templn, "<hr color=\"#FF0000\"/>");
	strcat(buffer, templn);
}

static void ps3mapi_gameplugin(char *buffer, char *templn, char *param)
{
	bool is_ps3mapi_home = (param[0] == ' ');

	u32 pid = 0;

	if(strstr(param, "gameplugin.ps3mapi?"))
	{
		unsigned int uslot = 99;

		char *pos;
		pos=strstr(param, "proc=");
		if(pos)
		{
			char pid_tmp[20];
			get_value(pid_tmp, pos + 5, 19);
			pid = val(pid_tmp);
		}
		else goto loadgameplug_err_arg;
		pos=strstr(param, "unload_slot=");
		if(pos)
		{
			char uslot_str[32];
			get_value(uslot_str, pos + 12, 31);
			uslot = val(uslot_str);
			{system_call_4(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_UNLOAD_PROC_MODULE, (u64)pid, (u64)uslot); }
		}
		else
		{
			pos=strstr(param, "load_slot=");
			if(pos)
			{
				char uslot_str[3];
				get_value(uslot_str, pos + 10, 2);
				uslot = val(uslot_str);
			}
			else goto loadgameplug_err_arg;
			pos=strstr(param, "prx=");
			if(pos)
			{
				char prx_path[256];
				get_value(prx_path, pos + 4, 256);
				{system_call_6(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_LOAD_PROC_MODULE, (u64)pid, (u64)(u32)prx_path, NULL, 0); }
			}
		}
	}

loadgameplug_err_arg:

	if(!is_ps3mapi_home)
		sprintf(templn, "<b>%s --> %s</b>"
						"<hr color=\"#0099FF\"/>",
						"PS3MAPI", "Game Plugins");
	else
		sprintf(templn, "<b>%s</b><hr color=\"#0099FF\"/>", "Game Plugins");

	strcat(buffer, templn);

	sprintf(templn, "<form action=\"/gameplugin.ps3mapi\" method=\"get\" enctype=\"application/x-www-form-urlencoded\" target=\"_self\"><br>"
					"<b><u>%s:</u></b>  ", "Process");
	strcat(buffer, templn); memset(templn, 0, MAX_LINE_LEN);

	if(pid == 0)
	{
		strcat(buffer, "<select name=\"proc\">");
		char pid_str[32];
		u32 pid_list[16];
		{system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_ALL_PROC_PID, (u64)(u32)pid_list); }
		for(int i = 0; i < 16; i++)
		{
			if(1 < pid_list[i])
			{
				memset(templn, 0, MAX_LINE_LEN);
				memset(pid_str, 0, sizeof(pid_str));
				{system_call_4(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_PROC_NAME_BY_PID, (u64)(u32)pid_list[i], (u64)(u32)templn); }
				sprintf(pid_str, "%i", pid_list[i]);
				if(1 < strlen(templn))add_option_item(pid_str , templn, true, buffer);
			}
		}
		strcat(buffer, "</select>   ");
	}
	else
	{
		memset(templn, 0, MAX_LINE_LEN);
		{system_call_4(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_PROC_NAME_BY_PID, (u64)pid, (u64)(u32)templn); }
		strcat(buffer, templn);
		sprintf(templn, "<input name=\"proc\" type=\"hidden\" value=\"%u\"><br><br>", pid);
		strcat(buffer, templn);
	}

	if(is_ps3mapi_home)
		sprintf(templn, "<input type=\"submit\" value=\" Set \" /></form>");
	else
		sprintf(templn, "</form>");

	strcat(buffer, templn);

	if(pid != 0)
	{
		sprintf(templn,
					"<table border=\"0\" cellspacing=\"2\" cellpadding=\"0\">"
					"<tr><td width=\"75\" style=\"text-align:left; float:left;\">%s</td>"
					"<td width=\"300\" style=\"text-align:left; float:left;\">%s</td>"
					"<td width=\"500\" style=\"text-align:left; float:left;\">%s</td>"
					"<td width=\"125\" style=\"text-align:right; float:right;\"> </td></tr>",
					"Slot", "Name", "File name");
		strcat(buffer, templn);

		char tmp_name[30];
		char tmp_filename[256];
		u32 mod_list[64];
		memset(mod_list, 0, sizeof(mod_list));
		{system_call_4(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_ALL_PROC_MODULE_PID, (u64)pid, (u64)(u32)mod_list);}

		for(unsigned int slot = 0; slot <= 60; slot++)
		{
			memset(tmp_name, 0, sizeof(tmp_name));
			memset(tmp_filename, 0, sizeof(tmp_filename));
			if(1 < mod_list[slot])
			{
				{system_call_5(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_PROC_MODULE_NAME, (u64)pid, (u64)mod_list[slot], (u64)(u32)tmp_name);}
				{system_call_5(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_PROC_MODULE_FILENAME, (u64)pid, (u64)mod_list[slot], (u64)(u32)tmp_filename);}
			}
			if(strlen(tmp_filename) > 0)
			{
				sprintf(templn,
						"<tr>"
						 "<td width=\"75\" style=\"text-align:left; float:left;\">%i</td>"
						 "<td width=\"300\" style=\"text-align:left; float:left;\">%s</td>"
						 "<td width=\"500\" style=\"text-align:left; float:left;\">%s</td>"
						 "<td width=\"100\" style=\"text-align:right; float:right;\">"
						  "<form action=\"/gameplugin.ps3mapi\" method=\"get\" enctype=\"application/x-www-form-urlencoded\" target=\"_self\">"
						  "<input name=\"proc\" type=\"hidden\" value=\"%u\">"
						  "<input name=\"unload_slot\" type=\"hidden\" value=\"%i\">"
						  "<input type=\"submit\" value=\" Unload \">"
						  "</form>"
						 "</td>"
						"</tr>",
						slot, tmp_name, tmp_filename, pid, mod_list[slot]);
			}
			else
			{
				sprintf(tmp_name, "NULL");
				//sprintf(tmp_filename, "/dev_hdd0/tmp/my_plugin_%i.sprx", slot);
				sprintf(templn,
						"<tr>"
						  "<td width=\"75\" style=\"text-align:left; float:left;\">%i</td>"
						  "<td width=\"300\" style=\"text-align:left; float:left;\">%s</td>"
						  "<td width=\"100\" style=\"text-align:right; float:right;\">"
						    "<form action=\"/gameplugin.ps3mapi\" method=\"get\" enctype=\"application/x-www-form-urlencoded\" target=\"_self\">"
						      "<td width=\"500\" style=\"text-align:left; float:left\">"
						        "<input name=\"proc\" type=\"hidden\" value=\"%u\">"
						        HTML_INPUT("prx\" list=\"plugins", "", "128", "75")
						        "<input name=\"load_slot\" type=\"hidden\" value=\"%i\">"
						        "<input type=\"submit\" value=\" Load \">"
						      "</td>"
						    "</form>"
						  "</td>>"
						"</tr>",
						slot, tmp_name, pid, slot);
			}
			strcat(buffer, templn);
		}

		add_plugins_list(buffer, templn);
	}

	sprintf(templn, "%s", "</table><br>");
	if(!is_ps3mapi_home) strcat(templn, "<hr color=\"#FF0000\"/>");
	strcat(buffer, templn);
}


///////////// PS3MAPI BEGIN //////////////


#define THREAD_NAME_PS3MAPI						"ps3m_api_server"
#define THREAD02_NAME_PS3MAPI					"ps3m_api_client"

#define PS3MAPIPORT			(7887)

#define PS3MAPI_RECV_SIZE  2048

static u32 BUFFER_SIZE_PS3MAPI = (_64KB_);

static sys_ppu_thread_t thread_id_ps3mapi = -1;

static void handleclient_ps3mapi(u64 conn_s_ps3mapi_p)
{
	int conn_s_ps3mapi = (int)conn_s_ps3mapi_p; // main communications socket
	int data_s = -1;							// data socket
	int data_ls = -1;

	int connactive = 1;							// whether the ps3mapi connection is active or not
	int dataactive = 0;							// prevent the data connection from being closed at the end of the loop

	char buffer[PS3MAPI_RECV_SIZE];
	char cmd[20], param1[384], param2[384];

	int p1x = 0;
	int p2x = 0;

	#define PS3MAPI_OK_150    "150 OK: Binary status okay; about to open data connection.\r\n"
	#define PS3MAPI_OK_200    "200 OK: The requested action has been successfully completed.\r\n"
	#define PS3MAPI_OK_220    "220 OK: PS3 Manager API Server v1.\r\n"
	#define PS3MAPI_OK_221    "221 OK: Service closing control connection.\r\n"
	#define PS3MAPI_OK_226    "226 OK: Closing data connection. Requested binary action successful.\r\n"
	#define PS3MAPI_OK_230    "230 OK: Connected to PS3 Manager API Server.\r\n"
	#define PS3MAPI_OK_250    "250 OK: Requested binary action okay, completed.\r\n"

	#define PS3MAPI_ERROR_425 "425 Error: Can't open data connection.\r\n"
	#define PS3MAPI_ERROR_451 "451 Error: Requested action aborted. Local error in processing.\r\n"
	#define PS3MAPI_ERROR_500 "500 Error: Syntax error, command unrecognized and the requested action did not take place.\r\n"
	#define PS3MAPI_ERROR_501 "501 Error: Syntax error in parameters or arguments.\r\n"
	#define PS3MAPI_ERROR_550 "550 Error: Requested action not taken.\r\n"
	#define PS3MAPI_ERROR_502 "502 Error: Command not implemented.\r\n"

	#define PS3MAPI_CONNECT_NOTIF 	 "PS3MAPI: Client connected [%s]\r\n"
	#define PS3MAPI_DISCONNECT_NOTIF "PS3MAPI: Client disconnected [%s]\r\n"

	CellRtcTick pTick;

	sys_net_sockinfo_t conn_info;
	sys_net_get_sockinfo(conn_s_ps3mapi, &conn_info, 1);

	char ip_address[16];
	char pasv_output[56];

	ssend(conn_s_ps3mapi, PS3MAPI_OK_220);

	sprintf(ip_address, "%s", inet_ntoa(conn_info.local_adr));
	for(u8 n = 0; n<strlen(ip_address); n++) if(ip_address[n] == '.') ip_address[n] = ',';

	ssend(conn_s_ps3mapi, PS3MAPI_OK_230);

	sprintf(buffer, PS3MAPI_CONNECT_NOTIF, inet_ntoa(conn_info.remote_adr)); show_msg(buffer);

	while(connactive == 1 && working)
	{

		if(working && (recv(conn_s_ps3mapi, buffer, PS3MAPI_RECV_SIZE, 0) > 0))
		{
			buffer[strcspn(buffer, "\n")] = '\0';
			buffer[strcspn(buffer, "\r")] = '\0';

			int split = ssplit(buffer, cmd, 19, param1, 383);
			if(strcasecmp(cmd, "DISCONNECT") == 0)
			{
				ssend(conn_s_ps3mapi, PS3MAPI_OK_221);
				connactive = 0;
			}
			else if(strcasecmp(cmd, "SERVER") == 0)
			{
				if(split == 1)
				{
					split = ssplit(param1, cmd, 19, param2, 383);
					if(strcasecmp(cmd, "GETVERSION") == 0)
					{
						sprintf(buffer, "200 %i\r\n", PS3MAPI_SERVER_VERSION);
						ssend(conn_s_ps3mapi, buffer);
					}
					else if(strcasecmp(cmd, "GETMINVERSION") == 0)
					{
						sprintf(buffer, "200 %i\r\n", PS3MAPI_SERVER_MINVERSION);
						ssend(conn_s_ps3mapi, buffer);
					}
					else ssend(conn_s_ps3mapi, PS3MAPI_ERROR_502);
				}
				else
				{
					ssend(conn_s_ps3mapi, PS3MAPI_ERROR_501);
				}
			}
			else if(strcasecmp(cmd, "CORE") == 0)
			{
				if(split == 1)
				{
					split = ssplit(param1, cmd, 19, param2, 383);
					if(strcasecmp(cmd, "GETVERSION") == 0)
					{
						int version = 0;
						{ system_call_2(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_CORE_VERSION); version = (int)(p1); }
						sprintf(buffer, "200 %i\r\n", version);
						ssend(conn_s_ps3mapi, buffer);
					}
					else if(strcasecmp(cmd, "GETMINVERSION") == 0)
					{
						int version = 0;
						{ system_call_2(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_CORE_MINVERSION); version = (int)(p1); }
						sprintf(buffer, "200 %i\r\n", version);
						ssend(conn_s_ps3mapi, buffer);
					}
					else ssend(conn_s_ps3mapi, PS3MAPI_ERROR_502);
				}
				else
				{
					ssend(conn_s_ps3mapi, PS3MAPI_ERROR_501);
				}
			}
			else if(strcasecmp(cmd, "PS3") == 0)
			{
				if(split == 1)
				{
					split = ssplit(param1, cmd, 19, param2, 383);
					if(strcasecmp(cmd, "SHUTDOWN") == 0)
					{
						ssend(conn_s_ps3mapi, PS3MAPI_OK_200);
						working = 0;
						{ DELETE_TURNOFF }
						{system_call_4(SC_SYS_POWER, SYS_SHUTDOWN, 0, 0, 0); }
						sys_ppu_thread_exit(0);
					}
					else if(strcasecmp(cmd, "REBOOT") == 0)
					{
						ssend(conn_s_ps3mapi, PS3MAPI_OK_200);
						working = 0;
						{ DELETE_TURNOFF }
						{system_call_3(SC_SYS_POWER, SYS_REBOOT, NULL, 0); }
						sys_ppu_thread_exit(0);
					}
					else if(strcasecmp(cmd, "SOFTREBOOT") == 0)
					{
						ssend(conn_s_ps3mapi, PS3MAPI_OK_200);
						working = 0;
						{ DELETE_TURNOFF }
						{system_call_3(SC_SYS_POWER, SYS_SOFT_REBOOT, NULL, 0); }
						sys_ppu_thread_exit(0);
					}
					else if(strcasecmp(cmd, "HARDREBOOT") == 0)
					{
						ssend(conn_s_ps3mapi, PS3MAPI_OK_200);
						working = 0;
						{ DELETE_TURNOFF }
						{system_call_3(SC_SYS_POWER, SYS_HARD_REBOOT, NULL, 0); }
						sys_ppu_thread_exit(0);
					}
					else if(strcasecmp(cmd, "GETFWVERSION") == 0)
					{
						int version = 0;
						{system_call_2(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_FW_VERSION); version = (int)(p1); }
						sprintf(buffer, "200 %i\r\n", version);
						ssend(conn_s_ps3mapi, buffer);
					}
					else if(strcasecmp(cmd, "GETFWTYPE") == 0)
					{
						memset(param2, 0, sizeof(param2));
						{system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_FW_TYPE, (u64)(u32)param2); }
						sprintf(buffer, "200 %s\r\n", param2);
						ssend(conn_s_ps3mapi, buffer);
					}
					else if(strcasecmp(cmd, "NOTIFY") == 0)
					{
						if(split == 1)
						{
							show_msg(param2);
							ssend(conn_s_ps3mapi, PS3MAPI_OK_200);
						}
						else ssend(conn_s_ps3mapi, PS3MAPI_ERROR_501);
					}
					else if(strcasecmp(cmd, "BUZZER1") == 0)
					{
						{ BEEP1 }
						ssend(conn_s_ps3mapi, PS3MAPI_OK_200);
					}
					else if(strcasecmp(cmd, "BUZZER2") == 0)
					{
						{ BEEP2 }
						ssend(conn_s_ps3mapi, PS3MAPI_OK_200);
					}
					else if(strcasecmp(cmd, "BUZZER3") == 0)
					{
						{ BEEP3 }
						ssend(conn_s_ps3mapi, PS3MAPI_OK_200);
					}
					else if(strcasecmp(cmd, "LED") == 0)
					{
						if(split == 1)
						{
							split = ssplit(param2, param1, 383, param2, 383);
							if(split == 1)
							{
								u64 color = val(param1);
								u64 mode = val(param2);
								{system_call_2(SC_SYS_CONTROL_LED, color, mode); }
								ssend(conn_s_ps3mapi, PS3MAPI_OK_200);
							}
							else ssend(conn_s_ps3mapi, PS3MAPI_ERROR_501);
						}
						else ssend(conn_s_ps3mapi, PS3MAPI_ERROR_501);
					}
					else if(strcasecmp(cmd, "GETTEMP") == 0)
					{
						u32 cpu_temp = 0;
						u32 rsx_temp = 0;
						get_temperature(0, &cpu_temp);
						get_temperature(1, &rsx_temp);
						cpu_temp = cpu_temp >> 24;
						rsx_temp = rsx_temp >> 24;
						sprintf(buffer, "200 %i|%i\r\n", cpu_temp, rsx_temp);
						ssend(conn_s_ps3mapi, buffer);
					}
					else if(strcasecmp(cmd, "DISABLESYSCALL") == 0)
					{
						if(split == 1)
						{
							int num = val(param2);
							{ system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_DISABLE_SYSCALL, (u64)num); }
							ssend(conn_s_ps3mapi, PS3MAPI_OK_200);
						}
						else ssend(conn_s_ps3mapi, PS3MAPI_ERROR_501);
					}
					else if(strcasecmp(cmd, "CHECKSYSCALL") == 0)
					{
						if(split == 1)
						{
							int num = val(param2);
							int check = 0;
							{ system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_CHECK_SYSCALL, (u64)num); check = (int)(p1); }
							sprintf(buffer, "200 %i\r\n", check);
							ssend(conn_s_ps3mapi, buffer);
						}
						else ssend(conn_s_ps3mapi, PS3MAPI_ERROR_501);
					}
					else if(strcasecmp(cmd, "PDISABLESYSCALL8") == 0)
					{
						if(split == 1)
						{
							int mode = val(param2);
							{ system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_PDISABLE_SYSCALL8, (u64)mode); }
							ssend(conn_s_ps3mapi, PS3MAPI_OK_200);
						}
						else ssend(conn_s_ps3mapi, PS3MAPI_ERROR_501);
					}
					else if(strcasecmp(cmd, "PCHECKSYSCALL8") == 0)
					{
						int check = 0;
						{ system_call_2(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_PCHECK_SYSCALL8); check = (int)(p1); }
						sprintf(buffer, "200 %i\r\n", check);
						ssend(conn_s_ps3mapi, buffer);
					}
					else if(strcasecmp(cmd, "DELHISTORY+F") == 0)
					{
						delete_history(true);
						ssend(conn_s_ps3mapi, PS3MAPI_OK_200);
					}
					else if(strcasecmp(cmd, "DELHISTORY") == 0)
					{
						delete_history(false);
						ssend(conn_s_ps3mapi, PS3MAPI_OK_200);
					}
					else if(strcasecmp(cmd, "REMOVEHOOK") == 0)
					{
						{ system_call_2(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_REMOVE_HOOK); }
						ssend(conn_s_ps3mapi, PS3MAPI_OK_200);
					}
					else if(strcasecmp(cmd, "GETIDPS") == 0)
					{
						u64 _new_idps[2] = { 0, 0};
						{ system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_IDPS, (u64)(u32)_new_idps);}
						sprintf(buffer, "200 %016llX%016llX\r\n", _new_idps[0], _new_idps[1]);
						ssend(conn_s_ps3mapi, buffer);
					}
					else if(strcasecmp(cmd, "SETIDPS") == 0)
					{
						if(split == 1)
						{
							split = ssplit(param2, param1, 383, param2, 383);
							if(split == 1)
							{
								u64 part1 = convertH(param1);
								u64 part2 = convertH(param2);
								{ system_call_4(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_SET_IDPS, part1, part2);}
								ssend(conn_s_ps3mapi, PS3MAPI_OK_200);
							}
							else ssend(conn_s_ps3mapi, PS3MAPI_ERROR_501);
						}
						else ssend(conn_s_ps3mapi, PS3MAPI_ERROR_501);
					}
					else if(strcasecmp(cmd, "GETPSID") == 0)
					{
						u64 _new_psid[2] = { 0, 0};
						{ system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_PSID, (u64)(u32)_new_psid);}
						sprintf(buffer, "200 %016llX%016llX\r\n", _new_psid[0], _new_psid[1]);
						ssend(conn_s_ps3mapi, buffer);
					}
					else if(strcasecmp(cmd, "SETPSID") == 0)
					{
						if(split == 1)
						{
							split = ssplit(param2, param1, 383, param2, 383);
							if(split == 1)
							{
								u64 part1 = convertH(param1);
								u64 part2 = convertH(param2);
								{ system_call_4(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_SET_PSID, (u64)part1, (u64)part2);}
								ssend(conn_s_ps3mapi, PS3MAPI_OK_200);
							}
							else ssend(conn_s_ps3mapi, PS3MAPI_ERROR_501);
						}
						else ssend(conn_s_ps3mapi, PS3MAPI_ERROR_501);
					}
					else ssend(conn_s_ps3mapi, PS3MAPI_ERROR_502);
				}
				else
				{
					ssend(conn_s_ps3mapi, PS3MAPI_ERROR_501);
				}
			}
			else if(strcasecmp(cmd, "PROCESS") == 0)
			{
				if(split == 1)
				{
					split = ssplit(param1, cmd, 19, param2, 383);
					if(strcasecmp(cmd, "GETNAME") == 0)
					{
						if(split == 1)
						{
							u32 pid = val(param2);
							memset(param2, 0, sizeof(param2));
							{system_call_4(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_PROC_NAME_BY_PID, (u64)pid, (u64)(u32)param2); }
							sprintf(buffer, "200 %s\r\n", param2);
							ssend(conn_s_ps3mapi, buffer);
						}
						else ssend(conn_s_ps3mapi, PS3MAPI_ERROR_501);
					}
					else if(strcasecmp(cmd, "GETALLPID") == 0)
					{
						u32 pid_list[16];
						memset(buffer, 0, sizeof(buffer));
						sprintf(buffer, "200 ");
						{system_call_3(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_ALL_PROC_PID, (u64)(u32)pid_list); }
						for(int i = 0; i < 16; i++)
						{
							sprintf(buffer + strlen(buffer), "%i|", pid_list[i]);
						}
						sprintf(buffer + strlen(buffer), "\r\n");
						ssend(conn_s_ps3mapi, buffer);
					}
					else ssend(conn_s_ps3mapi, PS3MAPI_ERROR_502);
				}
				else
				{
					ssend(conn_s_ps3mapi, PS3MAPI_ERROR_501);
				}
			}
			else if(strcasecmp(cmd, "MEMORY") == 0)
			{
				if(split == 1)
				{
					split = ssplit(param1, cmd, 19, param2, 383);
					if(strcasecmp(cmd, "GET") == 0)
					{
						if(data_s > 0)
						{
							if(split == 1)
							{
								split = ssplit(param2, param1, 383, param2, 383);
								if(split == 1)
								{
									u32 attached_pid = val(param1);
									split = ssplit(param2, param1, 383, param2, 383);
									if(split == 1)
									{
										u64 offset = convertH(param1);
										u32 size = val(param2);
										int rr = -4;
										sys_addr_t sysmem = 0;
										if(sys_memory_allocate(BUFFER_SIZE_PS3MAPI, SYS_MEMORY_PAGE_SIZE_64K, &sysmem) == 0)
										{
											char *buffer2 = (char*)sysmem;
											ssend(conn_s_ps3mapi, PS3MAPI_OK_150);
											rr = 0;
											while(working)
											{
												if(size > BUFFER_SIZE_PS3MAPI)
												{
													u32 sizetoread = BUFFER_SIZE_PS3MAPI;
													u32 leftsize = size;
													if(size < BUFFER_SIZE_PS3MAPI) sizetoread = size;
													while(0 < leftsize)
													{
														system_call_6(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_PROC_MEM, (u64)attached_pid, offset, (u64)(u32)buffer2, (u64)sizetoread);
														if(send(data_s, buffer2, sizetoread, 0)<0) { rr = -3; break; }
														offset += sizetoread;
														leftsize -= sizetoread;
														if(leftsize < BUFFER_SIZE_PS3MAPI) sizetoread = leftsize;
														if(sizetoread == 0) break;
													}
													break;
												}
												else
												{
													system_call_6(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_PROC_MEM, (u64)attached_pid, (u64)offset, (u64)(u32)buffer2, (u64)size);
													if(send(data_s, buffer2, size, 0)<0) { rr = -3; break; }
													break;
												}
											}
											sys_memory_free(sysmem);
										}
										if(rr == 0) ssend(conn_s_ps3mapi, PS3MAPI_OK_226);
										else if(rr == -4) ssend(conn_s_ps3mapi, PS3MAPI_ERROR_550);
										else ssend(conn_s_ps3mapi, PS3MAPI_ERROR_451);
									}
									else ssend(conn_s_ps3mapi, PS3MAPI_ERROR_501);
								}
								else ssend(conn_s_ps3mapi, PS3MAPI_ERROR_501);
							}
							else ssend(conn_s_ps3mapi, PS3MAPI_ERROR_501);
						}
						else ssend(conn_s_ps3mapi, PS3MAPI_ERROR_425);
					}
					else if(strcasecmp(cmd, "SET") == 0)
					{
						if(data_s > 0)
						{
							if(split == 1)
							{
								split = ssplit(param2, param1, 383, param2, 383);
								if(split == 1)
								{
									u32 attached_pid = val(param1);
									u64 offset = convertH(param2);
									int rr = -1;
									sys_addr_t sysmem = 0;
									if(sys_memory_allocate(BUFFER_SIZE_PS3MAPI, SYS_MEMORY_PAGE_SIZE_64K, &sysmem) == 0)
									{
										char *buffer2 = (char*)sysmem;
										u64 read_e = 0;
										ssend(conn_s_ps3mapi, PS3MAPI_OK_150);
										rr = 0;
										while(working)
										{
											if((read_e = (u64)recv(data_s, buffer2, BUFFER_SIZE_PS3MAPI, MSG_WAITALL)) > 0)
											{
												system_call_6(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_SET_PROC_MEM, (u64)attached_pid, offset, (u64)(u32)buffer2, read_e);
												offset += read_e;
											}
											else
											{
												break;
											}
										}
										sys_memory_free(sysmem);
									}
									if(rr == 0) ssend(conn_s_ps3mapi, PS3MAPI_OK_226);
									else ssend(conn_s_ps3mapi, PS3MAPI_ERROR_451);
								}
								else ssend(conn_s_ps3mapi, PS3MAPI_ERROR_501);
							}
							else ssend(conn_s_ps3mapi, PS3MAPI_ERROR_501);
						}
						else ssend(conn_s_ps3mapi, PS3MAPI_ERROR_425);
					}
					else ssend(conn_s_ps3mapi, PS3MAPI_ERROR_502);
				}
				else
				{
					ssend(conn_s_ps3mapi, PS3MAPI_ERROR_501);
				}
			}
			else if(strcasecmp(cmd, "MODULE") == 0)
			{
				if(split == 1)
				{
					split = ssplit(param1, cmd, 19, param2, 383);
					if(strcasecmp(cmd, "GETNAME") == 0)
					{
						if(split == 1)
						{
							split = ssplit(param2, param1, 383, param2, 383);
							if(split == 1)
							{
								u32 pid = val(param1);
								s32 prxid = val(param2);
								memset(param2, 0, sizeof(param2));
								{system_call_5(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_PROC_MODULE_NAME, (u64)pid, (u64)prxid, (u64)(u32)param2); }
								sprintf(buffer, "200 %s\r\n", param2);
								ssend(conn_s_ps3mapi, buffer);
							}
							else ssend(conn_s_ps3mapi, PS3MAPI_ERROR_501);
						}
						else ssend(conn_s_ps3mapi, PS3MAPI_ERROR_501);
					}
					else if(strcasecmp(cmd, "GETFILENAME") == 0)
					{
						if(split == 1)
						{
							split = ssplit(param2, param1, 383, param2, 383);
							if(split == 1)
							{
								u32 pid = val(param1);
								s32 prxid = val(param2);
								memset(param2, 0, sizeof(param2));
								{system_call_5(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_PROC_MODULE_FILENAME, (u64)pid, (u64)prxid, (u64)(u32)param2); }
								sprintf(buffer, "200 %s\r\n", param2);
								ssend(conn_s_ps3mapi, buffer);
							}
							else ssend(conn_s_ps3mapi, PS3MAPI_ERROR_501);
						}
						else ssend(conn_s_ps3mapi, PS3MAPI_ERROR_501);
					}
					else if(strcasecmp(cmd, "GETALLPRXID") == 0)
					{
						if(split == 1)
						{
							s32 prxid_list[128];
							u32 pid = val(param2);
							memset(buffer, 0, sizeof(buffer));
							sprintf(buffer, "200 ");
							{system_call_4(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_ALL_PROC_MODULE_PID, (u64)pid, (u64)(u32)prxid_list); }
							for(int i = 0; i < 128; i++)
							{
								sprintf(buffer + strlen(buffer), "%i|", prxid_list[i]);
							}
							sprintf(buffer + strlen(buffer), "\r\n");
							ssend(conn_s_ps3mapi, buffer);
						}
						else ssend(conn_s_ps3mapi, PS3MAPI_ERROR_501);
					}
					else if(strcasecmp(cmd, "LOAD") == 0)
					{
						if(split == 1)
						{
							split = ssplit(param2, param1, 383, param2, 383);
							if(split == 1)
							{
								u32 pid = val(param1);
								{system_call_6(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_LOAD_PROC_MODULE, (u64)pid, (u64)(u32)param2, NULL, 0); }
								ssend(conn_s_ps3mapi, PS3MAPI_OK_200);
							}
							else ssend(conn_s_ps3mapi, PS3MAPI_ERROR_501);
						}
						else ssend(conn_s_ps3mapi, PS3MAPI_ERROR_501);
					}
					else if(strcasecmp(cmd, "UNLOAD") == 0)
					{
						if(split == 1)
						{
							split = ssplit(param2, param1, 383, param2, 383);
							if(split == 1)
							{
								u32 pid = val(param1);
								s32 prx_id = val(param2);
								{system_call_4(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_UNLOAD_PROC_MODULE, (u64)pid, (u64)prx_id); }
								ssend(conn_s_ps3mapi, PS3MAPI_OK_200);
							}
							else ssend(conn_s_ps3mapi, PS3MAPI_ERROR_501);
						}
						else ssend(conn_s_ps3mapi, PS3MAPI_ERROR_501);
					}
					else if(strcasecmp(cmd, "LOADVSHPLUG") == 0)
					{
						if(split == 1)
						{
							split = ssplit(param2, param1, 383, param2, 383);
							if(split == 1)
							{
								unsigned int slot = val(param1);
								if( slot ) {{system_call_5(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_LOAD_VSH_PLUGIN, (u64)slot, (u64)(u32)param2, NULL, 0); }}
								ssend(conn_s_ps3mapi, PS3MAPI_OK_200);
							}
						}
						else ssend(conn_s_ps3mapi, PS3MAPI_ERROR_501);
					}
					else if(strcasecmp(cmd, "UNLOADVSHPLUGS") == 0)
					{
						if(split == 1)
						{
							unsigned int slot = val(param2);
							if( slot ) {{system_call_2(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_UNLOAD_VSH_PLUGIN, (u64)slot); }}
							ssend(conn_s_ps3mapi, PS3MAPI_OK_200);
						}
						else ssend(conn_s_ps3mapi, PS3MAPI_ERROR_501);
					}
					else if(strcasecmp(cmd, "GETVSHPLUGINFO") == 0)
					{
						if(split == 1)
						{
							unsigned int slot = val(param2);
							memset(param1, 0, sizeof(param1));
							memset(param2, 0, sizeof(param2));
							{system_call_5(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_VSH_PLUGIN_INFO, (u64)slot, (u64)(u32)param1, (u64)(u32)param2); }
							sprintf(buffer, "200 %s|%s\r\n", param1, param2);
							ssend(conn_s_ps3mapi, buffer);
						}
						else ssend(conn_s_ps3mapi, PS3MAPI_ERROR_501);
					}
					else ssend(conn_s_ps3mapi, PS3MAPI_ERROR_502);
				}
				else
				{
					ssend(conn_s_ps3mapi, PS3MAPI_ERROR_501);
				}
			}
			else if(strcasecmp(cmd, "TYPE") == 0)
			{
				if(split == 1)
				{
					ssend(conn_s_ps3mapi, PS3MAPI_OK_200);
					if(strcmp(param1, "A") == 0) dataactive = 1;
					else if(strcmp(param1, "I") == 0)  dataactive = 0;
					else  dataactive = 1;
				}
				else
				{
					dataactive = 1;
				}
			}
			else if(strcasecmp(cmd, "PASV") == 0)
			{
				u8 pasv_retry = 0;
			pasv_again:
				if(!p1x)
				{
					cellRtcGetCurrentTick(&pTick);
					p1x = (((pTick.tick & 0xfe0000) >> 16) & 0xff) | 0x80; // use ports 32768 -> 65279 (0x8000 -> 0xFEFF)
					p2x = (((pTick.tick & 0x00ff00) >> 8) & 0xff);
				}
				data_ls = slisten(getPort(p1x, p2x), 1);

				if(data_ls >= 0)
				{
					sprintf(pasv_output, "227 Entering Passive Mode (%s,%i,%i)\r\n", ip_address, p1x, p2x);
					ssend(conn_s_ps3mapi, pasv_output);

					if((data_s = accept(data_ls, NULL, NULL)) > 0)
					{
						dataactive = 1;
					}
					else
					{
						ssend(conn_s_ps3mapi, PS3MAPI_ERROR_451);
					}

				}
				else
				{
					p1x = 0;
					if(pasv_retry<10)
					{
						pasv_retry++;
						goto pasv_again;
					}
					ssend(conn_s_ps3mapi, PS3MAPI_ERROR_451);
				}
			}
			else ssend(conn_s_ps3mapi, PS3MAPI_ERROR_502);

			if(dataactive == 1) dataactive = 0;
			else
			{
				sclose(&data_s);
				if(data_ls>0) { sclose(&data_ls); data_ls = FAILED; }
			}
		}
		else
		{
			connactive = 0;
			break;
		}

		sys_timer_usleep(1668);
	}

	sprintf(buffer, PS3MAPI_DISCONNECT_NOTIF, inet_ntoa(conn_info.remote_adr));
	show_msg(buffer);
	sclose(&conn_s_ps3mapi);
	sclose(&data_s);
	sys_ppu_thread_exit(0);
}

static void ps3mapi_thread(u64 arg)
{
	int core_minversion = 0;
	{ system_call_2(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_CORE_MINVERSION); core_minversion = (int)(p1); }
	if((core_minversion !=0) &&(PS3MAPI_CORE_MINVERSION == core_minversion)) //Check if ps3mapi core has a compatible min_version.
	{
		int list_s = FAILED;
	relisten:
		if(working) list_s = slisten(PS3MAPIPORT, 4);
		else goto end;

		if(working && (list_s<0))
		{
			sys_timer_sleep(3);
			if(working) goto relisten;
			else goto end;
		}

		//if(working && (list_s >= 0))
		{
			while(working)
			{
				sys_timer_usleep(1668);
				int conn_s_ps3mapi;
				if (!working) goto end;
				else
				if(working && (conn_s_ps3mapi = accept(list_s, NULL, NULL)) > 0)
				{
					sys_ppu_thread_t id;
					if(working) sys_ppu_thread_create(&id, handleclient_ps3mapi, (u64)conn_s_ps3mapi, -0x1d8, 0x10000, 0, THREAD02_NAME_PS3MAPI);
					else {sclose(&conn_s_ps3mapi); break;}
				}
				else
				if((sys_net_errno == SYS_NET_EBADF) || (sys_net_errno == SYS_NET_ENETDOWN))
				{
					sclose(&list_s);
					list_s = FAILED;
					if(working) goto relisten;
					else break;
				}
			}
		}
end:
		sclose(&list_s);
		sys_ppu_thread_exit(0);
	}
	else show_msg((char *)"PS3MAPI Server not loaded!");

	sys_ppu_thread_exit(0);
}

////////////////////////////////////////
///////////// PS3MAPI END //////////////
////////////////////////////////////////

#endif

#ifdef COBRA_ONLY
 #ifndef SYSCALL8_OPCODE_PS3MAPI
	#define SYSCALL8_OPCODE_PS3MAPI			 			0x7777
	#define PS3MAPI_OPCODE_GET_VSH_PLUGIN_INFO			0x0047
 #endif

static unsigned int get_vsh_plugin_slot_by_name(char *name, bool unload)
{
	char tmp_name[30];
	char tmp_filename[256];
	unsigned int slot;

	for (slot = 1; slot < 7; slot++)
	{
		memset(tmp_name, 0, sizeof(tmp_name));
		memset(tmp_filename, 0, sizeof(tmp_filename));
		{system_call_5(SC_COBRA_SYSCALL8, SYSCALL8_OPCODE_PS3MAPI, PS3MAPI_OPCODE_GET_VSH_PLUGIN_INFO, (u64)slot, (u64)(u32)tmp_name, (u64)(u32)tmp_filename); }
		if(strstr(tmp_filename, name) || !strcmp(tmp_name, name)) {if(unload) cobra_unload_vsh_plugin(slot); break;}
	}
	return slot;
}
#endif // #ifdef COBRA_ONLY

#ifdef VIRTUAL_PAD
#include <cell/pad/libpad_dbg.h>

#define SC_PAD_SET_DATA_INSERT_MODE		(573)
#define SC_PAD_REGISTER_CONTROLLER		(574)

static uint32_t vcombo = 0;

static int32_t vpad_handle = -1;

static inline void sys_pad_dbg_ldd_register_controller(uint8_t *data, int32_t *handle, uint8_t addr, uint32_t capability)
{
  // syscall for registering a virtual controller with custom capabilities
  system_call_4(SC_PAD_REGISTER_CONTROLLER, (uint8_t *)data, (int32_t *)handle, (uint8_t)addr, (uint32_t)capability);
}

static inline void sys_pad_dbg_ldd_set_data_insert_mode(int32_t handle, uint16_t addr, uint32_t *mode, uint8_t addr2)
{
  // syscall for controlling button data filter (allows a virtual controller to be used in games)
  system_call_4(SC_PAD_SET_DATA_INSERT_MODE, handle, addr, mode, addr2);
}

static int32_t register_ldd_controller(void)
{
  uint8_t data[0x114];
  int32_t port;
  uint32_t capability, mode, port_setting;

  // register ldd controller with custom device capability
  if (vpad_handle < 0)
  {
    capability = 0xFFFF; // CELL_PAD_CAPABILITY_PS3_CONFORMITY | CELL_PAD_CAPABILITY_PRESS_MODE | CELL_PAD_CAPABILITY_HP_ANALOG_STICK | CELL_PAD_CAPABILITY_ACTUATOR;
    sys_pad_dbg_ldd_register_controller(data, (int32_t *)&(vpad_handle), 5, (uint32_t)capability << 1);
    //vpad_handle = cellPadLddRegisterController();
    sys_timer_usleep(1000*500); // allow some time for ps3 to register ldd controller

    if (vpad_handle < 0) return(vpad_handle);

    // all pad data into games
    mode = CELL_PAD_LDD_INSERT_DATA_INTO_GAME_MODE_ON; // = (1)
    sys_pad_dbg_ldd_set_data_insert_mode((int32_t)vpad_handle, 0x100, (uint32_t *)&mode, 4);

    // set press and sensor mode on
    port_setting = CELL_PAD_SETTING_PRESS_ON | CELL_PAD_SETTING_SENSOR_ON;
    port = cellPadLddGetPortNo(vpad_handle);

    if (port < 0) return(port);

    cellPadSetPortSetting(port, port_setting);
  }
  return(CELL_PAD_OK);
}

static int32_t unregister_ldd_controller(void)
{
  if (vpad_handle >= 0)
  {
    int32_t r = cellPadLddUnregisterController(vpad_handle);
    if (r != CELL_OK) return(r);
    vpad_handle = -1;
  }
  return(CELL_PAD_OK);
}

static void parse_pad_command(char *param, u8 is_combo)
{
	register_ldd_controller();

	CellPadData data;
	memset(&data, 0, sizeof(CellPadData));
	data.len = CELL_PAD_MAX_CODES;

	// set default controller values
	data.button[CELL_PAD_BTN_OFFSET_ANALOG_LEFT_X] = 0x0080;
	data.button[CELL_PAD_BTN_OFFSET_ANALOG_LEFT_Y] = 0x0080;

	data.button[CELL_PAD_BTN_OFFSET_ANALOG_RIGHT_X] = 0x0080;
	data.button[CELL_PAD_BTN_OFFSET_ANALOG_RIGHT_Y] = 0x0080;

	data.button[CELL_PAD_BTN_OFFSET_SENSOR_X] = 0x0200;
	data.button[CELL_PAD_BTN_OFFSET_SENSOR_Y] = 0x0200;
	data.button[CELL_PAD_BTN_OFFSET_SENSOR_Z] = 0x0200;
	data.button[CELL_PAD_BTN_OFFSET_SENSOR_G] = 0x0200;

	if(strcasestr(param, "off")) unregister_ldd_controller(); else
	{
		u32 delay = 70000;

		// press button
		if(strcasestr(param, "psbtn") ) {data.button[0] |= CELL_PAD_CTRL_LDD_PS;}

		if(strcasestr(param, "start") ) {data.button[CELL_PAD_BTN_OFFSET_DIGITAL1] |= CELL_PAD_CTRL_START; }
		if(strcasestr(param, "select")) {data.button[CELL_PAD_BTN_OFFSET_DIGITAL1] |= CELL_PAD_CTRL_SELECT;}

		if (strcasestr(param, "analogL"))
		{
			// pad.ps3?analogL_up
			if(strcasestr(param, "up"   )) {data.button[CELL_PAD_BTN_OFFSET_ANALOG_LEFT_Y] = 0x00;}
			if(strcasestr(param, "down" )) {data.button[CELL_PAD_BTN_OFFSET_ANALOG_LEFT_Y] = 0xff;}
			if(strcasestr(param, "left" )) {data.button[CELL_PAD_BTN_OFFSET_ANALOG_LEFT_X] = 0x00;}
			if(strcasestr(param, "right")) {data.button[CELL_PAD_BTN_OFFSET_ANALOG_LEFT_X] = 0xff;}
			delay = 150000;
		}
		else if (strcasestr(param, "analogR"))
		{
			// pad.ps3?analogR_up
			if(strcasestr(param, "up"   )) {data.button[CELL_PAD_BTN_OFFSET_ANALOG_RIGHT_Y] = 0x00;}
			if(strcasestr(param, "down" )) {data.button[CELL_PAD_BTN_OFFSET_ANALOG_RIGHT_Y] = 0xff;}
			if(strcasestr(param, "left" )) {data.button[CELL_PAD_BTN_OFFSET_ANALOG_RIGHT_X] = 0x00;}
			if(strcasestr(param, "right")) {data.button[CELL_PAD_BTN_OFFSET_ANALOG_RIGHT_X] = 0xff;}
			delay = 150000;
		}
		else
		{
			if(strcasestr(param, "up"   )) {data.button[CELL_PAD_BTN_OFFSET_DIGITAL1] |= CELL_PAD_CTRL_UP;    data.button[CELL_PAD_BTN_OFFSET_PRESS_UP]    = 0xFF;}
			if(strcasestr(param, "down" )) {data.button[CELL_PAD_BTN_OFFSET_DIGITAL1] |= CELL_PAD_CTRL_DOWN;  data.button[CELL_PAD_BTN_OFFSET_PRESS_DOWN]  = 0xFF;}
			if(strcasestr(param, "left" )) {data.button[CELL_PAD_BTN_OFFSET_DIGITAL1] |= CELL_PAD_CTRL_LEFT;  data.button[CELL_PAD_BTN_OFFSET_PRESS_LEFT]  = 0xFF;}
			if(strcasestr(param, "right")) {data.button[CELL_PAD_BTN_OFFSET_DIGITAL1] |= CELL_PAD_CTRL_RIGHT; data.button[CELL_PAD_BTN_OFFSET_PRESS_RIGHT] = 0xFF;}
		}

		if(strcasestr(param, "cross"   )) {data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] |= CELL_PAD_CTRL_CROSS;    data.button[CELL_PAD_BTN_OFFSET_PRESS_CROSS]    = 0xFF;}
		if(strcasestr(param, "square"  )) {data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] |= CELL_PAD_CTRL_SQUARE;   data.button[CELL_PAD_BTN_OFFSET_PRESS_SQUARE]   = 0xFF;}
		if(strcasestr(param, "circle"  )) {data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] |= CELL_PAD_CTRL_CIRCLE;   data.button[CELL_PAD_BTN_OFFSET_PRESS_CIRCLE]   = 0xFF;}
		if(strcasestr(param, "triangle")) {data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] |= CELL_PAD_CTRL_TRIANGLE; data.button[CELL_PAD_BTN_OFFSET_PRESS_TRIANGLE] = 0xFF;}

		if(strcasestr(param, "l1")) {data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] |= CELL_PAD_CTRL_L1; data.button[CELL_PAD_BTN_OFFSET_PRESS_L1] = 0xFF;}
		if(strcasestr(param, "l2")) {data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] |= CELL_PAD_CTRL_L2; data.button[CELL_PAD_BTN_OFFSET_PRESS_L2] = 0xFF;}
		if(strcasestr(param, "r1")) {data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] |= CELL_PAD_CTRL_R1; data.button[CELL_PAD_BTN_OFFSET_PRESS_R1] = 0xFF;}
		if(strcasestr(param, "r2")) {data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] |= CELL_PAD_CTRL_R2; data.button[CELL_PAD_BTN_OFFSET_PRESS_R2] = 0xFF;}

		if(strcasestr(param, "l3")) {data.button[CELL_PAD_BTN_OFFSET_DIGITAL1] |= CELL_PAD_CTRL_L3;}
		if(strcasestr(param, "r3")) {data.button[CELL_PAD_BTN_OFFSET_DIGITAL1] |= CELL_PAD_CTRL_R3;}

		if(is_combo) {vcombo = (data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] << 8) | (data.button[CELL_PAD_BTN_OFFSET_DIGITAL1]); return;}

		// send pad data to virtual pad
		cellPadLddDataInsert(vpad_handle, &data);

		if(!strcasestr(param, "hold"))
		{
			sys_timer_usleep(delay); // hold for 70ms

			// release all buttons and set default values
			memset(&data, 0, sizeof(CellPadData));
			data.len = CELL_PAD_MAX_CODES;

			data.button[CELL_PAD_BTN_OFFSET_ANALOG_LEFT_X] = 0x0080;
			data.button[CELL_PAD_BTN_OFFSET_ANALOG_LEFT_Y] = 0x0080;

			data.button[CELL_PAD_BTN_OFFSET_ANALOG_RIGHT_X] = 0x0080;
			data.button[CELL_PAD_BTN_OFFSET_ANALOG_RIGHT_Y] = 0x0080;

			data.button[CELL_PAD_BTN_OFFSET_SENSOR_X] = 0x0200;
			data.button[CELL_PAD_BTN_OFFSET_SENSOR_Y] = 0x0200;
			data.button[CELL_PAD_BTN_OFFSET_SENSOR_Z] = 0x0200;
			data.button[CELL_PAD_BTN_OFFSET_SENSOR_G] = 0x0200;

			// send pad data to virtual pad
			cellPadLddDataInsert(vpad_handle, &data);
		}
	}
}
#endif

static CellPadData pad_read(void)
{
	CellPadData pad_data; pad_data.len=0;

	for(u8 n=0;n<10;n++)
	{
		if(cellPadGetData(0, &pad_data) != CELL_PAD_OK || pad_data.len == 0)
			if(cellPadGetData(1, &pad_data) != CELL_PAD_OK || pad_data.len == 0)
					cellPadGetData(2, &pad_data);

		if(pad_data.len > 0) break;
		sys_timer_usleep(100000);
	}

	return pad_data;
}

#define MIN_FANSPEED	(20)
#define DEFAULT_MIN_FANSPEED	(25)
#define MAX_FANSPEED	(0xF4)
#define MAX_TEMPERATURE	(85)
#define MY_TEMP 		(68)

#define FAN_AUTO 		(0)

static u8 fan_speed=0x33;
static u8 old_fan=0x33;
static u32 max_temp=MY_TEMP;

#define SC_SET_FAN_POLICY				(389)
#define SC_GET_FAN_POLICY				(409)
#define SC_GET_TEMPERATURE				(383)

uint64_t get_fan_policy_offset=0;
uint64_t set_fan_policy_offset=0;

static u64 backup[3];

static bool fan_ps2_mode=false; // temporary disable dynamic fan control

static void get_temperature(u32 _dev, u32 *_temp);
static void fan_control(u8 set_fanspeed, u8 initial);
static void restore_fan(u8 set_ps2_temp);

static void get_temperature(u32 _dev, u32 *_temp)
{
	system_call_2(SC_GET_TEMPERATURE, (uint64_t)(u32) _dev, (uint64_t)(u32) _temp);
}

static int sys_sm_set_fan_policy(u8 arg0, u8 arg1, u8 arg2)
{
	system_call_3(SC_SET_FAN_POLICY, (u64) arg0, (u64) arg1, (u64) arg2);
	return_to_user_prog(int);
}

static int sys_sm_get_fan_policy(u8 id, u8 *st, u8 *mode, u8 *speed, u8 *unknown)
{
	system_call_5(SC_GET_FAN_POLICY, (u64) id, (u64)(u32) st, (u64)(u32) mode, (u64)(u32) speed, (u64)(u32) unknown);
	return_to_user_prog(int);
}

static void fan_control(u8 set_fanspeed, u8 initial)
{
	if(fan_ps2_mode) return; //do not change fan settings while PS2 game is mounted

	if(get_fan_policy_offset)
	{
		if(!initial)
		{
			if(backup[0]==0)
			{
				backup[0]=1;
				//backup[3]=peekq(syscall_base + (u64) (130 * 8));
				//backup[4]=peekq(syscall_base + (u64) (138 * 8));
				//backup[5]=peekq(syscall_base + (u64) (379 * 8));

				{ PS3MAPI_ENABLE_ACCESS_SYSCALL8 }

				backup[1]=peekq(set_fan_policy_offset);
				backup[2]=peekq(get_fan_policy_offset);
				lv2poke32(get_fan_policy_offset, 0x38600001); // sys 409 get_fan_policy  4.55/4.60/4.65/4.70/4.75/4.76
				lv2poke32(set_fan_policy_offset, 0x38600001); // sys 389 set_fan_policy

				sys_sm_set_fan_policy(0, 2, 0x33);

				{ PS3MAPI_DISABLE_ACCESS_SYSCALL8 }
		    }
		}

		if(set_fanspeed<0x33)
		{
			u8 st, mode, unknown;
			u8 fan_speed8=0;
			sys_sm_get_fan_policy(0, &st, &mode, &fan_speed8, &unknown);
			if(fan_speed8<0x33) return;
			fan_speed=fan_speed8;
		}
		else
			fan_speed=set_fanspeed;

		if(fan_speed<0x33 || fan_speed>0xFC)
		{
			fan_speed=0x48;
			sys_sm_set_fan_policy(0, 2, fan_speed);
			sys_timer_sleep(2);
		}
		old_fan=fan_speed;
		sys_sm_set_fan_policy(0, 2, fan_speed);
	}
}

static void restore_fan(u8 set_ps2_temp)
{
	if(backup[0]==1 && (get_fan_policy_offset>0))
	{
		//pokeq(backup[0] + (u64) (130 * 8), backup[3]);
		//pokeq(backup[0] + (u64) (138 * 8), backup[4]);
		//pokeq(backup[0] + (u64) (379 * 8), backup[5]);

		if(set_ps2_temp)
		{
			webman_config->ps2temp=RANGE(webman_config->ps2temp, 20, 99); //%
			sys_sm_set_fan_policy(0, 2, ((webman_config->ps2temp*255)/100));
			fan_ps2_mode=true;
		}
		else sys_sm_set_fan_policy(0, 1, 0x0); //syscon

		{ PS3MAPI_ENABLE_ACCESS_SYSCALL8 }

		pokeq(set_fan_policy_offset, backup[1]);  // sys 389 set_fan_policy
		pokeq(get_fan_policy_offset, backup[2]);  // sys 409 get_fan_policy  4.55/4.60/4.65/4.70/4.75/4.76

		{ PS3MAPI_DISABLE_ACCESS_SYSCALL8 }

		backup[0]=0;
	}
}

static void enable_fan_control(u8 enable, char *msg)
{
	if(enable < 2) webman_config->fanc = enable;
	else           webman_config->fanc = (webman_config->fanc ? 0 : 1);

	max_temp=0;
	if(webman_config->fanc)
	{
		if(webman_config->temp0==FAN_AUTO) max_temp=webman_config->temp1; else max_temp=0;
		fan_control(webman_config->temp0, 0);
		sprintf((char*) msg, "%s %s", STR_FANCTRL3, STR_ENABLED);
	}
	else
	{
		restore_fan(0); //syscon
		sprintf((char*) msg, "%s %s", STR_FANCTRL3, STR_DISABLED);
	}
	save_settings();
	show_msg((char*) msg);
}

static void reset_fan_mode(void)
{
	fan_ps2_mode=false;

	webman_config->temp0= (u8)(((float)(webman_config->manu+1) * 255.f)/100.f); // manual fan speed
	webman_config->temp0=RANGE(webman_config->temp0, 0x33, MAX_FANSPEED);
	fan_control(webman_config->temp0, 0);

	if(max_temp) webman_config->temp0=FAN_AUTO; // enable dynamic fan mode
}

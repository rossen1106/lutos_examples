#include "lutos.h"
#include "b2b.h"
#include "b2b_s.h"
#include "iot301.h"
#include "plc301.h"

plc_status_block_t PSB;
plc_control_block_t PCB;
broadcast_data_block_t BDB;

static uint8_t StationID, LinkHealth, OperationMoves;
static uint16_t ttlStroke;
static TICK *CommTimeoutTick;

#define MAIN_LOOP_MS	1

#define TTL_PLC			(1*1000/MAIN_LOOP_MS)
#define TTL_FRAME		(2*1000/MAIN_LOOP_MS)
//#define TTL_STROKE		(30*1000/MAIN_LOOP_MS)

#define ALARM_PARTNER_ABNORMAL		0x01
#define ALARM_FRAME_ERROR			0x02
#define ALARM_POSITION_ERROR		0x04
#define ALARM_STROKE_TIMEOUT		0x08

#define PLC_INIT		0
#define PLC_OFFLINE		1
#define PLC_ONLINE		2
#define PLC_OPERATION	3
#define PLC_ALARM		4

#define OP_END			0
#define OP_START		1
#define OP_FOO_RETURN	2
#define OP_BAR_RETURN	3
#define OP_FOO_READY	4
#define OP_BAR_READY	5
#define OP_GOTO_FOO		6
#define OP_GOTO_BAR		7

#define FOP_END			0
#define FOP_START		1
#define FOP_DOWN		2
#define FOP_FLOCK		3
#define FOP_UP			4

#define OL_IDLE			0
#define OL_BLADE		1
#define OL_ACK			2
#define OL_MOVE			3

#define CHECK_BLADES_POSITION_OK()		(PCB.fwd_limit ^ PCB.bwd_limit)

#define BLUB_OFF					0
#define BLUB_RED_ON					1
#define BLUB_GREEN_ON				2
#define BLUB_RED_BLINK				3
#define BLUB_GREEN_BLINK			4
#define BLUB_RED_ON_GREEN_BLINK		5
#define BLUB_GREEN_ON_RED_BLINK		6
#define BLUB_GREEN_RED_BLINK		7
#define BLUB_ALL_ON					8

void blub_control(uint8_t mode)
{
	static uint16_t tick;
	tick++;
	if( tick > 1000 ) tick = 0;
	switch(mode)
	{
	   case BLUB_RED_ON:
			PSB.red_blub = 1;
			PSB.green_blub = 0;
			break;
	   
	   case BLUB_GREEN_ON:
			PSB.red_blub = 0;
			PSB.green_blub = 1;
			break;
	   
	   case BLUB_RED_BLINK:
			PSB.red_blub = (tick > 500) ? 1 : 0;
			PSB.green_blub = 0;
			break;
	   
	   case BLUB_GREEN_BLINK:
			PSB.red_blub = 0;
			PSB.green_blub = (tick > 500) ? 1 : 0;
			break;
	   
	   case BLUB_RED_ON_GREEN_BLINK:
			PSB.red_blub = 1;
			PSB.green_blub = (tick > 500) ? 1 : 0;
			break;
	   
	   case BLUB_GREEN_ON_RED_BLINK:
			PSB.red_blub = (tick > 500) ? 1 : 0;
			PSB.green_blub = 1;
			break;
	   
	   case BLUB_GREEN_RED_BLINK:
			PSB.red_blub = (tick > 500) ? 1 : 0;
			PSB.green_blub = (tick > 500) ? 0 : 1;
			break;
	   
	   case BLUB_ALL_ON:
			PSB.red_blub = 1;
			PSB.green_blub = 1;
			break;
	   
	   case BLUB_OFF:
	   default:
			PSB.red_blub = 0;
			PSB.green_blub = 0;
			break;
	}
}

uint8_t get_blade_recipe(uint8_t m)
{
	uint8_t recipe;
	
	if( (m & 0x01) == 0 )
	{
		recipe = ( (PCB.dep_blade_recipe & (1<<(m>>1))) == 0 ) ? 0 : 1;
	}
	else
	{
		recipe = ( (PCB.ret_blade_recipe & (1<<(m>>1))) == 0 ) ? 0 : 1;
	}
	return recipe;
}

uint8_t get_frame_recipe(uint8_t m)
{
	uint8_t recipe;
	
	if( (m & 0x01) == 0 )
	{
		recipe = ( (PCB.dep_frame_recipe & (1<<(m>>1))) == 0 ) ? 0 : 1;
	}
	else
	{
		recipe = ( (PCB.ret_frame_recipe & (1<<(m>>1))) == 0 ) ? 0 : 1;
	}
	return recipe;
}

uint8_t get_speed_recipe(uint8_t m)
{
	uint8_t recipe;
	
	if( (m & 0x01) == 0 )
	{
		recipe = ( (PCB.dep_speed_recipe & (1<<(m>>1))) == 0 ) ? 0 : 1;
	}
	else
	{
		recipe = ( (PCB.ret_speed_recipe & (1<<(m>>1))) == 0 ) ? 0 : 1;
	}
	return recipe;
}

uint8_t operation_service(void)
{
	static uint8_t State, ReturnStroke;
	static uint16_t TimeCount;
	
	switch(State)
	{
	   case OP_END:
			TimeCount = 0;
			State = OP_START;
			break;
	   
	   case OP_START:
			if( CHECK_BLADES_POSITION_OK() == 1 )
			{
				if( PCB.stroke_dir_ctrl == 1 )
				{
					if( PCB.stroke_foo_bar == 1 )
					{
						if( PCB.fwd_limit == 1 )
						{
							ReturnStroke = 0;
							PSB.foo_valve = 0;
							PSB.bar_valve = get_blade_recipe(OperationMoves);
							PSB.frame_valve = get_frame_recipe(OperationMoves);
							TimeCount = 0;
							State = OP_FOO_READY;
						}
						else
						{
							ReturnStroke = 1;
							PSB.foo_valve = 0;
							PSB.bar_valve = 0;
							PSB.frame_valve = 0;
							TimeCount = 0;;
							State = OP_BAR_RETURN;
						}
					}
					else
					{
						if( PCB.fwd_limit == 1 )
						{
							ReturnStroke = 1;
							PSB.foo_valve = 0;
							PSB.bar_valve = 0;
							PSB.frame_valve = 0;
							TimeCount = 0;;
							State = OP_FOO_RETURN;
						}
						else
						{
							ReturnStroke = 0;
							PSB.bar_valve = 0;
							PSB.foo_valve = get_blade_recipe(OperationMoves);
							PSB.frame_valve = get_frame_recipe(OperationMoves);
							TimeCount = 0;
							State = OP_BAR_READY;
						}
					}
				}
				else
				{
					if( PCB.fwd_limit == 1 )
					{
						ReturnStroke = 0;
						PSB.foo_valve = 0;
						PSB.bar_valve = get_blade_recipe(OperationMoves);
						PSB.frame_valve = get_frame_recipe(OperationMoves);
						TimeCount = 0;
						State = OP_FOO_READY;
					}
					else
					{
						ReturnStroke = 0;
						PSB.bar_valve = 0;
						PSB.foo_valve = get_blade_recipe(OperationMoves);
						PSB.frame_valve = get_frame_recipe(OperationMoves);
						TimeCount = 0;
						State = OP_BAR_READY;
					}
				}
			}
			else
			{
				PSB.alarm |= ALARM_POSITION_ERROR;
				State = OP_END;
			}
			break;
	   
	   case OP_FOO_READY:
			if( TimeCount > ((uint16_t)PCB.stroke_delay * 10) )
			{
				if( PSB.frame_valve == PSB.frame_low &&
					PSB.frame_valve != PSB.frame_high )
				{
					PSB.apply_s2 = get_speed_recipe(OperationMoves);
					PSB.bwd_mov_req = 1;
					TimeCount = 0;
					State = OP_GOTO_BAR;
				}
				else
				{
					if( TimeCount > TTL_FRAME )
					{
						PSB.alarm |= ALARM_FRAME_ERROR;
						State = OP_END;
					}
				}
			}
			break;
	   
	   case OP_GOTO_BAR:
			if( PCB.move_complete == 1 &&
				PCB.fwd_limit == 0 &&
				PCB.bwd_limit == 1 )
			{
				if( ReturnStroke == 0 ) OperationMoves++;
				else ReturnStroke = 0;
				if( OperationMoves < PCB.stroke_moves )
				{
					PSB.bar_valve = 0;
					PSB.foo_valve = get_blade_recipe(OperationMoves);
					PSB.frame_valve = get_frame_recipe(OperationMoves);
					TimeCount = 0;
					State = OP_BAR_READY;
				}
				else
				{
					PSB.frame_valve = 0;
					OperationMoves = 0;
					State = OP_END;
				}
				PSB.bwd_mov_req = 0;
			}
			else
			{
				if( TimeCount > ttlStroke )
				{
					PSB.alarm |= ALARM_STROKE_TIMEOUT;
					State = OP_END;
				}
				if( TimeCount > TTL_FRAME && PCB.move_complete == 1 )
				{
					PSB.alarm |= ALARM_POSITION_ERROR;
					State = OP_END;
				}
			}
			if( PCB.move_complete == 0 ) PSB.bwd_mov_req = 0;
			break;
	   
	   case OP_BAR_READY:
			if( TimeCount > ((uint16_t)PCB.stroke_delay * 10) )
			{
				if( PSB.frame_valve == PSB.frame_low &&
					PSB.frame_valve != PSB.frame_high )
				{
					PSB.apply_s2 = get_speed_recipe(OperationMoves);
					PSB.fwd_mov_req = 1;
					TimeCount = 0;
					State = OP_GOTO_FOO;
				}
				else
				{
					if( TimeCount > TTL_FRAME )
					{
						PSB.alarm |= ALARM_FRAME_ERROR;
						State = OP_END;
					}
				}
			}
			break;
	   
	   case OP_GOTO_FOO:
			if( PCB.move_complete == 1 &&
				PCB.fwd_limit == 1 &&
				PCB.bwd_limit == 0 )
			{
				if( ReturnStroke == 0 ) OperationMoves++;
				else ReturnStroke = 0;
				if( OperationMoves < PCB.stroke_moves )
				{
					PSB.foo_valve = 0;
					PSB.bar_valve = get_blade_recipe(OperationMoves);
					PSB.frame_valve = get_frame_recipe(OperationMoves);
					TimeCount = 0;
					State = OP_FOO_READY;
				}
				else
				{
					PSB.frame_valve = 0;
					OperationMoves = 0;
					State = OP_END;
				}
				PSB.fwd_mov_req = 0;
			}
			else
			{
				if( TimeCount > ttlStroke )
				{
					PSB.alarm |= ALARM_STROKE_TIMEOUT;
					State = OP_END;
				}
				if( TimeCount > TTL_FRAME && PCB.move_complete == 1 )
				{
					PSB.alarm |= ALARM_POSITION_ERROR;
					State = OP_END;
				}
			}
			if( PCB.move_complete == 0 ) PSB.fwd_mov_req = 0;
			break;
	   
	   case OP_BAR_RETURN:
			if( PSB.frame_high == 1 && PSB.frame_low == 0 )
			{
				PSB.apply_s2 = 0;
				PSB.fwd_mov_req = 1;
				TimeCount = 0;
				State = OP_GOTO_FOO;
			}
			else
			{
				if( TimeCount > TTL_FRAME )
				{
					PSB.alarm |= ALARM_FRAME_ERROR;
					State = OP_END;
				}
			}
			break;
	   
	   case OP_FOO_RETURN:
			if( PSB.frame_high == 1 && PSB.frame_low == 0 )
			{
				PSB.apply_s2 = 0;
				PSB.bwd_mov_req = 1;
				TimeCount = 0;
				State = OP_GOTO_BAR;
			}
			else
			{
				if( TimeCount > TTL_FRAME )
				{
					PSB.alarm |= ALARM_FRAME_ERROR;
					State = OP_END;
				}
			}
			break;
	   
	   default:
			State = OP_END;
			break;
	}
	TimeCount++;
	return State;
}

uint8_t flocking_operation_service(void)
{
	static uint8_t State;
	static uint16_t TimeCount, FlockTime;
	
	switch(State)
	{
	   case FOP_END:
			TimeCount = 0;
			State = FOP_START;
			break;
	   
	   case FOP_START:
			PSB.frame_valve = 1;
			FlockTime = PCB.stroke_moves - 16;
			if( FlockTime > 60 ) FlockTime = 60;
			FlockTime *= 1000;
			State = FOP_DOWN;
			break;
	   
	   case FOP_DOWN:
			if( PSB.frame_valve == PSB.frame_low &&
				PSB.frame_valve != PSB.frame_high )
			{
				TimeCount = 0;
				State = FOP_FLOCK;
			}
			else
			{
				if( TimeCount > TTL_FRAME )
				{
					PSB.alarm |= ALARM_FRAME_ERROR;
					State = FOP_END;
				}
			}
			break;
	   
	   case FOP_FLOCK:
			if( TimeCount >= FlockTime )
			{
				PSB.frame_valve = 0;
				TimeCount = 0;
				State = FOP_UP;
			}
			break;
	   
	   case FOP_UP:
			if( PSB.frame_valve == PSB.frame_low &&
				PSB.frame_valve != PSB.frame_high )
			{
				TimeCount = 0;
				State = FOP_END;
			}
			else
			{
				if( TimeCount > TTL_FRAME )
				{
					PSB.alarm |= ALARM_FRAME_ERROR;
					State = FOP_END;
				}
			}
			break;
	   
	   default:
			State = FOP_END;
			break;
	}
	TimeCount++;
	return State;
}

void online_service(void)
{
	static uint8_t State;
	static uint16_t TimeCount;
	
	switch(State)
	{
	   case OL_IDLE:
			if( PSB.stroke_btn == 1 )
			{
				if( PCB.fwd_limit == 1 )
				{
					PSB.apply_s2 = ( PCB.stroke_dir_ctrl == 1 && PCB.stroke_foo_bar == 1 ) ? 1 : 0;
					PSB.foo_valve = 0;
					PSB.bar_valve = 1;
					PSB.bwd_mov_req = 1;
					State = OL_BLADE;
				}
				else
				{
					PSB.apply_s2 = ( PCB.stroke_dir_ctrl == 1 && PCB.stroke_foo_bar == 1 ) ? 0 : 1;
					PSB.foo_valve = 1;
					PSB.bar_valve = 0;
					PSB.fwd_mov_req = 1;
					State = OL_BLADE;
				}
				TimeCount = 0;
			}
			break;
	   
	   case OL_BLADE:
			if( TimeCount > ((uint16_t)PCB.stroke_delay * 10) )
			{
				if( PCB.fwd_limit == 1 )
				{
					PSB.bwd_mov_req = 1;
					State = OL_ACK;
				}
				else
				{
					PSB.fwd_mov_req = 1;
					State = OL_ACK;
				}
				TimeCount = 0;
			}
			break;
	   
	   case OL_ACK:
			if( TimeCount > ttlStroke || PCB.move_complete == 0 )
			{
				PSB.bwd_mov_req = 0;
				PSB.fwd_mov_req = 0;
				State = OL_MOVE;
			}
			break;
	   
	   case OL_MOVE:
			if( PCB.move_complete == 1 )
			{
				if( PSB.stroke_btn == 0 ) State = OL_IDLE;
			}
			break;
	   
		default:
			State = OL_IDLE;
			break;
	}
	TimeCount++;
	//PSB.foo_valve = PSB.foo_btn;
	//PSB.bar_valve = PSB.bar_btn;
	if( PSB.frame_btn != PLC_IN(DI_FRAME) && PSB.frame_btn == 1 )
	{
		PSB.frame_valve = !PSB.frame_valve;
	}
}

void offline_service(void)
{
	PSB.servo_off = 1;
	PSB.clear_alarm = PSB.stroke_btn;
}

void io_service(void)
{
	//PSB.online = PLC_IN(DI_ONLINE) & (!PCB.offline_ctrl);
	PSB.sys_run_req = PLC_IN(DI_RUN);
	PSB.sys_pause_req = PLC_IN(DI_PAUSE);
	PSB.rack_fwd_req = PLC_IN(DI_FWD);
	PSB.rack_rev_req = PLC_IN(DI_REV);
	
	PSB.online_btn = PLC_IN(DI_ONLINE);
	PSB.stroke_btn = PLC_IN(DI_STROKE);
	PSB.foo_btn = PLC_IN(DI_FOO);
	PSB.bar_btn = PLC_IN(DI_BAR);
	PSB.frame_btn = PLC_IN(DI_FRAME);
	
	PLC_OUT(PCB.outmap_pin, !PSB.pin_valve);
	PLC_OUT(PCB.outmap_frame, PSB.frame_valve);
	PLC_OUT(PCB.outmap_foo, PSB.foo_valve);
	PLC_OUT(PCB.outmap_bar, PSB.bar_valve);
	PLC_OUT(PCB.outmap_red, PSB.red_blub);
	PLC_OUT(PCB.outmap_green, PSB.green_blub);
	PLC_OUT(PCB.outmap_heater, PSB.heater_on);
	if( PCB.xpin_ctrl == 0 )
	{
		PLC_OUT(PCB.outmap_xpin, 0);
		PSB.pin_lock = PLC_IN(DI_LOCK);
		PSB.pin_release = PLC_IN(DI_RELEASE);
	}
	else
	{
		PLC_OUT(PCB.outmap_xpin, !PSB.pin_valve);
		PSB.pin_lock = PLC_IN(DI_LOCK) & (!(PLC_IN(DI_LOCK) ^ PLC_IN(DI_LOCK2)));
		PSB.pin_release = PLC_IN(DI_RELEASE) & (!(PLC_IN(DI_RELEASE) ^ PLC_IN(DI_RELEASE2)));
	}
	PSB.frame_high = PLC_IN(DI_HIGH);
	PSB.frame_low = PLC_IN(DI_LOW);
	PSB.custom_in_1 = PLC_IN(DI_C1);
	PSB.custom_in_2 = PLC_IN(DI_C2);
	PSB.custom_in_3 = PLC_IN(DI_C3);
	PSB.custom_in_4 = PLC_IN(DI_C4);
}

void main_control_loop(void *p)
{
	static uint8_t State;
	static uint16_t HeaterTimer, DigitalTimer;
	
	switch(State)
	{
	   case PLC_INIT:
			if( LinkHealth == 1 && PCB.PMC_abnormal == 0 && 
				(PCB.PMC_alive == 1 || PCB.stroke_moves > 16) )
			{
				PSB.clear_alarm = 0;
				PSB.servo_off = 1;
				PSB.alarm &= ~ALARM_PARTNER_ABNORMAL;
				State = PLC_OFFLINE;
			}
			PSB.pin_valve = 0;
			blub_control(BLUB_ALL_ON);
			break;
	   
	   case PLC_OFFLINE:
			if( LinkHealth == 0 || PCB.PMC_abnormal == 1 || 
				(PCB.PMC_alive == 0 && PCB.stroke_moves <= 16) )
			{
				PSB.alarm |= ALARM_PARTNER_ABNORMAL;
				State = PLC_ALARM;
			}
			else
			{
				if( BDB.sys_run_ctrl == 1 )
				{
					State = PLC_OPERATION;
				}
				else
				{
					if( PSB.online_btn == 1 )
					{
						PSB.servo_off = 0;
						State = PLC_ONLINE;
					}
					else
					{
						offline_service();
					}
				}
			}
			if( BDB.cart_set_ctrl == 1 ) PSB.pin_valve = 1;
			if( BDB.cart_free_ctrl == 1 ) PSB.pin_valve = 0;
			if( PSB.sys_pause_req == 0 ) blub_control(BLUB_RED_ON);
			else blub_control(BLUB_GREEN_RED_BLINK);
			break;
	   
	   case PLC_ONLINE:
			if( LinkHealth == 0 || PCB.PMC_abnormal == 1 || 
				(PCB.PMC_alive == 0 && PCB.stroke_moves <= 16) )
			{
				PSB.alarm |= ALARM_PARTNER_ABNORMAL;
				State = PLC_ALARM;
			}
			else
			{
				if( BDB.sys_run_ctrl == 1 )
				{
					State = PLC_OPERATION;
				}
				else
				{
					if( PSB.online_btn == 0 )
					{
						PSB.op_complete = 0;
						State = PLC_OFFLINE;
					}
					else
					{
						ttlStroke = (uint16_t)PCB.stroke_timeout*(1000/MAIN_LOOP_MS);
						online_service();
					}
				}
			}
			if( BDB.cart_set_ctrl == 1 ) PSB.pin_valve = 1;
			if( BDB.cart_free_ctrl == 1 ) PSB.pin_valve = 0;
			if( PSB.sys_pause_req == 0 ) blub_control(BLUB_GREEN_ON);
			else blub_control(BLUB_GREEN_RED_BLINK);
			break;
	   
	   case PLC_OPERATION:
			if( LinkHealth == 0 || PCB.PMC_abnormal == 1 || 
				(PCB.PMC_alive == 0 && PCB.stroke_moves <= 16) )
			{
				PSB.alarm |= ALARM_PARTNER_ABNORMAL;
				State = PLC_ALARM;
			}
			else
			{
				if( BDB.sys_run_ctrl == 1 )
				{
					if( PSB.pin_valve == 1 && PSB.pin_lock == 1 && 
						PSB.online_btn == 1 && PSB.op_complete == 0 &&
						(BDB.op_start_ctrl & (1<<(StationID-1))) != 0 )
					{
						if( PCB.stroke_moves > 16 )
						{
							if( flocking_operation_service() == FOP_END )
							{
								if( /*BDB.sys_stop_ctrl == 1 ||*/ PSB.alarm != 0 )
								{
									State = PLC_ALARM;
								}
								else
								{
									PSB.op_complete = 1;
								}
							}
						}
						else
						{
							if( operation_service() == OP_END )
							{
								if( /*BDB.sys_stop_ctrl == 1 ||*/ PSB.alarm != 0 )
								{
									State = PLC_ALARM;
								}
								else
								{
									PSB.op_complete = 1;
								}
							}
						}
						ttlStroke = (uint16_t)PCB.stroke_timeout*(1000/MAIN_LOOP_MS);
					}
					if( BDB.cart_set_ctrl == 1 )
					{
						if( PCB.heater_ctrl == 1 )
						{
							if( HeaterTimer > 0 )
							{
								PSB.heater_on = 1;
								HeaterTimer--;
							}
							else
							{
								PSB.heater_on = 0;
							}
						}
						PSB.pin_valve = 1;
						
						if( PCB.xpin_ctrl == 1 )
						{
							if( DigitalTimer > 0 )
							{
								PSB.digital_wait = 1;
								DigitalTimer--;
							}
							else
							{
								PSB.digital_wait = 0;
							}
						}
					}
					if( BDB.cart_free_ctrl == 1 )
					{
						if( PSB.op_complete == 1 )
						{
							HeaterTimer = PCB.heater_delay;
							HeaterTimer *= 100;
						}
						if( PCB.heater_ctrl == 1 )
						{
							if( HeaterTimer > 0 && PSB.heater_on == 0 )
							{
								PSB.heater_on = 0;
								HeaterTimer--;
							}
							else
							{
								HeaterTimer = PCB.heating_time;
								HeaterTimer *= 1000;
								PSB.heater_on = 1;
							}
						}
						else
						{
							PSB.heater_on = 0;
						}
						if( PCB.xpin_ctrl == 1 )
						{
							DigitalTimer = PCB.digital_time;
							DigitalTimer *= 1000;
						}
						OperationMoves = 0;
						PSB.pin_valve = 0;
						PSB.frame_valve = 0;
						PSB.op_complete = 0;
					}
					if( PSB.online_btn == 0 )
					{
						PSB.clear_alarm = PSB.stroke_btn;
					}
				}
				else
				{
					if( PSB.online_btn == 1 )
					{
						State = PLC_ONLINE;
					}
					else
					{
						State = PLC_OFFLINE;
					}
					PSB.heater_on = 0;
					PSB.fwd_mov_req = 0;
					PSB.bwd_mov_req = 0;
				}
				PSB.servo_off = !(PSB.online_btn);
			}
			blub_control(BLUB_GREEN_BLINK);
			break;
	   
	   case PLC_ALARM:
			if( BDB.alarm_recover == 1 || PSB.online_btn == 0 )
			{
				PSB.alarm &= ~ALARM_FRAME_ERROR;
				PSB.alarm &= ~ALARM_STROKE_TIMEOUT;
				PSB.alarm &= ~ALARM_POSITION_ERROR;
				PSB.clear_alarm = 1;
				PSB.servo_off = 1;
				State = PLC_INIT;
			}
			PSB.fwd_mov_req = 0;
			PSB.bwd_mov_req = 0;
			PSB.heater_on = 0;
			PSB.pin_valve = 0;
			PSB.frame_valve = 0;
			if( PCB.move_complete == 1 )
			{
				//PSB.foo_valve = 0;
				//PSB.bar_valve = 0;
			}
			blub_control(BLUB_RED_BLINK);
			break;
	   
		default:
			State = PLC_INIT;
			break;
	}
	io_service();
}

void ttl_update(void *p)
{
	static uint32_t LastTick=0;
	LinkHealth = ( LutosGetCurrentTicks() - LastTick > TTL_PLC ) ? 0 : 1;
	LastTick = LutosGetCurrentTicks();
	LutosTickEventResume(CommTimeoutTick);
}

void ttl_service(void *p)
{
	LinkHealth = 0;
}

void plc301_init(uint8_t id)
{
	TASK *pTask;
	StationID = id;
	PCB.outmap_pin = DO_PIN;
	PCB.outmap_frame = DO_FRAME;
	PCB.outmap_foo = DO_FOO;
	PCB.outmap_bar = DO_BAR;
	PCB.outmap_red = DO_RED;
	PCB.outmap_green = DO_GREEN;
	PCB.outmap_heater = DO_HEAT;
	PCB.outmap_xpin = DO_PIN2;
	B2bBusSlaveInit(3, 200000, StationID, 8, 3, 25);
	pTask = LutosTaskCreate( ttl_update, "TTL1", NULL, PRIORITY_NON_REALTIME );
	B2bBusRegister(B2B_TYPE_WRITE, 0, (uint8_t *)&BDB, sizeof(broadcast_data_block_t), NULL);
	B2bBusRegister(B2B_TYPE_READ, 10, (uint8_t *)&PSB, sizeof(plc_status_block_t), NULL);
	B2bBusRegister(B2B_TYPE_WRITE, 20, (uint8_t *)&PCB, SDB_PLC_WR_LEN, pTask);
	B2bBusRegister(B2B_TYPE_WRITE, 50, ((uint8_t *)(&PCB))+SDB_PLC_WR_LEN, SDB_PLC_W2_LEN, NULL);
	LutosTickEventCreate(
		MAIN_LOOP_MS,	// tick period => 1 ms
		0,
		LutosTaskCreate( main_control_loop, "MAIN", NULL, PRIORITY_NON_REALTIME )
	);
	CommTimeoutTick = LutosTickEventCreate(
		0,
		TTL_PLC,
		LutosTaskCreate( ttl_service, "TTL2", NULL, PRIORITY_NON_REALTIME )
	);
	PSB.servo_off = 1;
}


#include <LPC17xx.h>
#include "lutos.h"
#include "task.h"
#include "tick.h"
#include "gpio.h"
#include "s24_eeprom.h"
#include "ethernet.h"
#include "b2b_s.h"

#define BUF1_SIZE	10
#define BUF2_SIZE	100
#define BUF3_SIZE	100
#define B2B_ADDR1	0
#define B2B_ADDR2	(B2B_ADDR1+BUF1_SIZE)
#define B2B_ADDR3	(B2B_ADDR2+BUF2_SIZE)

const char *cstrVersion[] = {
	"FIRMWARE VERSION",
	"1.0.0"
};

int8u buf1[BUF1_SIZE], buf2[BUF2_SIZE], buf3[BUF3_SIZE];

void buf1_owner(void *p)
{
	buf3[1] = buf1[0];
}

void buf2_owner(void *p)
{
	buf3[2] = buf2[0];
}

void buf3_owner(void *p)
{
}

int main(void)
{
	SystemCoreClockUpdate();
	
	s24_eeprom_init();
	eeprom_busy_wait();
	
	ethernet_init();
	B2bBusUDPSlaveInit();
	
	B2bBusRegister(B2B_TYPE_WRITE, B2B_ADDR1, buf1, BUF1_SIZE, 
		LutosTaskCreate(buf1_owner, "BUF1", 0, PRIORITY_REALTIME)
	);
	B2bBusRegister(B2B_TYPE_WRITE, B2B_ADDR2, buf2, BUF2_SIZE, 
		LutosTaskCreate(buf2_owner, "BUF2", 0, PRIORITY_NON_REALTIME)
	);
	B2bBusRegister(B2B_TYPE_READ, B2B_ADDR3, buf3, BUF3_SIZE, 
		LutosTaskCreate(buf3_owner, "BUF3", 0, PRIORITY_NON_REALTIME)
	);
	
	buf3[0] = 0xBB;
	
	LutosTaskStartScheduler();
}

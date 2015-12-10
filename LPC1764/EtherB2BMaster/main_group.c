#include <LPC17xx.h>
#include "lutos.h"
#include "task.h"
#include "tick.h"
#include "gpio.h"
#include "s24_eeprom.h"
#include "ethernet.h"
#include "b2b_m.h"
#include "lwip/opt.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/udp.h"

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

B2B_GROUP *TestGrp;
B2B_CHANNEL *hB2bCh1, *hB2bCh2, *hB2bCh3;
int8u buf1[BUF1_SIZE], buf2[BUF2_SIZE], buf3[BUF3_SIZE];
int32u cycles, cycles_per_sec;

void group_owner(void *p)
{
	cycles++;
}

void CycleCalc(void *p)
{
	cycles_per_sec = cycles;
	cycles = 0;
}

int main(void)
{
	SystemCoreClockUpdate();
	
	s24_eeprom_init();
	eeprom_busy_wait();
	
	ethernet_init();
	
	B2bBusUDPMasterInit( netif_default->ip_addr.addr, 3 );
	
	hB2bCh1 = B2bBusChannelCreate( B2B_BROADCAST_ID, B2B_ADDR1, buf1, BUF1_SIZE, B2B_DIRECTION_OUT, NULL);
	hB2bCh2 = B2bBusChannelCreate( 10, B2B_ADDR2, buf2, BUF2_SIZE, B2B_DIRECTION_OUT, NULL);
	hB2bCh3 = B2bBusChannelCreate( 10, B2B_ADDR3, buf3, BUF3_SIZE, B2B_DIRECTION_IN, NULL);
	buf1[0] = 0xAA;
	buf2[0] = 0xCC;
	
	TestGrp = B2bBusGroupCreate(3, B2B_GROUP_TYPE_LOOP, B2bBusUDPChannelSync);
	B2bBusGroupAddChannel(TestGrp, hB2bCh1);
	B2bBusGroupAddChannel(TestGrp, hB2bCh2);
	B2bBusGroupAddChannel(TestGrp, hB2bCh3);
	B2bBusGroupSetCompleteTask(TestGrp, NR_TASK(group_owner,0));
	B2bBusGroupSync(TestGrp);
	
	LUTOS_CREATE_TICK_EVENT_TASK(1000,0,CycleCalc,0,PRIORITY_NON_REALTIME);
	
	LutosTaskStartScheduler();
}

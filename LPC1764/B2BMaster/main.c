#include <LPC17xx.h>
#include "lutos.h"
#include "tick.h"
#include "b2b.h"
#include "b2b_m.h"

// Tick Time Synchronization
uint32_t TimeSyncBuffer[2];
B2B_CHANNEL *hB2bTimeSync;

void SystemTimeSync(void *p)
{
	TimeSyncBuffer[0] = LutosGetCurrentTicks();
	TimeSyncBuffer[1] = 0;
	B2bBusChannelSync(hB2bTimeSync);
}

int main(void)
{
	SystemCoreClockUpdate();
	B2bBusMasterInit( 3, 115200, 255, 0 );
	
	// Tick Time Synchronization
	hB2bTimeSync = B2bBusChannelCreate(
		0, 100, 
		(uint8_t *)TimeSyncBuffer, 8, 
		B2B_DIRECTION_OUT, NULL
	);
	LUTOS_CREATE_TICK_EVENT_TASK(1000, 500, SystemTimeSync, NULL, PRIORITY_REALTIME);
	
	LutosTaskStartScheduler();
}

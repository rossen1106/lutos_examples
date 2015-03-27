#include <LPC17xx.h>
#include "lutos.h"
#include "b2b.h"
#include "b2b_s.h"

// Tick Time Synchronization
uint32_t TimeSyncBuffer[2];

void SlaveSystemTimeSync(void *p)
{
	LutosUnitTickSynchronize(TimeSyncBuffer[0], TimeSyncBuffer[1]);
}

int main(void)
{
	SystemCoreClockUpdate();
	B2bBusSlaveInit( 3, 115200, 1, 10, 255, 0 );
	
	// Tick Time Synchronization
	B2bBusRegister( B2B_TYPE_WRITE, 100, (uint8_t *)TimeSyncBuffer, 8,
		LutosTaskCreate(
			SlaveSystemTimeSync, "", NULL, PRIORITY_SOFT_REALTIME
		)
	);
	LutosTaskStartScheduler();
}

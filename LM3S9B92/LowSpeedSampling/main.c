#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "lutos.h"
#include "task.h"
#include "tick.h"

TASK *SampTask;

int16s ADConvert(void)
{
	int16s dummy=0;
	return dummy;
}

void SingleSampleSend(int16s data)
{
}

void LowSpeedSampling(void *p)
{
	int16s result;
	result = ADConvert();
	SingleSampleSend(result);
}

int main(void)
{
	//MAP_SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
    //                   SYSCTL_XTAL_16MHZ);	// for LM3S9B92
	MAP_SysCtlClockSet(SYSCTL_SYSDIV_3 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                       SYSCTL_XTAL_8MHZ);	// for LM3S6965

	SampTask = LutosTaskCreate(
		LowSpeedSampling,
		"LowSamp",
		NULL,
		PRIORITY_HARD_REALTIME);
	LutosTickEventCreate(
		10, //PERIOD_10_MS,
		0,
		SampTask);
	LutosTaskStartScheduler();
}

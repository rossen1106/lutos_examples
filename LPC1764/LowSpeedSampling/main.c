#include <LPC17xx.h>
#include "lutos.h"
#include "task.h"
#include "tick.h"
#include "debug.h"

TASK *SampTask;

int16s ADConvert(void)
{
	int16s dummy=0;
	return dummy;
}

void SingleSampleSend(int16s data)
{
	#if DEBUGINFO_ENABLE==1
	LutosDebugOutput(DEBUG_MESSAGE_USER, "Sample is sending...", 0);
	#endif
}

void LowSpeedSampling(void *p)
{
	int16s result;
	result = ADConvert();
	SingleSampleSend(result);
}

int main(void)
{
	SystemCoreClockUpdate();
	
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

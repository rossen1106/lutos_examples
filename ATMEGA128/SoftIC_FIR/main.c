#include "lutos.h"
#include "task.h"
#include "tick.h"
#include "debug.h"
#include "ic_fir_filter.h"

TASK *SampTask;

typedef struct
{
	TypeOfFIRFilter_str FIRFilter1_str;
	float u_result;
	float y;
} TypeOfSystem_str;

TypeOfSystem_str System_str;

int16s ADConvert(void)
{
	int16s dummy=0;
	return dummy;
}

void SingleSampleSend(TypeOfSystem_str *str_p)
{
	#if DEBUGINFO_ENABLE==1
	LutosDebugOutput(DEBUG_MESSAGE_NORMAL, CONST_STR("Sample is sending..."), 0);
	#endif
}

void LowSpeedSampling(void *p)
{
	System_str.u_result = ADConvert();
	LutosTaskTrigger( System_str.FIRFilter1_str.Job_p );
}

int main(void)
{
	SampTask = LutosTaskCreate(
		LowSpeedSampling,
		"LowSamp",
		NULL,
		PRIORITY_REALTIME);
	LutosTickEventCreate(
		10, //PERIOD_10_MS,
		0,
		SampTask);
	
	FIRFilter_ini( &(System_str.FIRFilter1_str) );
	System_str.FIRFilter1_str.NextJob_p[0] = LutosTaskCreate((LUTOS_CALL)SingleSampleSend, "OK", &System_str, PRIORITY_NON_REALTIME);
	System_str.FIRFilter1_str.InFloat_p[0] = &(System_str.u_result);
	System_str.FIRFilter1_str.OutFloat_p[0] = &(System_str.y);
	FIRFilter_ini( &(System_str.FIRFilter1_str) );
	
	LutosTaskStartScheduler();
}

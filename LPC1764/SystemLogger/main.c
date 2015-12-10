/**
 * @file main.c
 * @brief System Logger Demo Program
 *
 * Demostrate the functionailty of system logger.
 *
 * Demo Tasks & Ticks:
 *     Non realtime task chain: JobA -> JobB -> JobC -> JobA (loop)
 *     Non realtime tick: JobH (every 100 ms)
 *     Realtime task chain: JobD -> JobE -> JobF
 *     Realtime tick: JobG (every 10 ms)
 *     Target task of log: JobF
 *
 * System logger can record the sequence of task executions before the target task.
 * 
 * How to inspect it? (method 1):
 * With Keil's simulator, user can open the "UART#1" serial window in debug session 
 * and observe the output of logger via debug messages.
 *
 * How to inspect it? (method 2):
 * With Keil's simulator, user can set a breakpoint inside the DumpMyLog() function.
 * When program runs to the breakpoint, user can inspect the content of LogResult in
 * Watch window. (remember to add watch first)
 *
 * @author Li-Yeh Liu <rosssen@viewmove.com>
 * @version 1.0.0
 * @date 2015/04/15
 */

#include <LPC17xx.h>
#include "lutos.h"
#include "task.h"
#include "tick.h"
#include "debug.h"
#include "ring.h"
#include "queue.h"

TASK *hJobA, *hJobB, *hJobC, *hJobD, *hJobE, *hJobF, *hJobG, *hJobH;
task_log_t LogResult[8];
char logbuf[128];

QUEUE *hTestQueue;
int32u QueueDepth, TestData[6];
int32u QueueCycle=0, QueueError=0;

void DumpMyLog(void *p)
{
	int8u i;
	RING *hRing = p;
	LutosRingRead(hRing, LogResult, 8);
	/* set breakpoint after this line to inspect the content of LogResult */
	LutosDebugOutput(DEBUG_MESSAGE_USER, "Task    Trig    Run     Clks", 0);
	for(i=0; i<8; i++)
	{
		sprintf(logbuf, "%8s%8d%8d%8d",
			LogResult[i].hLogTask->cstrName,
			LogResult[i].TriggerTick,
			LogResult[i].RunStartTick,
			LogResult[i].CostClocks
		);
		LutosDebugOutput(DEBUG_MESSAGE_USER, logbuf, 0);
	}
}

void DummyWork(int32u Length)
{
	int32u i;
	for(i=0; i<Length; i++);
}

void JobA_StepFunction(void *p)
{
	LutosQueuePush(hTestQueue, TestData);
	//DummyWork(100);
	LutosTaskTrigger(hJobB);
}

void JobB_StepFunction(void *p)
{
	if( hTestQueue->Count > 1 )
	{
		LutosQueuePop(hTestQueue, TestData);
		QueueCycle++;
		
		if( hTestQueue->Head >= hTestQueue->Tail )
		{
			QueueDepth = (int32u)hTestQueue->Head - (int32u)hTestQueue->Tail;
			QueueDepth /= hTestQueue->ItemSize;
			
			if( QueueDepth != hTestQueue->Count )
			{
				QueueError++;
			}
		}
		else
		{
			QueueDepth = (int32u)hTestQueue->Head - (int32u)hTestQueue->Base;
			QueueDepth += (int32u)hTestQueue->Top - (int32u)hTestQueue->Tail;
			QueueDepth /= hTestQueue->ItemSize;
			
			if( QueueDepth != hTestQueue->Count )
			{
				QueueError++;
			}
		}
	}
	//DummyWork(200);
	LutosTaskTrigger(hJobC);
}

void JobC_StepFunction(void *p)
{
	DummyWork(300);
	LutosTaskTrigger(hJobA);
}

void JobD_StepFunction(void *p)
{
	DummyWork(1000);
	LutosTaskTrigger(hJobE);
}

void JobE_StepFunction(void *p)
{
	DummyWork(500);
	LutosTaskTrigger(hJobF);
}

void JobF_StepFunction(void *p)
{
	DummyWork(400);
}

void JobG_StepFunction(void *p)
{
	DummyWork(150);
}

void JobH_StepFunction(void *p)
{
	DummyWork(800);
	LutosTaskTrigger(hJobD);
}

int main(void)
{
	SystemCoreClockUpdate();
	
	hTestQueue = LutosQueueCreate( sizeof(TestData), 10 );
	hJobA = LutosTaskCreate(JobA_StepFunction, "JOB A", NULL, PRIORITY_NON_REALTIME);
	hJobB = LutosTaskCreate(JobB_StepFunction, "JOB B", NULL, PRIORITY_NON_REALTIME);
	hJobC = LutosTaskCreate(JobC_StepFunction, "JOB C", NULL, PRIORITY_NON_REALTIME);
	hJobD = LutosTaskCreate(JobD_StepFunction, "JOB D", NULL, PRIORITY_REALTIME);
	hJobE = LutosTaskCreate(JobE_StepFunction, "JOB E", NULL, PRIORITY_REALTIME);
	hJobF = LutosTaskCreate(JobF_StepFunction, "JOB F", NULL, PRIORITY_REALTIME);
	hJobG = LutosTaskCreate(JobG_StepFunction, "JOB G", NULL, PRIORITY_REALTIME);
	hJobH = LutosTaskCreate(JobH_StepFunction, "JOB H", NULL, PRIORITY_NON_REALTIME);
	LutosTickEventCreate(10, 0, hJobG);
	LutosTickEventCreate(100, 10, hJobH);
	LutosTaskTrigger(hJobA);
	LutosTaskLogger(hJobF, DumpMyLog, 8);
	
	LutosTaskStartScheduler();
}

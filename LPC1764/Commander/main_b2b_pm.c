#include <LPC17xx.h>
#include "lutos.h"
#include "b2b.h"
#include "b2b_s.h"
#include "flash_nvol.h"
#include "commander.h"

uint8_t BlockID, ParamD8 = 32;
int16_t ParamD16 = -10;
uint32_t ParamD32 = 500000;
double ParamD64 = 0.5;

#define COMMANDER_BUFFER_SIZE		200
#define COMMANDER_COMMAND_ADDRESS	5000
#define COMMANDER_RESPONSE_ADDRESS	5200

// Commander B2B Buffers
uint8_t SscCommandBuffer[COMMANDER_BUFFER_SIZE];
uint8_t SscResponseBuffer[COMMANDER_BUFFER_SIZE];

// Tick Time Synchronization
uint32_t TimeSyncBuffer[2];

uint16_t CommanderGlueRead(uint32_t address, void *dst, uint16_t size)
{
	return NVOL_GetVariable(address>>3, dst, size);
}

uint16_t CommanderGlueWrite(uint32_t address, void *dst, uint16_t size)
{
	return NVOL_SetVariable(address>>3, dst, size);
}

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
	
	NVOL_Init();
	
	SscInit(
		CommanderGlueRead,
		CommanderGlueWrite,
		1024,	// memory size (memory size / 8 = available variables)
		8,		// page size
		20, 	// page write delay (ms)
		10		// max blocks
	);
	
	B2bBusRegister( 
		B2B_TYPE_WRITE, 
		COMMANDER_COMMAND_ADDRESS, 
		(uint8_t *)SscCommandBuffer, 
		COMMANDER_BUFFER_SIZE,
		SscCommandTaskInit(
			"SSC TEST",
			"1.0.0",
			NULL,
			SscCommandBuffer,
			SscResponseBuffer,
			COMMANDER_BUFFER_SIZE
		)
	);
	
	B2bBusRegister( 
		B2B_TYPE_READ, 
		COMMANDER_RESPONSE_ADDRESS, 
		(uint8_t *)SscResponseBuffer, 
		COMMANDER_BUFFER_SIZE, 
		NULL
	);
	
	BlockID = SscRegisterBlock("TST1", NULL);
	VAR( BlockID, 0, ParamD8 );
	VAR_AUTO( BlockID, 1, ParamD16 );
	VAR( BlockID, 2, ParamD32 );
	VAR( BlockID, 3, ParamD64 );
	
	LutosTaskStartScheduler();
}

#include <LPC17xx.h>
#include "lutos.h"
#include "task.h"
#include "multidrop.h"
#include "s24_eeprom.h"
#include "commander.h"

TASK *hSscCmdTask;
multidrop_t *hDrop0;
uint8_t BlockID, ParamD8 = 32;
int16_t ParamD16 = -10;
uint32_t ParamD32 = 500000;
double ParamD64 = 0.5;

uint16_t CommanderGlueRead(uint32_t address, void *dst, uint16_t size)
{
	return eeprom_read_block(address, dst, size);
}

uint16_t CommanderGlueWrite(uint32_t address, void *dst, uint16_t size)
{
	return eeprom_write_block(address, dst, size);
}

void CommanderGlueReceive(void *p)
{
	uint16_t i;
	for(i=hDrop0->Length; i<MAX_PACKET_LENGTH; i++) hDrop0->Message[i] = 0;
	LutosTaskTrigger(hSscCmdTask);
}

void CommanderGlueSend(void)
{
	MultidropSend(hDrop0);
}

int main(void)
{
	SystemCoreClockUpdate();
	
	hDrop0 = MultidropInit(0, 115200, 0, 
		LutosTaskCreate(CommanderGlueReceive, "U0RX", NULL, PRIORITY_HARD_REALTIME)
	);
	
	s24_eeprom_init();
	eeprom_busy_wait();
	
	SscInit(
		CommanderGlueRead,
		CommanderGlueWrite,
		1024,	// memory size (memory size / 8 = available variables)
		32,		// page size
		10, 	// page write delay (ms)
		10		// max blocks
	);
	
	hSscCmdTask = SscCommandTaskInit(
		"SSC TEST",
		"1.0.0",
		CommanderGlueSend,
		hDrop0->Message,
		hDrop0->Message,
		MAX_PACKET_LENGTH
	);
	
	BlockID = SscRegisterBlock("TST1", hDrop0);
	//SscRegisterVariable(BlockID, 0, SSC_VAR_TYPE_8_UNSIGNED, &ParamD8, SSC_VAR_PROPERTY_SAVE_ON_DEMAND);
	//SscRegisterVariable(BlockID, 1, SSC_VAR_TYPE_16_SIGNED, &ParamD16, SSC_VAR_PROPERTY_SAVE_ON_DEMAND);
	//SscRegisterVariable(BlockID, 2, SSC_VAR_TYPE_32_UNSIGNED, &ParamD32, SSC_VAR_PROPERTY_SAVE_ON_DEMAND);
	//SscRegisterVariable(BlockID, 3, SSC_VAR_TYPE_64_DOUBLE, &ParamD64, SSC_VAR_PROPERTY_SAVE_ON_DEMAND);
	VAR( BlockID, 0, ParamD8 );
	VAR_AUTO( BlockID, 1, ParamD16 );
	VAR( BlockID, 2, ParamD32 );
	VAR( BlockID, 3, ParamD64 );
	
	LutosTaskStartScheduler();
}

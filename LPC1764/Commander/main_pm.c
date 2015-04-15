#include <LPC17xx.h>
#include "lutos.h"
#include "task.h"
#include "multidrop.h"
#include "flash_nvol.h"
#include "commander.h"

TASK *hSscCmdTask;
multidrop_t *hDrop0;
uint8_t BlockID, ParamD8 = 32;
uint8_t BlockID2, BlockID3, BlockID4;
int16_t ParamD16 = -10;
uint32_t ParamD32 = 500000;

char a = 12;
unsigned char b = 14;
int c = -256;
unsigned int d = 455;
short int e = -36;
unsigned short int f = 50;
long long int g = -456;
unsigned long int h = 700;
float i = 1.23;
double j = 4.569;

double test[3][3] = {1.1,2.2,3.3,
4.4,5.5,6.6,
7.7,8.8,9.9};
short int test2[3][3] = {1,2,3,
4,5,6,
7,8,9};

char test3[40],x;
double ParamD64 = 0.5;
 int temp;

uint16_t CommanderGlueRead(uint32_t address, void *dst, uint16_t size)
{
	return NVOL_GetVariable(address>>3, dst, size);
}

uint16_t CommanderGlueWrite(uint32_t address, void *dst, uint16_t size)
{
	return NVOL_SetVariable(address>>3, dst, size);
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
		LutosTaskCreate(CommanderGlueReceive, "U0RX", NULL, PRIORITY_NON_REALTIME)
	);
	
	NVOL_Init();
	
	SscInit(
		CommanderGlueRead,
		CommanderGlueWrite,
		1024,	// memory size (memory size / 8 = available variables)
		8,		// page size
		20		// page write delay (ms)
	);
	
	hSscCmdTask = SscCommandTaskInit(
		"SSC TEST",
		"1.0.0",
		CommanderGlueSend,
		hDrop0->Message,
		hDrop0->Message,
		MAX_PACKET_LENGTH
	);
	
	temp = sizeof(g);
	temp = (sizeof(j)<<4)+2;
	
	BlockID = SscRegisterBlock("TST1", hDrop0);
	BlockID2 = SscRegisterBlock("TST2", 0);
	BlockID3 = SscRegisterBlock("TST3", 0);
	BlockID4 = SscRegisterBlock("TST4", 0);
	
	VAR_U( BlockID, 0, ParamD8 );
	VAR_AUTO( BlockID, 1, ParamD16 );
	VAR_U( BlockID, 2, ParamD32 );
	VAR_F( BlockID, 3, ParamD64 );
	
	/*SscRegisterVariable(BlockID2, 0, (sizeof(a)<<4), &a, SSC_VAR_PROPERTY_SAVE_ON_DEMAND);
	SscRegisterVariable(BlockID2, 1, (sizeof(b)<<4)+1, &b, SSC_VAR_PROPERTY_SAVE_ON_DEMAND);
	SscRegisterVariable(BlockID2, 2, (sizeof(c)<<4), &c, SSC_VAR_PROPERTY_SAVE_ON_DEMAND);
	SscRegisterVariable(BlockID2, 3, (sizeof(d)<<4)+1, &d, SSC_VAR_PROPERTY_SAVE_ON_DEMAND);
	SscRegisterVariable(BlockID2, 4, (sizeof(e)<<4), &e, SSC_VAR_PROPERTY_SAVE_ON_DEMAND);
	SscRegisterVariable(BlockID2, 5, (sizeof(f)<<4)+1, &f, SSC_VAR_PROPERTY_SAVE_ON_DEMAND);
	SscRegisterVariable(BlockID2, 6, (sizeof(g)<<4), &g, SSC_VAR_PROPERTY_SAVE_ON_DEMAND);
	SscRegisterVariable(BlockID2, 7, (sizeof(h)<<4)+1, &h, SSC_VAR_PROPERTY_SAVE_ON_DEMAND);
	SscRegisterVariable(BlockID2, 8, (sizeof(i)<<4)+2, &i, SSC_VAR_PROPERTY_SAVE_ON_DEMAND);
	SscRegisterVariable(BlockID2, 9, (sizeof(j)<<4)+2, &j, SSC_VAR_PROPERTY_SAVE_ON_DEMAND);*/
	//SscRegisterVariable( BlockID2, 9, SSC_VAR_TYPE_64_DOUBLE, &j, SSC_VAR_PROPERTY_SAVE_ON_DEMAND);
	VAR( BlockID2, 0, a );
	VAR_U( BlockID2, 1, b );
	VAR( BlockID2, 2, c );
	VAR_U( BlockID2, 3, d );
	VAR( BlockID2, 4, e );
	VAR_U( BlockID2, 5, f );
	VAR( BlockID2, 6, g );
	VAR_U( BlockID2, 7, h );
	VAR_F( BlockID2, 8, i );
	VAR_F( BlockID2, 9, j );
	
	/*SscRegisterVariable(BlockID3, 0, (sizeof(test[0][0])<<4)+2, &test[0][0], SSC_VAR_PROPERTY_SAVE_ON_DEMAND);
	SscRegisterVariable(BlockID3, 1, (sizeof(test[0][1])<<4)+2, &test[0][1], SSC_VAR_PROPERTY_SAVE_ON_DEMAND);
	SscRegisterVariable(BlockID3, 2, (sizeof(test[0][2])<<4)+2, &test[0][2], SSC_VAR_PROPERTY_SAVE_ON_DEMAND);
	SscRegisterVariable(BlockID3, 3, (sizeof(test[1][0])<<4)+2, &test[1][0], SSC_VAR_PROPERTY_SAVE_ON_DEMAND);
	SscRegisterVariable(BlockID3, 4, (sizeof(test[1][1])<<4)+2, &test[1][1], SSC_VAR_PROPERTY_SAVE_ON_DEMAND);
	SscRegisterVariable(BlockID3, 5, (sizeof(test[1][2])<<4)+2, &test[1][2], SSC_VAR_PROPERTY_SAVE_ON_DEMAND);
	SscRegisterVariable(BlockID3, 6, (sizeof(test[2][0])<<4)+2, &test[2][0], SSC_VAR_PROPERTY_SAVE_ON_DEMAND);
	SscRegisterVariable(BlockID3, 7, (sizeof(test[2][1])<<4)+2, &test[2][1], SSC_VAR_PROPERTY_SAVE_ON_DEMAND);
	SscRegisterVariable(BlockID3, 8, SSC_VAR_TYPE_64_DOUBLE, &test[2][2], SSC_VAR_PROPERTY_SAVE_ON_DEMAND);
	
	SscRegisterVariable(BlockID3, 9, (sizeof(test2[0][0])<<4), &test2[0][0], SSC_VAR_PROPERTY_SAVE_ON_DEMAND);
	SscRegisterVariable(BlockID3, 10, (sizeof(test2[0][1])<<4), &test2[0][1], SSC_VAR_PROPERTY_SAVE_ON_DEMAND);
	SscRegisterVariable(BlockID3, 11, (sizeof(test2[0][2])<<4), &test2[0][2], SSC_VAR_PROPERTY_SAVE_ON_DEMAND);
	SscRegisterVariable(BlockID3, 12, (sizeof(test2[1][0])<<4), &test2[1][0], SSC_VAR_PROPERTY_SAVE_ON_DEMAND);
	SscRegisterVariable(BlockID3, 13, (sizeof(test2[1][1])<<4), &test2[1][1], SSC_VAR_PROPERTY_SAVE_ON_DEMAND);
	SscRegisterVariable(BlockID3, 14, (sizeof(test2[1][2])<<4), &test2[1][2], SSC_VAR_PROPERTY_SAVE_ON_DEMAND);
	SscRegisterVariable(BlockID3, 15, (sizeof(test2[2][0])<<4), &test2[2][0], SSC_VAR_PROPERTY_SAVE_ON_DEMAND);
	SscRegisterVariable(BlockID3, 16, (sizeof(test2[2][1])<<4), &test2[2][1], SSC_VAR_PROPERTY_SAVE_ON_DEMAND);
	SscRegisterVariable(BlockID3, 17, (sizeof(test2[2][2])<<4), &test2[2][2], SSC_VAR_PROPERTY_SAVE_ON_DEMAND);*/
	VAR_F( BlockID3, 0, test[0][0] );
	VAR_F( BlockID3, 1, test[0][1] );
	VAR_F( BlockID3, 2, test[0][2] );
	VAR_F( BlockID3, 3, test[1][0] );
	VAR_F( BlockID3, 4, test[1][1] );
	VAR_F( BlockID3, 5, test[1][2] );
	VAR_F( BlockID3, 6, test[2][0] );
	VAR_F( BlockID3, 7, test[2][1] );
	VAR_F( BlockID3, 8, test[2][2] );
	VAR( BlockID3, 9, test2[0][0] );
	VAR( BlockID3, 10, test2[0][1] );
	VAR( BlockID3, 11, test2[0][2] );
	VAR( BlockID3, 12, test2[1][0] );
	VAR( BlockID3, 13, test2[1][1] );
	VAR( BlockID3, 14, test2[1][2] );
	VAR( BlockID3, 15, test2[2][0] );
	VAR( BlockID3, 16, test2[2][1] );
	VAR( BlockID3, 17, test2[2][2] );
	
	for(x=0; x<40; x++)
		VAR( BlockID4, x, test3[x] );

	LutosTaskStartScheduler();
}

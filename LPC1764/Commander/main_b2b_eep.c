#include <LPC17xx.h>
#include "lutos.h"
#include "b2b.h"
#include "b2b_s.h"
#include "s24_eeprom.h"
#include "commander.h"

uint8_t BlockID, ParamD8 = 32;
int16_t ParamD16 = -10;
uint32_t ParamD32 = 500000;
double ParamD64 = 0.5;
uint8_t BlockID2, BlockID3, BlockID4;
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

#define COMMANDER_BUFFER_SIZE		200
#define COMMANDER_COMMAND_ADDRESS	5000
#define COMMANDER_RESPONSE_ADDRESS	5200

// Commander B2B Buffers
uint8_t SscCommandBuffer[COMMANDER_BUFFER_SIZE];
uint8_t SscResponseBuffer[COMMANDER_BUFFER_SIZE];

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
	
	s24_eeprom_init();
	eeprom_busy_wait();
	
	SscInit(
		eeprom_read_block,
		eeprom_write_block,
		1024,	// memory size (memory size / 8 = available variables)
		32,		// page size
		10		// page write delay (ms)
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
	BlockID2 = SscRegisterBlock("TST2", 0);
	BlockID3 = SscRegisterBlock("TST3", 0);
	BlockID4 = SscRegisterBlock("TST4", 0);
	
	VAR_U( BlockID, 0, ParamD8 );
	VAR_AUTO( BlockID, 1, ParamD16 );
	VAR_U( BlockID, 2, ParamD32 );
	VAR_F( BlockID, 3, ParamD64 );
	
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
	
	for(x=0; x<40; x++) VAR( BlockID4, x, test3[x] );
		
	LutosTaskStartScheduler();
}

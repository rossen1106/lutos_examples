#include <LPC17xx.h>
#include "lutos.h"
#include "modbus.h"
#include "modbus_s.h"

int main(void)
{
	SystemCoreClockUpdate();
	ModbusRTUSlaveInit( 0, 115200, 1, 255, 0 );
	LutosTaskStartScheduler();
}

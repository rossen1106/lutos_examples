#include <LPC17xx.h>
#include "lutos.h"
#include "b2b.h"
#include "b2b_s.h"

int main(void)
{
	SystemCoreClockUpdate();
	B2bBusSlaveInit( 3, 115200, 1, 10, 255, 0 );
	LutosTaskStartScheduler();
}

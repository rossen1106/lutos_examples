#include <LPC17xx.h>
#include "lutos.h"
#include "b2b.h"
#include "b2b_m.h"

int main(void)
{
	SystemCoreClockUpdate();
	B2bBusMasterInit( 3, 115200, 255, 0 );
	LutosTaskStartScheduler();
}

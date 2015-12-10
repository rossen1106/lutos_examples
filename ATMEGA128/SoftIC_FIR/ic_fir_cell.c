#include "ic_fir_cell.h"

void FIRCell_step(TypeOfFIRCell_str *str_p)
{
	*(str_p->OutFloat_p[0]) = str_p->uk_delay;
	*(str_p->OutFloat_p[1]) = ( *(str_p->InFloat_p[0]) * *(str_p->InFloat_p[1]) ) + *(str_p->InFloat_p[2]);
	str_p->uk_delay = *(str_p->InFloat_p[1]);
	
	LutosTaskTrigger( str_p->NextJob_p[0] );
}

void FIRCell_ini(TypeOfFIRCell_str *str_p)
{
	if( str_p->Job_p == NULL )
	{
		str_p->Job_p = LutosTaskCreate((LUTOS_CALL)FIRCell_step, "FIR_CELL", str_p, PRIORITY_NON_REALTIME);
		str_p->NextJobNum = 1;
	}
}


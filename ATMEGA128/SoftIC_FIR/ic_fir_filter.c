#include "ic_fir_filter.h"

void FIRFilter_step(TypeOfFIRFilter_str *str_p)
{
	*(str_p->OutFloat_p[0]) = 0;
	LutosTaskTrigger( str_p->FilterCell[0].Job_p );
}

void FIRFilter_ini(TypeOfFIRFilter_str *str_p)
{
	unsigned char i;
	
	for(i=0; i<IC_FIR_FILTER_TAPS; i++)
	{
		FIRCell_ini( &(str_p->FilterCell[i]) );
	}
	
	if( str_p->Job_p == NULL )
	{
		str_p->Job_p = LutosTaskCreate((LUTOS_CALL)FIRFilter_step, "FIR_FILT", str_p, PRIORITY_NON_REALTIME);
		str_p->NextJobNum = 1;
	}
	
	for(i=0; i<IC_FIR_FILTER_TAPS; i++)
	{
		str_p->a[i] = (float)i/6;
		str_p->FilterCell[i].InFloat_p[0] = &(str_p->a[i]);
		str_p->FilterCell[i].InFloat_p[1] = str_p->InFloat_p[0];
		str_p->FilterCell[i].InFloat_p[2] = str_p->OutFloat_p[0];
		str_p->FilterCell[i].OutFloat_p[0] = str_p->InFloat_p[0];
		str_p->FilterCell[i].OutFloat_p[1] = str_p->OutFloat_p[0];
		str_p->FilterCell[i].NextJob_p[0] = str_p->FilterCell[i+1].Job_p;
	}
	str_p->FilterCell[IC_FIR_FILTER_TAPS-1].NextJob_p[0] = str_p->NextJob_p[0];
	
	for(i=0; i<IC_FIR_FILTER_TAPS; i++)
	{
		FIRCell_ini( &(str_p->FilterCell[i]) );
	}

}


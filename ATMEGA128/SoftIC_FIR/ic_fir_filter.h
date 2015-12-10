#include "lutos.h"
#include "task.h"
#include "ic_fir_cell.h"

#define IC_FIR_FILTER_TAPS		3

typedef struct
{
	TASK *Job_p;
	char NextJobNum;
	TASK *NextJob_p[1];
	TypeOfFIRCell_str FilterCell[IC_FIR_FILTER_TAPS];
	float a[IC_FIR_FILTER_TAPS];
	float u[IC_FIR_FILTER_TAPS];
	float *InFloat_p[1];	//[0]: u(k)
	float *OutFloat_p[1];	//[0]: y(k)
} TypeOfFIRFilter_str;

void FIRFilter_ini(TypeOfFIRFilter_str *str_p);
void FIRFilter_step(TypeOfFIRFilter_str *str_p);

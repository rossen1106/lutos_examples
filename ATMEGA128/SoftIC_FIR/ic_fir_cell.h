#include "lutos.h"
#include "task.h"

typedef struct
{
	TASK *Job_p;
	char NextJobNum;
	TASK *NextJob_p[1];
	float uk_delay;
	float *InFloat_p[3];	//[0]: a(n), [1]: u(k-n), [2]: v(n) = sum{ a(n-1)*u(k-n-1) }
	float *OutFloat_p[2];	//[0]: u(k-n), [1]: v(n) + a(n)*u(k-n)
} TypeOfFIRCell_str;

void FIRCell_ini(TypeOfFIRCell_str *str_p);
void FIRCell_step(TypeOfFIRCell_str *str_p);

#ifndef __IOT301_H
#define __IOT301_H

#include <LPC17xx.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PLC_OUT(n, v)	gpio_write(Output_ports[(n)], Output_pins[(n)], !(v))
#define PLC_IN(n)		gpio_read(Input_ports[(n)], Input_pins[(n)])

#define DI_ONLINE	0
#define DI_HIGH		1
#define DI_LOW		2
#define DI_RELEASE2	3
#define DI_LOCK2	4
#define DI_STROKE	5
#define DI_RELEASE	6
#define DI_LOCK		7
#define DI_FRAME	8
#define DI_REV		9
#define DI_FWD		10
#define DI_RUN		11
#define DI_PAUSE	12
#define DI_FOO		13
#define DI_BAR		14
#define DI_C1		16
#define DI_C2		17
#define DI_C3		18
#define DI_C4		19

#define DO_PIN2		0
#define DO_FOO		3
#define DO_BAR		4
#define DO_FRAME	5
#define DO_PIN		6
#define DO_HEAT		7
#define DO_GREEN	8
#define DO_RED		9
#define DO_C1		10
#define DO_C2		11

extern uint8_t Input_ports[];
extern uint8_t Input_pins[];
extern uint8_t Output_ports[];
extern uint8_t Output_pins[];

void io_table_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __IOT301_H */

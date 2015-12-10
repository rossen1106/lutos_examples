#ifndef __PLC301_H
#define __PLC301_H

#include <LPC17xx.h>
#include "gpio.h"
#include "lutos.h"
#include "task.h"
#include "tick.h"
#include "../spm.h"

#ifdef __cplusplus
extern "C" {
#endif

extern plc_status_block_t PSB;
extern plc_control_block_t PCB;
extern broadcast_data_block_t BDB;

void plc301_init(uint8_t id);

#ifdef __cplusplus
}
#endif

#endif /* __PLC301_H */

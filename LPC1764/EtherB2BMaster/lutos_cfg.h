/****************************************************************
 *																*
 *  		LUTOS Configuration File (2014/04/10)   			*
 *																*
 ****************************************************************/

/* General Options */
#define LUTOS_F_CPU										96000000UL
#define LUTOS_VIEW_ENABLE								1
#define LUTOS_MEMORY_ALIGNMENT							4
#define LUTOS_PUBLIC_BUFFER_SIZE						4096
#define LUTOS_WATCHDOG_TIMEOUT_MS						0

/* Options for Task Scheduler */
#define SCHEDULER_MAX_TASK_QUANTITY						20
#define SCHEDULER_MAX_NORMAL_QUEUE_LENGTH				5
#define SCHEDULER_MAX_REALTIME_QUEUE_LENGTH				5
#define SCHEDULER_USE_LOOKUP_TABLE_METHOD				1
#define SCHEDULER_STORE_TABLES_IN_ROM					1

/* Options for Debug Information */
#define DEBUGINFO_ENABLE								0
#define DEBUGINFO_OUTPUT_MESSAGE_LEVEL					0

/* Options for B2B Communication */
#define B2B_PACKET_TIMEOUT								5
#define B2B_MAX_PACKET_RETRY							3
#define B2B_SLAVE_RAPID_MODE							1

/* Options for T-BUS Communication */
#define TBUS_UART_PORT_NO								1

/* Options for MODBUS Communication */
#define MODBUS_SLAVE_DI_MAPS							1
#define MODBUS_SLAVE_DO_MAPS							1
#define MODBUS_SLAVE_AI_MAPS							2
#define MODBUS_SLAVE_AO_MAPS							2


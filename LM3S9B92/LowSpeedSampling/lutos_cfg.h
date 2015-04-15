/****************************************************************
 *																*
 *  		LUTOS Configuration File (2012/05/31)   			*
 *																*
 ****************************************************************/

/* General Options */
#define LUTOS_F_CPU										50000000UL
#define LUTOS_PUBLIC_BUFFER_SIZE						2048
#define LUTOS_MEMORY_ALIGNMENT							4
#define LUTOS_WATCHDOG_TIMEOUT_MS						500

/* Options for Task Scheduler */
#define SCHEDULER_MAX_TASK_QUANTITY						20
#define SCHEDULER_MAX_NORMAL_QUEUE_LENGTH				5
#define SCHEDULER_MAX_REALTIME_QUEUE_LENGTH				5
#define SCHEDULER_USE_LOOKUP_TABLE_METHOD				1
#define SCHEDULER_STORE_TABLES_IN_ROM					1

/* Options for Debug Information */
#define DEBUGINFO_ENABLE								1
#define DEBUGINFO_OUTPUT_MESSAGE_LEVEL					0

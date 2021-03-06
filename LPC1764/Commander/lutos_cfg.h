/****************************************************************
 *																*
 *  		LUTOS Configuration File (2012/05/31)   			*
 *																*
 ****************************************************************/

/* General Options */
#define LUTOS_F_CPU										100000000UL
#define LUTOS_PUBLIC_BUFFER_SIZE						4096
#define LUTOS_MEMORY_ALIGNMENT							4

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


/* Options for System Setup Commander  */
#define SSC_USE_REDUNDANCY								0
#define SSC_REDUNDANCY_OFFSET							1024
#define SSC_INSTANT_LOAD_DEFAULT						0

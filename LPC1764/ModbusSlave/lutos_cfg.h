/****************************************************************
 *																*
 *  		LUTOS Configuration File (2012/12/24)   			*
 *																*
 ****************************************************************/

/* General Options

	LUTOS_F_CPU								Core clock rate (Hz) of the system.
	LUTOS_MEMORY_ALIGNMENT					Usually 1 for 8-bit AVR/8051, 4 for 32-bit ARM/DSP.
	LUTOS_PUBLIC_BUFFER_SIZE				Buffer size in bytes (usually 60~80% of free RAM)
*/
#define LUTOS_F_CPU										100000000UL
#define LUTOS_MEMORY_ALIGNMENT							4
#define LUTOS_PUBLIC_BUFFER_SIZE						2048


/* Options for Task Scheduler 

	SCHEDULER_MAX_TASK_QUANTITY				Maximun amount of tasks the OS can create.(it costs RAM)
	SCHEDULER_MAX_NORMAL_QUEUE_LENGTH		Queue length of tasks with normal priority.(default=5)
	SCHEDULER_MAX_REALTIME_QUEUE_LENGTH		Queue length of tasks with realtime priority.(default=5)
	SCHEDULER_USE_LOOKUP_TABLE_METHOD		1=Table method, 0=Polling method.
	SCHEDULER_STORE_TABLES_IN_ROM			Scheduler use ether ROM (1) or RAM (0) for table method.
*/
#define SCHEDULER_MAX_TASK_QUANTITY						20
#define SCHEDULER_MAX_NORMAL_QUEUE_LENGTH				5
#define SCHEDULER_MAX_REALTIME_QUEUE_LENGTH				5
#define SCHEDULER_USE_LOOKUP_TABLE_METHOD				1
#define SCHEDULER_STORE_TABLES_IN_ROM					1


/* Options for Debug Information 

	DEBUGINFO_ENABLE						1=Enable, 0=Disable.
	DEBUGINFO_BAUDRATE						UART communication baudrate.(bps)
	DEBUGINFO_OUTPUT_MESSAGE_LEVEL			More messages 0 <==> 3 Less messages
	DEBUGINFO_BUFFER_LENGTH					Information buffer size in bytes.(costs public buffer)
	DEBUGINFO_TIME_TAG_DIGITS				Time tag digits in messages.(default=10)
*/
#define DEBUGINFO_ENABLE								0
#define DEBUGINFO_BAUDRATE								115200UL
#define DEBUGINFO_OUTPUT_MESSAGE_LEVEL					0
#define	DEBUGINFO_BUFFER_LENGTH							1024
#define DEBUGINFO_TIME_TAG_DIGITS						10


/* Options for B2B Communication 

	B2B_PACKET_TIMEOUT						Packet timeout (ms) for B2B Master.(default=5)
	B2B_MAX_PACKET_RETRY					Packet retries for B2B Master.(default=3)
	B2B_SLAVE_RAPID_MODE					1=Enable, 0=Disable.
*/
#define B2B_PACKET_TIMEOUT								5
#define B2B_MAX_PACKET_RETRY							3
#define B2B_SLAVE_RAPID_MODE							1


/* Options for MODBUS RTU Communication */
#define MODBUS_SLAVE_COIL_ACCESS						1
#define MODBUS_SLAVE_COIL_BYTES							8
#define MODBUS_SLAVE_DISCRETE_ACCESS					1
#define MODBUS_SLAVE_DISCRETE_BYTES						8
#define MODBUS_SLAVE_HOLDING_REGISTER_ACCESS			1
#define MODBUS_SLAVE_HOLDING_REGISTER_WORDS				16
#define MODBUS_SLAVE_INPUT_REGISTER_ACCESS				1
#define MODBUS_SLAVE_INPUT_REGISTER_WORDS				8

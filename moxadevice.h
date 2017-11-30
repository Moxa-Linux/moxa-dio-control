
/* 
 * This file is to define Moxa CPU embedded device private ioctrl command. 
 * It is include UART operating mode (RS232, RS422, RS485-2wire, RS485-4wires).
 * UART device node is /dev/ttyM0 - /dev/ttyM3.
 * DIO device node is /dev/dio.
 * Copyright Moxa Systems L.T.D.
 * History:
 * Date		Author			Comment
 * 02-20-2006	Victor Yu.		Create it.
 */

#ifndef _MOXADEVICE_H
#define _MOXADEVICE_H

// following about UART operatin mode
// ioctl command define
// ioctl(fd, MOXA_SET_OP_MODE, &mode)
// ioctl(fd, MOXA_GET_OP_MODE, &mode)
#define MOXA_SET_OP_MODE      (0x400+66)	// to set operating mode
#define MOXA_GET_OP_MODE      (0x400+67)	// to get now operating mode
// following add by Victor Yu. 08-12-2004
// ioctl(fd, MOXA_SET_SPECIAL_BAUD_RATE, &speed)
// ioctl(fd, MOXA_GET_SPECIAL_BAUD_RATE, &speed)
#define UC_SET_SPECIAL_BAUD_RATE      	(0x400+68)	// to set special baud rate
#define UC_GET_SPECIAL_BAUD_RATE      	(0x400+69)	// to get now special baud rate
#define MOXA_SET_SPECIAL_BAUD_RATE      (0x400+100)	// to set special baud rate
#define MOXA_GET_SPECIAL_BAUD_RATE      (0x400+101)	// to get now special baud rate

// operating mode value define
#define RS232_MODE              0
#define RS485_2WIRE_MODE        1
#define RS422_MODE              2
#define RS485_4WIRE_MODE        3

// following to support DIO function
//#define MAX_DIN_PORT		4
//#define MAX_DOUT_PORT		4
#define DIO_HIGH		1 // the DIO data is high
#define DIO_LOW			0 // the DIO data is low

#define DIN_EVENT_STATE_CHANGE	2 // high to low or low to high
#define DIN_EVENT_HIGH_TO_LOW	1 // high to low
#define DIN_EVENT_LOW_TO_HIGH	0 // low to high
#define DIN_EVENT_CLEAR		-1 // clear callback event

#define DIO_ERROR_PORT		-1	// no such port
#define DIO_ERROR_MODE		-2	// no such mode or state
#define DIO_ERROR_CONTROL	-3	// open or ioctl fail
#define DIO_ERROR_DURATION	-4	// the value of duration is not 0 or not in the
					// range 40 <= duration <= 3600000 (1 hours)
					// the unit is ms
#define DIO_ERROR_DURATION_20MS	-5	// the value of duration not be a multiple of 20
#define DIO_OK			0

#ifdef __cplusplus
extern	"C" {
#endif

// following is used for DIO
// to set DO to output high or low
// input:	int doport	- the DO port number, 0 - 3
//		int state	- the state must DIO_HIGH or DIO_LOW
// output:	error code 
int set_dout_state(int doport, int state);

// to get DO state at now
// input:	int doport	- the DO port number, 0 - 3
//		int *state	- return the state at now
// output:	error code 
int get_dout_state(int doport, int *state);

// to get DI state at now
// input:	int doport	- the DO port number, 0 - 3
//		int *state	- return the state at now
// output:	error code 
int get_din_state(int diport, int *state);

// to set the DI event callback function entry or clear it
// input:	int doport			- the DO port number, 0 - 3
//		void (*func)(int diport)	- the callback function entry
//		int mode			- set the mode when the callback function
//						  will be called
//		unsigned long duration		- the duration time, unit is ms
//						  must be 40 - 3600000
//						  if the value is 0, it means no duration
// output:	error code
int set_din_event(int diport, void (*func)(int diport), int mode, unsigned long duration);

// to get the DI event mode & duration value at now
// input:	int doport	- the DO port number, 0 - 3
//		int *mode	- return the mode setting at now
//		int *duration	- return the duration value
// output:	error code
int get_din_event(int diport, int *mode, unsigned long *duration);

// following is used for WatchDog
// to open the watchdog controller and get the file handler
// intput:	none
// ouput:	> 0 		- the file handler
//		<= 0		- has error
int swtd_open(void);

// to enable the watchdog, so the application need to ack the watchdog
// input:	int fd			- the swtd_open() return file handler
//		unsigned long time	- the interval time which the application to ack
// output:	error code
int swtd_enable(int fd, unsigned long time);

// to disable the watchdog
// input:	int fd			- the swtd_open() return file handler
// output:	error code
int swtd_disable(int fd);

// to get the setting at now for watchdog
// input:	int fd			- the swtd_open() return file handler
//		int *mode		- 1 for enable, 0 for disable	
//		unsigned long *time	- the set time for application
// output:	error code
int swtd_get(int fd, int *mode, unsigned long *time);

// to ack the watchdog
// input:	int fd			- the swtd_open() return file handler
// output:	error code
int swtd_ack(int fd);

// to close the watchdog
// input:	int fd			- the swtd_open() return file handler
// output:	error code
int swtd_close(int fd);


//add by EthanSH Lee (02-17-2017)
#define MAX_BUFF_LEN 256
#define MAX_DIO_NUM 10
typedef struct config_struct {
    int max_din_port;
    int max_dout_port;
    char dio_node_name[MAX_BUFF_LEN];
    char method[MAX_BUFF_LEN];
    char product[MAX_BUFF_LEN];
    int din_gpio[MAX_DIO_NUM];
    int dout_gpio[MAX_DIO_NUM];
} CONFIG_STRUCT;

CONFIG_STRUCT config;

void create_din_event_array();

void clear_din_event_array();

#ifdef __cplusplus
}
#endif

#endif	// _MOXADEVICE_H

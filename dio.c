
/*
 * History:
 * Date		Author			Comment
 * 02-20-2006	Victor Yu.		Create it.
 */
#include	<sys/time.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include 	<string.h>
#include	<sys/ioctl.h>
#include	<fcntl.h>
#include	<pthread.h>
#include	"moxadevice.h"

// following DIN & DOUT implement
#define DIN_PORT_POLLING_INTERVAL       100
#define IOCTL_SET_DOUT	15
#define IOCTL_GET_DOUT	16
#define IOCTL_GET_DIN	17
#define MIN_DURATION 	40	// 40 ms
#define MAX_DURATION 	3600000	// about 3600 seconds (1 hour)  //4294967295
#define DIN_INACCURACY	24000	// Add by Jared 07-04-2005. Revise the inacuracy. 24000 usec.
//#define DIO_NODE 	"/dev/dio"

#define CheckDINPort(p) { \
	if ( (p) < 0 || (p) >= config.max_din_port ) \
		return DIO_ERROR_PORT;	\
}
#define CheckDOUTPort(p) { \
	if ( (p) < 0 || (p) >= config.max_dout_port ) \
		return DIO_ERROR_PORT;	\
}
#define CheckState(s) { \
	if ( (s) != DIO_HIGH && (s) != DIO_LOW ) \
		return DIO_ERROR_MODE;	\
}
#define CheckMode(m) { \
	if ( (m) != DIN_EVENT_HIGH_TO_LOW && (m) != DIN_EVENT_LOW_TO_HIGH && (m) != DIN_EVENT_CLEAR && (m) != DIN_EVENT_STATE_CHANGE ) \
		return DIO_ERROR_MODE; \
}
// Add by Jared , 06-14-2005, The value of duration is not 0 or  not in the range, 40000 <= duration <= 3600 seconds (1 hour) .
// Add by Jared , 07-6-2005, The value of duration must be a multiple of 20 ms .
#define CheckDuration(m) { \
	if ( (m) != 0 ) { \
		if (  (m) < MIN_DURATION || (m) > MAX_DURATION  ) \
			return DIO_ERROR_DURATION; \
		if ( (m) % 20 ) \
			return DIO_ERROR_DURATION_20MS; \
	} \
}


typedef struct dio_struct {
	int	port;
	int	data;
} dio_t;

static int		pthread_fork_flag=0;
static pthread_t	din_thread;
static pthread_mutex_t	lock;

typedef struct din_event_struct {
	void	(*func)(int diport);
	int	mode;
	int	oldstate;
	unsigned long	duration;
} din_event_t;

din_event_t *din_event;

void create_din_event_array()
{
	int i;
	din_event =  malloc(config.max_din_port * sizeof(din_event_t));
	for(i=0 ; i<config.max_din_port ; i++){
		din_event[i].func = NULL;
		din_event[i].mode = DIN_EVENT_CLEAR;
		din_event[i].oldstate = DIO_HIGH;
		din_event[i].duration = 0;
		//printf("din_event[%d].duration=%d\n",i ,din_event[i].duration);
	}
}

void clear_din_event_array(){
	free(din_event);
}


/*din_event_t din_event[MAX_DIN_PORT]={
	{NULL, DIN_EVENT_CLEAR, DIO_HIGH, 0},
	{NULL, DIN_EVENT_CLEAR, DIO_HIGH, 0},
	{NULL, DIN_EVENT_CLEAR, DIO_HIGH, 0},
	{NULL, DIN_EVENT_CLEAR, DIO_HIGH, 0}
};
*/

static void *din_poll(void *arg)
{
	int	i, state;
	struct timeval tv;
	// The din_starttime[i] is used to record the starting time of duration
	struct timeval din_starttime[config.max_din_port];
	// The din_duration[i] is used to record the duration of port i keep in some state
	unsigned long din_duration[config.max_din_port];
	// The bStartDurationCheck[i] flag is used to control if the din_duration[i] should start counting.
	int bStartDurationCheck[config.max_din_port];
	
	memset((void *)din_starttime, 0, sizeof(din_starttime));
	memset((void *)din_duration, 0, sizeof(din_duration));
	memset((void *)bStartDurationCheck, 0, sizeof(bStartDurationCheck));

	while ( 1 ) {
		pthread_mutex_lock(&lock);
		for ( i=0 ; i<config.max_din_port; i++ ) {
			if ( din_event[i].func == NULL || din_event[i].mode == DIN_EVENT_CLEAR )
				continue;
			if ( get_din_state(i, &state) != DIO_OK )
				continue;
			if ( din_event[i].duration == 0 )	{		//when the duration is zero, callback directly when the din pin state changed
				if ( state != din_event[i].oldstate ) {
					din_event[i].oldstate = state;
					if ( (din_event[i].mode == DIN_EVENT_HIGH_TO_LOW && state == DIO_LOW) || (din_event[i].mode == DIN_EVENT_LOW_TO_HIGH && state == DIO_HIGH) || (din_event[i].mode == DIN_EVENT_STATE_CHANGE) ) {
						din_event[i].func(i);
					}
				}
			} else {	//when the duration is not zero, check the din pin
				if ( state != din_event[i].oldstate ) {  // when the state changes, reset duration and bCallback flags
					if( (din_event[i].mode == DIN_EVENT_LOW_TO_HIGH && state == DIO_HIGH ) || (din_event[i].mode == DIN_EVENT_HIGH_TO_LOW && state == DIO_LOW) || (din_event[i].mode == DIN_EVENT_STATE_CHANGE) ) {
						bStartDurationCheck[i] = 1;		// start duration check
						gettimeofday(&din_starttime[i],NULL);	// record the starting time of this duration
					} else if( (din_event[i].mode == DIN_EVENT_HIGH_TO_LOW && state == DIO_HIGH) || (din_event[i].mode == DIN_EVENT_LOW_TO_HIGH && state == DIO_LOW) || (din_event[i].mode == DIN_EVENT_STATE_CHANGE) ) {
						bStartDurationCheck[i] = 0;	// stop duration check
					}
					din_event[i].oldstate = state;
				} else if ( bStartDurationCheck[i] == 1 ) {
					gettimeofday(&tv,NULL);	// calculate the duration
					din_duration[i] = (1000000*(unsigned long)(tv.tv_sec - din_starttime[i].tv_sec)) + (tv.tv_usec - din_starttime[i].tv_usec) ;
					if ( (din_duration[i] + DIN_INACCURACY) >= din_event[i].duration ) {  // Add by Jared 07-04-2005. Revise the inacuracy
						din_event[i].func(i);
						bStartDurationCheck[i] = 0;// reset the bStartDurationCheck[i] flag							
					}
				}
			}
		}  //end of for ( i=0 ; i<MAX_DIN_PORT; i++ )
		pthread_mutex_unlock(&lock);
		usleep(DIN_PORT_POLLING_INTERVAL);
	} // end of while (1)

	pthread_fork_flag = 0;

	return NULL;
}

int	set_dout_state(int doport, int state)
{
	int	fd;
	dio_t	dio;

	CheckDOUTPort(doport);
	CheckState(state);
	fd = open(config.dio_node_name, O_RDWR);
	if ( fd < 0 )
		return DIO_ERROR_CONTROL;
	dio.port = doport;
	dio.data = state;
	if ( ioctl(fd, IOCTL_SET_DOUT, &dio) != 0 ) {
		close(fd);
		return DIO_ERROR_CONTROL;
	}
	close(fd);
	return DIO_OK;
}

int	get_dout_state(int doport, int *state)
{
	int	fd;
	dio_t	dio;

	CheckDOUTPort(doport);
	fd = open(config.dio_node_name, O_RDWR);
	if ( fd < 0 )
		return DIO_ERROR_CONTROL;
	dio.port = doport;
	if ( ioctl(fd, IOCTL_GET_DOUT, &dio) != 0 ) {
		close(fd);
		return DIO_ERROR_CONTROL;
	}
	close(fd);
	*state = dio.data;
	return DIO_OK;
}

int	get_din_state(int diport, int *state)
{
	int	fd;
	dio_t	dio;
	CheckDINPort(diport);
	//printf("dio_node=%s\n",config.dio_node_name);
	fd = open(config.dio_node_name, O_RDWR);
	if ( fd < 0 )
		return DIO_ERROR_CONTROL;
	dio.port = diport;
	if ( ioctl(fd, IOCTL_GET_DIN, &dio) != 0 ) {
		close(fd);
		return DIO_ERROR_CONTROL;
	}
	close(fd);
	*state = dio.data;
	return DIO_OK;
}

int	set_din_event(int diport, void (*func)(int diport), int mode, unsigned long duration)
{
	CheckDINPort(diport);
	if ( func == NULL || mode == DIN_EVENT_CLEAR ) {
		if ( pthread_fork_flag == 0 )
			return DIO_OK;
		pthread_mutex_lock(&lock);
		din_event[diport].func = NULL;
		din_event[diport].mode = DIN_EVENT_CLEAR;
		din_event[diport].duration = 0;
		pthread_mutex_unlock(&lock);
		return DIO_OK;
	}
	CheckMode(mode);
	CheckDuration(duration);
	
	if ( pthread_fork_flag == 0 ) {
		pthread_mutex_init(&lock, NULL);

		din_event[diport].func = func;
		din_event[diport].mode = mode;
		din_event[diport].duration = duration * 1000;  // the input unit of duration is millisecond
		get_din_state(diport, &din_event[diport].oldstate);
		pthread_fork_flag = 1;
		
		pthread_create(&din_thread, NULL, din_poll, 0);
	} else {
		pthread_mutex_lock(&lock);
		din_event[diport].func = func;
		din_event[diport].mode = mode;
		din_event[diport].duration = duration * 1000;  // the input unit of duration is millisecond
		get_din_state(diport, &din_event[diport].oldstate);
		pthread_mutex_unlock(&lock);
	}
	return DIO_OK;
}

int	get_din_event(int diport, int *mode, unsigned long *duration)
{
	CheckDINPort(diport);
	*mode = din_event[diport].mode;
	*duration = din_event[diport].duration / 1000;   // the output unit of duration is millisecond
	return DIO_OK;
}

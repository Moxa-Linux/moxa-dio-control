/******************************************************************************
File Name : mx_dio_control.c

Description :
	This utility can control DO HIGH/LOW signal, and get DI/DO HIGH/LOW signal.

Usage :
	1.Get signal:
		./mx_dio_control -g [val] -n [slot]
			val=0 -> DO, val=1 -> DI
			slot -> slot number
	2.Set signal
		./mx_dio_control -s [val] -n [slot]
			val=0 -> LOW, val=1 -> HIGH
			slot -> slot number

History :
	Versoin		Author		Date		Comment
	1.0		HarryYJ.Jhou	07-22-2015	First Version
	1.1		EthanSH Lee	02-16-2017	Separate binary and config
*******************************************************************************/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<unistd.h>
#include	"moxadevice.h"
#include	<fcntl.h>
#include	<json-c/json.h>

#define GET 0
#define SET 1
#define DO 0
#define DI 1
#define GET 0
#define SET 1
#define MAX_GPIO_VALUE 200
#define MAX_CONFIG_VARIABLE_LEN 20
#define CONFIG_LINE_BUFFER_SIZE 100
#define CONFIG "dio.conf"
#define GPIO_EXPORT "/sys/class/gpio/export"

char	*DataString[2]={"Low  ", "High "};

void usage(){
	printf("Moxa DIO get/set utility.\n");
	printf("Usage: mx_dio_control [-g|-s] -n #slot\n");
	printf("\t-g [1|0] Get DI/DO value\n");
	printf("\t         0:DO\n");
	printf("\t         1:DI\n");
	printf("\t-s [1|0] Set DO value\n");
	printf("\t         0:LOW\n");
	printf("\t         1:HIGH\n");
	printf("\t-n [#num] port number\n");
	printf("\t         num: port number DI:1~%d DO:1~%d\n", config.max_din_port, config.max_dout_port);
	printf("\n");
	printf("Example:\n");
	printf("\tGet port 2 DI value\n");
	printf("\t         mx_dio_control -g 1 -n 2\n");
	printf("\tSet port 2 DO value to LOW\n");
	printf("\t         mx_dio_control -s 0 -n 2\n");
	exit(-1);
}



int get_dio_gpio(int gpio_num){
	char GPIO_PATH[]="/sys/class/gpio/gpio";
	char str_num[5];
	char value[]="/value";
	char GPIO_VALUE[MAX_GPIO_VALUE];
	int ret;

	sprintf(str_num, "%d", gpio_num);
	
	strcpy(GPIO_VALUE, GPIO_PATH);
	strcat(GPIO_VALUE, str_num);
	strcat(GPIO_VALUE, value);
	FILE *file=fopen(GPIO_VALUE,"r");
	if (file) {
        	fscanf(file, "%d", &ret);
	}
	fclose(file);
	return ret;
}

void write_file(char *file ,char *str){
	FILE *fp=fopen(file,"w");
	int ret;
	if (fp == NULL)
	{	
		printf ("Fail to open %s\nProgram exits.\n",file);
		exit(1);
	}
	
	ret = fprintf(fp, "%s", str);
	//printf("str=%s ,ret=%d\n", str, ret);
	fclose(fp);
}

void set_dout_gpio (int gpio_num, int low_high){
	char GPIO_PATH[]="/sys/class/gpio/gpio";
	char str_num[5];
	char low_high_str[1];
	char direction[]="/direction";
	char value[]="/value";
	char GPIO_DIRECTION[MAX_GPIO_VALUE];
	char GPIO_VALUE[MAX_GPIO_VALUE];
	FILE *fp;

	sprintf(low_high_str, "%d", low_high);
	sprintf(str_num, "%d", gpio_num);
	strcpy(GPIO_DIRECTION, GPIO_PATH);
	strcat(GPIO_DIRECTION, str_num);
	strcat(GPIO_DIRECTION, direction); 
	
	strcpy(GPIO_VALUE, GPIO_PATH);
	strcat(GPIO_VALUE, str_num);
	strcat(GPIO_VALUE, value);
	write_file(GPIO_VALUE, low_high_str);
}

int *read_gpio_from_config_line(char* config_line , int array_size) {    
		char prm_name[MAX_CONFIG_VARIABLE_LEN];
		char prm_value[MAX_CONFIG_VARIABLE_LEN];
		int i=0;
		int *gpio_array;
	
    		sscanf(config_line, "%s\t%s\n", prm_name, prm_value);
		gpio_array = (int*)malloc(sizeof(int) * array_size);
		char *number=strtok(prm_value, ",");
		
		while(number != NULL){
			*(gpio_array + i) = atoi(number);
			number = strtok(NULL, ",");
			i++;
		}
		//printf("gpio_array=%d\n", *(gpio_array+0) );	
		return gpio_array;
}

int read_int_from_config_line(char* config_line) {    
		char prm_name[MAX_CONFIG_VARIABLE_LEN];
    		int val;
    		sscanf(config_line, "%s\t%d\n", prm_name, &val);
    		return val;
}

void read_str_from_config_line(char* config_line, char* val) {    
  		char prm_name[MAX_CONFIG_VARIABLE_LEN];
    		sscanf(config_line, "%s\t%s\n", prm_name, val);
}


void load_config_file(char* config_filename) {
	int i;

	struct json_object *jobj=json_object_from_file(config_filename);
	struct json_object *jarray;
	struct json_object *jitem;

	const char *MAX_DIN_PORT;
	const char *MAX_DOUT_PORT;
	const char *METHOD;
	const char *DIO_NODE;
	int DOUT_GPIO[10];
	int DIN_GPIO[10];

	json_object_object_get_ex(jobj, "MAX_DIN_PORT", &jarray);
	config.max_din_port=json_object_get_int(jarray);

	json_object_object_get_ex(jobj, "MAX_DOUT_PORT", &jarray);
	config.max_dout_port=json_object_get_int(jarray);

	json_object_object_get_ex(jobj, "METHOD", &jarray);
	strcpy(config.method, json_object_get_string(jarray));

	if (strcmp(config.method, "gpio")==0){
		json_object_object_get_ex(jobj, "DIN_PIN", &jarray);
		for (i = 0 ; i<3 ; i++){
			jitem=json_object_array_get_idx(jarray, i);
			DIN_GPIO[i]=json_object_get_int(jitem);
			(config.din_gpio[i])=DIN_GPIO[i];
		}
		
		json_object_object_get_ex(jobj, "DOUT_PIN", &jarray);
		for (i = 0 ; i<3 ; i++){
			jitem=json_object_array_get_idx(jarray, i);
			DOUT_GPIO[i]=json_object_get_int(jitem);
			(config.dout_gpio[i])=DOUT_GPIO[i];
		}
	}else if (strcmp(config.method, "ioctl")==0){
		json_object_object_get_ex(jobj, "DIO_NODE", &jarray);
		strcpy(config.dio_node_name, json_object_get_string(jarray));
	}else{
		printf("Unknown method\n");
		exit(-3);
	}
}

int main(int argc, char * argv[])
{
	int 	i, j, state, retval;
	int	c, val, gs=-1, slot=-1; 
	// val==0 --> DO val==1 --> DI, when gs=GET
	// val==0 --> low, val==1 --> high, when gs=SET
	// gs(get/set)

	char optstring[] = "g:s:n:";
	while ((c = getopt(argc, argv, optstring)) != -1)
		switch (c) {
		case 'g':
			if ( gs == -1 ){
				gs = GET;
				val = atoi(optarg);
			}else{
				usage();
			}
			break;
		case 's':
			if ( gs == -1 ){
				gs = SET;
				val = atoi(optarg);
			}else{
				usage();
			}
			break;
		case 'n':
			slot = atoi(optarg);
			break;
		default:
			usage();
			return -1;
		}

	//char kversion[100];
	char config_path[100];
	//FILE *fp = popen("kversion | awk '{print $1}'", "r");
	//fscanf(fp, "%s.conf", &kversion);
	//pclose(fp);	
	strcpy(config_path, "/etc/moxa-configs/moxa-dio-control.json");
	
	load_config_file(config_path);
	if(strcmp(config.method, "ioctl") == 0)
	{
		create_din_event_array();
		if ( !( ((gs == GET && slot>=1 && ((val == DI && slot<=config.max_din_port) || (val == DO && slot<=config.max_dout_port))))
			|| (gs == SET && slot>=1 && slot<=config.max_dout_port) ) )
			usage();

		if ( gs == GET ){
			if ( val == DI ){
				get_din_state(slot-1, &state);
				printf("DIN(%d):%s\n", slot, DataString[state]);
			}else if ( val == DO ){
				get_dout_state(slot-1, &state);
				printf("DOUT(%d):%s\n", slot, DataString[state]);	
			}
		}else if ( gs == SET ){
				retval=set_dout_state(slot-1, val);
				if ( retval ){
					printf("SET DO error\n");
					return -1;
				}
		}
		clear_din_event_array();
	}else if (strcmp(config.method, "gpio") == 0) {
		if ( !( ((gs == GET && slot>=1 && ((val == DI && slot<=config.max_din_port) || (val == DO && slot<=config.max_dout_port))))
			|| (gs == SET && slot>=1 && slot<=config.max_dout_port) ) )
			usage();
		if ( gs == GET ){
			if ( val == DI ){
				int ret_val= get_dio_gpio(*(config.din_gpio+(slot-1)));
				printf("DIN(%d):%s\n", slot, DataString[ret_val]);
			}else if ( val == DO ){
				int ret_val= get_dio_gpio(*(config.dout_gpio+(slot-1)));
				printf("DOUT(%d):%s\n", slot, DataString[ret_val]);
			}
		}else if ( gs == SET ){
			set_dout_gpio( *(config.dout_gpio+(slot-1)), val);
		}
	}else{
		printf("Error METHOD in %s\n", CONFIG);	
	}
	return 0;
}

/*
 * Copyright (C) MOXA Inc. All rights reserved.
 *
 * Name:
 *	MOXA DIO Library
 *
 * Description:
 *	Library for getting DIN/DOUT state and setting DOUT state.
 *
 * Authors:
 *	2006	Victor Yu	<victor.yu@moxa.com>
 *	2015	Harry YJ Jhou	<HarryYJ.Jhou@moxa.com>
 *	2017	Ethan SH Lee	<EthanSH.Lee@moxa.com>
 *	2018	Ken CJ Chou	<KenCJ.Chou@moxa.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <json-c/json.h>
#include <moxa/mx_gpio.h>
#include <moxa/mx_errno.h>

#include "mx_dio.h"

#define CONF_FILE "/etc/moxa-configs/moxa-dio-control.json"
#define CONF_VER_SUPPORTED "1.1.*"

#define DIO_DEVICE_NODE "/dev/dio"
#define MAX_FILEPATH_LEN 256	/* reserved length for file path */
#define DIN_INACCURACY 24000

enum ioctl_number {
	IOCTL_SET_DOUT = 15,
	IOCTL_GET_DOUT = 16,
	IOCTL_GET_DIN = 17
};

struct dio_struct {
	int port;
	int data;
};

struct din_poll_thread_struct {
	int flag;
	pthread_t thread;
	pthread_mutex_t lock;
};

struct din_event_struct {
	void (*func)(int diport);
	int mode;
	int last_state;
	unsigned long duration;
	int checking;
	struct timeval start_time;
};

static int lib_initialized;
static struct json_object *config;
static struct din_poll_thread_struct din_poll_thread;
static struct din_event_struct *din_event;

/*
 * json-c utilities
 */

static inline int obj_get_obj(struct json_object *obj, char *key, struct json_object **val)
{
	return -!json_object_object_get_ex(obj, key, val);
}

static int obj_get_int(struct json_object *obj, char *key, int *val)
{
	struct json_object *tmp;

	if (obj_get_obj(obj, key, &tmp) < 0)
		return -1;

	*val = json_object_get_int(tmp);
	return 0;
}

static int obj_get_str(struct json_object *obj, char *key, const char **val)
{
	struct json_object *tmp;

	if (obj_get_obj(obj, key, &tmp) < 0)
		return -1;

	*val = json_object_get_string(tmp);
	return 0;
}

static int obj_get_arr(struct json_object *obj, char *key, struct array_list **val)
{
	struct json_object *tmp;

	if (obj_get_obj(obj, key, &tmp) < 0)
		return -1;

	*val = json_object_get_array(tmp);
	return 0;
}

static int arr_get_obj(struct array_list *arr, int idx, struct json_object **val)
{
	if (arr == NULL || idx >= arr->length)
		return -1;

	*val = array_list_get_idx(arr, idx);
	return 0;
}

static int arr_get_int(struct array_list *arr, int idx, int *val)
{
	struct json_object *tmp;

	if (arr_get_obj(arr, idx, &tmp) < 0)
		return -1;

	*val = json_object_get_int(tmp);
	return 0;
}

static int arr_get_str(struct array_list *arr, int idx, const char **val)
{
	struct json_object *tmp;

	if (arr_get_obj(arr, idx, &tmp) < 0)
		return -1;

	*val = json_object_get_string(tmp);
	return 0;
}

static int arr_get_arr(struct array_list *arr, int idx, struct array_list **val)
{
	struct json_object *tmp;

	if (arr_get_obj(arr, idx, &tmp) < 0)
		return -1;

	*val = json_object_get_array(tmp);
	return 0;
}

/*
 * static functions
 */

static int check_config_version_supported(const char *conf_ver)
{
	int cv[2], sv[2];

	if (sscanf(conf_ver, "%d.%d.%*s", &cv[0], &cv[1]) < 0) {
		perror("sscanf version code failed");
		return E_SYSFUNCERR;
	}

	if (sscanf(CONF_VER_SUPPORTED, "%d.%d.%*s", &sv[0], &sv[1]) < 0) {
		perror("sscanf version code failed");
		return E_SYSFUNCERR;
	}

	if (cv[0] != sv[0] || cv[1] != sv[1]) {
		fprintf(stderr, "mx_dio config version not supported,\n");
		fprintf(stderr, "need to be%s\n", CONF_VER_SUPPORTED);
		return E_UNSUPCONFVER;
	}
	return E_SUCCESS;
}

static int init_din_event_array(void)
{
	int num_of_din_ports, i;

	if (obj_get_int(config, "NUM_OF_DIN_PORTS", &num_of_din_ports) < 0)
		return E_CONFERR;

	din_event = (struct din_event_struct *)
		malloc(num_of_din_ports * sizeof(struct din_event_struct));
	if (din_event == NULL) {
		perror("malloc");
		return E_SYSFUNCERR;
	}

	for (i = 0; i < num_of_din_ports; i++) {
		din_event[i].func = NULL;
		din_event[i].mode = DIN_EVENT_CLEAR;
		din_event[i].checking = 0;
	}
	return E_SUCCESS;
}

static int set_dout_state_ioctl(int doport, int state)
{
	struct dio_struct dio;
	const char *dio_node;
	int fd;

	if (obj_get_str(config, "DIO_NODE", &dio_node) < 0)
		return E_CONFERR;

	fd = open(dio_node, O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "open %s: %s\n", dio_node, strerror(errno));
		return E_SYSFUNCERR;
	}

	dio.port = doport;
	dio.data = state;
	if (ioctl(fd, IOCTL_SET_DOUT, &dio) < 0) {
		perror("ioctl: IOCTL_SET_DOUT");
		close(fd);
		return E_SYSFUNCERR;
	}
	close(fd);

	return E_SUCCESS;
}

static int get_dout_state_ioctl(int doport, int *state)
{
	struct dio_struct dio;
	const char *dio_node;
	int fd;

	if (obj_get_str(config, "DIO_NODE", &dio_node) < 0)
		return E_CONFERR;

	fd = open(dio_node, O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "open %s: %s\n", dio_node, strerror(errno));
		return E_SYSFUNCERR;
	}

	dio.port = doport;
	if (ioctl(fd, IOCTL_GET_DOUT, &dio) < 0) {
		perror("ioctl: IOCTL_GET_DOUT");
		close(fd);
		return E_SYSFUNCERR;
	}
	close(fd);

	*state = dio.data;
	return E_SUCCESS;
}

static int get_din_state_ioctl(int diport, int *state)
{
	struct dio_struct dio;
	const char *dio_node;
	int fd;

	if (obj_get_str(config, "DIO_NODE", &dio_node) < 0)
		return E_CONFERR;

	fd = open(dio_node, O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "open %s: %s\n", dio_node, strerror(errno));
		return E_SYSFUNCERR;
	}

	dio.port = diport;
	if (ioctl(fd, IOCTL_GET_DIN, &dio) < 0) {
		perror("ioctl: IOCTL_GET_DIN");
		close(fd);
		return E_SYSFUNCERR;
	}
	close(fd);

	*state = dio.data;
	return E_SUCCESS;
}

static int set_dout_state_gpio(int doport, int state)
{
	struct array_list *gpio_nums_of_doports;
	int ret, gpio_num;

	if (obj_get_arr(config, "GPIO_NUMS_OF_DOUT_PORTS", &gpio_nums_of_doports) < 0)
		return E_CONFERR;

	if (arr_get_int(gpio_nums_of_doports, doport, &gpio_num) < 0)
		return E_CONFERR;

	if (state == DIO_STATE_LOW) {
		ret = mx_gpio_set_value(gpio_num, GPIO_VALUE_LOW);
		if (ret < 0)
			return ret;
	} else if (state == DIO_STATE_HIGH) {
		ret = mx_gpio_set_value(gpio_num, GPIO_VALUE_HIGH);
		if (ret < 0)
			return ret;
	}

	return E_SUCCESS;
}

static int get_dout_state_gpio(int doport, int *state)
{
	struct array_list *gpio_nums_of_doports;
	int ret, gpio_num;

	if (obj_get_arr(config, "GPIO_NUMS_OF_DOUT_PORTS", &gpio_nums_of_doports) < 0)
		return E_CONFERR;

	if (arr_get_int(gpio_nums_of_doports, doport, &gpio_num) < 0)
		return E_CONFERR;

	ret = mx_gpio_get_value(gpio_num, state);
	if (ret < 0)
		return ret;

	return E_SUCCESS;
}

static int get_din_state_gpio(int diport, int *state)
{
	struct array_list *gpio_nums_of_diports;
	int ret, gpio_num;

	if (obj_get_arr(config, "GPIO_NUMS_OF_DIN_PORTS", &gpio_nums_of_diports) < 0)
		return E_CONFERR;

	if (arr_get_int(gpio_nums_of_diports, diport, &gpio_num) < 0)
		return E_CONFERR;

	ret = mx_gpio_get_value(gpio_num, state);
	if (ret < 0)
		return ret;

	return E_SUCCESS;
}

static unsigned long count_timeval_diff(struct timeval t1, struct timeval t2)
{
	unsigned long diff = 0;

	diff += 1000000 * (unsigned long) (t2.tv_sec - t1.tv_sec);
	diff += t2.tv_usec - t1.tv_usec;
	return diff;
}

static void check_event(int diport)
{
	struct din_event_struct *ev;
	int state;
	struct timeval tv;

	ev = &din_event[diport];

	if (ev->func == NULL ||
		ev->mode == DIN_EVENT_CLEAR)
		return;

	if (mx_din_get_state(diport, &state) < 0)
		return;

	if (ev->duration == 0) {
		if (state != ev->last_state) {
			if ((ev->mode == DIN_EVENT_HIGH_TO_LOW && state == DIO_STATE_LOW) ||
				(ev->mode == DIN_EVENT_LOW_TO_HIGH && state == DIO_STATE_HIGH) ||
				(ev->mode == DIN_EVENT_STATE_CHANGE)) {
				ev->func(diport);
			}
			ev->last_state = state;
		}
	} else {
		if (state != ev->last_state) {
			if ((ev->mode == DIN_EVENT_LOW_TO_HIGH && state == DIO_STATE_HIGH) ||
				(ev->mode == DIN_EVENT_HIGH_TO_LOW && state == DIO_STATE_LOW) ||
				(ev->mode == DIN_EVENT_STATE_CHANGE)) {
				gettimeofday(&ev->start_time, NULL);
				ev->checking = 1;
			} else if ((ev->mode == DIN_EVENT_HIGH_TO_LOW && state == DIO_STATE_HIGH) ||
				(ev->mode == DIN_EVENT_LOW_TO_HIGH && state == DIO_STATE_LOW)) {
				ev->checking = 0;
			}
			ev->last_state = state;
		} else if (ev->checking == 1) {
			gettimeofday(&tv, NULL);
			if ((count_timeval_diff(ev->start_time, tv)
				+ DIN_INACCURACY) >= ev->duration) {
				ev->func(diport);
				ev->checking = 0;
			}
		}
	}
}

static void *din_poll(void *arg)
{
	int num_of_din_ports, polling_interval;
	int i;

	obj_get_int(config, "NUM_OF_DIN_PORTS", &num_of_din_ports);
	obj_get_int(config, "DIN_PORT_POLLING_INTERVAL", &polling_interval);

	while (1) {
		pthread_mutex_lock(&din_poll_thread.lock);
		for (i = 0; i < num_of_din_ports; i++)
			check_event(i);
		pthread_mutex_unlock(&din_poll_thread.lock);
		usleep(polling_interval);
	}

	din_poll_thread.flag = 0;
	return NULL;
}

/*
 * APIs
 */

int mx_dio_init(void)
{
	int ret;
	const char *conf_ver;

	if (lib_initialized)
		return E_SUCCESS;

	config = json_object_from_file(CONF_FILE);
	if (config == NULL)
		return E_CONFERR;

	if (obj_get_str(config, "CONFIG_VERSION", &conf_ver) < 0)
		return E_CONFERR;

	ret = check_config_version_supported(conf_ver);
	if (ret < 0)
		return ret;

	din_poll_thread.flag = 0;
	ret = init_din_event_array();
	if (ret < 0)
		return ret;

	lib_initialized = 1;
	return E_SUCCESS;
}

int mx_dout_set_state(int doport, int state)
{
	const char *method;
	int num_of_dout_ports;

	if (!lib_initialized)
		return E_LIBNOTINIT;

	if (obj_get_int(config, "NUM_OF_DOUT_PORTS", &num_of_dout_ports) < 0)
		return E_CONFERR;

	if (doport < 0 || doport >= num_of_dout_ports)
		return E_INVAL;

	if (state != DIO_STATE_LOW && state != DIO_STATE_HIGH)
		return E_INVAL;

	if (obj_get_str(config, "METHOD", &method) < 0)
		return E_CONFERR;

	if (strcmp(method, "IOCTL") == 0)
		return set_dout_state_ioctl(doport, state);
	else if (strcmp(method, "GPIO") == 0)
		return set_dout_state_gpio(doport, state);

	return E_CONFERR;
}

int mx_dout_get_state(int doport, int *state)
{
	const char *method;
	int num_of_dout_ports;

	if (!lib_initialized)
		return E_LIBNOTINIT;

	if (obj_get_int(config, "NUM_OF_DOUT_PORTS", &num_of_dout_ports) < 0)
		return E_CONFERR;

	if (doport < 0 || doport >= num_of_dout_ports)
		return E_INVAL;

	if (obj_get_str(config, "METHOD", &method) < 0)
		return E_CONFERR;

	if (strcmp(method, "IOCTL") == 0)
		return get_dout_state_ioctl(doport, state);
	else if (strcmp(method, "GPIO") == 0)
		return get_dout_state_gpio(doport, state);

	return E_CONFERR;
}

int mx_din_get_state(int diport, int *state)
{
	const char *method;
	int num_of_din_ports;

	if (!lib_initialized)
		return E_LIBNOTINIT;

	if (obj_get_int(config, "NUM_OF_DIN_PORTS", &num_of_din_ports) < 0)
		return E_CONFERR;

	if (diport < 0 || diport >= num_of_din_ports)
		return E_INVAL;

	if (obj_get_str(config, "METHOD", &method) < 0)
		return E_CONFERR;

	if (strcmp(method, "IOCTL") == 0)
		return get_din_state_ioctl(diport, state);
	else if (strcmp(method, "GPIO") == 0)
		return get_din_state_gpio(diport, state);

	return E_CONFERR;
}

//int mx_dout_set_multi_state(u32 set_bits, u32 clear_bits);

int mx_din_set_event(int diport, void (*func)(int diport), int mode, unsigned long duration)
{
	int num_of_din_ports;

	if (!lib_initialized)
		return E_LIBNOTINIT;

	if (obj_get_int(config, "NUM_OF_DIN_PORTS", &num_of_din_ports) < 0)
		return E_CONFERR;

	if (diport < 0 || diport >= num_of_din_ports)
		return E_INVAL;

	if (func == NULL || mode == DIN_EVENT_CLEAR) {
		if (din_poll_thread.flag == 0)
			return E_SUCCESS;

		pthread_mutex_lock(&din_poll_thread.lock);
		din_event[diport].func = NULL;
		din_event[diport].mode = DIN_EVENT_CLEAR;
		din_event[diport].duration = 0;
		pthread_mutex_unlock(&din_poll_thread.lock);
		return E_SUCCESS;
	}

	if (mode != DIN_EVENT_LOW_TO_HIGH &&
		mode != DIN_EVENT_HIGH_TO_LOW &&
		mode != DIN_EVENT_STATE_CHANGE)
		return E_INVAL;

	if (duration != 0 && (duration < 40 || duration > 3600000))
		return E_INVAL;

	if (din_poll_thread.flag == 0) {
		pthread_mutex_init(&din_poll_thread.lock, NULL);

		din_event[diport].func = func;
		din_event[diport].mode = mode;
		din_event[diport].duration = duration * 1000;
		din_poll_thread.flag = 1;

		pthread_create(&din_poll_thread.thread, NULL, din_poll, NULL);
	} else {
		pthread_mutex_lock(&din_poll_thread.lock);
		din_event[diport].func = func;
		din_event[diport].mode = mode;
		din_event[diport].duration = duration * 1000;
		pthread_mutex_unlock(&din_poll_thread.lock);
	}

	return E_SUCCESS;
}

int mx_din_get_event(int diport, int *mode, unsigned long *duration)
{
	int num_of_din_ports;

	if (!lib_initialized)
		return E_LIBNOTINIT;

	if (obj_get_int(config, "NUM_OF_DIN_PORTS", &num_of_din_ports) < 0)
		return E_CONFERR;

	if (diport < 0 || diport >= num_of_din_ports)
		return E_INVAL;

	*mode = din_event[diport].mode;
	*duration = din_event[diport].duration / 1000;
	return E_SUCCESS;
}

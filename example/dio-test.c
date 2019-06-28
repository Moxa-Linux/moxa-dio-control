/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Name:
 *	C Example code for MOXA DIO Library
 *
 * Description:
 *	Example code for demonstrating the usage of MOXA DIO Library in C
 *
 * Authors:
 *	2018	Ken CJ Chou	<KenCJ.Chou@moxa.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <moxa/mx_dio.h>

int main(int argc, char *argv[])
{
	int ret, diport, doport, state;

	if (argc != 3) {
		fprintf(stderr, "Usage: %s <dout_port> <din_port>\n", argv[0]);
		exit(99);
	}
	doport = atoi(argv[1]);
	diport = atoi(argv[2]);

	ret = mx_dio_init();
	if (ret < 0) {
		fprintf(stderr, "Error: Initialize Moxa dio control library failed\n");
		fprintf(stderr, "Return code: %d\n", ret);
		exit(1);
	}

	printf("Testing DIO from DOUT port %d to DIN port %d:\n", doport, diport);
	printf("Please make sure they are connected to each other.\n");
	printf("========================================\n");

	printf("- Setting DOUT port %d to high.\n", doport);
	ret = mx_dout_set_state(doport, DIO_STATE_HIGH);
	if (ret < 0) {
		fprintf(stderr, "Error: Failed to set DOUT port %d\n", doport);
		fprintf(stderr, "Return code: %d\n", ret);
		exit(1);
	}
	sleep(1);

	printf("- Getting DIN port %d's state.\n", diport);
	ret = mx_din_get_state(diport, &state);
	if (ret < 0) {
		fprintf(stderr, "Error: Failed to get DIN port %d\n", diport);
		fprintf(stderr, "Return code: %d\n", ret);
		exit(1);
	}
	printf("  DIN port %d's state is %d.\n", diport, state);
	sleep(1);

	printf("- Setting DOUT port %d to low.\n", doport);
	ret = mx_dout_set_state(doport, DIO_STATE_LOW);
	if (ret < 0) {
		fprintf(stderr, "Error: Failed to set DOUT port %d\n", doport);
		fprintf(stderr, "Return code: %d\n", ret);
		exit(1);
	}
	sleep(1);

	printf("- Getting DIN port %d's state.\n", diport);
	ret = mx_din_get_state(diport, &state);
	if (ret < 0) {
		fprintf(stderr, "Error: Failed to get DIN port %d\n", diport);
		fprintf(stderr, "Return code: %d\n", ret);
		exit(1);
	}
	printf("  DIN port %d's state is %d.\n", diport, state);

	printf("========================================\n");
	printf("Test OK.\n");
	return 0;
}

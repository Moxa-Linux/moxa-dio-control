/*
 * Copyright (C) MOXA Inc. All rights reserved.
 *
 * Name:
 *	MOXA DIO Control Utility
 *
 * Description:
 *	Utility for getting DIN/DOUT state and setting DOUT state.
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
#include <stdlib.h>
#include <getopt.h>

#include "mx_dio.h"

#define UNSET -1

enum action_type {
	GET_DIN = 0,
	GET_DOUT = 1,
	SET_DOUT = 2
};

struct action_struct {
	int type;
	int port;
	int state;
};

void usage(FILE *fp)
{
	fprintf(fp, "Usage:\n");
	fprintf(fp, "	mx-dio-ctl <-g #DIN/DOUT |-s #state > -p <#port>\n\n");
	fprintf(fp, "OPTIONS:\n");
	fprintf(fp, "	-g <#DI/DO>\n");
	fprintf(fp, "		Set target to DIN or DOUT port\n");
	fprintf(fp, "		0 --> DIN\n");
	fprintf(fp, "		1 --> DOUT\n");
	fprintf(fp, "	-s <#state>\n");
	fprintf(fp, "		Set state for target DOUT port\n");
	fprintf(fp, "		0 --> LOW\n");
	fprintf(fp, "		1 --> HIGH\n");
	fprintf(fp, "	-p <#port>\n");
	fprintf(fp, "		Set target port number\n");
	fprintf(fp, "\n");
	fprintf(fp, "Example:\n");
	fprintf(fp, "	Get value from DIN port 1\n");
	fprintf(fp, "	# mx_dio_control -g 1 -p 1\n");
	fprintf(fp, "\n");
	fprintf(fp, "	Set DOUT port 2 value to LOW\n");
	fprintf(fp, "	# mx_dio_control -s 0 -p 2\n");
}

void do_action(struct action_struct action)
{
	switch (action.type) {
	case GET_DIN:
		if (mx_din_get_state(action.port, &action.state) < 0)
			exit(-1);
		printf("DIN port %d state: %d\n", action.port, action.state);
		break;
	case GET_DOUT:
		if (mx_dout_get_state(action.port, &action.state) < 0)
			exit(-1);
		printf("DOUT port %d state: %d\n", action.port, action.state);
		break;
	case SET_DOUT:
		if (mx_dout_set_state(action.port, action.state) < 0)
			exit(-1);
		printf("DOUT port %d state: %d\n", action.port, action.state);
		break;
	}
}

int main(int argc, char *argv[])
{
	struct option long_options[] = {
		{"help", no_argument, 0, 'h'},
		{"get-dio", required_argument, 0, 'g'},
		{"set-dout", required_argument, 0, 's'},
		{"port", required_argument, 0, 'p'},
		{0, 0, 0, 0}
	};
	struct action_struct action = {
		.type = UNSET,
		.port = UNSET,
		.state = UNSET
	};
	int c;

	while (1) {
		c = getopt_long(argc, argv, "hg:s:p:", long_options, NULL);
		if (c == -1)
			break;

		switch (c) {
		case 'h':
			usage(stdout);
			exit(0);
		case 'g':
			if (action.type != UNSET) {
				fprintf(stderr, "action has already set\n");
				usage(stderr);
				exit(-1);
			}
			action.type = atoi(argv[optind - 1]);
			break;
		case 's':
			if (action.type != UNSET) {
				fprintf(stderr, "action has already set\n");
				usage(stderr);
				exit(-1);
			}
			action.type = SET_DOUT;
			action.state = atoi(argv[optind - 1]);
			break;
		case 'p':
			action.port = atoi(argv[optind - 1]);
			break;
		default:
			usage(stderr);
			exit(-1);
		}
	}

	if (optind < argc) {
		fprintf(stderr, "non-option arguments found: ");
		while (optind < argc)
			fprintf(stderr, "%s ", argv[optind++]);
		fprintf(stderr, "\n");

		usage(stderr);
		exit(-1);
	}

	if (action.type == UNSET) {
		fprintf(stderr, "action type is unset\n");
		usage(stderr);
		exit(-1);
	}

	if (action.port == UNSET) {
		fprintf(stderr, "port number is unset\n");
		usage(stderr);
		exit(-1);
	}

	mx_dio_init();

	do_action(action);

	exit(0);
}

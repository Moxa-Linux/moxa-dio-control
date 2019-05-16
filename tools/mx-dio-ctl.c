/*
 * SPDX-License-Identifier: Apache-2.0
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
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mx_dio.h>

#define UNSET -1

enum action_type {
	GET_DOUT = 0,
	GET_DIN = 1,
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
	fprintf(fp, "	mx-dio-ctl <-g #DOUT/DIN |-s #state > -n <#port>\n\n");
	fprintf(fp, "OPTIONS:\n");
	fprintf(fp, "	-g <#DI/DO>\n");
	fprintf(fp, "		Get target to DOUT or DIN port\n");
	fprintf(fp, "		0 --> DOUT\n");
	fprintf(fp, "		1 --> DIN\n");
	fprintf(fp, "	-s <#state>\n");
	fprintf(fp, "		Set state for target DOUT port\n");
	fprintf(fp, "		0 --> LOW\n");
	fprintf(fp, "		1 --> HIGH\n");
	fprintf(fp, "	-n <#port>\n");
	fprintf(fp, "		Set target port number\n");
	fprintf(fp, "\n");
	fprintf(fp, "Example:\n");
	fprintf(fp, "	Get value from DIN port 1\n");
	fprintf(fp, "	# mx-dio-ctl -g 1 -n 1\n");
	fprintf(fp, "\n");
	fprintf(fp, "	Set DOUT port 2 value to LOW\n");
	fprintf(fp, "	# mx-dio-ctl -s 0 -n 2\n");
}

int my_atoi(const char *nptr, int *number)
{
	int tmp;

	tmp = atoi(nptr);
	if (tmp != 0) {
		*number = tmp;
		return 0;
	} else {
		if (!strcmp(nptr, "0")) {
			*number = 0;
			return 0;
		}
	}
	return -1;
}

void do_action(struct action_struct action)
{
	switch (action.type) {
	case GET_DIN:
		if (mx_din_get_state(action.port, &action.state) < 0) {
			fprintf(stderr, "Failed to get DIN state\n");
			exit(1);
		}
		printf("DIN port %d state: %d\n", action.port, action.state);
		break;
	case GET_DOUT:
		if (mx_dout_get_state(action.port, &action.state) < 0) {
			fprintf(stderr, "Failed to get DOUT state\n");
			exit(1);
		}
		printf("DOUT port %d state: %d\n", action.port, action.state);
		break;
	case SET_DOUT:
		if (mx_dout_set_state(action.port, action.state) < 0) {
			fprintf(stderr, "Failed to set DOUT state\n");
			exit(1);
		}
		printf("DOUT port %d state: %d\n", action.port, action.state);
		break;
	}
}

int main(int argc, char *argv[])
{
	struct action_struct action = {
		.type = UNSET,
		.port = UNSET,
		.state = UNSET
	};
	int c;

	while (1) {
		c = getopt(argc, argv, "hg:s:n:");
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
				exit(99);
			}
			if (my_atoi(optarg, &action.type) != 0) {
				fprintf(stderr, "%s is not a number\n", optarg);
				exit(1);
			}
			break;
		case 's':
			if (action.type != UNSET) {
				fprintf(stderr, "action has already set\n");
				usage(stderr);
				exit(99);
			}
			action.type = SET_DOUT;
			if (my_atoi(optarg, &action.state) != 0) {
				fprintf(stderr, "%s is not a number\n", optarg);
				exit(1);
			}
			break;
		case 'n':
			if (my_atoi(optarg, &action.port) != 0) {
				fprintf(stderr, "%s is not a number\n", optarg);
				exit(1);
			}
			break;
		default:
			usage(stderr);
			exit(99);
		}
	}

	if (optind < argc) {
		fprintf(stderr, "non-option arguments found: ");
		while (optind < argc)
			fprintf(stderr, "%s ", argv[optind++]);
		fprintf(stderr, "\n");

		usage(stderr);
		exit(99);
	}

	if (action.type == UNSET) {
		fprintf(stderr, "action type is unset\n");
		usage(stderr);
		exit(99);
	}

	if (action.port == UNSET) {
		fprintf(stderr, "port number is unset\n");
		usage(stderr);
		exit(99);
	}

	if (mx_dio_init() < 0) {
		fprintf(stderr, "Initialize Moxa dio control library failed\n");
		exit(1);
	}

	do_action(action);

	exit(0);
}

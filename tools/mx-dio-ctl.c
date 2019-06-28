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

enum action_version {
	ACTION_VER_OLD = 0,
	ACTION_VER_NEW = 1
};

struct action_struct {
	int version;
	int type;
	int port;
	int state;
};

void usage(FILE *fp)
{
	fprintf(fp, "Usage:\n");
	fprintf(fp, "	mx-dio-ctl <-i|-o <#port number> [-s <#state>]>\n\n");
	fprintf(fp, "OPTIONS:\n");
	fprintf(fp, "	-i <#DIN port number>\n");
	fprintf(fp, "	-o <#DOUT port number>\n");
	fprintf(fp, "	-s <#state>\n");
	fprintf(fp, "		Set state for target DOUT port\n");
	fprintf(fp, "		0 --> LOW\n");
	fprintf(fp, "		1 --> HIGH\n");
	fprintf(fp, "\n");
	fprintf(fp, "Example:\n");
	fprintf(fp, "	Get value from DIN port 0\n");
	fprintf(fp, "	# mx-dio-ctl -i 0\n");
	fprintf(fp, "	Get value from DOUT port 0\n");
	fprintf(fp, "	# mx-dio-ctl -o 0\n");
	fprintf(fp, "\n");
	fprintf(fp, "	Set DOUT port 0 value to LOW\n");
	fprintf(fp, "	# mx-dio-ctl -o 0 -s 0\n");
	fprintf(fp, "	Set DOUT port 0 value to HIGH\n");
	fprintf(fp, "	# mx-dio-ctl -o 0 -s 1\n");
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
		.version = UNSET,
		.type = UNSET,
		.port = UNSET,
		.state = UNSET
	};
	int c;

	while (1) {
		c = getopt(argc, argv, "hg:s:n:i:o:");
		if (c == -1)
			break;

		switch (c) {
		case 'h':
			usage(stdout);
			exit(0);
		case 'g':
			if (action.version != UNSET) {
				fprintf(stderr, "action has already set\n");
				usage(stderr);
				exit(99);
			}
			action.version = ACTION_VER_OLD;
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
		case 'i':
			if (action.version != UNSET) {
				fprintf(stderr, "action has already set\n");
				usage(stderr);
				exit(99);
			}
			action.version = ACTION_VER_NEW;
			action.type = GET_DIN;
			if (my_atoi(optarg, &action.port) != 0) {
				fprintf(stderr, "%s is not a number\n", optarg);
				exit(1);
			}
			break;
		case 'o':
			if (action.version != UNSET) {
				fprintf(stderr, "action has already set\n");
				usage(stderr);
				exit(99);
			}
			action.version = ACTION_VER_NEW;
			action.type = GET_DOUT;
			if (my_atoi(optarg, &action.port) != 0) {
				fprintf(stderr, "%s is not a number\n", optarg);
				exit(1);
			}
			break;
		case 's':
			if (action.type != UNSET && action.version == ACTION_VER_OLD) {
				fprintf(stderr, "action has already set\n");
				usage(stderr);
				exit(99);
			}
			if (action.type == GET_DIN && action.version == ACTION_VER_NEW) {
				fprintf(stderr, "can't set di state\n");
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

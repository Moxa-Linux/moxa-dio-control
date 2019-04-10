/*
 * SPDX-License-Identifier: Apache-2.0
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
 */

#ifndef _MOXA_DIO_H
#define _MOXA_DIO_H

enum dio_state {
	DIO_STATE_LOW = 0,
	DIO_STATE_HIGH = 1
};

enum din_event_mode {
	DIN_EVENT_CLEAR = -1,
	DIN_EVENT_LOW_TO_HIGH = 0,
	DIN_EVENT_HIGH_TO_LOW = 1,
	DIN_EVENT_STATE_CHANGE = 2
};

#ifdef __cplusplus
extern "C" {
#endif

extern int mx_dio_init(void);
extern int mx_dout_set_state(int doport, int state);
extern int mx_dout_get_state(int doport, int *state);
extern int mx_din_get_state(int diport, int *state);
// extern int mx_dout_set_multi_state(u32 set_bits, u32 clear_bits);
extern int mx_din_set_event(int diport, void (*func)(int diport), int mode, unsigned long duration);
extern int mx_din_get_event(int diport, int *mode, unsigned long *duration);


#ifdef __cplusplus
}
#endif

#endif /* _MOXA_DIO_H */


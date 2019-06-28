# API References

---
### int mx_dio_init(void)

Initialize Moxa DIO control library.

#### Return value
* 0 on success.
* negative numbers on error.

---
### int mx_dout_set_state(int doport, int state)

Set state for target Digital Output port.

#### Parameters
* doport: target DOUT port number
* state: DIO_STATE_LOW or DIO_STATE_HIGH

#### Return value
* 0 on success.
* negative numbers on error.

---
### int mx_dout_get_state(int doport, int *state)

Get state from target Digital Output port.

#### Parameters
* doport: target DOUT port number
* state: where the output value will be set.

#### Return value
* 0 on success.
* negative numbers on error.

---
### int mx_din_get_state(int diport, int *state)

Get state from target Digital Input port.

#### Parameters
* diport: target DIN port number
* state: where the output value will be set.

#### Return value
* 0 on success.
* negative numbers on error.

---
# moxa-dio-control

`moxa-dio-control` is a C library for getting and setting Digital Input/Output
ports state.

## Build

This project use autotools as buildsystem. You can build this project by the following commands:

* If the build target architecture is x86_64

	```
	# ./autogen.sh --host=x86_64-linux-gnu --includedir=/usr/include/moxa --libdir=/usr/lib/x86_64-linux-gnu --sbindir=/sbin
	# make
	# make install
	```
* If the build target architecture is armhf

	```
	# ./autogen.sh --host=arm-linux-gnueabihf --includedir=/usr/include/moxa --libdir=/usr/lib/arm-linux-gnueabihf --sbindir=/sbin
	# make
	# make install
	```

The autogen script will execute ./configure and pass all the command-line
arguments to it.

## Usage of mx-dio-ctl

```
Usage:
	mx-dio-ctl -g <DOUT/DIN>|-s <state> -n <port>

OPTIONS:
	-g <DOUT/DIN>
		Get target to DOUT or DIN port
		0 --> DOUT
		1 --> DIN
	-s <state>
		Set state for target DOUT port
		0 --> LOW
		1 --> HIGH
	-n <port>
		Set target port number

Example:
	Get value from DIN port 1
	# mx-dio-ctl -g 1 -n 1

	Set DOUT port 2 value to LOW
	# mx-dio-ctl -s 0 -n 2
```

## Documentation

[Config Example](/Config_Example.md)

[API Reference](/API_References.md)
## Config Example

### Path
```
/etc/moxa-configs/moxa-dio-control.json
```

### Description

* `CONFIG_VERSION`: The version of config file
* `METHOD`: The method to manipulate DIO, including GPIO and IOCTL
* `NUM_OF_DIN_PORTS`: The number of DIN ports on this device
* `NUM_OF_DOUT_PORTS`: The number of DOUT ports on this device
* `GPIO_NUMS_OF_DIN_PORTS`: The DIN ports' GPIO pin number
* `GPIO_NUMS_OF_DOUT_PORTS`: The DOUT ports' GPIO pin number
* `DIN_NODE`: The DIN device node of IOCTL
* `DOUT_NODE`: The DOUT device node of IOCTL
* `DIN_PORT_POLLING_INTERVAL`: The time interval between polling DIN ports for listening event


### Example1: UC-8410

```
{
	"CONFIG_VERSION": "1.1.0",

	"METHOD": "IOCTL",

	"NUM_OF_DIN_PORTS": 4,
	"NUM_OF_DOUT_PORTS": 4,

	"DIN_NODE": "/dev/di",
	"DOUT_NODE": "/dev/do",

	"DIN_PORT_POLLING_INTERVAL": 100
}
```

### Example2: UC-5111-LX

```
{
	"CONFIG_VERSION": "1.1.0",

	"METHOD": "GPIO",

	"NUM_OF_DIN_PORTS": 4,
	"NUM_OF_DOUT_PORTS": 4,

	"GPIO_NUMS_OF_DIN_PORTS": [86, 87, 88, 89],
	"GPIO_NUMS_OF_DOUT_PORTS": [54, 55, 56, 57],

	"DIN_PORT_POLLING_INTERVAL": 100
}
```

### Links
- [Product page](https://www.waveshare.com/sim7000e-nb-iot-hat.htm)
- [User guide](https://www.waveshare.com/wiki/SIM7000E_NB-IoT_HAT)

## Reading GPS data on Raspberry Pi

The `gps_console` program communicates with the SIM7000E module over UART
(115200 8N1), enables GNSS, and sends the complete `AT+CGNSINF` data set once
per second using Protocol Buffers. The schema is located at
`gps/proto/gnss.proto`. Each message written to standard output is prefixed with
its length encoded as a varint (`length-delimited` format).

### Cross-compiling on x86-64 for 64-bit Raspberry Pi OS

Rebuild the dev container image after changing the `Dockerfile`, then run:

```sh
cmake --preset raspberry-pi-arm64
cmake --build --preset raspberry-pi-arm64
```

This produces the following ARM64 binary:

```text
build/raspberry-pi-arm64/gps_console
```

Copy it to the Raspberry Pi, for example with `scp`. On the Raspberry Pi, enable
UART in `raspi-config`, make sure the login console is not using the serial port,
and ensure that the user belongs to the `dialout` group. Run:

```sh
./gps_console /dev/serial0 1 > gnss.pb
```

The first argument is the UART device and the second is the interval in seconds.
Protobuf data is written to stdout, while status messages and errors are written
to stderr. Stdout can therefore be piped directly to another process:

```sh
./gps_console /dev/serial0 1 | gnss_receiver
```

The `openmass.gps.v2` schema uses types that match the `AT+CGNSINF` table in the
SIMCom documentation:

- `double`: latitude, longitude, MSL altitude, speed, course, HDOP/PDOP/VDOP,
  and horizontal and vertical position accuracy;
- `uint32`: fix mode, GPS/GNSS/GLONASS satellite counts, and maximum C/N0;
- `bool`: GNSS running state and fix state;
- `string`: UTC in the modem's `yyyyMMddhhmmss.sss` format and reserved fields.

Numeric fields are `optional`, so a missing measurement is not confused with
zero. A record is also sent before a fix is acquired. Because the wire format
changed, the sender sets `schema_version` to 2.

If the HAT is connected over USB, the device may be `/dev/ttyUSB2` or another
`/dev/ttyUSB*` entry; identify it with `dmesg` or `ls -l /dev/ttyUSB*`. The
program assumes that the modem has already been powered on with the PWRKEY button
or the appropriate GPIO sequence. The first position fix is best acquired
outdoors with the GNSS antenna connected to the GNSS connector.

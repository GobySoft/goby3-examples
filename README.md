# goby3-examples
Examples for the Goby3 middleware (https://github.com/GobySoft/goby3)

## Compilation

Run `./build.sh` to compile the examples. You will need Goby3 installed or built from source (in the latter case, set the environmental variable GOBY3_EXAMPLES_CMAKE_FLAGS=-DGOBY_DIR=/path/to/goby3/build).
Once compiled, include the build directory to your system PATH.

```bash
export PATH=$PATH:/path/to/goby3-examples/build/bin
```

## Interthread

### Basic Multi-Threaded Publish/Subscribe

The `main.cpp` uses the main thread to spawn one publisher (`publisher.h`) and two subscribers (`subscriber.h`), then sits idle. The code is in `src/interthread/basic_publisher_subscriber`.

```
cd launch/interthread
goby_launch -x basic_interthread.launch
```

This will launch the standalone application `goby3_example_basic_interthread` with each thread's output shown in its own panel using NCurses.

When finished, type CTRL-C into the original terminal (the one from which `goby_launch` was run)


## Interprocess

### Basic Multi-Process Single-Threaded Publish/Subscribe

This example takes much of the same code from the basic interthread example above, and splits it into two single-threaded processes, communicating on the interprocess layer via the `gobyd`.

The code for this example is given in src/interprocess/basic_publisher and src/interprocess/basic_subscriber. To run using the default UNIX sockets use:

```
cd launch/interprocess
goby_launch -x basic_publisher_subscriber.launch
```

As will be apparent if you look at the contents of `basic_publisher_subscriber.launch`, this will launch `gobyd`, the publisher application, and two copies of the subscriber application, each in their own XTerm windows. When finished, type CTRL-C into the original terminal (the one from which `goby_launch` was run)


### GPS Driver
A working example using a standard NMEA-0183 GPS is given in `src/interprocess/gps_driver`.

This example consists of two processes: `goby3_example_gps_driver` which has three threads: one thread blocks reading the serial port, one subscribes to the data read by the first thread and writes it to the screen (as a proxy for doing some data analysis on it), while the third (the main thread) waits for control data published by another process. Based on the control data parameter, the main thread spawns or joins the reader thread. `gps_controller` is a simple application that writes the value from the configuration file, and then quits.

To run, you will need a GPS connected and know the serial port.

1. Start a `gobyd`

        gobyd

1. Launch the driver (substitute your actual serial port path for `/dev/ttyUSB0` and change the baud if necessary).

        cd build/bin
        ./goby3_example_gps_driver --serial_port=/dev/ttyUSB0 --serial_baud=4800 -v

1. Then you can disable GPS reading via

        cd build/bin
        ./goby3_example_gps_controller --read_gps=false

    And re-enable it by 

        ./goby3_example_gps_controller --read_gps=true

## Intervehicle


### Basic Multi-Vehicle Single-Threaded Publish/Subscribe

This example is similar to the basic interthread/interprocess examples above but splits it across two different vehicles that are connected via two different physical links (for ease of use, both of these links are demonstrated using UDP multicast with the goby::acomms::UDPMulticastDriver).

The code for this example is given in src/intervehicle/basic_publisher and src/intervehicle/basic_subscriber. To run using the configuration in launch/intervehicle/basic/*.pb.cfg:

```
cd launch/intervehicle/basic
# vehicle 1
goby_launch -x basic_vehicle1.launch

# vehicle 2
goby_launch -x basic_vehicle1.launch

```

As configured in the InterVehiclePortal configuration (here in `gobyd*.pb.cfg`), vehicle 1 is modem ID 1 (0x01) on one link and modem ID 17 (0x11) on the other link. Vehicle 2 is modem ID 2 (0x02) and 18 (0x12) on these same respective links. 

The subnet mask of `0xF0` allows for up to 16 address per subnet (though 0 is reserved for broadcast). The first link is `0xF0 & 0x01 == 0xF0 & 0x02 == 0x00`, and the second is `0xF0 & 0x11 == 0xF0 & 0x12 == 0x10`.

In this example, the vehicle 2 subscribe subscribes to vehicle 1 over both links. This means when both links are operational (as they should be here), all data will be duplicated once.

In general, it is a key responsibility of the intervehicle subscriber to be able to reject duplicated data. Duplicates could come from multiple transmissions with lost acknowledgments or from multiple links being successful (as is the case here).


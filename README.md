# goby3-examples
Examples for the Goby3 middleware (https://github.com/GobySoft/goby3)

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
Examples to be posted in the near future...

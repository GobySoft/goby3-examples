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

### Basic Multi-Process Single-Threaded Publish/Subscribe (ZeroMQ)

This example takes much of the same code from the basic interthread example above, and splits it into two single-threaded processes, communicating on the interprocess layer via the `gobyd` (ZeroMQ-based broker).

The code for this example is given in src/interprocess/zeromq/basic_publisher and src/interprocess/zeromq/basic_subscriber. To run using the default UNIX sockets use:

```
cd launch/interprocess/zeromq
goby_launch -x basic_publisher_subscriber.launch
```

As will be apparent if you look at the contents of `basic_publisher_subscriber.launch`, this will launch `gobyd`, the publisher application, and two copies of the subscriber application, each in their own XTerm windows. When finished, type CTRL-C into the original terminal (the one from which `goby_launch` was run)

### Basic Multi-Process Single-Threaded Publish/Subscribe (UDP Multicast)

This example is equivalent to the ZeroMQ example above but uses the UDP Multicast (udpm) interprocess transport instead. No central broker (`gobyd`) is required.

The code for this example is given in src/interprocess/udpm/basic_publisher and src/interprocess/udpm/basic_subscriber. To run:

```
cd launch/interprocess/udpm
goby_launch -x basic_publisher_subscriber.launch
```

This will launch the publisher application and two copies of the subscriber application, each in their own XTerm windows. When finished, type CTRL-C into the original terminal.

### Basic Multi-Process Publish/Subscribe in Julia (ZeroMQ)

This example is the Julia equivalent of the basic ZeroMQ publisher/subscriber above: the application logic is written in Julia (using the `Goby` Julia module installed with Goby), while the Goby application itself is a C++ shared library generated from an `interface.yml` describing what the application publishes and subscribes.

The code is given in `src/interprocess/zeromq/julia/basic_publisher` and `src/interprocess/zeromq/julia/basic_subscriber`. Each application consists of:

- `publisher.jl` / `subscriber.jl`: the application logic
- `interface.yml`: the publish/subscribe interface, from which the C++ bridge is generated
- `config.proto`: the application's configuration message

These examples are built only if `julia` is found on the `PATH` and [CxxWrap.jl](https://github.com/JuliaInterop/CxxWrap.jl) is installed:

```bash
julia -e 'import Pkg; Pkg.add("CxxWrap")'
```

They can be explicitly disabled with `-Denable_julia_examples=OFF`. The remaining Julia dependencies (ProtoBuf.jl, YAML.jl) are installed into `build/julia/Goby.jl` at build time.

Unlike the C++ examples, the Julia applications take the path to their configuration file as a positional argument. To run, first write a minimal configuration for each, e.g. `publisher.pb.cfg`:

```
app { name: "basic_julia_publisher" }
interprocess { platform: "vehicle1" }
```

then, with `gobyd` running:

```
julia build/julia/basic_julia_publisher/publisher.jl publisher.pb.cfg
julia build/julia/basic_julia_subscriber/subscriber.jl subscriber.pb.cfg
```

The publisher sends a `NavigationReport` at 10 Hz on the `julia_navigation` group, which the subscriber prints as it receives them.

### GPS Driver
A working example using a standard NMEA-0183 GPS is given in `src/interprocess/zeromq/gps_driver`.

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


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

### Basic Multi-Process Single-Threaded Publish/Subscribe in Python (ZeroMQ)

The same example written in Python instead of C++. The code is in `src/interprocess/zeromq/python/basic_publisher_subscriber`.

This requires Goby built and installed with `-Dbuild_python=ON`; the examples enable the Python examples automatically when that is the case (`cmake -Dbuild_python=OFF` to skip them).

```
cd launch/interprocess/zeromq
goby_launch -x python_basic_publisher_subscriber.launch
```

Because the group (`groups::nav`) and the message (`protobuf::NavigationReport`) are the same ones the C++ example uses, the applications interoperate: the launch file above runs the Python publisher with both the Python and the C++ subscriber, and either publisher feeds either subscriber.

A Goby Python application is two files. `publisher.yml` declares the publish/subscribe interface, which the build turns into the C++ glue that makes the statically typed Goby calls (Goby resolves groups, types and schemes at compile time, which Python cannot do at import time). `publisher.py` is the application itself, and looks like its C++ counterpart:

```python
class BasicPublisher(SingleThreadApplication):
    def __init__(self):
        super().__init__(loop_frequency_hertz=10)

    def loop(self):
        nav = nav_pb2.NavigationReport(x=..., y=..., z=...)
        self.interprocess().publish(groups.nav, nav)
```

Command line and configuration file handling are the same as for a C++ application, so `goby3_example_python_basic_publisher --help`, `--example_config` and `--my_value 10` all work as expected.

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

To run:

```
cd launch/interprocess/zeromq
goby_launch -x julia_publisher_subscriber.launch
```

This launches `gobyd`, the Julia subscriber, and the Julia publisher, each in their own XTerm window. When finished, type CTRL-C into the original terminal (the one from which `goby_launch` was run).

The publisher sends a `NavigationReport` at 10 Hz on the `groups::julia_nav` group, which the subscriber prints as it receives them. The group is named through the module `goby_add_julia_app()` generates from `interface.yml` beside each application (`basic_julia_publisher_goby.jl`), so `groups.julia_nav` and `interprocess()` stand in for the group string and the layer constant. The expression then lives in one place, and a mistyped group is an `UndefVarError` naming it rather than a `GOBY_JULIA_FAIL` that terminates the application once the publish is reached.

Unlike the C++ applications, the Julia applications take the path to their configuration file as their only argument, so `basic_julia_publisher.pb.cfg` and `basic_julia_subscriber.pb.cfg` are provided alongside the launch file:

```
basic_julia_publisher <config.pb.cfg>
basic_julia_subscriber <config.pb.cfg>
```

### Multi-Threaded Publish/Subscribe in Julia (ZeroMQ)

`src/interprocess/zeromq/julia/basic_multithread` is the Julia equivalent of the interthread publisher/subscriber, and shows the layers available to a Julia task.

`Goby.run(app, Main, [PublisherTask, SubscriberTask])` runs each task module on its own Julia thread. The publisher task sends `NavigationReport`s over `INTERTHREAD`, which is implemented in Julia over `Channel`s rather than in the generated C++ -- so its groups are plain strings chosen by the application rather than groups declared in `interface.yml`, and its messages can be any Julia value (the depth alert `Main` receives is a `NamedTuple`).

The subscriber task publishes the deep reports out to `INTERPROCESS` itself. Goby.jl hands those to the task that owns the C++ application, the way a C++ thread's `InterProcessForwarder` does, so no task needs a portal of its own and none has to route through `Main`. Subscribing works the same way: the publisher task subscribes to the same group and sees each report that goes out, alongside the separate `basic_julia_subscriber` process.

Julia fixes its thread count at startup, so the application needs one thread per task module plus three, for `Main`, the loop timer and the C++ application. `goby_add_julia_app()`'s `THREADS` gives that to the generated launcher.

To run:

```
cd launch/interprocess/zeromq
goby_launch -x julia_multithread.launch
```

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


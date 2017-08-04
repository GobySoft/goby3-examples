# goby3-examples
Examples for the Goby3 middleware (https://github.com/GobySoft/goby3)

## Interprocess
A basic single-thread interprocess publish/subscribe example is given in src/single-thread/basic_publisher and src/single-thread/basic_subscriber. To run using the default UNIX sockets use:
```
gobyd
```
```
cd build/bin
./basic_publisher
```
```
cd build/bin
./basic_subscriber
```
To configure for TCP or other parameters, use the '-h' flag to see command line options or the '-e' flag to see an example configuration file.

## Interthread

### Basic Multi-Threaded Publish/Subscribe 

This takes much of the same code from the basic single-thread example above, and moves it inside a single process, using the InterThreadTransporter instead. The ```main.cpp``` uses the main thread to spawn one publisher (```publisher.h```) and two subscribers (```subscriber.h```), then sits idle. The code is in ```src/multi-thread/basic_publisher_subscriber```. Even though nothing is published in this basic example on the interprocess level, you still need a running ```gobyd```.

```
gobyd
```

```
cd build/bin
./basic_multithread_pubsub -v
```


### GPS Driver
A working example using a standard NMEA-0183 GPS is given in ```src/multi-thread/gps_driver```.

This example consists of two processes: ```gps_driver``` which has two threads: one thread blocks reading the serial port, while the other (the main thread) waits for control data published by another process. Based on the control data parameter, the main thread spawns or joins the reader thread. ```gps_controller``` is a simple application that writes the value from the configuration file, and then quits.

To run, you will need a GPS connected and know the serial port:
```
gobyd
```

```
cd build/bin
./gps_driver --serial_port=/dev/ttyUSB0 --serial_baud=4800
```

Then you can disable GPS reading via
```
cd build/bin
./gps_controller --read_gps=false
```

And re-enable it by 
```
./gps_controller --read_gps=true
```


## Intervehicle
Examples to be posted in the near future...

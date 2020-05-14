#!/bin/bash

# writes "DEADBEEF" to can_id 0x1b4 used by goby_example_can at 1 Hz

while [ 1 ]; do
    cansend vcan0 1b4#DEADBEEF
    sleep 1
done

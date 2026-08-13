#!/usr/bin/env python3
"""Python version of src/interprocess/zeromq/basic_publisher.

Publishes a NavigationReport on the "navigation" group at 10 Hz. The group and the message are
the same ones the C++ example uses, so this publisher can feed the C++ subscriber and vice versa.
"""

import random
import sys

import goby

# generated from publisher.yml by goby_add_python_app(); provides the application base class and
# the groups declared in the interface file
from basic_python_publisher_goby import SingleThreadApplication, groups

# protoc --python_out of src/messages/nav.proto
from messages import nav_pb2


class BasicPublisher(SingleThreadApplication):
    def __init__(self):
        # loop() is called at this frequency; leave it out for an event driven application
        super().__init__(loop_frequency_hertz=10)

        # all configuration defined in BasicPythonPublisherConfig is available using self.cfg
        print(f"My configuration int is: {self.cfg.my_value}")

    def loop(self):
        nav = nav_pb2.NavigationReport(
            x=95 + random.randrange(20),
            y=195 + random.randrange(20),
            z=-305 + random.randrange(10),
        )

        print(f"Tx: {nav}", end="", flush=True)
        self.interprocess().publish(groups.nav, nav)


# reads command line parameters based on the BasicPythonPublisherConfig definition, exactly as a
# C++ application does (try "--help" to see the parameters, or "--example_config" for the correct
# configuration file syntax). Edit "config.proto" to add parameters.
if __name__ == "__main__":
    sys.exit(goby.run(BasicPublisher))

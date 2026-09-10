#!/usr/bin/env python3
"""Python version of src/interthread/basic_publisher_subscriber, showing the INTERTHREAD layer.

A publisher thread produces NavigationReports, a watcher thread consumes them and forwards each
one to the rest of the Goby system, and the application prints an alert when the vehicle is deep.

Three things this example exists to show:

  1. INTERTHREAD is pure Python. Goby carries these messages between threads without marshalling
     them, so they never enter the generated C++ and interthread groups are not declared in
     interface.yml -- any string will do. The application is a SingleThreadApplication: the C++
     side has no idea these threads exist.
  2. Because of that, an interthread message can be any Python object, not only a protobuf
     message. The alert below is a dataclass.
  3. INTERPROCESS belongs to the application's thread, but a thread publishes to it directly
     anyway: the publication is handed to the application behind the scenes, as a C++ thread's
     InterProcessForwarder hands its traffic to the portal.

Run it as any other Goby application:

    goby3_example_python_multithread --alert_depth 300
"""

import random
import sys
from dataclasses import dataclass

import goby

# generated from interface.yml by goby_add_python_app(); provides the application and thread base
# classes, and the groups declared in the interface file
from basic_python_multithread_goby import SingleThreadApplication, Thread, groups

# protoc --python_out of src/messages/nav.proto
from messages import nav_pb2

# interthread groups are Python-side only, so they are named here rather than in interface.yml
NAV_GROUP = "nav"
DEPTH_ALERT_GROUP = "depth_alert"


@dataclass
class DepthAlert:
    """An interthread message that is not a protobuf message, and never becomes one."""

    depth: float
    x: float
    y: float


class NavPublisher(Thread):
    """Produces reports on the interthread layer at its own rate."""

    def __init__(self):
        super().__init__(loop_frequency_hertz=1)

    def loop(self):
        nav = nav_pb2.NavigationReport(
            x=95 + random.randrange(20),
            y=195 + random.randrange(20),
            z=-305 + random.randrange(10),
        )

        print(f"[publisher] Tx: z={nav.z:.1f}")
        self.interthread().publish(NAV_GROUP, nav)


class DepthWatcher(Thread):
    """Consumes reports, forwards them outside the process, and alerts on deep ones."""

    def __init__(self):
        # no loop frequency: this thread is woken by its subscription rather than by a clock
        super().__init__()

        # subscribing in the constructor is what a C++ SimpleThread does, and works for the same
        # reason: the thread is constructed on the thread it runs on, so the callback lands here
        self.interthread().subscribe(NAV_GROUP, self.incoming_nav)

    def incoming_nav(self, nav: nav_pb2.NavigationReport):
        print(f"[watcher] Rx: z={nav.z:.1f}")

        # the interprocess portal lives on the application's thread; publishing to it from here
        # hands it over rather than touching the portal
        self.interprocess().publish(groups.nav, nav)

        if -nav.z > self.cfg.alert_depth:
            self.interthread().publish(
                DEPTH_ALERT_GROUP, DepthAlert(depth=-nav.z, x=nav.x, y=nav.y)
            )


class BasicMultithread(SingleThreadApplication):
    def __init__(self):
        # no loop frequency: launching a thread raises the rate the application is serviced at,
        # which is what picks up the alerts below
        super().__init__()

        print(f"Alerting below {self.cfg.alert_depth} m")

        self.interthread().subscribe(DEPTH_ALERT_GROUP, self.incoming_depth_alert)

        self.launch_thread(NavPublisher)
        self.launch_thread(DepthWatcher)

    # the annotation names the type this subscription takes, as it does for a protobuf message
    def incoming_depth_alert(self, alert: DepthAlert):
        print(f"[main] alert: {alert.depth:.1f} m at x={alert.x:.1f} y={alert.y:.1f}")


# reads command line parameters based on the BasicPythonMultithreadConfig definition, exactly as a
# C++ application does (try "--help" to see the parameters, or "--example_config" for the correct
# configuration file syntax). Edit "multithread_config.proto" to add parameters.
if __name__ == "__main__":
    sys.exit(goby.run(BasicMultithread))

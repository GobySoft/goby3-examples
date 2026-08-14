#!/usr/bin/env python3
"""Python version of src/interprocess/zeromq/basic_subscriber.

Prints every NavigationReport published on the "navigation" group.
"""

import sys

import goby

# generated from subscriber.yml by goby_add_python_app()
from basic_python_subscriber_goby import SingleThreadApplication, groups

from messages import nav_pb2


class BasicSubscriber(SingleThreadApplication):
    def __init__(self):
        # event driven subscriber: no loop frequency, so no loop() method either
        super().__init__()

        # subscribe to a group for a given message type, and call incoming_nav with each message.
        # The message type is read from the callback's type annotation; pass it explicitly if you
        # would rather not annotate:
        #   self.interprocess().subscribe(groups.nav, nav_pb2.NavigationReport, self.incoming_nav)
        self.interprocess().subscribe(groups.nav, self.incoming_nav)

    # called each time a NavigationReport on the group "navigation" is received
    def incoming_nav(self, nav: nav_pb2.NavigationReport) -> None:
        print(f"Rx: {nav}", end="", flush=True)


if __name__ == "__main__":
    sys.exit(goby.run(BasicSubscriber))

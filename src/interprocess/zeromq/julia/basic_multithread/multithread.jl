# Julia equivalent of src/interthread/basic_publisher_subscriber, showing the INTERTHREAD layer
# and pub/sub from a task rather than from Main.
#
# A publisher task produces NavigationReports on INTERTHREAD; a subscriber task consumes them,
# and publishes the deep ones straight out to INTERPROCESS. Those reach every application on the
# system that subscribed to the group (basic_julia_subscriber, in julia_multithread.launch), this
# one included: the publisher task subscribed to the same group and sees each one.
#
# Four things this example exists to show:
#
#   1. INTERTHREAD is pure Julia. Goby.jl carries these messages between tasks over Channels,
#      so they never enter the generated C++ and interthread groups are not declared in
#      interface.yml -- any string will do.
#   2. Because of that, an interthread message can be any Julia value, not only a protobuf
#      message. The alert Main receives below is a NamedTuple.
#   3. INTERPROCESS works from any task. Goby.jl hands the message to the task that owns the C++
#      application, the way a C++ thread's InterProcessForwarder does, so a task neither needs a
#      portal of its own nor has to route through Main.
#   4. Subscribing from a task works the same way: the subscription is registered on the task
#      that owns the application, and messages come back to the task that asked for them.
#
# Julia fixes its thread count at startup, and Goby.run() needs one thread per task module plus
# three (Main, the loop timer, and the C++ application). Two task modules therefore need five,
# which goby_add_julia_app()'s THREADS gives the launcher:
#
#   build/bin/basic_julia_multithread basic_julia_multithread.pb.cfg

include(joinpath(@__DIR__, "..", "goby_application_init.jl"))
init_cxxwrap("basic_julia_multithread")
include_protos(@__MODULE__, "goby3_examples")

# the groups and layer accessors generated from interface.yml, written beside this script by
# goby_add_julia_app()
include(joinpath(@__DIR__, "basic_julia_multithread_goby.jl"))
using .BasicJuliaMultithreadGoby: groups, interprocess

# interthread groups are Julia-side only, so they are named here rather than in interface.yml
const NAV_GROUP = "nav"
const DEPTH_ALERT_GROUP = "depth_alert"

# ---------------------------------------------------------------------------
# Publisher task: produces reports, and watches the ones that reach interprocess.
# ---------------------------------------------------------------------------
module PublisherTask

using Goby

function loop()
    nav = Main.goby3_examples.protobuf.NavigationReport(
        x = 95.0 + rand() * 20,
        y = 195.0 + rand() * 20,
        z = -305.0 + rand() * 10,
    )

    println("[publisher] Tx: z=$(round(nav.z, digits = 1))")
    Goby.publish(Main.app, Goby.INTERTHREAD, Main.NAV_GROUP, nav)
end

function published_nav(nav::Main.goby3_examples.protobuf.NavigationReport)
    println("[publisher] on the wire: z=$(round(nav.z, digits = 1))")
end

# subscriptions belong in start(), which Goby.run() calls on this task's own thread
function start()
    Goby.subscribe(Main.app, Main.interprocess(), Main.groups.julia_nav, published_nav)
end

# read by Goby.run() to drive loop() from the timer thread
goby_cfg = Dict(
    :loop_frequency => 1,   # Hz
    :loop_function => loop,
)

end # module PublisherTask

# ---------------------------------------------------------------------------
# Subscriber task: consumes reports, and sends the deep ones to the outside.
# ---------------------------------------------------------------------------
module SubscriberTask

using Goby

function incoming_nav(nav::Main.goby3_examples.protobuf.NavigationReport)
    println("[subscriber] Rx: z=$(round(nav.z, digits = 1))")

    if -nav.z > Main.alert_depth
        # straight out of this task: no portal here, and no hop through Main
        Goby.publish(Main.app, Main.interprocess(), Main.groups.julia_nav, nav)

        # an interthread message is any Julia value; this one never becomes protobuf
        Goby.publish(Main.app, Goby.INTERTHREAD, Main.DEPTH_ALERT_GROUP,
                     (depth = -nav.z, x = nav.x, y = nav.y))
    end
end

function start()
    Goby.subscribe(Main.app, Goby.INTERTHREAD, Main.NAV_GROUP, incoming_nav)
end

end # module SubscriberTask

# ---------------------------------------------------------------------------
# Main: reports the alerts the subscriber task raises.
# ---------------------------------------------------------------------------

function incoming_depth_alert(alert)
    println("[main] alert: $(round(alert.depth, digits = 1)) m at " *
            "x=$(round(alert.x, digits = 1)) y=$(round(alert.y, digits = 1))")
end

function start()
    Goby.subscribe(app, Goby.INTERTHREAD, DEPTH_ALERT_GROUP, incoming_depth_alert)
end

# how often the C++ application drains the channels the tasks publish to it through
goby_cfg = Dict(:cxx_channel_check_frequency => 10)

config_str = Goby.read_cli_cfg()
app = Goby.BasicJuliaMultithread(config_str)

# the application's own configuration, decoded into the Julia bindings generated from
# multithread_config.proto by goby_add_julia_protos()
pb_cfg = Goby.cfg(app, goby3_examples.config.BasicJuliaMultithreadConfig)
alert_depth = pb_cfg.alert_depth
println("Alerting below $(alert_depth) m")

Goby.run(app, Main, [PublisherTask, SubscriberTask])

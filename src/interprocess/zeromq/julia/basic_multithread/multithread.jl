# Julia equivalent of src/interthread/basic_publisher_subscriber, showing the INTERTHREAD layer.
#
# A publisher task produces NavigationReports, a subscriber task consumes them and raises an
# alert when the vehicle is deep, and Main forwards each report on to INTERPROCESS so the rest
# of the Goby system sees it.
#
# Three things this example exists to show:
#
#   1. INTERTHREAD is pure Julia. Goby.jl carries these messages between tasks over Channels,
#      so they never enter the generated C++ and interthread groups are not declared in
#      interface.yml -- any string will do.
#   2. Because of that, an interthread message can be any Julia value, not only a protobuf
#      message. The alert below is a NamedTuple.
#   3. INTERPROCESS is only available from Main, so a task that needs the outside world hands
#      the message to Main over INTERTHREAD, as the subscriber task does here. Publishing to it
#      from a task instead raises an AssertionError naming the task, which stops the
#      application.
#
# Julia fixes its thread count at startup, and Goby.run() needs one thread per task module plus
# three (Main, the loop timer, and the C++ application). Two task modules therefore need five,
# which goby_add_julia_app()'s THREADS gives the launcher:
#
#   build/bin/basic_julia_multithread basic_julia_multithread.pb.cfg

include(joinpath(@__DIR__, "..", "goby_application_init.jl"))
init_cxxwrap("basic_julia_multithread")
include_protos(@__MODULE__, "goby3_examples")

# interthread groups are Julia-side only, so they are named here rather than in interface.yml
const NAV_GROUP = "nav"
const DEPTH_ALERT_GROUP = "depth_alert"

# ---------------------------------------------------------------------------
# Publisher task: runs on its own thread, publishing at the rate in goby_cfg.
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

# read by Goby.run() to drive loop() from the timer thread
goby_cfg = Dict(
    :loop_frequency => 1,   # Hz
    :loop_function => loop,
)

end # module PublisherTask

# ---------------------------------------------------------------------------
# Subscriber task: consumes reports and reports deep ones back to Main.
# ---------------------------------------------------------------------------
module SubscriberTask

using Goby

function incoming_nav(nav::Main.goby3_examples.protobuf.NavigationReport)
    println("[subscriber] Rx: z=$(round(nav.z, digits = 1))")

    if -nav.z > Main.alert_depth
        # an interthread message is any Julia value; this one never becomes protobuf
        Goby.publish(Main.app, Goby.INTERTHREAD, Main.DEPTH_ALERT_GROUP,
                     (depth = -nav.z, x = nav.x, y = nav.y))
    end
end

# subscriptions belong in start(), which Goby.run() calls on this task's own thread
function start()
    Goby.subscribe(Main.app, Goby.INTERTHREAD, Main.NAV_GROUP, incoming_nav)
end

end # module SubscriberTask

# ---------------------------------------------------------------------------
# Main: bridges the tasks to the rest of the Goby system.
# ---------------------------------------------------------------------------

function forward_nav(nav::goby3_examples.protobuf.NavigationReport)
    Goby.publish(app, Goby.INTERPROCESS, "groups::julia_nav", nav)
end

function incoming_depth_alert(alert)
    println("[main] alert: $(round(alert.depth, digits = 1)) m at " *
            "x=$(round(alert.x, digits = 1)) y=$(round(alert.y, digits = 1))")
end

function start()
    Goby.subscribe(app, Goby.INTERTHREAD, NAV_GROUP, forward_nav)
    Goby.subscribe(app, Goby.INTERTHREAD, DEPTH_ALERT_GROUP, incoming_depth_alert)
end

# how often the C++ application drains the channel Main publishes to it through
goby_cfg = Dict(:cxx_channel_check_frequency => 10)

config_str = Goby.read_cli_cfg()
app = Goby.BasicJuliaMultithread(config_str)

# the application's own configuration, decoded into the Julia bindings generated from
# multithread_config.proto by goby_add_julia_protos()
pb_cfg = Goby.cfg(app, goby3_examples.config.BasicJuliaMultithreadConfig)
alert_depth = pb_cfg.alert_depth
println("Alerting below $(alert_depth) m")

Goby.run(app, Main, [PublisherTask, SubscriberTask])

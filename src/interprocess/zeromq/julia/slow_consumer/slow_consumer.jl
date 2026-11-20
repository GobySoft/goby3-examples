# What a Julia INTERTHREAD publisher does when a subscriber cannot keep up.
#
# A producer task publishes work faster than a slow consumer task can handle it. A monitor task
# subscribes to the same group and handles each item at once. Main reports, once a second, what
# each subscription has delivered and dropped, and publishes the same numbers on INTERPROCESS.
#
# Each INTERTHREAD subscription queues messages for its own task, and the configuration chooses
# what a publication does when the slow consumer's queue is full:
#
#   overflow: "drop_oldest"   the default here: the producer keeps its rate, the consumer
#                             handles the latest items, and the oldest are dropped
#   overflow: "block"         the producer waits for room, so everything slows to the
#                             consumer's rate -- the monitor included
#   overflow: "drop_newest"   the producer keeps its rate, and new items are dropped instead
#   overflow: "error"         publish throws InterthreadOverflowError, which stops the
#                             producer task and with it the application
#
# The monitor's subscription has its own queue, so with any policy but block it keeps up however
# far behind the consumer falls. The consumer is a task module of its own for the same reason: a
# callback runs on its task's thread, so a slow one delays everything else that task does.
#
#   build/bin/basic_julia_slow_consumer basic_julia_slow_consumer.pb.cfg

include(joinpath(@__DIR__, "..", "goby_application_init.jl"))
init_cxxwrap("basic_julia_slow_consumer")
include_protos(@__MODULE__, "goby3_examples")

include(joinpath(@__DIR__, "basic_julia_slow_consumer_goby.jl"))
using .BasicJuliaSlowConsumerGoby: groups, interprocess

# read first: the task modules below take their rates and queue settings from it
config_str = Goby.read_cli_cfg()
app = Goby.BasicJuliaSlowConsumer(config_str)
pb_cfg = Goby.cfg(app, goby3_examples.config.BasicJuliaSlowConsumerConfig)
println("Producing at $(pb_cfg.producer_rate) Hz for a consumer taking $(pb_cfg.consumer_delay) s " *
        "an item, queueing $(pb_cfg.capacity) with overflow $(pb_cfg.overflow)")

const WORK_GROUP = "work"

struct Work
    id::Int
    published::Float64
end

module ProducerTask

using Goby

const produced = Threads.Atomic{Int}(0)

function loop()
    id = Threads.atomic_add!(produced, 1) + 1
    Goby.publish(Main.app, Goby.INTERTHREAD, Main.WORK_GROUP, Main.Work(id, time()))
end

goby_cfg = Dict(:loop_frequency => Main.pb_cfg.producer_rate, :loop_function => loop)

end # module ProducerTask

module SlowConsumerTask

using Goby

function handle(work::Main.Work)
    sleep(Main.pb_cfg.consumer_delay)
    println("[consumer] handled work $(work.id), $(round(time() - work.published, digits = 2)) s after it was published")
end

function start()
    Goby.subscribe(Main.app, Goby.INTERTHREAD, Main.WORK_GROUP, handle;
                   capacity = Main.pb_cfg.capacity, overflow = Symbol(Main.pb_cfg.overflow))
end

end # module SlowConsumerTask

module MonitorTask

using Goby

const max_latency = Ref(0.0)

function watch(work::Main.Work)
    max_latency[] = max(max_latency[], time() - work.published)
end

start() = Goby.subscribe(Main.app, Goby.INTERTHREAD, Main.WORK_GROUP, watch)

end # module MonitorTask

function stats_for(stats, task)
    index = findfirst(s -> s.group == WORK_GROUP && s.task == task, stats)
    return index === nothing ? nothing : stats[index]
end

function report()
    stats = Goby.interthread_stats()
    slow = stats_for(stats, :SlowConsumerTask)
    monitor = stats_for(stats, :MonitorTask)
    (slow === nothing || monitor === nothing) && return

    msg = goby3_examples.protobuf.InterthreadQueueStats(
        produced = ProducerTask.produced[],
        slow_delivered = slow.delivered,
        slow_dropped = slow.dropped,
        slow_queued = slow.queued,
        monitor_delivered = monitor.delivered,
        monitor_max_latency = MonitorTask.max_latency[],
    )
    println("[main] produced $(msg.produced); consumer: $(msg.slow_delivered) handled, " *
            "$(msg.slow_dropped) dropped, $(msg.slow_queued) queued; monitor: " *
            "$(msg.monitor_delivered) handled, worst latency $(round(msg.monitor_max_latency, digits = 3)) s")
    Goby.publish(app, interprocess(), groups.julia_queue_stats, msg)
end

goby_cfg = Dict(:loop_frequency => 1, :loop_function => report)

Goby.run(app, Main, [ProducerTask, SlowConsumerTask, MonitorTask])

# Example test harnesses

Each `*.test.yml` file here is a test definition for `goby test` (see `goby help test`
and the goby3 command line tool documentation). It names the applications that must be alive, then publishes
messages into the running example and checks what comes back, on which group,
with which contents, and how quickly.

`run_test.sh` launches the example's `.launch` file(s), runs the test, and
shuts the example down:

```bash
./build.sh
test/run_test.sh test/interprocess/zeromq/basic_publisher_subscriber.test.yml \
    launch/interprocess/zeromq/basic_publisher_subscriber.launch
```

The exit code is 0 when every
expectation passed, 1 when one failed, and 2 when the test could not be set up
(for example a required application never answered the health request).

To get a machine-readable result instead, forward options to the tool:

```bash
TEST_ARGS=(--output_format json) test/run_test.sh ...
```

The same tests are registered with ctest when the examples are configured with
`-Denable_tests=ON`:

```bash
cd build && ctest --output-on-failure
```

| Test | Example | What it checks |
|---|---|---|
| `interprocess/zeromq/basic_publisher_subscriber` | basic ZeroMQ pub/sub | publisher rate and value ranges; an externally published report is visible on the group |
| `interprocess/zeromq/state_machine` | state machine | `StateControl` → `StateReport` round trip for ON/OFF; a redundant command causes no extra transition |
| `interprocess/zeromq/json` | JSON marshalling | typed and untyped JSON messages flow in parallel; hand-published JSON is visible |
| `interprocess/udpm/basic_publisher_subscriber` | basic UDPM pub/sub | as the ZeroMQ basic test, without a broker |
| `intervehicle/basic` | two vehicles over UDP multicast modem | DCCL report from vehicle1 reaches vehicle2's interprocess layer; nothing echoes back to vehicle1 |

Groups defined with `GOBY_DEFINE_INTERVEHICLE_GROUP` (such as `groups::nav`) appear on the
interprocess layer with a numeric suffix, e.g. `groups::nav;0`, so expectations match them with
a regex such as `"groups::nav(;.*)?"` and publications name the suffixed group.

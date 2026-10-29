# Example test harnesses

Each `*.test.yml` file here is a test definition for `goby test` (see `goby help test`
and the goby3 command line tool documentation). It names the applications that must be alive, then publishes
messages into the running example and checks what comes back, on which group,
with which contents, and how quickly.

Each file also declares its example's launch file(s) as fixtures: `goby test run`
starts them with `goby_launch`, waits for "All processes running", runs the test,
and stops them again. With the examples built and their binaries and message
library available:

```bash
./build.sh
export PATH=$PWD/build/bin:$PATH
export GOBY_TOOL_LOAD_SHARED_LIBRARY=$PWD/build/lib/libgoby3_example_messages.so
goby test run test/interprocess/zeromq/basic_publisher_subscriber.test.yml
```

The exit code is 0 when every expectation passed, 1 when one failed, and 2 when
the test could not run (for example a required application never answered the
health request). Add `--output_format JSON` for a machine-readable report.
`goby test validate <file>` checks a file without running anything, and
`goby test visualize <file> | dot -Tpng -o plan.png` draws its plan.

The same tests are registered with ctest when the examples are configured with
`-Denable_tests=ON` (which requires a goby with `goby test validate`); each file
gets a `validate` test and a `test` that runs it:

```bash
cd build && ctest --output-on-failure
```

| Test | Example | What it checks |
|---|---|---|
| `interprocess/zeromq/basic_publisher_subscriber` | basic ZeroMQ pub/sub | publisher rate and value ranges; an externally published report is visible on the group |
| `interprocess/zeromq/state_machine` | state machine | `StateControl` → `StateReport` round trip for ON/OFF; a redundant command causes no extra transition |
| `interprocess/zeromq/json` | JSON marshalling | typed and untyped JSON messages flow in parallel; hand-published JSON is visible |
| `interprocess/udpm/basic_publisher_subscriber` | basic UDPM pub/sub | as the ZeroMQ basic test, without a broker |
| `intervehicle/basic` | two vehicles over UDP multicast modem | DCCL report is visible on vehicle1 and arrives on vehicle2 as `DCCLForwardedData` on `modem_data_in` |

Groups defined with `GOBY_DEFINE_INTERVEHICLE_GROUP` (such as `groups::nav`) appear on the
interprocess layer with a numeric suffix, e.g. `groups::nav;0`, so expectations match them with
a regex such as `"groups::nav(;.*)?"` and publications name the suffixed group.

# PTY Example

`goby3_example_serial` opens an existing serial port and connects it to the publish/subscribe interface of SerialThreadLineBased.

At a minimum to run this example:

```
gobyd
socat pty,link=/tmp/ttytest1,raw,echo=0 pty,link=/tmp/ttytest2,raw,echo=0
goby3_example_serial --serial 'port: "/tmp/ttytest1" end_of_line: "\r\n" baud: 9600' -vvv
```

This creates a connected pair of pseudoterminals (virtual serial ports) using `socat` and connects `goby3_example_serial` to one of them (`/tmp/ttytest1`). Any data written to `/tmp/ttytest2` will show up as a SerialThreadLineBased publication. For example:

```
printf 'hello world\r\n' > /tmp/ttytest2
```

Alternatively, you can use a real serial device (e.g. a GPS) instead of `socat`.
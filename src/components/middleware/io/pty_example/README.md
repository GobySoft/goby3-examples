# PTY Example

`goby3_example_pty` opens a pseudoterminal (pty) and connects the "master" end to the publish/subscribe interface of PTYThreadLineBased. The slave end is at `/tmp/ttygoby3-example` by default, or at the path specified by `pty_config.port`.

The usual use of the PTYThreadLineBased is to create "virtual serial ports" for emulation/simulation of serial based sensors  entirely in software.

At a minimum to run this example:

```
gobyd
goby3_example_pty -vvv
```

Then write data into `/tmp/ttygoby3-example`.

To change the virtual serial port location, baud, or end-of-line delimiter use, for example:

```
goby3_example_pty -vvv --pty_config 'port: "/tmp/ttytest" end_of_line: "\r\n" baud: 9600'
```
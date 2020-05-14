# CAN Example

`goby3_example_can` opens a virtual CAN interface (vcan0) and reads data as ID 0x1b4. 

You can set up the virtual CAN interface using:

```
sudo setup_vcan.sh
```

Additionally, the script `cansend.sh` writes `DEADBEEF` at 1 Hz to ID 0x1b4 so you can see data arriving on `goby3_example_can`.

In short, to run this example,

```
sudo ./setup_vcan.sh
gobyd
# in a new terminal window
goby3_example_can -vvv
# in a new terminal window
./cansend.sh
```
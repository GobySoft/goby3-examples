This example demonstrates two vehicles exchanging position status information using the SimpleStatus message defined in `goby3-examples/src/components/moos/gobyexample_protobuf/simple_status.proto`

To generate the basic example:

```
> ./generate.sh
```

To run:

```
(probably separate windows / workspaces)
> pAntler mm1.moos
> pAntler mm2.moos
```

To generate the example using the Micro-Modem driver instead of the UDP Multicast Driver, use

```
> ./generate.sh mmdriver
```


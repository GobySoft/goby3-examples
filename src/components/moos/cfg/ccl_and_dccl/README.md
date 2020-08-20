This example extends the `dual_status` example with two vehicles exchanging position status information, but with one vehicle (Modem ID 1) using the CCL MDAT State message and the other (Modem ID 2) using the DCCL SimpleStatus message.

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


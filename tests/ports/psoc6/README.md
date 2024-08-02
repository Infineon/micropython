# PSoC6 Port Tests

The PSoc6 tests are organized in the following way:

```
psoc6/
├─ hw_ext/              --> Tests require board with extended hardware config
│  ├─ img/              --> Boards hardware extension diagrams
│  ├─ multi/            --> Tests require multiple boards instances
│  ├─ single/           --> Tests require a single board
│  ├─ README
├─ mpremote/            --> Tests based on mpremote
├─ stand_alone/         --> Tests only require the boards (no additional hardware)
│  ├─ multi/            --> Tests require multiple boards instances 
│  ├─ single/           --> Tests require a single board
├─ run_psoc6_tests.sh   --> Script handling PSoC6 tests
├─ README
```

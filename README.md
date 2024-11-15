# pettson
A low level json parser written in c.

This parser should ideally be used to populate a hash table or similar intermediate data structure.

See `functional/sample.c` for a slightly bloated sample/functional test.

```bash
make clean && make sample && valgrind ./sample 2>&1
```


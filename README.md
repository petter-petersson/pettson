# pettson
A low level json parser written in c.

This parser should ideally be used to populate a hash table or similar intermediate data structure.
See sample.c for usage.

See json-tests.c for usage

```c

  char * buf;

  ...

  json_context_t ctx;
  json_init_context(&ctx, buf);

  json_obj_t * res = NULL;
  //assuming we know the json type
  res = json_read_array(NULL, &ctx);

  parse_array(1, res);

  json_object_destroy(res, &ctx);

  ...

```


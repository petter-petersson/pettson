#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include "json.h"

static int test_passed = 0;
static int test_failed = 0;
static int checks_run = 0;

#define fail()  return __LINE__

#define check(cond) do { if (!(cond)) fail(); checks_run++;  } while (0)

static void test(int (*func)(void), const char *name) {
  int r = func();
  if (r == 0) {
    test_passed++;
  } else {
    test_failed++;
    printf("FAILED: %s (at line %d)\n", name, r);
  }
}

static int read_json_file(char ** buf, char * filename){
  struct stat st; 

  if (stat(filename, &st) != 0) {
    return 1;
  }
  printf("file size: %lld \n", st.st_size);

  *buf = malloc(sizeof(char) * (st.st_size + 1));
  //*buf = calloc((st.st_size + 1), sizeof(char));
  //memset(*buf, 0, sizeof(char) * (st.st_size + 1));

 
  if(*buf == NULL){
    printf("failed to malloc buf\n");
    return 2;
  }
  
  FILE *fp = fopen(filename, "rb");

  fread((*buf), 1, st.st_size, fp);
  (*buf)[st.st_size]='\0';

  fclose(fp);

  return 0;
}


#define TEST_FILE_1 "fixtures/mock.json" 

int test_read_array(){

  char * js_str = "[{\"apa\": \"123\"},{\"apa\": \"123\"},{\"apa\": \"123\"},{\"apa\": \"123\"},{\"apa\": \"123\"}]";
  json_context_t ctx;
  int init = json_init_context(&ctx, js_str);
  check(init == 0);

  json_obj_t *parent = NULL;

  json_obj_t *res;

  res = json_read_array(parent, &ctx);

  json_obj_t * iterator = res->children;
  while(iterator != NULL){
    check(strncmp(iterator->str, "{\"apa\": \"123\"}", iterator->length) == 0);
    iterator = iterator->next_sibling;
  }

  json_object_destroy(res, &ctx);

  return 0;
}

int test_read_object(){

  char * js_str = "{\"apa\": {\"gnu\": 321}, \"lennart\": "
                  "{\"samoa\": \"gitarr\"}, \"sune\": [{}, 123 , \"apan\", {\"seppo\":321}]}";
  json_context_t ctx;
  int init = json_init_context(&ctx, js_str);
  check(init == 0);

  json_obj_t *parent = NULL;

  json_obj_t *res;

  res = json_read_object(parent, &ctx);

  check(ctx.object_count == 23);
  check(ctx.object_retained == 0);

  json_object_destroy(res, &ctx);

  check(ctx.object_retained == 23);

  return 0;
}

int test_read_invalid_object(){

  char * js_str = "{\"apa\": {\"gnu\": 321}, \"lennart\": {\"samoa\": " 
                  "\"gitarr\"}, \"sune\": [{}, 123 , \"apan\", {\"seppo\":321}}]";

  json_context_t ctx;
  int init = json_init_context(&ctx, js_str);
  check(init == 0);

  json_obj_t *res;

  res = json_read_object(NULL, &ctx);

  check(res == NULL);

  printf("retained: %d \n",ctx.object_retained);

  return 0;
}

/* TODO: check utf-8 support */
int test_read_invalid_primitive(){

  char * js_str = "{\"apa\": {\"gnu\": 3ö21}, \"lennart\": {\"samoa\": " 
                  "\"gitarr\"}, \"sune\": [{}, 123 , \"apan\", {\"seppo\":321}]}";

  json_context_t ctx;
  int init = json_init_context(&ctx, js_str);
  check(init == 0);

  json_obj_t *res;

  res = json_read_object(NULL, &ctx);

  check(res == NULL);

  printf("retained: %d \n",ctx.object_retained);

  return 0;
}

int test_init_context(){

  char * js_str = "{\"apa\": {\"gnu\": 321}, \"lennart\": {\"samoa\": \"gitarr\"}, \"sune\": {}}";

  json_context_t ctx;
  int init = json_init_context(&ctx, js_str);
  check(init == 0);

  check(ctx.cursor == 0);
  check(ctx.object_count == 0);
  check(ctx.source == js_str);

  return 0;
}

int test_parse_escape_chars(){
  char * js_str = "{\"ap\\\"a\": {\"g\\uc1b2nu\": 321}, \"lennart\": {\"samoa\": \"gitarr\"}, \"sune\": {}}";

  json_context_t ctx;
  int init = json_init_context(&ctx, js_str);
  check(init == 0);
  
  
  json_obj_t *res;
  res = json_read_object(NULL, &ctx);
  check(res != NULL);

  json_obj_t * c = res->children->children->next_sibling->children->children;
  check(c != NULL);
  check(strncmp(c->str, "g\\uc1b2nu", c->length) == 0);

  json_object_destroy(res, &ctx);
  return 0;
}

int test_add_to_children(){

  char * js_str = "{}";
  json_context_t ctx;
  check(json_init_context(&ctx, js_str) == 0);

  json_obj_t * parent = json_get_object(&ctx);

  for( int i = 0; i < 500000; i++ ){

    json_obj_t * obj = json_get_object(&ctx);
    obj->type = JSON_OBJECT;

    json_add_to_children(parent, obj);
  }

  int counter = 0;
  json_object_destroy(parent, &ctx);

  check(ctx.object_retained == 500001);
  return 0;
}

/* obsolete */
int test_add_to_children_without_end_reference(){

  char * js_str = "{}";
  json_context_t ctx;
  check(json_init_context(&ctx, js_str) == 0);

  json_obj_t * parent = json_get_object(&ctx);

  for( int i = 0; i < 500; i++ ){
    json_obj_t * obj = json_get_object(&ctx);
    obj->type = JSON_OBJECT;
    json_add_to_children(parent, obj);
  }

  int counter = 0;
  json_object_destroy(parent, &ctx);

  check(ctx.object_retained == 501);
  return 0;
}

int test_get_object_attribute(){
  
  char *buf;
  if(read_json_file(&buf, TEST_FILE_1)!=0){
    fail();
  }
  json_context_t ctx;
  json_init_context(&ctx, buf);
  json_obj_t * iterator;
  json_obj_t * res;
  res = json_parse(&ctx);

  check(res != NULL);

  iterator = res->children;
  while(iterator != NULL){
    json_obj_t * id = json_get_object_attribute("id", iterator);
    json_obj_t * index  = json_get_object_attribute("index", iterator);
    printf("%.*s  - %.*s\n", id->length, id->str, index->length, index->str);

    json_obj_t *friends = json_get_object_attribute("friends", iterator);

    json_obj_t *i2;
    i2 = friends->children;

    while(i2 != NULL){
      json_obj_t *name = json_get_object_attribute("name", i2);
      printf("    %.*s\n", name->length, name->str);
      i2 = i2->next_sibling;
    }
    
    iterator = iterator->next_sibling;
  }

  check(res->size == 5);

  json_object_destroy(res, &ctx);

  printf("count: %d \n", ctx.object_count);
  printf("retained: %d \n", ctx.object_retained);

  check(ctx.object_count == 476);
  check(ctx.object_retained == 476);

  free(buf);
  return 0;
}

int test_json_parse(){

  char * js_obj = "       {}  ";
  char * js_arr = "   []";
  char * gibberish = "abc";

  json_context_t ctx;
  
  check(json_init_context(&ctx, js_obj)==0);
  json_obj_t *obj = json_parse(&ctx);
  check(obj != NULL);
  check(obj->type == JSON_OBJECT);
  json_object_destroy(obj, &ctx);


  check(json_init_context(&ctx, js_arr)==0);
  json_obj_t *arr = json_parse(&ctx);
  check(arr != NULL);
  check(arr->type == JSON_ARRAY);
  json_object_destroy(arr, &ctx);
  
  check(json_init_context(&ctx, gibberish)==0);
  json_obj_t *g = json_parse(&ctx);
  check(g == NULL);

  return 0;
}

int test_json_parse_invalid_data(){
  
  char * js_obj = "{apa}";
  json_obj_t *obj;

  json_context_t ctx;
  
  check(json_init_context(&ctx, js_obj)==0);
  obj = json_read_object(NULL, &ctx);
  check(obj == NULL);

  char * js_obj2 = "{  \"apa\":   {[a]}}";
  check(json_init_context(&ctx, js_obj2)==0);
  obj = json_read_object(NULL, &ctx);
  check(obj == NULL);

  return 0;
}

int test_json_object_each(){
  
  json_obj_t *obj, *iterator = NULL;
  json_context_t ctx;
  char *buf;

  if(read_json_file(&buf, TEST_FILE_1)!=0){
    fail();
  }

  check(json_init_context(&ctx, buf)==0);

  obj = json_parse(&ctx);
  check(obj != NULL);

  //first item in array
  json_obj_t * doc = obj->children;
  check(doc != NULL);

  json_obj_t * attr = json_get_object_attribute("balance", doc);
  check(strncmp("$1,122.98", attr->str, attr->length) == 0);
  printf("%.*s\n", attr->length, attr->str);
  
  attr = json_get_object_attribute("registered", doc);
  check(strncmp("2014-08-23T01:35:10 -02:00", attr->str, attr->length) == 0);
  printf("%.*s\n", attr->length, attr->str);

  int count = 0;
  iterator = doc->children;
  while(iterator != NULL){
    iterator = iterator->next_sibling;
    count++;
  }
  printf("%d \n", doc->size);
  check(doc->size == 22);
  check(doc->size == count);

  json_object_destroy(obj, &ctx);

  free(buf);

  return 0;
}

int main() {
  test(test_read_array, "test reading json array");
  test(test_read_object, "test reading json object");
  test(test_read_invalid_object, "test reading invalid json object");
  test(test_read_invalid_primitive, "test reading invalid json primitive");
  test(test_init_context, "test init context");
  test(test_add_to_children, "test add to children");
  test(test_add_to_children_without_end_reference, "test add to children without keeping end node");
  test(test_parse_escape_chars, "test parsing escape chars in string");
  test(test_get_object_attribute, "test getting object attribute");
  test(test_json_parse, "test json_parse");
  test(test_json_parse_invalid_data, "test error handling");
  test(test_json_object_each, "test large file");

  printf("\n");
  printf("-------------------------------------\n");
  printf("passed tests: %d \n", test_passed);
  printf("failed tests: %d \n", test_failed);
  printf("checks run:   %d \n", checks_run);
}






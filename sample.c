#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "json.h"

#define JSON_FILE "./fixtures/mock.json"

static int read_json_file(char ** buf, char * filename){
  struct stat st; 

  if (stat(filename, &st) != 0) {
    return 1;
  }
  *buf = malloc(sizeof(char) * (st.st_size + 1));
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

#define INDENT(ident) ident * 5

static void parse_key_value(int indent, json_obj_t * kv);
static void parse_object(int indent, json_obj_t * object);
static void parse_array(int indent, json_obj_t * array);

static void parse_key_value(int indent, json_obj_t * kv){
  if(kv->type != JSON_KEY_VALUE_PAIR){
    printf("%d: unexpected type %d\n", __LINE__, kv->type);
    return;
  }
  json_obj_t * key = kv->children;
  json_obj_t * value = kv->children->next_sibling;

  printf("%*s%.*s",INDENT(indent), "",  key->length, key->str);

  switch(value->type){
    case JSON_STRING:
      printf(" -> %.*s\n", value->length, value->str);
      break;
    case JSON_PRIMITIVE:
      printf(" -> %.*s\n", value->length, value->str);
      break;
    case JSON_OBJECT:
      printf("\n");
      parse_object(indent + 1, value);
      printf("\n");
      break;
    case JSON_ARRAY:
      printf("\n");
      parse_array(indent + 1, value);
      printf("\n");
      break;
    default:
      printf("%d: unexpected type %d in array\n", __LINE__, value->type);
      return;
  }
}

static void parse_object(int indent, json_obj_t * object){
  if(object->type != JSON_OBJECT){
    printf("%d: unexpected type %d\n", __LINE__, object->type);
    return;
  }

  json_obj_t * object_members = object->children;

  while(object_members != NULL){
    parse_key_value(indent, object_members);
    object_members = object_members->next_sibling;
  }
}

static void parse_array(int indent, json_obj_t * array){
  if(array->type != JSON_ARRAY){
    printf("%d: unexpected type %d\n", __LINE__, array->type);
    return;
  }

  json_obj_t * item = array->children;

  while(item != NULL){

    switch(item->type){
      case JSON_STRING:
        printf("%*s%.*s\n", INDENT(indent), "", item->length, item->str);
        break;
      case JSON_PRIMITIVE:
        printf("%*s%.*s\n", INDENT(indent), "", item->length, item->str);
        break;
      case JSON_OBJECT:
        parse_object(indent, item);
        break;
      default:
        printf("%d: unexpected type %d in array\n", __LINE__, item->type);
        return;
    }

    item = item->next_sibling;
  }
}

int main(int argc, char **argv) {

  char *buf;
  if(read_json_file(&buf, JSON_FILE)!=0){
    printf("failed to read fixture\n");
    if(buf != NULL){
      free(buf);
    }
    return 1;
  }

  json_context_t ctx;
  json_init_context(&ctx, buf);

  json_obj_t * res = NULL;
  res = json_read_array(NULL, &ctx);

  parse_array(1, res);


  json_object_destroy(res, &ctx);

  if(buf != NULL){
    free(buf);
  }
  return 0;
}


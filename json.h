#ifndef __JSON_H
#define __JSON_H

typedef enum {
  JSON_OBJECT = 1,
  JSON_ARRAY = 2,
  JSON_KEY_VALUE_PAIR = 3,
  JSON_STRING = 4,
  JSON_PRIMITIVE = 5
} json_type_t;

struct json_obj_s {
  char *str;
  int length;
  json_type_t type;
  int size; //num children
  struct json_obj_s *children;
  struct json_obj_s *last_child;
  struct json_obj_s *next_sibling; 
  struct json_obj_s *parent;
};
typedef struct json_obj_s json_obj_t;

typedef struct {
  unsigned int object_count;
  unsigned int object_retained;

  char * source;
  unsigned int cursor;

} json_context_t;

json_obj_t * json_parse(json_context_t * ctx);
int json_init_context(json_context_t *ctx, char * js);
void json_object_destroy(json_obj_t *obj, json_context_t *ctx);

json_obj_t * json_get_object_attribute(char * key, json_obj_t *obj);

/* private */
json_obj_t * json_read_object(json_obj_t * parent, json_context_t * ctx);
json_obj_t * json_read_key_value(json_obj_t * parent, json_context_t * ctx);
json_obj_t * json_read_string(json_obj_t * parent, json_context_t *ctx);
json_obj_t * json_read_primitive(json_obj_t * parent, json_context_t *ctx);
json_obj_t * json_read_array(json_obj_t * parent, json_context_t *ctx );

int json_read_esc_char(json_context_t * ctx);

void json_add_to_children(json_obj_t *parent, json_obj_t * obj);
json_obj_t * json_get_object(json_context_t *ctx);

void print_error(char * msg, int lineno, json_context_t *ctx);

#endif

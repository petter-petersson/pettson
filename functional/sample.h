#ifndef __SAMPLE_H
#define __SAMPLE_H

#include "../json.h"
#include "store.h"

#define JSON_FILE "./fixtures/mock.json"

#define PARSE_STRING_FIELD(n, field) \
  do { \
    assert(n->type =JSON_STRING); \
    sample_context_t * context = (sample_context_t *) user_data; \
    person_t * person = context->person; \
    int len = MIN(n->length, sizeof(person->field) - 1); \
    strncpy(person->field, n->str, len); \
    person->field[len] = 0; \
  } while(0) \

/* use prime number for size to minimize collision error */
#define ACTION_TABLE_SIZE 5099

typedef struct sample_context_s {
  person_t * person;
  person_store_t * person_store;
  friend_store_t * friend_store;
} sample_context_t;

int read_json_file(char ** buf, char * filename);
int hash_from_key(char * key, int len);
void process_array_item(json_obj_t * item, void * user_data);
void parse_key_value(json_obj_t * key, json_obj_t * value, void * user_data);
void parse_id(json_obj_t * value, void * user_data);
void parse_email(json_obj_t * value, void * user_data);
void parse_phone(json_obj_t * value, void * user_data);
void parse_address(json_obj_t * value, void * user_data);
void parse_about(json_obj_t * value, void * user_data);
void parse_age(json_obj_t * value, void * user_data);
void parse_active(json_obj_t * value, void * user_data);
void parse_friends(json_obj_t * value, void * user_data);
void parse_friend(json_obj_t * value, void * user_data);
void parse_friend_name(json_obj_t * value, void * user_data);
void parse_friend_id(json_obj_t * value, void * user_data);
void parse_friend_key_value(json_obj_t * key, json_obj_t * value, void * user_data);

#endif

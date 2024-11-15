#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <stdbool.h>
#include <assert.h>
#include <errno.h>

#include "sample.h"
#include "store.h"
#include "../json.h"
#include "common.h"

/* hash table of function pointers for each attribute to be parsed */
void (*action_callback_table[ACTION_TABLE_SIZE]) (json_obj_t * value, void * user_data);

/* callback methods for processing fields */
void parse_id(json_obj_t * value, void * user_data) {
  PARSE_STRING_FIELD(value, id);
}

void parse_email(json_obj_t * value, void * user_data) {
  PARSE_STRING_FIELD(value, email);
}

void parse_phone(json_obj_t * value, void * user_data) {
  PARSE_STRING_FIELD(value, phone);
}

void parse_address(json_obj_t * value, void * user_data) {
  PARSE_STRING_FIELD(value, address);
}

void parse_about(json_obj_t * value, void * user_data) {
  PARSE_STRING_FIELD(value, about);
}

void parse_friends(json_obj_t * value, void * user_data) {
  assert(value->type == JSON_ARRAY);
  json_each_array(value, user_data, parse_friend);
}

void parse_friend(json_obj_t * value, void * user_data) {
  assert(value->type == JSON_OBJECT);
  sample_context_t * context = (sample_context_t *) user_data;
  person_t * person = context->person;
  friend_t ** friend = (friend_t **) &(person->friends);
  if(*friend == NULL) {
    *friend = friend_store_get_new_friend(context->friend_store);
    json_each_object_attribute(value,(void *) *friend, parse_friend_key_value);
    return;
  }
  assert((*friend)->next == NULL);

  friend_t * next = friend_store_get_new_friend(context->friend_store);
  next->previous = *friend;
  (*friend)->next = next;
  *friend = (*friend)->next;

  json_each_object_attribute(value,(void *) *friend, parse_friend_key_value);
}

void parse_friend_name(json_obj_t * value, void * user_data) {
  assert(value->type == JSON_STRING);
  friend_t * friend = (friend_t *) user_data;
  int len = MIN(value->length, sizeof(friend->name) - 1);
  strncpy(friend->name, value->str, len);
  friend->name[len] = 0;
}

void parse_friend_id(json_obj_t * value, void * user_data) {
  assert(value->type == JSON_PRIMITIVE);
  friend_t * friend = (friend_t *) user_data;
  char id[4];
  int len = MIN(value->length, sizeof(id) - 1);
  strncpy(id, value->str, len);
  id[len] = 0;
  friend->id = strtol(id, NULL, 10);
}

void parse_friend_key_value(json_obj_t * key, json_obj_t * value, void * user_data){
  /* using a prefix so we can use same action table */
  char prefix[] = "friend.";
  char concat_key[20];
  strcpy(concat_key, prefix);
  int len = MIN(strlen(prefix) + key->length, sizeof(concat_key) - 1);
  strncat(concat_key, key->str, len);
  concat_key[len] = 0;

  int action_hash = hash_from_key(concat_key, strlen(concat_key));
  void (*callback) (json_obj_t * value, void * user_data) = action_callback_table[action_hash];
  if (callback != 0) {
    callback(value, user_data);
  }
}

void parse_age(json_obj_t * value, void * user_data) {
  assert(value->type == JSON_PRIMITIVE);
  sample_context_t * context = (sample_context_t *) user_data;
  person_t * person = context->person;
  char age[4];
  int len = MIN(value->length, sizeof(age) - 1);
  strncpy(age, value->str, len);
  age[len] = 0;
  person->age = strtol(age, NULL, 10);
}

void parse_active(json_obj_t * value, void * user_data) {
  assert(value->type == JSON_PRIMITIVE);
  sample_context_t * context = (sample_context_t *) user_data;
  person_t * person = context->person;
  char boolstring[20];
  int len = MIN(value->length, sizeof(boolstring) - 1);
  strncpy(boolstring, value->str, len);
  boolstring[len] = 0;
  person->active = strcmp(boolstring, "true") ? true : false;
}
/* END callback methods for processing fields */

/* callback method for processing each main object attribute
   looking up callback in action table for processing individual fields */
void parse_key_value(json_obj_t * key, json_obj_t * value, void * user_data){
  int action_hash = hash_from_key(key->str, key->length);
  void (*callback) (json_obj_t * value, void * user_data) = action_callback_table[action_hash];
  if (callback != 0) {
    callback(value, user_data);
  }
}

/* callback method for processing each array element */
void process_array_item(json_obj_t * item, void * user_data){
  assert(item->type == JSON_OBJECT);
  sample_context_t * context = (sample_context_t *) user_data;
  person_t ** person = (person_t **) &(context->person);
  if(*person == NULL) {
    *person = person_store_get_new_person(context->person_store);
    json_each_object_attribute(item,(void *) user_data, parse_key_value);
    return;
  }
  assert((*person)->next == NULL);

  person_t * next = person_store_get_new_person(context->person_store);
  next->previous = *person;
  (*person)->next = next;
  *person = (*person)->next;

  json_each_object_attribute(item,(void *) user_data, parse_key_value);
}

/* calculate hash from string for action table lookup */
int hash_from_key(char * key, int len) {
  unsigned int h = 5381;
  int i = 0;

  while(*key != '\0' && i < len) {
    h = ((h << 5) + h) + *key;
    i++;
    key++;
  }

  return h & ACTION_TABLE_SIZE;
}

/* read test fixture */
int read_json_file(char ** buf, char * filename){
  struct stat st;

  if (stat(filename, &st) != 0) {
    return 1;
  }
  *buf = malloc(sizeof(char) * (st.st_size + 1));
  if(*buf == NULL){
    perror(MEM_ERR_MSG);
  }
  FILE *fp = fopen(filename, "rb");

  fread((*buf), 1, st.st_size, fp);
  (*buf)[st.st_size]='\0';

  fclose(fp);
  return 0;
}

int main(int argc, char **argv) {
  /* read test data into memory */
  char *buf = NULL;
  if(read_json_file(&buf, JSON_FILE)!=0){
    printf("failed to read fixture\n");
    if(buf != NULL){
      free(buf);
    }
    return 1;
  }
  /* clear action table */
  for(int i=0;i < ACTION_TABLE_SIZE; i++) {
    action_callback_table[i] = 0;
  }
  /* populate action table */
  action_callback_table[hash_from_key("id", 2)] = parse_id;
  action_callback_table[hash_from_key("email", 5)] = parse_email;
  action_callback_table[hash_from_key("phone", 5)] = parse_phone;
  action_callback_table[hash_from_key("address", 7)] = parse_address;
  action_callback_table[hash_from_key("about", 5)] = parse_about;
  action_callback_table[hash_from_key("age", 3)] = parse_age;
  action_callback_table[hash_from_key("isActive", 8)] = parse_active;
  action_callback_table[hash_from_key("friends", 7)] = parse_friends;
  action_callback_table[hash_from_key("friend.name", 11)] = parse_friend_name;
  action_callback_table[hash_from_key("friend.id", 9)] = parse_friend_id;

  /* initialize json context */
  json_context_t ctx;
  json_init_context(&ctx, buf);

  json_obj_t * res = NULL;
  res = json_read_array(NULL, &ctx);

  person_store_t * person_store = person_store_create();
  friend_store_t * friend_store = friend_store_create();

  sample_context_t context;
  context.person = NULL;
  context.person_store = person_store;
  context.friend_store = friend_store;
  json_each_array(res, (void *) &context, process_array_item);

  /* display result */
  person_t * cursor = context.person;
  while(cursor){
    printf("id: %s\n", cursor->id);
    printf("age: %d\n", cursor->age);
    printf("active: %d\n", cursor->active);
    printf("email: %s\n", cursor->email);
    printf("phone: %s\n", cursor->phone);
    printf("address: %s\n", cursor->address);
    printf("about: %s\n", cursor->about);

    friend_t * friend = cursor->friends;
    while(friend) {
      printf("friend.name: %s\n", friend->name);
      printf("friend.id: %d\n", friend->id);
      friend = friend->previous;
    }

    printf("----------------\n");

    cursor = cursor->previous;
  }

  printf("person_store person index: %d\n", person_store->index);
  printf("friend_store friend index: %d\n", friend_store->index);

  /* clean up */
  json_object_destroy(res, &ctx);

  if(buf != NULL){
    free(buf);
  }
  person_store_destroy(person_store);
  friend_store_destroy(friend_store);
  return 0;
}

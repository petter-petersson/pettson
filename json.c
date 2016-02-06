#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include "json.h"


void print_error(char * msg, int lineno, json_context_t *ctx){
  fprintf( stderr, "%s:%d Parse error. Cursor position %d.\n", __FILE__, lineno, ctx->cursor);
  int startpos = ctx->cursor;
  int counter = 0;
  while(startpos != 0 && counter < 8) {
    startpos--;
    counter++;
  }
  fprintf( stderr, "> %.*s\n", 16, &(ctx->source[startpos]) );
  fprintf( stderr, "> ");
  for(int i=0; i < (ctx->cursor - startpos); i++){
    fprintf(stderr, " ");
  }
  fprintf(stderr, "^\n");

  if(msg!=NULL){
    fprintf(stderr, "%s\n",msg);
  }
}

json_obj_t * json_parse(json_context_t * ctx) {
  
  char c;
  while(1){

    c = ctx->source[ctx->cursor];

    switch(c) {
      case 0:
        print_error("unexpected EOF", __LINE__, ctx);
        goto err;
        break;

      case '{':
        return json_read_object(NULL, ctx);
        break;

      case '[':
        return json_read_array(NULL, ctx);
        break;

      case '\t' : case '\r' : case '\n' : case ' ':
        break;

      default:
        print_error("unexpected char reached", __LINE__, ctx);
        goto err;
        break;
    }

    ctx->cursor++;
  }

err:
  return NULL;
}

int json_init_context(json_context_t *ctx, char * js) {
  ctx->object_count = 0;
  ctx->object_retained = 0;
  ctx->cursor = 0;
  ctx->source = js;
  return 0;
}

json_obj_t * json_get_object(json_context_t *ctx){

  json_obj_t * ret;
  
  ret  = malloc(sizeof(*ret));
  if (ret == 0) abort();

  ret->children = NULL;
  ret->next_sibling = NULL;
  ret->parent = NULL;
  ret->length = 0;
  ret->str = NULL;
  ret->size = 0;
  
  ctx->object_count++;

  return ret;
}

void json_object_destroy(json_obj_t *obj, json_context_t *ctx){

  json_obj_t * iterator;
  json_obj_t *next;

  if(obj->children != NULL){
    iterator = obj->children;
    while(iterator != NULL){
      next = iterator->next_sibling;
      json_object_destroy(iterator, ctx);
      iterator = next;
    }
  }
  free(obj);
  obj = NULL;
  ctx->object_retained++;
}

void json_add_to_children(json_obj_t *parent, json_obj_t * previous, json_obj_t * obj) {
  if(obj == NULL){
    fprintf(stderr, "json_add_to_children: object is null");
    return;
  }
  //these should be set already:
  obj->next_sibling = NULL;
  obj->parent = parent;

  if (parent->children == NULL) {
    parent->children = obj;
    parent->size++;
    return;
  }

  //NOTE this is an optimization that can backfire if input is bad
  if(previous != NULL){
    //not failproof validation but something
    assert(previous->parent != NULL && obj->parent != NULL &&
           previous->parent == parent);

    previous->next_sibling = obj;
    parent->size++;
    return;
  }

  /* we dont have last element in chain so we iterate */
  json_obj_t *iterator = parent->children;

  while(1){
    if(iterator->next_sibling == NULL){
      iterator->next_sibling = obj;
      parent->size++;
      return;
    }
    iterator = iterator->next_sibling;
  }
  return;
}


json_obj_t * json_read_object(json_obj_t * parent, json_context_t *ctx ) {

  char c;
  int len;
  json_obj_t *kv;

  int start_pos = ctx->cursor; 
  ctx->cursor++;

  json_obj_t *self = NULL;
  self = json_get_object(ctx);
  self->type = JSON_OBJECT;
  self->parent = parent;
  self->str = &(ctx->source[start_pos]);

  json_obj_t *previous_value = NULL;

  while(1){

    c = ctx->source[ctx->cursor];

    switch(c) {
      case 0:
        print_error("unexpected EOF", __LINE__, ctx);
        goto err;
        break;

      case '\"':

        kv = json_read_key_value(self, ctx);
        if(kv == NULL){
          print_error("keyvalue pair returned was null", __LINE__, ctx);
          goto err;
        }
        //printf("%.*s\n",kv->length, kv->str);
        //TODO: add kv->key + kv->value to a hash + implement lookup
        json_add_to_children(self, previous_value, kv);
        previous_value = kv;
        break;

      case '}':
        self->length = ctx->cursor + 1 - start_pos;
        return self;
        break;

      case ',':
        break;

      case '\t' : case '\r' : case '\n' : case ' ':
        break;

      default:
        print_error("primitives as keys are not supported", __LINE__, ctx);
        goto err;
        break;
    }

    ctx->cursor++;
  }

err:
  if(self != NULL){
    json_object_destroy(self, ctx);
  }
  return NULL;
}

json_obj_t * json_read_array(json_obj_t * parent, json_context_t *ctx ){

  char c;
  json_obj_t *val;

  
  int start_pos = ctx->cursor; 

  ctx->cursor++;

  json_obj_t *self = NULL;
  self = json_get_object(ctx);
  self->parent = parent;
  self->type = JSON_ARRAY;
  self->str = &(ctx->source[start_pos]);

  bool have_comma = true; //first element

  json_obj_t *previous_value = NULL;
  while(1){

    c = ctx->source[ctx->cursor];

    switch(c) {
      case 0:
        print_error("unexpected EOF", __LINE__, ctx);
        goto err;
        break;

      case '\"':

        if(!have_comma){
          print_error("Comma expected.", __LINE__, ctx);
          goto err;
        }
        have_comma = false;
        val = json_read_string(self, ctx);
        if(val == NULL){
          print_error("Did not receive value.", __LINE__, ctx);
          goto err;
        }
        json_add_to_children(self, previous_value, val);
        previous_value = val; 
        break;
      
      case '{':

        if(!have_comma){
          print_error("Comma expected.", __LINE__, ctx);
          goto err;
        }
        have_comma = false;
        val = json_read_object(self, ctx);
        if(val == NULL){
          print_error("Did not receive value.", __LINE__, ctx);
          goto err;
        }
        json_add_to_children(self, previous_value, val);
        previous_value = val; 
        break;

      case ']':
        self->length = ctx->cursor + 1 - start_pos;
        //printf("%.*s\n", self->length, self->str);
        return self;
        break;

      case ',':
        have_comma = true;
        break;

      case '\t' : case '\r' : case '\n' : case ' ':
        break;
      default:
        if(!have_comma){
          print_error("Comma expected.", __LINE__, ctx);
          goto err;
        }
        have_comma = false;
        val = json_read_primitive(self, ctx);
        if(val == NULL){
          print_error("Did not receive value.", __LINE__, ctx);
          goto err;
        }
        json_add_to_children(self, previous_value, val);
        previous_value = val; 
        break;
    }

    ctx->cursor++;
  }

err:
  if(self != NULL){
    json_object_destroy(self, ctx);
  }
  return NULL;
}

/* refactor */
json_obj_t * json_read_key_value(json_obj_t * parent, json_context_t *ctx ) {

  char c;
  int len;
  int start_pos = ctx->cursor; 

  bool have_key = false;

  json_obj_t *self = NULL;
  self = json_get_object(ctx);
  self->type = JSON_KEY_VALUE_PAIR;
  self->parent = parent;
  self->str = &(ctx->source[ctx->cursor]);

  json_obj_t *key = NULL;
  json_obj_t *value = NULL;
  
  while(1){

    c = ctx->source[ctx->cursor];

    switch(c) {
      case 0:
        print_error("unexpected EOF", __LINE__, ctx);
        goto err;
        break;

      case '\"':
        if(!have_key){
          key = json_read_string(self, ctx);
        } else {
          if (key == NULL) {
            print_error("key value was null.", __LINE__, ctx);
            goto err;
          }
          value = json_read_string(self, ctx );

          goto finalize_value;
        }
        break;
      case ':':
        have_key = true;
        break;

      case '{':
        if(have_key){
          if (key == NULL) {
            print_error("key value was null.", __LINE__, ctx);
            goto err;
          }
          value = json_read_object(self, ctx);

          goto finalize_value;
        } else {
            print_error("Did not have key.", __LINE__, ctx);
            goto err;
        }
        break;
      case '[':
        if(have_key){
          if (key == NULL) {
            print_error("Key was null.", __LINE__, ctx);
            goto err;
          }
          value = json_read_array(self, ctx);

          goto finalize_value;
        } else {
            print_error("Did not have key.", __LINE__, ctx);
            goto err;
        }
        break;
      case '\t' : case '\r' : case '\n' : case ' ':
        break;
      default:
        //read_primitive - no support for primitives as key as of now
        if(have_key){
          if (key == NULL) {
            print_error("Key was null.", __LINE__, ctx);
            goto err;
          }
          value = json_read_primitive(self, ctx);

          goto finalize_value;

        } else {
            print_error("Did not have key.", __LINE__, ctx);
            goto err;
        }
        break;
    }
    
    ctx->cursor++;
  }
err:
  if(self != NULL){
    json_object_destroy(self, ctx);
  }
  if(key != NULL){
    json_object_destroy(key, ctx);
  }
  if(value != NULL){
    json_object_destroy(value, ctx);
  }
  return NULL;

finalize_value:

  if (value == NULL) {
    print_error("Value is null.", __LINE__, ctx);
    goto err;
  }
  self->length = ctx->cursor + 1 - start_pos;
  key->next_sibling = value;
  self->children = key;
  //printf("%.*s\n", self->length, self->str);
  return self;
}

json_obj_t * json_read_string(json_obj_t * parent, json_context_t *ctx){

  char c;
  int len;

  ctx->cursor++;

  int start_pos = ctx->cursor; 
  char * str = &(ctx->source[ctx->cursor]);

  if(str == NULL) {
    //unexpected
    return NULL;
  }

  json_obj_t *self = json_get_object(ctx);
  self->parent = parent;
  self->type = JSON_STRING;

  while(1){

    c = ctx->source[ctx->cursor];
    switch(c) {
      case 0:
        print_error("Unexpected EOF", __LINE__, ctx);
        goto err;
        break;

      case '\"':

        len = ctx->cursor - start_pos;
        self->length = len;
        self->str = str;
        return self;
        break;

      case '\\':

        if(json_read_esc_char(ctx) != 0){
          print_error("illegal escape char", __LINE__, ctx);
          goto err;
        }
        break;
    }
    ctx->cursor++;
  }
err:
  json_object_destroy(self, ctx);
  return NULL;
}

int json_read_esc_char(json_context_t * ctx){

  char c;

  ctx->cursor++;

  c = ctx->source[ctx->cursor];
  switch(c) {
    case 0:
      print_error("Unexpected EOF", __LINE__, ctx);
      goto err;
      break;

    case '\"': case '/' : case '\\' : case 'b' :
    case 'f' : case 'r' : case 'n'  : case 't' :
      return 0;
      break;

    case 'u':
      ctx->cursor++;
      c = ctx->source[ctx->cursor];
      if(c==0){
        print_error("Unexpected EOF", __LINE__, ctx);
        goto err;
      }

      for(int i = 0; i < 4; i++) {
        /* If it isn't a hex character we have an error */
        c = ctx->source[ctx->cursor];
        if(c==0){
          print_error("Unexpected EOF", __LINE__, ctx);
          goto err;
        }
        if(!( (c >= 48 && c <= 57) || /* 0-9 */
              (c >= 65 && c <= 70) || /* A-F */
              (c >= 97 && c <= 102))) { /* a-f */

          fprintf(stderr, "unexpected hex char %d\n", c);
          return 1;
        }
        ctx->cursor++;
      }
      ctx->cursor--;
      return 0; 
      break;
  }

err:
  return 1;
}

json_obj_t * json_read_primitive(json_obj_t * parent, json_context_t *ctx){

  char c;
  int len;
  int start_pos = ctx->cursor; 
  char * str = &(ctx->source[ctx->cursor]);

  json_obj_t *self = json_get_object(ctx);
  self->parent = parent;
  self->type = JSON_PRIMITIVE;

  while(1){

    c = ctx->source[ctx->cursor];
    switch(c) {
      case 0:
        print_error("Unexpected EOF", __LINE__, ctx);
        goto err;
        break;

      case ':' :
      case '\t': 
      case '\r': 
      case '\n': 
      case ' ' :
      case ',' : 
      case ']' : 
      case '}' :

        len = ctx->cursor - start_pos;
        ctx->cursor--;

        self->length = len;
        self->str = str;
        return self;
        break;

    }
    if (c < 32 || c >= 127) {
      print_error("illegal char", __LINE__, ctx);
      goto err;
    }
    ctx->cursor++;
  }

err:
  json_object_destroy(self, ctx);
  return NULL;

}

json_obj_t * json_get_object_attribute(char * key, json_obj_t *obj) {
  if(obj == NULL || obj->children == NULL){
    return NULL;
  }
  json_obj_t *iterator = obj->children;

  while(iterator != NULL){
    if(iterator->type==JSON_KEY_VALUE_PAIR){

        json_obj_t * k = iterator->children;
        if(k != NULL){
          //printf("%u: %s - %.*s\n",k->type, key, k->length, k->str);
          if(strncmp(k->str, key, k->length)==0){
            return k->next_sibling;
          }

        }
    }
    iterator = iterator->next_sibling;
  }

  return NULL;
}





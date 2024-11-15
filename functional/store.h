#ifndef __STORE_H
#define __STORE_H

#include <stdbool.h>

#define STORE_PAGE_SIZE 2

typedef struct friend_s {
  char name[50];
  int id;
  struct friend_s *next;
  struct friend_s *previous;
} friend_t;

typedef struct person_s {
  char id[50];
  int age;
  bool active;
  char email[50];
  char phone[50];
  char address[100];
  char about[500];
  friend_t * friends;
  struct person_s *next;
  struct person_s *previous;
} person_t;

typedef struct person_page_s {
  person_t * person;
  struct person_page_s * next;
} person_page_t;

typedef struct person_store_s {
  person_page_t * pages;
  person_page_t * tail;
  int index;
} person_store_t;

typedef struct friend_page_s {
  friend_t * friend;
  struct friend_page_s * next;
} friend_page_t;

typedef struct friend_store_s {
  friend_page_t * pages;
  friend_page_t * tail;
  int index;
} friend_store_t;

person_store_t * person_store_create();
person_t * person_store_get_new_person(person_store_t * store);
void person_store_destroy(person_store_t * store);

friend_store_t * friend_store_create();
friend_t * friend_store_get_new_friend(friend_store_t * store);
void friend_store_destroy(friend_store_t * store);
#endif

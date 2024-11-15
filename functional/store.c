#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <errno.h>
#include "store.h"
#include "common.h"

person_store_t * person_store_create() {
  person_store_t * store;
  store = (person_store_t *) malloc(sizeof(*store));
  if (store == NULL) {
    perror(MEM_ERR_MSG);
  }
  store->pages = (person_page_t *) malloc(sizeof(*(store->pages)));
  if (store->pages == NULL) {
    perror(MEM_ERR_MSG);
  }

  store->pages->person = (person_t *) malloc(STORE_PAGE_SIZE * sizeof(*(store->pages->person)));
  if (store->pages->person == NULL) {
    perror(MEM_ERR_MSG);
  }
  store->pages->next = NULL;
  store->tail = store->pages;

  store->index = 0;
  return store;
}

void person_store_destroy(person_store_t * store) {
  person_page_t * page = store->pages;
  while(page) {
    person_page_t * tmp = page->next;
    person_t * person = page->person;
    free(person);
    free(page);
    page = tmp;
  }
  free(store);
}

person_t * person_store_get_new_person(person_store_t * store) {
  int index = store->index % STORE_PAGE_SIZE;

  if(store->index > 0 && index == 0) {
    person_page_t * tail = store->tail;
    person_page_t * next_page = (person_page_t *) malloc(sizeof(*(store->pages)));
    if (next_page == NULL) {
      perror(MEM_ERR_MSG);
    }
    next_page->next = NULL;
    tail->next = next_page;
    store->tail = next_page;
    store->tail->person = (person_t *) malloc(STORE_PAGE_SIZE * sizeof(*(store->pages->person)));
    if (store->tail->person == NULL) {
      perror(MEM_ERR_MSG);
    }
  }
  person_page_t * tail = store->tail;
  assert(index < STORE_PAGE_SIZE);
  assert(tail != NULL);
  assert(tail->person != NULL);
  person_t * person =  tail->person + index; 
  person->next = NULL;
  person->previous = NULL;
  person->friends = NULL;
  store->index++;
  return person;
}

/* friend type methods */
friend_store_t * friend_store_create() {
  friend_store_t * store;
  store = (friend_store_t *) malloc(sizeof(*store));
  if (store == NULL) {
    perror(MEM_ERR_MSG);
  }
  store->pages = (friend_page_t *) malloc(sizeof(*(store->pages)));
  if (store->pages == NULL) {
    perror(MEM_ERR_MSG);
  }

  store->pages->friend = (friend_t *) malloc(STORE_PAGE_SIZE * sizeof(friend_t));
  if (store->pages->friend == NULL) {
    perror(MEM_ERR_MSG);
  }
  store->pages->next = NULL;
  store->tail = store->pages;

  store->index = 0;
  return store;
}

void friend_store_destroy(friend_store_t * store) {
  friend_page_t * page = store->pages;
  while(page) {
    friend_page_t * tmp = page->next;
    friend_t * friend = page->friend;
    free(friend);
    free(page);
    page = tmp;
  }
  free(store);
}

friend_t * friend_store_get_new_friend(friend_store_t * store) {
  int index = store->index % STORE_PAGE_SIZE;

  if(store->index > 0 && index == 0) {
    friend_page_t * tail = store->tail;
    friend_page_t * next_page = (friend_page_t *) malloc(sizeof(*(store->pages)));
    if (next_page == NULL) {
      perror(MEM_ERR_MSG);
    }
    next_page->next = NULL;
    tail->next = next_page;
    store->tail = next_page;
    store->tail->friend = (friend_t *) malloc(STORE_PAGE_SIZE * sizeof(friend_t));
    if (store->tail->friend == NULL) {
      perror(MEM_ERR_MSG);
    }
  }
  friend_page_t * tail = store->tail;
  friend_t * friend =  tail->friend + index; 
  friend->next = NULL;
  friend->previous = NULL;
  store->index++;
  return friend;
}

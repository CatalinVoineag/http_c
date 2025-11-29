#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node {
  char* key;
  char* value;
} node_t;

typedef struct HashMap {
  int capacity;
  int size;
  node_t **nodes;
} hash_t;

int hash_get_node_index(char* key, hash_t *hash) {
  for (int i = 0; i < hash->size; i++) {
    if (strcmp(hash->nodes[i]->key, key) == 0) {
      return i;
    } 
  } 

  return -1;
}

void hash_add(char* key, char* value, hash_t *hash) {
  if (!hash || !key || !value) return;

  int node_index = hash_get_node_index(key, hash);
  if (node_index >= 0) {
    hash->nodes[node_index]->value = value;
    return;
  }

  node_t *node = malloc(sizeof(node_t));
  node->key = strdup(key);
  node->value = strdup(value);

  if (hash->size + 1 > hash->capacity) {
    hash->capacity *= 2;
    node_t **temp = realloc(hash->nodes, sizeof(*hash->nodes) * hash->capacity);
    if (temp == NULL) {
      printf("Could not allocate more memory");
      free(temp);
      return;
    }

    hash->nodes = temp;
  }

  if (hash->size == 0) {
    hash->nodes[0] = node;
  } else {
    for (int i = 0; i < hash->size + 1; i++) {
      if (hash->nodes[i] == 0) {
        hash->nodes[i] = node;
        break;
      }
    }
  } 

  hash->size++;
  return;
}

hash_t hash_init() {
  hash_t hash = { .capacity = 5, .size = 0, .nodes = malloc(hash.capacity * sizeof(node_t))};
  return hash;
}

void hash_free_memory(hash_t *hash) {
  for (int i = 0; i < hash->size + 1; i++) {
    hash->nodes[i] = NULL;
  }
  free(hash->nodes);
  hash->size = 0;
}

int hash_delete(char* key, hash_t *hash) {
  int index = hash_get_node_index(key, hash);
  if (index <= 0) { return -1; }

  hash->nodes[index] = NULL;
  hash->size--;

  return index;
}

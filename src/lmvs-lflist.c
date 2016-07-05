#include "lmvs-lflist.h"
#include "lmvs-atomic.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int inline
is_marked_reference(intptr_t p) {
	return (int) (p & 0x1L);
}

intptr_t inline
get_marked_reference(intptr_t p) {
	return p | 0x1L;
}

intptr_t inline
get_unmarked_reference(intptr_t p) {
	return p & ~0x1L;
}

static lmvs_lflist_node_t* inner_search(lmvs_lflist_t* l, int64_t key, lmvs_lflist_node_t** left_node);

lmvs_lflist_node_t*
lmvs_lflist_node_new(int64_t key, void* data) {
	lmvs_lflist_node_t* node = calloc(1, sizeof(*node));
	node->key = key;
	node->data = data;
	node->next = NULL;
	return node;
}

void
lmvs_lflist_node_free(lmvs_lflist_node_t* node) {
	free(node);
}

lmvs_lflist_t*
lmvs_lflist_new() {
	lmvs_lflist_t* l = calloc(1, sizeof(*l));
	l->head = lmvs_lflist_node_new(INT64_MIN, NULL);
	l->tail = lmvs_lflist_node_new(INT64_MAX, NULL);
	l->head->next = l->tail;
	return l;
}

void
lmvs_lflist_free(lmvs_lflist_t* l) {
	lmvs_lflist_node_t* head = l->head->next;
	lmvs_lflist_node_t* tmp;
	while (head != l->tail) {
		tmp = head->next;
		lmvs_lflist_node_free(head);
		head = tmp;
	}
	lmvs_lflist_node_free(l->head);
	lmvs_lflist_node_free(l->tail);
	free(l);
}

int
lmvs_lflist_insert(lmvs_lflist_t* l, int64_t key, void* data) {
	lmvs_lflist_node_t* new_node = lmvs_lflist_node_new(key, data);
	lmvs_lflist_node_t* right_node = NULL;
	lmvs_lflist_node_t* left_node = NULL;

	while (1) {
		right_node = inner_search(l, key, &left_node);
		if ((right_node != l->tail) && (right_node->key == key)) {
			free(new_node);
			return -1;
		}
		new_node->next = right_node;
		if (CAS(&(left_node->next), right_node, new_node)) {
			return 0;
		}
	}
}

int
lmvs_lflist_delete(lmvs_lflist_t* l, int64_t key) {
	lmvs_lflist_node_t* right_node = NULL;
	lmvs_lflist_node_t* right_node_next = NULL;
	lmvs_lflist_node_t* left_node = NULL;

	while (1) {
		right_node = inner_search(l, key, &left_node);
		if ((right_node == l->tail) || right_node->key != key) {
			return -1;
		}

		right_node_next = right_node->next;
		if (!is_marked_reference((intptr_t)right_node_next)) {
			if (CAS(&(right_node->next), right_node_next, get_marked_reference((intptr_t)right_node_next))) {
				break;
			}
		}
	}

	if (!CAS(&(left_node->next), right_node, right_node_next)) {
		right_node = inner_search(l, right_node->key, &left_node);
	}
	lmvs_lflist_node_free(right_node);

	return 0;
}

lmvs_lflist_node_t*
lmvs_lflist_search(lmvs_lflist_t* l, int64_t key) {
	lmvs_lflist_node_t* right_node = NULL;
	lmvs_lflist_node_t* left_node = NULL;
	right_node = inner_search(l, key, &left_node);

	if ((right_node == l->tail) || (right_node->key != key)) {
		return NULL;
	} else {
		return right_node;
	}
}

static inline lmvs_lflist_node_t*
inner_search(lmvs_lflist_t* l, int64_t key, lmvs_lflist_node_t** left_node) {
	lmvs_lflist_node_t* left_node_next = NULL;
	lmvs_lflist_node_t* right_node = NULL;

	while (1) {
		lmvs_lflist_node_t* t = l->head;
		lmvs_lflist_node_t* t_next = l->head->next;
		do {
			if (!is_marked_reference((intptr_t)t_next)) {
				(*left_node) = t;
				left_node_next = t_next;
			}
			t = (lmvs_lflist_node_t*)get_unmarked_reference((intptr_t)t_next);

			if (t == l->tail) {
				break;
			}

			t_next = t->next;
		} while (is_marked_reference((intptr_t)t_next) || (t->key < key));

		right_node = t;

		if (left_node_next == right_node) {
			if ((right_node != l->tail) && is_marked_reference((intptr_t)right_node->next)) {
				continue;
			} else {
				return right_node;
			}
		} else {
			if (CAS(&(*left_node)->next, left_node_next, right_node)) {
				if ((right_node != l->tail) && is_marked_reference((intptr_t)right_node->next)) {
					continue;
				} else {
					return right_node;
				}
			}
		}
	}
}


#include "rbtree.h"
#include <stdlib.h>

static void free_nodes_recursive(node_t *cur, node_t *nil);
static void rb_insert_fixup(rbtree *t, node_t *problem_node);
static void left_rotate(rbtree *t, node_t *y);
static void right_rotate(rbtree *t, node_t *y);

rbtree *new_rbtree(void) {

  //rbtree 구조체 할당
  rbtree *p = (rbtree *)calloc(1, sizeof(rbtree));
  if(p == NULL){
    return NULL;
  }

  //nil (sentinal) 노드 생성
  node_t *nil_node = (node_t *)malloc(sizeof(node_t));
  if(nil_node == NULL){
    free(p);
    return NULL;
  }
  nil_node->color = RBTREE_BLACK;
  nil_node->parent = nil_node;
  nil_node->left = nil_node;
  nil_node->right = nil_node;

  //rbtree의 nil과 root포인터 설정
  p->nil = nil_node;
  p->root = p->nil;

  return p;
}

void delete_rbtree(rbtree *t) {
  //t의 root노드의 주소값을 저장한다.
  node_t *cur = t->root;

  //재귀함수로 후위순회를 돌면서 rbtree의 노드들을 free한다.
  free_nodes_recursive(cur, t->nil);

  //마지막으로 rbtree의 nil을 free하고 rbtree도 할당을 해제한다.
  free(t->nil);
  free(t);
}

static void free_nodes_recursive(node_t *cur, node_t *nil){
  if(cur == nil) return;
  free_nodes_recursive(cur->left, nil);
  free_nodes_recursive(cur->right, nil);
  free(cur);
}

node_t *rbtree_insert(rbtree *t, const key_t key) {

  node_t *insert_node = (node_t *)malloc(sizeof(node_t));;
  if(insert_node == NULL) return NULL;
  insert_node->color = RBTREE_RED;
  insert_node->key = key;
  insert_node->left = t->nil;
  insert_node->right = t->nil;
  insert_node->parent = t->nil;

  node_t *cur = t->root;
  node_t *prev_cur = t->nil;

  while(cur != t->nil){
    if(cur->key > key){
      prev_cur = cur;
      cur = cur->left;
    }
    else if(cur->key < key){
      prev_cur = cur;
      cur = cur->right;
    }
    else{
      free(insert_node);
      return NULL;
    }
  }

  if(prev_cur == t->nil){
    t->root = insert_node;
    insert_node->parent = t->nil;
  }
  else{
    if(prev_cur->key > key) prev_cur->left = insert_node;
    else prev_cur->right = insert_node;
    insert_node->parent = prev_cur;
  }

  rb_insert_fixup(t, insert_node);

  return t->root;
}

static void rb_insert_fixup(rbtree *t, node_t *problem_node){
  node_t *cur_grandparent;
  node_t *cur_parent;
  node_t *cur_uncle;
    
  while(problem_node->parent->color == RBTREE_RED){

    cur_grandparent = problem_node->parent->parent;
    cur_parent = problem_node->parent;

    if (cur_parent == cur_grandparent->left) {
        cur_uncle = cur_grandparent->right;
    } else {
        cur_uncle = cur_grandparent->left;
    }

    if(cur_uncle->color == RBTREE_RED){
      cur_grandparent->color = RBTREE_RED;
      cur_parent->color = RBTREE_BLACK;
      cur_uncle->color = RBTREE_BLACK;
      problem_node = cur_grandparent;
    }
    else{
      if(cur_parent == cur_grandparent->left){
        if(problem_node == cur_parent->right){
          // LR 케이스: 부모 기준 좌회전
          problem_node = cur_parent;
          left_rotate(t, problem_node);

          // 회전 후 참조 재설정
          cur_parent = problem_node->parent;
          cur_grandparent = cur_parent->parent;
        }

        // LL 케이스: 색 보정 후 할아버지 기준 우회전
        cur_parent->color = RBTREE_BLACK;
        cur_grandparent->color = RBTREE_RED;
        right_rotate(t, cur_grandparent);
      }
      else{
        if(problem_node == cur_parent->left){
          // RL 케이스: 부모 기준 우회전
          problem_node = cur_parent;
          right_rotate(t, problem_node);

          // 회전 후 참조 재설정
          cur_parent = problem_node->parent;
          cur_grandparent = cur_parent->parent;
        }

        // RR 케이스: 색 보정 후 할아버지 기준 좌회전
        cur_parent->color = RBTREE_BLACK;
        cur_grandparent->color = RBTREE_RED;
        left_rotate(t, cur_grandparent);
      }
    }
  }
  t->root->color = RBTREE_BLACK;
}

static void left_rotate(rbtree *t, node_t *y){
  node_t *x = y->right;

  y->right = x->left;
  if(x->left != t->nil) x->left->parent = y; 

  x->parent = y->parent;
  if(y->parent != t->nil){
    if(y->parent->right == y){
      y->parent->right = x;
    }
    else if(y->parent->left == y) {
      y->parent->left = x;
    }
  }
  else{
    t->root = x;
  }

  x->left = y;
  y->parent = x;
}

static void right_rotate(rbtree *t, node_t *y){
  node_t *x = y->left;

  y->left = x->right;
  if(x->right != t->nil) x->right->parent = y; 

  x->parent = y->parent;
  if(y->parent != t->nil){
    if(y->parent->left == y){
      y->parent->left = x;
    }
    else if(y->parent->right == y) {
      y->parent->right = x;
    }
  }
  else{
    t->root = x;
  }

  x->right = y;
  y->parent = x;
}

node_t *rbtree_find(const rbtree *t, const key_t key) {
  // TODO: implement find
  return t->root;
}

node_t *rbtree_min(const rbtree *t) {
  // TODO: implement find
  return t->root;
}

node_t *rbtree_max(const rbtree *t) {
  // TODO: implement find
  return t->root;
}

int rbtree_erase(rbtree *t, node_t *p) {
  // TODO: implement erase
  return 0;
}

int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n) {
  // TODO: implement to_array
  return 0;
}

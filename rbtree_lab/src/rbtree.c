#include "rbtree.h"
#include <stdlib.h>

static void free_nodes_recursive(node_t *cur, node_t *nil);
static void rb_insert_fixup(rbtree *t, node_t *problem_node);
static void left_rotate(rbtree *t, node_t *y);
static void right_rotate(rbtree *t, node_t *y);
static void rb_transplant(rbtree *t, node_t *p, node_t *v);
static void rb_delete_fixup(rbtree *t, node_t *replace_node);
static int add_nodes_recursive(node_t *cur, node_t *nil, key_t *arr, const size_t n, int cur_n);

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
  //rbtree의 nil과 root포인터 설정
  p->nil = nil_node;
  p->root = p->nil;

  nil_node->color = RBTREE_BLACK;
  nil_node->parent = p->nil;
  nil_node->left = p->nil;
  nil_node->right = p->nil;

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
    else{
      prev_cur = cur;
      cur = cur->right;
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

static void rb_insert_fixup(rbtree *t, node_t *problem_node) {
    node_t *cur_parent;
    node_t *cur_grandparent;
    node_t *cur_uncle;

    while (problem_node->parent->color == RBTREE_RED) {
        cur_parent = problem_node->parent;
        cur_grandparent = cur_parent->parent;

        // 부모가 할아버지의 왼쪽 자식일 경우
        if (cur_parent == cur_grandparent->left) {
            cur_uncle = cur_grandparent->right;
            
            // Case 1: 삼촌이 RED
            if (cur_uncle->color == RBTREE_RED) {
                cur_parent->color = RBTREE_BLACK;
                cur_uncle->color = RBTREE_BLACK;
                cur_grandparent->color = RBTREE_RED;
                problem_node = cur_grandparent; // 문제를 위로 올리고 계속
            }
            // Case 2 & 3: 삼촌이 BLACK
            else {
                // Case 2: "꺾인" 모양 (LR) -> "직선" 모양 (LL)으로 변경
                if (problem_node == cur_parent->right) {
                    problem_node = cur_parent;
                    left_rotate(t, problem_node);
                    // 회전 후, 바뀐 관계를 반영하기 위해 포인터를 재설정
                    cur_parent = problem_node->parent;
                    cur_grandparent = cur_parent->parent;
                }
                
                // Case 3: "직선" 모양 (LL) -> 문제 해결
                cur_parent->color = RBTREE_BLACK;
                cur_grandparent->color = RBTREE_RED;
                right_rotate(t, cur_grandparent);
            }
        }
        // 부모가 할아버지의 오른쪽 자식일 경우 (위와 완벽히 대칭)
        else {
            cur_uncle = cur_grandparent->left;
            
            // Case 1
            if (cur_uncle->color == RBTREE_RED) {
                cur_parent->color = RBTREE_BLACK;
                cur_uncle->color = RBTREE_BLACK;
                cur_grandparent->color = RBTREE_RED;
                problem_node = cur_grandparent;
            }
            // Case 2 & 3
            else {
                // Case 2: "꺾인" 모양 (RL) -> "직선" 모양 (RR)으로 변경
                if (problem_node == cur_parent->left) {
                    problem_node = cur_parent;
                    right_rotate(t, problem_node);
                    cur_parent = problem_node->parent;
                    cur_grandparent = cur_parent->parent;
                }
                
                // Case 3: "직선" 모양 (RR) -> 문제 해결
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

  if(t == NULL) return NULL;

  node_t *cur = t->root;

  while(cur != t->nil){
    if(cur->key > key){
      cur = cur->left;
    }
    else if(cur->key < key){
      cur = cur->right;
    }
    else{
      return cur;
    }
  }

  return NULL;
}

node_t *rbtree_min(const rbtree *t) {
  if(t == NULL) return NULL;

  node_t *cur = t->root;

  while(cur->left != t->nil){
    cur = cur->left;
  }

  return cur;
}

node_t *rbtree_max(const rbtree *t) {
  if(t == NULL) return NULL;

  node_t *cur = t->root;

  while(cur->right != t->nil){
    cur = cur->right;
  }

  return cur;
}

int rbtree_erase(rbtree *t, node_t *p) {

  node_t *replace_node;
  node_t *remove_node = p;
  color_t original_color = remove_node->color;
  node_t *cur;

  if(p->left != t->nil && p->right != t->nil){
    cur = remove_node->right;
    while(cur->left != t->nil){
      cur = cur->left;
    }
    remove_node = cur;
    replace_node = remove_node->right;
    original_color = remove_node->color;
    p->key = remove_node->key;
    rb_transplant(t, remove_node, replace_node);
  }
  else{
    cur = (p->left != t->nil) ? p->left : p->right;
    replace_node = cur;
    rb_transplant(t, remove_node, replace_node);
  }

  if(original_color == RBTREE_BLACK) rb_delete_fixup(t, replace_node);

  free(remove_node);

  return 0;
}

static void rb_transplant(rbtree *t, node_t *p, node_t *v){

  if(p->parent == t->nil) t->root = v;
  else if(p == p->parent->left) p->parent->left = v;
  else p->parent->right = v;

  v->parent = p->parent;
  
}

static void rb_delete_fixup(rbtree *t, node_t *replace_node){

  int position = 0;
  node_t *parent_node;
  node_t *brother;
  color_t tmp_color;
  
  while(replace_node->parent != t->nil && replace_node->color == RBTREE_BLACK){

    parent_node = replace_node->parent;
    brother = (replace_node->parent->left == replace_node) ? replace_node->parent->right : replace_node->parent->left;
    position = (replace_node->parent->left == replace_node) ? 1 : 2;

    switch(position){
      //brother가 오른쪽일 때
      case 1: 
        if(brother->color == RBTREE_RED){
          parent_node->color = RBTREE_RED;
          brother->color = RBTREE_BLACK;
          left_rotate(t, parent_node);
        }else if(brother->color == RBTREE_BLACK && brother->left->color == RBTREE_BLACK && brother->right->color == RBTREE_BLACK){
          brother->color = RBTREE_RED;
          replace_node = parent_node;
        }else if(brother->color == RBTREE_BLACK && brother->left->color == RBTREE_RED){
          tmp_color = brother->color;
          brother->color = brother->left->color;
          brother->left->color = tmp_color;
          right_rotate(t, brother);
        }else if(brother->color == RBTREE_BLACK && brother->right->color == RBTREE_RED){
          brother->color = parent_node->color;
          parent_node->color = RBTREE_BLACK;
          brother->right->color = RBTREE_BLACK;
          left_rotate(t, parent_node);
          replace_node = t->root;
        }
        break;
      //brother가 왼쪽일 때
      case 2:
        if(brother->color == RBTREE_RED){
          parent_node->color = RBTREE_RED;
          brother->color = RBTREE_BLACK;
          right_rotate(t, parent_node);
        }else if(brother->color == RBTREE_BLACK && brother->right->color == RBTREE_BLACK && brother->left->color == RBTREE_BLACK){
          brother->color = RBTREE_RED;
          replace_node = parent_node;
        }else if(brother->color == RBTREE_BLACK && brother->right->color == RBTREE_RED){
          tmp_color = brother->color;
          brother->color = brother->right->color;
          brother->right->color = tmp_color;
          left_rotate(t, brother);
        }else if(brother->color == RBTREE_BLACK && brother->left->color == RBTREE_RED){
          brother->color = parent_node->color;
          parent_node->color = RBTREE_BLACK;
          brother->left->color = RBTREE_BLACK;
          right_rotate(t, parent_node);
          replace_node = t->root;
        }
        break;
    }
  }
  replace_node->color = RBTREE_BLACK;
}

int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n) {
  if(t == NULL || arr == NULL) return -1;

  add_nodes_recursive(t->root, t->nil, arr, n, 0);

  return 0;
}

static int add_nodes_recursive(node_t *cur, node_t *nil, key_t *arr, const size_t n, int cur_n){
  if(cur == nil || cur_n >= n) return cur_n;
  cur_n = add_nodes_recursive(cur->left, nil, arr, n, cur_n);
  if(cur_n < n) arr[cur_n++] = cur->key;
  cur_n = add_nodes_recursive(cur->right, nil, arr, n, cur_n);
  return cur_n;
}
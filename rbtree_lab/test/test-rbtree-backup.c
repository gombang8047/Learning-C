#include "unity.h"
#include "rbtree.h"
#include <stdlib.h>
#include <stdbool.h>

// A static rbtree pointer, shared across all tests in this file.
// It's initialized in setUp() and cleaned up in tearDown().
static rbtree *t = NULL;

// setUp is called by the Unity framework before each test function.
void setUp(void) {
    t = new_rbtree();
    TEST_ASSERT_NOT_NULL(t); // Ensure rbtree creation was successful.
}

// tearDown is called by the Unity framework after each test function.
void tearDown(void) {
    delete_rbtree(t);
}

// -------------------------------------------------------------------
// Helper functions from the original test code (made static)
// -------------------------------------------------------------------

static void insert_arr(rbtree *tree, const key_t *arr, const size_t n) {
    for (size_t i = 0; i < n; i++) {
        rbtree_insert(tree, arr[i]);
    }
}

static int comp(const void *p1, const void *p2) {
    const key_t *e1 = (const key_t *)p1;
    const key_t *e2 = (const key_t *)p2;
    if (*e1 < *e2) return -1;
    if (*e1 > *e2) return 1;
    return 0;
}

// -------------------------------------------------------------------
// Test Cases (Converted from the original test code)
// -------------------------------------------------------------------

// Test case 1: Verify the initial state of a new rbtree.
void test_initialization(void) {
#ifdef SENTINEL
    TEST_ASSERT_NOT_NULL(t->nil);
    TEST_ASSERT_EQUAL_PTR(t->nil, t->root);
#else
    TEST_ASSERT_NULL(t->root);
#endif
}

// Test case 2: Test insertion of a single node.
void test_insert_single(void) {
    const key_t key = 1024;
    node_t *p = rbtree_insert(t, key);

    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_EQUAL_PTR(p, t->root);
    TEST_ASSERT_EQUAL_INT(key, p->key);

#ifdef SENTINEL
    TEST_ASSERT_EQUAL_PTR(t->nil, p->left);
    TEST_ASSERT_EQUAL_PTR(t->nil, p->right);
    TEST_ASSERT_EQUAL_PTR(t->nil, p->parent);
#else
    TEST_ASSERT_NULL(p->left);
    TEST_ASSERT_NULL(p->right);
    TEST_ASSERT_NULL(p->parent);
#endif
}

// Test case 3: Test finding a node.
void test_find_single(void) {
    const key_t key = 512;
    const key_t wrong_key = 1024;
    node_t *p = rbtree_insert(t, key);

    node_t *q = rbtree_find(t, key);
    TEST_ASSERT_NOT_NULL(q);
    TEST_ASSERT_EQUAL_INT(key, q->key);
    TEST_ASSERT_EQUAL_PTR(p, q);

    q = rbtree_find(t, wrong_key);
    TEST_ASSERT_NULL(q);
}

// Test case 4: Test erasing the root node.
void test_erase_root(void) {
    const key_t key = 128;
    node_t *p = rbtree_insert(t, key);
    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_EQUAL_PTR(t->root, p);

    rbtree_erase(t, p);

#ifdef SENTINEL
    TEST_ASSERT_EQUAL_PTR(t->nil, t->root);
#else
    TEST_ASSERT_NULL(t->root);
#endif
}

// Test case 5: Test finding minimum and maximum values.
void test_minmax(void) {
    key_t arr[] = {10, 5, 8, 34, 67, 23, 156, 24, 2, 12};
    const size_t n = sizeof(arr) / sizeof(arr[0]);
    insert_arr(t, arr, n);

    qsort((void *)arr, n, sizeof(key_t), comp);

    node_t *p = rbtree_min(t);
    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_EQUAL_INT(arr[0], p->key);

    node_t *q = rbtree_max(t);
    TEST_ASSERT_NOT_NULL(q);
    TEST_ASSERT_EQUAL_INT(arr[n - 1], q->key);
}

// Test case 6: Test converting tree to a sorted array.
void test_to_array(void) {
    key_t arr[] = {10, 5, 8, 34, 67, 23, 156, 24, 2, 12, 24, 36, 990, 25};
    const size_t n = sizeof(arr) / sizeof(arr[0]);
    insert_arr(t, arr, n);
    qsort((void *)arr, n, sizeof(key_t), comp);

    key_t *res = calloc(n, sizeof(key_t));
    TEST_ASSERT_NOT_NULL(res);
    rbtree_to_array(t, res, n);
    
    TEST_ASSERT_EQUAL_INT_ARRAY(arr, res, n);

    free(res);
}

// Test case 7: Check insert, find, and erase operations in a sequence.
void test_find_erase_sequence(void) {
    const key_t arr[] = {10, 5, 8, 34, 67, 23, 156, 24, 2, 12, 24, 36, 990, 25};
    const size_t n = sizeof(arr) / sizeof(arr[0]);

    for (size_t i = 0; i < n; i++) {
        rbtree_insert(t, arr[i]);
    }

    for (size_t i = 0; i < n; i++) {
        node_t *p = rbtree_find(t, arr[i]);
        TEST_ASSERT_NOT_NULL(p);
        TEST_ASSERT_EQUAL_INT(arr[i], p->key);
        rbtree_erase(t, p);
    }

    for (size_t i = 0; i < n; i++) {
        node_t *p = rbtree_find(t, arr[i]);
        TEST_ASSERT_NULL(p);
    }
}

// Test case 8: Test with randomized keys.
void test_find_erase_randomized(void) {
    const size_t n = 1000;
    srand(17); // Use a fixed seed for reproducible tests
    
    key_t *arr = calloc(n, sizeof(key_t));
    TEST_ASSERT_NOT_NULL(arr);
    for (size_t i = 0; i < n; i++) {
        arr[i] = rand();
        rbtree_insert(t, arr[i]);
    }

    for (size_t i = 0; i < n; i++) {
        node_t *p = rbtree_find(t, arr[i]);
        TEST_ASSERT_NOT_NULL(p);
        rbtree_erase(t, p);
        p = rbtree_find(t, arr[i]);
        TEST_ASSERT_NULL(p);
    }
    free(arr);
}

// Test case 9: Test creating and managing multiple rbtree instances.
// This test does not use the global `t` from setUp/tearDown.
void test_multiple_instances(void) {
    rbtree *t1 = new_rbtree();
    TEST_ASSERT_NOT_NULL(t1);
    rbtree *t2 = new_rbtree();
    TEST_ASSERT_NOT_NULL(t2);

    key_t arr1[] = {10, 5, 8, 34, 67, 23, 156, 24, 2, 12, 24, 36, 990, 25};
    const size_t n1 = sizeof(arr1) / sizeof(arr1[0]);
    insert_arr(t1, arr1, n1);
    qsort(arr1, n1, sizeof(key_t), comp);

    key_t arr2[] = {4, 8, 10, 5, 3};
    const size_t n2 = sizeof(arr2) / sizeof(arr2[0]);
    insert_arr(t2, arr2, n2);
    qsort(arr2, n2, sizeof(key_t), comp);

    key_t *res1 = calloc(n1, sizeof(key_t));
    rbtree_to_array(t1, res1, n1);
    TEST_ASSERT_EQUAL_INT_ARRAY(arr1, res1, n1);

    key_t *res2 = calloc(n2, sizeof(key_t));
    rbtree_to_array(t2, res2, n2);
    TEST_ASSERT_EQUAL_INT_ARRAY(arr2, res2, n2);

    free(res1);
    free(res2);
    delete_rbtree(t1);
    delete_rbtree(t2);
}


// -------------------------------------------------------------------
// Test Runner
// -------------------------------------------------------------------

// The main function that runs all the tests.
int main(void) {
    UNITY_BEGIN();

    // Register and run the test functions.
    RUN_TEST(test_initialization);
    RUN_TEST(test_insert_single);
    RUN_TEST(test_find_single);
    RUN_TEST(test_erase_root);
    RUN_TEST(test_minmax);
    RUN_TEST(test_to_array);
    RUN_TEST(test_find_erase_sequence);
    RUN_TEST(test_find_erase_randomized);
    RUN_TEST(test_multiple_instances);

    return UNITY_END();
}


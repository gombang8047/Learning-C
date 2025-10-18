// test_rbtree.c
#include "unity.h"
#include "rbtree.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

// 전역 트리 인스턴스(각 테스트마다 setUp/tearDown으로 새로 생성/파괴)
static rbtree *t = NULL;

// ---------------------------------------------------------
// 보조 유틸(테스트 안정성/가독성 향상)
// ---------------------------------------------------------

// 트리의 노드 개수(센티넬 제외)를 재귀로 셈
static size_t count_nodes_rec(const rbtree *tree, const node_t *x) {
    if (x == tree->nil) return 0;
    return 1 + count_nodes_rec(tree, x->left) + count_nodes_rec(tree, x->right);
}
static size_t rbtree_size(const rbtree *tree) {
    return count_nodes_rec(tree, tree->root);
}

// 연속 RED 금지 검사: 어떤 RED 노드도 RED 자식을 가지면 안 됨
static bool check_no_double_red(const rbtree *tree, const node_t *x) {
    if (x == tree->nil) return true;
    if (x->color == RBTREE_RED) {
        if (x->left->color == RBTREE_RED)  return false;
        if (x->right->color == RBTREE_RED) return false;
    }
    return check_no_double_red(tree, x->left) && check_no_double_red(tree, x->right);
}

// 블랙 높이 검사: 각 리프(nil)까지 가는 경로의 블랙 노드 수가 동일해야 함
// 동일하면 해당 블랙 높이(>=1, nil 포함)를 반환, 불일치면 -1 반환
static int black_height_rec(const rbtree *tree, const node_t *x) {
    if (x == tree->nil) return 1; // nil은 BLACK으로 1 카운트(관례)
    int lh = black_height_rec(tree, x->left);
    int rh = black_height_rec(tree, x->right);
    if (lh < 0 || rh < 0 || lh != rh) return -1;
    return lh + (x->color == RBTREE_BLACK ? 1 : 0);
}

// 중위 순회로 res 배열에 키를 채움(최대 n개)
static void inorder_fill(const rbtree *tree, const node_t *x, key_t *res, size_t n, size_t *idx) {
    if (x == tree->nil || *idx >= n) return;
    inorder_fill(tree, x->left, res, n, idx);
    if (*idx < n) res[(*idx)++] = x->key;
    inorder_fill(tree, x->right, res, n, idx);
}

// 유니크 정렬 배열 생성(테스트 기대값용)
static key_t* make_sorted_unique_copy(const key_t *arr, size_t n, size_t *out_m) {
    if (n == 0) { *out_m = 0; return NULL; }
    key_t *tmp = (key_t*)malloc(n * sizeof(key_t));
    if (!tmp) { *out_m = 0; return NULL; }
    for (size_t i = 0; i < n; ++i) tmp[i] = arr[i];
    // 표준 정렬
    int comp(const void *a, const void *b) {
        const key_t *x = (const key_t*)a;
        const key_t *y = (const key_t*)b;
        return (*x > *y) - (*x < *y);
    }
    qsort(tmp, n, sizeof(key_t), comp);
    // 유니크 압축
    size_t m = 1;
    for (size_t i = 1; i < n; ++i) {
        if (tmp[i] != tmp[m - 1]) tmp[m++] = tmp[i];
    }
    key_t *uniq = (key_t*)malloc(m * sizeof(key_t));
    if (!uniq) { free(tmp); *out_m = 0; return NULL; }
    for (size_t i = 0; i < m; ++i) uniq[i] = tmp[i];
    free(tmp);
    *out_m = m;
    return uniq;
}

// ---------------------------------------------------------
// Unity setUp/tearDown
// ---------------------------------------------------------

void setUp(void) {
    t = new_rbtree();
    TEST_ASSERT_NOT_NULL_MESSAGE(t, "new_rbtree() returned NULL");

    // SENTINEL 모드 기준(권장): nil이 BLACK, root가 nil
#ifdef SENTINEL
    TEST_ASSERT_NOT_NULL_MESSAGE(t->nil, "t->nil must be allocated");
    TEST_ASSERT_EQUAL_PTR_MESSAGE(t->nil, t->root, "root must start at nil");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RBTREE_BLACK, t->nil->color, "nil must be BLACK");
#else
    // NULL 기반이라면 root가 NULL일 수 있음(하지만 센티넬을 권장합니다)
    TEST_ASSERT_NULL_MESSAGE(t->root, "root must start NULL in non-sentinel mode");
#endif
}

void tearDown(void) {
    delete_rbtree(t);
    t = NULL;
}

// ---------------------------------------------------------
// 개별 테스트 케이스
// ---------------------------------------------------------

// [테스트 1] 초기화
void test_initialization(void) {
#ifdef SENTINEL
    TEST_ASSERT_NOT_NULL(t->nil);
    TEST_ASSERT_EQUAL_PTR(t->nil, t->root);
    TEST_ASSERT_EQUAL_INT(RBTREE_BLACK, t->nil->color);
#else
    TEST_ASSERT_NULL(t->root);
#endif
}

// [테스트 2] 단일 노드 삽입
void test_insert_single(void) {
    const key_t key = 1024;
    node_t *p = rbtree_insert(t, key);

    TEST_ASSERT_NOT_NULL_MESSAGE(p, "insert should return non-NULL (first key)");
    // 빈 트리에서 첫 삽입이면 루트가 삽입 노드여야 함
    TEST_ASSERT_EQUAL_PTR_MESSAGE(p, t->root, "root must be the inserted node after first insert");
    TEST_ASSERT_EQUAL_INT_MESSAGE(key, p->key, "inserted node must carry the key");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RBTREE_BLACK, t->root->color, "root must be BLACK after fixup");
#ifdef SENTINEL
    TEST_ASSERT_EQUAL_PTR_MESSAGE(t->nil, p->left,  "leaf left should be nil");
    TEST_ASSERT_EQUAL_PTR_MESSAGE(t->nil, p->right, "leaf right should be nil");
#endif
    // RBT 성질 점검
    TEST_ASSERT_TRUE_MESSAGE(check_no_double_red(t, t->root), "No RED node should have RED child");
    TEST_ASSERT_NOT_EQUAL_MESSAGE(-1, black_height_rec(t, t->root), "All root->leaf paths must have equal black height");
}

// [테스트 3] 단일 노드 검색
void test_find_single(void) {
    const key_t key = 512;
    const key_t wrong_key = 1024;
    node_t *p = rbtree_insert(t, key);
    TEST_ASSERT_NOT_NULL(p);

    node_t *q = rbtree_find(t, key);
    TEST_ASSERT_NOT_NULL(q);
    TEST_ASSERT_EQUAL_PTR(p, q);

    q = rbtree_find(t, wrong_key);
    TEST_ASSERT_NULL(q);
}

// [테스트 4] 루트 삭제
void test_erase_root(void) {
    const key_t key = 128;
    node_t *p = rbtree_insert(t, key);
    TEST_ASSERT_NOT_NULL(p);

    rbtree_erase(t, p);
#ifdef SENTINEL
    TEST_ASSERT_EQUAL_PTR(t->nil, t->root);
#else
    TEST_ASSERT_NULL(t->root);
#endif
    // 삭제 후 성질 점검(빈 트리이므로 OK)
    TEST_ASSERT_TRUE(check_no_double_red(t, t->root));
    TEST_ASSERT_NOT_EQUAL(-1, black_height_rec(t, t->root));
}

// [테스트 5] Min/Max
void test_minmax(void) {
    key_t arr[] = {10, 5, 8, 34, 67, 23, 156, 24, 2, 12};
    const size_t n = sizeof(arr) / sizeof(arr[0]);
    for (size_t i = 0; i < n; ++i) {
        TEST_ASSERT_NOT_NULL(rbtree_insert(t, arr[i]));
    }

    // 기대값 계산
    int comp(const void *a, const void *b) {
        const key_t *x = (const key_t*)a;
        const key_t *y = (const key_t*)b;
        return (*x > *y) - (*x < *y);
    }
    qsort(arr, n, sizeof(key_t), comp);

    node_t *p = rbtree_min(t);
    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_EQUAL_INT(arr[0], p->key);

    node_t *q = rbtree_max(t);
    TEST_ASSERT_NOT_NULL(q);
    TEST_ASSERT_EQUAL_INT(arr[n - 1], q->key);

    // 성질 점검
    TEST_ASSERT_TRUE(check_no_double_red(t, t->root));
    TEST_ASSERT_NOT_EQUAL(-1, black_height_rec(t, t->root));
}

// [테스트 6] to_array (중복 키 입력을 고려해 '유니크 정렬' 기준으로 비교)
void test_to_array(void) {
    key_t arr[] = {10, 5, 8, 34, 67, 23, 156, 24, 2, 12, 24, 36, 990, 25};
    const size_t n = sizeof(arr) / sizeof(arr[0]);

    // 삽입(중복은 구현에 따라 무시됨)
    for (size_t i = 0; i < n; ++i) (void)rbtree_insert(t, arr[i]);

    // 기대값: 유니크 정렬 배열
    size_t m = 0;
    key_t *uniq = make_sorted_unique_copy(arr, n, &m);
    TEST_ASSERT_NOT_NULL_MESSAGE(uniq, "allocation failed in test helper");

    // 결과 배열: 트리 크기만큼만 채움
    size_t size = rbtree_size(t);
    TEST_ASSERT_EQUAL_MESSAGE(m, size, "tree size must equal unique key count");
    key_t *res = (key_t*)calloc(size, sizeof(key_t));
    TEST_ASSERT_NOT_NULL(res);

    rbtree_to_array(t, res, size);

    TEST_ASSERT_EQUAL_INT_ARRAY(uniq, res, size);

    free(uniq);
    free(res);

    // 성질 점검
    TEST_ASSERT_TRUE(check_no_double_red(t, t->root));
    TEST_ASSERT_NOT_EQUAL(-1, black_height_rec(t, t->root));
}

// [테스트 7] 삽입-검색-삭제 시퀀스
void test_find_erase_sequence(void) {
    const key_t arr[] = {10, 5, 8, 34, 67, 23, 156, 24, 2, 12, 24, 36, 990, 25};
    const size_t n = sizeof(arr) / sizeof(arr[0]);

    for (size_t i = 0; i < n; ++i) (void)rbtree_insert(t, arr[i]);

    // 유니크 개수 계산
    size_t m = 0;
    key_t *uniq = make_sorted_unique_copy(arr, n, &m);
    TEST_ASSERT_NOT_NULL(uniq);

    // 모두 찾아서 삭제
    for (size_t i = 0; i < m; ++i) {
        node_t *p = rbtree_find(t, uniq[i]);
        TEST_ASSERT_NOT_NULL(p);
        rbtree_erase(t, p);
        TEST_ASSERT_NULL(rbtree_find(t, uniq[i]));
    }

    // 트리가 비었는지 확인
#ifdef SENTINEL
    TEST_ASSERT_EQUAL_PTR(t->nil, t->root);
#else
    TEST_ASSERT_NULL(t->root);
#endif
    TEST_ASSERT_EQUAL_UINT(0u, rbtree_size(t));

    free(uniq);
}

// [테스트 8] 다중 인스턴스
void test_multiple_instances(void) {
    rbtree *t1 = new_rbtree();
    rbtree *t2 = new_rbtree();
    TEST_ASSERT_NOT_NULL(t1);
    TEST_ASSERT_NOT_NULL(t2);

#ifdef SENTINEL
    TEST_ASSERT_EQUAL_PTR(t1->nil, t1->root);
    TEST_ASSERT_EQUAL_PTR(t2->nil, t2->root);
#endif

    key_t arr1[] = {10, 5, 8, 34, 67, 23, 156, 24, 2, 12};
    key_t arr2[] = {4, 8, 11, 5, 3};

    for (size_t i = 0; i < sizeof(arr1)/sizeof(arr1[0]); ++i) (void)rbtree_insert(t1, arr1[i]);
    for (size_t i = 0; i < sizeof(arr2)/sizeof(arr2[0]); ++i) (void)rbtree_insert(t2, arr2[i]);

    TEST_ASSERT_NOT_NULL(rbtree_find(t1, 10));
    TEST_ASSERT_NULL(rbtree_find(t1, 4));
    TEST_ASSERT_NOT_NULL(rbtree_find(t2, 4));
    TEST_ASSERT_NULL(rbtree_find(t2, 10));

    // 성질 점검
    TEST_ASSERT_TRUE(check_no_double_red(t1, t1->root));
    TEST_ASSERT_NOT_EQUAL(-1, black_height_rec(t1, t1->root));
    TEST_ASSERT_TRUE(check_no_double_red(t2, t2->root));
    TEST_ASSERT_NOT_EQUAL(-1, black_height_rec(t2, t2->root));

    delete_rbtree(t1);
    delete_rbtree(t2);
}

// ---------------------------------------------------------
// Test Runner
// ---------------------------------------------------------
int main(void) {
    UNITY_BEGIN();

    printf("------------------------------------------\n");
    printf(" [Test 1: Initialization]\n");
    RUN_TEST(test_initialization);

    printf("\n [Test 2: Insert Single Node]\n");
    RUN_TEST(test_insert_single);

    printf("\n [Test 3: Find Single Node]\n");
    RUN_TEST(test_find_single);

    printf("\n [Test 4: Erase Root Node]\n");
    RUN_TEST(test_erase_root);

    printf("\n [Test 5: Min/Max]\n");
    RUN_TEST(test_minmax);

    printf("\n [Test 6: To Array]\n");
    RUN_TEST(test_to_array);

    printf("\n [Test 7: Insert/Find/Erase Sequence]\n");
    RUN_TEST(test_find_erase_sequence);

    printf("\n [Test 8: Multiple Instances]\n");
    RUN_TEST(test_multiple_instances);
    printf("------------------------------------------\n");

    return UNITY_END();
}

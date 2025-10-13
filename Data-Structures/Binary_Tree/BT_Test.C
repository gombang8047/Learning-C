//////////////////////////////////////////////////////////////////////////////////
/* Enhanced Binary Tree Test Suite with Detailed Error Reporting */
//////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

//////////////////////////////////////////////////////////////////////////////////
// Test Statistics
//////////////////////////////////////////////////////////////////////////////////

typedef struct {
    int total_tests;
    int passed_tests;
    int failed_tests;
} TestStats;

TestStats global_stats = {0, 0, 0};

//////////////////////////////////////////////////////////////////////////////////
// Enhanced Assertion Macros
//////////////////////////////////////////////////////////////////////////////////

#define TEST_ASSERT_INT_EQ(actual, expected, test_name) do { \
    global_stats.total_tests++; \
    if ((actual) != (expected)) { \
        global_stats.failed_tests++; \
        printf("❌ FAILED: %s\n", test_name); \
        printf("   Expected: %d\n", (expected)); \
        printf("   Actual:   %d\n", (actual)); \
        printf("   Location: Line %d\n", __LINE__); \
        return; \
    } else { \
        global_stats.passed_tests++; \
        printf("✓ %s\n", test_name); \
    } \
} while(0)

//////////////////////////////////////////////////////////////////////////////////
// Data Structures
//////////////////////////////////////////////////////////////////////////////////

typedef struct _btnode {
    int item;
    struct _btnode *left;
    struct _btnode *right;
} BTNode;

//////////////////////////////////////////////////////////////////////////////////
// Helper Functions
//////////////////////////////////////////////////////////////////////////////////

BTNode* createBTNode(int item) {
    BTNode *newNode = (BTNode*)malloc(sizeof(BTNode));
    newNode->item = item;
    newNode->left = NULL;
    newNode->right = NULL;
    return newNode;
}

void removeAll(BTNode **node) {
    if(*node != NULL) {
        removeAll(&((*node)->left));
        removeAll(&((*node)->right));
        free(*node);
        *node = NULL;
    }
}

void printTreeStructure(BTNode *node, int level, const char *prefix) {
    if (node == NULL) {
        for (int i = 0; i < level; i++) printf("    ");
        printf("%sNULL\n", prefix);
        return;
    }
    printTreeStructure(node->right, level + 1, "R: ");
    for (int i = 0; i < level; i++) printf("    ");
    printf("%s%d\n", prefix, node->item);
    printTreeStructure(node->left, level + 1, "L: ");
}

BTNode* createSampleTree1() {
    BTNode *root = createBTNode(5);
    root->left = createBTNode(3);
    root->right = createBTNode(7);
    root->left->left = createBTNode(1);
    root->left->right = createBTNode(2);
    root->right->left = createBTNode(4);
    root->right->right = createBTNode(8);
    return root;
}

BTNode* createSampleTree2() {
    BTNode *root = createBTNode(4);
    root->left = createBTNode(2);
    root->right = createBTNode(6);
    root->left->left = createBTNode(1);
    root->left->right = createBTNode(3);
    root->right->left = createBTNode(5);
    root->right->right = createBTNode(7);
    return root;
}

BTNode* createSampleTree3() {
    BTNode *root = createBTNode(50);
    root->left = createBTNode(20);
    root->right = createBTNode(60);
    root->left->left = createBTNode(10);
    root->left->right = createBTNode(30);
    root->left->right->left = createBTNode(55);
    root->right->right = createBTNode(80);
    return root;
}

int compareTreesDetailed(BTNode *tree1, BTNode *tree2, const char *test_name, int show_structure) {
    global_stats.total_tests++;
    
    if (tree1 == NULL && tree2 == NULL) {
        global_stats.passed_tests++;
        printf("✓ %s\n", test_name);
        return 1;
    }
    
    if (tree1 == NULL || tree2 == NULL) {
        global_stats.failed_tests++;
        printf("❌ FAILED: %s\n", test_name);
        printf("   Structure mismatch (NULL vs non-NULL)\n");
        if (show_structure) {
            printf("   Expected:\n");
            printTreeStructure(tree2, 1, "");
            printf("   Actual:\n");
            printTreeStructure(tree1, 1, "");
        }
        return 0;
    }
    
    if (tree1->item != tree2->item) {
        global_stats.failed_tests++;
        printf("❌ FAILED: %s\n", test_name);
        printf("   Value mismatch: Expected %d, Got %d\n", tree2->item, tree1->item);
        if (show_structure) {
            printf("   Expected:\n");
            printTreeStructure(tree2, 1, "");
            printf("   Actual:\n");
            printTreeStructure(tree1, 1, "");
        }
        return 0;
    }
    
    // Recursively check without printing (to avoid duplicate stats)
    int left_match = 1, right_match = 1;
    
    if ((tree1->left == NULL) != (tree2->left == NULL) || 
        (tree1->left && tree2->left && tree1->left->item != tree2->left->item)) {
        left_match = 0;
    }
    
    if ((tree1->right == NULL) != (tree2->right == NULL) || 
        (tree1->right && tree2->right && tree1->right->item != tree2->right->item)) {
        right_match = 0;
    }
    
    if (!left_match || !right_match) {
        global_stats.failed_tests++;
        printf("❌ FAILED: %s\n", test_name);
        printf("   Subtree mismatch\n");
        if (show_structure) {
            printf("   Expected:\n");
            printTreeStructure(tree2, 1, "");
            printf("   Actual:\n");
            printTreeStructure(tree1, 1, "");
        }
        global_stats.total_tests--; // Compensate for the increment at start
        return 0;
    }
    
    global_stats.passed_tests++;
    printf("✓ %s\n", test_name);
    return 1;
}

// For testing print functions
static int printed_values[100];
static int printed_count = 0;

void reset_printed() {
    printed_count = 0;
}

void capture_print(int value) {
    if (printed_count < 100) {
        printed_values[printed_count++] = value;
    }
}

//////////////////////////////////////////////////////////////////////////////////
// Function Prototypes
//////////////////////////////////////////////////////////////////////////////////

int identical(BTNode *tree1, BTNode *tree2);
int maxHeight(BTNode *root);
int countOneChildNodes(BTNode *root);
int sumOfOddNodes(BTNode *root);
void mirrorTree(BTNode *node);
void printSmallerValues(BTNode *node, int m);
int smallestValue(BTNode *node);
int hasGreatGrandchild(BTNode *node);

//////////////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////////////

void test_identical() {
    printf("\n=== Testing Q1: identical ===\n");
    BTNode *tree1, *tree2;
    
    // Test 1
    tree1 = createSampleTree1();
    tree2 = createSampleTree1();
    TEST_ASSERT_INT_EQ(identical(tree1, tree2), 1, "Test 1: Two identical trees");
    removeAll(&tree1);
    removeAll(&tree2);
    
    // Test 2
    tree1 = createSampleTree1();
    tree2 = createSampleTree2();
    TEST_ASSERT_INT_EQ(identical(tree1, tree2), 0, "Test 2: Two different trees");
    removeAll(&tree1);
    removeAll(&tree2);
    
    // Test 3
    tree1 = NULL;
    tree2 = NULL;
    TEST_ASSERT_INT_EQ(identical(tree1, tree2), 1, "Test 3: Both NULL trees");
    
    // Test 4
    tree1 = createBTNode(5);
    tree2 = NULL;
    TEST_ASSERT_INT_EQ(identical(tree1, tree2), 0, "Test 4: One NULL, one not");
    removeAll(&tree1);
    
    // Test 5
    tree1 = createBTNode(5);
    tree1->left = createBTNode(3);
    tree2 = createBTNode(5);
    tree2->left = createBTNode(4);
    TEST_ASSERT_INT_EQ(identical(tree1, tree2), 0, "Test 5: Different values");
    removeAll(&tree1);
    removeAll(&tree2);
}

void test_maxHeight() {
    printf("\n=== Testing Q2: maxHeight ===\n");
    BTNode *tree;
    
    // Test 1
    tree = createSampleTree2();
    TEST_ASSERT_INT_EQ(maxHeight(tree), 2, "Test 1: Balanced tree height = 2");
    removeAll(&tree);
    
    // Test 2
    tree = createBTNode(5);
    TEST_ASSERT_INT_EQ(maxHeight(tree), 0, "Test 2: Single node height = 0");
    removeAll(&tree);
    
    // Test 3
    tree = NULL;
    TEST_ASSERT_INT_EQ(maxHeight(tree), -1, "Test 3: NULL tree height = -1");
    
    // Test 4
    tree = createBTNode(1);
    tree->left = createBTNode(2);
    tree->left->left = createBTNode(3);
    tree->left->left->left = createBTNode(4);
    TEST_ASSERT_INT_EQ(maxHeight(tree), 3, "Test 4: Skewed tree height = 3");
    removeAll(&tree);
    
    // Test 5
    tree = createSampleTree3();
    TEST_ASSERT_INT_EQ(maxHeight(tree), 3, "Test 5: Unbalanced tree height = 3");
    removeAll(&tree);
}

void test_countOneChildNodes() {
    printf("\n=== Testing Q3: countOneChildNodes ===\n");
    BTNode *tree;
    
    // Test 1
    tree = createSampleTree3();
    TEST_ASSERT_INT_EQ(countOneChildNodes(tree), 2, "Test 1: Tree with 2 one-child nodes");
    removeAll(&tree);
    
    // Test 2
    tree = createSampleTree2();
    TEST_ASSERT_INT_EQ(countOneChildNodes(tree), 0, "Test 2: Perfect binary tree = 0");
    removeAll(&tree);
    
    // Test 3
    tree = createBTNode(5);
    TEST_ASSERT_INT_EQ(countOneChildNodes(tree), 0, "Test 3: Single node = 0");
    removeAll(&tree);
    
    // Test 4
    tree = createBTNode(1);
    tree->left = createBTNode(2);
    tree->left->right = createBTNode(3);
    TEST_ASSERT_INT_EQ(countOneChildNodes(tree), 2, "Test 4: Chain = 2");
    removeAll(&tree);
    
    // Test 5
    tree = NULL;
    TEST_ASSERT_INT_EQ(countOneChildNodes(tree), 0, "Test 5: NULL tree = 0");
}

void test_sumOfOddNodes() {
    printf("\n=== Testing Q4: sumOfOddNodes ===\n");
    BTNode *tree;
    
    // Test 1
    tree = createBTNode(50);
    tree->left = createBTNode(40);
    tree->right = createBTNode(60);
    tree->left->left = createBTNode(11);
    tree->left->right = createBTNode(35);
    tree->right->left = createBTNode(80);
    tree->right->right = createBTNode(85);
    TEST_ASSERT_INT_EQ(sumOfOddNodes(tree), 131, "Test 1: Mixed sum = 131");
    removeAll(&tree);
    
    // Test 2
    tree = createBTNode(2);
    tree->left = createBTNode(4);
    tree->right = createBTNode(6);
    TEST_ASSERT_INT_EQ(sumOfOddNodes(tree), 0, "Test 2: All even = 0");
    removeAll(&tree);
    
    // Test 3
    tree = createBTNode(1);
    tree->left = createBTNode(3);
    tree->right = createBTNode(5);
    TEST_ASSERT_INT_EQ(sumOfOddNodes(tree), 9, "Test 3: All odd = 9");
    removeAll(&tree);
    
    // Test 4
    tree = createBTNode(7);
    TEST_ASSERT_INT_EQ(sumOfOddNodes(tree), 7, "Test 4: Single odd = 7");
    removeAll(&tree);
    
    // Test 5
    tree = NULL;
    TEST_ASSERT_INT_EQ(sumOfOddNodes(tree), 0, "Test 5: NULL tree = 0");
}

void test_mirrorTree() {
    printf("\n=== Testing Q5: mirrorTree ===\n");
    BTNode *tree, *expected;
    
    // Test 1
    tree = createBTNode(4);
    tree->left = createBTNode(2);
    tree->right = createBTNode(6);
    tree->left->left = createBTNode(1);
    tree->left->right = createBTNode(3);
    tree->right->left = createBTNode(5);
    tree->right->right = createBTNode(7);
    
    expected = createBTNode(4);
    expected->left = createBTNode(6);
    expected->right = createBTNode(2);
    expected->left->left = createBTNode(7);
    expected->left->right = createBTNode(5);
    expected->right->left = createBTNode(3);
    expected->right->right = createBTNode(1);
    
    mirrorTree(tree);
    compareTreesDetailed(tree, expected, "Test 1: Mirror tree", 1);
    removeAll(&tree);
    removeAll(&expected);
    
    // Test 2
    tree = createBTNode(5);
    mirrorTree(tree);
    global_stats.total_tests++;
    if (tree->item == 5 && tree->left == NULL && tree->right == NULL) {
        global_stats.passed_tests++;
        printf("✓ Test 2: Single node unchanged\n");
    } else {
        global_stats.failed_tests++;
        printf("❌ FAILED: Test 2: Single node changed\n");
    }
    removeAll(&tree);
    
    // Test 3
    tree = NULL;
    mirrorTree(tree);
    global_stats.total_tests++;
    if (tree == NULL) {
        global_stats.passed_tests++;
        printf("✓ Test 3: NULL tree unchanged\n");
    } else {
        global_stats.failed_tests++;
        printf("❌ FAILED: Test 3: NULL tree modified\n");
    }
    
    // Test 4
    tree = createSampleTree2();
    expected = createSampleTree2();
    mirrorTree(tree);
    mirrorTree(tree);
    compareTreesDetailed(tree, expected, "Test 4: Double mirror", 0);
    removeAll(&tree);
    removeAll(&expected);
}

void test_printSmallerValues() {
    printf("\n=== Testing Q6: printSmallerValues ===\n");
    BTNode *tree;
    
    // Test 1
    tree = createBTNode(50);
    tree->left = createBTNode(30);
    tree->right = createBTNode(60);
    tree->left->left = createBTNode(25);
    tree->left->right = createBTNode(65);
    tree->right->left = createBTNode(10);
    tree->right->right = createBTNode(75);
    
    reset_printed();
    printSmallerValues(tree, 55);
    global_stats.total_tests++;
    if (printed_count == 4 && printed_values[0] == 50 && printed_values[1] == 30 && printed_values[2] == 25 && printed_values[3] == 10) {
        global_stats.passed_tests++;
        printf("✓ Test 1: Found 4 values < 55\n");
    } else {
        global_stats.failed_tests++;
        printf("❌ FAILED: Test 1: Expected 50, 30, 25, 10 values, got ");
        for (int i = 0; i < printed_count; i++)
            printf("%d ", printed_values[i]);
        printf("\n");
    }
    removeAll(&tree);
    
    // Test 2
    tree = createBTNode(100);
    tree->left = createBTNode(200);
    reset_printed();
    printed_count = 0;
    printSmallerValues(tree, 50);
    global_stats.total_tests++;
    if (printed_count == 0) {
        global_stats.passed_tests++;
        printf("✓ Test 2: No values smaller\n");
    } else {
        global_stats.failed_tests++;
        printf("❌ FAILED: Test 2: Expected 0 values, got ");
        for (int i = 0; i < printed_count; i++)
            printf("%d ", printed_values[i]);
        printf("\n");
    }
    removeAll(&tree);
    
    // Test 3
    tree = createBTNode(10);
    tree->left = createBTNode(5);
    tree->right = createBTNode(15);
    reset_printed();
    printed_count = 0;
    printSmallerValues(tree, 100);
    global_stats.total_tests++;
    if (printed_values[0] == 10 && printed_values[1] == 5 && printed_values[2] == 15) {
        global_stats.passed_tests++;
        printf("✓ Test 3: All values smaller\n");
    } else {
        global_stats.failed_tests++;
        printf("❌ FAILED: Test 3: Expected 3 values, got ");
        for (int i = 0; i < printed_count; i++)
            printf("%d ", printed_values[i]);
        printf("\n");
    }
    removeAll(&tree);
}

void test_smallestValue() {
    printf("\n=== Testing Q7: smallestValue ===\n");
    BTNode *tree;
    
    // Test 1
    tree = createBTNode(50);
    tree->left = createBTNode(30);
    tree->right = createBTNode(60);
    tree->left->left = createBTNode(25);
    tree->left->right = createBTNode(65);
    tree->right->left = createBTNode(10);
    tree->right->right = createBTNode(75);
    TEST_ASSERT_INT_EQ(smallestValue(tree), 10, "Test 1: Smallest = 10");
    removeAll(&tree);
    
    // Test 2
    tree = createBTNode(42);
    TEST_ASSERT_INT_EQ(smallestValue(tree), 42, "Test 2: Single node = 42");
    removeAll(&tree);
    
    // Test 3
    tree = createBTNode(1);
    tree->left = createBTNode(100);
    tree->right = createBTNode(200);
    TEST_ASSERT_INT_EQ(smallestValue(tree), 1, "Test 3: Smallest at root");
    removeAll(&tree);
    
    // Test 4
    tree = createBTNode(50);
    tree->left = createBTNode(20);
    tree->left->left = createBTNode(5);
    tree->right = createBTNode(100);
    TEST_ASSERT_INT_EQ(smallestValue(tree), 5, "Test 4: Smallest in left");
    removeAll(&tree);
    
    // Test 5
    tree = createBTNode(0);
    tree->left = createBTNode(-10);
    tree->right = createBTNode(10);
    TEST_ASSERT_INT_EQ(smallestValue(tree), -10, "Test 5: Negative values");
    removeAll(&tree);
}

void test_hasGreatGrandchild() {
    printf("\n=== Testing Q8: hasGreatGrandchild ===\n");
    BTNode *tree;
    
    // Test 1
    tree = createBTNode(50);
    tree->left = createBTNode(30);
    tree->right = createBTNode(60);
    tree->left->left = createBTNode(25);
    tree->left->right = createBTNode(65);
    tree->left->right->left = createBTNode(20);
    tree->right->left = createBTNode(10);
    tree->right->right = createBTNode(75);
    tree->right->right->right = createBTNode(15);
    
    reset_printed();
    hasGreatGrandchild(tree);
    global_stats.total_tests++;
    if (printed_count == 1 && printed_values[0] == 50) {
        global_stats.passed_tests++;
        printf("✓ Test 1: Found 1 node (50)\n");
    } else {
        global_stats.failed_tests++;
        printf("❌ FAILED: Test 1: Expected 1 node (50), got %d nodes\n", printed_count);
    }
    removeAll(&tree);
    
    // Test 2
    tree = createSampleTree2();
    reset_printed();
    hasGreatGrandchild(tree);
    global_stats.total_tests++;
    if (printed_count == 0) {
        global_stats.passed_tests++;
        printf("✓ Test 2: No great-grandchildren\n");
    } else {
        global_stats.failed_tests++;
        printf("❌ FAILED: Test 2: Expected 0, got %d\n", printed_count);
    }
    removeAll(&tree);
    
    // Test 3
    tree = createBTNode(1);
    tree->left = createBTNode(2);
    tree->right = createBTNode(3);
    tree->left->left = createBTNode(4);
    tree->left->right = createBTNode(5);
    tree->right->left = createBTNode(6);
    tree->right->right = createBTNode(7);
    tree->left->left->left = createBTNode(8);
    tree->right->right->right = createBTNode(9);
    tree->right->right->right->left = createBTNode(10);
    
    reset_printed();
    hasGreatGrandchild(tree);
    global_stats.total_tests++;
    if (printed_count == 2 && printed_values[0] == 3 && printed_values[1] == 1) {
        global_stats.passed_tests++;
        printf("✓ Test 3: Found 2 nodes\n");
    } else {
        global_stats.failed_tests++;
        printf("❌ FAILED: Test 3: Expected 3, 1 values, got ");
        for (int i = 0; i < printed_count; i++)
            printf("%d ", printed_values[i]);
        printf("\n");
    }
    removeAll(&tree);
}

//////////////////////////////////////////////////////////////////////////////////
// Test Summary
//////////////////////////////////////////////////////////////////////////////////

void print_test_summary() {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║               TEST SUITE SUMMARY                       ║\n");
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║  Total Tests:  %-4d                                    ║\n", global_stats.total_tests);
    printf("║  Passed:       %-4d  ✅                                ║\n", global_stats.passed_tests);
    printf("║  Failed:       %-4d  ❌                                ║\n", global_stats.failed_tests);
    printf("╠════════════════════════════════════════════════════════╣\n");
    
    if (global_stats.failed_tests == 0) {
        printf("║  🎉 ALL TESTS PASSED! 🎉                              ║\n");
    } else {
        double pass_rate = (double)global_stats.passed_tests / global_stats.total_tests * 100;
        printf("║  Pass Rate: %.1f%%                                     ║\n", pass_rate);
        printf("║  ⚠️  Some tests failed. Review errors above.          ║\n");
    }
    
    printf("╚════════════════════════════════════════════════════════╝\n");
}

//////////////////////////////////////////////////////////////////////////////////
// MAIN
//////////////////////////////////////////////////////////////////////////////////

int main() {
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║  Binary Tree Test Suite - All 8 Questions             ║\n");
    printf("║  Enhanced with Detailed Error Reporting               ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n");
    
    test_identical();
    test_maxHeight();
    test_countOneChildNodes();
    test_sumOfOddNodes();
    test_mirrorTree();
    test_printSmallerValues();
    test_smallestValue();
    test_hasGreatGrandchild();
    
    print_test_summary();
    
    return (global_stats.failed_tests == 0) ? 0 : 1;
}

//////////////////////////////////////////////////////////////////////////////////
// Q1: identical
//////////////////////////////////////////////////////////////////////////////////

int identical(BTNode *tree1, BTNode *tree2)

{
   /* add your code here */
    if(tree1 == NULL && tree2 == NULL) return 1;
    if(tree1 == NULL || tree2 == NULL || tree1->item != tree2->item) return 0;

    return identical(tree1->left, tree2->left) && identical(tree1->right, tree2->right);
}

//////////////////////////////////////////////////////////////////////////////////
// Q2: maxHeight
//////////////////////////////////////////////////////////////////////////////////

int maxHeight(BTNode *node)
{
    // 1. 기저 조건(Base Case): 현재 노드가 NULL이면 -1을 반환합니다.
    if (node == NULL) {
        return -1;
    }

    // 2. 재귀 단계(Recursive Step)
    // 왼쪽 서브트리의 높이를 계산합니다.
    int leftHeight = maxHeight(node->left);

    // 오른쪽 서브트리의 높이를 계산합니다.
    int rightHeight = maxHeight(node->right);

    // 3. 왼쪽과 오른쪽 높이 중 더 큰 값을 선택하고,
    //    현재 노드의 높이인 1을 더해서 반환합니다.
    if (leftHeight > rightHeight) {
        return leftHeight + 1;
    } else {
        return rightHeight + 1;
    }
}


//////////////////////////////////////////////////////////////////////////////////
// Q3: countOneChildNodes
//////////////////////////////////////////////////////////////////////////////////

int countOneChildNodes(BTNode *node)

{
    /* add your code here */
    if(node == NULL) return 0;

    int a = countOneChildNodes(node->left);
    int b = countOneChildNodes(node->right);

    if((node->left == NULL && node->right != NULL) || (node->left != NULL && node->right == NULL)){
        return a + b + 1;
    }

    return a + b;
}

//////////////////////////////////////////////////////////////////////////////////
// Q4: sumOfOddNodes
//////////////////////////////////////////////////////////////////////////////////

int sumOfOddNodes(BTNode *node)

{
    /* add your code here */
    if(node == NULL) return 0;

    int left = sumOfOddNodes(node->left);
    int right = sumOfOddNodes(node->right);

    if(node->item % 2 == 1){
        return left + right + node->item;
    }

    return left + right;
}

//////////////////////////////////////////////////////////////////////////////////
// Q5: mirrorTree
//////////////////////////////////////////////////////////////////////////////////

void mirrorTree(BTNode *node)
{
	/* add your code here */
    if(node == NULL) return;

    mirrorTree(node->left);
    mirrorTree(node->right);

    BTNode *tmp = node->left;
    node->left = node->right;
    node->right = tmp;

}


//////////////////////////////////////////////////////////////////////////////////
// Q6: printSmallerValues
//////////////////////////////////////////////////////////////////////////////////


void printSmallerValues(BTNode *node, int m)
{
	/* add your code here */
    if(node == NULL) return;

    if(node->item < m) capture_print(node->item);
    printSmallerValues(node->left, m);
    printSmallerValues(node->right, m);
}

//////////////////////////////////////////////////////////////////////////////////
// Q7: smallestValue
//////////////////////////////////////////////////////////////////////////////////

int smallestValue(BTNode *node)
{
	/* add your code here */
    if(node == NULL) return INT_MAX;

    int a = smallestValue(node->left);
    int b = smallestValue(node->right);

    int min_val = node->item;
    if (a < min_val) min_val = a;
    if (b < min_val) min_val = b;
    return min_val;
}

//////////////////////////////////////////////////////////////////////////////////
// Q8: hasGreatGrandchild
//////////////////////////////////////////////////////////////////////////////////

int hasGreatGrandchild(BTNode *node)
{
	/* add your code here */
    if(node == NULL) return 0;

    int left_depth = hasGreatGrandchild(node->left);
    int right_depth = hasGreatGrandchild(node->right);

    int max_depth = (left_depth > right_depth) ? left_depth : right_depth;

    if(max_depth >= 3){
        capture_print(node->item);
    }

    return max_depth + 1;
}

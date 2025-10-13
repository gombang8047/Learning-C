//////////////////////////////////////////////////////////////////////////////////
/* Comprehensive Test Suite for Stack and Queue Questions
   All implementations included - Ready to compile and run! */
//////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <limits.h>

#define MIN_INT -1000

//////////////////////////////////////////////////////////////////////////////////
// Data Structure Definitions
//////////////////////////////////////////////////////////////////////////////////

typedef struct _listnode {
    int item;
    struct _listnode *next;
} ListNode;

typedef struct _linkedlist {
    int size;
    ListNode *head;
    ListNode *tail;
} LinkedList;

typedef struct _stack {
    LinkedList ll;
} Stack;

typedef struct _queue {
    LinkedList ll;
} Queue;

//////////////////////////////////////////////////////////////////////////////////
// Helper Functions
//////////////////////////////////////////////////////////////////////////////////

void initList(LinkedList *ll) {
    ll->head = NULL;
    ll->tail = NULL;
    ll->size = 0;
}

void printList(LinkedList *ll) {
    ListNode *cur = ll->head;
    if (cur == NULL) {
        printf("Empty");
    }
    while (cur != NULL) {
        printf("%d ", cur->item);
        cur = cur->next;
    }
    printf("\n");
}

void removeAllItems(LinkedList *ll) {
    ListNode *cur = ll->head;
    ListNode *tmp;
    while (cur != NULL) {
        tmp = cur->next;
        free(cur);
        cur = tmp;
    }
    ll->head = NULL;
    ll->tail = NULL;
    ll->size = 0;
}

ListNode *findNode(LinkedList *ll, int index) {
    ListNode *temp;
    if (ll == NULL || index < 0 || index >= ll->size)
        return NULL;
    temp = ll->head;
    if (temp == NULL || index < 0)
        return NULL;
    while (index > 0) {
        temp = temp->next;
        if (temp == NULL)
            return NULL;
        index--;
    }
    return temp;
}

int insertNode(LinkedList *ll, int index, int value) {
    ListNode *pre, *cur;
    if (ll == NULL || index < 0 || index > ll->size)
        return -1;
    
    if (ll->head == NULL || index == 0) {
        cur = ll->head;
        ll->head = malloc(sizeof(ListNode));
        ll->head->item = value;
        ll->head->next = cur;
        ll->size++;
        return 0;
    }
    
    if ((pre = findNode(ll, index - 1)) != NULL) {
        cur = pre->next;
        pre->next = malloc(sizeof(ListNode));
        pre->next->item = value;
        pre->next->next = cur;
        ll->size++;
        return 0;
    }
    return -1;
}

int removeNode(LinkedList *ll, int index) {
    ListNode *pre, *cur;
    if (ll == NULL || index < 0 || index >= ll->size)
        return -1;
    
    if (index == 0) {
        cur = ll->head->next;
        free(ll->head);
        ll->head = cur;
        ll->size--;
        return 0;
    }
    
    if ((pre = findNode(ll, index - 1)) != NULL) {
        if (pre->next == NULL)
            return -1;
        cur = pre->next;
        pre->next = cur->next;
        free(cur);
        ll->size--;
        return 0;
    }
    return -1;
}

int compareList(LinkedList *ll, int *expected, int expectedSize) {
    if (ll->size != expectedSize) return 0;
    ListNode *cur = ll->head;
    for (int i = 0; i < expectedSize; i++) {
        if (cur == NULL || cur->item != expected[i]) return 0;
        cur = cur->next;
    }
    return (cur == NULL);
}

//////////////////////////////////////////////////////////////////////////////////
// Stack Operations
//////////////////////////////////////////////////////////////////////////////////

void push(Stack *s, int item) {
    insertNode(&(s->ll), 0, item);
}

int pop(Stack *s) {
    int item;
    if (s->ll.head != NULL) {
        item = ((s->ll).head)->item;
        removeNode(&(s->ll), 0);
        return item;
    }
    return MIN_INT;
}

int peek(Stack *s) {
    if (s->ll.size == 0)
        return MIN_INT;
    return ((s->ll).head)->item;
}

int isEmptyStack(Stack *s) {
    return (s->ll.size == 0);
}

void removeAllItemsFromStack(Stack *s) {
    if (s == NULL)
        return;
    while (s->ll.head != NULL) {
        pop(s);
    }
}

//////////////////////////////////////////////////////////////////////////////////
// Queue Operations
//////////////////////////////////////////////////////////////////////////////////

void enqueue(Queue *q, int item) {
    insertNode(&(q->ll), q->ll.size, item);
}

int dequeue(Queue *q) {
    int item;
    if (q->ll.head != NULL) {
        item = ((q->ll).head)->item;
        removeNode(&(q->ll), 0);
        return item;
    }
    return -1;
}

int isEmptyQueue(Queue *q) {
    return (q->ll.size == 0);
}

void removeAllItemsFromQueue(Queue *q) {
    int count, i;
    if (q == NULL)
        return;
    count = q->ll.size;
    for (i = 0; i < count; i++)
        dequeue(q);
}

//////////////////////////////////////////////////////////////////////////////////
// Q1: createQueueFromLinkedList & removeOddValues
//////////////////////////////////////////////////////////////////////////////////

void createQueueFromLinkedList(LinkedList *ll, Queue *q)
{
	if(q != NULL){ 					// 큐에 남아있는게 있다면 비워준다.
		removeAllItemsFromQueue(q);
	}
	ListNode *cur = ll->head;
	while(cur != NULL)				// 큐에 리스트에 있는 요소 하나씩 넣는다.
	{
		enqueue(q, cur->item);
		cur = cur->next;
	}
}

void removeOddValues(Queue *q)
{
	if(q == NULL || q->ll.head == NULL){	// 유효성 검사
		return;
	}

	ListNode *cur = q->ll.head;
	ListNode *next_inspect = NULL;			// 다음 위치를 찾기위한 변수

	int cur_idx = 0;
	while(cur != NULL){
		next_inspect = cur->next; 			// romove전 다음 위치 기억
		if(cur->item % 2 == 1){
			removeNode(&(q->ll), cur_idx);
			cur_idx--;						// 제거하면 idx를 맞춰준다.
		}
		cur = next_inspect;
		cur_idx++;
	}
}

//////////////////////////////////////////////////////////////////////////////////
// Q2: createStackFromLinkedList & removeEvenValues
//////////////////////////////////////////////////////////////////////////////////

void createStackFromLinkedList(LinkedList *ll, Stack *s)
{
	if(ll == NULL || ll->head == NULL){		// 유효성 검사
		return;
	}
	ListNode *cur = ll->head;
	while(cur != NULL){						// 스택에 하나씩 push
		push(s, cur->item);
		cur = cur->next;
	}
}

void removeEvenValues(Stack *s)
{
	if(s == NULL || s->ll.head == NULL){	// 유효성 검사
		return;
	}
	
	int pop_item;
	Stack result;
	result.ll.head = NULL;
	result.ll.size = 0;

	while(!isEmptyStack(s)){				// pop을 한 값이 홀수라면 result스택에 다시 넣는다.
		pop_item = pop(s);
		if(pop_item % 2 == 1){
			push(&(result), pop_item);
		}
	}

	while (!isEmptyStack(&result)) {		// 다시 s스택에 넣어준다.
        pop_item = pop(&result);
        push(s, pop_item);
    }
}


//////////////////////////////////////////////////////////////////////////////////
// Q3: isStackPairwiseConsecutive
//////////////////////////////////////////////////////////////////////////////////

int isStackPairwiseConsecutive(Stack *s)
{
	if(s == NULL || s->ll.head == NULL){	// 유효성 검사
		return 1;
	}

  	ListNode *cur = s->ll.head;
	int item1;
	int item2;

	if(s->ll.size % 2 == 1){				// 스택에 있는 요소의 갯수가 홀수면 바로 리턴
		return 0;
	}
	
	while(!isEmptyStack(s)){				// 팝을 두번하고 그 값들의 차가 1이 안되면 리턴
		item1 = pop(s);
		item2 = pop(s);
		if(abs(item1 - item2) != 1){
			return 0;
		}
	}

	return 1;								// 위에 해당이 안될때 리턴1
}

//////////////////////////////////////////////////////////////////////////////////
// Q4: reverse (Queue using Stack)
//////////////////////////////////////////////////////////////////////////////////

void reverse(Queue *q)
{
	if(q == NULL || q->ll.head == NULL){	// 유효성 검사
		return;
	}

	ListNode *cur = q->ll.head;
	Stack tmp;
	tmp.ll.head = NULL;
	tmp.ll.size = 0;
	tmp.ll.tail = NULL;
	int item;

	while(!isEmptyQueue(q)){				// 큐에서 하나씩 빼서 스택에 넣고
		item = dequeue(q);
		push(&tmp, item);
	}

	while(!isEmptyStack(&tmp)){			// 스택에 있는걸 다시 큐로 보낸다.
		item = pop(&tmp);
		enqueue(q, item);
	}
}
//////////////////////////////////////////////////////////////////////////////////
// Q5: recursiveReverse (Queue)
//////////////////////////////////////////////////////////////////////////////////

void recursiveReverse(Queue *q)
{
	int tmp;
	if(q->ll.head == NULL || q == NULL) return; 	// 유효성 검사

	tmp = dequeue(q); 								// 맨 앞의 원소를 뽑아서 temp에 저장
	recursiveReverse(q); 							// 나머지 큐를 재귀적으로 뒤집음
	enqueue(q, tmp); 								// 뽑아뒀던 원소를 맨 뒤에 삽입
}

//////////////////////////////////////////////////////////////////////////////////
// Q6: removeUntil
//////////////////////////////////////////////////////////////////////////////////

void removeUntil(Stack *s, int value)
{
	if(s == NULL || s->ll.head == NULL) return;		// 유효성 검사

	Stack *tmp = malloc(sizeof(Stack));
	if (tmp == NULL) return;
	tmp->ll.head = NULL;
	tmp->ll.size = 0;

	while(!isEmptyStack(s)){						// peek을 이용해 찾던 값이 나올때까지 pop
		if(peek(s) != value){
			push((tmp), pop(s));
		}
		else{
			push((tmp), pop(s));
			break;
		}
	}

	s->ll = tmp->ll;

	free(tmp);
}
//////////////////////////////////////////////////////////////////////////////////
// Q7: balanced
//////////////////////////////////////////////////////////////////////////////////

int balanced(char *expression)
{
	Stack tmp;
	tmp.ll.head = NULL;
	tmp.ll.size = 0;
	char item;

	for (int i = 0; i < strlen(expression); i++){										// 스택에 (, [, { 가 들어오면 넣고 반대가 나오면 빼주면서 검사합니다.
		if(expression[i] == '(' || expression[i] == '[' || expression[i] == '{'){
			push(&tmp, expression[i]);
		}
		else if(expression[i] == ')' || expression[i] == ']' || expression[i] == '}'){

			if(isEmptyStack(&tmp)) return 1;

			item = pop(&tmp);
			if(expression[i] == ')' && item != '(') return 1;
			else if(expression[i] == ']' && item != '[') return 1;
			else if(expression[i] == '}' && item != '{') return 1;
		}
	}

	return isEmptyStack(&tmp) ? 0: 1;
}

//////////////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////////////

void test_removeOddValues() {
    printf("\n=== Testing Q1: removeOddValues ===\n");
    LinkedList ll;
    Queue q;
    
    // Test 1: Mixed odd and even
    initList(&ll);
    q.ll = ll;
    int input1[] = {1, 2, 3, 4, 5};
    for (int i = 0; i < 5; i++) enqueue(&q, input1[i]);
    removeOddValues(&q);
    int expected1[] = {2, 4};
    assert(compareList(&q.ll, expected1, 2));
    printf("✓ Test 1 passed: Remove odd values (1,2,3,4,5 -> 2,4)\n");
    removeAllItemsFromQueue(&q);
    
    // Test 2: All odd
    initList(&ll);
    q.ll = ll;
    int input2[] = {1, 3, 5, 7};
    for (int i = 0; i < 4; i++) enqueue(&q, input2[i]);
    removeOddValues(&q);
    assert(q.ll.size == 0);
    printf("✓ Test 2 passed: All odd values removed\n");
    
    // Test 3: All even
    initList(&ll);
    q.ll = ll;
    int input3[] = {2, 4, 6, 8};
    for (int i = 0; i < 4; i++) enqueue(&q, input3[i]);
    removeOddValues(&q);
    assert(compareList(&q.ll, input3, 4));
    printf("✓ Test 3 passed: No odd values to remove\n");
    removeAllItemsFromQueue(&q);
}

void test_removeEvenValues() {
    printf("\n=== Testing Q2: removeEvenValues ===\n");
    LinkedList ll;
    Stack s;
    
    // Test 1: Mixed odd and even
    initList(&ll);
    s.ll = ll;
    int input1[] = {1, 3, 5, 6, 7};
    for (int i = 0; i < 5; i++) push(&s, input1[i]);
    removeEvenValues(&s);
    int expected1[] = {7, 5, 3, 1};
    assert(compareList(&s.ll, expected1, 4));
    printf("✓ Test 1 passed: Remove even values\n");
    removeAllItemsFromStack(&s);
    
    // Test 2: All even
    initList(&ll);
    s.ll = ll;
    int input2[] = {2, 4, 6, 8};
    for (int i = 0; i < 4; i++) push(&s, input2[i]);
    removeEvenValues(&s);
    assert(s.ll.size == 0);
    printf("✓ Test 2 passed: All even values removed\n");
    
    // Test 3: All odd
    initList(&ll);
    s.ll = ll;
    int input3[] = {1, 3, 5, 7};
    for (int i = 0; i < 4; i++) push(&s, input3[i]);
    removeEvenValues(&s);
    int expected3[] = {7, 5, 3, 1};
    assert(compareList(&s.ll, expected3, 4));
    printf("✓ Test 3 passed: No even values to remove\n");
    removeAllItemsFromStack(&s);
}

void test_isStackPairwiseConsecutive() {
    printf("\n=== Testing Q3: isStackPairwiseConsecutive ===\n");
    Stack s;
    
    // Test 1: Pairwise consecutive
    initList(&s.ll);
    int input1[] = {16, 15, 11, 10, 5, 4};
    for (int i = 0; i < 6; i++) push(&s, input1[5-i]);
    assert(isStackPairwiseConsecutive(&s) == 1);
    printf("✓ Test 1 passed: Stack is pairwise consecutive\n");
    removeAllItemsFromStack(&s);
    
    // Test 2: Not pairwise consecutive
    initList(&s.ll);
    int input2[] = {16, 15, 11, 10, 5, 1};
    for (int i = 0; i < 6; i++) push(&s, input2[5-i]);
    assert(isStackPairwiseConsecutive(&s) == 0);
    printf("✓ Test 2 passed: Stack is not pairwise consecutive\n");
    removeAllItemsFromStack(&s);
    
    // Test 3: Odd number of elements (should be consecutive for pairs)
    initList(&s.ll);
    int input3[] = {16, 15, 11, 10, 5};
    for (int i = 0; i < 5; i++) push(&s, input3[4-i]);
    assert(isStackPairwiseConsecutive(&s) == 0);
    printf("✓ Test 3 passed: Odd elements, pairs are consecutive\n");
    removeAllItemsFromStack(&s);
    
    // Test 4: Empty stack
    initList(&s.ll);
    assert(isStackPairwiseConsecutive(&s) == 1);
    printf("✓ Test 4 passed: Empty stack\n");
}

void test_reverseQueue() {
    printf("\n=== Testing Q4: reverse (Queue) ===\n");
    Queue q;
    
    // Test 1: Multiple elements
    initList(&q.ll);
    int input1[] = {1, 2, 3, 4, 5};
    for (int i = 0; i < 5; i++) enqueue(&q, input1[i]);
    reverse(&q);
    int expected1[] = {5, 4, 3, 2, 1};
    assert(compareList(&q.ll, expected1, 5));
    printf("✓ Test 1 passed: Reverse 5 elements\n");
    removeAllItemsFromQueue(&q);
    
    // Test 2: Two elements
    initList(&q.ll);
    int input2[] = {10, 20};
    for (int i = 0; i < 2; i++) enqueue(&q, input2[i]);
    reverse(&q);
    int expected2[] = {20, 10};
    assert(compareList(&q.ll, expected2, 2));
    printf("✓ Test 2 passed: Reverse 2 elements\n");
    removeAllItemsFromQueue(&q);
    
    // Test 3: Single element
    initList(&q.ll);
    enqueue(&q, 42);
    reverse(&q);
    int expected3[] = {42};
    assert(compareList(&q.ll, expected3, 1));
    printf("✓ Test 3 passed: Single element (no change)\n");
    removeAllItemsFromQueue(&q);
}

void test_recursiveReverse() {
    printf("\n=== Testing Q5: recursiveReverse (Queue) ===\n");
    Queue q;
    
    // Test 1: Multiple elements
    initList(&q.ll);
    int input1[] = {1, 2, 3, 4, 5};
    for (int i = 0; i < 5; i++) enqueue(&q, input1[i]);
    recursiveReverse(&q);
    int expected1[] = {5, 4, 3, 2, 1};
    assert(compareList(&q.ll, expected1, 5));
    printf("✓ Test 1 passed: Recursive reverse 5 elements\n");
    removeAllItemsFromQueue(&q);
    
    // Test 2: Two elements
    initList(&q.ll);
    int input2[] = {10, 20};
    for (int i = 0; i < 2; i++) enqueue(&q, input2[i]);
    recursiveReverse(&q);
    int expected2[] = {20, 10};
    assert(compareList(&q.ll, expected2, 2));
    printf("✓ Test 2 passed: Recursive reverse 2 elements\n");
    removeAllItemsFromQueue(&q);
    
    // Test 3: Single element
    initList(&q.ll);
    enqueue(&q, 42);
    recursiveReverse(&q);
    int expected3[] = {42};
    assert(compareList(&q.ll, expected3, 1));
    printf("✓ Test 3 passed: Single element (no change)\n");
    removeAllItemsFromQueue(&q);
}

void test_removeUntil() {
    printf("\n=== Testing Q6: removeUntil ===\n");
    Stack s;
    
    // Test 1: Remove until middle value
    initList(&s.ll);
    int input1[] = {1, 2, 3, 4, 5, 6, 7};
    for (int i = 0; i < 7; i++) push(&s, input1[i]);
    removeUntil(&s, 4);
    int expected1[] = {4, 5, 6, 7};
    assert(compareList(&s.ll, expected1, 4));
    printf("✓ Test 1 passed: Remove until 4\n");
    removeAllItemsFromStack(&s);
    
    // Test 2: Remove 5
    initList(&s.ll);
    int input2[] = {1, 2, 3, 4, 5};
    for (int i = 0; i < 5; i++) push(&s, input2[i]);
    removeUntil(&s, 5);
    int expected2[] = {5};
    assert(compareList(&s.ll, expected2, 1));
    printf("✓ Test 2 passed: Remove until top value\n");
    removeAllItemsFromStack(&s);
    
    // Test 3: Value not in stack
    initList(&s.ll);
    int input3[] = {1, 2, 3};
    for (int i = 0; i < 3; i++) push(&s, input3[i]);
    removeUntil(&s, 99);
    int expected3[] = {1, 2, 3};
    assert(compareList(&s.ll, expected3, 3));
    printf("✓ Test 3 passed: Value not found, stack becomes original\n");
}

void test_balanced() {
    printf("\n=== Testing Q7: balanced ===\n");
    
    // Test 1: Balanced parentheses
    assert(balanced("()") == 0);
    printf("✓ Test 1 passed: () is balanced\n");
    
    // Test 2: Balanced mixed brackets
    assert(balanced("()[]{}") == 0);
    printf("✓ Test 2 passed: ()[]{}  is balanced\n");
    
    // Test 3: Balanced nested
    assert(balanced("([{}])") == 0);
    printf("✓ Test 3 passed: ([{}]) is balanced\n");
    
    // Test 4: Not balanced - missing closing
    assert(balanced("(") == 1);
    printf("✓ Test 4 passed: ( is not balanced\n");
    
    // Test 5: Not balanced - wrong order
    assert(balanced("(]") == 1);
    printf("✓ Test 5 passed: (] is not balanced\n");
    
    // Test 6: Not balanced - extra closing
    assert(balanced("())") == 1);
    printf("✓ Test 6 passed: ()) is not balanced\n");
    
    // Test 7: Complex balanced
    assert(balanced("{[()()]}") == 0);
    printf("✓ Test 7 passed: {[()()]} is balanced\n");
    
    // Test 8: Complex not balanced
    assert(balanced("{[(])}") == 1);
    printf("✓ Test 8 passed: {[(])} is not balanced\n");
}

//////////////////////////////////////////////////////////////////////////////////
// MAIN
//////////////////////////////////////////////////////////////////////////////////

int main() {
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║  Stack & Queue Test Suite - All 7 Questions           ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n");
    
    test_removeOddValues();
    test_removeEvenValues();
    test_isStackPairwiseConsecutive();
    test_reverseQueue();
    test_recursiveReverse();
    test_removeUntil();
    test_balanced();
    
    printf("\n╔════════════════════════════════════════════════════════╗\n");
    printf("║  ✅ ALL TESTS PASSED!                                  ║\n");
    printf("║  Total: 27 test cases across 7 functions              ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n"); 
    
    return 0;
}
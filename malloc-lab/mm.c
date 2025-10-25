/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 *
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/* Basic constants and macros */
#define WSIZE 4 /* Word and header/footer size (bytes) */
#define DSIZE 8 /* Double word size (bytes) */
#define CHUNKSIZE (1<<12) /* Extend heap by this amount (bytes) */

#define MAX(x, y) ((x) > (y)? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc, prev_alloc) ((size) | (alloc) | (prev_alloc << 1))
#undef GET_PREV_ALLOC
#define GET_PREV_ALLOC(p) (((GET(p)) >> 1) & 0x1)
#define SET_PREV_ALLOC(p) (PUT((p), GET(p) | 0x2))
#define CLEAR_PREV_ALLOC(p) (PUT((p), GET(p) & ~0x2))

/* Read and write a word at address p */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

/* Read/Write a pointer-sized value at address p */
#define GET_P(p) (*(unsigned long *)(p))
#define PUT_P(p, val) (*(unsigned long *)(p) = (val))

// 가용리스트 주소 확인 및 설정 (64-bit 호환 - 올바른 코드)
#define PRED_FREE(bp) ((void *)GET_P(bp))
#define SUCC_FREE(bp) ((void *)GET_P((char *)(bp) + DSIZE))
#define SET_PRED_FREE(bp, ptr) (PUT_P((bp), (unsigned long)(ptr)))
#define SET_SUCC_FREE(bp, ptr) (PUT_P(((char *)(bp) + DSIZE), (unsigned long)(ptr)))

static char *heap_listp = 0;
static char *free_list_head = NULL;

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/*
 * mm_init - initialize the malloc package.
 */

static void *extend_heap(size_t words) 
{
    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;

    int prev_alloc_bit = GET_PREV_ALLOC(HDRP(bp));
    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0, prev_alloc_bit)); /* Free block header */
    PUT(FTRP(bp), PACK(size, 0, prev_alloc_bit)); /* Free block footer */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1, 0)); /* New epilogue header */

    /* Coalesce if the previous block was free */
    return coalesce(bp);
}

int mm_init(void)
{
    if(((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)) return -1;

    // 패딩
    PUT(heap_listp, 0);
    // 프롤로그 헤더
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1, 1));
    // 프롤로그 풋터
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1, 1));
    // 에필로그 헤더
    PUT(heap_listp + (3*WSIZE), PACK(0, 1, 1));

    heap_listp += (2*WSIZE);

    free_list_head = NULL;

    if(extend_heap(CHUNKSIZE/WSIZE) == NULL) return -1;

    return 0;
}

static void *find_fit(size_t asize)
{
    /* First-fit search */
    void *bp;

    for (bp = free_list_head; bp != NULL; bp = SUCC_FREE(bp)) {
        if (asize <= GET_SIZE(HDRP(bp))) {
            return bp;
        }
    }
    return NULL; /* No fit */
}

static void place(void *bp, size_t asize)
{

    remove_free_block(bp);

    size_t csize = GET_SIZE(HDRP(bp));

    if ((csize - asize) >= (3 * DSIZE)) {
        PUT(HDRP(bp), PACK(asize, 1, GET_PREV_ALLOC(HDRP(bp))));
        void *next_bp = NEXT_BLKP(bp);
        PUT(HDRP(next_bp), PACK(csize - asize, 0, 1));
        PUT(FTRP(next_bp), PACK(csize - asize, 0, 1));
        insert_free_block(next_bp);
    } 
    else {
        PUT(HDRP(bp), PACK(csize, 1, GET_PREV_ALLOC(HDRP(bp))));
        SET_PREV_ALLOC(HDRP(NEXT_BLKP(bp)));
        if(!GET_ALLOC(HDRP(NEXT_BLKP(bp)))) SET_PREV_ALLOC(FTRP(NEXT_BLKP(bp)));
    }
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize;      /* Adjusted block size */
    size_t extendsize; /* Amount to extend heap if no fit */
    char *bp;

    /* Ignore spurious requests */
    if (size == 0)
        return NULL;

    /* Adjust block size to include overhead and alignment reqs. */
    if (size <= DSIZE)
        asize = 3 * DSIZE;
    else
        asize = ALIGN(size + DSIZE);

    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize / WSIZE)) == NULL)
        return NULL;
    place(bp, asize);
    return bp;
}

static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_PREV_ALLOC(HDRP(bp));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    size_t prev_prev_alloc;

    if (prev_alloc && !next_alloc) {
        remove_free_block(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0, 1));
        PUT(FTRP(bp), PACK(size, 0, 1));
    }

    else if (!prev_alloc && next_alloc) {
        remove_free_block(PREV_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        bp = PREV_BLKP(bp);
        prev_prev_alloc = GET_PREV_ALLOC(HDRP(bp));
        PUT(HDRP(bp), PACK(size, 0, prev_prev_alloc));
        PUT(FTRP(bp), PACK(size, 0, prev_prev_alloc));
    }

    else if (!prev_alloc && !next_alloc){
        remove_free_block(NEXT_BLKP(bp));
        remove_free_block(PREV_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp)));
        bp = PREV_BLKP(bp);
        prev_prev_alloc = GET_PREV_ALLOC(HDRP(bp));
        PUT(HDRP(bp), PACK(size, 0, prev_prev_alloc));
        PUT(FTRP(bp), PACK(size, 0, prev_prev_alloc));
    }

    insert_free_block(bp);
    CLEAR_PREV_ALLOC(HDRP(NEXT_BLKP(bp)));
    if(!GET_ALLOC(HDRP(NEXT_BLKP(bp)))) CLEAR_PREV_ALLOC(FTRP(NEXT_BLKP(bp)));

    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(size, 0, GET_PREV_ALLOC(HDRP(bp))));
    PUT(FTRP(bp), PACK(size, 0, GET_PREV_ALLOC(HDRP(bp))));
    coalesce(bp);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *newptr;
    size_t copySize;

    // ptr이 NULL이면 mm_malloc(size)와 동일하게 동작합니다.
    if (ptr == NULL) {
        return mm_malloc(size);
    }
    
    // size가 0이면 mm_free(ptr)와 동일하게 동작합니다.
    if (size == 0) {
        mm_free(ptr);
        return NULL;
    }

    newptr = mm_malloc(size);
    if (newptr == NULL)
        return NULL;

    copySize = GET_SIZE(HDRP(ptr)) - WSIZE;

    if (size < copySize)
        copySize = size;

    memcpy(newptr, ptr, copySize);
    mm_free(ptr);
    return newptr;
}

// 1. **가용 리스트에 블록을 '삽입'하는 함수** (예: `insert_free_block(void *bp)`)
//     - **역할:** LIFO 정책에 따라 **새로운 가용 블록 `bp`를 리스트의 맨 앞에 추가**합니다.
//     - **동작:**
//         1. 새 블록(`bp`)의 '다음'을 현재의 `free_list_head`로 설정합니다.
//         2. 만약 리스트가 비어있지 않았다면, 기존의 첫 번째 블록의 '이전'을 새 블록(`bp`)으로 설정합니다.
//         3. `free_list_head`를 새 블록(`bp`)으로 업데이트합니다.
static void insert_free_block(void *bp){

    SET_PRED_FREE(bp, NULL);

    if(free_list_head != NULL){
        SET_SUCC_FREE(bp, free_list_head);
        SET_PRED_FREE(free_list_head, bp);
    } else{
        SET_SUCC_FREE(bp, NULL);
    }

    free_list_head = bp;

}

// 2. **가용 리스트에서 블록을 '제거'하는 함수** (예: `remove_free_block(void *bp)`)
//     - **역할:** 가용 리스트 중간에 있는 블록 `bp`를 **연결 리스트에서 안전하게 제거**합니다.
//     - **동작:** `bp`의 이전 블록과 다음 블록을 서로 직접 연결시켜, `bp`가 리스트에서 빠지도록 포인터를 조작합니다. (리스트의 맨 앞이나 맨 뒤일 경우도 고려해야 합니다.)
static void remove_free_block(void *bp){

    void *prev_ptr = PRED_FREE(bp);
    void *succ_ptr = SUCC_FREE(bp);

    if(prev_ptr == NULL && succ_ptr == NULL){
        free_list_head = NULL;
    }
    else if(prev_ptr == NULL) {
        SET_PRED_FREE(succ_ptr, NULL);
        free_list_head = succ_ptr;
    }
    else if(succ_ptr == NULL) {
        SET_SUCC_FREE(prev_ptr, NULL);
    }
    else{
        SET_PRED_FREE(succ_ptr, prev_ptr);
        SET_SUCC_FREE(prev_ptr, succ_ptr);
    }
}
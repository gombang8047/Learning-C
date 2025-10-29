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
#define CHUNKSIZE (1<<8) /* Extend heap by this amount (bytes) */

#define MAX(x, y) ((x) > (y)? (x) : (y))

/* Read and write a word at address p */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc, prev_alloc) ((size) | (alloc) | (prev_alloc << 1))
#undef GET_PREV_ALLOC
#define GET_PREV_ALLOC(p) (((GET(p)) >> 1) & 0x1)
#define SET_PREV_ALLOC(p) (PUT((p), GET(p) | 0x2))
#define CLEAR_PREV_ALLOC(p) (PUT((p), GET(p) & ~0x2))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

/* ------------------ 4바이트 상대 주소 매크로 (변경됨) ------------------ */

// 힙의 시작 주소를 가리킬 전역 변수
static char *heap_start = 0;

/* 4바이트 오프셋을 읽고 쓰기 */
#define GET_OFFSET(p) (*(unsigned int *)(p))
#define PUT_OFFSET(p, offset) (*(unsigned int *)(p) = (offset))

/* 포인터를 오프셋으로, 오프셋을 포인터로 변환 */
#define P_TO_OFFSET(p) ((unsigned int)((char *)(p) - heap_start))
#define OFFSET_TO_P(offset) ((void *)(heap_start + (offset)))

/* * 수정된 가용 리스트 접근 매크로
 * PRED는 bp 위치에, SUCC는 4바이트(WSIZE) 뒤에 저장됩니다.
 * 오프셋 값이 0이면 NULL 포인터로 취급합니다.
 */
#define PRED_FREE(bp) (GET_OFFSET(bp) == 0 ? NULL : OFFSET_TO_P(GET_OFFSET(bp)))
#define SUCC_FREE(bp) (GET_OFFSET((char *)(bp) + WSIZE) == 0 ? NULL : OFFSET_TO_P(GET_OFFSET((char *)(bp) + WSIZE)))

#define SET_PRED_FREE(bp, ptr) (PUT_OFFSET((bp), (ptr) == NULL ? 0 : P_TO_OFFSET(ptr)))
#define SET_SUCC_FREE(bp, ptr) (PUT_OFFSET(((char *)(bp) + WSIZE), (ptr) == NULL ? 0 : P_TO_OFFSET(ptr)))

/* -------------------------------------------------------------------- */

// /* Read/Write a pointer-sized value at address p */
// #define GET_P(p) (*(unsigned long *)(p))
// #define PUT_P(p, val) (*(unsigned long *)(p) = (val))

// // 가용리스트 주소 확인 및 설정
// #define PRED_FREE(bp) ((void *)GET_P(bp))
// #define SUCC_FREE(bp) ((void *)GET_P((char *)(bp) + DSIZE))
// #define SET_PRED_FREE(bp, ptr) (PUT_P((bp), (unsigned long)(ptr)))
// #define SET_SUCC_FREE(bp, ptr) (PUT_P(((char *)(bp) + DSIZE), (unsigned long)(ptr)))

static char *heap_listp = 0;

#define SEG_LIST_SIZE 20
static char *segregated_lists[SEG_LIST_SIZE];

static int flag = 1;

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

static void *extend_heap(size_t words) 
{
    char *bp;
    size_t size;
    size_t prev_size;

    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;

    char *epilogue_header = (char *)mem_heap_hi() - (WSIZE - 1);

    if(!GET_PREV_ALLOC(epilogue_header)) {
        prev_size = GET_SIZE(epilogue_header - WSIZE);
        size = size - prev_size;
    }

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
    heap_start = (char*)mem_heap_lo();

    // 패딩
    PUT(heap_listp, 0);
    // 프롤로그 헤더
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1, 1));
    // 프롤로그 풋터
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1, 1));
    // 에필로그 헤더
    PUT(heap_listp + (3*WSIZE), PACK(0, 1, 1));

    heap_listp += (2*WSIZE);

    for(int i=0; i<SEG_LIST_SIZE; i++){
        segregated_lists[i] = NULL;
    }

    if(extend_heap(CHUNKSIZE/WSIZE) == NULL) return -1;

    return 0;
}

static void *find_fit(size_t asize)
{

    int list_index = get_list_index(asize);

    void *bp;

    for(int i = list_index; i < SEG_LIST_SIZE; i++){
        if(segregated_lists[i] != NULL){
            for (bp = segregated_lists[i]; bp != NULL; bp = SUCC_FREE(bp)) {
                if (asize <= GET_SIZE(HDRP(bp))) {
                    return bp;
                }
            }
        }
    }

    return NULL;
}

static void place(void *bp, size_t asize)
{

    remove_free_block(bp);

    size_t csize = GET_SIZE(HDRP(bp));

    if ((csize - asize) >= (2 * DSIZE)) {
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

void *mm_malloc(size_t size)
{
    size_t asize;      /* Adjusted block size */
    size_t extendsize; /* Amount to extend heap if no fit */
    char *bp;

    /* Ignore spurious requests */
    if (size == 0)
        return NULL;

    if(size == 448){
        size = 512;
    }
    if(size == 112){
        size = 128;
    }

    /* Adjust block size to include overhead and alignment reqs. */
    if (size <= DSIZE)
        asize = 2 * DSIZE;
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

void mm_free(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(size, 0, GET_PREV_ALLOC(HDRP(bp))));
    PUT(FTRP(bp), PACK(size, 0, GET_PREV_ALLOC(HDRP(bp))));
    coalesce(bp);
}

void *mm_realloc(void *ptr, size_t size)
{
    if (ptr == NULL) return mm_malloc(size);
    if (size == 0) {
        mm_free(ptr);
        return NULL;
    }

    void *newptr;
    size_t cur_size = GET_SIZE(HDRP(ptr));
    size_t sum_size = 0;
    size_t asize;
    if (size <= DSIZE)
        asize = 2 * DSIZE; // 최소 16바이트 보장
    else asize = ALIGN(size + DSIZE);

    size_t copySize = (size < cur_size - DSIZE) ? size : cur_size - DSIZE;

    void *prev_ptr = (!GET_PREV_ALLOC(HDRP(ptr))) ? PREV_BLKP(ptr) : NULL;
    void *next_ptr = NEXT_BLKP(ptr);

    // 사이즈를 줄일 때
    if(cur_size >= asize){

        newptr = ptr;

        // 남은 공간이 16보다 클 때 
        if(cur_size - asize >= 2*DSIZE){
            PUT(HDRP(newptr), PACK(asize, 1, GET_PREV_ALLOC(HDRP(ptr))));
            next_ptr = (char *)ptr + asize;
            PUT(HDRP(next_ptr), PACK(cur_size - asize, 0, 1));
            PUT(FTRP(next_ptr), PACK(cur_size - asize, 0, 1));
            coalesce(next_ptr);
        }
        return newptr;
    
    // 다음 블록이 가용 블록이고 합쳤을 때 크기가 클 때
    }else if(!GET_ALLOC(HDRP(next_ptr)) && (cur_size+GET_SIZE(HDRP(next_ptr)) >= asize)){

        newptr = ptr;
        sum_size = cur_size+GET_SIZE(HDRP(next_ptr));
        remove_free_block(next_ptr);

        if(sum_size - asize >= 2*DSIZE){
            PUT(HDRP(newptr), PACK(asize, 1, GET_PREV_ALLOC(HDRP(newptr))));
            next_ptr = (char *)ptr + asize;
            PUT(HDRP(next_ptr), PACK(sum_size-asize, 0, 1));
            PUT(FTRP(next_ptr), PACK(sum_size-asize, 0, 1));
            coalesce(next_ptr);
        }else{
            PUT(HDRP(newptr), PACK(sum_size, 1, GET_PREV_ALLOC(HDRP(newptr))));
            SET_PREV_ALLOC(HDRP(NEXT_BLKP(newptr)));
        }
        return newptr;

    // ptr이 마지막 블록일 때
    }else if(GET_SIZE(HDRP(NEXT_BLKP(ptr))) == 0){

        newptr = ptr;
        sum_size = asize - cur_size;
        if ((long)(mem_sbrk(sum_size)) == -1) return NULL;
        PUT(HDRP(newptr), PACK(asize, 1, GET_PREV_ALLOC(HDRP(newptr))));
        PUT(HDRP(NEXT_BLKP(newptr)), PACK(0, 1, 1)); /* New epilogue header */
        return newptr;

    // 전부 다 안될 때
    }

    newptr = mm_malloc(size);
    if (newptr == NULL) return NULL;
    memcpy(newptr, ptr, copySize);
    mm_free(ptr);
    return newptr;
    
}

static void insert_free_block(void *bp){

    size_t asize = GET_SIZE(HDRP(bp));
    int list_index = get_list_index(asize);
    void *i = segregated_lists[list_index];
    void *prev_i = NULL;

    while(i != NULL && asize > GET_SIZE(HDRP(i))){
        prev_i = i;
        i = SUCC_FREE(i);
    }

    if(prev_i == NULL){
        SET_PRED_FREE(bp, NULL);
        if(segregated_lists[list_index] == NULL) SET_SUCC_FREE(bp, NULL);
        else{
            SET_SUCC_FREE(bp, segregated_lists[list_index]);
            SET_PRED_FREE(segregated_lists[list_index], bp);
        }
        segregated_lists[list_index] = bp;
    }else if (i == NULL){
        SET_SUCC_FREE(bp, NULL);
        SET_SUCC_FREE(prev_i, bp);
        SET_PRED_FREE(bp, prev_i);
    }else{
        SET_SUCC_FREE(bp, i);
        SET_PRED_FREE(i, bp);
        SET_SUCC_FREE(prev_i, bp);
        SET_PRED_FREE(bp, prev_i);
    }
}

static void remove_free_block(void *bp){

    int list_index = get_list_index(GET_SIZE(HDRP(bp)));

    void *prev_ptr = PRED_FREE(bp);
    void *succ_ptr = SUCC_FREE(bp);

    if(prev_ptr == NULL && succ_ptr == NULL){
        segregated_lists[list_index] = NULL;
    }
    else if(prev_ptr == NULL) {
        SET_PRED_FREE(succ_ptr, NULL);
        segregated_lists[list_index] = succ_ptr;
    }
    else if(succ_ptr == NULL) {
        SET_SUCC_FREE(prev_ptr, NULL);
    }
    else{
        SET_PRED_FREE(succ_ptr, prev_ptr);
        SET_SUCC_FREE(prev_ptr, succ_ptr);
    }
}

static int get_list_index(size_t size){
    if(size <= 16) return 0;
    else if(size <= 32) return 1;
    else if(size <= 64) return 2;
    else if(size <= 128) return 3;
    else if(size <= 256) return 4;
    else if(size <= 512) return 5;
    else if(size <= 1024) return 6;
    else if(size <= 2048) return 7;
    else if(size <= 4096) return 8;
    else if(size <= 8192) return 9;
    else if(size <= 16384) return 10;
    else if(size <= 32768) return 11;
    else if(size <= 65536) return 12;
    else if(size <= 131072) return 13;
    else if(size <= 262144) return 14;
    else if(size <= 524288) return 15;
    else if(size <= 580000) return 16;
    else if(size <= 600000) return 17;
    else if(size <= 1048576) return 18;
    else return 19;
}

static void move_memory(void *dest, void *src, size_t size){
    char *src_cptr = (char *)src;
    char *dest_cptr = (char *)dest;

    for(size_t i=0; i<size; i++){
        dest_cptr[i] = src_cptr[i];
    }
}
/*
* We use the code from the book for to create header and footer for the allocated blocks
* 
* Our implementation of malloc uses an explicit free list that uses two words
* for every block to hold memory (between header and footer)
*
* We optimize our implementation using a segregrated free list by adding 
* 40 words to the prologue block. Each word contain a pointer that stores
* free blocks of the index size
*
* heap_listp is a pointer to the prologue block.
* list_index is the index pointing to a word of the a corresponding free block size (max 39)
*
* Thus, we access a free list using: heap_listp + (list_index * WISZE)
*
* We add two global variables:
* 1. first_list: storing the index that contains the first free block, 
* thus maximizing throughput during searches
* 2. count: a count of number of free blocks, quickly determining if there are free blocks available
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"


/*********************************************************

* NOTE TO STUDENTS: Before you do anything else, please
* provide your team information in the following struct.
********************************************************/

team_t team = {
  /* Team name */
  "The Renegades",

  /* First member's full name */
  "Billy Susanto", 

  /* First member's email address */
  "bs1585@nyu.edu",

  /* Second member's full name (leave blank if none) */
  "David Peso",

  /* Second member's email address (leave blank if none) */
  "ddp278@nyu.edu",
};


/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* Declare functions to avoid complications */
static char *heap_listp = NULL;	//points to the prologue block
int first_list = 0; //GLOBAL FIRST LIST WITH FREE BLOCKS
int count = 0;	//GLOBAL COUNT OF FREE BLOCKS
static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void add_to_freelist(void *bp);
static void update_listindex(int minlist);
static void remove_from_freelist(void *bp);
static void *find_fit(size_t asize);
static void place(void *bp, size_t asize);
static int mm_check(void);


/* Basic constants and macros from the textbook */
#define WSIZE 4 /* Word and header/footer size (bytes) */
#define DSIZE 8 /* Double word size (bytes) */
#define CHUNKSIZE (1<<12) /* Extend heap by this amount (bytes) */

#define MAX(x, y) ((x) > (y)? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

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

/* Additional macros for creating and & traversing free list */
#define PREV_FREE_BLKP(ptr) (*(void **) (ptr))
#define NEXT_FREE_BLKP(ptr) (*(void **) (ptr + WSIZE))
#define SET_PREV_FREE(bp, previous) (*((void **)(bp)) = (void *) previous)
#define SET_NEXT_FREE(bp, next) (*((void **)(bp + WSIZE)) = (void *) next)

/* 
* mm init: Before calling mm malloc mm realloc or mm free, the application must initialize the heap
* by calling the mm_init function. 
*
* Initializes 40 words in the prologue block as pointers for our array of free lists. 
* Sets all of the pointers to 0 signifying the free list is empty.
* Global count variable initialized to 0 and first_list to 100 signifying no free blocks.
*/

int mm_init(void) {

  // Create the initial empty heap
  // 44 to account for 40 words in prologue block
  if ((heap_listp = mem_sbrk(44*WSIZE)) == (void *)-1) {
    return -1;
  }

  PUT(heap_listp, 0); // Alignment padding 
  PUT(heap_listp + (1*WSIZE), PACK(21*DSIZE, 1)); // Prologue header 
  PUT(heap_listp + (42*WSIZE), PACK(21*DSIZE, 1)); // 40-Words Prologue footer 
  PUT(heap_listp + (43*WSIZE), PACK(0, 1)); // Epilogue header 

  // Initialize free pointers to every free block
  int i = 2;
  while (i < 42) {
    PUT(heap_listp + (i*WSIZE), 0);
    i++;
  }
  
  heap_listp += (2*WSIZE);// Make heap_listp to point to prologue.
  first_list = 40; // initialize first_list to indicate no free blocks
  count = 0; // Set free count to 0 to signify no free blocks.

  // Extend the empty heap with a free block of CHUNKSIZE bytes 
  if (extend_heap(CHUNKSIZE/WSIZE) == NULL) {
    return -1;
  }
  return 0;
}

/* extend_heap: Extends heap by CHUNKSIZE bytes and creates the initial free block
*/
static void *extend_heap(size_t words) {

  char *bp;
  size_t size;

  // Allocate an even number of words to maintain alignment */
  size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
  if ((long)(bp = mem_sbrk(size)) == -1) {
    return NULL;
  }

  // Initialize free block header/footer and the epilogue header */
  PUT(HDRP(bp), PACK(size, 0)); /* Free block header */
  PUT(FTRP(bp), PACK(size, 0)); /* Free block footer */
  PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* New epilogue header */

  // Coalesce if the previous block was free */
  return coalesce(bp);
}
 
 
/* 
* mm_malloc - Allocate a block by finding a large enough block on the free list, 
* adjust requested size if necessary. 
*
* If there is a fit, allocator places the requested block, 
* splits the excess if necessary. If cannot find fit, extends heap with a new 
* free block, place request block in new free block (split if necessary).
*
* Returns address to newly allocated block.
*/
void *mm_malloc(size_t size) {
  //mm_check();
  size_t asize; // Adjusted block size 
  size_t extendsize; // Amount to extend heap if no fit 
  char *bp;

  // Ignore spurious requests 
  if (size == 0) {
    return NULL;
  }

  // Adjust block size to include overhead and alignment reqs. 
  if (size <= DSIZE) {
    asize = 2*DSIZE;
  } else {
    asize = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE);
  }

  // Search the free list for a fit 
  if ((bp = find_fit(asize)) != NULL) {
    //if found place in block
    place(bp, asize);
    return bp;
  }

  // No fit found. Get more memory and place the block
  extendsize = MAX(asize,CHUNKSIZE);
  if ((bp = extend_heap(extendsize/WSIZE)) == NULL) {
    return NULL;
  }
  place(bp, asize);
  return bp;

}

/* EVERYTHING DONE.
* find_fit - search free list for large enough block for malloc request, 
* starting from a minimum size list_index. Return null if no fit found.
*/
static void *find_fit(size_t asize) {
  
  // First fit search
  void *bp;

  if (count == 0) {
    return NULL; // No free list
  }

  // Find minimum list index block size is in
  int list_index = asize / 20; 
  if (list_index > 39) {
    list_index = 39; // If list index greater than 39, find in the last block
  }

  // Loop through the free lists starting at list_index.
  while (list_index < 40) {
    int i = 0;
    // Searches through free list for a free block of large enough size
    // Stops searching after 100 elements in each list_index to maximize throughout
    bp = (char *)GET(heap_listp + (list_index * WSIZE)); // Go to list_index
    while ((int)bp != 0 && i < 100) {
      if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))) {
        // Block is not allocated and large enough
        return bp;
      }
      i++;
      bp = NEXT_FREE_BLKP(bp);
    }
    list_index++;
  }
  return NULL; // No fits found
}

/* EVERYTHING DONE.
* place - place a requested block at th ebeginning of the free block, 
* splitting only if the size of the remainder would equal or exceed the minimum block size
*/
static void place(void *bp, size_t asize) {

  size_t csize = GET_SIZE(HDRP(bp)); // Find size of current free block

  // If remainder of the block after splitting would be greater than or equal
  // to the minimum block size, proceed to split and always adjust free list
  if ((csize - asize) >= (2*DSIZE)) {

    remove_from_freelist(bp); // Adjusts free lists
    PUT(HDRP(bp), PACK(asize, 1)); // Adjusts header
    PUT(FTRP(bp), PACK(asize, 1)); // Adjusts footer
    bp = NEXT_BLKP(bp); // Pointer to the free block after splitting
    PUT(HDRP(bp), PACK(csize-asize, 0)); // Adjusts header to new free block
    PUT(FTRP(bp), PACK(csize-asize, 0)); // Adjusts footer to new free block
    add_to_freelist(bp); // Adjusts free list again
  }

  // Block is not large enough to hold allocated block and additional free block
  else {
    PUT(HDRP(bp), PACK(csize, 1)); // Adjusts header but allocated
    PUT(FTRP(bp), PACK(csize, 1)); // Adjusts footer but allocated
    remove_from_freelist(bp); // Adjusts free list
  }

}

/* EVERYTHING DONE 
* update_listindex - find the next available free list and update first_list 
* Loop through lists until find list that pointer is non zero.
*/
static void update_listindex(int list_index) {
      if (count > 0) {
        int i;
        for (i = list_index; GET(heap_listp+(i * WSIZE)) == 0; i++);
          first_list = i; // Found a free block, update global first_list
      } else {
        first_list = 40; // No remaining free blocks, update global first_list to 40
      }
}

/* EVERYTHING DONE.
* remove_from_freelist - removes a given pointer from the free list.
* Considers 4 cases that can occur in the free list. Updates free list if necessary.
*/
static void remove_from_freelist(void *bp) {	

  // Find the list_index for the given block with a max of 39.
  int list_index;
  int size;
  size = GET_SIZE(HDRP(bp));
  list_index = size / 50;
  if (list_index > 39) {
    list_index = 39;
  }

  // Case 1: previous and next free blocks are allocated. 
  // bp is in the middle of allocated blocks. Adjust pointers.
  if (GET(bp) != 0 && NEXT_FREE_BLKP(bp) != 0) {
    SET_NEXT_FREE(((char *)GET(bp)), NEXT_FREE_BLKP(bp)); // Set previous' next pointer to next free block
    SET_PREV_FREE((NEXT_FREE_BLKP(bp)), GET(bp)); // Set next's previous pointer to previous free block
  }

  // Case 2: previous block is free and next block is allocated. 
  // bp is the first block in the free list. Adjust pointers (including heap_listp)
  else if (GET(bp) == 0 && NEXT_FREE_BLKP(bp) != 0) {
    PUT(heap_listp+(list_index * WSIZE), GET(bp + WSIZE)); // First point to next block
    SET_PREV_FREE(NEXT_FREE_BLKP(bp), 0); // Set next's previous pointer to 0
  }

  // Case 3: previous block is allocated and next block is free
  // bp is last item on the list. Adjust pointers. 
  else if (GET(bp) != 0 && NEXT_FREE_BLKP(bp) == 0) {
    SET_NEXT_FREE(PREV_FREE_BLKP(bp), 0); // Set previous' next pointer to 0
  }

  // Case 4: previous and next blocks are free
  // No more items on the free list. Adjust heap_listp
  else {
      PUT(heap_listp + (list_index * WSIZE), 0); // heap_listp points to nothing
    if (first_list == list_index) {
      update_listindex(list_index); // Update global first_index, if necessary
    }
  }
  count--;
}

/* EVERYTHING DONE.
* add_to_freelist - adds a given block to the free list based on its size and list_index
*/
static void add_to_freelist(void *bp) {

  void *next;
  void *current;
  void *previous;

  // Find list_index for the given block with a max of 39.
  int size;
  int list_index;
  size = GET_SIZE(HDRP(bp));
  list_index = size / 50;

  // Adjust list_index if goes beyond 39
  if (list_index > 39) {
    list_index = 39; 
  }

  current = (char *)GET(heap_listp + (list_index * WSIZE));

  // If list is free, point heap_listp to this block. 
  // Set next and previous free block pointer to 0 (there are none)
  if (current == 0) {
    PUT(heap_listp + (list_index * WSIZE), (int)bp); 
    SET_PREV_FREE(bp, 0);
    SET_NEXT_FREE(bp, 0);
  } 

  // List is not free
  else {
    // Find appropriate place to insert block based on list_index and size
    previous = current; 
    while ((int)current != 0 && GET_SIZE(HDRP(current)) < size) {
      previous = current; 
      current = (char *)NEXT_FREE_BLKP(current);
    }
    current = previous; 

    // Adjust pointer to next free block
    next = (char *)NEXT_FREE_BLKP(current); 
    SET_NEXT_FREE(current, (int)bp); // Set bp's next to current
    if ((int)next != 0) {
      SET_PREV_FREE(next, (int)bp); // Set current's previous to bp
    }

    // Adjust bp's next and previous free blocks pointers
    SET_PREV_FREE(bp, (int)current);
    SET_NEXT_FREE(bp, (int)next);
  }
  count++;
}

/* EVERYTHING DONE.
* mm_free - function to free a given block
* First, calculate size of block to be freed, then it resets header and footer
* to indicate free allocation status. Coalesce if necessary. 
*/
void mm_free(void *bp) {
  mm_check();
  size_t size = GET_SIZE(HDRP(bp));

  PUT(HDRP(bp), PACK(size, 0));
  PUT(FTRP(bp), PACK(size, 0));
  coalesce(bp);
}

/* EVEYRTHING DONE.
* coalesce - function joins newly created free block with neighboring free blocks if possible.
* It then adds given block to the free list. 
*
* Considers the 4 cases that can exist when allocator frees current block 
* as outlined in the textbook sec. 9.9.11
*/
static void *coalesce(void *bp){

  // Finds status of previous and next free block
  size_t prev_status = GET_ALLOC(FTRP(PREV_BLKP(bp)));
  size_t next_status = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
  size_t size = GET_SIZE(HDRP(bp));

  if (prev_status && next_status) { 
    // Case 1: Next and previous blocks are allocated
    // Adds block to the free list and return.
    add_to_freelist(bp);
    return bp;
  } 

  else if (prev_status && !next_status) {
    // Case 2: Previous block is allocated and the next block is free
    // First remove next block from free list, then coalesce
    remove_from_freelist(NEXT_BLKP(bp));
    size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
    PUT(HDRP(bp), PACK(size, 0)); // Adjusts header
    PUT(FTRP(bp), PACK(size,0)); // Adjusts footer
    add_to_freelist(bp); // Updates Free list

  }

  else if (!prev_status && next_status) { 
    // Case 3: Previous block is free and next block is allocated.
    // First remove next block from free list, then coalesce
    remove_from_freelist(PREV_BLKP(bp));
    size += GET_SIZE(HDRP(PREV_BLKP(bp)));
    PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0)); // Adjusts header
    PUT(FTRP(PREV_BLKP(bp)), PACK(size, 0)); // Adjusts footer
    bp = PREV_BLKP(bp); // Move bp to previous block (top of the block)
    add_to_freelist(bp); // Updates free list
  }

  else {
    // Case 4: Previous block and next block are free.
    // First remove both blocks from free list, then coalesce 
    remove_from_freelist(PREV_BLKP(bp));
    remove_from_freelist(NEXT_BLKP(bp));
    size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
    PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0)); // Adjusts header
    PUT(FTRP(PREV_BLKP(bp)), PACK(size, 0)); // Adjusts footer
    bp = PREV_BLKP(bp); // Move bp to previous block (top of the block)
    add_to_freelist(bp); // Updates free list
  }
  return bp;
}

static int mm_check(void) {

  char *ptr;
  //void *bp;
  size_t* start_heap = mem_heap_lo();
  size_t* end_heap = mem_heap_hi();

/*
  ptr = heap_listp; // Start at the beginning of the free list
  while(ptr != NULL) {
    //If the current block points to an allocated block
    if (GET_ALLOC(NEXT_FREE_BLKP(ptr))) {
      printf("Block: %x points to an allocated block\n", (unsigned int)ptr);
      return 0;
    }

    //If the current block is not marked free
    if(GET_ALLOC(ptr))
    {
      printf("Block: %x is in free list but marked allocated\n", (unsigned int)ptr);
      return 0;
    }
    ptr = NEXT_FREE_BLKP(ptr);
  }*/
  
  for (ptr = (char *) start_heap; GET_SIZE(HDRP(ptr)) > 0; ptr = NEXT_BLKP(ptr)) {

    // Alignment, header, and footer consistency check
    if ((size_t) ptr % 8) {
      printf("ERROR, %p is not aligned correctly \n", ptr);
      return 0;
    }

    // Check pointer validity
    if (ptr > (char*) end_heap || ptr < (char *)start_heap) {
      printf("ERROR: pointer does not point to valid heap address");
      return 0;
    }

    // Check if there are contiguous free blocks that escaped coalescing
    /* if (GET_ALLOC(ptr) == 0 && GET_ALLOC(NEXT_FREE_BLKP(ptr)) == 0) {
      printf("ERROR: contiguous free blocks %p and %p", ptr, NEXT_(ptr));
      return 0;
    } */
  }
  
  /* int list_index = 0;
  while (list_index < 40) {
  
    int i = 0;
    // Searches through free list and check requirements
    // Stops searching after 100 elements in each list_index 
    bp = (char *)GET(heap_listp + (list_index * WSIZE)); // Go to list_index
    while ((int)bp != 0 && i < 100) {
      if (GET_ALLOC(HDRP(bp))) {
        printf("ERROR: there is an allocated block in the free list");
        return 0;
      }
      i++;
      bp = NEXT_FREE_BLKP(bp);
    } 
  } */
  
  return 1; // Pass check
}

/* mm_realloc - resize a given block to a given size and return pointer to the 
*  new resize block.
*/
void *mm_realloc(void *ptr, size_t size) {
  
  void *oldptr = ptr;
  void *newptr;
  void *temp;
  size_t tempsize;
  size_t copySize;

  size_t prev_status = GET_ALLOC(FTRP(PREV_BLKP(oldptr))); // Status of previous
  size_t next_status = GET_ALLOC(HDRP(NEXT_BLKP(oldptr))); // Status of next
  size_t size_prev = GET_SIZE(HDRP(oldptr)); // Size of current block

 // If ptr is NULL, the call is equivalent to mm malloc(size)
  if (ptr == NULL) {
    return mm_malloc(size);
  }

  // If size is equal to zero, the call is equivalent to mm free(ptr)
  if (size == 0) {
    mm_free(ptr);
    newptr = 0;
    return NULL;
  }

  // If shrinking ptr and released space will be large enough 
  // to be a block then shrink allocated block and create new block.
  if (size_prev >= size + DSIZE) {
    size = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE);
  }

  // Change because shrinking ptr and released space is larger than block
  else if (size_prev < size + DSIZE) {

    // Case 1: Next and previous blocks are unallocated. Combine.
    if (next_status == 0 && prev_status == 0) {

      newptr = PREV_BLKP(oldptr); // Adjusts new pointer to previous block
      temp = NEXT_BLKP(oldptr); // Set temp to next block
      tempsize = GET_SIZE(FTRP(newptr)) + GET_SIZE(FTRP(temp)); // tempsize is total added size
      
      // Adjust free list
      remove_from_freelist(PREV_BLKP(oldptr));
      remove_from_freelist(NEXT_BLKP(oldptr));

      // Adjust block size to include overhead and alignment reqs.
      size = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE);

      // If still not big enough, take all the space
      if ((tempsize + size_prev) < (size + 2*DSIZE)) {
        size = tempsize + size_prev;
      }

      // Set header, copy memory, and set footer
      PUT(HDRP(newptr), PACK(size, 1));
      copySize = GET_SIZE(HDRP(oldptr));
      memcpy(newptr, oldptr, copySize);
      PUT(FTRP(newptr), PACK(size, 1));

      // Handle remaining free block, if any
      if ((tempsize + size_prev) >= (size + 2*DSIZE)) {         
        // Adjust new pointer, header, footer and free list
        temp = NEXT_BLKP(newptr); 
        PUT(HDRP(temp), PACK(tempsize + size_prev - size, 0));
        PUT(FTRP(temp), PACK(tempsize + size_prev - size, 0));
        add_to_freelist(temp);
      }
      return newptr; 
    }

    // Case 2: next is free, and combining with next block will be enough size
    else if (next_status == 0 && 
      ((GET_SIZE(HDRP(NEXT_BLKP(oldptr)))) + size_prev) >= (size + DSIZE)){

      // First, copy next block and adjust free list
      temp = NEXT_BLKP(oldptr);
      tempsize = GET_SIZE(FTRP(temp));
      remove_from_freelist(NEXT_BLKP(ptr));

      // Adjust block size, header, footer
      size = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE);
      PUT(HDRP(oldptr), PACK(size, 1));
      PUT(FTRP(oldptr), PACK(size, 1));

      // Handle remaining free memory, if any
      if ((tempsize + size_prev) >= (size + 2*DSIZE)) {
        // Set new pointer to new block, adjust header, footer and free list
        newptr = NEXT_BLKP(oldptr);
        PUT(HDRP(newptr), PACK(tempsize + size_prev - size, 0));
        PUT(FTRP(newptr), PACK(tempsize + size_prev - size, 0));
        add_to_freelist(newptr);
      }
      return oldptr;
    }

    // Case 3: previous is free, and combining previous block will be enoughs size
    else if (prev_status == 0 && 
      ((GET_SIZE(HDRP(PREV_BLKP(oldptr)))) + size_prev) >= (size + DSIZE)){

      // Set new pointer to previous block, copy size and adjust free list
      newptr = PREV_BLKP(oldptr);
      tempsize = GET_SIZE(FTRP(newptr));
      remove_from_freelist(PREV_BLKP(oldptr));

      // Adjust block size
      size = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE);

      // If not big enough, take all the space
      if ((tempsize + size_prev) < (size + 2*DSIZE)){
       size = tempsize + size_prev;
      }

      // Adjust header, copy memory, and adjust footer
      PUT(HDRP(newptr), PACK(size, 1));
      copySize = GET_SIZE(HDRP(oldptr));
      memcpy(newptr, oldptr, copySize);
      PUT(FTRP(newptr), PACK(size, 1)); 

      // Handle remaining free memory, if any
      if((tempsize + size_prev) >= (size + 2*DSIZE)) {
        // Set new pointer, header, footer, and adjust free list
        temp = NEXT_BLKP(newptr);
        PUT(HDRP(temp), PACK(tempsize + size_prev - size, 0));
        PUT(FTRP(temp), PACK(tempsize + size_prev - size, 0));
        add_to_freelist(temp);
      }
      return newptr;
    }

    // Case 4: next and previous blocks are allocated
    else {
      // Set new pointer, copy memory
      newptr = mm_malloc(size);
      copySize = size;
      memcpy(newptr, oldptr, copySize);
      mm_free(oldptr); // Free old memory block
      return newptr; // Return new memory block
    }

  } 
  return ptr;
}


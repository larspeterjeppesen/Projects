#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

typedef uint32_t heapNode;
typedef uint16_t heapIndex;

/* Nodes are represented as follows:                   */
/* 0 represents (), the empty list                     */
/* An odd value represents an integer                  */
/* A number 0 <= n < 2^30 is represented as 2n+1       */
/* A even value v represents a pair                    */
/*    where (v>>16) & 0x7fff is the index of the head  */
/*    and (v>>1) &  0x7fff is the index of the tail    */
/* Index 0 represents the empty list () no matter what */
/*    is stored at index 0 -- it is never followed     */
/* Note that the most significant bit is not used, so  */
/*    (node & 0x7ffffffff) == 0                        */
/* This means that you can use this bit for marking    */
/* Note that all nodes are the same size, so           */
/*   - No size field needed                            */
/*   - No splitting or joining of blocks needed        */
/*   - If a free list is used, it can be singly linked */
/*     and nodes can be taken from and put back to the */
/*     front of the list                               */

heapNode* heap;
heapNode* toHeap;
heapIndex nextFreeNode; /* index of next free node */
heapIndex next;
heapIndex scan;

void initHeap() /* initialize heap */
{
  heap = malloc(0x8000 * sizeof(heapNode));
  toHeap = malloc(0x8000 * sizeof(heapNode));
  nextFreeNode = 1;
}

heapIndex allocateNumber(uint32_t n) /* allocate number node */
{
  if (nextFreeNode == 0x8000) {
    fprintf(stderr, "Out of heap space\n");
    exit(1);
  }
  if (n>0x3fffffff) {
    fprintf(stderr, "Number too big to represent: 0x%"PRIx32"\n", n);
    exit(1);
  }
  heap[nextFreeNode] = (n<<1) | 1;
  return nextFreeNode++;
}

uint32_t getNumber(heapIndex v) /* get number value of number node */
{
  heapNode node = heap[v];
  if (node & 1 == 0) {
    fprintf(stderr, "Not a number node: (0x%"PRIx32"\n", node);
    exit(1);
  }
  return node >> 1;
}

heapIndex cons(heapIndex head, heapIndex tail) /* allocate pair node */
{
  if (nextFreeNode == 0x8000) {
    fprintf(stderr, "Out of heap space\n");
    exit(1);
  }
  if (head > 0x7fff || tail > 0x7fff) {
    fprintf(stderr, "Illegal pair: (0x%"PRIx32", 0x%"PRIx32")\n", head, tail);
    exit(1);
  }
  heap[nextFreeNode] = (head<<16) | (tail<<1);
  return nextFreeNode++;
}

heapIndex car(heapIndex value) /* take head of pair node */
{
  
  heapNode node = heap[value];
  if (node & 3 != 2) {
    fprintf(stderr, "Not a pair node: (0x%"PRIx32"\n", node);
    exit(1);
  }
  return (node>>16) & 0x7fff;
}

heapIndex cdr(heapIndex value) /* take tail of pair node */
{
  
  heapNode node = heap[value];
  if (node & 3 != 2) {
    fprintf(stderr, "Not a pair node: (0x%"PRIx32"\n", node);
    exit(1);
  }
  return (node>>1) & 0x7fff;
}

void printTree(heapIndex v) /* print tree */
{
  if (v == 0) { printf("()"); return; }
  heapNode n = heap[v];
  if (n & 1) {
    printf("%"PRIu32"",getNumber(v));
    return;
  }
  printf("(");
  printTree(car(v));
  printf(".");
  printTree(cdr(v));
  printf(")");
}

void forward(heapIndex p) {
  heapNode v = toHeap[p];
  if ((v & 1) | !(v & 0xffffffff)) {
    return;
  }

  heapIndex head = v >> 16 & 0x7fff;
  heapIndex tail = v >> 1 & 0x7fff;
  heapIndex toHead;
  heapIndex toTail;
  if (heap[head]>>31 == 0) { //copy node and update from-space
    toHeap[next] = heap[head];
    heap[head] = (1<<31) | (next << 1);
    toHead = next++;
  } else { // node already copied
    toHead = (heap[head]>>1) & 0x7fff;
  }
  if (heap[tail]>> 31 == 0) { //copy node and update from-space
    toHeap[next] = heap[tail];
    heap[tail] = (1<<31) | (next << 1);
    toTail = next++;
  } else { // node already copied
    toTail = heap[tail]>>1 & 0x7fff;
  }

  toHeap[p] = (toHead<<16) | (toTail<<1); // update indexes in pair currently scanned
  return;
}

heapIndex gc(heapIndex root) /* garbage collect and return new root */
{
  int liveCount = 0;

  scan = 1;
  next = scan + 1;
  //copy root to start of to-space
  toHeap[scan] = heap[root];
  heap[root] = (1<<31) | (scan << 1);

  root = scan;
  while (scan < next) {
    forward(scan);
    scan++;
  }
  nextFreeNode = scan;
  liveCount = scan;

  //Swap to- and from-space
  heapNode* temp = heap;
  heap = toHeap;
  toHeap = temp;

  fprintf(stderr,"%d live nodes\n", liveCount);
  return root;
}

/* You can add and modify code above this line */
  
/* Don't change anything below this line */

heapIndex iota(uint32_t n) /* make a list of integers from 1 to n */
{
  heapIndex list = 0; /* empty list */
  while (n > 0) {
    heapIndex head = allocateNumber(n--);
    list = cons(head, list);
  }
  return(list);
}

heapIndex filterDivides(heapIndex list, uint32_t n)
/* removes elements that divide evenly by n */
{
  if (list == 0) return list;  /* empty list */
  uint32_t m = getNumber(car(list));
  if (m % n == 0)
    return filterDivides(cdr(list), n);
  return cons(car(list), filterDivides(cdr(list), n));
}

heapIndex makeTree(int n)
/* Make tree of depth n */
{
  if (n < 1) return 0;
  heapIndex tree = makeTree(n-1);
  return cons(tree,cons(allocateNumber(n),tree));
}

int main(int ac, char **av)
{
  initHeap();
  
  heapIndex numbers = iota(1000); /* list of 1 .. n */
  numbers = cdr(numbers); /* drop first element */
  heapIndex primes = 0; /* will contain primes */
  while (numbers != 0) /* until end of list */
  {
    primes = cons(car(numbers), primes);
    uint32_t n = getNumber(car(numbers));
    numbers = filterDivides(numbers, n);
  }

  primes = gc(primes);

  printf("\n");
  printTree(primes);
  printf("\n\n");

  heapIndex tree = makeTree(8);

  tree = gc(tree);

  printf("\n");
  printTree(tree);
  printf("\n\n");
  numbers = iota(1000); /* list of 1 .. 1000 */
  numbers = cdr(numbers); /* drop first element */
  primes = 0; /* will contain primes at end */
  while (numbers != 0) /* until end of list */
  {
    primes = cons(car(numbers), primes);
    uint32_t n = getNumber(car(numbers));
    numbers = filterDivides(numbers, n);
  }
  
  primes = gc(primes);

  exit(0);
}

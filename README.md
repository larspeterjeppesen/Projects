# Projects
Various school projects/problems I have enjoyed solving during my time studying Computer science


## Garbage Collector in C
This was a task to add garbage collection to a program that produces data. The program uses a heap which is simulated as an array of constant size. Without modification, it runs out of memory during execution. The program needs to garbage collect a Lisp-like data-structure, ie. pairs consisting of a head and tail, where head is a value and tail is a pair.

I implemented garbage collection based on the copying collection model, which works by defining a from-space and to-space that is switched between as memory gets used. Allocated objects are tracked using a list of root nodes that point to pairs. 

What needs to happen when garbage collecting in this specific program is copying the node that the root points to from from-space to to-space, then updating the roots' pointer to point to the copied node in to-space. Then the copied node is scanned, and if it is a pair, we need to follow the indexes it contains and copy them and update the pair, similarly to how the root was updated. This process is repeated until as many nodes have been scanned as have been copied.
To make sure that no nodes are copied more than once from from-space, when a node is first copied, it's converted into a forwarding pointer by setting its value to \begin{verbatim} 1<<31 | next,\end{verbatim} where \verb|next| is the index the node has been copied to in to-space. This way, if a node has already been copied, the pair in to-space that is currently being scanned can simply set it's index or indexes to point to the already copied node or nodes.

The root nodes are copied manually to index 1 of to-space (as index 0 always represents the empty list). Then the copied contents are scanned as explained above. Lastly, pointers to the heap and to-heap are swapped.

The handed out code uses \verb|nextFreeNode| when allocating data. With copying collection, I have had no reason to alter that process, however the variable needs to be updated to the final index of \verb|scan| after garbage collecting, as that will represent the next unallocated block of heap memory.

As the \verb|scan|-variable always starts at index 1, the live count after garbage collection is simply the value of \verb|scan| when garbage collection is done, as index 0 should be counted as an allocated node.



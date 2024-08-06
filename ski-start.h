#ifndef SKI_START_H
#define SKI_START_H

#include <stdio.h>
#include <stdlib.h>

// Define constants
#define MAX_NODES 100000

typedef enum { K, I, S, APP } NodeType;

typedef struct {
    NodeType type;
    int left;
    int right;
} Node;

typedef struct {
    Node nodes[MAX_NODES];
    int size;
    int freeList[MAX_NODES];
    int freeListSize;
    int root;
} CombinatorTree;

// Function declarations
void initTree(CombinatorTree* tree);
void freeNode(CombinatorTree* tree, int nodeIndex);
int copyNodeRecursively(CombinatorTree* tree, int nodeIndex);
int addNode(CombinatorTree* tree, NodeType type, int left, int right);

#endif // SKI_START_H
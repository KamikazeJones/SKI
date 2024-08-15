#ifndef SKI_START_H
#define SKI_START_H

#include <stdio.h>
#include <stdlib.h>

#ifdef DEBUG
#define debug(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__)
#else
#define debug(fmt, ...)
#endif

// Define constants
#define MAX_NODES 1000000

typedef enum { K, I, S, APP, VAR, EMPTY } NodeType;

typedef struct {
    NodeType type;
    char *value;
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
int addVarNode(CombinatorTree* tree, char* value, int left, int right);

#endif // SKI_START_H
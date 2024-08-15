#include "ski-start.h"
#include "ski-parse.h"
#include <string.h>
#include <ctype.h>

void serializeSubtree(CombinatorTree* tree, int nodeIndex, char** buffer, int* size, int* pos);
void freeNodeRecursively(CombinatorTree* tree, int nodeIndex);

void initTree(CombinatorTree* tree) {
    tree->size = 0;
    tree->freeListSize = 0;
    tree->root = -1;
}

void freeTree(CombinatorTree* tree) {
    freeNodeRecursively(tree, tree->root);
    free(tree);
}

void printNode(CombinatorTree* tree, int index) {
    if(index == -1) {
        printf("NULL\n");
        return;
    }
    Node* node = &tree->nodes[index];
    printf("%d / ", index);
    switch (node->type) {
        case I: printf(" I"); break;
        case K: printf(" K"); break;
        case S: printf(" S"); break;
        case VAR: 
             printf(" VAR: %s", node->value); 
             if(node->value == NULL) {
                printf(" VAR: %s", "NULL\0");
             }
             else {
                printf(" VAR: %s", node->value);
             }
             break;
        case APP: printf("()"); break;
        case EMPTY: printf("--"); break;
    }    
}

int addNode(CombinatorTree* tree, NodeType type, int left, int right) {
    int index;

    if (tree->size >= MAX_NODES && tree->freeListSize == 0) {
        fprintf(stderr, "Error: Maximum number of nodes reached: used nodes: %d, free nodes: %d\n",  tree->size,  tree->freeListSize);
        exit(1);
    }
    if (tree->freeListSize > 0) {
        index = tree->freeList[--tree->freeListSize];
        // printf("reuse Index: %d\n", index);
    } else {
        index = tree->size++;
    }
    tree->nodes[index].type = type;
    tree->nodes[index].left = left;
    tree->nodes[index].right = right;

#ifdef DEBUG
    printf("addNode ");
    printNode(tree, index);
    printf("\n");
#endif
    return index;
}

int addVarNode(CombinatorTree* tree, char* value, int left, int right) {
    int index;

    if (tree->size >= MAX_NODES && tree->freeListSize == 0) {
        fprintf(stderr, "Error: Maximum number of nodes reached: used nodes: %d, free nodes: %d\n", tree->size, tree->freeListSize);
        exit(1);
    }
    if (tree->freeListSize > 0) {
        index = tree->freeList[--tree->freeListSize];
    } else {
        index = tree->size++;
    }
    tree->nodes[index].type = VAR;
    tree->nodes[index].value = strndup(value, strlen(value));
    if (tree->nodes[index].value == NULL) {
        fprintf(stderr, "Error: variable is null!\n");
        exit(1);
    }
    tree->nodes[index].left = left;
    tree->nodes[index].right = right;

#ifdef DEBUG
    printf("addVarNode ");
    printNode(tree, index);
    printf("\n");
#endif    

    return index;
}

void freeNode(CombinatorTree* tree, int nodeIndex) {
#ifdef DEBUG
    printf("freeNode ");
    printNode(tree, nodeIndex);
    printf("\n");
#endif
    if (nodeIndex != -1) {
        // printf("free Node: %d\n", nodeIndex);

        if (tree->freeListSize >= MAX_NODES) {
            fprintf(stderr, "Error: Free list overflow\n");
            exit(1);
        }        
        Node* node = &tree->nodes[nodeIndex];
        tree->freeList[tree->freeListSize++] = nodeIndex;
        if(node->type == VAR && node->value != NULL) {
            free(node->value);
        }
        node->value = NULL;
        node->type = EMPTY;
    }
    else {
#ifdef DEBUG        
        printf("free Node: -1\n");
#endif        
    }
}

void freeNodeRecursively(CombinatorTree* tree, int nodeIndex) {
    if (nodeIndex == -1) return;
    Node* node = &(tree->nodes[nodeIndex]);
    freeNodeRecursively(tree, node->left);
    freeNodeRecursively(tree, node->right);
    freeNode(tree, nodeIndex);
}


int copyNodeRecursively(CombinatorTree* tree, int nodeIndex) {
#ifdef DEBUG
        printf("copyNodeRecursively ");
        printNode(tree, nodeIndex);
        printf("\n");
#endif

    if (nodeIndex == -1) return -1;

    Node* node = &(tree->nodes[nodeIndex]);

    // Recursively copy the left and right children
    int newLeft = copyNodeRecursively(tree, node->left);
    int newRight = copyNodeRecursively(tree, node->right);

    int newNodeIndex;
    // Add the new node using the addNode function
    if(node->type == VAR) {
        newNodeIndex = addVarNode(tree, node->value, newLeft, newRight);
    }
    else {
        newNodeIndex = addNode(tree, node->type, newLeft, newRight);
    }

#ifdef DEBUG
    printf("copyNodeRecursively ");
    printNode(tree, nodeIndex);
    printf(" -> ");
    printNode(tree, newNodeIndex);
    printf("\n");
#endif        

    return newNodeIndex;
}

void serializeSubtree(CombinatorTree* tree, int nodeIndex, char** buffer, int* size, int* pos) {
    if (nodeIndex == -1) return;
    Node* node = &tree->nodes[nodeIndex];

    // Ensure there is enough space in the buffer
    if (*pos + 10 >= *size) { // 10 is an arbitrary number to ensure enough space
        *size *= 2;
        *buffer = (char*) realloc(*buffer, *size);
    }

    switch (node->type) {
        case I: (*buffer)[(*pos)++] = 'I'; break;
        case K: (*buffer)[(*pos)++] = 'K'; break;
        case S: (*buffer)[(*pos)++] = 'S'; break;
        case VAR: 
            (*buffer)[(*pos)++] = ' ';
            for (int i = 0; node->value[i] != '\0'; i++) {
                (*buffer)[(*pos)++] = node->value[i];
            }
            break;
        case APP:
            (*buffer)[(*pos)++] = '(';
            serializeSubtree(tree, node->left, buffer, size, pos);
            (*buffer)[(*pos)++] = ' ';
            serializeSubtree(tree, node->right, buffer, size, pos);
            (*buffer)[(*pos)++] = ')';
            break;
        case EMPTY: 
            printf("-%d-", nodeIndex);
            break;
    }
    (*buffer)[*pos] = '\0';
}

char *TreeToString(CombinatorTree* tree) {
    char* buffer = (char*) malloc(100000);
    int size = 100000;
    int pos = 0;
    serializeSubtree(tree, tree->root, &buffer, &size, &pos);
    return buffer;
}

void printTree(CombinatorTree* tree) {
    char *buf = TreeToString(tree); 
        printf("%s", buf);
        free(buf);
}

void applyI(CombinatorTree* tree, int root) {
    // we know already:
    // 1. root is an APP node
    // 2. left is an I node
    // 3. right is not null

    // we need to replace the root with right

        /*         Node
        //         /  \
        //        I    x 
        */
    
    Node* node = &tree->nodes[root];

    int I = node->left;
    int x = node->right;
    Node* xNode = &tree->nodes[x];

#ifdef DEBUG
    printf("applyI\n");
    printNode(tree, root);
    printf(", ");
    printNode(tree, x);
    printf("\n");
#endif

    node->type = xNode->type;
    node->left = xNode->left;
    node->right = xNode->right;

    if(xNode->type == VAR) {
        node->value = xNode->value;
        xNode->value = NULL;
    }
    // we can free the I node and the x node
    freeNode(tree, I);
    freeNode(tree, x);
}

void applyK(CombinatorTree* tree, int root) {
    // we know already:
    // 1. root is an APP node
    // 2. leftleft is an K node
    // 3. leftright is not null
    // 4. right is not null

    // we need to replace the root with leftright

    /*         Node
    //         /  \
    //       APP   y 
    //      /  \
    //     K    x
    */

    Node* node = &tree->nodes[root];
    int y = node->right;
    int Kx = node->left;
    int x = tree->nodes[Kx].right;
    int K = tree->nodes[Kx].left;
    Node* xNode = &tree->nodes[x];

#ifdef DEBUG
    printf("applyK\n");
    printNode(tree, root);
    printf(", ");
    printNode(tree, x);
    printf("\n");
#endif

    /* we can free the right node (y) recursivly (not used), the K node, the left node (APP), and the x node */
    freeNodeRecursively(tree, y);

    node->type = xNode->type;
    if(xNode->type == VAR) {
        node->value = xNode->value;
        xNode->value = NULL;
    }
    node->left = xNode->left;
    node->right = xNode->right;
                        
    freeNode(tree, K);
    freeNode(tree, Kx);
    freeNode(tree, x);                        

}

void applyS(CombinatorTree* tree, int root) {
    // copy node z!

    /*          APP
    //         /  \
    //       APP   z 
    //      /  \
    //    APP   y
    //    / \
    //   S   x
    */

    Node* node = &tree->nodes[root];
    int z = node->right;
    int Sxy = node->left;
    int Sx = tree->nodes[Sxy].left;
    int S = tree->nodes[Sx].left;
    int x = tree->nodes[Sx].right;    
    int y = tree->nodes[Sxy].right;

#ifdef DEBUG
    printf("applyS\n");
    printNode(tree, root);
    printf(", ");
    printNode(tree, x);
    printf(", ");
    printNode(tree, y);
    printf(", ");
    printNode(tree, z);
    printf("\n");
#endif

    int z1 = copyNodeRecursively(tree, z);

    int left = addNode(tree, APP, x, z);
    int right = addNode(tree, APP, y, z1);

    node->type = APP;
    node->left = left;
    node->right = right;

    // we can free the S node, the Sx-APP node and the ((Sx)y)-APP node
    freeNode(tree, S);      // S
    freeNode(tree, Sx);     // Sx-APP
    freeNode(tree, Sxy);    // ((Sx)y)-APP
}

int reduce(CombinatorTree* tree, int root) {
    if (root == -1) return -1;
    Node* node = &tree->nodes[root];
    if (node->type == APP) {

        Node* leftNode = &tree->nodes[node->left];
   
        if(node->right != -1) {
            if (leftNode->type == I) {
                applyI(tree, root);                
                return 1;
            } 
            else if (leftNode->type == APP) {
                Node* leftLeftNode = &tree->nodes[leftNode->left];
                if(leftNode->right != -1 ) {
                    if (leftLeftNode->type == K) {
                        applyK(tree, root);                        
                        return 1;
                    }

                    else if (leftLeftNode->type == APP) {
                        Node* s = &tree->nodes[leftLeftNode->left];
                        if (s->type == S) {                            
                            applyS(tree, root);
                            return 1;
                        }
                    }
                }
            }
        }

        if(reduce(tree, node->left)) return 1;

        if(reduce(tree, node->right)) return 1;
    }
    return 0;
}


void normalReduction(char *input) {
    CombinatorTree* tree = parse(input);
    int result = reduce(tree, tree->root);
    int count = 0;
    while(result) {
        count++;
        result = reduce(tree, tree->root);
    }
    printTree(tree);
    printf("\n");
}


void runTest(const char* testName, CombinatorTree* tree) {
    printf("%s - Original Tree: ", testName);
    printTree(tree);
    printf("\n");

    int result = reduce(tree, tree->root);
    int count = 0;
    while(result) {
        // printf("- Reduce Step (%d): ", count);
        count++;
        // printTree(tree);
        // printf("\nused nodes: %d, free nodes: %d\n", tree->size, tree->freeListSize);
        result = reduce(tree, tree->root);
    }

    printf("%s - Reduced Tree (%d): ", testName, count);
    printTree(tree);
    printf("\n");
    printf("\nused nodes: %d, free nodes: %d\n", tree->size, tree->freeListSize);
}

int test_ki() {
    CombinatorTree tree;
    initTree(&tree);

    // Example tree: (K I)
    int k = addNode(&tree, K, -1, -1);
    int i = addNode(&tree, I, -1, -1);
    int ki = addNode(&tree, APP, k, i);
    tree.root = ki;
    runTest("test_ki", &tree);

    return 0;
}

void testSKI(const char* input) {
    CombinatorTree* tree = parse(input);
    runTest(input, tree);
    freeTree(tree);
}


void appendStr(char** buffer, size_t* len, size_t* size, const char* str) {
    if (*len + strlen(str) + 10 >= *size) {
        // printf("realloc - size:%lu, len:%lu, strlen:%lu\n", *size, *len, strlen(str));
        *size += 1000;
        
        *buffer = (char*) realloc(*buffer, *size);
        if(*buffer == NULL) {
            printf("realloc failed\n");
            exit(1);
        }
        else {
            printf("realloc success\n");
        }
    }
    //Cprintf("%s | %s\n", str, *buffer);
    strcpy(*buffer + *len, str);
    *len = *len + strlen(str);
}

char* getNumber(int n) {
    char *buffer;
    
    size_t size = 100;
    size_t length = 0;
    buffer = (char*) malloc(size);
    const char *succ = "(S(S(KS)K) ";
    
    for(int i=1; i<n; i++) {
        appendStr(&buffer, &length, &size, succ);
        // printf("i: %d, strlen: %lu, length: %lu, size: %lu\n", i, strlen(buffer), length, size);
    }
    appendStr(&buffer, &length, &size, "I");
    for (int i = 1; i < n; i++) {
        appendStr(&buffer, &length, &size, ")");
        // printf("i: %d, strlen: %lu, length: %lu, size: %lu\n", i, strlen(buffer), length, size);
    }
    // printf("strlen: %lu, length: %lu, size: %lu\n", strlen(buffer), length, size);

    // printf("done: %s\n", buffer);

    return buffer;
}

int countSubString(char *str, char *sub) {
    int count = 0;
    int len = strlen(sub);
    while(strstr(str, "number") != NULL) {
        count++;
        str = strstr(str, "number") + len;
    }
    return count;
}

int mainTest() {
    char* buf;
    char buffer[100000];
    // test_ki();
    testSKI("S x func test");

    // testSKI("(S ((K ((K K) I)) (K I)) K)"); // (S ((K ((K K) I)) (K I)) K)
    testSKI("(((S K) I) ((S K)I))"); // (S ((K ((K K) I)) (K I)) K)
    testSKI("(S ((K ((K K) I)) (K I)))"); // (S ((K ((K K) I)) (K I)))
    testSKI("(((SI)I)K)");
    testSKI("S(KK)IK");
    testSKI("K");
    

#define NUMBER_7 "(S(S(KS)K) (S(S(KS)K) (S(S(KS)K) (S(S(KS)K) (S(S(KS)K) (S(S(KS)K) I))))))"

    printf("number 7: %s\n", NUMBER_7);
    buf = getNumber(7);
    printf("number 7: %s\n", buf);
    printf("before free: %s\n", buf);
    free(buf);
    buf = getNumber(3);

    printf("number 3: %s\n", buf);

    sprintf(buffer, "%s %s %s", buf, "f", "x");
    printf("test for number 3: %s\n", buffer);
    testSKI(buffer);
    free(buf);

    buf = getNumber(100);

    // sprintf(buffer, "%s %s", "S(S(S(SI(K(S(S(KS)(S(KK)S))(K(S(KK)(S(S(KS)K)))))))(KK))(K(KI)))(K(KI))", buf);
    
    sprintf(buffer, "%s %s f x", "S(K(SI(KK)))(S(SI(K(S(S(K(S(K(S(S(K(S(KS)K))S)(KK)))(S(K(SI))K)))(SI(K(KI))))(S(K(S(S(KS)K)))(SI(KK))))))(K(S(SI(K(KI)))(K(KI)))))", buf);
    
    printf("test for number 100: %s\n", buffer);

    testSKI(buffer);
    // testSKI("S(S(S(SI(K(S(S(KS)(S(KK)S))(K(S(KK)(S(S(KS)K)))))))(KK))(K(KI)))(K(KI)) (S(S(KS)K) I)");

    // testSKI("S(S(S(SI(K(S(S(KS)(S(KK)S))(K(S(KK)(S(S(KS)K)))))))(KK))(K(KI)))(K(KI))" NUMBER_7 " f x");
    // testSKI("S(K(SI(KK)))(S(SI(K(S(S(K(S(K(S(S(K(S(KS)K))S)(KK)))(S(K(SI))K)))(SI(K(KI))))(S(K(S(S(KS)K)))(SI(KK))))))(K(S(SI(K(KI)))(K(KI)))))" NUMBER_7 "S K");
    return 0;
}

int main(int argc, char** argv) {
    if (argc == 1) {
        return mainTest();
    }
    else {
        for (int i = 1; i < argc; i++) {
            normalReduction(argv[i]);
        }
    }
    return 0;
}
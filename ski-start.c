#include "ski-start.h"
#include "ski-parse.h"
#include <string.h>
#include <ctype.h>

void initTree(CombinatorTree* tree) {
    tree->size = 0;
    tree->freeListSize = 0;
    tree->root = -1;
}

void freeTree(CombinatorTree* tree) {
    free(tree);
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
    return index;
}

void freeNode(CombinatorTree* tree, int nodeIndex) {
    if (nodeIndex != -1) {
        // printf("free Node: %d\n", nodeIndex);

        if (tree->freeListSize >= MAX_NODES) {
            fprintf(stderr, "Error: Free list overflow\n");
            exit(1);
        }        
        tree->freeList[tree->freeListSize++] = nodeIndex;
    }
    else {
        // printf("free Node: -1\n");
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
    if (nodeIndex == -1) return -1;

    Node* node = &(tree->nodes[nodeIndex]);

    // Recursively copy the left and right children
    int newLeft = copyNodeRecursively(tree, node->left);
    int newRight = copyNodeRecursively(tree, node->right);

    // Add the new node using the addNode function
    int newNodeIndex = addNode(tree, node->type, newLeft, newRight);

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
        case APP:
            (*buffer)[(*pos)++] = '(';
            serializeSubtree(tree, node->left, buffer, size, pos);
            (*buffer)[(*pos)++] = ' ';
            serializeSubtree(tree, node->right, buffer, size, pos);
            (*buffer)[(*pos)++] = ')';
            break;
    }
    (*buffer)[*pos] = '\0';
}

char *TreeToString(CombinatorTree* tree) {
    char* buffer = (char*) malloc(1024);
    int size = 1024;
    int pos = 0;
    serializeSubtree(tree, tree->root, &buffer, &size, &pos);
    return buffer;
}

void printTree(CombinatorTree* tree) {
    char *buf = TreeToString(tree); 
        printf("%s", buf);
        free(buf);
}

void applyI(Node* root, Node* right) {
    // we know already:
    // 1. root is an APP node
    // 2. left is an I node
    // 3. right is not null

    // we need to replace the root with right

    root->type = right->type;
    root->left = right->left;
    root->right = right->right;
}

void applyK(Node* root, Node* leftright) {
    // we know already:
    // 1. root is an APP node
    // 2. leftleft is an K node
    // 3. leftright is not null
    // 4. right is not null

    // we need to replace the root with leftright

    root->type = leftright->type;
    root->left = leftright->left;
    root->right = leftright->right;
}

void applyS(CombinatorTree* tree, Node* root, int x, int y, int z) {
    // copy node z!
    int z1 = copyNodeRecursively(tree, z);

    int left = addNode(tree, APP, x, z);
    int right = addNode(tree, APP, y, z1);
    root->type = APP;
    root->left = left;
    root->right = right;
}

int reduce(CombinatorTree* tree, int root) {
    if (root == -1) return -1;
    Node* node = &tree->nodes[root];
    if (node->type == APP) {

        Node* leftNode = &tree->nodes[node->left];
   
        if(node->right != -1) {
            if (leftNode->type == I) {
                    /*         Node
                    //         /  \
                    //        I    x 
                    */
                freeNode(tree, node->left);
                freeNode(tree, node->right);
                applyI(node, &tree->nodes[node->right]);
                // we can free the I node and the right node
                return 1;
            } 
            else if (leftNode->type == APP) {
                Node* leftLeftNode = &tree->nodes[leftNode->left];
                if(leftNode->right != -1 ) {
                    if (leftLeftNode->type == K) {
                    /*         Node
                    //         /  \
                    //       APP   y 
                    //      /  \
                    //     K    x
                    */
                        /* we can free the right node (y) recursivly (not used), the K node, the left node (APP), and the x node */

                        
                        freeNodeRecursively(tree, node->right);
                        freeNode(tree, leftNode->left);
                        freeNode(tree, leftNode->right);
                        freeNode(tree, node->left);                        
                        applyK(node, &tree->nodes[leftNode->right]);
                        return 1;
                    }

                    /*          APP
                    //         /  \
                    //       APP   z 
                    //      /  \
                    //    APP   y
                    //    / \
                    //   S   x
                    */

                    else if (leftLeftNode->type == APP) {
                        Node* s = &tree->nodes[leftLeftNode->left];
                        if (s->type == S) {
                            
                            // we can free the S node, the Sx-APP node and the ((Sx)y)-APP node
                            freeNode(tree, leftLeftNode->left); // S
                            freeNode(tree, leftNode->left);     // Sx-APP
                            freeNode(tree, node->left);         // ((Sx)y)-APP
                            //                      x                     y              z
                            applyS(tree, node, leftLeftNode->right, leftNode->right, node->right);
                            
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

void runTest(const char* testName, CombinatorTree* tree) {
    printf("%s - Original Tree: ", testName);
    printTree(tree);
    printf("\n");

    int result = reduce(tree, tree->root);
    int count = 0;
    while(result) {
        printf("- Reduce Step (%d): ", ++count);
        printTree(tree);
        printf("\nused nodes: %d, free nodes: %d\n", tree->size, tree->freeListSize);
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


int main() {

    test_ki();
    // testSKI("(S ((K ((K K) I)) (K I)) K)"); // (S ((K ((K K) I)) (K I)) K)
    testSKI("(((S K) I) ((S K)I))"); // (S ((K ((K K) I)) (K I)) K)
    testSKI("(S ((K ((K K) I)) (K I)))"); // (S ((K ((K K) I)) (K I)))
    testSKI("(((SI)I)K)");
    testSKI("S(KK)IK");
    testSKI("K");
#define NUMBER_7 "(S(S(KS)K)  (S(S(KS)K)  (S(S(KS)K)  (S(S(KS)K)  (S(S(KS)K)  (S(S(KS)K) I))))))"
    testSKI(NUMBER_7 " S K ");
    // testSKI("S(S(S(SI(K(S(S(KS)(S(KK)S))(K(S(KK)(S(S(KS)K)))))))(KK))(K(KI)))(K(KI)) (S(S(KS)K) I)");

    //testSKI("S(S(S(SI(K(S(S(KS)(S(KK)S))(K(S(KK)(S(S(KS)K)))))))(KK))(K(KI)))(K(KI))" NUMBER_7 " S K");
    testSKI("S(K(SI(KK)))(S(SI(K(S(S(K(S(K(S(S(K(S(KS)K))S)(KK)))(S(K(SI))K)))(SI(K(KI))))(S(K(S(S(KS)K)))(SI(KK))))))(K(S(SI(K(KI)))(K(KI)))))" NUMBER_7 "S K");
    return 0;
}
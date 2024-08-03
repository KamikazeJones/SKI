#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_NODES 8192

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

void initTree(CombinatorTree* tree) {
    tree->size = 0;
    tree->freeListSize = 0;
    tree->root = -1;
}

int addNode(CombinatorTree* tree, NodeType type, int left, int right) {
    int index;
    if (tree->size >= MAX_NODES && tree->freeListSize == 0) {
        fprintf(stderr, "Error: Maximum number of nodes reached\n");
        exit(1);
    }
    if (tree->freeListSize > 0) {
        index = tree->freeList[--tree->freeListSize];
        printf("reuse Index: %d\n", index);
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
        printf("free Node: %d\n", nodeIndex);

        if (tree->freeListSize >= MAX_NODES) {
            fprintf(stderr, "Error: Free list overflow\n");
            exit(1);
        }        
        tree->freeList[tree->freeListSize++] = nodeIndex;
    }
    else {
        printf("free Node: -1\n");
    }
}

int copyNodeRecursively(CombinatorTree* tree, int nodeIndex) {
    if (nodeIndex == -1) return -1;

    Node* sourceNode = &tree->nodes[nodeIndex];

    // Recursively copy the left and right children
    int newLeft = copyNodeRecursively(tree, sourceNode->left);
    int newRight = copyNodeRecursively(tree, sourceNode->right);

    // Add the new node using the addNode function
    int newNodeIndex = addNode(tree, sourceNode->type, newLeft, newRight);

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
                applyI(node, &tree->nodes[node->right]);
                // we can free the I node
                freeNode(tree, node->left);
                return 1;
            } 
            else if (leftNode->type == APP) {
                Node* leftLeftNode = &tree->nodes[leftNode->left];
                if(leftNode->right != -1 ) {
                    if (leftLeftNode->type == K) {
                        // we should free the right node (not used) and the K node
                        freeNode(tree, node->left);
                        freeNode(tree, node->right);
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
                            
                            // we can free the S node, the Sx-APP node, and the ((Sx)y)-APP node
                            freeNode(tree, leftLeftNode->left);
                            freeNode(tree, leftNode->left);
                            freeNode(tree, node->left);
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
    while(result) {
        printf("- Reduce Step: ");
        printTree(tree);
        printf("\n");
        result = reduce(tree, tree->root);
    }

    printf("%s - Reduced Tree: ", testName);
    printTree(tree);
    printf("\n");
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

typedef struct {
    const char* input;
    int pos;
} Tokenizer;

typedef enum { TOKEN_S, TOKEN_K, TOKEN_I, TOKEN_LPAREN, TOKEN_RPAREN, TOKEN_END } TokenType;

typedef struct {
    TokenType type;
} Token;

Token getNextToken(Tokenizer* tokenizer) {
    while (isspace(tokenizer->input[tokenizer->pos])) {
        tokenizer->pos++;
    }

    char current = tokenizer->input[tokenizer->pos];
    tokenizer->pos++;

    switch (current) {
        case 'S': return (Token){TOKEN_S};
        case 'K': return (Token){TOKEN_K};
        case 'I': return (Token){TOKEN_I};
        case '(': return (Token){TOKEN_LPAREN};
        case ')': return (Token){TOKEN_RPAREN};
        case '\0': return (Token){TOKEN_END};
        default:
            fprintf(stderr, "Error: Unexpected character '%c'\n", current);
            exit(1);
    }
}

int parseExpression(Tokenizer* tokenizer, CombinatorTree* tree);

int parsePrimary(Tokenizer* tokenizer, CombinatorTree* tree) {
    Token token = getNextToken(tokenizer);
    switch (token.type) {
        case TOKEN_S: return addNode(tree, S, -1, -1);
        case TOKEN_K: return addNode(tree, K, -1, -1);
        case TOKEN_I: return addNode(tree, I, -1, -1);
        case TOKEN_LPAREN: {
            int left = parseExpression(tokenizer, tree);
            int right = parseExpression(tokenizer, tree);
            Token nextToken = getNextToken(tokenizer);
            if (nextToken.type != TOKEN_RPAREN) {
                fprintf(stderr, "Error: Expected ')'\n");
                exit(1);
            }
            return addNode(tree, APP, left, right);
        }
        default:
            fprintf(stderr, "Error: Unexpected token\n");
            exit(1);
    }
}

int parseExpression(Tokenizer* tokenizer, CombinatorTree* tree) {
    return parsePrimary(tokenizer, tree);
}

 CombinatorTree* parse(const char* input) {
    CombinatorTree* tree = (CombinatorTree*) malloc(sizeof(CombinatorTree));
    initTree(tree);
    Tokenizer tokenizer = {input, 0};
    tree->root = parseExpression(&tokenizer, tree);
    return tree;
 }

void testSKI(const char* input) {
    CombinatorTree* tree = parse(input);
 
    runTest(input, tree);
    free(tree);
}


int main() {

    test_ki();
    // testSKI("(S ((K ((K K) I)) (K I)) K)"); // (S ((K ((K K) I)) (K I)) K)
    testSKI("(((S K) I) ((S K)I))"); // (S ((K ((K K) I)) (K I)) K)
    testSKI("(S ((K ((K K) I)) (K I)))"); // (S ((K ((K K) I)) (K I)))
    testSKI("(((SI)I)K)");
    return 0;
}
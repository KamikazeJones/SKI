#include "ski-start.h"
#include <ctype.h>

typedef enum {
    TOKEN_S,
    TOKEN_K,
    TOKEN_I,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_END,
    TOKEN_ERROR
} TokenType;

typedef struct {
    const char* input;
    int pos;
} Tokenizer;

TokenType nextToken(Tokenizer* tokenizer) {
    while (tokenizer->input[tokenizer->pos] == ' ') {
        tokenizer->pos++;
    }

    char c = tokenizer->input[tokenizer->pos];
    tokenizer->pos++;

    switch (c) {
        case 'S': return TOKEN_S;
        case 'K': return TOKEN_K;
        case 'I': return TOKEN_I;
        case '(': return TOKEN_LPAREN;
        case ')': return TOKEN_RPAREN;
        case '\0': return TOKEN_END;
        default: return TOKEN_ERROR;
    }
}

// Tokenizer and other functions remain the same

int parsePrimary(Tokenizer* tokenizer, CombinatorTree* tree);

int parseExpression(Tokenizer* tokenizer, CombinatorTree* tree) {
    int left = parsePrimary(tokenizer, tree);
    while (1) {
        TokenType token = nextToken(tokenizer);
        if (token == TOKEN_END || token == TOKEN_RPAREN) {
            tokenizer->pos--;  // Unread the token
            return left;
        }
        tokenizer->pos--;  // Unread the token
        int right = parsePrimary(tokenizer, tree);
        left = addNode(tree, APP, left, right);
    }
}

int parsePrimary(Tokenizer* tokenizer, CombinatorTree* tree) {
    TokenType token = nextToken(tokenizer);
    if (token == TOKEN_LPAREN) {
        int expr = parseExpression(tokenizer, tree);
        if (nextToken(tokenizer) != TOKEN_RPAREN) {
            fprintf(stderr, "Error: Expected ')'\n");
            exit(1);
        }
        return expr;
    } else if (token == TOKEN_S) {
        return addNode(tree, S, -1, -1);
    } else if (token == TOKEN_K) {
        return addNode(tree, K, -1, -1);
    } else if (token == TOKEN_I) {
        return addNode(tree, I, -1, -1);
    } else {
        fprintf(stderr, "Error: Unexpected token\n");
        exit(1);
    }
}

CombinatorTree* parse(const char* input) {
    Tokenizer tokenizer = {input, 0};
    CombinatorTree* tree = (CombinatorTree*)malloc(sizeof(CombinatorTree));
    if (tree == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(1);
    }
    initTree(tree);
    tree->root = parseExpression(&tokenizer, tree);
    return tree;
}
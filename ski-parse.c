#include "ski-start.h"
#include <ctype.h>
#include <string.h>


typedef enum {
    TOKEN_S,
    TOKEN_K,
    TOKEN_I,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_VAR,
    TOKEN_END,
    TOKEN_ERROR
} TokenType;

typedef struct {
    const char* input;
    int pos;
    char *value;
} Tokenizer;

// Define the Token struct
typedef struct {
    TokenType type;
    char value[256];
} 
Token;

Token parseToken;

// Function to create a new token
Token *createToken(TokenType type) {
    parseToken.type = type;
    parseToken.value[0] = '\0';
    return &parseToken;
}

Token *createVarToken(TokenType type, const char *value, int length) {
    parseToken.type = type;
    if(length > 255) { 
        length = 255;
    }
    strncpy(parseToken.value, value, length);
    parseToken.value[length] = '\0';
    return &parseToken;

}   

Token* nextToken(Tokenizer* tokenizer) {
    while (tokenizer->input[tokenizer->pos] == ' ') {
        tokenizer->pos++;
    }

    char c = tokenizer->input[tokenizer->pos];
    // printf("(%d: %c),", tokenizer->pos, c);
    tokenizer->pos++;
    tokenizer->value = NULL;

    if (islower(c)) {
        // Handle lowercase letters
        int startpos = tokenizer->pos-1;
        char ch = tokenizer->input[tokenizer->pos];
        while (islower(ch)) {
            tokenizer->pos++;
            ch = tokenizer->input[tokenizer->pos];
        }   
        return createVarToken(TOKEN_VAR, tokenizer->input + startpos, tokenizer->pos - startpos);
    }
    switch (c) {
        case 'S': return createToken(TOKEN_S);
        case 'K': return createToken(TOKEN_K);
        case 'I': return createToken(TOKEN_I);
        case '(': return createToken(TOKEN_LPAREN);
        case ')': return createToken(TOKEN_RPAREN); 
        case '\0': return createToken(TOKEN_END);
        default: return createToken(TOKEN_ERROR);
    }
}

int parsePrimary(Tokenizer* tokenizer, CombinatorTree* tree);

int parseExpression(Tokenizer* tokenizer, CombinatorTree* tree) {
    int left = parsePrimary(tokenizer, tree);
    while (1) {
        int pos = tokenizer->pos;
        Token *token = nextToken(tokenizer);
        if (token->type == TOKEN_END || token->type == TOKEN_RPAREN) {
            tokenizer->pos = pos;  // Unread the token
            return left;
        }
        tokenizer->pos = pos;  // Unread the token
        int right = parsePrimary(tokenizer, tree);
        left = addNode(tree, APP, left, right);
    }
}

int parsePrimary(Tokenizer* tokenizer, CombinatorTree* tree) {
    Token *token = nextToken(tokenizer);
    if (token->type == TOKEN_LPAREN) {
        int expr = parseExpression(tokenizer, tree);
        if (nextToken(tokenizer)->type != TOKEN_RPAREN) {
            fprintf(stderr, "Error: Expected ')'\n");
            exit(1);
        }
        return expr;
    } else if (token->type == TOKEN_S) {
        return addNode(tree, S, -1, -1);
    } else if (token->type == TOKEN_K) {
        return addNode(tree, K, -1, -1);
    } else if (token->type == TOKEN_I) {
        return addNode(tree, I, -1, -1);
    }
    else if (token->type == TOKEN_VAR) {
        return addVarNode(tree, token->value, -1, -1);
    } 
    else {
        fprintf(stderr, "Error: Unexpected token\n");
        exit(1);
    }
}

CombinatorTree* parse(const char* input) {
    Tokenizer tokenizer = {input, 0, NULL};
    CombinatorTree* tree = (CombinatorTree*)malloc(sizeof(CombinatorTree));
    if (tree == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(1);
    }
    initTree(tree);
    tree->root = parseExpression(&tokenizer, tree);
    return tree;
}  
#pragma once

#include "sakura.h"
#include "sstr.h"

enum TokenType {
    // Single Character Tokens
    SAKURA_TOKEN_LEFT_PAREN,
    SAKURA_TOKEN_RIGHT_PAREN,
    SAKURA_TOKEN_LEFT_BRACE,
    SAKURA_TOKEN_RIGHT_BRACE,
    SAKURA_TOKEN_COMMA,
    SAKURA_TOKEN_DOT,
    SAKURA_TOKEN_MINUS,
    SAKURA_TOKEN_PLUS,
    SAKURA_TOKEN_SEMICOLON,
    SAKURA_TOKEN_SLASH,
    SAKURA_TOKEN_STAR,

    // Comparisons
    SAKURA_TOKEN_BANG,
    SAKURA_TOKEN_BANG_EQUAL,
    SAKURA_TOKEN_EQUAL,
    SAKURA_TOKEN_EQUAL_EQUAL,
    SAKURA_TOKEN_GREATER,
    SAKURA_TOKEN_GREATER_EQUAL,
    SAKURA_TOKEN_LESS,
    SAKURA_TOKEN_LESS_EQUAL,

    // Literals
    SAKURA_TOKEN_IDENTIFIER,
    SAKURA_TOKEN_STRING,
    SAKURA_TOKEN_NUMBER,

    // Keywords
    SAKURA_TOKEN_AND,
    SAKURA_TOKEN_CLASS,
    SAKURA_TOKEN_ELSE,
    SAKURA_TOKEN_FALSE,
    SAKURA_TOKEN_FUNCTION,
    SAKURA_TOKEN_FOR,
    SAKURA_TOKEN_IF,
    SAKURA_TOKEN_NIL,
    SAKURA_TOKEN_OR,
    SAKURA_TOKEN_PRINT,
    SAKURA_TOKEN_RETURN,
    SAKURA_TOKEN_SUPER,
    SAKURA_TOKEN_THIS,
    SAKURA_TOKEN_TRUE,
    SAKURA_TOKEN_VAR,
    SAKURA_TOKEN_WHILE,

    // Node Types
    SAKURA_NODE_UNARY_OPERATION,
    SAKURA_NODE_BINARY_OPERATION,

    // Misc
    SAKURA_TOKEN_SENTINEL
};

struct Token {
    enum TokenType type;
    const char *start;
    size_t length;
};

struct Node {
    enum TokenType type;
    struct Node *left;
    struct Node *right;
    struct Token *token;

    int leftLocation;
    int rightLocation;
};

struct TokenStack {
    struct Token **tokens;
    size_t size;
    size_t capacity;
};

struct NodeStack {
    struct Node **nodes;
    size_t size;
    size_t capacity;
};

struct TokenStack *sakuraX_newTokStack();
void sakuraX_freeTokStack(struct TokenStack *stack);
void sakuraX_pushTokStack(struct TokenStack *stack, struct Token *token);
struct Token *sakuraX_popTokStack(struct TokenStack *stack);
struct Token *sakuraX_peekTokStack(struct TokenStack *stack, int silent);
struct Token *sakuraX_peekTokStack_s(struct TokenStack *stack);

struct NodeStack *sakuraX_newNodeStack();
void sakuraX_freeNodeStack(struct NodeStack *stack);
void sakuraX_pushNodeStack(struct NodeStack *stack, struct Node *node);
struct Node *sakuraX_popNodeStack(struct NodeStack *stack);
struct Node *sakuraX_peekNodeStack(struct NodeStack *stack, int silent);
struct Node *sakuraX_peekNodeStack_s(struct NodeStack *stack);

struct TokenStack *sakuraY_analyze(SakuraState *S, struct s_str *source);
struct NodeStack *sakuraY_parse(SakuraState *S, struct TokenStack *tokens);

void sakuraY_freeToken(struct Token *token);
void sakuraY_freeNode(struct Node *node);

struct Node *sakuraX_parseUnary(SakuraState *S, struct Token *token, struct Node *left);
struct Node *sakuraX_binaryOperation(SakuraState *S, struct TokenStack *tokens, enum TokenType types[],
                                     struct Node *(*fn)(SakuraState *, struct TokenStack *));
struct Node *sakuraX_parseFactor(SakuraState *S, struct TokenStack *tokens);
struct Node *sakuraX_parseTerm(SakuraState *S, struct TokenStack *tokens);
struct Node *sakuraX_parseExpression(SakuraState *S, struct TokenStack *tokens);
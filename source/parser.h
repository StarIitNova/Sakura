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
    SAKURA_TOKEN_WHILE
};

enum NodeType { SAKURA_NODE_BINARY_OPERATION, SAKURA_NODE_UNARY_OPERATION, SAKURA_NODE_LITERAL };

struct Token {
    enum TokenType type;
    const char *start;
    size_t length;
};

struct Node {
    enum NodeType type;
    struct Node *left;
    struct Node *right;
    struct Token *token;
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

struct NodeStack *sakuraX_newNodeStack();
void sakuraX_freeNodeStack(struct NodeStack *stack);
void sakuraX_pushNodeStack(struct NodeStack *stack, struct Node *node);

struct TokenStack *sakuraY_analyze(SakuraState *state, struct s_str *source);
struct NodeStack *sakuraY_parse(SakuraState *state, struct TokenStack *tokens);

void sakuraY_freeToken(struct Token *token);
void sakuraY_freeNode(struct Node *node);
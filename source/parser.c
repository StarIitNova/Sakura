#include "parser.h"

#include <ctype.h>
#include <stdlib.h>

struct TokenStack *sakuraX_newTokStack() {
    struct TokenStack *stack = malloc(sizeof(struct TokenStack));
    stack->tokens = malloc(16 * sizeof(struct Token *));
    stack->capacity = 16;
    stack->size = 0;
    return stack;
}

void sakuraX_freeTokStack(struct TokenStack *stack) {
    if (stack->size > 0) {
        for (size_t i = 0; i < stack->size; i++) {
            sakuraY_freeToken(stack->tokens[i]);
        }
    }

    free(stack->tokens);
    free(stack);
}

void sakuraX_pushTokStack(struct TokenStack *stack, struct Token *token) {
    if (stack->size >= stack->capacity) {
        stack->capacity *= 2;
        stack->tokens = realloc(stack->tokens, stack->capacity * sizeof(struct Token *));
    }
    stack->tokens[stack->size++] = token;
}

struct NodeStack *sakuraX_newNodeStack() {
    struct NodeStack *stack = malloc(sizeof(struct NodeStack));
    stack->nodes = malloc(16 * sizeof(struct Node *));
    stack->capacity = 16;
    stack->size = 0;
    return stack;
}

void sakuraX_freeNodeStack(struct NodeStack *stack) {
    if (stack->size > 0) {
        for (size_t i = 0; i < stack->size; i++) {
            sakuraY_freeNode(stack->nodes[i]);
        }
    }

    free(stack->nodes);
    free(stack);
}

void sakuraX_pushNodeStack(struct NodeStack *stack, struct Node *node) {
    if (stack->size >= stack->capacity) {
        stack->capacity *= 2;
        stack->nodes = realloc(stack->nodes, stack->capacity * sizeof(struct Node *));
    }
    stack->nodes[stack->size++] = node;
}

struct TokenStack *sakuraY_analyze(SakuraState *state, struct s_str *source) {
    state->currentState = SAKURA_FLAG_LEXER;

    struct TokenStack *stack = sakuraX_newTokStack();

    for (size_t i = 0; i < source->len; i++) {
        while (i < source->len && isspace(source->str[i]))
            i++;

        if (i >= source->len)
            break;

        if (isdigit(source->str[i]) || source->str[i] == '.') {
            struct Token *tok = malloc(sizeof(struct Token));
            tok->type = SAKURA_TOKEN_NUMBER;
            tok->start = source->str + i;
            while (i < source->len && (isdigit(source->str[i]) || source->str[i] == '.'))
                i++;

            tok->length = --i - (tok->start - source->str);
            sakuraX_pushTokStack(stack, tok);
        } else if (isalpha(source->str[i]) || source->str[i] == '_') {
            struct Token *tok = malloc(sizeof(struct Token));
            tok->type = SAKURA_TOKEN_IDENTIFIER;
            tok->start = source->str + i;
            while (i < source->len && (isalnum(source->str[i]) || source->str[i] == '_' || isdigit(source->str[i])))
                i++;

            tok->length = --i - (tok->start - source->str);
            sakuraX_pushTokStack(stack, tok);
        } else if (source->str[i] == '"' || source->str[i] == '\'') {
            struct Token *tok = malloc(sizeof(struct Token));
            tok->type = SAKURA_TOKEN_STRING;
            tok->start = source->str + i;
            while (i < source->len && source->str[i] != *tok->start)
                i++;

            tok->length = i - (tok->start - source->str);
            sakuraX_pushTokStack(stack, tok);
        } else {
            struct Token *tok = malloc(sizeof(struct Token));
            tok->start = source->str + i;
            tok->length = 1;

            if (source->str[i] == '+')
                tok->type = SAKURA_TOKEN_PLUS;
            else if (source->str[i] == '-')
                tok->type = SAKURA_TOKEN_MINUS;
            else if (source->str[i] == '*')
                tok->type = SAKURA_TOKEN_STAR;
            else if (source->str[i] == '/')
                tok->type = SAKURA_TOKEN_SLASH;

            sakuraX_pushTokStack(stack, tok);
        }
    }

    return stack;
}

struct NodeStack *sakuraY_parse(SakuraState *state, struct TokenStack *tokens) {
    state->currentState = SAKURA_FLAG_PARSER;

    struct NodeStack *stack = sakuraX_newNodeStack();

    return stack;
}

void sakuraY_freeToken(struct Token *token) { free(token); }

void sakuraY_freeNode(struct Node *node) {
    if (node->left != NULL)
        sakuraY_freeNode(node->left);
    if (node->right != NULL)
        sakuraY_freeNode(node->right);
    if (node->token != NULL)
        sakuraY_freeToken(node->token);
    free(node);
}
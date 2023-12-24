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
    if (stack == NULL)
        return;

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

struct Token *sakuraX_popTokStack(struct TokenStack *stack) {
    if (stack->size == 0) {
        printf("Error: stack underflow\n");
        return NULL;
    }

    struct Token *token = stack->tokens[0];
    for (size_t i = 0; i < stack->size - 1; i++) {
        stack->tokens[i] = stack->tokens[i + 1];
    }

    stack->size--;
    return token;
}

struct Token *sakuraX_peekTokStack(struct TokenStack *stack, int silent) {
    if (stack->size > 0) {
        return stack->tokens[0];
    } else {
        if (!silent)
            printf("Error: stack underflow\n");
        return NULL;
    }
}

struct Token *sakuraX_peekTokStack_s(struct TokenStack *stack) { return sakuraX_peekTokStack(stack, 0); }

struct NodeStack *sakuraX_newNodeStack() {
    struct NodeStack *stack = malloc(sizeof(struct NodeStack));
    stack->nodes = malloc(16 * sizeof(struct Node *));
    stack->capacity = 16;
    stack->size = 0;
    return stack;
}

void sakuraX_freeNodeStack(struct NodeStack *stack) {
    if (stack == NULL)
        return;

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

struct Node *sakuraX_popNodeStack(struct NodeStack *stack) {
    if (stack->size == 0) {
        printf("Error: stack underflow\n");
        return NULL;
    }

    struct Node *token = stack->nodes[0];
    for (size_t i = 0; i < stack->size - 1; i++) {
        stack->nodes[i] = stack->nodes[i + 1];
    }

    stack->size--;
    return token;
}

struct Node *sakuraX_peekNodeStack(struct NodeStack *stack, int silent) {
    if (stack->size > 0) {
        return stack->nodes[0];
    } else {
        if (!silent)
            printf("Error: stack underflow\n");
        return NULL;
    }
}

struct Node *sakuraX_peekNodeStack_s(struct NodeStack *stack) { return sakuraX_peekNodeStack(stack, 0); }

struct TokenStack *sakuraY_analyze(SakuraState *S, struct s_str *source) {
    S->currentState = SAKURA_FLAG_LEXER;

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

            tok->length = i-- - (tok->start - source->str);
            sakuraX_pushTokStack(stack, tok);
        } else if (isalpha(source->str[i]) || source->str[i] == '_') {
            struct Token *tok = malloc(sizeof(struct Token));
            tok->type = SAKURA_TOKEN_IDENTIFIER;
            tok->start = source->str + i;
            while (i < source->len && (isalnum(source->str[i]) || source->str[i] == '_' || isdigit(source->str[i])))
                i++;

            tok->length = i-- - (tok->start - source->str);
            sakuraX_pushTokStack(stack, tok);
        } else if (source->str[i] == '"' || source->str[i] == '\'') {
            struct Token *tok = malloc(sizeof(struct Token));
            tok->type = SAKURA_TOKEN_STRING;
            tok->start = source->str + i;
            char quoteChar = source->str[i++];

            while (i < source->len && source->str[i] != quoteChar) {
                if (source->str[i] == '\\' && i + 1 < source->len) {
                    // handle escape sequences
                    i += 2;
                } else {
                    i++;
                }
            }

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
            else if (source->str[i] == ';')
                tok->type = SAKURA_TOKEN_SEMICOLON;
            else
                printf("Error: unexpected character '%c'\n", source->str[i]);

            sakuraX_pushTokStack(stack, tok);
        }
    }

    return stack;
}

struct Node *sakuraX_parseUnary(SakuraState *S, struct Token *token, struct Node *left) {
    struct Node *node = malloc(sizeof(struct Node));
    if (node == NULL) {
        printf("Error: could not allocate memory for node\n");
        return NULL;
    }

    node->type = SAKURA_NODE_UNARY_OPERATION;
    node->left = left;
    node->token = token;
    node->right = NULL;

    return node;
}

struct Node *sakuraX_binaryOperation(SakuraState *S, struct TokenStack *tokens, enum TokenType types[],
                                     struct Node *(*fn)(SakuraState *, struct TokenStack *)) {
    struct Node *left = fn(S, tokens);
    while (1) {
        struct Token *token = sakuraX_peekTokStack(tokens, 1);
        if (token == NULL)
            break;

        int hasType = 0;
        for (size_t i = 0; types[i] != SAKURA_TOKEN_SENTINEL; i++) {
            if (token->type == types[i]) {
                if (sakuraX_popTokStack(tokens) == NULL) {
                    S->error = SAKURA_EFLAG_SYNTAX;
                    printf("Error: could not pop token from stack\n");
                    return NULL;
                }

                struct Node *right = fn(S, tokens);
                struct Node *newNode = malloc(sizeof(struct Node));
                if (newNode == NULL) {
                    printf("Error: could not allocate memory for node\n");
                    return NULL;
                }

                newNode->type = SAKURA_NODE_BINARY_OPERATION;
                newNode->left = left;
                newNode->right = right;
                newNode->token = token;
                left = newNode;
                hasType = 1;
                break;
            }
        }

        if (hasType == 0)
            break;
    }

    return left;
}

struct Node *sakuraX_parseFactor(SakuraState *S, struct TokenStack *tokens) {
    struct Token *token = sakuraX_popTokStack(tokens);

    if (token->type == SAKURA_TOKEN_NUMBER) {
        struct Node *node = malloc(sizeof(struct Node));
        if (node == NULL) {
            printf("Error: could not allocate memory for node\n");
            return NULL;
        }

        node->type = SAKURA_TOKEN_NUMBER;
        node->left = NULL;
        node->right = NULL;
        node->token = token;

        return node;
    } else if (token->type == SAKURA_TOKEN_PLUS || token->type == SAKURA_TOKEN_MINUS) {
        struct Node *op = sakuraX_parseFactor(S, tokens);
        return sakuraX_parseUnary(S, token, op);
    } else if (token->type == SAKURA_TOKEN_LEFT_PAREN) {
        struct Node *node = sakuraX_parseExpression(S, tokens);
        token = sakuraX_popTokStack(tokens);
        if (token->type != SAKURA_TOKEN_RIGHT_PAREN) {
            printf("Error: expected ')'\n");
            sakuraY_freeNode(node);
            sakuraY_freeToken(token);
            return NULL;
        }

        sakuraY_freeToken(token);
        return node;
    } else {
        printf("Error: unexpected token '%.*s'\n", (int)token->length, token->start);
        sakuraY_freeToken(token);
        return NULL;
    }
}

struct Node *sakuraX_parseTerm(SakuraState *S, struct TokenStack *tokens) {
    return sakuraX_binaryOperation(S, tokens,
                                   (enum TokenType[]){SAKURA_TOKEN_STAR, SAKURA_TOKEN_SLASH, SAKURA_TOKEN_SENTINEL},
                                   sakuraX_parseFactor);
}

struct Node *sakuraX_parseExpression(SakuraState *S, struct TokenStack *tokens) {
    return sakuraX_binaryOperation(
        S, tokens, (enum TokenType[]){SAKURA_TOKEN_PLUS, SAKURA_TOKEN_MINUS, SAKURA_TOKEN_SENTINEL}, sakuraX_parseTerm);
}

struct NodeStack *sakuraY_parse(SakuraState *S, struct TokenStack *tokens) {
    S->currentState = SAKURA_FLAG_PARSER;
    struct NodeStack *stack = sakuraX_newNodeStack();

    while (tokens->size > 0) {
        struct Node *node = sakuraX_parseExpression(S, tokens);

        if (node != NULL) {
            sakuraX_pushNodeStack(stack, node);

            struct Token *token = sakuraX_popTokStack(tokens);
            if (token->type != SAKURA_TOKEN_SEMICOLON) {
                printf("Error: expected semicolon\n");
                sakuraY_freeToken(token);
                sakuraX_freeNodeStack(stack);
                stack = NULL;
                break;
            }

            sakuraY_freeToken(token);
        } else {
            printf("Error: could not parse expression\n");
            sakuraX_freeNodeStack(stack);
            stack = NULL;
            break;
        }
    }

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
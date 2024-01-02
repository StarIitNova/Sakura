#pragma once

#include "sakura.h"
#include "sstr.h"

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
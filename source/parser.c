#include "parser.h"

#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

struct Node *sakuraX_makeNode(enum TokenType type) {
    struct Node *node = malloc(sizeof(struct Node));
    if (node == NULL) {
        printf("Error: could not allocate memory for node\n");
        return NULL;
    }

    node->type = type;
    node->left = NULL;
    node->right = NULL;
    node->token = NULL;
    node->args = NULL;
    node->argCount = 0;
    node->elseBlock = NULL;
    node->leftLocation = -11111111;
    node->rightLocation = -11111111;

    return node;
}

void sakuraDEBUG_dumpNode(struct Node *node) {
    printf("Node %p: T%d L%p R%p Tk%p A%p AC%zu EB%p LL%d RL%d\n", node, node->type, node->left, node->right,
           node->token, node->args, node->argCount, node->elseBlock, node->leftLocation, node->rightLocation);
}

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
        printf("Error: stack underflow while popping token\n");
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
            printf("Error: stack underflow while peeking token\n");
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
        printf("Error: stack underflow while popping node\n");
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
            printf("Error: stack underflow while peeking node\n");
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

            switch (source->str[i]) {
            case '+':
                tok->type = SAKURA_TOKEN_PLUS;
                break;
            case '-':
                tok->type = SAKURA_TOKEN_MINUS;
                break;
            case '*':
                tok->type = SAKURA_TOKEN_STAR;
                break;
            case '/':
                tok->type = SAKURA_TOKEN_SLASH;
                break;
            case '^':
                tok->type = SAKURA_TOKEN_CARET;
                break;
            case '%':
                tok->type = SAKURA_TOKEN_PERCENT;
                break;
            case ';':
                tok->type = SAKURA_TOKEN_SEMICOLON;
                break;
            case ',':
                tok->type == SAKURA_TOKEN_COMMA;
                break;
            case '.':
                tok->type == SAKURA_TOKEN_DOT;
                break;
            case '(':
                tok->type = SAKURA_TOKEN_LEFT_PAREN;
                break;
            case ')':
                tok->type = SAKURA_TOKEN_RIGHT_PAREN;
                break;
            case '{':
                tok->type = SAKURA_TOKEN_LEFT_BRACE;
                break;
            case '}':
                tok->type = SAKURA_TOKEN_RIGHT_BRACE;
                break;
            case '#':
                tok->type = SAKURA_TOKEN_HASHTAG;
                break;
            case '!':
                if (i + 1 < source->len && source->str[i + 1] == '=') {
                    tok->type = SAKURA_TOKEN_BANG_EQUAL;
                    break;
                }
                tok->type = SAKURA_TOKEN_BANG;
                break;
            case '=':
                if (i + 1 < source->len && source->str[i + 1] == '=') {
                    tok->type = SAKURA_TOKEN_EQUAL_EQUAL;
                    break;
                }
                tok->type = SAKURA_TOKEN_EQUAL;
                break;
            case '>':
                if (i + 1 < source->len && source->str[i + 1] == '=') {
                    tok->type = SAKURA_TOKEN_GREATER_EQUAL;
                    break;
                }
                tok->type = SAKURA_TOKEN_GREATER;
                break;
            case '<':
                if (i + 1 < source->len && source->str[i + 1] == '=') {
                    tok->type = SAKURA_TOKEN_LESS_EQUAL;
                    break;
                }
                tok->type = SAKURA_TOKEN_LESS;
                break;
            case '&':
                if (i + 1 < source->len && source->str[i + 1] == '&') {
                    tok->type = SAKURA_TOKEN_AND;
                    break;
                }
                printf("Error: unexpected character '&', expected '&&'\n");
                break;
            case '|':
                if (i + 1 < source->len && source->str[i + 1] == '|') {
                    tok->type = SAKURA_TOKEN_OR;
                    break;
                }
                printf("Error: unexpected character '|', expected '||'\n");
                break;
            default:
                printf("Error: unexpected character '%c'\n", source->str[i]);
                break;
            }

            sakuraX_pushTokStack(stack, tok);
        }
    }

    return stack;
}

struct Node *sakuraX_parseExpressionEntry(SakuraState *S, struct TokenStack *tokens);

struct Node *sakuraX_parseUnary(SakuraState *S, struct Token *token, struct Node *left) {
    if (left->type == SAKURA_TOKEN_NUMBER) {
        if (token->type == SAKURA_TOKEN_MINUS) {
            left->storageValue = -left->storageValue;
        } else if (token->type == SAKURA_TOKEN_PLUS) {
            ; // do nothing
        } else {
            printf("Error: unknown unary operation '%d' on number\n", token->type);
        }
        return left;
    }

    struct Node *node = sakuraX_makeNode(SAKURA_NODE_UNARY_OPERATION);

    node->left = left;
    node->token = token;

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
                    sakuraY_freeToken(token);
                    printf("Error: could not pop token from stack\n");
                    return NULL;
                }

                struct Node *right = fn(S, tokens);

                int countPass = 0;

                if (left->type == SAKURA_TOKEN_NUMBER && right->type == SAKURA_TOKEN_NUMBER) {
                    int exitV = 0;
                    switch (token->type) {
                    case SAKURA_TOKEN_PLUS:
                        left->storageValue += right->storageValue;
                        break;
                    case SAKURA_TOKEN_MINUS:
                        left->storageValue -= right->storageValue;
                        break;
                    case SAKURA_TOKEN_STAR:
                        left->storageValue *= right->storageValue;
                        break;
                    case SAKURA_TOKEN_SLASH:
                        left->storageValue /= right->storageValue;
                        break;
                    case SAKURA_TOKEN_CARET:
                        left->storageValue = pow(left->storageValue, right->storageValue);
                        break;
                    case SAKURA_TOKEN_PERCENT:
                        left->storageValue = fmod(left->storageValue, right->storageValue);
                        break;
                    case SAKURA_TOKEN_LESS:
                        left->storageValue = left->storageValue < right->storageValue ? 1 : 0;
                        break;
                    case SAKURA_TOKEN_LESS_EQUAL:
                        left->storageValue = left->storageValue <= right->storageValue ? 1 : 0;
                        break;
                    case SAKURA_TOKEN_GREATER:
                        left->storageValue = left->storageValue > right->storageValue ? 1 : 0;
                        break;
                    case SAKURA_TOKEN_GREATER_EQUAL:
                        left->storageValue = left->storageValue >= right->storageValue ? 1 : 0;
                        break;
                    case SAKURA_TOKEN_EQUAL_EQUAL:
                        left->storageValue = left->storageValue == right->storageValue ? 1 : 0;
                        break;
                    case SAKURA_TOKEN_BANG_EQUAL:
                        left->storageValue = left->storageValue != right->storageValue ? 1 : 0;
                        break;
                    case SAKURA_TOKEN_AND:
                        left->storageValue = left->storageValue == 1 && right->storageValue == 1 ? 1 : 0;
                        break;
                    case SAKURA_TOKEN_OR:
                        left->storageValue = left->storageValue == 1 || right->storageValue == 1 ? 1 : 0;
                        break;
                    default:
                        exitV = 1;
                        break;
                    }

                    if (exitV == 0) {
                        sakuraY_freeToken(token);
                        sakuraY_freeNode(right);
                        hasType = 1;
                        break;
                    }
                }

                struct Node *newNode = sakuraX_makeNode(SAKURA_NODE_BINARY_OPERATION);

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
        struct Node *node = sakuraX_makeNode(SAKURA_TOKEN_NUMBER);

        node->token = token;

        char *tokStr = (char *)malloc(token->length + 1);
        memcpy(tokStr, token->start, token->length);
        tokStr[token->length] = '\0';
        node->storageValue = strtod(tokStr, NULL);
        free(tokStr);

        return node;
    } else if (token->type == SAKURA_TOKEN_PLUS || token->type == SAKURA_TOKEN_MINUS) {
        struct Node *op = sakuraX_parseFactor(S, tokens);
        return sakuraX_parseUnary(S, token, op);
    } else if (token->type == SAKURA_TOKEN_LEFT_PAREN) {
        struct Node *node = sakuraX_parseExpression(S, tokens);
        sakuraY_freeToken(token);
        token = sakuraX_popTokStack(tokens);
        if (token->type != SAKURA_TOKEN_RIGHT_PAREN) {
            printf("Error: expected ')'\n");
            sakuraY_freeNode(node);
            sakuraY_freeToken(token);
            return NULL;
        }

        sakuraY_freeToken(token);
        return node;
    } else if (token->type == SAKURA_TOKEN_IDENTIFIER) {
        struct Token *nextToken = sakuraX_peekTokStack(tokens, 1);
        if (nextToken != NULL && nextToken->type == SAKURA_TOKEN_LEFT_PAREN) {
            struct Node *node = sakuraX_makeNode(SAKURA_NODE_CALL);

            node->token = token;

            sakuraY_freeToken(sakuraX_popTokStack(tokens));
            int hasRparen = 0;
            while (1) {
                struct Node *arg = sakuraX_parseExpression(S, tokens);
                if (arg != NULL) {
                    node->args = realloc(node->args, (node->argCount + 1) * sizeof(struct Node *));
                    node->args[node->argCount++] = arg;

                    struct Token *comma = sakuraX_popTokStack(tokens);
                    if (comma != NULL && comma->type == SAKURA_TOKEN_COMMA) {
                        sakuraY_freeToken(comma);
                        continue;
                    } else {
                        if (comma != NULL && comma->type == SAKURA_TOKEN_RIGHT_PAREN) {
                            hasRparen = 1;
                            sakuraY_freeToken(comma);
                            break;
                        } else {
                            printf("Error: expected ',' or ')'\n");
                            sakuraY_freeNode(node);
                            sakuraY_freeToken(comma);
                            return NULL;
                        }
                    }

                    break;
                } else {
                    printf("Error: could not parse argument\n");
                    sakuraY_freeNode(node);
                    return NULL;
                }
            }

            if (!hasRparen) {
                struct Token *rightParen = sakuraX_popTokStack(tokens);
                if (rightParen == NULL || rightParen->type != SAKURA_TOKEN_RIGHT_PAREN) {
                    printf("Error: expected ')'\n");
                    sakuraY_freeNode(node);
                    sakuraY_freeToken(rightParen);
                    return NULL;
                }

                sakuraY_freeToken(rightParen);
            }

            return node;
        } else {
            struct Node *node = sakuraX_makeNode(SAKURA_TOKEN_IDENTIFIER);

            node->token = token;

            return node;
        }
    } else {
        printf("Error: unexpected token '%.*s'\n", (int)token->length, token->start);
        sakuraY_freeToken(token);
        return NULL;
    }
}

struct Node *sakuraX_parseAtom(SakuraState *S, struct TokenStack *tokens) {
    return sakuraX_binaryOperation(S, tokens, (enum TokenType[]){SAKURA_TOKEN_CARET, SAKURA_TOKEN_SENTINEL},
                                   sakuraX_parseFactor);
}

struct Node *sakuraX_parseTerm(SakuraState *S, struct TokenStack *tokens) {
    return sakuraX_binaryOperation(
        S, tokens,
        (enum TokenType[]){SAKURA_TOKEN_STAR, SAKURA_TOKEN_SLASH, SAKURA_TOKEN_PERCENT, SAKURA_TOKEN_SENTINEL},
        sakuraX_parseAtom);
}

struct Node *sakuraX_parseExpression(SakuraState *S, struct TokenStack *tokens) {
    return sakuraX_binaryOperation(
        S, tokens, (enum TokenType[]){SAKURA_TOKEN_PLUS, SAKURA_TOKEN_MINUS, SAKURA_TOKEN_SENTINEL}, sakuraX_parseTerm);
}

struct Node *sakuraX_parseComparisons(SakuraState *S, struct TokenStack *tokens) {
    return sakuraX_binaryOperation(S, tokens,
                                   (enum TokenType[]){SAKURA_TOKEN_EQUAL_EQUAL, SAKURA_TOKEN_BANG_EQUAL,
                                                      SAKURA_TOKEN_GREATER, SAKURA_TOKEN_GREATER_EQUAL,
                                                      SAKURA_TOKEN_LESS, SAKURA_TOKEN_LESS_EQUAL,
                                                      SAKURA_TOKEN_SENTINEL},
                                   sakuraX_parseExpression);
}

struct Node *sakuraX_parseLogical(SakuraState *S, struct TokenStack *tokens) {
    return sakuraX_binaryOperation(S, tokens,
                                   (enum TokenType[]){SAKURA_TOKEN_AND, SAKURA_TOKEN_OR, SAKURA_TOKEN_SENTINEL},
                                   sakuraX_parseComparisons);
}

struct Node *sakuraX_parseVar(SakuraState *S, struct TokenStack *tokens) {
    struct Token *token = sakuraX_peekTokStack(tokens, 1);

    if (token->type == SAKURA_TOKEN_IDENTIFIER && str_cmp_cl(token->start, token->length, "let") == 0) {
        sakuraY_freeToken(sakuraX_popTokStack(tokens));
        struct Token *name = sakuraX_popTokStack(tokens);
        if (name == NULL || name->type != SAKURA_TOKEN_IDENTIFIER) {
            printf("Error: expected identifier\n");
            sakuraY_freeToken(name);
            return NULL;
        }

        struct Node *node = sakuraX_makeNode(SAKURA_NODE_VAR);
        node->token = name;

        struct Token *equal = sakuraX_popTokStack(tokens);
        if (equal == NULL || equal->type != SAKURA_TOKEN_EQUAL) {
            printf("Error: expected '='\n");
            sakuraY_freeToken(equal);
            sakuraY_freeNode(node);
            return NULL;
        }

        sakuraY_freeToken(equal);

        struct Node *value = sakuraX_parseExpressionEntry(S, tokens);
        if (value == NULL) {
            printf("Error: could not parse value\n");
            sakuraY_freeNode(node);
            return NULL;
        }

        node->left = value;

        return node;
    }

    return sakuraX_parseLogical(S, tokens);
}

struct Node *sakuraX_parseExpressionEntry(SakuraState *S, struct TokenStack *tokens) {
    return sakuraX_parseVar(S, tokens);
}

struct Node *sakuraX_parseBlocks(SakuraState *S, struct TokenStack *tokens) {
    // if statements
    struct Token *token = sakuraX_peekTokStack(tokens, 1);
    if (token->type == SAKURA_TOKEN_IDENTIFIER && str_cmp_cl(token->start, token->length, "if") == 0) {
        struct Node *node = sakuraX_makeNode(SAKURA_NODE_IF);

        sakuraY_freeToken(sakuraX_popTokStack(tokens));
        struct Node *condition = sakuraX_parseExpressionEntry(S, tokens);
        if (condition == NULL) {
            printf("Error: could not parse condition\n");
            sakuraY_freeNode(node);
            return NULL;
        }

        node->left = condition;

        struct Node *block = sakuraX_parseBlocks(S, tokens);
        if (block == NULL) {
            printf("Error: could not parse block\n");
            sakuraY_freeNode(node);
            return NULL;
        }

        node->right = block;

        struct Token *elseToken = sakuraX_peekTokStack(tokens, 1);
        if (elseToken != NULL && elseToken->type == SAKURA_TOKEN_IDENTIFIER &&
            str_cmp_cl(elseToken->start, elseToken->length, "else") == 0) {
            sakuraY_freeToken(sakuraX_popTokStack(tokens));
            struct Node *elseBlock = sakuraX_parseBlocks(S, tokens);
            if (elseBlock == NULL) {
                printf("Error: could not parse else block\n");
                sakuraY_freeNode(node);
                return NULL;
            }

            node->elseBlock = elseBlock;
        } else {
            node->elseBlock = NULL;
        }

        return node;
    } else if (token->type == SAKURA_TOKEN_IDENTIFIER && str_cmp_cl(token->start, token->length, "while") == 0) {
        struct Node *node = sakuraX_makeNode(SAKURA_NODE_WHILE);

        sakuraY_freeToken(sakuraX_popTokStack(tokens));
        struct Node *condition = sakuraX_parseExpressionEntry(S, tokens);
        if (condition == NULL) {
            printf("Error: could not parse condition\n");
            sakuraY_freeNode(node);
            return NULL;
        }

        node->left = condition;

        struct Node *block = sakuraX_parseBlocks(S, tokens);
        if (block == NULL) {
            printf("Error: could not parse block\n");
            sakuraY_freeNode(node);
            return NULL;
        }

        node->right = block;

        return node;
    } else if (token->type == SAKURA_TOKEN_IDENTIFIER && str_cmp_cl(token->start, token->length, "loop") == 0) {
        struct Node *node = sakuraX_makeNode(SAKURA_NODE_WHILE);
        node->left = NULL;

        sakuraY_freeToken(sakuraX_popTokStack(tokens));

        struct Node *block = sakuraX_parseBlocks(S, tokens);
        if (block == NULL) {
            printf("Error: could not parse block\n");
            sakuraY_freeNode(node);
            return NULL;
        }

        node->right = block;

        return node;
    } else if (token->type == SAKURA_TOKEN_LEFT_BRACE) {
        struct Node *node = sakuraX_makeNode(SAKURA_NODE_BLOCK);

        sakuraY_freeToken(sakuraX_popTokStack(tokens));

        while (sakuraX_peekTokStack_s(tokens)->type != SAKURA_TOKEN_RIGHT_BRACE) {
            struct Node *block = sakuraX_parseBlocks(S, tokens);
            if (block != NULL) {
                node->args = realloc(node->args, (node->argCount + 1) * sizeof(struct Node *));
                node->args[node->argCount++] = block;
            } else {
                printf("Error: could not parse block\n");
                sakuraY_freeNode(node);
                return NULL;
            }
        }

        sakuraY_freeToken(sakuraX_popTokStack(tokens));

        return node;
    }

    return sakuraX_parseExpressionEntry(S, tokens);
}

struct NodeStack *sakuraY_parse(SakuraState *S, struct TokenStack *tokens) {
    S->currentState = SAKURA_FLAG_PARSER;
    struct NodeStack *stack = sakuraX_newNodeStack();

    while (tokens->size > 0) {
        struct Node *node = sakuraX_parseBlocks(S, tokens);

        if (node != NULL) {
            sakuraX_pushNodeStack(stack, node);
        } else {
            printf("Error: could not parse expression\n");
            sakuraX_freeNodeStack(stack);
            stack = NULL;
            break;
        }
    }

    return stack;
}

void sakuraY_freeToken(struct Token *token) {
    free(token);
    token = NULL;
}

void sakuraY_freeNode(struct Node *node) {
    if (node->left != NULL) {
        sakuraY_freeNode(node->left);
        node->left = NULL;
    }

    if (node->right != NULL) {
        sakuraY_freeNode(node->right);
        node->right = NULL;
    }

    if (node->token != NULL) {
        sakuraY_freeToken(node->token);
        node->token = NULL;
    }

    if (node->args != NULL) {
        if (node->argCount > 0) {
            for (size_t i = 0; i < node->argCount; i++) {
                sakuraY_freeNode(node->args[i]);
            }
        }
        free(node->args);
        node->args = NULL;
    }

    if (node->elseBlock != NULL) {
        sakuraY_freeNode(node->elseBlock);
        node->elseBlock = NULL;
    }

    free(node);
    node = NULL;
}
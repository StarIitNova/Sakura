#include "parser.h"

#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

struct Node *sakuraX_makeNode(enum TokenType type) {
    struct Node *node = (struct Node *)malloc(sizeof(struct Node));
    if (node == NULL) {
        printf("Error: could not allocate memory for node\n");
        return NULL;
    }

    node->type = type;
    node->left = NULL;
    node->right = NULL;
    node->token = NULL;
    node->keys = NULL;
    node->args = NULL;
    node->argCount = 0;
    node->elseBlock = NULL;
    node->leftLocation = -11111111;  // value is chosen as a poison
    node->rightLocation = -11111111; // value is chosen as a poison

    return node;
}

void sakuraDEBUG_dumpNode(struct Node *node) {
    printf("Node %p: T%d L%p R%p Tk%p A%p AC%zu EB%p LL%d RL%d\n", node, node->type, node->left, node->right,
           node->token, node->args, node->argCount, node->elseBlock, node->leftLocation, node->rightLocation);
}

struct TokenStack *sakuraX_newTokStack(void) {
    struct TokenStack *stack = (struct TokenStack *)malloc(sizeof(struct TokenStack));
    stack->tokens = (struct Token **)malloc(16 * sizeof(struct Token *));
    stack->capacity = 16;
    stack->size = 0;
    return stack;
}

void sakuraX_freeTokStack(struct TokenStack *stack) {
    if (stack == NULL)
        return;

    if (stack->size > 0) {
        for (ull i = 0; i < stack->size; i++) {
            sakuraY_freeToken(stack->tokens[i]);
        }
    }

    free(stack->tokens);
    free(stack);
}

void sakuraX_pushTokStack(struct TokenStack *stack, struct Token *token) {
    if (stack->size >= stack->capacity) {
        stack->capacity *= 2;
        stack->tokens = (struct Token **)realloc(stack->tokens, stack->capacity * sizeof(struct Token *));
    }
    stack->tokens[stack->size++] = token;
}

struct Token *sakuraX_popTokStack(struct TokenStack *stack) {
    struct Token *token;

    if (stack->size == 0) {
        printf("Error: stack underflow while popping token\n");
        return NULL;
    }

    token = stack->tokens[0];
    for (ull i = 0; i < stack->size - 1; i++)
        stack->tokens[i] = stack->tokens[i + 1];

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

struct NodeStack *sakuraX_newNodeStack(void) {
    struct NodeStack *stack = (struct NodeStack *)malloc(sizeof(struct NodeStack));
    stack->nodes = (struct Node **)malloc(16 * sizeof(struct Node *));
    stack->capacity = 16;
    stack->size = 0;
    return stack;
}

void sakuraX_freeNodeStack(struct NodeStack *stack) {
    if (stack == NULL)
        return;

    if (stack->size > 0) {
        for (ull i = 0; i < stack->size; i++) {
            sakuraY_freeNode(stack->nodes[i]);
        }
    }

    free(stack->nodes);
    free(stack);
}

void sakuraX_pushNodeStack(struct NodeStack *stack, struct Node *node) {
    if (stack->size >= stack->capacity) {
        stack->capacity *= 2;
        stack->nodes = (struct Node **)realloc(stack->nodes, stack->capacity * sizeof(struct Node *));
    }
    stack->nodes[stack->size++] = node;
}

struct Node *sakuraX_popNodeStack(struct NodeStack *stack) {
    struct Node *node;
    if (stack->size == 0) {
        printf("Error: stack underflow while popping node\n");
        return NULL;
    }

    node = stack->nodes[0];
    for (ull i = 0; i < stack->size - 1; i++) {
        stack->nodes[i] = stack->nodes[i + 1];
    }

    stack->size--;
    return node;
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
    struct TokenStack *stack = sakuraX_newTokStack();

    LOG_CALL();

    S->currentState = SAKURA_FLAG_LEXER;

    for (int i = 0; i < source->len; i++) {
        while (i < source->len && isspace(source->str[i]))
            i++;

        if (i >= source->len)
            break;

        if (isdigit(source->str[i]) || source->str[i] == '.') {
            struct Token *tok = (struct Token *)malloc(sizeof(struct Token));
            tok->type = SAKURA_TOKEN_NUMBER;
            tok->start = source->str + i;
            while (i < source->len && (isdigit(source->str[i]) || source->str[i] == '.'))
                i++;

            tok->length = i-- - (tok->start - source->str);
            sakuraX_pushTokStack(stack, tok);
        } else if (isalpha(source->str[i]) || source->str[i] == '_') {
            struct Token *tok = (struct Token *)malloc(sizeof(struct Token));
            tok->type = SAKURA_TOKEN_IDENTIFIER;
            tok->start = source->str + i;
            while (i < source->len && (isalnum(source->str[i]) || source->str[i] == '_' || isdigit(source->str[i])))
                i++;

            tok->length = i-- - (tok->start - source->str);
            sakuraX_pushTokStack(stack, tok);
        } else if (source->str[i] == '"' || source->str[i] == '\'') {
            struct Token *tok = (struct Token *)malloc(sizeof(struct Token));
            char quoteChar;

            tok->type = SAKURA_TOKEN_STRING;
            tok->start = source->str + i + 1;
            quoteChar = source->str[i++];

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
            struct Token *tok = (struct Token *)malloc(sizeof(struct Token));
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
                tok->type = SAKURA_TOKEN_COMMA;
                break;
            case '.':
                tok->type = SAKURA_TOKEN_DOT;
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
            case '[':
                tok->type = SAKURA_TOKEN_LEFT_SQUARE;
                break;
            case ']':
                tok->type = SAKURA_TOKEN_RIGHT_SQUARE;
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

    LOG_POP();
    return stack;
}

struct Node *sakuraX_parseUnary(SakuraState *S, struct Token *token, struct Node *left) {
    struct Node *node;

    UNUSED(S);

    LOG_CALL();

    if (left->type == SAKURA_TOKEN_NUMBER) {
        if (token->type == SAKURA_TOKEN_MINUS) {
            left->storageValue = -left->storageValue;
        } else if (token->type == SAKURA_TOKEN_PLUS) {
            ; // do nothing
        } else {
            printf("Error: unknown unary operation '%d' on number\n", token->type);
        }

        LOG_POP();
        return left;
    }

    node = sakuraX_makeNode(SAKURA_NODE_UNARY_OPERATION);
    node->left = left;
    node->token = token;

    LOG_POP();
    return node;
}

struct Node *sakuraX_binaryOperation(SakuraState *S, struct TokenStack *tokens, enum TokenType types[],
                                     struct Node *(*fn)(SakuraState *, struct TokenStack *)) {
    struct Node *left, *right, *newNode;
    int hasType = 0;

    LOG_CALL();

    left = fn(S, tokens);
    while (1) {
        struct Token *token = sakuraX_peekTokStack(tokens, 1);
        if (token == NULL)
            break;

        hasType = 0;
        for (ull i = 0; types[i] != SAKURA_TOKEN_SENTINEL; i++) {
            if (token->type == types[i]) {
                if (sakuraX_popTokStack(tokens) == NULL) {
                    S->error = SAKURA_EFLAG_SYNTAX;
                    sakuraY_freeToken(token);
                    printf("Error: could not pop token from stack\n");
                    LOG_POP();
                    return NULL;
                }

                right = fn(S, tokens);

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

                newNode = sakuraX_makeNode(SAKURA_NODE_BINARY_OPERATION);
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

    LOG_POP();
    return left;
}

struct Node *sakuraX_parseFactor(SakuraState *S, struct TokenStack *tokens) {
    struct Token *token;

    LOG_CALL();

    token = sakuraX_popTokStack(tokens);
    if (token->type == SAKURA_TOKEN_NUMBER) {
        struct Node *node = sakuraX_makeNode(SAKURA_TOKEN_NUMBER);
        char *tokStr;

        node->token = token;

        tokStr = (char *)malloc(token->length + 1);
        memcpy(tokStr, token->start, token->length);
        tokStr[token->length] = '\0';
        node->storageValue = strtod(tokStr, NULL);
        free(tokStr);

        LOG_POP();
        return node;
    } else if (token->type == SAKURA_TOKEN_STRING) {
        struct Node *node = sakuraX_makeNode(SAKURA_TOKEN_STRING);
        node->token = token;
        LOG_POP();
        return node;
    } else if (token->type == SAKURA_TOKEN_BANG) {
        struct Node *op = sakuraX_parseUnary(S, token, sakuraX_parseFactor(S, tokens));
        LOG_POP();
        return op;
    } else if (token->type == SAKURA_TOKEN_PLUS || token->type == SAKURA_TOKEN_MINUS) {
        struct Node *op = sakuraX_parseUnary(S, token, sakuraX_parseFactor(S, tokens));
        LOG_POP();
        return op;
    } else if (token->type == SAKURA_TOKEN_LEFT_BRACE) {
        // parse table
        struct Node *node = sakuraX_makeNode(SAKURA_NODE_TABLE);
        sakuraY_freeToken(token);

        while ((token = sakuraX_peekTokStack(tokens, 1))->type != SAKURA_TOKEN_RIGHT_BRACE) {
            struct Node *key = NULL, *value = NULL;
            if (token->type == SAKURA_TOKEN_LEFT_SQUARE) {
                // get key
                sakuraY_freeToken(sakuraX_popTokStack(tokens));
                key = sakuraX_parseExpressionEntry(S, tokens);
                if (key == NULL) {
                    printf("Error: could not parse key\n");
                    sakuraY_freeNode(node);
                    LOG_POP();
                    return NULL;
                }

                if (sakuraX_peekTokStack(tokens, 1)->type != SAKURA_TOKEN_RIGHT_SQUARE) {
                    printf("Error: expected ']'\n");
                    sakuraY_freeNode(node);
                    sakuraY_freeNode(key);
                    sakuraY_freeToken(token);
                    LOG_POP();
                    return NULL;
                }

                sakuraY_freeToken(sakuraX_popTokStack(tokens));

                if (sakuraX_peekTokStack(tokens, 1)->type != SAKURA_TOKEN_EQUAL) {
                    printf("Error: expected '='\n");
                    sakuraY_freeNode(node);
                    sakuraY_freeNode(key);
                    sakuraY_freeToken(token);
                    LOG_POP();
                    return NULL;
                }

                sakuraY_freeToken(sakuraX_popTokStack(tokens));
            }

            value = sakuraX_parseExpressionEntry(S, tokens);
            node->keys = (struct Node **)realloc(node->keys, (node->argCount + 1) * sizeof(struct Node *));
            node->args = (struct Node **)realloc(node->args, (node->argCount + 1) * sizeof(struct Node *));
            node->keys[node->argCount] = key;
            node->args[node->argCount++] = value;

            if (sakuraX_peekTokStack(tokens, 1)->type == SAKURA_TOKEN_COMMA) {
                sakuraY_freeToken(sakuraX_popTokStack(tokens));
                continue;
            } else {
                if (sakuraX_peekTokStack(tokens, 1)->type == SAKURA_TOKEN_RIGHT_BRACE) {
                    sakuraY_freeToken(sakuraX_popTokStack(tokens));
                    break;
                } else {
                    printf("Error: expected ',' or '}', got '%.*s' (%d)\n",
                           (int)sakuraX_peekTokStack(tokens, 1)->length, sakuraX_peekTokStack(tokens, 1)->start,
                           sakuraX_peekTokStack(tokens, 1)->type);
                    sakuraY_freeNode(node);
                    sakuraY_freeToken(token);
                    LOG_POP();
                    return NULL;
                }
            }
        }

        LOG_POP();
        return node;
    } else if (token->type == SAKURA_TOKEN_LEFT_PAREN) {
        struct Node *node = sakuraX_parseExpressionEntry(S, tokens);
        sakuraY_freeToken(token);
        token = sakuraX_popTokStack(tokens);
        if (token->type != SAKURA_TOKEN_RIGHT_PAREN) {
            printf("Error: expected ')'\n");
            sakuraY_freeNode(node);
            sakuraY_freeToken(token);
            LOG_POP();
            return NULL;
        }

        sakuraY_freeToken(token);
        LOG_POP();
        return node;
    } else if (token->type == SAKURA_TOKEN_IDENTIFIER) {
        struct Node *node = sakuraX_makeNode(SAKURA_TOKEN_IDENTIFIER);
        node->token = token;
        LOG_POP();
        return node;
    } else {
        printf("Error: unexpected token '%.*s' while factoring\n", (int)token->length, token->start);
        sakuraY_freeToken(token);
        LOG_POP();
        return NULL;
    }
}

struct Node *sakuraX_parseAtom(SakuraState *S, struct TokenStack *tokens) {
    enum TokenType ops[] = {SAKURA_TOKEN_CARET, SAKURA_TOKEN_SENTINEL};
    struct Node *node;
    LOG_CALL();
    node = sakuraX_binaryOperation(S, tokens, ops, sakuraX_parseFactor);
    LOG_POP();
    return node;
}

struct Node *sakuraX_parseTerm(SakuraState *S, struct TokenStack *tokens) {
    enum TokenType ops[] = {SAKURA_TOKEN_STAR, SAKURA_TOKEN_SLASH, SAKURA_TOKEN_PERCENT, SAKURA_TOKEN_SENTINEL};
    struct Node *node;
    LOG_CALL();
    node = sakuraX_binaryOperation(S, tokens, ops, sakuraX_parseAtom);
    LOG_POP();
    return node;
}

struct Node *sakuraX_parseExpression(SakuraState *S, struct TokenStack *tokens) {
    enum TokenType ops[] = {SAKURA_TOKEN_PLUS, SAKURA_TOKEN_MINUS, SAKURA_TOKEN_SENTINEL};
    struct Node *node;
    LOG_CALL();
    node = sakuraX_binaryOperation(S, tokens, ops, sakuraX_parseTerm);
    LOG_POP();
    return node;
}

struct Node *sakuraX_parseComparisons(SakuraState *S, struct TokenStack *tokens) {
    enum TokenType ops[] = {SAKURA_TOKEN_EQUAL_EQUAL, SAKURA_TOKEN_BANG_EQUAL, SAKURA_TOKEN_GREATER,
                            SAKURA_TOKEN_GREATER_EQUAL, SAKURA_TOKEN_LESS, SAKURA_TOKEN_LESS_EQUAL,
                            SAKURA_TOKEN_SENTINEL};
    struct Node *node;
    LOG_CALL();
    node = sakuraX_binaryOperation(S, tokens, ops, sakuraX_parseExpression);
    LOG_POP();
    return node;
}

struct Node *sakuraX_parseLogical(SakuraState *S, struct TokenStack *tokens) {
    enum TokenType ops[] = {SAKURA_TOKEN_AND, SAKURA_TOKEN_OR, SAKURA_TOKEN_SENTINEL};
    struct Node *node;
    LOG_CALL();
    node = sakuraX_binaryOperation(S, tokens, ops, sakuraX_parseComparisons);
    LOG_POP();
    return node;
}

struct Node *sakuraX_parseVar(SakuraState *S, struct TokenStack *tokens) {
    struct Token *token;
    struct Node *node;

    LOG_CALL();

    token = sakuraX_peekTokStack(tokens, 1);
    if (token->type == SAKURA_TOKEN_IDENTIFIER && str_cmp_cl(token->start, token->length, "let") == 0) {
        struct Token *name, *equal;
        struct Node *value;

        sakuraY_freeToken(sakuraX_popTokStack(tokens));
        name = sakuraX_popTokStack(tokens);
        if (name == NULL || name->type != SAKURA_TOKEN_IDENTIFIER) {
            printf("Error: expected identifier\n");
            sakuraY_freeToken(name);
            LOG_POP();
            return NULL;
        }

        node = sakuraX_makeNode(SAKURA_NODE_VAR);
        node->token = name;

        equal = sakuraX_popTokStack(tokens);
        if (equal == NULL || equal->type != SAKURA_TOKEN_EQUAL) {
            printf("Error: expected '='\n");
            sakuraY_freeToken(equal);
            sakuraY_freeNode(node);
            LOG_POP();
            return NULL;
        }

        sakuraY_freeToken(equal);

        value = sakuraX_parseExpressionEntry(S, tokens);
        if (value == NULL) {
            printf("Error: could not parse value\n");
            sakuraY_freeNode(node);
            LOG_POP();
            return NULL;
        }

        node->left = value;
        LOG_POP();
        return node;
    }

    node = sakuraX_parseLogical(S, tokens);
    LOG_POP();
    return node;
}

struct Node *sakuraX_parseExpressionEntry(SakuraState *S, struct TokenStack *tokens) {
    struct Node *val;
    LOG_CALL();
    val = sakuraX_parseVar(S, tokens);
    LOG_POP();
    return val;
}

struct Node *sakuraX_parseBlocks(SakuraState *S, struct TokenStack *tokens) {
    struct Token *token;
    struct Node *retNode;

    LOG_CALL();

    token = sakuraX_peekTokStack(tokens, 1);
    if (token->type == SAKURA_TOKEN_IDENTIFIER && str_cmp_cl(token->start, token->length, "if") == 0) {
        struct Node *node = sakuraX_makeNode(SAKURA_NODE_IF), *condition, *block;
        struct Token *elseToken;

        sakuraY_freeToken(sakuraX_popTokStack(tokens));
        condition = sakuraX_parseExpressionEntry(S, tokens);
        if (condition == NULL) {
            printf("Error: could not parse condition\n");
            sakuraY_freeNode(node);
            LOG_POP();
            return NULL;
        }

        node->left = condition;

        block = sakuraX_parseBlocks(S, tokens);
        if (block == NULL) {
            printf("Error: could not parse block\n");
            sakuraY_freeNode(node);
            LOG_POP();
            return NULL;
        }

        node->right = block;

        elseToken = sakuraX_peekTokStack(tokens, 1);
        if (elseToken != NULL && elseToken->type == SAKURA_TOKEN_IDENTIFIER &&
            str_cmp_cl(elseToken->start, elseToken->length, "else") == 0) {
            struct Node *elseBlock;

            sakuraY_freeToken(sakuraX_popTokStack(tokens));
            elseBlock = sakuraX_parseBlocks(S, tokens);
            if (elseBlock == NULL) {
                printf("Error: could not parse else block\n");
                sakuraY_freeNode(node);
                LOG_POP();
                return NULL;
            }

            node->elseBlock = elseBlock;
        } else {
            node->elseBlock = NULL;
        }

        LOG_POP();
        return node;
    } else if (token->type == SAKURA_TOKEN_IDENTIFIER && str_cmp_cl(token->start, token->length, "while") == 0) {
        struct Node *node = sakuraX_makeNode(SAKURA_NODE_WHILE), *condition, *block;

        sakuraY_freeToken(sakuraX_popTokStack(tokens));
        condition = sakuraX_parseExpressionEntry(S, tokens);
        if (condition == NULL) {
            printf("Error: could not parse condition\n");
            sakuraY_freeNode(node);
            LOG_POP();
            return NULL;
        }

        node->left = condition;

        block = sakuraX_parseBlocks(S, tokens);
        if (block == NULL) {
            printf("Error: could not parse block\n");
            sakuraY_freeNode(node);
            LOG_POP();
            return NULL;
        }

        node->right = block;

        LOG_POP();
        return node;
    } else if (token->type == SAKURA_TOKEN_IDENTIFIER && str_cmp_cl(token->start, token->length, "loop") == 0) {
        struct Node *node = sakuraX_makeNode(SAKURA_NODE_WHILE), *block;
        node->left = NULL;

        sakuraY_freeToken(sakuraX_popTokStack(tokens));

        block = sakuraX_parseBlocks(S, tokens);
        if (block == NULL) {
            printf("Error: could not parse block\n");
            sakuraY_freeNode(node);
            LOG_POP();
            return NULL;
        }

        node->right = block;

        LOG_POP();
        return node;
    } else if (token->type == SAKURA_TOKEN_IDENTIFIER && str_cmp_cl(token->start, token->length, "fn") == 0) {
        struct Node *node = sakuraX_makeNode(SAKURA_NODE_FUNCTION), *block;
        struct Token *name, *leftParen, *arg, *comma;

        sakuraY_freeToken(sakuraX_popTokStack(tokens));
        name = sakuraX_popTokStack(tokens);
        if (name == NULL || name->type != SAKURA_TOKEN_IDENTIFIER) {
            printf("Error: expected identifier, got %d\n", name->type);
            sakuraY_freeToken(name);
            LOG_POP();
            return NULL;
        }

        node->token = name;

        leftParen = sakuraX_popTokStack(tokens);
        if (leftParen == NULL || leftParen->type != SAKURA_TOKEN_LEFT_PAREN) {
            printf("Error: expected '('\n");
            sakuraY_freeToken(leftParen);
            sakuraY_freeNode(node);
            LOG_POP();
            return NULL;
        }

        sakuraY_freeToken(leftParen);

        while (1) {
            arg = sakuraX_popTokStack(tokens);
            if (arg != NULL && arg->type == SAKURA_TOKEN_IDENTIFIER) {
                node->args = (struct Node **)realloc(node->args, (node->argCount + 1) * sizeof(struct Node *));
                node->args[node->argCount++] = sakuraX_makeNode(SAKURA_TOKEN_IDENTIFIER);
                node->args[node->argCount - 1]->token = arg;

                comma = sakuraX_popTokStack(tokens);
                if (comma != NULL && comma->type == SAKURA_TOKEN_COMMA) {
                    sakuraY_freeToken(comma);
                    continue;
                } else {
                    if (comma != NULL && comma->type == SAKURA_TOKEN_RIGHT_PAREN) {
                        sakuraY_freeToken(comma);
                        break;
                    } else {
                        printf("Error: expected ',' or ')'\n");
                        sakuraY_freeNode(node);
                        sakuraY_freeToken(comma);
                        LOG_POP();
                        return NULL;
                    }
                }
            } else {
                if (arg->type == SAKURA_TOKEN_RIGHT_PAREN) {
                    sakuraY_freeToken(arg);
                    break;
                }

                printf("Error: expected identifier\n");
                sakuraY_freeNode(node);
                sakuraY_freeToken(arg);
                LOG_POP();
                return NULL;
            }
        }

        block = sakuraX_parseBlocks(S, tokens);
        if (block == NULL) {
            printf("Error: could not parse block\n");
            sakuraY_freeNode(node);
            LOG_POP();
            return NULL;
        }

        node->left = block;

        LOG_POP();
        return node;
    } else if (token->type == SAKURA_TOKEN_LEFT_BRACE) {
        struct Node *node = sakuraX_makeNode(SAKURA_NODE_BLOCK), *block;

        sakuraY_freeToken(sakuraX_popTokStack(tokens));

        while (sakuraX_peekTokStack_s(tokens)->type != SAKURA_TOKEN_RIGHT_BRACE) {
            block = sakuraX_parseBlocks(S, tokens);
            if (block != NULL) {
                node->args = (struct Node **)realloc(node->args, (node->argCount + 1) * sizeof(struct Node *));
                node->args[node->argCount++] = block;
            } else {
                printf("Error: could not parse block\n");
                sakuraY_freeNode(node);
                LOG_POP();
                return NULL;
            }
        }

        sakuraY_freeToken(sakuraX_popTokStack(tokens));

        LOG_POP();
        return node;
    }

    retNode = sakuraX_parseExpressionEntry(S, tokens);
    LOG_POP();
    return retNode;
}

// let x = {1, 2, [3] = 3}

struct Node *sakuraX_parseCall(SakuraState *S, struct Node *prev, struct TokenStack *tokens) {
    struct Node *node;
    struct Token *peeked;
    int hasRparen = 0;

    LOG_CALL();

    node = sakuraX_makeNode(SAKURA_NODE_CALL);
    node->left = prev;

    sakuraY_freeToken(sakuraX_popTokStack(tokens));
    while (sakuraX_peekTokStack(tokens, 1)->type != SAKURA_TOKEN_RIGHT_PAREN) {
        struct Node *arg = sakuraX_parseExpressionEntry(S, tokens);
        if (arg != NULL) {
            struct Token *comma;

            node->args = (struct Node **)realloc(node->args, (node->argCount + 1) * sizeof(struct Node *));
            node->args[node->argCount++] = arg;

            comma = sakuraX_popTokStack(tokens);
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
                    LOG_POP();
                    return NULL;
                }
            }

            break;
        } else {
            printf("Error: could not parse argument\n");
            sakuraY_freeNode(node);
            LOG_POP();
            return NULL;
        }
    }

    if (!hasRparen) {
        struct Token *rightParen = sakuraX_popTokStack(tokens);
        if (rightParen == NULL || rightParen->type != SAKURA_TOKEN_RIGHT_PAREN) {
            printf("Error: expected ')'\n");
            sakuraY_freeNode(node);
            sakuraY_freeToken(rightParen);
            LOG_POP();
            return NULL;
        }

        sakuraY_freeToken(rightParen);
    }

    peeked = sakuraX_peekTokStack(tokens, 1);
    if (peeked != NULL && peeked->type == SAKURA_TOKEN_LEFT_PAREN) {
        LOG_POP();
        return sakuraX_parseCall(S, node, tokens);
    } else if (peeked != NULL && peeked->type == SAKURA_TOKEN_LEFT_SQUARE) {
        LOG_POP();
        return sakuraX_parseIndex(S, node, tokens);
    }

    LOG_POP();
    return node;
}

struct Node *sakuraX_parseIndex(SakuraState *S, struct Node *prev, struct TokenStack *tokens) {
    struct Node *node, *idx;
    struct Token *peeked;

    LOG_CALL();

    node = sakuraX_makeNode(SAKURA_NODE_INDEX);
    node->left = prev;

    sakuraY_freeToken(sakuraX_popTokStack(tokens));
    idx = sakuraX_parseExpressionEntry(S, tokens);

    if (idx == NULL) {
        printf("Error: could not parse index\n");
        sakuraY_freeNode(node);
        LOG_POP();
        return NULL;
    }

    node->right = idx;

    if (sakuraX_peekTokStack_s(tokens)->type != SAKURA_TOKEN_RIGHT_SQUARE) {
        printf("Error: expected ']'\n");
        sakuraY_freeNode(node);
        LOG_POP();
        return NULL;
    }

    sakuraY_freeToken(sakuraX_popTokStack(tokens));

    peeked = sakuraX_peekTokStack(tokens, 1);
    if (peeked != NULL && peeked->type == SAKURA_TOKEN_LEFT_PAREN) {
        LOG_POP();
        return sakuraX_parseCall(S, node, tokens);
    } else if (peeked != NULL && peeked->type == SAKURA_TOKEN_LEFT_SQUARE) {
        LOG_POP();
        return sakuraX_parseIndex(S, node, tokens);
    }

    LOG_POP();
    return node;
}

struct Node *sakuraX_parseExecution(SakuraState *S, struct TokenStack *tokens) {
    struct Node *block;
    struct Token *nextToken;

    LOG_CALL();

    block = sakuraX_parseBlocks(S, tokens);
    nextToken = sakuraX_peekTokStack(tokens, 1);

    if (nextToken != NULL && nextToken->type == SAKURA_TOKEN_LEFT_PAREN) {
        // call operator
        LOG_POP();
        return sakuraX_parseCall(S, block, tokens);
    } else if (nextToken != NULL && nextToken->type == SAKURA_TOKEN_LEFT_SQUARE) {
        // index operator
        LOG_POP();
        return sakuraX_parseIndex(S, block, tokens);
    }

    LOG_POP();
    return block;
}

struct NodeStack *sakuraY_parse(SakuraState *S, struct TokenStack *tokens) {
    struct NodeStack *stack;
    struct Node *node;

    LOG_CALL();

    stack = sakuraX_newNodeStack();
    S->currentState = SAKURA_FLAG_PARSER;

    while (tokens->size > 0) {
        node = sakuraX_parseExecution(S, tokens);

        if (node != NULL) {
            sakuraX_pushNodeStack(stack, node);
        } else {
            printf("Error: could not parse expression\n");
            sakuraX_freeNodeStack(stack);
            stack = NULL;
            break;
        }
    }

    LOG_POP();
    return stack;
}

void sakuraY_freeToken(struct Token *token) {
    LOG_CALL();

    free(token);
    token = NULL;

    LOG_POP();
}

void sakuraY_freeNode(struct Node *node) {
    LOG_CALL();

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
            for (ull i = 0; i < node->argCount; i++) {
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

    LOG_POP();
}
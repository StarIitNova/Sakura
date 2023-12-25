#include "assembler.h"

#include <stdlib.h>

void sakuraV_visitNode(SakuraState *S, struct SakuraAssembly *assembly, struct Node *node);

void sakuraV_visitUnary(SakuraState *S, struct SakuraAssembly *assembly, struct Node *node) {
    // visit the operand
    sakuraV_visitNode(S, assembly, node->left);

    // determine the specific unary operation
    if (node->token->type == SAKURA_TOKEN_MINUS) {
        // negate the value, storing it back in it's original register
        SakuraAssembly_push3(assembly, SAKURA_UNM, node->leftLocation, node->leftLocation);
    }
    // TODO: add more cases as needed
    // ignore '+' case as it does not affect the value
}

void sakuraV_visitBinary(SakuraState *S, struct SakuraAssembly *assembly, struct Node *node) {
    // visit the operands
    sakuraV_visitNode(S, assembly, node->left);
    sakuraV_visitNode(S, assembly, node->right);

    node->leftLocation = assembly->registers++;

    // determine the specific binary operation
    if (node->token->type == SAKURA_TOKEN_PLUS) {
        // add the values, storing the result in the left register
        SakuraAssembly_push4(assembly, SAKURA_ADD, node->leftLocation, node->left->leftLocation,
                             node->right->leftLocation);
    } else if (node->token->type == SAKURA_TOKEN_MINUS) {
        // subtract the values, storing the result in the left register
        SakuraAssembly_push4(assembly, SAKURA_SUB, node->leftLocation, node->left->leftLocation,
                             node->right->leftLocation);
    } else if (node->token->type == SAKURA_TOKEN_STAR) {
        // multiply the values, storing the result in the left register
        SakuraAssembly_push4(assembly, SAKURA_MUL, node->leftLocation, node->left->leftLocation,
                             node->right->leftLocation);
    } else if (node->token->type == SAKURA_TOKEN_SLASH) {
        // divide the values, storing the result in the left register
        SakuraAssembly_push4(assembly, SAKURA_DIV, node->leftLocation, node->left->leftLocation,
                             node->right->leftLocation);
    } else {
        printf("Error: unknown binary operation '%d'\n", node->token->type);
    }
}

void SakuraV_visitNumber(SakuraState *S, struct SakuraAssembly *assembly, struct Node *node) {
    // convert the string in the token to a double
    char *tokStr = (char *)malloc(node->token->length + 1);
    memcpy(tokStr, node->token->start, node->token->length);
    tokStr[node->token->length] = '\0';
    double value = strtod(tokStr, NULL);
    free(tokStr);

    // store the value in the constant pool
    int index = sakuraX_pushKNumber(assembly, value);
    // load the value into the next register (note this is actually the stack, this value is NOT used)
    size_t reg = assembly->registers++;
    SakuraAssembly_push3(assembly, SAKURA_LOADK, reg, index);
    // store the register location in the node
    node->leftLocation = reg;
}

void sakuraV_visitNode(SakuraState *S, struct SakuraAssembly *assembly, struct Node *node) {
    switch (node->type) {
    case SAKURA_NODE_UNARY_OPERATION:
        sakuraV_visitUnary(S, assembly, node);
        break;
    case SAKURA_NODE_BINARY_OPERATION:
        sakuraV_visitBinary(S, assembly, node);
        break;
    case SAKURA_TOKEN_NUMBER:
        SakuraV_visitNumber(S, assembly, node);
        break;
    default:
        printf("Error: unknown node type '%d'\n", node->type);
        break;
    }
}

struct SakuraAssembly *sakuraY_assemble(SakuraState *S, struct NodeStack *nodes) {
    S->currentState = SAKURA_FLAG_ASSEMBLING;
    struct SakuraAssembly *assembly = SakuraAssembly();

    for (size_t i = 0; i < nodes->size; i++) {
        struct Node *node = nodes->nodes[i];
        sakuraV_visitNode(S, assembly, node);
    }

    return assembly;
}

struct SakuraAssembly *SakuraAssembly() {
    struct SakuraAssembly *assembly = malloc(sizeof(struct SakuraAssembly));
    assembly->instructions = malloc(sizeof(int) * 64);
    assembly->size = 0;
    assembly->capacity = 64;

    assembly->registers = 0;

    assembly->pool.size = 0;
    assembly->pool.capacity = 8;
    assembly->pool.constants = malloc(assembly->pool.capacity * sizeof(TValue));
    return assembly;
}

void sakuraX_freeAssembly(struct SakuraAssembly *assembly) {
    if (assembly->pool.constants) {
        free(assembly->pool.constants);
        assembly->pool.constants = NULL;
        assembly->pool.size = 0;
    }

    if (assembly->instructions) {
        free(assembly->instructions);
        assembly->instructions = NULL;
    }

    free(assembly);
}

void SakuraAssembly_push(struct SakuraAssembly *assembly, int instruction) {
    if (assembly->size >= assembly->capacity) {
        assembly->capacity *= 2;
        assembly->instructions = realloc(assembly->instructions, assembly->capacity);
    }
    assembly->instructions[assembly->size++] = instruction;
}

void SakuraAssembly_push2(struct SakuraAssembly *assembly, int instruction, int a) {
    SakuraAssembly_push(assembly, instruction);
    SakuraAssembly_push(assembly, a);
}

void SakuraAssembly_push3(struct SakuraAssembly *assembly, int instruction, int a, int b) {
    SakuraAssembly_push(assembly, instruction);
    SakuraAssembly_push2(assembly, a, b);
}

void SakuraAssembly_push4(struct SakuraAssembly *assembly, int instruction, int a, int b, int c) {
    SakuraAssembly_push2(assembly, instruction, a);
    SakuraAssembly_push2(assembly, b, c);
}

int sakuraX_pushKNumber(struct SakuraAssembly *assembly, double value) {
    if (assembly->pool.size >= assembly->pool.capacity) {
        assembly->pool.capacity *= 2;
        assembly->pool.constants = realloc(assembly->pool.constants, assembly->pool.capacity);
    }

    size_t idx = assembly->pool.size;
    assembly->pool.constants[assembly->pool.size++] = (TValue){.value.n = value, .tt = SAKURA_TNUMFLT};
    return -(idx + 1);
}

int sakuraX_pushKString(struct SakuraAssembly *assembly, const struct s_str *value) {
    if (assembly->pool.size >= assembly->pool.capacity) {
        assembly->pool.capacity *= 2;
        assembly->pool.constants = realloc(assembly->pool.constants, assembly->pool.capacity);
    }

    size_t idx = assembly->pool.size;
    assembly->pool.constants[assembly->pool.size++] = (TValue){.value.s = s_str_copy(value), .tt = SAKURA_TSTR};
    return -(idx + 1);
}
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
    // visit the operands, swapped order is so the less than/greater than system works properly
    if (node->token->type == SAKURA_TOKEN_GREATER || node->token->type == SAKURA_TOKEN_GREATER_EQUAL) {
        sakuraV_visitNode(S, assembly, node->right);
        sakuraV_visitNode(S, assembly, node->left);
    } else {
        sakuraV_visitNode(S, assembly, node->left);
        sakuraV_visitNode(S, assembly, node->right);
    }

    assembly->registers -= 2;

    node->leftLocation = assembly->registers++;

    // determine the specific binary operation
    switch (node->token->type) {
    case SAKURA_TOKEN_PLUS:
        // add the values, storing the result in the left register
        SakuraAssembly_push4(assembly, SAKURA_ADD, node->leftLocation, node->left->leftLocation,
                             node->right->leftLocation);
        break;
    case SAKURA_TOKEN_MINUS:
        // subtract the values, storing the result in the left register
        SakuraAssembly_push4(assembly, SAKURA_SUB, node->leftLocation, node->left->leftLocation,
                             node->right->leftLocation);
        break;
    case SAKURA_TOKEN_STAR:
        // multiply the values, storing the result in the left register
        SakuraAssembly_push4(assembly, SAKURA_MUL, node->leftLocation, node->left->leftLocation,
                             node->right->leftLocation);
        break;
    case SAKURA_TOKEN_SLASH:
        // divide the values, storing the result in the left register
        SakuraAssembly_push4(assembly, SAKURA_DIV, node->leftLocation, node->left->leftLocation,
                             node->right->leftLocation);
        break;
    case SAKURA_TOKEN_CARET:
        // power the values, storing the result in the left register
        SakuraAssembly_push4(assembly, SAKURA_POW, node->leftLocation, node->left->leftLocation,
                             node->right->leftLocation);
        break;
    case SAKURA_TOKEN_PERCENT:
        // mod the values, storing the result in the left register
        SakuraAssembly_push4(assembly, SAKURA_MOD, node->leftLocation, node->left->leftLocation,
                             node->right->leftLocation);
        break;
    case SAKURA_TOKEN_EQUAL_EQUAL:
        // check if the values are equal, storing the result in the left register
        SakuraAssembly_push4(assembly, SAKURA_EQ, node->leftLocation, node->left->leftLocation,
                             node->right->leftLocation);
        break;
    case SAKURA_TOKEN_BANG_EQUAL:
        // check if the values are not equal, storing the result in the left register
        SakuraAssembly_push4(assembly, SAKURA_EQ, node->leftLocation, node->left->leftLocation,
                             node->right->leftLocation);
        // invert the result, storing it in the left register
        SakuraAssembly_push3(assembly, SAKURA_NOT, node->leftLocation, node->leftLocation);
        break;
    case SAKURA_TOKEN_LESS:
    case SAKURA_TOKEN_GREATER:
        // check if the left value is less than the right value, storing the result in the left register
        SakuraAssembly_push4(assembly, SAKURA_LT, node->leftLocation, node->left->leftLocation,
                             node->right->leftLocation);
        break;
    case SAKURA_TOKEN_LESS_EQUAL:
    case SAKURA_TOKEN_GREATER_EQUAL:
        // check if the left value is less than or equal to the right value, storing the result in the left register
        SakuraAssembly_push4(assembly, SAKURA_LE, node->leftLocation, node->left->leftLocation,
                             node->right->leftLocation);
        break;
    default:
        printf("Error: unknown binary operation '%d' in node '%d' ('%.*s' = ?)\n", node->token->type, node->type,
               node->token->length, node->token->start);
        break;
    }
}

void sakuraV_visitNumber(SakuraState *S, struct SakuraAssembly *assembly, struct Node *node) {
    // store the value in the constant pool
    int index = sakuraX_pushKNumber(assembly, node->storageValue);
    // load the value into the next register (note this is actually the stack, this value is NOT used)
    size_t reg = assembly->registers++;
    SakuraAssembly_push3(assembly, SAKURA_LOADK, reg, index);
    // store the register location in the node
    node->leftLocation = reg;

    if (reg >= assembly->highestRegister) {
        assembly->highestRegister = reg;
    }
}

void sakuraV_visitIdentifier(SakuraState *S, struct SakuraAssembly *assembly, struct Node *node) {
    // get the variable name as an s_str
    struct s_str name;
    name.str = (char *)node->token->start;
    name.len = node->token->length;

    // get the variable from the globals table
    TValue *val = sakuraX_TVMapGet(&S->globals, &name);
    int idx = sakuraX_TVMapGetIndex(&S->globals, &name);

    // check if the variable exists
    if (idx != -1) {
        // load the value into the next register
        size_t reg = assembly->registers++;
        SakuraAssembly_push3(assembly, SAKURA_GETGLOBAL, reg, idx);
        // store the register location in the node
        node->leftLocation = reg;

        if (reg >= assembly->highestRegister) {
            assembly->highestRegister = reg;
        }
    } else {
        // check the locals table (user created variable)
        for (size_t i = 0; i < S->localsSize; i++) {
            if (s_str_cmp(&name, &S->locals[i]) == 0) {
                // load the value into the next register
                size_t reg = assembly->registers++;
                SakuraAssembly_push3(assembly, SAKURA_MOVE, reg, i);
                // store the register location in the node
                node->leftLocation = reg;

                if (reg >= assembly->highestRegister) {
                    assembly->highestRegister = reg;
                }

                return;
            }
        }
    }
}

void sakuraV_visitCall(SakuraState *S, struct SakuraAssembly *assembly, struct Node *node) {
    // get the function name as an s_str
    struct s_str name;
    name.str = (char *)node->token->start;
    name.len = node->token->length;

    // get the function from the global table
    TValue *func = sakuraX_TVMapGet(&S->globals, &name);
    int idx = sakuraX_TVMapGetIndex(&S->globals, &name);

    // check if the function exists
    if (idx == -1) {
        printf("Error: function '%.*s' does not exist\n", name.len, name.str);
        return;
    }

    // check if the function is a function
    if (func->tt != SAKURA_TCFUNC && func->tt != SAKURA_TFUNC) {
        printf("Error: '%.*s' is not a function\n", name.len, name.str);
        return;
    }

    // bytecode to call the function
    size_t reg = assembly->registers++;
    assembly->functionsLoaded++;
    SakuraAssembly_push3(assembly, SAKURA_GETGLOBAL, reg, idx);

    // bytecode to push arguments onto the stack
    for (size_t i = 0; i < node->argCount; i++) {
        sakuraV_visitNode(S, assembly, node->args[i]);
    }

    SakuraAssembly_push3(assembly, SAKURA_CALL, reg, node->argCount);
    assembly->registers -= (node->argCount + 1);

    if (reg >= assembly->highestRegister) {
        assembly->highestRegister = reg;
    }
}

void sakuraV_visitIf(SakuraState *S, struct SakuraAssembly *assembly, struct Node *node) {
    // visit the condition
    sakuraV_visitNode(S, assembly, node->left);

    // bytecode to jump to the else block if the condition is false
    size_t jump = assembly->size;
    SakuraAssembly_push3(assembly, SAKURA_JMPIF, 0, node->left->leftLocation);
    assembly->registers--;

    // visit the if block
    sakuraV_visitNode(S, assembly, node->right);

    // check if theres an else block
    if (node->right != NULL) {
        // bytecode to jump to the end of the if statement
        size_t end = assembly->size;
        SakuraAssembly_push2(assembly, SAKURA_JMP, 0);

        // set the jump location
        assembly->instructions[jump + 1] = assembly->size;

        // visit the else block
        sakuraV_visitNode(S, assembly, node->elseBlock);

        // set the end location
        assembly->instructions[end + 1] = assembly->size;
    } else {
        // set the jump location
        assembly->instructions[jump + 1] = assembly->size;
    }
}

void sakuraV_visitWhile(SakuraState *S, struct SakuraAssembly *assembly, struct Node *node) {
    size_t start = assembly->size;
    size_t jump = 0;
    if (node->left != NULL) {
        // visit the condition
        sakuraV_visitNode(S, assembly, node->left);

        // bytecode to jump to the else block if the condition is false
        jump = assembly->size;
        SakuraAssembly_push3(assembly, SAKURA_JMPIF, 0, node->left->leftLocation);
        assembly->registers--;
    }

    // visit the while block
    sakuraV_visitNode(S, assembly, node->right);

    // bytecode to jump to the start of the while loop
    SakuraAssembly_push2(assembly, SAKURA_JMP, start);

    // set the jump location
    if (node->left != NULL)
        assembly->instructions[jump + 1] = assembly->size;
}

void sakuraV_visitBlock(SakuraState *S, struct SakuraAssembly *assembly, struct Node *node) {
    for (size_t i = 0; i < node->argCount; i++) {
        sakuraV_visitNode(S, assembly, node->args[i]);
    }
}

void sakuraV_visitVar(SakuraState *S, struct SakuraAssembly *assembly, struct Node *node) {
    // get the variable name as an s_str
    struct s_str name;
    name.str = (char *)node->token->start;
    name.len = node->token->length;

    // push the value onto the stack
    size_t reg = assembly->registers;
    sakuraV_visitNode(S, assembly, node->left);

    // store the register location in the node
    sakuraY_storeLocal(S, &name, reg);
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
        sakuraV_visitNumber(S, assembly, node);
        break;
    case SAKURA_NODE_CALL:
        sakuraV_visitCall(S, assembly, node);
        break;
    case SAKURA_NODE_BLOCK:
        sakuraV_visitBlock(S, assembly, node);
        break;
    case SAKURA_NODE_IF:
        sakuraV_visitIf(S, assembly, node);
        break;
    case SAKURA_NODE_WHILE:
        sakuraV_visitWhile(S, assembly, node);
        break;
    case SAKURA_NODE_VAR:
        sakuraV_visitVar(S, assembly, node);
        break;
    case SAKURA_TOKEN_IDENTIFIER:
        sakuraV_visitIdentifier(S, assembly, node);
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

    SakuraAssembly_push3(assembly, SAKURA_RETURN, 0, 0);

    return assembly;
}

struct SakuraAssembly *SakuraAssembly() {
    struct SakuraAssembly *assembly = malloc(sizeof(struct SakuraAssembly));
    assembly->instructions = malloc(64 * sizeof(int));
    assembly->size = 0;
    assembly->capacity = 64;

    assembly->registers = 0;

    assembly->highestRegister = 0;
    assembly->functionsLoaded = 0;

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
        assembly->instructions = realloc(assembly->instructions, assembly->capacity * sizeof(int));
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
        assembly->pool.constants = realloc(assembly->pool.constants, assembly->pool.capacity * sizeof(TValue));
    }

    size_t idx = assembly->pool.size;
    assembly->pool.constants[assembly->pool.size++] = (TValue){.value.n = value, .tt = SAKURA_TNUMFLT};
    return -(idx + 1);
}

int sakuraX_pushKString(struct SakuraAssembly *assembly, const struct s_str *value) {
    if (assembly->pool.size >= assembly->pool.capacity) {
        assembly->pool.capacity *= 2;
        assembly->pool.constants = realloc(assembly->pool.constants, assembly->pool.capacity * sizeof(TValue));
    }

    size_t idx = assembly->pool.size;
    assembly->pool.constants[assembly->pool.size++] = (TValue){.value.s = s_str_copy(value), .tt = SAKURA_TSTR};
    return -(idx + 1);
}
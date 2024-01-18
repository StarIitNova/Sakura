#include "assembler.h"

#include <stdlib.h>

void sakuraV_visitUnary(SakuraState *S, struct SakuraAssembly *assembly, struct Node *node) {
    LOG_CALL();

    // visit the operand
    sakuraV_visitNode(S, assembly, node->left);

    // determine the specific unary operation
    if (node->token->type == SAKURA_TOKEN_MINUS) {
        // negate the value, storing it back in it's original register
        SakuraAssembly_push3(assembly, SAKURA_UNM, node->leftLocation, node->leftLocation);
    }
    // TODO: add more cases as needed
    // ignore '+' case as it does not affect the value

    LOG_POP();
}

void sakuraV_visitBinary(SakuraState *S, struct SakuraAssembly *assembly, struct Node *node) {
    LOG_CALL();

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
               (int)node->token->length, node->token->start);
        break;
    }

    LOG_POP();
}

void sakuraV_visitNumber(SakuraState *S, struct SakuraAssembly *assembly, struct Node *node) {
    int index;
    ull reg;

    UNUSED(S);

    LOG_CALL();

    // store the value in the constant pool
    index = sakuraX_pushKNumber(assembly, node->storageValue);
    // load the value into the stack
    reg = assembly->registers++;

    SakuraAssembly_push3(assembly, SAKURA_LOADK, reg, index);
    // store the register location in the node
    node->leftLocation = reg;

    if (reg >= assembly->highestRegister) {
        assembly->highestRegister = reg;
    }

    LOG_POP();
}

void sakuraV_visitString(SakuraState *S, struct SakuraAssembly *assembly, struct Node *node) {
    int index;
    ull reg;
    struct s_str val;

    UNUSED(S);

    LOG_CALL();

    // store the value in the constant pool
    val.str = (char *)node->token->start;
    val.len = node->token->length;
    index = sakuraX_pushKString(assembly, &val);
    // load the value into the stack
    reg = assembly->registers++;
    SakuraAssembly_push3(assembly, SAKURA_LOADK, reg, index);
    // store the register location in the node
    node->leftLocation = reg;

    if (reg >= assembly->highestRegister) {
        assembly->highestRegister = reg;
    }

    LOG_POP();
}

void sakuraV_visitIdentifier(SakuraState *S, struct SakuraAssembly *assembly, struct Node *node) {
    struct s_str name;
    int idx;
    ull reg;

    LOG_CALL();

    // get the variable name as an s_str
    name.str = (char *)node->token->start;
    name.len = node->token->length;

    // get the variable from the globals table
    idx = sakuraX_TVMapGetIndex(&S->globals, &name);

    // check if the variable exists
    if (idx != -1) {
        // load the value into the next register
        reg = assembly->registers++;
        SakuraAssembly_push3(assembly, SAKURA_GETGLOBAL, reg, idx);
        // store the register location in the node
        node->leftLocation = reg;

        if (reg >= assembly->highestRegister) {
            assembly->highestRegister = reg;
        }
    } else {
        // check the locals table (user created variable)
        for (ull i = 0; i < S->localsSize; i++) {
            if (s_str_cmp(&name, &S->locals[i]) == 0) {
                // load the value into the next register
                reg = assembly->registers++;
                SakuraAssembly_push3(assembly, SAKURA_MOVE, reg, i);
                // store the register location in the node
                node->leftLocation = reg;

                if (reg >= assembly->highestRegister) {
                    assembly->highestRegister = reg;
                }

                LOG_POP();
                return;
            }
        }
    }

    LOG_POP();
}

void sakuraV_visitFunction(SakuraState *S, struct SakuraAssembly *assembly, struct Node *node) {
    struct SakuraAssembly *funcAssembly;
    struct s_str v;
    ull reg;
    int idx;

    LOG_CALL();

    // create a new assembly for the function
    funcAssembly = SakuraAssembly();

    // visit the function body
    sakuraV_visitNode(S, funcAssembly, node->left);

    // bytecode to return from the function
    SakuraAssembly_push3(funcAssembly, SAKURA_RETURN, 0, 0);

    // create a closure from the function
    reg = assembly->registers++;
    SakuraAssembly_push3(assembly, SAKURA_CLOSURE, reg, assembly->closureIdx);

    // bytecode to set the function name
    v.str = (char *)node->token->start;
    v.len = node->token->length;
    idx = sakuraX_pushKString(assembly, &v);
    SakuraAssembly_push3(assembly, SAKURA_SETGLOBAL, reg, idx);
    sakuraX_TVMapInsert(&S->globals, &v, sakuraY_makeTFunc(funcAssembly));
    assembly->registers--;

    // store the register location in the node
    node->leftLocation = reg;

    if (reg >= assembly->highestRegister) {
        assembly->highestRegister = reg;
    }

    SakuraAssembly_pushChildAssembly(assembly, funcAssembly);

    sakuraY_mergePoolsA(&assembly->pool, &funcAssembly->pool);

    LOG_POP();
}

void sakuraV_visitCall(SakuraState *S, struct SakuraAssembly *assembly, struct Node *node) {
    struct s_str name;
    TValue *func;
    int idx;
    ull reg;

    LOG_CALL();

    if (node->left->type == SAKURA_TOKEN_IDENTIFIER) {
        // get the function name as an s_str
        name.str = (char *)node->left->token->start;
        name.len = node->left->token->length;

        // get the function from the global table
        func = sakuraX_TVMapGet(&S->globals, &name);
        idx = sakuraX_TVMapGetIndex(&S->globals, &name);

        if (idx == -1) {
            for (ull i = 0; i < S->localsSize; i++) {
                if (s_str_cmp(&name, &S->locals[i]) == 0) {
                    // load the value into the next register
                    reg = assembly->registers++;

                    if (i != reg)
                        SakuraAssembly_push3(assembly, SAKURA_MOVE, reg, i);

                    if (reg >= assembly->highestRegister) {
                        assembly->highestRegister = reg;
                    }

                    for (ull j = 0; j < node->argCount; j++) {
                        sakuraV_visitNode(S, assembly, node->args[j]);
                    }

                    SakuraAssembly_push3(assembly, SAKURA_CALL, reg, node->argCount);
                    assembly->registers -= (node->argCount + 1);

                    LOG_POP();
                    return;
                }
            }
        }

        // check if the function exists
        if (idx == -1) {
            printf("Error: function '%.*s' does not exist\n", name.len, name.str);
            LOG_POP();
            return;
        }

        // check if the function is a function
        if (func->tt != SAKURA_TCFUNC && func->tt != SAKURA_TFUNC) {
            printf("Error: '%.*s' is not a function\n", name.len, name.str);
            LOG_POP();
            return;
        }

        // bytecode to call the function
        reg = assembly->registers++;
        assembly->functionsLoaded++;
        SakuraAssembly_push3(assembly, SAKURA_GETGLOBAL, reg, idx);

        if (reg >= assembly->highestRegister) {
            assembly->highestRegister = reg;
        }

        // bytecode to push arguments onto the stack
        for (ull i = 0; i < node->argCount; i++) {
            sakuraV_visitNode(S, assembly, node->args[i]);
        }

        SakuraAssembly_push3(assembly, SAKURA_CALL, reg, node->argCount);
        assembly->registers -= (node->argCount + 1);

        node->leftLocation = reg;
    } else {
        // visit the function
        sakuraV_visitNode(S, assembly, node->left);

        // bytecode to push arguments onto the stack
        for (ull i = 0; i < node->argCount; i++) {
            sakuraV_visitNode(S, assembly, node->args[i]);
        }

        SakuraAssembly_push3(assembly, SAKURA_CALL, node->left->leftLocation, node->argCount);
        assembly->registers -= (node->argCount + 1);
        node->leftLocation = node->left->leftLocation;
    }

    LOG_POP();
}

void sakuraV_visitIndex(SakuraState *S, struct SakuraAssembly *assembly, struct Node *node) {
    ;
}

void sakuraV_visitIf(SakuraState *S, struct SakuraAssembly *assembly, struct Node *node) {
    ull jump, end;

    LOG_CALL();

    // visit the condition
    sakuraV_visitNode(S, assembly, node->left);

    // bytecode to jump to the else block if the condition is false
    jump = assembly->size;
    SakuraAssembly_push3(assembly, SAKURA_JMPIF, 0, node->left->leftLocation);
    assembly->registers--;

    // visit the if block
    sakuraV_visitNode(S, assembly, node->right);

    // check if theres an else block
    if (node->right != NULL) {
        // bytecode to jump to the end of the if statement
        end = assembly->size;
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

    LOG_POP();
}

void sakuraV_visitWhile(SakuraState *S, struct SakuraAssembly *assembly, struct Node *node) {
    ull start = assembly->size;
    ull jump = 0;

    LOG_CALL();

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

    LOG_POP();
}

void sakuraV_visitBlock(SakuraState *S, struct SakuraAssembly *assembly, struct Node *node) {
    LOG_CALL();

    for (ull i = 0; i < node->argCount; i++) {
        sakuraV_visitNode(S, assembly, node->args[i]);
    }

    LOG_POP();
}

void sakuraV_visitVar(SakuraState *S, struct SakuraAssembly *assembly, struct Node *node) {
    struct s_str name;
    ull reg;

    LOG_CALL();

    // get the variable name as an s_str
    name.str = (char *)node->token->start;
    name.len = node->token->length;

    // push the value onto the stack
    reg = assembly->registers;
    sakuraV_visitNode(S, assembly, node->left);
    assembly->registers--;

    // store the register location in the node
    sakuraY_storeLocal(S, &name, reg);

    LOG_POP();
}

void sakuraV_visitTable(SakuraState *S, struct SakuraAssembly *assembly, struct Node *node) {
    ull reg, tblIdx, keyIdx = 0;
    int index;

    LOG_CALL();

    // create a new table
    reg = assembly->registers++;
    SakuraAssembly_push3(assembly, SAKURA_NEWTABLE, reg, node->argCount);

    // store the register location in the node
    node->leftLocation = reg;

    if (reg >= assembly->highestRegister) {
        assembly->highestRegister = reg;
    }

    // bytecode to set the table keys
    for (ull i = 0; i < node->argCount; i++) {
        if (node->keys[i] == NULL) {
            tblIdx = keyIdx++;
            // push idx as a constant and use that
            index = sakuraX_pushKNumber(assembly, tblIdx);
            sakuraV_visitNode(S, assembly, node->args[i]);
            SakuraAssembly_push4(assembly, SAKURA_SETTABLE, reg, index, node->args[i]->leftLocation);
            assembly->registers--;
        } else {
            sakuraV_visitNode(S, assembly, node->keys[i]);
            sakuraV_visitNode(S, assembly, node->args[i]);
            SakuraAssembly_push4(assembly, SAKURA_SETTABLE, reg, node->keys[i]->leftLocation,
                                 node->args[i]->leftLocation);
            assembly->registers -= 2;
        }
    }

    LOG_POP();
}

void sakuraV_visitNode(SakuraState *S, struct SakuraAssembly *assembly, struct Node *node) {
    LOG_CALL();

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
    case SAKURA_TOKEN_STRING:
        sakuraV_visitString(S, assembly, node);
        break;
    case SAKURA_NODE_CALL:
        sakuraV_visitCall(S, assembly, node);
        break;
    case SAKURA_NODE_FUNCTION:
        sakuraV_visitFunction(S, assembly, node);
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
    case SAKURA_NODE_TABLE:
        sakuraV_visitTable(S, assembly, node);
        break;
    default:
        printf("Error: unknown node type '%d'\n", node->type);
        break;
    }

    LOG_POP();
}

struct SakuraAssembly *sakuraY_assemble(SakuraState *S, struct NodeStack *nodes) {
    struct SakuraAssembly *assembly;
    struct Node *node;

    LOG_CALL();

    S->currentState = SAKURA_FLAG_ASSEMBLING;
    assembly = SakuraAssembly();

    for (ull i = 0; i < nodes->size; i++) {
        node = nodes->nodes[i];
        sakuraV_visitNode(S, assembly, node);
    }

    SakuraAssembly_push3(assembly, SAKURA_RETURN, 0, 0);

    LOG_POP();
    return assembly;
}

struct SakuraAssembly *SakuraAssembly_new(int fullSetup) {
    struct SakuraAssembly *assembly;

    LOG_CALL();

    assembly = (struct SakuraAssembly *)malloc(sizeof(struct SakuraAssembly));
    assembly->instructions = (int *)malloc(64 * sizeof(int));
    assembly->size = 0;
    assembly->capacity = 64;

    assembly->registers = 0;

    assembly->highestRegister = 0;
    assembly->functionsLoaded = 0;

    assembly->closures = (struct SakuraAssembly **)malloc(4 * sizeof(struct SakuraAssembly *));
    assembly->closureCapacity = 4;
    assembly->closureIdx = 0;

    assembly->pool.size = 0;
    assembly->pool.capacity = fullSetup ? 8 : 0;
    assembly->pool.constants = fullSetup ? (TValue *)malloc(assembly->pool.capacity * sizeof(TValue)) : NULL;

    LOG_POP();
    return assembly;
}

void sakuraX_freeAssembly(struct SakuraAssembly *assembly) {
    LOG_CALL();

    if (assembly->pool.constants) {
        for (ull i = 0; i < assembly->pool.size; i++) {
            if (assembly->pool.constants[i].tt == SAKURA_TSTR)
                s_str_free(&assembly->pool.constants[i].value.s);
        }

        free(assembly->pool.constants);
        assembly->pool.constants = NULL;
        assembly->pool.size = 0;
    }

    if (assembly->instructions) {
        free(assembly->instructions);
        assembly->instructions = NULL;
    }

    if (assembly->closures) {
        for (ull i = 0; i < assembly->closureIdx; i++)
            sakuraX_freeAssembly(assembly->closures[i]);
        free(assembly->closures);
        assembly->closures = NULL;
    }

    free(assembly);

    LOG_POP();
}

void SakuraAssembly_push(struct SakuraAssembly *assembly, int instruction) {
    if (assembly->size >= assembly->capacity) {
        assembly->capacity *= 2;
        assembly->instructions = (int *)realloc(assembly->instructions, assembly->capacity * sizeof(int));
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
    ull idx;

    if (assembly->pool.size >= assembly->pool.capacity) {
        assembly->pool.capacity *= 2;
        assembly->pool.constants =
            (TValue *)realloc(assembly->pool.constants, assembly->pool.capacity * sizeof(TValue));
    }

    idx = assembly->pool.size;
    assembly->pool.constants[assembly->pool.size++] = (TValue){.value.n = value, .tt = SAKURA_TNUMFLT};
    return -(idx + 1);
}

int sakuraX_pushKString(struct SakuraAssembly *assembly, const struct s_str *value) {
    ull idx;

    if (assembly->pool.size >= assembly->pool.capacity) {
        assembly->pool.capacity *= 2;
        assembly->pool.constants =
            (TValue *)realloc(assembly->pool.constants, assembly->pool.capacity * sizeof(TValue));
    }

    idx = assembly->pool.size;
    assembly->pool.constants[assembly->pool.size++] = (TValue){.value.s = s_str_copy(value), .tt = SAKURA_TSTR};
    return -(idx + 1);
}

void SakuraAssembly_pushChildAssembly(struct SakuraAssembly *assembly, struct SakuraAssembly *child) {
    if (assembly->closureIdx >= assembly->closureCapacity) {
        assembly->closureCapacity *= 2;
        assembly->closures = (struct SakuraAssembly **)realloc(assembly->closures, assembly->closureCapacity *
                                                                                       sizeof(struct SakuraAssembly *));
    }

    assembly->closures[assembly->closureIdx++] = child;
}
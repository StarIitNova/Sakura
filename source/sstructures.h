#pragma once

#define SAKURA_STACK_SIZE 8000

#define SAKURA_FLAG_LEXER 0
#define SAKURA_FLAG_PARSER 1
#define SAKURA_FLAG_ASSEMBLING 2
#define SAKURA_FLAG_DISASSEMBLING 5
#define SAKURA_FLAG_RUNTIME 3
#define SAKURA_FLAG_ENDED 4

#define SAKURA_EFLAG_NONE 0
#define SAKURA_EFLAG_SYNTAX 1
#define SAKURA_EFLAG_RUNTIME 2
#define SAKURA_EFLAG_FATAL 3

// #define SAKURA_TNUMINT 1 // integer tag
#define SAKURA_TNUMFLT 0 // float tag
#define SAKURA_TSTR 2    // string tag
#define SAKURA_TCFUNC 3  // C function tag
#define SAKURA_TFUNC 5   // function tag
#define SAKURA_TNIL 6    // nil tag
#define SAKURA_TTABLE 7  // ttable tag

typedef unsigned short SakuraFlag;

#define NPOS (size_t)(-1)

struct SakuraState;
struct SakuraTTable;

enum TokenType {
    // Single Character Tokens (fully implemented)
    SAKURA_TOKEN_LEFT_PAREN,
    SAKURA_TOKEN_RIGHT_PAREN,
    SAKURA_TOKEN_LEFT_BRACE,
    SAKURA_TOKEN_RIGHT_BRACE,
    SAKURA_TOKEN_LEFT_SQUARE,
    SAKURA_TOKEN_RIGHT_SQUARE,
    SAKURA_TOKEN_COMMA,
    SAKURA_TOKEN_DOT,
    SAKURA_TOKEN_MINUS,
    SAKURA_TOKEN_PLUS,
    SAKURA_TOKEN_SEMICOLON,
    SAKURA_TOKEN_SLASH,
    SAKURA_TOKEN_STAR,
    SAKURA_TOKEN_CARET,
    SAKURA_TOKEN_PERCENT,
    SAKURA_TOKEN_HASHTAG,

    // Comparisons (fully implemented)
    SAKURA_TOKEN_BANG,
    SAKURA_TOKEN_BANG_EQUAL,
    SAKURA_TOKEN_EQUAL,
    SAKURA_TOKEN_EQUAL_EQUAL,
    SAKURA_TOKEN_GREATER,
    SAKURA_TOKEN_GREATER_EQUAL,
    SAKURA_TOKEN_LESS,
    SAKURA_TOKEN_LESS_EQUAL,
    SAKURA_TOKEN_AND,
    SAKURA_TOKEN_OR,

    // Literals (fully implemented)
    SAKURA_TOKEN_IDENTIFIER,
    SAKURA_TOKEN_STRING,
    SAKURA_TOKEN_NUMBER,

    // Keywords
    SAKURA_TOKEN_ELSE,
    SAKURA_TOKEN_FALSE,
    SAKURA_TOKEN_NIL,
    SAKURA_TOKEN_RETURN,
    SAKURA_TOKEN_SUPER,
    SAKURA_TOKEN_THIS,
    SAKURA_TOKEN_TRUE,

    // Node Types
    SAKURA_NODE_UNARY_OPERATION,
    SAKURA_NODE_BINARY_OPERATION,
    SAKURA_NODE_CALL,
    SAKURA_NODE_FUNCTION,
    SAKURA_NODE_BLOCK,
    SAKURA_NODE_IF,
    SAKURA_NODE_WHILE,
    SAKURA_NODE_VAR,
    SAKURA_NODE_TABLE,
    SAKURA_NODE_INDEX,

    // Misc
    SAKURA_TOKEN_SENTINEL // for telling the binary operation parser to stop
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

    struct Node **keys;

    struct Node **args;
    size_t argCount;

    struct Node *elseBlock;

    double storageValue;

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

union SakuraValue {
    double n;                         // TNUMFLT
    struct s_str s;                   // TSTR
    int (*cfn)(struct SakuraState *); // TCFUNC
    struct SakuraAssembly *assembly;  // TFUNC
    int nil;                          // TNIL
    struct SakuraTTable *table;       // TTABLE
};

// TValue represents a tagged value
typedef struct {
    int tt; // type tag
    union SakuraValue value;
} TValue;

typedef struct {
    TValue *constants; // contents of the constant pool
    size_t size;
    size_t capacity;
} SakuraConstantPool;

typedef struct {
    TValue rax;
    TValue rbx;
    TValue rcx;
    TValue rdx;

    TValue args[64];
} SakuraRegistry;

struct TVMapPair {
    struct s_str key;
    TValue value;
    int init;
};

struct TVMap {
    struct TVMapPair *pairs;
    size_t size;
    size_t capacity;
};

struct SakuraState {
    TValue stack[SAKURA_STACK_SIZE];
    int stackIndex;
    SakuraRegistry registry;
    SakuraConstantPool pool;
    struct TVMap globals;
    struct s_str *locals;
    size_t localsSize;
    int *callStack;
    size_t callStackSize;
    size_t callStackIndex;
    SakuraFlag error;
    struct s_str errorMessage;
    SakuraFlag currentState;
    size_t internalOffset;
};

typedef struct SakuraState SakuraState;

struct TTableHashEntry {
    TValue key;
    TValue value;
    struct TTableHashEntry *next; // collision handling
};

struct SakuraTTable {
    struct TTableHashEntry **hashPart;
    size_t capacity;
    size_t size;
};
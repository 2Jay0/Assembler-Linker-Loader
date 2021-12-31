#define MAX_HASH 20
#define MAX_SYMBOL 9
#define MAX_REGISTER 9
typedef struct _Node_hash
{
    int hash_ascii;
    char hash_mneonic[10];
    char hash_index[5];
    struct _Node_hash* next;
}Node_hash;

Node_hash* hash[MAX_HASH];

typedef struct _Node_symbol
{
    int locctr;
    char label[10];
    struct _Node_symbol* next;
}Node_symbol;

typedef struct _REG
{
    char reg_name[5];
    int reg_num;
}reg;

Node_symbol* symbol[20][MAX_SYMBOL];

char length_index[5];

void Add_Hash(int ascii, Node_hash* node);

void read_opcode();

void Print_oplist();

int Search_hash(char* order);

int print_type(char* str);

void Add_symbol(int alpha, Node_symbol* node, int now);

int Search_symbol(char* order, int now);

void Print_symbol(int now);

int pass1(char* name, int now);

int pass2(char* name, int now);

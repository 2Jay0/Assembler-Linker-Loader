#define MAX_HASH 20
#define MAX_SYMBOL 9
#define MAX_REGISTER 9
#define MAX_ESTAB 100
#define MAX_ADDRESS  65536

extern int address[MAX_ADDRESS];
extern int memory[MAX_ADDRESS][16];
extern int mem_bp[MAX_ADDRESS];

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
    int reg_value;
}reg;

extern reg Register[MAX_REGISTER];

Node_symbol* symbol[20][MAX_SYMBOL];

typedef struct _Estab {
    char symbol[50];
    int address;
    struct _Estab* link;
}Estab;

Estab* table[MAX_ESTAB];

char length_index[5];
char opcode_name[10];

void Add_Hash(int ascii, Node_hash* node);

void read_opcode();

void Print_oplist();

int Search_hash(char* order);

int Search_hash_num(int num);

int print_type(char* str);

void Add_symbol(int alpha, Node_symbol* node, int now);

int Search_symbol(char* order, int now);

void Print_symbol(int now);

int pass1(char* name, int now);

int pass2(char* name, int now);

int load_pass1(char* file, int* progaddr,int *load_key,int load_flag, int *total_length);

void create_estab(int key, Estab* node);

int load_pass2(char *file, int *cur_addr);

int search_estab(char* string,int size);

int run(int* progaddr,int length,int locctr,int flag);

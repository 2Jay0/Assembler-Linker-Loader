#define MAX_HASH 20

typedef struct _Node_hash
{
    int hash_ascii;
    char hash_mneonic[10];
    char hash_index[5];
    struct _Node_hash* next;
}Node_hash;

Node_hash* hash[MAX_HASH];

void Add_Hash(int ascii, Node_hash* node);

void read_opcode();

void Print_oplist();

int Search_hash(char* order);

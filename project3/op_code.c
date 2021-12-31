#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "20171669.h"

/*----------------------------------------------------------*/
/* 함수 : Add_Hash(int ascii, Node_hash* node)
   목적 : 입력받은 integer값을 19로 나눈 나머지로 구분해 
          해당하는 hash[] table에 넣는다
   리턴값 : 없음*/
/*--------------------------------------------------------- */
void Add_Hash(int ascii, Node_hash* node)
{
    int key = ascii % 19;

    if (hash[key] == NULL)
        hash[key] = node;
    else
    {
        node->next = hash[key];
        hash[key] = node;
    }
}

/*----------------------------------------------------------*/
/* 함수 : Print_oplist()
   목적 : hash table에 저장된 opcode를 저장된 방식으로 출력한다
   리턴값 : 없음*/
/*--------------------------------------------------------- */
void Print_oplist()
{
    for (int i = 0; i < MAX_HASH; i++)
    {
        printf("%-2d : ",i);
       
        if (hash[i] != NULL)
        {
            Node_hash* node = hash[i];

            while (node->next)
            {
                printf("[%-5s, %02X] ", node->hash_mneonic, node->hash_ascii);
                node = node->next;

                if (node->next != NULL)
                    printf("-> ");
            }
            printf("-> [%-5s, %02X]",node->hash_mneonic,node->hash_ascii);
        }
        printf("\n");   
    }
}

/*----------------------------------------------------------*/
/* 함수 : Search_hash(char* order)
   목적 : hash table에 연산 명령어가 있으면 해당하는 아스키값을
          리턴하고 없다면 -1을 리턴한다.(temp변수에 아스키값 저장)
   리턴값 : 연산 명령어 존재 return temp
            존재하지 않으면  return -1 */
/*--------------------------------------------------------- */
int Search_hash(char* order)
{
    int temp;

    strcpy(length_index,"\0");

    for(int i=0;i<MAX_HASH;i++)
    {
        if(hash[i]!=NULL)
        {
            Node_hash* node = hash[i];

            while(node->next)
            {
                if(strcmp(node->hash_mneonic,order)==0)
                {
                    temp=node->hash_ascii;
                    strcpy(length_index,node->hash_index);
                    return temp;
                }
                node=node->next;
            }
            if(strcmp(node->hash_mneonic,order)==0)
            {
                temp=node->hash_ascii;
                strcpy(length_index,node->hash_index);
                return temp;
            }
        }
    }

    return -1;
}

int Search_hash_num(int num)
{
    strcpy(length_index,"\0");
    for(int i=0;i<MAX_HASH;i++)
    {
        if(hash[i]!=NULL)
        {
            Node_hash* node = hash[i];

            while(node->next)
            {
                if(node->hash_ascii==num)
                {
                    strcpy(length_index,node->hash_index);
                    strcpy(opcode_name,node->hash_mneonic);
                    return 1;
                }
                node=node->next;
            }
            if(node->hash_ascii==num)
            {
                strcpy(length_index,node->hash_index);
                strcpy(opcode_name,node->hash_mneonic);
                return 1;
            }
        }
    }

    return -1;
}
/*----------------------------------------------------------*/
/* 함수 : read_opcode()
   목적 : "opcode.txt"파일을 읽어 txt파일에 있는 값을 변수들에
           저장해 정의한 node에 값들을 저장한 후 Add_Hash 함수로
           보낸다.
   리턴값 : 없음*/
/*--------------------------------------------------------- */
void read_opcode()
{
    FILE* fp;
    int ascii;
    char mneonic[10];
    char index[5];

    fp = fopen("opcode.txt", "r");

    if (fp == NULL) {
        printf("error\n");
    }

    for (int k = 0; k < 58; k++)
    {
        fscanf(fp, "%X\t%s\t\t%s", &ascii, mneonic, index);
        Node_hash* node = (Node_hash*)malloc(sizeof(Node_hash));
        node->hash_ascii = ascii;
        strcpy(node->hash_mneonic, mneonic);
        strcpy(node->hash_index, index);
        node->next = NULL;
        Add_Hash(ascii, node);
    }

    fclose(fp);

}

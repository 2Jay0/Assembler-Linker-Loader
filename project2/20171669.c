/* including file */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include "20171669.h"
#define MAX_ADDRESS  65536

int address[MAX_ADDRESS] = { 0, };
int memory[MAX_ADDRESS][16] = { 0, };
int temp_address = 0;

/* linked list 구현을 위한 구조체 */
typedef struct _Node {
    int count;
    char data[100];
    struct _Node* link;
}Node;

Node* head = NULL;

/*----------------------------------------------------------*/
/* 함수 : create_Node(int input_count, char* input)
   목적 : 입력받은 명령어를 linked list 방식으로 저장하는 함수
   리턴값 : 없음 */
/*--------------------------------------------------------- */
void create_Node(int input_count, char* input)
{
    Node* new;
    Node* proc = NULL;
    new = (Node*)malloc(sizeof(Node));
    new->count = input_count;
    strcpy(new->data, input);
    new->link = NULL;

    if (head == NULL) {
        head = new;
    }
    else {
        proc = head;
        while (proc->link != NULL) {
            proc = proc->link;
        }
        proc->link = new;
    }
}

/*----------------------------------------------------------*/
/* 함수 : help()
   목적 : 기능을 수행하는 명령어들을 출력한다.
   리턴값 : 없음 */
/*--------------------------------------------------------- */
void help()
{
    printf("h[elp]\nd[ir]\nq[uit]\n");
    printf("hi[story]\ndu[mp] [start, end]\n");
    printf("e[dit] address value\nf[ill] start, end value\n");
    printf("reset\nopcode mnemonic\nopcodelist\n");
    printf("assemble filename\ntype filename\nsymbol\n");
}

/*----------------------------------------------------------*/
/* 함수 : directory()
   목적 : 현재 디렉토리에 있는 파일들을 출력한다.
   리턴값 : 없음 */
/*--------------------------------------------------------- */
void directory()
{
    DIR* dir = opendir(".");
    struct dirent* dirent;
    struct stat buf;

    while (dirent = readdir(dir)) {
        lstat(dirent->d_name, &buf);
        if(S_ISDIR(buf.st_mode))
            printf("%s/\n", dirent->d_name);
        else if(buf.st_mode & S_IXUSR)
            printf("%s*\n",dirent->d_name);
        else
            printf("%s\n",dirent->d_name);
    } 
    closedir(dir);
}
/*----------------------------------------------------------*/
/* 함수 : dump_print(int start, int end)
   목적 : 메모리에 저장된 내용을 명령어 방식에 맞춰 출력한다.
   리턴값 : 잘못된 명령어 return -1
            정상적으로 작동 return 1 */
/*--------------------------------------------------------- */
int dump_print(int start, int end)
{
    int i, j;
    int start_row, start_col;
    int end_row, end_col;

    if (temp_address == 16 * MAX_ADDRESS && end == -2) 
    {
        temp_address = 0;
        return -1;
    } 

    if(end==-3||start==-3)
        return -1;
    else if (end == -1) 
        end = start + 159;
    else if (end == -2) //dump 
    {
        end = start + 159;
        temp_address = end + 1;
    }

    if (end > 16 * MAX_ADDRESS - 1)  
        end = 16 * MAX_ADDRESS - 1;

    if (end < start || start == -1) 
        return -1;

    start_row = start / 16;
    start_col = start % 16;
    end_row = end / 16;
    end_col = end % 16;

    printf("%05X ", address[start_row]);
    for (i = 0; i < start_col; i++)
        printf("   ");

    if(start_row==end_row)
    {
        for(i=start_col;i<=end_col;i++)
            printf("%02X ", memory[start_row][i]);
        for(i=end_col+1;i<16;i++)
            printf("   ");
    }
    else
        for (i = start_col; i < 16; i++)
            printf("%02X ", memory[start_row][i]);

    printf(" ; ");

    for (i = 0; i < start_col; i++)
        printf(".");
    
    if(start_row!=end_row)
    {
        for (i = start_col; i < 16; i++)
        {
            if (memory[start_row][i] < 32 || memory[start_row][i] > 126)
                printf(".");
            else if (memory[start_row][i] == 32)
                printf(" ");
            else
                printf("%c", (char)memory[start_row][i]);
        }
    }
    else
    {
        for(i=start_col;i<=end_col;i++)
        {
            if (memory[start_row][i] < 32 || memory[start_row][i] > 126)
                printf(".");
            else if (memory[start_row][i] == 32)
                printf(" ");
            else
                printf("%c", (char)memory[start_row][i]);
        }
        for(i=end_col+1;i<16;i++)
            printf(".");
    }
    printf("\n");

    for (i = start_row + 1; i < end_row; i++)
    {
        printf("%05X ", address[i]);
        for (j = 0; j < 16; j++)
        {
            printf("%02X ", memory[i][j]);
        }
        printf(" ; ");
        for (j = 0; j < 16; j++)
        {
            if (memory[i][j] < 32 || memory[i][j] > 126)
                printf(".");
            else if (memory[i][j] == 32)
                printf(" ");
            else
                printf("%c", (char)memory[i][j]);
        }
        printf("\n");
    }

    if(start_row!=end_row)
    {
        printf("%05X ", address[end_row]);
        for (i = 0; i < end_col + 1; i++)
            printf("%02X ", memory[end_row][i]);
        for (i = end_col + 1; i < 16; i++)
            printf("   ");
        printf(" ; ");

        for (i = 0; i < end_col + 1; i++)
        {
            if (memory[end_row][i] < 32 || memory[end_row][i] > 126)
                printf(".");
            else if (memory[end_row][i] == 32)
                printf(" ");
            else
                printf("%c", memory[end_row][i]);
        }
        for (i = end_col + 1; i < 16; i++)
            printf(".");

        printf("\n");
    }

    return 1;

}
/*-------------------------------------------------------*/
/* 프로그램 시작 */
int main()
{
    int i;
    char input[100];
    char order[20];
    int i_count = 1;
    int input_length;
    int Search_ascii;
    int tab_flag=0; //tab 있는지 확인
    char temp_str[100]; // input값 임시 저장 위치
    char cnt_str[100]; // input값 임시 저장(목적 :  한 단어 명령어 위한 단어 개수 체크)
    char *check_str; // 첫 명령어 잘라서 저장 -> 명령어 확인
    char perfect_str[100]; //입력어 정제해서 저장

    /*--------dump,fill,edit에 사용되는 변수---------*/
    char store[4][10];  // 명령어에서 나눈 값들 저장(최대 4개)
    int cur = 0;        // 0~2 store문자열 하나씩 올라감
    int num, ex;
    int cnt = 0;
    int first_cnt=0;    //단어 한개 들어오는 명령어의 경우 2개이상인지 체크
    int start_temp = 0, end_temp = 0;
    int flag = 1; // comma유무 나타내는 변수
    char* ptr;    // 문자열 자르고 저장하는 문자열 
    double n_exp;
    int comma_count = 0; // comma개수 세는 변수
    int value = 0;       // 메모리에 수정할 값 저장하는 변수
    int new_row, new_col; // 수정될 위치 저장하는 row,col
    int f1_new_row,f1_new_col; //fill 시작 주소
    int f2_new_row,f2_new_col; //fill 끝 주소
    int f1_start_temp=0,f2_start_temp=0; //주소값
    /*------------------type--------------------------*/
    int file_check;
    int check_pass1; //pass1 과정 성공/실패 나타내는 변수
    int check_pass2; //pass2 과정 성공/실패 나타내는 변수
    int error_flag=0; // pass1 과정 실패 시 1
    int symbol_flag=0; //ASSEMBLE 성공 시 SYMTAB에 내용 있는 경우
    char cut_temp[20];
    int now=0; //가장 최근 symbol 출력 시 사용

    for (int j = 0; j < MAX_ADDRESS; j++)
        address[j] = 16 * j;

    read_opcode();

    Node* temp = NULL; // history사용시 필요

    while (1) {
        printf("sicsim>");
        fgets(input, sizeof(input), stdin); //입력
        input[strlen(input) - 1] = '\0';

        num=strlen(input);
        if(input[0]!='\0')
        {
            strcpy(temp_str,input);
            strcpy(cnt_str,input);
            check_str=strtok(input," \t");
            if(check_str==NULL)
                tab_flag=1;
            else
            {
                ptr = strtok(cnt_str, "\t ");
                while (ptr != NULL)
                {
                    first_cnt++;    //한단어 명령어 혹은 opcode mnemonic 명령어 시 개수 세는 부분
                    ptr = strtok(NULL, " \t");
                }
            }
        }
        if(num==0||tab_flag==1)
            printf("Wrong\n");
        else if (strcmp(check_str, "h") == 0 || strcmp(check_str, "help") == 0)
        {
            if(first_cnt==1)
            {
                strcpy(perfect_str,check_str);//정제해서 저장 
                create_Node(i_count, perfect_str);
                i_count++;
                help();
            }
            else
                printf("Wrong\n");

        }
        else if (strcmp(check_str, "q") == 0 || strcmp(check_str, "quit") == 0)
        {
            if(first_cnt==1)
                break;
            else
                printf("Wrong\n");
        }
        else if (strcmp(check_str, "d") == 0 || strcmp(check_str, "dir") == 0)
        {
            if(first_cnt==1)
            {
                strcpy(perfect_str,check_str);
                create_Node(i_count, perfect_str);
                i_count++;
                directory();
            }
            else
                printf("Wrong\n");
        }
        else if (strcmp(check_str, "dump") == 0 || strcmp(check_str, "du") == 0)
        {
            num = strlen(temp_str);
            for (int k = 0; k < num; k++)
                if (temp_str[k] == ',')      //comma개수 세기
                    comma_count++;

            if (comma_count != 1)
                flag = -1;

            ptr = strtok(temp_str, "\t ");
            while (ptr != NULL)
            {
                cnt++;
                if(cur>2)
                    break;
                strcpy(store[cur++], ptr);
                ptr = strtok(NULL, ", \t");
            }  //입력한 단어 개수세기

            if (cnt == 3)
            {
                if (flag == 1) //comma가 있는 경우, 없다면 명령어 오류
                {   
                    /* start에 해당하는 값 start_temp에 추출 */
                    num = strlen(store[1]);

                    if(num>5)
                        start_temp=-3;
                    else
                    {
                        ex = num - 1;

                        for (i = 0; i < num; i++)
                        {
                            n_exp = pow(16, (double)ex);
                            ex--;
                            if (store[1][i] >= 48 && store[1][i] <= 57)
                                start_temp += (int)n_exp * ((int)store[1][i] - 48);

                            else if (store[1][i] >= 65 && store[1][i] <= 70)
                                start_temp += (int)n_exp * ((int)store[1][i] - 55);

                            else if (store[1][i] >= 97 && store[1][i] <= 102)
                                start_temp += (int)n_exp * ((int)store[1][i] - 87);

                            else
                            {
                                start_temp = -1;
                                break;
                            }
                        }
                    }
                    /*-----------------------------------------------*/
                    /* end에 해당하는 값 end_temp에 추출 */
                    num = strlen(store[2]);
                    if(num>5)
                        end_temp=-3;
                    else
                    {
                        ex = num - 1;

                        for (i = 0; i < num; i++)
                        {
                            n_exp = pow(16, (double)ex);
                            ex--;
                            if (store[2][i] >= 48 && store[2][i] <= 57)
                                end_temp += (int)n_exp * ((int)store[2][i] - 48);

                            else if (store[2][i] >= 65 && store[2][i] <= 70)
                                end_temp += (int)n_exp * ((int)store[2][i] - 55);

                            else if (store[2][i] >= 97 && store[2][i] <= 102)
                                end_temp += (int)n_exp * ((int)store[2][i] - 87);

                            else
                            {
                                end_temp = -1;
                                break;
                            }
                        }
                    }

                    if (dump_print(start_temp, end_temp) == -1)
                        printf("Wrong\n");
                    else
                    {
                        for(int j=0;j<num;j++)
                            if(store[2][j]>=97 && store[2][j]<=102)
                                store[2][j]-=32;

                        num=strlen(store[1]);

                        for(int j=0;j<num;j++)
                            if(store[1][j]>=97 && store[1][j]<=102)
                                store[1][j]-=32;

                        strcpy(perfect_str,store[0]);
                        strcat(perfect_str," ");
                        strcat(perfect_str,store[1]);
                        strcat(perfect_str,", ");
                        strcat(perfect_str,store[2]);

                        create_Node(i_count,perfect_str);
                        i_count++;
                    }
                }
                else //comma없거나 1개 이상으로 start end 입력받음(오류)
                    printf("Wrong(comma error)\n");

            }
            else if (cnt == 2)
            {
                if(comma_count==0)
                {
                    num = strlen(store[1]);
                    if(num>5)
                        start_temp=-3;
                    else
                    {
                        ex = num - 1;

                        for (i = 0; i < num; i++)
                        {
                            n_exp = pow(16, (double)ex);
                            ex--;
                            if (store[1][i] >= 48 && store[1][i] <= 57)
                                start_temp += (int)n_exp * ((int)store[1][i] - 48);

                            else if (store[1][i] >= 65 && store[1][i] <= 70)
                                start_temp += (int)n_exp * ((int)store[1][i] - 55);

                            else if (store[1][i] >= 97 && store[1][i] <= 102)
                                start_temp += (int)n_exp * ((int)store[1][i] - 87);

                            else
                            {
                                start_temp = -1;
                                break;
                            }
                        }
                    }

                    if (dump_print(start_temp, -1) == -1)
                        printf("Wrong(Write Correct number)\n");
                    else
                    {
                        for(int j=0;j<num;j++)
                            if(store[1][j]>=97 && store[1][j]<=102)
                                store[1][j]-=32;

                        strcpy(perfect_str,store[0]);
                        strcat(perfect_str," ");
                        strcat(perfect_str,store[1]);

                        create_Node(i_count,perfect_str);
                        i_count++;
                    }

                }
                else
                    printf("Wrong(comma error)\n");
            }
            else if (cnt == 1) // dump뒤에 공백 같이 입력시
            {
                if (dump_print(temp_address, -2) == -1)
                    printf("Over Boundary error.\n");
                else
                {
                    strcpy(perfect_str,store[0]);
                    create_Node(i_count,perfect_str);
                    i_count++;
                }
            }
            else
            {
                printf("Wrong Instruction.\n");
            }
            /* 변수 초기화 */
            start_temp = 0;
            end_temp = 0;
            flag = 1;
            cnt = 0;
            cur = 0;
            comma_count = 0;

            for (i = 0; i < 4; i++)
                for (int j = 0; j < 10; j++)
                    store[i][j] = '\0';
            
        }
        else if (strcmp(check_str, "edit") == 0 || strcmp(check_str, "e") == 0)
        {
            num = strlen(temp_str);
            for (int k = 0; k < num; k++)
                if (temp_str[k] == ',')      //comma개수 세기
                    comma_count++;

            if (comma_count != 1)
                flag = -1;

            ptr = strtok(temp_str, " \t");
            while (ptr != NULL)
            {
                cnt++;
                if (cur > 2)
                    break;
                strcpy(store[cur++], ptr);
                ptr = strtok(NULL, ",\t ");
            }

            if (flag == -1 || cnt != 3)
                printf("Wrong instruction\n");
            else
            {
                /* 수정할 주소 추출 */
                num = strlen(store[1]);
                ex = num - 2; //마지막 입력값은 따로니까 하나 더 

                if (num == 1)
                {
                    new_row = 0;
                    if (store[1][0] >= 48 && store[1][0] <= 57)
                        new_col = (int)store[1][0] - 48;
                    else if (store[1][0] >= 65 && store[1][0] <= 70)
                        new_col = (int)store[1][0] - 55;
                    else if (store[1][0] >= 97 && store[1][0] <= 102)
                        new_col = (int)store[1][0] - 87;
                    else
                        new_col = -1;
                }

                else if(num>=2 && num<6)
                {
                    for(i=0;i<num-1;i++)
                    {
                        n_exp = pow(16, (double)ex);
                        ex--;
                        
                        if (store[1][i] >= 48 && store[1][i] <= 57)
                            start_temp += (int)n_exp * ((int)store[1][i] - 48);

                        else if (store[1][i] >= 65 && store[1][i] <= 70)
                            start_temp += (int)n_exp * ((int)store[1][i] - 55);

                        else if (store[1][i] >= 97 && store[1][i] <= 102)
                            start_temp += (int)n_exp * ((int)store[1][i] - 87);

                        else
                            start_temp = -1;
                    }
                    new_row = start_temp;

                    if (store[1][num - 1] >= 48 && store[1][num - 1] <= 57)
                        new_col = (int)store[1][num - 1] - 48;
                    else if (store[1][num - 1] >= 65 && store[1][num - 1] <= 70)
                        new_col = (int)store[1][num - 1] - 55;
                    else if (store[1][num-1] >= 97 && store[1][num-1] <= 102)
                        new_col = (int)store[1][num-1] - 87;
                    else
                        new_col = -1;

                }
                else
                    new_col=-1;
                /* 수정 value 추출 */
                num = strlen(store[2]);
                if(num>2)
                    store[2][0]='G'; //주소값이 5자리 넘어가는 경우 아스키 문자에 해당하지 않은 값 'G'를 넣어서 아래 for문에서 오류가 나게 설정
                ex = num - 1;

                for (i = 0; i < num; i++)
                {
                    n_exp = pow(16, (double)ex);
                    ex--;
                    if (store[2][i] >= 48 && store[2][i] <= 57)
                        value += (int)n_exp * ((int)store[2][i] - 48);

                    else if (store[2][i] >= 65 && store[2][i] <= 70)
                        value += (int)n_exp * ((int)store[2][i] - 55);

                    else if (store[2][i] >= 97 && store[2][i] <= 102)
                        value += (int)n_exp * ((int)store[2][i] - 87);

                    else
                    {
                        value = -1;
                        break;
                    }
                    if (value >= 256)
                        value = -1;
                }
                if (new_col != -1 && new_row != -1 && value != -1)
                {
                    for(int j=0;j<num;j++)
                        if(store[2][j]>=97 && store[2][j]<=102)
                            store[2][j]-=32;

                    num=strlen(store[1]);

                    for(int j=0;j<num;j++)
                        if(store[1][j]>=97 && store[1][j]<=102)
                            store[1][j]-=32;

                    strcpy(perfect_str,store[0]);
                    strcat(perfect_str," ");
                    strcat(perfect_str,store[1]);
                    strcat(perfect_str,", ");
                    strcat(perfect_str,store[2]);

                    create_Node(i_count,perfect_str);
                    i_count++;
                    memory[new_row][new_col] = value;
                }  
                else
                    printf("Wrong instruction!\n");
            }
            /* 변수 초기화*/
            start_temp = 0;
            flag = 1;
            cnt = 0;
            cur = 0;
            comma_count = 0;
            value = 0;

            for (i = 0; i < 4; i++)
                for (int j = 0; j < 10; j++)
                    store[i][j] = '\0';
        }
        else if(strcmp(check_str,"fill")==0||strcmp(check_str,"f")==0)
        {
            num = strlen(temp_str);
            for (int k = 0; k < num; k++)
                if (temp_str[k] == ',')      //comma개수 세기
                    comma_count++;

            if (comma_count != 2)
                flag = -1;

            ptr = strtok(temp_str, "\t ");
            while (ptr != NULL)
            {
                cnt++;
                if(cur > 3)
                    break;
                strcpy(store[cur++], ptr);
                ptr = strtok(NULL, "\t, ");
            }

            if (flag == -1 || cnt != 4)
                printf("Wrong instruction\n");
            else
            {
                /* fill start address */
                num = strlen(store[1]);
                ex = num - 2; //마지막 입력값은 따로니까 하나 더 

                if (num == 1)
                {
                    f1_new_row = 0;
                    if (store[1][0] >= 48 && store[1][0] <= 57)
                        f1_new_col = (int)store[1][0] - 48;
                    else if (store[1][0] >= 65 && store[1][0] <= 70)
                        f1_new_col = (int)store[1][0] - 55;
                    else if (store[1][0] >= 97 && store[1][0] <= 102)
                        f1_new_col = (int)store[1][0] - 87;
                    else
                        f1_new_col = -1;
                }

                else if(num>=2 && num<6)
                {
                    for(i=0;i<num-1;i++)
                    {
                        n_exp = pow(16, (double)ex);
                        ex--;
                        
                        if (store[1][i] >= 48 && store[1][i] <= 57)
                            f1_start_temp += (int)n_exp * ((int)store[1][i] - 48);

                        else if (store[1][i] >= 65 && store[1][i] <= 70)
                            f1_start_temp += (int)n_exp * ((int)store[1][i] - 55);

                        else if (store[1][i] >= 97 && store[1][i] <= 102)
                            f1_start_temp += (int)n_exp * ((int)store[1][i] - 87);

                        else
                            f1_start_temp = -1;
                    }
                    f1_new_row = f1_start_temp;

                    if (store[1][num - 1] >= 48 && store[1][num - 1] <= 57)
                        f1_new_col = (int)store[1][num - 1] - 48;
                    else if (store[1][num - 1] >= 65 && store[1][num - 1] <= 70)
                        f1_new_col = (int)store[1][num - 1] - 55;
                    else if (store[1][num - 1] >= 97 && store[1][num - 1] <= 102)
                        f1_new_col = (int)store[1][num - 1] - 87;
                    else
                        f1_new_col = -1;

                }
                else
                    f1_new_col=-1;
                /* fill end address */
                num = strlen(store[2]);
                ex = num - 2; //마지막 입력값은 따로니까 하나 더 

                if (num == 1)
                {
                    f2_new_row = 0;
                    if (store[2][0] >= 48 && store[2][0] <= 57)
                        f2_new_col = (int)store[2][0] - 48;
                    else if (store[2][0] >= 65 && store[2][0] <= 70)
                        f2_new_col = (int)store[2][0] - 55;
                    else if (store[2][0] >= 97 && store[2][0] <= 102)
                        f2_new_col = (int)store[2][0] - 87;
                    else
                        f2_new_col = -1;
                }
                else if(num>=2 && num<6)
                {
                    for(i=0;i<num-1;i++)
                    {
                        n_exp = pow(16, (double)ex);
                        ex--;
                        
                        if (store[2][i] >= 48 && store[2][i] <= 57)
                            f2_start_temp += (int)n_exp * ((int)store[2][i] - 48);

                        else if (store[2][i] >= 65 && store[2][i] <= 70)
                            f2_start_temp += (int)n_exp * ((int)store[2][i] - 55);

                        else if (store[2][i] >= 97 && store[2][i] <= 102)
                            f2_start_temp += (int)n_exp * ((int)store[2][i] - 87);

                        else
                            f2_start_temp = -1;
                    }
                    f2_new_row = f2_start_temp;

                    if (store[2][num - 1] >= 48 && store[2][num - 1] <= 57)
                        f2_new_col = (int)store[2][num - 1] - 48;
                    else if (store[2][num - 1] >= 65 && store[2][num - 1] <= 70)
                        f2_new_col = (int)store[2][num - 1] - 55;
                    else if (store[2][num - 1] >= 97 && store[2][num - 1] <= 102)
                        f2_new_col = (int)store[2][num - 1] - 87;
                    else
                        f2_new_col = -1;
                }
                else
                    f2_new_col=-1;
                /* value값 추출 */
                num = strlen(store[3]);
                if(num>2)
                    value=-1;
                else
                {
                    ex = num - 1;
                
                    for (i = 0; i < num; i++)
                    {
                        n_exp = pow(16, (double)ex);
                        ex--;
                        if (store[3][i] >= 48 && store[3][i] <= 57)
                            value += (int)n_exp * ((int)store[3][i] - 48);

                        else if (store[3][i] >= 65 && store[3][i] <= 70)
                            value += (int)n_exp * ((int)store[3][i] - 55);

                        else if (store[3][i] >= 97 && store[3][i] <= 102)
                            value += (int)n_exp * ((int)store[3][i] - 87);

                        else
                        {
                            value = -1;
                            break;
                        }
                        if (value >= 256)
                            value = -1;
                    }
                }
                if(value==-1||f1_new_row==-1||f2_new_row==-1||f1_new_col==-1||f2_new_col==-1)
                    printf("Wrong instruction!\n");
                else
                {
                    num=strlen(store[1]);
                    for(int j=0;j<num;j++)
                        if(store[1][j]>=97 && store[1][j]<=102)
                            store[1][j]-=32;

                    num=strlen(store[2]);
                    for(int j=0;j<num;j++)
                        if(store[2][j]>=97 && store[2][j]<=102)
                            store[2][j]-=32;

                    num=strlen(store[3]);
                    for(int j=0;j<num;j++)
                        if(store[3][j]>=97 && store[3][j]<=102)
                            store[3][j]-=32;

                    strcpy(perfect_str,store[0]);
                    strcat(perfect_str," ");
                    strcat(perfect_str,store[1]);
                    strcat(perfect_str,", ");
                    strcat(perfect_str,store[2]);
                    strcat(perfect_str,", ");
                    strcat(perfect_str,store[3]);

                    create_Node(i_count,perfect_str);
                    i_count++;

                    if(f1_new_row==f2_new_row)
                    {
                        for(int j=f1_new_col;j<=f2_new_col;j++)
                            memory[f1_new_row][j]=value;
                    }
                    else   
                    {
                        for(int j=f1_new_col;j<16;j++)
                            memory[f1_new_row][j]=value;

                        for(int j=f1_new_row+1;j<f2_new_row;j++)
                        {
                            for(int k=0;k<16;k++)
                                memory[j][k]=value;
                        }
                        for(int j=0;j<f2_new_col+1;j++)
                            memory[f2_new_row][j]=value;
                    }

                }
            }
            /* 변수 초기화 */
            f1_start_temp = 0;
            f2_start_temp=0;
            flag = 1;
            cnt = 0;
            cur = 0;
            comma_count = 0;
            value = 0;

            for (i = 0; i < 4; i++)
                for (int j = 0; j < 10; j++)
                    store[i][j] = '\0';
        }
        else if(strcmp(check_str, "reset")==0)
        {
            if(first_cnt==1)
            {
                strcpy(perfect_str,check_str);
                create_Node(i_count,perfect_str);
                i_count++;

                for(i=0;i<MAX_ADDRESS;i++)
                    for(int j=0;j<16;j++)
                        memory[i][j]=0;
            }
            else
                printf("Wrong\n");

        }
        else if (strcmp(check_str, "hi") == 0 || strcmp(check_str, "history") == 0)
        {
            if(first_cnt==1)
            {
                strcpy(perfect_str,check_str);
                create_Node(i_count, perfect_str);
                i_count++;
                temp = head;

                while (temp != NULL) {
                    printf("%d\t%s\n", temp->count, temp->data);
                    temp = temp->link;
                }
            }
            else
                printf("Wrong\n");
        }
        else if (strcmp(check_str, "opcodelist") == 0)
        {
            if(first_cnt==1)
            {
                strcpy(perfect_str,check_str);
                create_Node(i_count, perfect_str);
                i_count++;
                Print_oplist();
            }
            else
                printf("Wrong\n");
        }
        else if (strcmp(check_str, "opcode") == 0)
        {
            if(first_cnt==2)
            {
                ptr = strtok(temp_str, "\t ");
                while (ptr != NULL)
                {
                    strcpy(store[cur++], ptr);
                    ptr = strtok(NULL, "\t ");
                    cnt++;
                    if(cur>1)
                        break;
                }

                if(cnt==2)
                {
                    strcpy(order,store[1]);
                    Search_ascii = Search_hash(order);

                    if (Search_ascii == -1)
                        printf("Wrong mneonic\n");
                    else
                    {
                        strcpy(perfect_str,store[0]);
                        strcat(perfect_str," ");
                        strcat(perfect_str,store[1]);
                        create_Node(i_count, perfect_str);
                        i_count++;
                        printf("opcode is %02X\n", Search_ascii);
                    }
                }
                else
                    printf("Wrong\n");
            }
            else
                printf("Wrong\n");

            cur=0;
            cnt=0;
            for (int j = 0; j < 20; j++)
                order[j] = '\0';
        }
        else if (strcmp(check_str, "type") == 0)
        {
            if(first_cnt==2)
            {
                ptr = strtok(temp_str, "\t ");
                while (ptr != NULL)
                {
                    strcpy(store[cur++], ptr);
                    ptr = strtok(NULL, "\t ");
                    cnt++;
                    if(cur>1)
                        break;
                }
                if(cnt==2)
                {
                    strcpy(order,store[1]);
                    file_check = print_type(order);

                    if (file_check != -1)
                    {
                        strcpy(perfect_str,store[0]);
                        strcat(perfect_str," ");
                        strcat(perfect_str,store[1]);
                        create_Node(i_count, perfect_str);
                        i_count++;
                    }
                }
                else
                    printf("Wrong\n");
            }
            else
                printf("Wrong\n");

            cur=0;
            cnt=0;
            for (int j = 0; j < 20; j++)
                order[j] = '\0';
        }
        else if (strcmp(check_str, "assemble") == 0)
        {
            if(first_cnt==2)
            {
                ptr = strtok(temp_str, "\t ");
                while (ptr != NULL)
                {
                    strcpy(store[cur++], ptr);
                    ptr = strtok(NULL, "\t ");
                    cnt++;
                    if(cur>1)
                        break;
                }
                /*------------------pass1----------------*/
                if(cnt==2)
                {
                    strcpy(order,store[1]);
                    char* result = strstr(order,".asm");

                    if(result!=NULL)
                        check_pass1=pass1(order,now);
                    else
                        check_pass1=-3;

                    if(check_pass1==-1) //pass1 success
                    {
                        strcpy(perfect_str,store[0]);
                        strcat(perfect_str," ");
                        strcat(perfect_str,store[1]);
                        create_Node(i_count, perfect_str);
                        i_count++;
                    }
                    else if(check_pass1==-2) //file open fail
                    {
                        printf("파일읽기 실패\n");
                        error_flag=1;
                    }
                    else if(check_pass1==-3)
                    {
                        printf("assmble file must be .asm\n");
                        error_flag=1;
                    }
                    else if(check_pass1<=-5)// error in n-line(n=return line number)
                    {
                        printf("[%d]line error(Write Correct OP_CODE).\n",(-1)*check_pass1);
                        error_flag=1;
                    }
                    else
                    {
                        printf("[%d]line error(Write Correct Range of number.)\n",check_pass1);
                        error_flag=1;
                    }
                }
                else
                    printf("Wrong\n");
                /*-----------------pass2---------------*/
                if(error_flag==0) //pass1에서 오류 없는 경우
                {
                    strcpy(cut_temp,order);
                    check_pass2=pass2(order,now);
                    
                    if(check_pass2==1) //pass2 success, assemble success
                    {
                        ptr=strtok(cut_temp,".");
                        strcpy(cut_temp,ptr);
                        printf("[%s.lst],[%s.obj]\n",cut_temp,cut_temp);
                        now++;
                        symbol_flag=1;
                    }
                    else //error
                    {
                        printf("[%d]line error(Write Correct range of number).\n",check_pass2);
                    }

                    int erase = remove("intermediate.txt");
                    if(erase==-1)
                        printf("erase fail\n");
                }
            }
            else //error
                printf("Wrong instruction\n");

            cur=0;
            cnt=0;
            for (int j = 0; j < 20; j++)
                order[j] = '\0';
            strcpy(cut_temp,"\0");
            error_flag=0;
        }
        else if (strcmp(check_str, "symbol") == 0)
        {
            if(symbol_flag==1)
            {
                if(first_cnt==1&&now!=0)
                {
                    strcpy(perfect_str,check_str);//정제해서 저장 
                    create_Node(i_count, perfect_str);
                    i_count++;
                    Print_symbol(now-1);
                }
                else
                    printf("Wrong instruction\n");
            }
            else
            {
                if(first_cnt==1)
                {
                    strcpy(perfect_str,check_str);//정제해서 저장 
                    create_Node(i_count, perfect_str);
                    i_count++;
                    printf("EMPTY IN SYMBOLTABLE\n");
                }
                else
                    printf("Wrong instruction!\n");
            }
        }
        else
            printf("Wrong instruction!\n");

        /*변수 초기화*/
        first_cnt=0;
        tab_flag=0;
        /* 문자열 임시 저장 공간들 초기화 */
        for(int j=0;j<100;j++)
        {
            perfect_str[j]='\0';
            temp_str[j]='\0';
            cnt_str[j]='\0';
        }


    }

}

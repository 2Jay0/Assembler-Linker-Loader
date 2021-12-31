#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "20171669.h"

reg Register[MAX_REGISTER]=
{
    {"A",0},{"X",1},{"L",2},
    {"B",3},{"S",4},{"T",5},
    {"F",6},{"PC",8},{"SW",9}
}; //Register Number 저장해놓음
 
int pg_length=0; //프로그램의 길이
int num_f4=0; //(modification 필요한) Format4의 개수

/*----------------------------------------------------------*/
/* 함수 : print_type(char* str)
   목적 : 전달받은 문자열 이름의 파일을 열어서 그 내용을
          출력한다.
   리턴값 : 파일열기 실패 시 return -1
            함수 성공적으로 수행 시 return 1 */
/*--------------------------------------------------------- */

int print_type(char* str)
{
    FILE *file;
    char f_name[100];
    char buff[100];
    char *print_line;

    strcpy(f_name,str);

    file=fopen(f_name,"r");

    if(file==NULL)
    {
        printf("Failure file open!\n");
        return -1;
    }

    while(!feof(file))
    {
        print_line=fgets(buff,100,file);
        if(print_line!=NULL)
            printf("%s",print_line);
    }
    fclose(file);
    return 1;
}

/*----------------------------------------------------------*/
/* 함수 : Add_symbol(int alpha, Node_symbol* node, int now)
   목적 : 전달받은 알파벳값을 3으로 나눈 몫으로 구분해 
          해당하는 symbol[now][] table에 넣는다
   리턴값 : 없음 */
/*--------------------------------------------------------- */

void Add_symbol(int alpha, Node_symbol* node,int now)
{
    int key = alpha / 3; //ABC, DEF 순으로 3개씩 저장

    if (symbol[now][key] == NULL)
        symbol[now][key] = node;
    else
    {
        Node_symbol* temp = symbol[now][key];
        
        if(temp->next==NULL) //symtab에 하나만 있을 시
        {
            if(strcmp(node->label,temp->label)<=0) //symtab보다 순서 앞이면 앞에 삽입
            {
                node->next=symbol[now][key];
                symbol[now][key]=node;
            }
            else //순서 뒤라면 뒤에 삽입
            {
                temp->next=node;
            }
        }
        else //symtab에 원소 여러개 달려있을 시
        {
            if(strcmp(node->label,temp->label)<=0) //맨 앞 내용보다 순서 앞이면 앞에 삽입
            {
                node->next=symbol[now][key];
                symbol[now][key]=node;
            }
            else //아니라면 순서 맞을때까지 가봐야함
            {
                while(1)
                {
                    if(temp->next==NULL)
                    {
                        temp->next=node;
                        break;
                    }
                    else if(strcmp(node->label,temp->next->label)<=0)
                    {
                        node->next=temp->next;
                        temp->next=node;
                        break;
                    }
                    temp=temp->next;
                }
            }
        } 
    }
}

/*----------------------------------------------------------*/
/* 함수 : Search_symbol(char* order, int now)
   목적 : 전달받은 명령어를 symbol이 저장된 해시테이블을 돌면서
          그 값이 있다면 locctr 값을 반환한다.
   리턴값 : symbol값이 있다면 return temp(해당 temp=node->locctr)
            없다면 return -1   */
/*--------------------------------------------------------- */
int Search_symbol(char* order, int now)
{
    int temp;

    for(int i=0;i<MAX_SYMBOL;i++)
    {
        if(symbol[now][i]!=NULL)
        {
            Node_symbol* node = symbol[now][i];

            while(node->next)
            {
                if(strcmp(node->label,order)==0)
                {
                    temp=node->locctr;
                    return temp;
                }
                node=node->next;
            }
            if(strcmp(node->label,order)==0)
            {
                temp=node->locctr;
                return temp;
            }
        }
    }

    return -1;
}
/*----------------------------------------------------------*/
/* 함수 : Print_symbol(int now)
   목적 : symbol이 저장된 해시테이블 내용을 출력한다.
   리턴값 : 없음*/
/*--------------------------------------------------------- */
void Print_symbol(int now)
{
    for (int i = 0; i < MAX_SYMBOL; i++)
    { 
        if (symbol[now][i] != NULL)
        {
            Node_symbol* node = symbol[now][i];

            while (node->next)
            {
                printf("\t%-5s\t%04X\n", node->label, node->locctr);
                node = node->next;
            }
            printf("\t%-5s\t%04X\n",node->label,node->locctr);
        } 
    }
}
/*----------------------------------------------------------*/
/* 함수 : pass1(char* name, int now)
   목적 : .asm 파일을 받아와 중간파일(intermediate.txt)를 생성한다
          operand에 해당하는 부분의 location을 기록한다.
   리턴값 : pass1함수가 정상적으로 수행 되었다면 return -1
            파일 읽기가 실패 되었다면 return -2
            assemble 과정 중 오류시 line number 반환*/
/*--------------------------------------------------------- */
int pass1(char *name, int now)
{
    FILE *pass1_fp;
    FILE *intm_fp;

    char buffer[100];
    char f_name[100];
    char temp[3][20];
    char temp_str[10];
    int cur=0,cnt=0;
    int i=0,num=0;
    int alpha;
    char *ptr;
    int pre_location;
    int location;
    int start_address=0;
    int n_exp,ex;

    int found_opcode;
    /*------flag------*/
    int comma_flag=0;
    int plus_flag=0;
    int dot_flag=0;
    int con_flag=0;
    int base_flag=0;
    /*---------------*/
    char byte_constant[20];//ex) BYTE X'05' 에서 05에 해당하는 부분
    int byte_length;
    int line_num=5;
    int dex_operand=0;

    strcpy(f_name,name);

    pass1_fp=fopen(f_name,"r");
    intm_fp=fopen("intermediate.txt","w");

    if(pass1_fp==NULL)
    {
        return -2; //파일 읽기 실패
    }

    /*------------첫 문장 입력--------------*/
    fgets(buffer,sizeof(buffer),pass1_fp);
    ptr = strtok(buffer, "\n ");
    while (ptr != NULL)
    {
        strcpy(temp[cur++], ptr);
        ptr = strtok(NULL, "\t \n");
    }

    if(strcmp(temp[1],"START")==0)
    {
        num=strlen(temp[2]);
        ex = num - 1;
        for (int k = 0; k < num; k++)
        {
            n_exp = pow(16, (double)ex);
            ex--;
            if (temp[2][k] >= 48 && temp[2][k] <= 57)
                start_address += (int)n_exp * ((int)temp[2][k] - 48);
            else
                return line_num; //operation error
        } //starting address 입력받기

        location=start_address;
    }
    else
        location=0;
    
    fprintf(intm_fp,"%3d  %04X\t%-6s\t%-5s\t%-6s\n",line_num,location,temp[0],temp[1],temp[2]);
    
    //변수 초기화
    for(int j=0;j<3;j++)
    {
        for(int k=0;k<10;k++)
            temp[j][k]='\0';
    }
    cur=0;
    line_num+=5;
    /*----------------------------------------------*/
    /*-----------------END나올때까지----------------*/
    while(1)
    {
        fgets(buffer,sizeof(buffer),pass1_fp);

        pre_location=location; //이전 주소 잠시 저장

        num=sizeof(buffer);
        for(int k=0;k<num;k++)
        {
            if(buffer[k]==',')
                comma_flag=1;

            if(buffer[k]=='+')
                plus_flag=1;

            if(buffer[k]=='.')
                dot_flag=1;
        }

        if(dot_flag==0)
        {
            ptr=strtok(buffer,"\n ");
            while(ptr!=NULL)
            {
                strcpy(temp[cur++],ptr);
                ptr=strtok(NULL,"\t \n");
                cnt++;
            }
            if(strcmp(temp[0],"END")==0)
            {
                fprintf(intm_fp,"%3d      \t      \t%-5s\t%-6s\n",line_num,temp[0],temp[1]);
                pg_length=location-start_address;
                break;
            }
            if(cnt==3) //Label field에 symbol이 있는 경우
            {
                /*comma있는 부분은 개수가 3개로 읽히지만 label이
                있는 경우가 아니므로 구분해준다.*/
                if(comma_flag==0)
                {
                    alpha=(int)temp[0][0]-65;
                    Node_symbol* node = (Node_symbol*)malloc(sizeof(Node_symbol));
                    node->locctr = location;
                    strcpy(node->label, temp[0]);
                    node->next = NULL;
                    Add_symbol(alpha,node,now);
                }
                else// COMP A, S 같은 경우
                {
                    strcat(temp[1],temp[2]); 
                    strcpy(temp[2],temp[1]);
                    strcpy(temp[1],temp[0]);
                    strcpy(temp[0],"\0");
                }
            }
            else if(cnt==2)//label없는 경우 label에 \0저장 후 한칸씩 뒤로
            {
                strcpy(temp[2],temp[1]);
                strcpy(temp[1],temp[0]);
                strcpy(temp[0],"\0");
            }
            else if(cnt==1)
            {
                if(strncmp(temp[0],"RSUB",4)==0)
                {
                    ptr=strtok(temp[0]," \t");
                    strcpy(temp[0],ptr);

                    strcpy(temp[1],temp[0]);
                    strcpy(temp[0],"\0");
                    strcpy(temp[2],"\0");
                }
            }

            if(plus_flag==1)
            {
                strcpy(temp_str,temp[1]);
                char *plus_ptr=strtok(temp_str,"+");
                found_opcode=Search_hash(plus_ptr);
                num_f4++;
            }
            else
                found_opcode = Search_hash(temp[1]); //opcode table에 있는지 확인

            if(found_opcode!=-1)
            {
                if(plus_flag==0)
                    location+=((int)length_index[0]-48);
                else
                    location+=((int)length_index[2]-48);
            }
            else if(strcmp(temp[1],"WORD")==0)
            {
                location+=3;
            }
            else if(strcmp(temp[1],"RESW")==0)
            {
                num=strlen(temp[2]);
                ex = num - 1;
                for (int k = 0; k < num; k++)
                {
                    n_exp = pow(10, (double)ex);
                    ex--;
                    if (temp[2][k] >= 48 && temp[2][k] <= 57)
                        dex_operand += (int)n_exp * ((int)temp[2][k] - 48);
                    else
                        return line_num; //operation error
                }
                location+=3*dex_operand;
            }
            else if(strcmp(temp[1],"RESB")==0)
            {
                num=strlen(temp[2]);
                ex = num - 1;
                for (int k = 0; k < num; k++)
                {
                    n_exp = pow(10, (double)ex);
                    ex--;
                    if (temp[2][k] >= 48 && temp[2][k] <= 57)
                        dex_operand += (int)n_exp * ((int)temp[2][k] - 48);
                    else
                        return line_num; //operation error
                }
                location+=dex_operand;
            }
            else if(strcmp(temp[1],"BYTE")==0)
            {
                strcpy(temp_str,temp[2]);
                char *Byte_ptr=strtok(temp_str,"'");
                if(strcmp(Byte_ptr,"C")==0)
                    con_flag=1;
                while(Byte_ptr!=NULL)
                {
                    strcpy(byte_constant,Byte_ptr);
                    Byte_ptr=strtok(NULL,"'");
                }

                if(con_flag==1) //C인경우 EOF length=3
                {
                    byte_length=strlen(byte_constant);
                }
                else //다른 레지스터의 경우 length/2
                {
                    byte_length=strlen(byte_constant)/2;
                }

                location+=byte_length;
            }
            else if(strcmp(temp[1],"BASE")==0)
                    base_flag=1;
            else
            {
                return (-1)*line_num;//invalid operation code
            }

            if(base_flag==1)
                fprintf(intm_fp,"%3d      \t%-6s\t%-5s\t%-6s\n",line_num,temp[0],temp[1],temp[2]);
            else if(((int)length_index[0]-48)==2) //register 다루는 opcode인 경우 중간파일에 표시
                fprintf(intm_fp,"%-3s  %04X\t%-6s\t%-5s\t%-6s\n",length_index,location,temp[0],temp[1],temp[2]);
            else
                fprintf(intm_fp,"%3d  %04X\t%-6s\t%-5s\t%-6s\n",line_num,location,temp[0],temp[1],temp[2]);

            base_flag=0;
            comma_flag=0;
            plus_flag=0;
            con_flag=0;
            cur=0;
            cnt=0;
            dex_operand=0;

            for(int j=0;j<3;j++)
            {
                for(int k=0;k<10;k++)
                    temp[j][k]='\0';
            }

            strcpy(length_index,"\0");
        }
        else
        {
            fprintf(intm_fp,"%3d  \t%-s",line_num,buffer);
        }
        dot_flag=0;
        line_num+=5;
    }

    fclose(intm_fp);
    fclose(pass1_fp);

    return -1;
}
/*----------------------------------------------------------*/
/* 함수 : pass2(char* name, int now)
   목적 : 중간파일(intermediate.txt)을 열어 그 내용을 완전한
          assemble을 수행한다.
   리턴값 : operand에 있는 숫자 오류시 그 line number 반환
            정상적으로 수행 시 return 1*/
/*--------------------------------------------------------- */
int pass2(char* name,int now)
{
    FILE* intm_fp;
    FILE* lst_fp;
    FILE* obj_fp;
    char buffer[100];
    char temp[5][20];
    char trash[20]; //완전 임시 저장 문자열
    char lst_name[20];
    char obj_name[20];
    char* ptr;
    int cur=0, num;
    /*----------문자확인용 flag------------*/
    int shap_flag=0;
    int golbang_flag=0;
    int plus_flag=0;
    int dot_flag=0;
    int comma_flag=0;
    int mod_flag=0;
    int enter_flag=0;
    int res_flag=0;
    /*------------------------------------*/
    int pre_location=0; //진짜 (이전)주소->중간파일에는 다음주소 저장되어있음
    int next_location=0; //intermediate file에 next location 저장해놓은 것 저장하는 변수(pc relative시 사용될 주소)
    int sym_location; //Symbol table에 저장되어있는 symbol의 주소
    int print_location; //lst파일과 obj파일에 출력될 주소값

    int base_address; //base relative시 base에 해당하는 주소
    int search_ascii;
    int xbpe; //xbpe 변수
    int n_exp,ex;
    int hex_num=0; //
    int line_num=5;//line number (5씩 증가)
    
    int print_reg; //레지스터 번호 값 저장
    int print2_reg; //레지스터 번호 값 저장(2개 나올 시)
    char reg_temp[2][20];
    int temp_cur=0;
    /*--------obj file 작성시 필요한 변수-------------*/
    char real_record[100]; //obj file에 작성될 record 문자열
    char record[100]; //주소값들 붙여 넣는 record 문자열
    char rec_temp1[10]; //record에 붙일 값(문자열로 변환 시 사용)
    char rec_temp2[10]; //record에 붙일 값(문자열로 변환 시 사용)
    char rec_temp3[10]; //record에 붙일 값(문자열로 변환 시 사용)
    int obj_line=0; //record 한 줄의 길이 더해 가는 변수
    int real_obj_line=0; //obj file에 작성 될 길이
    int obj_flag=0; //obj file에 적는 경우를 나타내는 flag
    int start_adr; //END 부분 obj file 에 넣을 주소
    int mod_cur=0;
    int total_line=0;

    ptr = strtok(name,".");
    strcpy(name,ptr);

    strcpy(lst_name,name);
    strcat(lst_name,".lst");
    strcpy(obj_name,name);
    strcat(obj_name,".obj");

    intm_fp=fopen("intermediate.txt","r");
    lst_fp=fopen(lst_name,"w");
    obj_fp=fopen(obj_name,"w");

    /*------------read first line------------------*/
    fgets(buffer,sizeof(buffer),intm_fp);
    ptr = strtok(buffer, "\n ");
    while (ptr != NULL)
    {
        strcpy(temp[cur++], ptr);
        ptr = strtok(NULL, "\t \n");
    }
 
    num=strlen(temp[1]);
    ex = num - 1;
                
    for (int k = 0; k < num; k++)
    {
        n_exp = pow(16, (double)ex);
        ex--;
        if (temp[1][k] >= 48 && temp[1][k] <= 57)
            pre_location += (int)n_exp * ((int)temp[1][k] - 48);

        else if (temp[1][k] >= 65 && temp[1][k] <= 70)
            pre_location += (int)n_exp * ((int)temp[1][k] - 55);

        else if (temp[1][k] >= 97 && temp[1][k] <= 102)
            pre_location += (int)n_exp * ((int)temp[1][k] - 87);
    } //처음 값 저장
    /*-------------------START인 경우------------------*/ 
    if(strcmp(temp[3],"START")==0)
    {
        fprintf(lst_fp,"%3d  %04X\t%-6s\t%-5s\t%-10s\t\n",line_num,pre_location,temp[2],temp[3],temp[4]);
    }
    fprintf(obj_fp,"H%s  ",temp[2]);
    fprintf(obj_fp,"%06X%06X\n",pre_location,pg_length);
    
    for(int j=0;j<5;j++)
        strcpy(temp[j],"\0");
    
    start_adr=pre_location; //END에 입력 될 주소 저장
    cur=0;
    line_num+=5;
    strcpy(record,"\0");

    int *mod_num=(int*)malloc(sizeof(int)*num_f4);

    /*----------------------------------------------*/
    /*                 END나올때까지                */
    /*----------------------------------------------*/
    fprintf(obj_fp,"T%06X",pre_location);
    while(1)
    {
        fgets(buffer,sizeof(buffer),intm_fp);
        
        num=strlen(buffer);
        for(int j=0;j<num;j++)
        {
            if(buffer[j]=='+')
                plus_flag=1;
            if(buffer[j]=='#')
                shap_flag=1;
            if(buffer[j]=='@')
                golbang_flag=1;
            if(buffer[j]=='.')
                dot_flag=1;
            if(buffer[j]==',')
                comma_flag=1;
        }

        if(dot_flag==0) //설명부분 아닌 명령문 부분
        {
            ptr = strtok(buffer, "\n ");
            while (ptr != NULL)
            {
                strcpy(temp[cur++], ptr);
                ptr = strtok(NULL, "\t \n");
            }

            if(strcmp(temp[1],"END")==0)
            {
                fprintf(obj_fp,"%02X%s\n",obj_line/2,record);
                fprintf(lst_fp,"%03d      \t      \t%-5s\t%-10s\n",line_num,temp[1],temp[2]);

                if(mod_flag==1)
                    for(int j=0;j<mod_cur;j++)
                        fprintf(obj_fp,"M%06X05\n",mod_num[j]);

                fprintf(obj_fp,"E%06X\n",start_adr);
                break;
            } 

            if(strcmp(temp[0],"2")==0) //format2 인 경우
            {
                res_flag=0;

                num=strlen(temp[1]);
                ex = num - 1;
                
                for (int k = 0; k < num; k++)
                {
                    n_exp = pow(16, (double)ex);
                    ex--;
                    if (temp[1][k] >= 48 && temp[1][k] <= 57)
                        next_location += (int)n_exp * ((int)temp[1][k] - 48);

                    else if (temp[1][k] >= 65 && temp[1][k] <= 70)
                        next_location += (int)n_exp * ((int)temp[1][k] - 55);

                    else if (temp[1][k] >= 97 && temp[1][k] <= 102)
                        next_location += (int)n_exp * ((int)temp[1][k] - 87);
                } //주소값 저장

                if(comma_flag==0)//operand에 레지스터 한 개 있는 경우
                {
                    if(cur==5) //label 있는 경우
                    {
                        search_ascii=Search_hash(temp[3]);

                        for(int k=0;k<MAX_REGISTER;k++)
                        {
                            if(strcmp(Register[k].reg_name,temp[4])==0)
                            {
                                print_reg=Register[k].reg_num;
                                break;
                            }
                        }

                        total_line+=4;
                        obj_line+=4;
                        real_obj_line=obj_line-4;
                        if(obj_line>60)
                        {
                            strcpy(real_record,record);//지금까지 record에 입력한 것 real_record로 전달
                            strcpy(record,"\0"); //record 다시 초기화
                            obj_line=4; //obj라인 초기화
                            obj_flag=1; //obj에 입력 되어야 한다는 표시
                        }
                        sprintf(rec_temp1,"%02X",search_ascii);
                        sprintf(rec_temp2,"%d",print_reg);
                        strcat(record,rec_temp1);
                        strcat(record,rec_temp2);
                        strcat(record,"0");

                        fprintf(lst_fp,"%3d  %04X\t%-6s\t%-5s\t%-10s\t%02X%d0\n",line_num,pre_location,temp[2],temp[3],temp[4],search_ascii,print_reg);
                    }
                    else if(cur==4) //lable 없는 경우
                    {
                        search_ascii=Search_hash(temp[2]);
                        for(int k=0;k<MAX_REGISTER;k++)
                        {
                            if(strcmp(Register[k].reg_name,temp[3])==0)
                            {
                                print_reg=Register[k].reg_num;
                                break;
                            }
                        }

                        total_line+=4;
                        obj_line+=4;
                        real_obj_line=obj_line-4;
                        if(obj_line>60)
                        {
                            strcpy(real_record,record);
                            strcpy(record,"\0");
                            obj_line=4;
                            obj_flag=1;
                        }
                        sprintf(rec_temp1,"%02X",search_ascii);
                        sprintf(rec_temp2,"%d",print_reg);
                        strcat(record,rec_temp1);
                        strcat(record,rec_temp2);
                        strcat(record,"0");

                        fprintf(lst_fp,"%3d  %04X\t      \t%-5s\t%-10s\t%02X%d0\n",line_num,pre_location,temp[2],temp[3],search_ascii,print_reg);
                    }
                }
                else // register 비교부분 ex) COMPR A, S
                {
                    search_ascii=Search_hash(temp[2]);
                    strcpy(trash,temp[3]);
                    char *trash_ptr=strtok(trash,","); //,자르고 찾기
                    while (trash_ptr != NULL)
                    {
                        strcpy(reg_temp[temp_cur++], trash_ptr);
                        trash_ptr = strtok(NULL, ",");
                    }

                    for(int k=0;k<MAX_REGISTER;k++)
                    {
                        if(strcmp(Register[k].reg_name,reg_temp[0])==0)
                            print_reg=Register[k].reg_num;
                        else if(strcmp(Register[k].reg_name,reg_temp[1])==0)
                            print2_reg=Register[k].reg_num;
                    }

                    strcpy(trash,reg_temp[0]);
                    strcat(trash,", ");
                    strcat(trash,reg_temp[1]);
                    strcpy(temp[3],trash);

                    total_line+=4;
                    obj_line+=4;
                    real_obj_line-obj_line-4;
                    if(obj_line>60)
                    {
                        strcpy(real_record,record);
                        strcpy(record,"\0");
                        obj_line=4;
                        obj_flag=1;
                    }
                    sprintf(rec_temp1,"%02X",search_ascii);
                    sprintf(rec_temp2,"%d",print_reg);
                    sprintf(rec_temp3,"%d",print2_reg);
                    strcat(record,rec_temp1);
                    strcat(record,rec_temp2);
                    strcat(record,rec_temp3);
                    fprintf(lst_fp,"%3d  %04X\t      \t%-5s\t%-10s\t%02X%d%d\n",line_num,pre_location,temp[2],temp[3],search_ascii,print_reg,print2_reg);
                }

                temp_cur=0;

                for(int k=0;k<2;k++)
                    strcpy(reg_temp[k],"\0");

            }
            else if(cur==5) //Label 있는 경우
            {
                num=strlen(temp[1]);
                ex = num - 1;
                
                for (int k = 0; k < num; k++)
                {
                    n_exp = pow(16, (double)ex);
                    ex--;
                    if (temp[1][k] >= 48 && temp[1][k] <= 57)
                        next_location += (int)n_exp * ((int)temp[1][k] - 48);

                    else if (temp[1][k] >= 65 && temp[1][k] <= 70)
                        next_location += (int)n_exp * ((int)temp[1][k] - 55);

                    else if (temp[1][k] >= 97 && temp[1][k] <= 102)
                        next_location += (int)n_exp * ((int)temp[1][k] - 87);
                } //중간 파일에 작성 되어 있는 next_location(다음 주소) 저장
                
                if(shap_flag==1) //Immediate addressing mode
                {
                    if(plus_flag==0)
                    {
                        res_flag=0;

                        search_ascii=Search_hash(temp[3]);
                        search_ascii+=1; //intermediate의 경우 +1(n0,i1)

                        strcpy(trash,temp[4]);
                        char *trash_ptr=strtok(trash,"#"); //#자르고 찾기
                        strcpy(trash,trash_ptr);
                        sym_location=Search_symbol(trash,now);

                        if(sym_location!=-1)//symbol이 symbol table에 있는 경우
                        {
                            if((sym_location-next_location>=-2048)&&(sym_location-next_location<=2047)) //pc relative
                            {
                                print_location=sym_location-next_location;
                                xbpe=2;
                                if(print_location<0)
                                print_location=print_location&0x00000FFF;
                            }
                            else if((sym_location-next_location>=2048)&&(sym_location-next_location<=4095)||(sym_location-next_location<=-4095))
                            {
                                print_location=sym_location-base_address;
                                xbpe=4;
                            }
                        }
                        else//symbol 아니면 상수인지 확인
                        {
                            /*-------------십진수변환--------------*/
                            num=strlen(trash);
                            ex = num - 1;
                            for (int k = 0; k < num; k++)
                            {
                                n_exp = pow(10, (double)ex);
                                ex--;
                                if (trash[k] >= 48 && trash[k] <= 57)
                                    hex_num += (int)n_exp * ((int)trash[k] - 48);
                                else
                                    return line_num; //10진수 내에 있는 숫자(상수) 아닌 경우
                            }
                            xbpe=0;
                            print_location=hex_num;
                            hex_num=0;
                        }

                        total_line+=6;
                        obj_line+=6;
                        real_obj_line=obj_line-6;
                        if(obj_line>60)
                        {
                            strcpy(real_record,record);
                            strcpy(record,"\0");
                            obj_line=6;
                            obj_flag=1;
                        }
                        sprintf(rec_temp1,"%02X",search_ascii);
                        sprintf(rec_temp2,"%d",xbpe);
                        sprintf(rec_temp3,"%03X",print_location);
                        strcat(record,rec_temp1);
                        strcat(record,rec_temp2);
                        strcat(record,rec_temp3);

                        fprintf(lst_fp,"%3d  %04X\t%-6s\t%-5s\t%-10s\t%02X%d%03X\n",line_num,pre_location,temp[2],temp[3],temp[4],search_ascii,xbpe,print_location);
                    }
                    else //format 4
                    {
                        res_flag=0;

                        strcpy(trash,temp[4]);
                        char *trash_ptr=strtok(trash,"#");
                        strcpy(trash,trash_ptr);

                        num=strlen(trash);
                        ex = num - 1;
                        for (int k = 0; k < num; k++)
                        {
                            n_exp = pow(10, (double)ex);
                            ex--;
                            if (trash[k] >= 48 && trash[k] <= 57)
                                hex_num += (int)n_exp * ((int)trash[k] - 48);
                            else
                                return line_num; //10진수 내에 있는 숫자 아닌 경우
                        }
                        xbpe=1;
                        print_location=hex_num;
                    
                        strcpy(trash,temp[3]);
                        trash_ptr=strtok(trash,"+"); //+자르고 찾기
                        strcpy(trash,trash_ptr);

                        search_ascii=Search_hash(trash);
                        search_ascii+=1;
 
                        total_line+=8;
                        obj_line+=8; //format 4의 경우 머신어 변환 시 8자리
                        real_obj_line=obj_line-8;
                        if(obj_line>60)
                        {
                            strcpy(real_record,record);
                            strcpy(record,"\0");
                            obj_line=8;
                            obj_flag=1;
                        }
                        sprintf(rec_temp1,"%02X",search_ascii);
                        sprintf(rec_temp2,"%d",xbpe);
                        sprintf(rec_temp3,"%05X",print_location);
                        strcat(record,rec_temp1);
                        strcat(record,rec_temp2);
                        strcat(record,rec_temp3);

                        fprintf(lst_fp,"%3d  %04X\t%-6s\t%-5s\t%-10s\t%02X%d%05X\n",line_num,pre_location,temp[2],temp[3],temp[4],search_ascii,xbpe,print_location);
                        hex_num=0;
                    }
                }
                else if(golbang_flag==1) // Indirect addressing mode
                {
                    res_flag=0;

                    search_ascii=Search_hash(temp[3]);
                    search_ascii+=2; //indirect의 경우 +2(n=1,i=0)

                    strcpy(trash,temp[4]);
                    char* trash_ptr=strtok(trash,"@");
                    strcpy(trash,trash_ptr);
                    sym_location=Search_symbol(trash,now);

                    if((sym_location-next_location>=-2048)&&(sym_location-next_location<=2047)) //pc relative
                    {
                        print_location=sym_location-next_location;
                        xbpe=2;
                        if(print_location<0)
                            print_location=print_location&0x00000FFF; //16진수 음수 처리 시 뒤의 3자리만 나오도록 비트 연산
                    }
                    else if((sym_location-next_location>=2048)&&(sym_location-next_location<=4095)||(sym_location-next_location<=-4095)) //base relative
                    {
                        print_location=sym_location-base_address;
                        xbpe=4;
                    }

                    total_line+=6;
                    obj_line+=6;
                    real_obj_line=obj_line-6;
                    if(obj_line>60)
                    {
                        strcpy(real_record,record);
                        strcpy(record,"\0");
                        obj_line=6;
                        obj_flag=1;
                    }
                    sprintf(rec_temp1,"%02X",search_ascii);
                    sprintf(rec_temp2,"%d",xbpe);
                    sprintf(rec_temp3,"%03X",print_location);
                    strcat(record,rec_temp1);
                    strcat(record,rec_temp2);
                    strcat(record,rec_temp3);

                    fprintf(lst_fp,"%3d  %04X\t%-6s\t%-5s\t%-10s\t%02X%d%03X\n",line_num,pre_location,temp[2],temp[3],temp[4],search_ascii,xbpe,print_location);
                }
                else if(plus_flag==1) //format 4
                {
                    res_flag=0;

                    strcpy(trash,temp[3]);
                    char *trash_ptr=strtok(trash,"+"); //+자르고 찾기
                    strcpy(trash,trash_ptr);

                    search_ascii=Search_hash(trash);
                    search_ascii+=3;

                    sym_location=Search_symbol(temp[4],now);
                    print_location=sym_location;
                    xbpe=1;

                    mod_num[mod_cur++]=(total_line+2)/2;
                    mod_flag=1;

                    total_line+=8;
                    obj_line+=8;
                    real_obj_line-obj_line-8;
                    if(obj_line>60)
                    {
                        strcpy(real_record,record);
                        strcpy(record,"\0");
                        obj_line=8;
                        obj_flag=1;
                    }
                    sprintf(rec_temp1,"%02X",search_ascii);
                    sprintf(rec_temp2,"%d",xbpe);
                    sprintf(rec_temp3,"%05X",print_location);
                    strcat(record,rec_temp1);
                    strcat(record,rec_temp2);
                    strcat(record,rec_temp3);

                    fprintf(lst_fp,"%3d  %04X\t%-6s\t%-5s\t%-10s\t%02X%d%05X\n",line_num,pre_location,temp[2],temp[3],temp[4],search_ascii,xbpe,print_location);
                }
                else if(strcmp(temp[3],"BYTE")==0) 
                {
                    res_flag=0;

                    if(temp[4][0]=='C')//constant인 경우 값 하나당 1byte
                    {
                        strcpy(trash,temp[4]);
                        char *trash_ptr=strtok(trash,"'");
                        while(trash_ptr!=NULL)
                        {
                            strcpy(trash,trash_ptr);
                            trash_ptr=strtok(NULL,"'");
                        }
                        num=strlen(trash);
                        fprintf(lst_fp,"%3d  %04X\t%-6s\t%-5s\t%-10s\t",line_num,pre_location,temp[2],temp[3],temp[4]);

                        for(int k=0;k<num;k++)
                            fprintf(lst_fp,"%02X",trash[k]);

                        fprintf(lst_fp,"\n");
                        
                        total_line+=(num*2);
                        obj_line+=(num*2);
                        real_obj_line=obj_line-(num*2);
                        if(obj_line>60)
                        {
                            strcpy(real_record,record);
                            strcpy(record,"\0");
                            obj_line=(num*2);
                            obj_flag=1;
                        }

                        for(int k=0;k<num;k++)
                        {
                            sprintf(rec_temp1,"%02X",trash[k]);
                            strcat(record,rec_temp1);
                        }
                    }
                    else //constant 아닌 경우 ' '안에 있는 변수 자체가 BYTE
                    {
                        strcpy(trash,temp[4]);
                        char*trash_ptr=strtok(trash,"'");
                        while(trash_ptr!=NULL)
                        {
                            strcpy(trash,trash_ptr);
                            trash_ptr=strtok(NULL,"'");
                        }
                    
                        num=strlen(trash);
                        total_line+=num;
                        obj_line+=num;
                        real_obj_line=obj_line-num;
                        if(obj_line>60)
                        {
                            strcpy(real_record,record);
                            strcpy(record,"\0");
                            obj_line=num;
                            obj_flag=1;
                        }

                        strcat(record,trash);
                        
                        fprintf(lst_fp,"%3d  %04X\t%-6s\t%-5s\t%-10s\t%-s\n",line_num,pre_location,temp[2],temp[3],temp[4],trash);
                    }
                }
                else if(strcmp(temp[3],"WORD")==0)
                {
                    res_flag=0;

                    strcpy(trash,temp[4]);
                    num=strlen(trash);
                    ex = num - 1;
                    for (int k = 0; k < num; k++)
                    {
                        n_exp = pow(10, (double)ex);
                        ex--;
                        if (trash[k] >= 48 && trash[k] <= 57)
                            hex_num += (int)n_exp * ((int)trash[k] - 48);
                        else
                            return line_num; //10진수 내에 있는 숫자 아닌 경우
                    }
                    print_location=hex_num;
                    
                    total_line+=6;
                    obj_line+=6;
                    real_obj_line=obj_line-6;
                    if(obj_line>60)
                    {
                        strcpy(real_record,record);
                        strcpy(record,"\0");
                        obj_line=6;
                        obj_flag=1;
                    }
                    
                    fprintf(lst_fp,"%3d  %04X\t%-6s\t%-5s\t%-10s\t%04X\n",line_num,pre_location,temp[2],temp[3],temp[4],print_location);

                    hex_num=0;
                }
                else if(strcmp(temp[3],"RESW")==0)
                {
                    fprintf(lst_fp,"%3d  %04X\t%-6s\t%-5s\t%-10s\n",line_num,pre_location,temp[2],temp[3],temp[4]);
                    res_flag++;
                }
                else if(strcmp(temp[3],"RESB")==0)
                {
                    fprintf(lst_fp,"%3d  %04X\t%-6s\t%-5s\t%-10s\n",line_num,pre_location,temp[2],temp[3],temp[4]);
                    res_flag++;
                }
                else //Simple addressing mode
                {
                    res_flag=0;

                    search_ascii=Search_hash(temp[3]);
                    search_ascii+=3;

                    sym_location=Search_symbol(temp[4],now);
                    
                    if((sym_location-next_location>=-2048)&&(sym_location-next_location<=2047)) //pc relative
                    {
                        print_location=sym_location-next_location;
                        xbpe=2;
                        if(print_location<0)
                            print_location=print_location&0x00000FFF;
                    }
                    else if((sym_location-next_location>=2048)&&(sym_location-next_location<=4095)||(sym_location-next_location<=-4095)) //base relative
                    {
                        print_location=sym_location-base_address;
                        xbpe=4;
                    }

                    total_line+=6;
                    obj_line+=6;
                    real_obj_line=obj_line-6;
                    if(obj_line>60)
                    {
                        strcpy(real_record,record);
                        strcpy(record,"\0"); 
                        obj_line=6;
                        obj_flag=1;
                    }
                    sprintf(rec_temp1,"%02X",search_ascii);
                    sprintf(rec_temp2,"%d",xbpe);
                    sprintf(rec_temp3,"%03X",print_location);
                    strcat(record,rec_temp1);
                    strcat(record,rec_temp2);
                    strcat(record,rec_temp3);

                    fprintf(lst_fp,"%3d  %04X\t%-6s\t%-5s\t%-10s\t%02X%d%03X\n",line_num,pre_location,temp[2],temp[3],temp[4],search_ascii,xbpe,print_location);
                }
            }

            else if(cur==4) //Lable 없는 경우
            {
                res_flag = 0;

                num=strlen(temp[1]);
                ex = num - 1;
                
                for (int k = 0; k < num; k++)
                {
                    n_exp = pow(16, (double)ex);
                    ex--;
                    if (temp[1][k] >= 48 && temp[1][k] <= 57)
                        next_location += (int)n_exp * ((int)temp[1][k] - 48);

                    else if (temp[1][k] >= 65 && temp[1][k] <= 70)
                        next_location += (int)n_exp * ((int)temp[1][k] - 55);

                    else if (temp[1][k] >= 97 && temp[1][k] <= 102)
                        next_location += (int)n_exp * ((int)temp[1][k] - 87);
                }

                if(shap_flag==1) //Immediate addressing mode
                {
                    if(plus_flag==0)
                    {
                        search_ascii=Search_hash(temp[2]);
                        search_ascii+=1; //intermediate의 경우 +1(n0,i1)

                        strcpy(trash,temp[3]);
                        char *trash_ptr=strtok(trash,"#"); //#자르고 찾기
                        strcpy(trash,trash_ptr);
                        sym_location=Search_symbol(trash,now);

                        if(sym_location!=-1)//symbol에 있으면
                        {
                            if((sym_location-next_location>=-2048)&&(sym_location-next_location<=2047)) //pc relative
                            {
                                print_location=sym_location-next_location;
                                xbpe=2;
                                if(print_location<0)
                                print_location=print_location&0x00000FFF;
                            }
                            else if((sym_location-next_location>=2048)&&(sym_location-next_location<=4095)||(sym_location-next_location<=-4095))
                            {
                                print_location=sym_location-base_address;
                                xbpe=4;
                            }
                        }
                        else//symbol 아니면 상수인지 확인
                        {
                            /*-------------십진수변환--------------*/
                            num=strlen(trash);
                            ex = num - 1;
                            for (int k = 0; k < num; k++)
                            {
                                n_exp = pow(10, (double)ex);
                                ex--;
                                if (trash[k] >= 48 && trash[k] <= 57)
                                    hex_num += (int)n_exp * ((int)trash[k] - 48);
                                else
                                    return line_num; //10진수 내에 있는 숫자 아닌 경우
                            }
                            xbpe=0;
                            print_location=hex_num;
                            hex_num=0;
                        }

                        total_line+=6;
                        obj_line+=6;
                        real_obj_line=obj_line-6;
                        if(obj_line>60)
                        {
                            strcpy(real_record,record);
                            strcpy(record,"\0");
                            obj_line=6;
                            obj_flag=1;
                        }
                        sprintf(rec_temp1,"%02X",search_ascii);
                        sprintf(rec_temp2,"%d",xbpe);
                        sprintf(rec_temp3,"%03X",print_location);
                        strcat(record,rec_temp1);
                        strcat(record,rec_temp2);
                        strcat(record,rec_temp3);

                        fprintf(lst_fp,"%3d  %04X\t      \t%-5s\t%-10s\t%02X%d%03X\n",line_num,pre_location,temp[2],temp[3],search_ascii,xbpe,print_location);
                    }
                    else //Format 4인 경우
                    {
                        strcpy(trash,temp[3]);
                        char *trash_ptr=strtok(trash,"#");
                        strcpy(trash,trash_ptr);

                        num=strlen(trash);
                        ex = num - 1;
                        for (int k = 0; k < num; k++)
                        {
                            n_exp = pow(10, (double)ex);
                            ex--;
                            if (trash[k] >= 48 && trash[k] <= 57)
                                hex_num += (int)n_exp * ((int)trash[k] - 48);
                            else
                                return line_num; //10진수 내에 있는 숫자 아닌 경우
                        }
                        xbpe=1;
                        print_location=hex_num;
                    
                        hex_num=0;

                        strcpy(trash,temp[2]);
                        trash_ptr=strtok(trash,"+"); //+자르고 찾기
                        strcpy(trash,trash_ptr);

                        search_ascii=Search_hash(trash);
                        search_ascii+=1;

                        total_line+=8;
                        obj_line+=8;
                        real_obj_line=obj_line-8;
                        if(obj_line>60)
                        {
                            strcpy(real_record,record);
                            strcpy(record,"\0"); 
                            obj_line=8;
                            obj_flag=1;
                        }
                        sprintf(rec_temp1,"%02X",search_ascii);
                        sprintf(rec_temp2,"%d",xbpe);
                        sprintf(rec_temp3,"%05X",print_location);
                        strcat(record,rec_temp1);
                        strcat(record,rec_temp2);
                        strcat(record,rec_temp3);

                        fprintf(lst_fp,"%3d  %04X\t      \t%-5s\t%-10s\t%02X%d%05X\n",line_num,pre_location,temp[2],temp[3],search_ascii,xbpe,print_location);
                    }
                }
                else if(golbang_flag==1) // Indirect addressing mode
                {
                    search_ascii=Search_hash(temp[2]);
                    search_ascii+=2; //indirect의 경우 +2(n=1,i=0)

                    strcpy(trash,temp[3]);
                    char* trash_ptr=strtok(trash,"@");
                    strcpy(trash,trash_ptr);
                    sym_location=Search_symbol(trash,now);

                    if((sym_location-next_location>=-2048)&&(sym_location-next_location<=2047)) //pc relative
                    {
                        print_location=sym_location-next_location;
                        xbpe=2;
                        if(print_location<0)
                            print_location=print_location&0x00000FFF;
                    }
                    else if((sym_location-next_location>=2048)&&(sym_location-next_location<=4095)||(sym_location-next_location<=-4095))
                    {
                        print_location=sym_location-base_address;
                        xbpe=4;
                    }

                    total_line+=6;
                    obj_line+=6;
                    real_obj_line=obj_line-6;
                    if(obj_line>60)
                    {
                        strcpy(real_record,record);
                        strcpy(record,"\0"); 
                        obj_line=6;
                        obj_flag=1;
                    }
                    sprintf(rec_temp1,"%02X",search_ascii);
                    sprintf(rec_temp2,"%d",xbpe);
                    sprintf(rec_temp3,"%03X",print_location);
                    strcat(record,rec_temp1);
                    strcat(record,rec_temp2);
                    strcat(record,rec_temp3);

                    fprintf(lst_fp,"%3d  %04X\t      \t%-5s\t%-10s\t%02X%d%03X\n",line_num,pre_location,temp[2],temp[3],search_ascii,xbpe,print_location);
                }
                else if(plus_flag==1) //Format 4인 경우
                {
                    strcpy(trash,temp[2]);
                    char *trash_ptr=strtok(trash,"+"); //+자르고 찾기
                    strcpy(trash,trash_ptr);

                    search_ascii=Search_hash(trash);
                    search_ascii+=3;

                    sym_location=Search_symbol(temp[3],now);
                    print_location=sym_location;
                    xbpe=1;

                    mod_num[mod_cur++]=(total_line+2)/2;
                    mod_flag=1;

                    total_line+=8;
                    obj_line+=8;
                    real_obj_line=obj_line-8;
                    if(obj_line>60)
                    {
                        strcpy(real_record,record);
                        strcpy(record,"\0"); 
                        obj_line=8;
                        obj_flag=1;
                    }
                    sprintf(rec_temp1,"%02X",search_ascii);
                    sprintf(rec_temp2,"%d",xbpe);
                    sprintf(rec_temp3,"%05X",print_location);
                    strcat(record,rec_temp1);
                    strcat(record,rec_temp2);
                    strcat(record,rec_temp3);

                    fprintf(lst_fp,"%3d  %04X\t       \t%-5s\t%-10s\t%02X%d%05X\n",line_num,pre_location,temp[2],temp[3],search_ascii,xbpe,print_location);
                }
                else if(strcmp(temp[2],"STCH")==0||strcmp(temp[2],"LDCH")==0)
                {
                    search_ascii=Search_hash(temp[2]);
                    search_ascii+=3;

                    strcpy(trash,temp[3]);
                    char *trash_ptr=strtok(trash,","); //,자르고 찾기
                    while (trash_ptr != NULL)
                    {
                        strcpy(reg_temp[temp_cur++], trash_ptr);
                        trash_ptr = strtok(NULL, ",");
                    }
                    xbpe=12; //Base relative x=1 b=1 p=0 e=0
                    sym_location=Search_symbol(reg_temp[0],now);
                    
                    strcpy(trash,reg_temp[0]);
                    strcat(trash,", ");
                    strcat(trash,reg_temp[1]);
                    strcpy(temp[3],trash);

                    total_line+=6;
                    obj_line+=6;
                    real_obj_line=obj_line-6;
                    if(obj_line>60)
                    {
                        strcpy(real_record,record);
                        strcpy(record,"\0"); 
                        obj_line=6;
                        obj_flag=1;
                    }
                    sprintf(rec_temp1,"%02X",search_ascii);
                    sprintf(rec_temp2,"%X",xbpe);
                    sprintf(rec_temp3,"%03X",sym_location-base_address);
                    strcat(record,rec_temp1);
                    strcat(record,rec_temp2);
                    strcat(record,rec_temp3);

                    fprintf(lst_fp,"%3d  %04X\t      \t%-5s\t%-10s\t%02X%X%03X\n",line_num,pre_location,temp[2],temp[3],search_ascii,xbpe,sym_location-base_address);
                temp_cur=0;

                for(int k=0;k<2;k++)
                    strcpy(reg_temp[k],"\0");
                }
                else //Simple addressing mode
                {
                    search_ascii=Search_hash(temp[2]);
                    search_ascii+=3;

                    sym_location=Search_symbol(temp[3],now);
                    if((sym_location-next_location>=-2048)&&(sym_location-next_location<=2047)) //pc relative
                    {
                        print_location=sym_location-next_location;
                        xbpe=2;
                        if(print_location<0)
                            print_location=print_location&0x00000FFF;
                    }
                    else if((sym_location-next_location>=2048)&&(sym_location-next_location<=4095)||(sym_location-next_location<=-4095)) //base relative
                    {
                        print_location=sym_location-base_address;
                        xbpe=4;
                    }

                    total_line+=6;
                    obj_line+=6;
                    real_obj_line=obj_line-6;
                    if(obj_line>60)
                    {
                        strcpy(real_record,record);
                        strcpy(record,"\0"); 
                        obj_line=6;
                        obj_flag=1;
                    }
                    sprintf(rec_temp1,"%02X",search_ascii);
                    sprintf(rec_temp2,"%d",xbpe);
                    sprintf(rec_temp3,"%03X",print_location);
                    strcat(record,rec_temp1);
                    strcat(record,rec_temp2);
                    strcat(record,rec_temp3);

                    fprintf(lst_fp,"%3d  %04X\t      \t%-5s\t%-10s\t%02X%d%03X\n",line_num,pre_location,temp[2],temp[3],search_ascii,xbpe,print_location);
                }
            }

            else if(cur==3) //Lable, operand 없는 경우
            {
                if(strcmp(temp[2],"RSUB")==0)
                {
                    num=strlen(temp[1]);
                    ex = num - 1;
                
                    for (int k = 0; k < num; k++)
                    {
                        n_exp = pow(16, (double)ex);
                        ex--;
                        if (temp[1][k] >= 48 && temp[1][k] <= 57)
                            next_location += (int)n_exp * ((int)temp[1][k] - 48);

                        else if (temp[1][k] >= 65 && temp[1][k] <= 70)
                            next_location += (int)n_exp * ((int)temp[1][k] - 55);

                        else if (temp[1][k] >= 97 && temp[1][k] <= 102)
                            next_location += (int)n_exp * ((int)temp[1][k] - 87);
                    }
                    search_ascii=Search_hash(temp[2]);
                    search_ascii+=3;

                    xbpe=0;
                    print_location=0;

                    total_line+=6;
                    obj_line+=6;
                    real_obj_line=obj_line-6;
                    if(obj_line>60)
                    {
                        strcpy(real_record,record);
                        strcpy(record,"\0"); 
                        obj_line=6;
                        obj_flag=1;
                    }
                    sprintf(rec_temp1,"%02X",search_ascii);
                    sprintf(rec_temp2,"%d",xbpe);
                    sprintf(rec_temp3,"%03X",print_location);
                    strcat(record,rec_temp1);
                    strcat(record,rec_temp2);
                    strcat(record,rec_temp3);

                    fprintf(lst_fp,"%3d  %04X\t        %-5s\t          \t%02X%d%03X\n",line_num,pre_location,temp[2],search_ascii,xbpe,print_location);
                }

                if(strcmp(temp[1],"BASE")==0) //set Base relative
                {
                    sym_location=Search_symbol(temp[2],now); //BASE 뒤의 operand 주소값 가져와서
                    base_address=sym_location; //base address에 저장한다.
                    next_location=pre_location;
                    fprintf(lst_fp,"%3d      \t        %-5s\t%-10s\n",line_num,temp[1],temp[2]);
                }
            }

            if(obj_flag==1)
            {
                fprintf(obj_fp,"%02X%s\n",real_obj_line/2,real_record);
                fprintf(obj_fp,"T%06X",pre_location);
                obj_flag=0;
                real_obj_line=0;
                strcpy(real_record,"\0");
            }
            
            if(res_flag==1)
                enter_flag=1;

            if(enter_flag==1)
            {
                fprintf(obj_fp,"%02X%s\n",obj_line/2,record);
                obj_flag=0;
                obj_line=0;
                strcpy(record,"\0");
                enter_flag++;
            }

            if(res_flag==0&&enter_flag>=2)
            {
                fprintf(obj_fp,"T%06X",pre_location);
                enter_flag=0;
            }

            pre_location=next_location; //intermediate file에는 next line을 저장했으므로 진짜 주소 저장
            next_location=0; //변수 초기화
        }
        else //dot_flag == 0 인 경우
        { 
            fprintf(lst_fp,"%s",buffer);
        }

        /*--------변수 초기화---------*/
        cur=0;
        shap_flag=0;
        plus_flag=0;
        golbang_flag=0;
        dot_flag=0;
        comma_flag=0;
        hex_num=0;

        line_num+=5;
        strcpy(trash,"\0");
    }
    free(mod_num);
    fclose(lst_fp);
    fclose(obj_fp);

    return 1;

}

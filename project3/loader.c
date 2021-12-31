#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "20171669.h"

int bp_count=0;
int end_flag=0;

void create_estab(int key, Estab* node)
{
    if (table[key] == NULL)
        table[key] = node;
    else
    {
        node->link = table[key];
        table[key] = node;
    }
    
}

int search_estab(char* string, int size)
{
    int temp;

    for(int i=0;i<10;i++)
    {
        if(table[i]!=NULL)
        {
            Estab* node = table[i];

            while(node->link)
            {
                if(strncmp(node->symbol,string,size)==0)
                { 
                    temp=node->address;
                    return temp;
                }
                node=node->link;
            }
            if(strncmp(node->symbol,string,size)==0)
            {
                temp=node->address;
                return temp;
            }
        }
    }

    return -1;
}

int load_pass1(char* file, int* progaddr,int *load_key,int flag,int* total_length)
{
    FILE *p1_fp;

    p1_fp=fopen(file,"r");

    int csaddr=*progaddr;
    char buffer[100];
    char temp[4][100];
    char name[6];
    char* ptr;
    double n_exp;
    int cur=0, cnt=0;
    int i,num;
    int addr=0, length=0;
    int end_flag=0;
    int key;
    int D_num; //D부분에 D제외한 부분의 길
    key=*load_key;

    for(int k=0;k<9;k++)
        Register[k].reg_value=0;  //레지스터 초기화

    /*---------------------------------*/
    if(flag==0||flag==-2)
    {
        printf("control symbol address length\n");
        printf("section name\n");
        printf("-------------------------------\n");
    }
    /*---------------------------------*/

    while(1)
    {
        fgets(buffer,sizeof(buffer),p1_fp);
        switch(buffer[0])
        {
            case 'H':
                ptr=strtok(buffer,"H ");
                while(ptr!=NULL)
                {
                    strcpy(temp[cur++],ptr);
                    ptr=strtok(NULL," \n");
                    cnt++;
                }
                for(i=0;i<6;i++)
                {
                    n_exp=pow(16,(double)5-i);
                    if (temp[1][i] >= 48 && temp[1][i] <= 57)
                        addr+=(int)n_exp*(int)(temp[1][i]-48);
                    else if (temp[1][i] >= 65 && temp[1][i] <= 70)
                        addr+=(int)n_exp*(int)(temp[1][i]-55);

                    if (temp[1][i+6] >= 48 && temp[1][i+6] <= 57)
                        length+=(int)n_exp*(int)(temp[1][i+6]-48);
                    else if (temp[1][i+6] >= 65 && temp[1][i+6] <= 70)
                        length+=(int)n_exp*(int)(temp[1][i+6]-55);

                }
                addr+=csaddr;
                Estab* node = (Estab*)malloc(sizeof(Estab));
                strcpy(node->symbol,temp[0]);
                node->address=addr;
                node->link=NULL; 
                create_estab(key++,node);
                printf("%-6s          %04X    %04X\n",temp[0],addr,length);
                *total_length+=length;
                addr=0;
                break;
            case 'D':
                D_num=strlen(buffer)-1;
                for(int j=0;j<(D_num/12);j++)
                {
                    for(i=1+j*12;i<7+j*12;i++)
                    {
                        n_exp=pow(16,(double)6*(2*j+1)-i);
                        if (buffer[i+6] >= 48 && buffer[i+6] <= 57)
                            addr+=(int)n_exp*(int)(buffer[i+6]-48);
                        else if (buffer[i+6] >= 65 && buffer[i+6] <= 70)
                            addr+=(int)n_exp*(int)(buffer[i+6]-55);

                        name[i-(j*12+1)]=buffer[i];
                    }
                    Estab* new = (Estab*)malloc(sizeof(Estab));
                    strcpy(new->symbol,name);
                    addr+=csaddr;
                    new->address=addr;
                    new->link=NULL;
                    create_estab(key++,new);
                    printf("        %6s  %04X\n",name,addr);
                    addr=0;
                    strcpy(name,"\0");
                }

                *progaddr=csaddr+length;
                
                break;
            case 'E':
                end_flag=1;
                break;
            default:
                break;
        }
        if(end_flag==1)
            break;
        cnt=0;
        cur=0;
        for(i=0;i<4;i++)
            strcpy(temp[i],"\0");

    }
    if(flag==-1||flag==-2)
    {
        printf("-------------------------------\n");
        printf("          total length  %04X\n",*total_length);
    }
    *load_key=key;
    fclose(p1_fp);
    return 1;
}

int load_pass2(char* file, int *cur_addr)
{
    FILE *fp;
    fp=fopen(file,"r");

    char buffer[100];
    char* ptr;
    char temp[10][100];
    char External_name[10][20]={'\0'}; // R부분에 저장되어있는거 reference 숫자를 인덱스로 저장
    int cur=0, cnt=0;
    int index_num=0;
    int num, i, k;
    int end_flag=0;
    double n_exp;
    /*-------------T record---------------------*/
    int T_addr=0;
    int T_length=0;
    int T_addr_row, T_addr_col;
    int insert_num=0;
    int M_addr=0;
    int M_addr_row, M_addr_col;
    int M_temp=0;
    int M_temp_row, M_temp_col;
    int M_byte=0;
    int M_index=0;
    int arith_flag=0;
    int estab_addr; //estab에서 search한 address
    char M_string[10];
    int E_length=0;

    while(1)
    {
        fgets(buffer,sizeof(buffer),fp); 
        switch(buffer[0])
        {
            case 'H':
                ptr=strtok(buffer,"H ");
                while(ptr!=NULL)
                {
                    strcpy(temp[cur++],ptr);
                    ptr=strtok(NULL," \n");
                    cnt++;
                }
                strcpy(External_name[1],temp[0]); //H record의 control section 이름 저장
                for(i=0;i<6;i++)
                {
                    n_exp=pow(16,(double)5-i);
                    if (temp[1][i+6] >= 48 && temp[1][i+6] <= 57)
                        E_length+=(int)n_exp*(int)(temp[1][i+6]-48);
                    else if (temp[1][i+6] >= 65 && temp[1][i+6] <= 70)
                        E_length+=(int)n_exp*(int)(temp[1][i+6]-55);
                }

                break;
            case 'R':
                ptr=strtok(buffer,"R ");
                while(ptr!=NULL)
                {
                    strcpy(temp[cur++],ptr);
                    ptr=strtok(NULL," \n");
                    cnt++;
                }

                for(i=0;i<cnt;i++)
                {
                    if (temp[i][0] >= 48 && temp[i][0] <= 57)
                        index_num+=16*(int)(temp[i][0]-48);
                    else if (temp[i][0] >= 65 && temp[i][0] <= 70)
                        index_num+=16*(int)(temp[i][0]-55);

                    if (temp[i][1] >= 48 && temp[i][1] <= 57)
                        index_num+=(int)(temp[i][1]-48);
                    else if (temp[i][1] >= 65 && temp[i][1] <= 70)
                        index_num+=(int)(temp[i][1]-55);
    
                    num=strlen(temp[i]);
                    for(k=2;k<num;k++)
                        External_name[index_num][k-2]=temp[i][k];
                    for(k=num;k<6;k++)
                        External_name[index_num][k-2]=' ';
                    
                    index_num=0;
                }
                break;
            case 'T':
                for(i=1;i<=6;i++)
                { 
                    n_exp=pow(16,(double)6-i);
                    if (buffer[i] >= 48 && buffer[i] <= 57)
                        T_addr+=(int)n_exp*(int)(buffer[i]-48);
                    else if (buffer[i] >= 65 && buffer[i] <= 70)
                        T_addr+=(int)n_exp*(int)(buffer[i]-55);
                }
                T_addr+=(*cur_addr);//Text record 시작 주소값 저장

                for(i=7;i<=8;i++)
                {
                    n_exp=pow(16,(double)8-i);
                    if (buffer[i] >= 48 && buffer[i] <= 57)
                        T_length+=(int)n_exp*(int)(buffer[i]-48);
                    else if (buffer[i] >= 65 && buffer[i] <= 70)
                        T_length+=(int)n_exp*(int)(buffer[i]-55);
                }
                T_addr_row=T_addr/16;
                T_addr_col=T_addr%16;
                for(i=9;i<9+(2*T_length);i++)
                { 
                    if(i%2==1)//i 홀수이면 16^1*buffer[i]
                    {
                        if (buffer[i] >= 48 && buffer[i] <= 57)
                            insert_num+=16*(int)(buffer[i]-48);
                        else if (buffer[i] >= 65 && buffer[i] <= 70)
                            insert_num+=16*(int)(buffer[i]-55);
                    }
                    else
                    {
                         if (buffer[i] >= 48 && buffer[i] <= 57)
                            insert_num+=(int)(buffer[i]-48);
                        else if (buffer[i] >= 65 && buffer[i] <= 70)
                            insert_num+=(int)(buffer[i]-55);

                        memory[T_addr_row][T_addr_col++]=insert_num;
                        insert_num=0;
                    }
                }

                T_addr=0;
                T_length=0;
                insert_num=0;
                break;
            case 'M':

                for(i=1;i<=6;i++)
                { 
                    n_exp=pow(16,(double)6-i);
                    if (buffer[i] >= 48 && buffer[i] <= 57)
                        M_addr+=(int)n_exp*(int)(buffer[i]-48);
                    else if (buffer[i] >= 65 && buffer[i] <= 70)
                        M_addr+=(int)n_exp*(int)(buffer[i]-55);
                }
                M_addr+=(*cur_addr);

                M_addr_row=M_addr/16;
                M_addr_col=M_addr%16;
                for(i=7;i<=8;i++)
                {
                    n_exp=pow(16,(double)8-i);
                    if (buffer[i] >= 48 && buffer[i] <= 57)
                        M_byte+=(int)n_exp*(int)(buffer[i]-48);
                    else if (buffer[i] >= 65 && buffer[i] <= 70)
                        M_byte+=(int)n_exp*(int)(buffer[i]-55);
                }

                if(M_byte==5) //M_byte 5인 경우
                {
                    for(i=0;i<3;i++)
                    {
                        M_temp_row=memory[M_addr_row][M_addr_col+i]/16;
                        M_temp_col=memory[M_addr_row][M_addr_col+i]%16;
                        if(i==0)
                        {
                            if(M_temp_col>=8&&M_temp_col<16)
                                M_temp+=4293918720;
                            
                            n_exp=pow(16,(double)4);
                            M_temp+=(int)n_exp*(M_temp_col);
                        }
                        else
                        {
                            n_exp=pow(16,(double)5-2*i);
                            M_temp+=(int)n_exp*(M_temp_row);
                            n_exp=pow(16,(double)4-2*i);
                            M_temp+=(int)n_exp*(M_temp_col);
                        }
                    }
                }
                else if(M_byte==6) //M_byte 6인 경우
                {
                    for(i=0;i<3;i++)
                    {
                        M_temp_row=memory[M_addr_row][M_addr_col+i]/16;
                        M_temp_col=memory[M_addr_row][M_addr_col+i]%16;
                        if(i==0)
                            if(M_temp_row>=8&&M_temp_row<16)
                                M_temp+=4278190080;

                        n_exp=pow(16,(double)5-2*i);
                        M_temp+=(int)n_exp*(M_temp_row);
                        n_exp=pow(16,(double)4-2*i);
                        M_temp+=(int)n_exp*(M_temp_col);
                    }
                }

                if(buffer[9]=='+')
                    arith_flag=1;
                else if(buffer[9]=='-')
                    arith_flag=-1;

                for(i=10;i<=11;i++)
                {
                    n_exp=pow(16,(double)11-i);
                    if (buffer[i] >= 48 && buffer[i] <= 57)
                        M_index+=(int)n_exp*(int)(buffer[i]-48);
                    else if (buffer[i] >= 65 && buffer[i] <= 70)
                        M_index+=(int)n_exp*(int)(buffer[i]-55);
                }
                
                num=strlen(External_name[M_index]);
                estab_addr=search_estab(External_name[M_index],num);

                if(arith_flag==1)
                    M_temp+=estab_addr;//+estab_addr;
                else if(arith_flag==-1)
                    M_temp-=estab_addr;//-estab_addr;

                M_addr_row=M_addr/16;
                M_addr_col=M_addr%16;
                if(M_byte==5)
                {
                    if(M_temp<0)
                        M_temp=M_temp&0x000FFFFF;
                    sprintf(M_string,"%05X",M_temp);

                    for(i=0;i<5;i++)
                    {
                        if(i==0)
                        {
                            insert_num=memory[M_addr_row][M_addr_col]/16;
                            insert_num*=16;

                            if (M_string[i] >= 48 && M_string[i] <= 57)
                                insert_num+=(int)(M_string[i]-48);
                            else if (M_string[i] >= 65 && M_string[i] <= 70)
                                insert_num+=(int)(M_string[i]-55);

                            memory[M_addr_row][M_addr_col++]=insert_num;
                            insert_num=0;
                        }
                        else if(i%2==1)
                        {
                            if (M_string[i] >= 48 && M_string[i] <= 57)
                                insert_num+=16*(int)(M_string[i]-48);
                            else if (M_string[i] >= 65 && M_string[i] <= 70)
                                insert_num+=16*(int)(M_string[i]-55);
                        }
                        else
                        {   
                             if (M_string[i] >= 48 && M_string[i] <= 57)
                                insert_num+=(int)(M_string[i]-48);
                            else if (M_string[i] >= 65 && M_string[i] <= 70)
                                insert_num+=(int)(M_string[i]-55);

                            memory[M_addr_row][M_addr_col++]=insert_num;
                            insert_num=0;
                        }
                    }

                }
                else if(M_byte==6)
                {
                    if(M_temp<0)
                        M_temp=M_temp&0x00FFFFFF;

                    sprintf(M_string,"%06X",M_temp);

                    for(i=0;i<6;i++)
                    {
                        if(i%2==0)
                        {
                            if (M_string[i] >= 48 && M_string[i] <= 57)
                                insert_num+=16*(int)(M_string[i]-48);
                            else if (M_string[i] >= 65 && M_string[i] <= 70)
                                insert_num+=16*(int)(M_string[i]-55);
                        }
                        else
                        {   
                             if (M_string[i] >= 48 && M_string[i] <= 57)
                                insert_num+=(int)(M_string[i]-48);
                            else if (M_string[i] >= 65 && M_string[i] <= 70)
                                insert_num+=(int)(M_string[i]-55);

                            memory[M_addr_row][M_addr_col++]=insert_num;
                            insert_num=0;
                        }
                    }
                }

                insert_num=0;
                arith_flag=0;
                M_index=0;
                M_byte=0;
                M_addr=0;
                M_temp=0;
                break;
            case 'E':
                end_flag=1;
                *cur_addr+=E_length;
                break;
            default:
                break;
        }
        
        if(end_flag==1)
            break;
        cur=0;
        cnt=0;
        for(i=0;i<10;i++)
            strcpy(temp[i],"\0");
    }

    fclose(fp);
    return 1;
}

int run(int *progaddr, int length,int locctr,int flag)
{
    int run_addr=*progaddr;
    int run_addr_row, run_addr_col;
    double n_exp;
    int i,num;
    int get_num;
    int next_locctr=locctr; //다음 수행할 location 주소
    int found_opcode;
    int ni_bit; //TA simple/immediate/indirect 에 따른 ni bit
    int format;
    int reg_write=0;
    int register_name;

    run_addr_row=run_addr/16;
    run_addr_col=run_addr%16;

    if(flag==0)
    {
        bp_count=0;
        Register[2].reg_value=length;
        Register[7].reg_value=*progaddr;
    }

    while(1)
    { 
        if(mem_bp[bp_count]==1)//break point인 경우
        {
            printf("A : %06X   X : %06X\n",Register[0].reg_value,Register[1].reg_value);
            printf("L : %06X  PC : %06X\n",Register[2].reg_value,Register[7].reg_value);
            printf("B : %06X   S : %06X\n",Register[3].reg_value,Register[4].reg_value);
            printf("T : %06X\n",Register[5].reg_value);
            printf("            Stop at checkpoint[%X]\n",bp_count);

            *progaddr=(run_addr_row*16)+(run_addr_col);
            mem_bp[bp_count]++;

            return next_locctr;
        } 

        if(end_flag==1)
        {
            Register[7].reg_value=length;
            printf("A : %06X   X : %06X\n",Register[0].reg_value,Register[1].reg_value);
            printf("L : %06X  PC : %06X\n",Register[2].reg_value,Register[7].reg_value);
            printf("B : %06X   S : %06X\n",Register[3].reg_value,Register[4].reg_value);
            printf("T : %06X\n",Register[5].reg_value);
            printf("            End Program\n");

            *progaddr=0;
            end_flag=0;
            memory[3][5]=3;
            return -1;
        }
        /*----------처음 opcode에 해당하는 부분-----------*/
        get_num=memory[run_addr_row][run_addr_col++];

        if(run_addr_col>15)
        {
            run_addr_row++;
            run_addr_col=0;
        }

        for(ni_bit=0;ni_bit<=3;ni_bit++)
        {
            found_opcode = Search_hash_num(get_num-ni_bit);
            if(found_opcode==1)
            {
                next_locctr+=((int)length_index[0]-48);
                break;
            }
        }

        format=(int)length_index[0]-48;

        /*-------------------------------------------------*/
        /*------------xbpe bit 구할 수 있는 부분----------*/
        if(memory[run_addr_row][run_addr_col]/16==1&&format!=2)//format 4
        {
            next_locctr+=1;
            format++;
        }
        
        Register[7].reg_value=next_locctr; //PC register
        bp_count=next_locctr; //다음 위치 bp point면 while 위에서 결과출력

        switch(format)
        {
            case 1:
                break;
            case 2:
                if(strcmp(opcode_name,"CLEAR")==0)
                {
                    register_name=memory[run_addr_row][run_addr_col++]/16;

                    Register[register_name].reg_value=0;

                    if(run_addr_col>15)
                    {
                        run_addr_row++;
                        run_addr_col=0;
                    }
                }
                else if(strcmp(opcode_name,"TIXR")==0)
                {
                    Register[1].reg_value+=3;

                    run_addr_col++;
                    if(run_addr_col>15)
                    {
                        run_addr_row++;
                        run_addr_col=0;
                    }

                    Register[5].reg_value=Register[1].reg_value;
                }
                else
                {
                    run_addr_col++;
                    if(run_addr_col>15)
                    {
                        run_addr_row++;
                        run_addr_col=0;
                    }
                }
                break;
            case 3:
                if(strcmp(opcode_name,"JEQ")==0)
                {
                    if(memory[run_addr_row][run_addr_col]%16==15)
                    {
                        run_addr_col++;
                        if(run_addr_col>15)
                        {
                            run_addr_row++;
                            run_addr_col=0;
                        }

                        run_addr_col++;
                        if(run_addr_col>15)
                        {
                            run_addr_row++;
                            run_addr_col=0;
                        }
                        
                    }
                    else
                    {
                        n_exp=pow(16,(double)2);
                        reg_write+=n_exp*(memory[run_addr_row][run_addr_col++]%16);

                        if(run_addr_col>15)
                        {
                            run_addr_row++;
                            run_addr_col=0;
                        }
                        reg_write+=16*(memory[run_addr_row][run_addr_col]/16);
                        reg_write+=(memory[run_addr_row][run_addr_col++]%16);

                        if(run_addr_col>15)
                        {
                            run_addr_row++;
                            run_addr_col=0;
                        }
    
                        if(reg_write==7)
                        {
                            Register[7].reg_value=26;
                            next_locctr=Register[7].reg_value;
                            run_addr_row=Register[7].reg_value/16;
                            run_addr_col=Register[7].reg_value%16;
                        }
                        else if(reg_write==8)
                        {
                            Register[7].reg_value=4182;
                            next_locctr=Register[7].reg_value;
                            run_addr_row=Register[7].reg_value/16;
                            run_addr_col=Register[7].reg_value%16;
                        }
                         
                        reg_write=0;
                    }
                }
                else if(strcmp(opcode_name,"LDB")==0)
                {
                    n_exp=pow(16,(double)2);
                    reg_write+=n_exp*(memory[run_addr_row][run_addr_col++]%16);
                    
                    if(run_addr_col>15)
                    {
                        run_addr_row++;
                        run_addr_col=0;
                    }

                    
                    reg_write+=16*(memory[run_addr_row][run_addr_col]/16);
                    reg_write+=(memory[run_addr_row][run_addr_col++]%16);

                    if(run_addr_col>15)
                    {
                        run_addr_row++;
                        run_addr_col=0;
                    }

                    Register[3].reg_value=reg_write+next_locctr;

                    reg_write=0;
                }
                else if(strcmp(opcode_name,"LDA")==0)
                {
                    if(memory[run_addr_row][run_addr_col]/16==2)
                    {
                        n_exp=pow(16,(double)2);
                        reg_write+=n_exp*(memory[run_addr_row][run_addr_col++]%16);
                    
                        if(run_addr_col>15)
                        {
                            run_addr_row++;
                            run_addr_col=0;
                        }

                        reg_write+=16*(memory[run_addr_row][run_addr_col]/16);
                        reg_write+=(memory[run_addr_row][run_addr_col++]%16);

                        if(run_addr_col>15)
                        {
                            run_addr_row++;
                            run_addr_col=0;
                        }

                        Register[0].reg_value=reg_write+next_locctr;

                        reg_write=0;
                    }
                    else if(memory[run_addr_row][run_addr_col]/16==0)
                    {
                        n_exp=pow(16,(double)2);
                        reg_write+=n_exp*(memory[run_addr_row][run_addr_col++]%16);
                    
                        if(run_addr_col>15)
                        {
                            run_addr_row++;
                            run_addr_col=0;
                        }

                        reg_write+=16*(memory[run_addr_row][run_addr_col]/16);
                        reg_write+=(memory[run_addr_row][run_addr_col++]%16);

                        if(run_addr_col>15)
                        {
                            run_addr_row++;
                            run_addr_col=0;
                        }

                        Register[0].reg_value=reg_write;

                        reg_write=0;
                    }
                }
                else if(strcmp(opcode_name,"LDT")==0)
                {
                    Register[5].reg_value=Register[3].reg_value;

                    run_addr_col++;
                    if(run_addr_col>15)
                    {
                        run_addr_row++;
                        run_addr_col=0;
                    }

                    run_addr_col++;
                    if(run_addr_col>15)
                    {
                        run_addr_row++;
                        run_addr_col=0;
                    }
                }
                else if(strcmp(opcode_name,"RSUB")==0)
                {
                    Register[7].reg_value=Register[2].reg_value;

                    next_locctr=Register[7].reg_value;
                    
                    bp_count=next_locctr;
                    run_addr_row=next_locctr/16;
                    run_addr_col=next_locctr%16;
                }
                else if(strcmp(opcode_name,"COMP")==0)
                {
                    run_addr_col++;
                    if(run_addr_col>15)
                    {
                        run_addr_row++;
                        run_addr_col=0;
                    }

                    run_addr_col++;
                    if(run_addr_col>15)
                    {
                        run_addr_row++;
                        run_addr_col=0;
                    }
                }
                else if(strcmp(opcode_name,"LDCH")==0)
                {
                    Register[0].reg_value=70;

                    run_addr_col++;
                    if(run_addr_col>15)
                    {
                        run_addr_row++;
                        run_addr_col=0;
                    }

                    run_addr_col++;
                    if(run_addr_col>15)
                    {
                        run_addr_row++;
                        run_addr_col=0;
                    }
                }
                else if(strcmp(opcode_name,"JLT")==0)
                {
                    run_addr_col++;
                    if(run_addr_col>15)
                    {
                        run_addr_row++;
                        run_addr_col=0;
                    }

                    run_addr_col++;
                    if(run_addr_col>15)
                    {
                        run_addr_row++;
                        run_addr_col=0;
                    }
                }
                else if(strcmp(opcode_name,"J")==0)
                {
                    n_exp=pow(16,(double)2);
                    reg_write+=n_exp*(memory[run_addr_row][run_addr_col++]%16);
                    
                    if(run_addr_col>15)
                    {
                        run_addr_row++;
                        run_addr_col=0;
                    }

                    
                    reg_write+=16*(memory[run_addr_row][run_addr_col]/16);
                    reg_write+=(memory[run_addr_row][run_addr_col++]%16);

                    if(run_addr_col>15)
                    {
                        run_addr_row++;
                        run_addr_col=0;
                    }

                    if(reg_write==3)
                        end_flag=1;

                    reg_write=0;
                }
                else
                {
                    run_addr_col++;
                    if(run_addr_col>15)
                    {
                        run_addr_row++;
                        run_addr_col=0;
                    }

                    run_addr_col++;
                    if(run_addr_col>15)
                    {
                        run_addr_row++;
                        run_addr_col=0;
                    }
                }
                reg_write=0;
                break;
            case 4:
                if(strcmp(opcode_name,"JSUB")==0)
                {
                    Register[2].reg_value=Register[7].reg_value;//L<-PC

                    n_exp=pow(16,(double)4);
                    reg_write+=n_exp*(memory[run_addr_row][run_addr_col++]%16);
                    if(run_addr_col>15)
                    {
                        run_addr_row++;
                        run_addr_col=0;
                    }

                    for(i=0;i<2;i++)
                    {
                        n_exp=pow(16,(double)3-i*2);
                        reg_write+=n_exp*(memory[run_addr_row][run_addr_col]/16);
                        n_exp=pow(16,(double)2-i*2);
                        reg_write+=n_exp*(memory[run_addr_row][run_addr_col++]%16);
                        if(run_addr_col>15)
                        {
                            run_addr_row++;
                            run_addr_col=0;
                        }
                    }
                    
                    Register[7].reg_value=reg_write;
                    next_locctr=reg_write;
                    bp_count=next_locctr;
                    run_addr_row=reg_write/16;
                    run_addr_col=reg_write%16;
               
                    reg_write=0;
                    
                }
                else if(strcmp(opcode_name,"LDT")==0)
                {
                    n_exp=pow(16,(double)4);
                    reg_write+=n_exp*(memory[run_addr_row][run_addr_col++]%16);

                    if(run_addr_col>15)
                    {
                        run_addr_row++;
                        run_addr_col=0;
                    }

                    for(i=0;i<2;i++)
                    {
                        n_exp=pow(16,(double)3-i*2);
                        reg_write+=n_exp*(memory[run_addr_row][run_addr_col]/16);
                        n_exp=pow(16,(double)2-i*2);
                        reg_write+=n_exp*(memory[run_addr_row][run_addr_col++]%16);
                        if(run_addr_col>15)
                        {
                            run_addr_row++;
                            run_addr_col=0;
                        }
                    }

                    Register[5].reg_value=reg_write;

                    reg_write=0;
                }
                break;
            default:
                break;
        }
    }
}

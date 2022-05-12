#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <cstdio>

/*����� ��������*/
unsigned int PC, IR;
FILE* pFile;


/*�ùķ����Ϳ� ���� �Լ� ����*/
void printNotice();
int checkArgument1(int lenCode, char type);
int checkArgument2(int lenCode, char type);
int checkArgument3(int lenCode, int type);

/*������ ����� �Լ� ����*/     
void initializeRegister();//�������� �ʱ�ȭ
void setRegister(unsigned int regNum, unsigned int val);//���ϴ� �������� ���� ������ �� �ִ� �Լ�.
void setMemory(char* offset, char* val);//���ϴ� ������ �ش� �޸𸮿� �����Ͽ� ���� �����ϴ� �Լ�.
void updatePC(unsigned int addr);//���� pc���� ���ϴ� ������ �����ϴ� �Լ��̴�.
void loadInitTask();//���̳ʸ� ������ �ε��ϰ� �޸𸮿� ?�����ϴ� �۾��� ����ϴ� �Լ�.
void showRegister();//�������̽� 'r'����� ��ȯ�Ǵ� �Լ�
void startGoTask();//�������̽� 'g'����� ��ȯ�Ǵ� �Լ�
void startStepTask();//�������̽� 's'����� ��ȯ�Ǵ� �Լ�   �� debugging �Լ� ���ԵǾ�����

char* regArr[32] = { "$zero","$at","$v0","$v1","$a0","$a1","$a2","$a3",
"$t0","$t1","$t2","$t3","$t4","$t5","$t6","$t7",
"$s0","$s1","$s2","$s3","$s4","$s5","$s6","$s7",
"$t8","$t9","$k0","$k1","$gp","$sp","$s8","$ra" };

static unsigned char progMEM[0x100000], dataMEM[0x100000], stakMEM[0x100000];

static unsigned int R[32], PC;

void openBinaryFile(char* filePath);   // l ��ɾ� ����� filePath�� �޾Ƽ� ���̳ʸ� ���� ���� �Լ�
unsigned int To_BigEndian(unsigned int x);  // �򿣵�� ���� �Լ� => hex��
unsigned char getOp(int opc);  // opcode Ȯ�� �Լ�
// binary to decimal �� ���� int������ ������
unsigned char* getInstName(int opc, int fct, int* isImmediate);   // debugging �Լ�
void instExecute(int opc, int fct, int* isImmediate);   // instruction �����Լ�
int MEM(unsigned int A, int V, int nRW, int S); // memory access�Լ�
// ALU
int logicOperation(int X, int Y, int C);
int addSubtract(int X, int Y, int C);
int shiftOperation(int V, int Y, int C);
int checkZero(int S);
int checkSetLess(int X, int Y);
int ALU(int X, int Y, int C, int* Z);   // R-format ��ɾ�� ALU�Լ� �ʿ�



int main(){
    //�ùķ����� ���� ����
    char cmdLine[50];
    int lenCode;
    int cmdLen;
    int cmdErr;

    //�������� �ʱ�ȭ �Լ�


    while(1){
        //COMAND ���� �ʱ�ȭ
        lenCode = 0; //��ɾ��� ���� ��
        cmdLen = 0; //��ɾ��� �ڸ���(1~2�ڸ� �ĺ���)
        cmdErr = 0;
        char *cmdArr[10] = {NULL, };
        

        //�ùķ����� ���� ���
        printNotice();

    /*����Է¹ޱ�*/
        printf("��ɾ �Է��ϼ���.\n>>> ");
        gets(cmdLine);

        char* ptr = strtok(cmdLine, " ");

        while (ptr != NULL)            // �ڸ� ���ڿ��� ������ ���� ������ �ݺ�
        {
            cmdArr[lenCode] = ptr;      // ���ڿ��� �ڸ� �� �޸� �ּҸ� ���ڿ� ������ �迭�� ����
            lenCode++;                       

            ptr = strtok(NULL, " ");   // ���� ���ڿ��� �߶� �����͸� ��ȯ
        }

        if(cmdArr[0] != NULL){
            cmdLen = strlen(cmdArr[0]);
        }

    /*� ��ɾ����� �ĺ�*/
        if(cmdLen == 1){ //��ɾ �ѱ����� ��

            switch (*cmdArr[0])
            {
        /*l ��ɾ�*/
            case 'l':
                if(checkArgument2(lenCode, 'l') == 1) //��ɾ� ��ȿ���˻�
                    break;
                
                //�Լ�����

                //���̳ʸ� ������ ���� ���� ��쿡 ���� ���� ó��


                break;
        /*j ��ɾ�*/
            case 'j':
                if(checkArgument2(lenCode, 'j') == 1) //��ɾ� ��ȿ���˻�
                    break;
                
                //�Լ�����

                break;
        
        /*g ��ɾ�*/
            case 'g':
                if(checkArgument1(lenCode, 'g') == 1) //��ɾ� ��ȿ���˻�
                    break;
                
                //�Լ�����

                break;

        /*s ��ɾ�*/    
            case 's':
                if(checkArgument1(lenCode, 's') == 1) //��ɾ� ��ȿ���˻�
                    break;
                
                //�Լ�����

                break;

        /*m ��ɾ�*/
            case 'm':
                if(checkArgument3(lenCode, 1) == 1) //��ɾ� ��ȿ���˻�
                    break;
                
                //�Լ�����

                break;

        /*r ��ɾ�*/
            case 'r':
                if(checkArgument1(lenCode, 'r') == 1) //��ɾ� ��ȿ���˻�
                    break;
                
                //�Լ�����

                break;

        /*x ��ɾ�*/
            case 'x':
                printf("���α׷��� �����մϴ�.\n");
                exit(1);
                break;

        /*���ǵ��� ���� ��ɾ� ����ó��: ��ɾ� 1��¥��*/
            default:
                printf("Error: �ùٸ� ��ɾ �Է����ּ���.");
                break;
            }

        }
        

        else if(cmdLen == 2){ //��ɾ �α����� ��

        /*sr ��ɾ�*/
            if(!strcmp(cmdArr[0], "sr")){
                if(checkArgument3(lenCode, 2) == 1){ //��ɾ� ��ȿ���˻�
                    printf("\n\n");
                    continue;
                } 
                else{
                    //�Լ�����
                }
            }

        /*sm ��ɾ�*/
            else if(!strcmp(cmdArr[0], "sm")){
                if(checkArgument3(lenCode, 3) == 1){ //��ɾ� ��ȿ���˻�
                    printf("\n\n");
                    continue;
                }
                else{
                    //�Լ�����
                }
            }
        /*���ǵ��� ���� ��ɾ� ����ó��: ��ɾ� 2��¥��*/
            else{
                printf("Error: �ùٸ� ��ɾ �Է����ּ���.");
            }

        }
    /*���ǵ��� ���� ��ɾ� ����ó��: ��ɾ� �Է�x�� ��� + ���ǵ��� ���� ��ɾ��� ���*/
        else{
            printf("Error: �ùٸ� ��ɾ �Է����ּ���.");
        }
        
        printf("\n\n");
    }
}


//�ùķ����� ���� ����Լ�
void printNotice(){
    printf("\t\t\t*��ɾ� �Է�����*\n");
    printf("l ���������̸�\t\t\t:���������� �ùķ����� �޸𸮿� �ö󰩴ϴ�.\n");
    printf("j ���α׷� ���� ��ġ \t\t:�Է��� ��ġ�� �ùķ����� ������ �غ��մϴ�.\n");
    printf("g\t\t\t\t:����pc��ġ���� �ùķ����Ͱ� ��ɾ ������ ó���մϴ�.\n");
    printf("s\t\t\t\t:��ɾ� �ϳ��� ó���ϰ� ����� ����� ��� ���·� �����մϴ�.\n");
    printf("m start end\t\t\t:start~end������ �޸� ������ ����մϴ�.\n");
    printf("r\t\t\t\t:���� ���������� ������ ����մϴ�.\n");
    printf("x\t\t\t\t:�ùķ����� ���α׷��� �����մϴ�.\n");
    printf("sr register number value\t:Ư�� ���������� ���� �����մϴ�.\n");
    printf("sm location value\t\t:�޸� Ư�� �ּ��� ���� �����մϴ�.\n");
    printf("---------------------------------------------------------------------------------------------\n");
}

/*�ùٸ��� ���� ���� Ȯ�� �Լ�*/
int checkArgument1(int lenCode, char type){ //���ڰ� 1���� ��ɾ��
    int result = 0;

    if(lenCode >= 2){
        printf("Error: ��ɾ��� ������ �����ּ���.\n");

        switch (type)
        {

        case 'g':
            printf("\tex) g");
            result = 1;
            break;

        case 's':
            printf("\tex) s");
            result = 1;
            break;

        case 'r':
            printf("\tex) r");
            result = 1;
            break;

        default:
            break;
        }
    }

    return result;
}

int checkArgument2(int lenCode, char type){ //���ڰ� 2���� ��ɾ��
    int result = 0;

    switch (type)
    {
    case 'l':
        if(lenCode == 2){ //������ ���� �������� ������ ���
            break;
        }
        printf("Error: ��ɾ��� ������ �����ּ���.\n");
        printf("\tex) l ���������ǰ�ο��̸�");
        break;

    case 'j':
        if(lenCode == 2){ //������ ���� �������� ������ ���
            break;
        }
        printf("Error: ��ɾ��� ������ �����ּ���.\n");
        printf("\tex) j ���α׷�������ġ");
        result = 1;
        break;

    default:
        break;
    }
    

    return result;
}

int checkArgument3(int lenCode, int type){ //���ڰ� 3���� ��ɾ��
    int result = 0;

    switch (type)
    {
/*m ��ɾ�*/
    case 1:
        if(lenCode == 3){ //������ ���� �������� ������ ���
            break;
        }
        printf("Error: ��ɾ��� ������ �����ּ���.\n");
        printf("\tex) m �����ּ� �����ּ�");
        result = 1;
        break;

/*sr ��ɾ�*/
    case 2:
        if(lenCode == 3){ //������ ���� �������� ������ ���
            break;
        }
        printf("Error: ��ɾ��� ������ �����ּ���.\n");
        printf("\tex) sr �������͹�ȣ �����Ұ�");
        result = 1;
        break;  

/*sm ��ɾ�*/
    case 3:
        if(lenCode == 3){ //������ ���� �������� ������ ���
            break;
        }
        printf("Error: ��ɾ��� ������ �����ּ���.\n");
        printf("\tex) sm �޸��ּ� �����Ұ�");
        result = 1;
        break;                  
    default:
        break;
    }

    return result;
}
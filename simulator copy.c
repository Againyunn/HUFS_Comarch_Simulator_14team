#define _CRT_SECURE_NO_WARNINGS
#define M_SIZE 1000
#define REG_SIZE 32
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
//#include <cstdio>


/*����� ��������*/
char fileName[100];
const int check = 1;
static FILE* pFile = NULL;
static int continueTask = 1;
static unsigned int R[32], PC;    // ��� �������Ϳ� PC �����Ҷ� ���� �Լ�
static unsigned char progMEM[0x100000], dataMEM[0x100000], stakMEM[0x100000];
static unsigned int var = 0xAABBCCDD; //MEM �ʱ�ȭ�� Ȱ���� ����



unsigned char* rTypeName(int fct);
unsigned char* iTypeName(int opc, int* isImmediate);
unsigned char* getInstName(int opc, int fct, int* isImmediate);
//char* getOp(int opc);

char* regArr[32] = { "$zero","$at","$v0","$v1","$a0","$a1","$a2","$a3",
"$t0","$t1","$t2","$t3","$t4","$t5","$t6","$t7",
"$s0","$s1","$s2","$s3","$s4","$s5","$s6","$s7",
"$t8","$t9","$k0","$k1","$gp","$sp","$s8","$ra" };

/*�� format�� ���� ����ü �����̴�. �����ڷ� ������.
RI = r-format ����ü
II = i-format ����ü
JI = j-format ����ü*/
union itype {
	unsigned int I;
	struct rFormat {
		unsigned int opcode : 6;
		unsigned int rs : 5;
		unsigned int rt : 5;
		unsigned int rd : 5;
		unsigned int funct : 6;
	}RI;
	struct iFormat {
		unsigned int opcode : 6;
		unsigned int rs : 5;
		unsigned int rt : 5;
		unsigned int offset : 16;
	}II;
	struct jFormat {
		unsigned int opcode : 6;
		unsigned int jumpAddr : 26;
	}JI;
}IR;
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





void openBinaryFile(char* filePath);   // l ��ɾ� ����� filePath�� �޾Ƽ� ���̳ʸ� ���� ���� �Լ�
unsigned int To_BigEndian(unsigned int x);  // �򿣵�� ���� �Լ� => hex��
unsigned char getOp(int opc);  // opcode Ȯ�� �Լ�
// binary to decimal �� ���� int������ ������
//unsigned char* getInstName(int opc, int fct, int* isImmediate);   // debugging �Լ�
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

    //�ùķ����� ���� ���
    printNotice();

    //�������� �ʱ�ȭ �Լ�
    // Initialize
	initializeRegister();

    while(1){
        //COMAND ���� �ʱ�ȭ
        lenCode = 0; //��ɾ��� ���� ��
        cmdLen = 0; //��ɾ��� �ڸ���(1~2�ڸ� �ĺ���)
        cmdErr = 0;
        //char *cmdArr[10] = {NULL, };
    /*����Է¹ޱ�*/
        printf("Enter a command.\n>>> ");
        gets(cmdLine);
        // ���� ���ڸ� �������� ���ڿ��� �ڸ��� ������ ��ȯ
        char* ptr = strtok(cmdLine, " ");

        // while (ptr != NULL)            // �ڸ� ���ڿ��� ������ ���� ������ �ݺ�
        // {
        //     cmdArr[lenCode] = ptr;      // ���ڿ��� �ڸ� �� �޸� �ּҸ� ���ڿ� ������ �迭�� ����
        //     lenCode++;

        //     ptr = strtok(NULL, " ");   // ���� ���ڿ��� �߶� �����͸� ��ȯ
        // }

        // if(cmdArr[0] != NULL){
        //     cmdLen = strlen(cmdArr[0]);
        // }

		// ��ɾ� �ڵ�
		char* cmdArr = ptr;
		if (ptr != NULL) {  // ��ɾ null�� �ƴϸ� ��ȯ
			// ��ɾ� �ڵ� ���� ��
			cmdLen = strlen(cmdArr);
		}
        if(cmdLen == 1){ //��ɾ �ѱ����� ��
            switch (*cmdArr){
        /*l ��ɾ�*/
            case 'l':
                if(checkArgument2(lenCode, 'l') == 1) //��ɾ� ��ȿ���˻�
                    break;
                // load program
				// ptr�� Filename ���ڿ��� ����Ų��.
				ptr = strtok(NULL, " ");
				char* filePath = NULL;
				if (ptr == NULL) {
					printf("Error: Not enough arguments.\n");
					printf("ex) l C:\\pub\\as_ex01_arith.bin\n");
				}
				else {
					filePath = ptr;
					openBinaryFile(filePath);
				}
				break;


                break;
        /*j ��ɾ�*/
            case 'j':
                if(checkArgument2(lenCode, 'j') == 1) //��ɾ� ��ȿ���˻�
                    break;

                // jump, �Է��� ��ġ���� simulator ������ �غ��Ѵ�.
				// ptr�� ���α׷� ���� ��ġ ���ڿ��� ����Ų��.
				ptr = strtok(NULL, " ");
				char* newAddr = NULL;
				if (ptr == NULL) {
					printf("Error: Not enough arguments.\n");
					printf("ex) j 0x40000000\n");
				}
				else {
					newAddr = ptr;
					updatePC(strtol(newAddr, NULL, 16));
				}
				break;

        /*g ��ɾ�*/
            case 'g':
                if(checkArgument1(lenCode, 'g') == 1) //��ɾ� ��ȿ���˻�
                    break;

                /*Go program, ����PC��ġ���� simulator�� ��ɾ ������ ó���Ѵ�.
				�̶� ���Ǵ� �Լ��� startGoTask()�̴�.*/
				if (pFile != NULL) {
					startGoTask();
				}
				else {
					printf("Error: Load Binary File in advance!\n");
				}
				break;

        /*s ��ɾ�*/
            case 's':{
                if(checkArgument1(lenCode, 's') == 1) //��ɾ� ��ȿ���˻�
                    break;

                /* Step, ��ɾ ���Ͽ� ����� ��������, �޸� ������ ����Ѵ�.
				�̶� ���Ǵ� �Լ��� startStepTask()�̴�.*/
				if (pFile != NULL) {
					startStepTask();
					showRegister();
				}
				else {
					printf("Error: Load Binary File in advance!\n");
				}
				break; }
        /*m ��ɾ�*/
            case 'm':
                if(checkArgument3(lenCode, 1) == 1) //��ɾ� ��ȿ���˻�
                    break;

               // View memory
				// ptr�� start ���ڿ��� ����Ų��., startAddr~endAddr������ �޸� ���� ���
				ptr = strtok(NULL, " ");

				if (ptr == NULL) {
					printf("Error: Not enough arguments.\n");
					printf("ex) m 0x10000000 0x1000FFFF\n");
				}
				else {
					char* start = ptr;

					// ptr�� end ���ڿ��� ����Ų��.
					ptr = strtok(NULL, " ");
					if (ptr == NULL) {
						printf("Error: Not enough arguments.\n");
						printf("ex) m 0x10000000 0x1000FFFF\n");
					}
					else {
						char* end = ptr;

						unsigned int startAddr = strtol(start, NULL, 16);
						unsigned int endAddr = strtol(end, NULL, 16);

						for (unsigned int i = startAddr; i <= endAddr; i = i + 4) {
							printf("[0x%08x] => 0x%x\n", i, MEM(i, var, 0, 2));
						}
					}
				}
				break;
        /*r ��ɾ�*/
            case 'r':
                if(checkArgument1(lenCode, 'r') == 1) //��ɾ� ��ȿ���˻�
                    break;

                /* View register, ���� �������� ���� ���
				�̶� ���Ǵ� �Լ��� showRegister()�̴�.*/
				showRegister();
				break;

        /*x ��ɾ�*/
            case 'x':
                printf("Terminate program.\n");
                exit(1);
                break;

        /*���ǵ��� ���� ��ɾ� ����ó��: ��ɾ� 1��¥��*/
            default:
                printf("Error: Enter a valid command.");
                break;
            }

        }
        else if(cmdLen == 2)
        {
			if (strcmp(cmdArr, "sr") == 0) {
				// Ư�� ���������� �� ����
				char* regNum = NULL;
				char* regVal = NULL;
				// ptr�� register number ���ڿ��� ����Ų��.
				ptr = strtok(NULL, " ");

				if (ptr == NULL) {
					printf("Error: Not enough arguments.\n");
					printf("ex) sr 1 20\n");
				}
				else {
					regNum = ptr;
					// ptr�� value ���ڿ��� ����Ų��.
					ptr = strtok(NULL, " ");
					if (ptr == NULL) {
						printf("Error: Not enough arguments.\n");
						printf("ex) sr 1 20\n");
					}
					else {
						regVal = ptr;
						setRegister(atoi(regNum), strtol(regVal, NULL, 16));
					}
				}
			}
			else if (strcmp(cmdArr, "sm") == 0) {
				// �޸� Ư�� �ּ��� �� ����
				// ptr�� start ���ڿ��� ����Ų��.
				ptr = strtok(NULL, " ");

				if (ptr == NULL) {
					printf("Error: Not enough arguments.\n");
					printf("ex) sm 0xFFFFFFFF 20\n");
				}
				else {
					printf("OK\n");
					char* memLoc = ptr;

					// ptr�� end ���ڿ��� ����Ų��.
					ptr = strtok(NULL, " ");
					if (ptr == NULL) {
						printf("Error: Not enough arguments.\n");
						printf("ex) sm 0xFFFFFFFF 20\n");
					}
					else {
						printf("OK\n");
						char* memVal = ptr;
					}
				}
			}
			else {
				printf("Error: Invalid command arguments.\n");
			}
		}
         else{
            printf("Error: Enter a valid command.");
        }

        printf("\n\n");
    }
    return 0;
}
        // { //��ɾ �α����� ��

        // /*sr ��ɾ�*/
        //     if(!strcmp(cmdArr[0], "sr")){
        //         if(checkArgument3(lenCode, 2) == 1){ //��ɾ� ��ȿ���˻�
        //             printf("\n\n");
        //             continue;
        //         }
        //         else{
        //             //�Լ�����
        //         }
        //     }

        // /*sm ��ɾ�*/
        //     else if(!strcmp(cmdArr[0], "sm")){
        //         if(checkArgument3(lenCode, 3) == 1){ //��ɾ� ��ȿ���˻�
        //             printf("\n\n");
        //             continue;
        //         }
        //         else{
        //             //�Լ�����
        //         }
        //     }
        // /*���ǵ��� ���� ��ɾ� ����ó��: ��ɾ� 2��¥��*/
        //     else{
        //         printf("Error: �ùٸ� ��ɾ �Է����ּ���.");
        //     }

        // }
    /*���ǵ��� ���� ��ɾ� ����ó��: ��ɾ� �Է�x�� ��� + ���ǵ��� ���� ��ɾ��� ���*/

//�������̽� 's'����� ��ȯ�Ǵ� �Լ�
void startStepTask() {
	printf("current value : %x\n", MEM(PC, var, 0, 2));
	unsigned instBinary = MEM(PC, var, 0, 2);
	PC = PC + 4;
	/* Instruction Decode */
	// ��ɾ� Ÿ��(R, I, J) üũ ��
	// ��ɾ� type�� ���� �б��Ͽ� ����
	switch (getOp((instBinary >> 26) & 0x3F))
	{
	case 'R':
		// R-Format ����, opcode ����
		IR.RI.opcode = (instBinary >> 26) & 0x3F;
		// rs ����
		IR.RI.rs = (instBinary >> 21) & 0x1F;
		// rt ����
		IR.RI.rt = (instBinary >> 16) & 0x1F;
		// rd ����
		IR.RI.rd = (instBinary >> 11) & 0x1F;
		// funct ����
		IR.RI.funct = instBinary & 0x3F;

		instExecute(IR.RI.opcode, IR.RI.funct, NULL);

		// ��ɾ� ���п� ���� ��� ��� ��ȭ
		if (IR.RI.opcode == 0 && IR.RI.funct == 12) {
			// syscall ��ɾ� case
			printf("%s\n\n", getInstName(IR.RI.opcode, IR.RI.funct, NULL));
			continueTask = 0;
		}
		else if (IR.RI.opcode == 0 && IR.RI.funct == 8) {
			// jr ��ɾ� case
			printf("%s %s\n\n", getInstName(IR.RI.opcode, IR.RI.funct, NULL), regArr[IR.RI.rs]);
		}
		else {
			printf("%s %s %s %s\n\n", getInstName(IR.RI.opcode, IR.RI.funct, NULL), regArr[IR.RI.rd], regArr[IR.RI.rs], regArr[IR.RI.rt]);
		}
		break;
	case 'I':
		// I-Format ����, opcode ����
		IR.II.opcode = (instBinary >> 26) & 0x3F;
		int isImmediate = 0; // immediate ���̸� 1�� �ٲ�-----------------------------------------------------------------------------------------
		// rs ����
		IR.II.rs = (instBinary >> 21) & 0x1F;
		// rt ����
		IR.II.rt = (instBinary >> 16) & 0x1F;
		// offset/immediate value ����
		IR.II.offset = instBinary & 0xFFFF;

		instExecute(IR.II.opcode, 0, &isImmediate); //void instExecute(int opc, int fct, int* isImmediate) opcode�� �� fct�� 0���� ����

		// offset���� immediate value ������ ���� ��� ��� ��ȭ
		printf("%s", getInstName(IR.II.opcode, 0, &isImmediate));
		if (isImmediate == 1) {
			printf(" %s %s %d\n\n", regArr[IR.II.rt], regArr[IR.II.rs], IR.II.offset);
		}
		else {
			printf(" %s %d(%s)\n\n", regArr[IR.II.rt], IR.II.offset, regArr[IR.II.rs]);
		}
		break;
	case 'J':
		// J-Format ����, opcode ����
		IR.JI.opcode = (instBinary >> 26) & 0x3F;
		// jump target address ����
		IR.JI.jumpAddr = instBinary & 0x3FFFFFF;

		instExecute(IR.JI.opcode, 0, NULL);

		// ��� ���
		printf("%s %d\n\n", getInstName(IR.JI.opcode, 0, NULL), IR.JI.jumpAddr);
		break;
	default:
		break;
	}
}
//�������̽� 'g'����� ��ȯ�Ǵ� �Լ�
void startGoTask() {

	while (continueTask) {
		/* Instruction Fetch */
		printf("current value : %x\n", MEM(PC, var, 0, 2));
		unsigned instBinary = MEM(PC, var, 0, 2);
		PC = PC + 4;
		/* Instruction Decode */
		// ��ɾ� Ÿ��(R, I, J) üũ ��
		// ��ɾ� type�� ���� �б��Ͽ� ����
		switch (getOp((instBinary >> 26) & 0x3F))
		{
		case 'R':
			// R-Format ����, opcode ����
			IR.RI.opcode = (instBinary >> 26) & 0x3F;
			// rs ����
			IR.RI.rs = (instBinary >> 21) & 0x1F;
			// rt ����
			IR.RI.rt = (instBinary >> 16) & 0x1F;
			// rd ����
			IR.RI.rd = (instBinary >> 11) & 0x1F;
			// funct ����
			IR.RI.funct = instBinary & 0x3F;

			instExecute(IR.RI.opcode, IR.RI.funct, NULL);


			// ��ɾ� ���п� ���� ��� ��� ��ȭ (For Debugging)
			if (IR.RI.opcode == 0 && IR.RI.funct == 12) {
				// syscall ��ɾ� case
				printf("%s\n\n", getInstName(IR.RI.opcode, IR.RI.funct, NULL));
				continueTask = 0;
			}
			else if (IR.RI.opcode == 0 && IR.RI.funct == 8) {
				// jr ��ɾ� case
				printf("%s %s\n\n", getInstName(IR.RI.opcode, IR.RI.funct, NULL), regArr[IR.RI.rs]);
			}
			else {
				printf("%s %s %s %s\n\n", getInstName(IR.RI.opcode, IR.RI.funct, NULL), regArr[IR.RI.rd], regArr[IR.RI.rs], regArr[IR.RI.rt]);
			}

			break;
		case 'I':
			// I-Format ����, opcode ����
			IR.II.opcode = (instBinary >> 26) & 0x3F;
			int isImmediate = 0; // immediate ���̸� 1�� �ٲ�
			// rs ����
			IR.II.rs = (instBinary >> 21) & 0x1F;
			// rt ����
			IR.II.rt = (instBinary >> 16) & 0x1F;
			// offset/immediate value ����
			IR.II.offset = instBinary & 0xFFFF;

			instExecute(IR.II.opcode, 0, &isImmediate);


			// offset���� immediate value ������ ���� ��� ��� ��ȭ (For Debugging)
			printf("%s", getInstName(IR.II.opcode, 0, &isImmediate));
			if (isImmediate == 1) {
				printf(" %s %s %d\n\n", regArr[IR.II.rt], regArr[IR.II.rs], IR.II.offset);
			}
			else {
				printf(" %s %d(%s)\n\n", regArr[IR.II.rt], IR.II.offset, regArr[IR.II.rs]);
			}

			break;
		case 'J':
			// J-Format ����, opcode ����
			IR.JI.opcode = (instBinary >> 26) & 0x3F;
			// jump target address ����
			IR.JI.jumpAddr = instBinary & 0x3FFFFFF;

			instExecute(IR.JI.opcode, 0, NULL);


			// ��� ��� (For Debugging)
			printf("%s %d\n\n", getInstName(IR.JI.opcode, 0, NULL), IR.JI.jumpAddr);

			break;
		default:
			break;
		}
	}
}
//�������̽� 'r'����� ��ȯ�Ǵ� �Լ�
void showRegister() {
	// 16������ ���
	printf("[REGISTER]\n");
	for (int i = 0; i < REG_SIZE; i++) {
		printf("$%d=\t0x%x\n", i, R[i]);
	}
	printf("PC=\t0x%x\n", PC);
}
//�ùķ����� ���� ����Լ�
void printNotice(){
    printf("\t\t\t*Command Input Format*\n");
    printf("l Executable File Path\t\t:The file will be uploaded to the simulator memory.\n");
    printf("j Program Start Location\t:Prepare to run the simulator at the location you entered\n");
    printf("g\t\t\t\t:At the current pc location, the simulator handles the command to the end.\n");
    printf("s\t\t\t\t:Process one command and stop to receive user commands.\n");
    printf("m StartAddress EndAddress\t:Print the memory contents of the start~end range.\n");
    printf("r\t\t\t\t:Print the contents of the current register.\n");
    printf("x\t\t\t\t:Terminate the simulator program.\n");
    printf("sr Register Number Value\t:Set the value of a particular register.\n");
    printf("sm Location Value\t\t:Set the value of a memory-specific address.\n");
    printf("---------------------------------------------------------------------------------------------\n");
}
/*Instruction Decode�ܰ� => getOp() = instruction�� Op code, �� operation�� ������ ��ȯ�ϴ� �Լ�
0�̸� R-type, 2�Ǵ�3�̸� J-type, �� �ܴ� I-type���� ó����.
instExecute() = op, function code�� ���� ��ɾ �з��ϰ� �ش�Ǵ� ������ �����ϴ� �Լ��̴�.*/
unsigned char getOp(int opc) {
	char format;
	// R-Format instruction
	if (opc == 0) {
		format = 'R';
	}
	// J-Format instruction
	else if ((opc == 2) || (opc == 3)) {
		format = 'J';
	}
	// I-Format instruction
	else {
		format = 'I';
	}
	return format;
}
/*�ùٸ��� ���� ���� Ȯ�� �Լ�*/
int checkArgument1(int lenCode, char type){ //���ڰ� 1���� ��ɾ��
    int result = 0;

    if(lenCode >= 2){
        printf("Error: Keep the format of the command.\n");

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
        printf("Error: Keep the format of the command.\n");
        printf("\tex) l Path of Executable File Name");
        break;

    case 'j':
        if(lenCode == 2){ //������ ���� �������� ������ ���
            break;
        }
        printf("Error: Keep the format of the command.\n");
        printf("\tex) j Program Start Location");
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
        printf("Error: Keep the format of the command.\n");
        printf("\tex) m startAddress endAddress");
        result = 1;
        break;

/*sr ��ɾ�*/
    case 2:
        if(lenCode == 3){ //������ ���� �������� ������ ���
            break;
        }
        printf("Error: Keep the format of the command.\n");
        printf("\tex) sr register number value");
        result = 1;
        break;

/*sm ��ɾ�*/
    case 3:
        if(lenCode == 3){ //������ ���� �������� ������ ���
            break;
        }
        printf("Error: Keep the format of the command.\n");
        printf("\tex) sm location value");
        result = 1;
        break;
    default:
        break;
    }

    return result;
}
//----------------------------------------------------------------             ----------------------------------------------------------------------
//     l filePath
unsigned char* getInstName(int opc, int fct, int* isImmediate) {  // �����

	// int val = instruction->inst;
	// int opc = val >> 26;
	// int fct = val & 0x3f;

	switch (opc) {
		case 0:   	// R-Type ��ɾ�
			return rTypeName(fct);
		case 2:   	// J-Type ��ɾ�
			return "j";        /////
		case 3:		// J-Type ��ɾ�
			return "jal";   //////
		default:	// I-Type ��ɾ�
			return iTypeName(opc, isImmediate);
	}
}
// ���̳ʸ� ���� ���� �Լ�   -> l��ɾ�
// �������� �ʱ�ȭ
void initializeRegister() {
	for (int i = 0; i < REG_SIZE; i++) {
		// 32bit
		R[i] = 0x00000000;
	}
	// PC �ʱⰪ ����
	PC = 0x00400000;
	// SP �ʱⰪ ����
	R[29] = 0x80000000;
}
void openBinaryFile(char* filePath) {
	//err = fopen_s(&pFile, "as_ex01_arith.bin", "rb");
	//err = fopen_s(&pFile, "as_ex02_logic.bin", "rb");
	//err = fopen_s(&pFile, "as_ex03_ifelse.bin", "rb");

	// File Validation TEST

    // FILE* testFile = NULL;
    //--------------------------------------------------------�̺κ� ��ġ�� file ������
	FILE* testFile = fopen( filePath, "rb");
	if (testFile == NULL) {
		printf("Cannot open file\n");
		return;// return 1���� ����
	}
	unsigned int data;
	unsigned int data1 = 0xAABBCCDD;
	if (fread(&data, sizeof(data1), 1, testFile) != 1)
		exit(1);
	fclose(testFile);

	// Load Real File
	fopen( filePath, "rb");
	printf("The Binary File Has Been Loaded Successfully.\n");

	// Load Init Task (�޸� ����)
	loadInitTask();
}
/*To_BigEndian = �����Ͱ� ������ ū ������ ������ ������ ����� �Լ�.
������������ ������Ʈ�� �� ���� ���� Ŀ���� ������ �������� �� ���� ������ Ŀ����.*/
unsigned int To_BigEndian(unsigned int x)
{
	unsigned int result = (x & 0xFF) << 24;

	result |= ((x >> 8) & 0xFF) << 16;
	result |= ((x >> 16) & 0xFF) << 8;
	result |= ((x >> 24) & 0xFF);

	return result;
}

/*Instruction Fetch�ܰ� =>loadInintTask() = ���̳ʸ� ������ load�ϰ� �޸𸮿� �����ϴ� �۾��� ����ϴ� �Լ�*/
void loadInitTask() {
	updatePC(0x400000);
	setRegister(29, 0x80000000);

	//printf("\n%s\n", loadedFilePath);
	unsigned int data;
	unsigned int data1 = 0xAABBCCDD;
	unsigned int numInst;
	unsigned int numData;

	// Read the number of Instructions
	fread(&numInst, sizeof(data1), 1, pFile);
	numInst = To_BigEndian(numInst);
	// Read the number of Datas
	fread(&numData, sizeof(data1), 1, pFile);
	numData = To_BigEndian(numData);

	printf("size of Instructions : %d\n", numInst);
	printf("size of Datas : %d\n", numData);

	unsigned int memAddr = 0x00400000;
	unsigned int dataAddr = 0x10000000;

	for (int i = 0; i < numInst; i++) {
		if (fread(&data, sizeof(data1), 1, pFile) != 1)
			exit(1);
		// ��ɾ� �޸� ����
		data = To_BigEndian(data);
		printf("Instruction = %08x\n", data);

		MEM(memAddr, data, 1, 2);
		memAddr = memAddr + 4;
	}

	for (int i = 0; i < numData; i++) {
		if (fread(&data, sizeof(data1), 1, pFile) != 1)
			exit(1);
		data = To_BigEndian(data);
		// ������ �޸� ����
		printf("Data = %08x\n", data);

		MEM(dataAddr, data, 1, 2);
		dataAddr = dataAddr + 4;
	}
}

//���� pc���� ���ϴ� ������ �����ϴ� �Լ��̴�.
void updatePC(unsigned int addr) {
	PC = addr;
}
//���ϴ� �������� ���� ������ �� �ִ� �Լ�.
void setRegister(unsigned int regNum, unsigned int val) {

	R[regNum] = val;
}

//���ϴ� ������ �ش� �޸𸮿� �����Ͽ� ���� �����ϴ� �Լ�.
void setMemory(char* offset, char* val) {

	R[atoi(offset)] = strtol(val, NULL, 16);
}


//Memory Access �κ��̴�. int MEM(unsigned int A, int V, int nRW, int S); // memory access�Լ�
int MEM(unsigned int A, int V, int nRW, int S) {
	unsigned int sel, offset;
	unsigned char* pM;
	sel = A >> 20;
	offset = A & 0xFFFFF;

	if (sel == 0x004) pM = progMEM;			// Program memory
	else if (sel == 0x100) pM = dataMEM;	// Data memory
	else if (sel == 0x7FF) pM = stakMEM;	// Stack
	else {

		printf("No memory\n");
		// ���� ���̽� �׽�Ʈ�� ���� ��ü ���α׷��� �������� �ʰ�
		// �Լ��� �����Ѵ�
		return 1;
	}

	if (S == 0) {
		// Byte
		if (nRW == 0) {
			// Read
			return pM[offset];
		}
		else if (nRW == 1) {
			// Write
			pM[offset] = V;
			return 1;
		}
		else {
			printf("nRW input error\n");
			return 1;
			//exit(1);
		}
	}
	else if (S == 1) {
		// Half word
		if (offset % 2 != 0)	// Half-word-aligned Check
		{
			printf("Not an half-word-aligned address input!\n");
			return 1;
			//exit(1);
		}
		if (nRW == 0) {
			// Read
			int result = (pM[offset] << 8) + (pM[offset + 1]);
			return result;
		}
		else if (nRW == 1) {
			// Write
			pM[offset] = (V >> 8) & 0xFF;
			pM[offset + 1] = V & 0xFF;
		}
		else {
			printf("nRW input error\n");
			return 1;
			//exit(1)
		}
	}
	else if (S == 2) {
		// Word
		if (offset % 4 != 0)	// Word-aligned Check
		{
			printf("Not an word-aligned address input!\n");
			return 1;
			//exit(1)
		}
		if (nRW == 0) {
			// Read
			int result = (pM[offset] << 24) + (pM[offset + 1] << 16) + (pM[offset + 2] << 8) + (pM[offset + 3]);
			return result;
		}
		else if (nRW == 1) {
			// Write
			pM[offset] = (V >> 24) & 0xFF;
			pM[offset + 1] = (V >> 16) & 0xFF;
			pM[offset + 2] = (V >> 8) & 0xFF;
			pM[offset + 3] = V & 0xFF;
		}
		else {
			printf("nRW input error\n");
			return 1;
			//exit(1)
		}
	}
	else {//S�� ��ȿ���� ���� ���� ��� ����ó��
		printf("Size input error\n");
		return 1;
		//exit(1)
	}
}
/*R-format + (I-format �Ǵ� J-format)��� 2���� format���� ��������.
switch���� ����� case���� ��ɾ� ó���ߴ�.
�� instruction�� MIPS simulator �����ڷ� ������*/
void instExecute(int opc, int fct, int* isImmediate) {
	//zero flag ����
	int Z;
	Z = 0;

	int sub;

    if(opc != 0){
        // I-Format �Ǵ� J-Format �� ���
        switch(opc){
            case 1:
			// bltz
			// 0���� ������ �̵�

			

			// ALU�� checkSetLess����(0�� ��)
			// if���� ���� 1, 0�� �����ص� �Ǵ��� �𸣰ڽ��ϴ�.
			if(ALU(R[IR.RI.rs], 0, 0x4, &Z)) { 
				// 32bit�� sign extension �� immediate ����� << 2 + ( PC + 4 )  
				//---> mips������ PC�� ��Ʈ���� offset�� ��Ʈ���� �ٸ��⋚���� offset�� 32��Ʈ�� ���� ����Ѵٰ� �ϴµ� C������ ��� ó���Ǵ��� �𸣰ڽ��ϴ�. (bltz, beq, bne)
				updatePC((MEM(R[IR.II.rs] + IR.II.offset, var, 0, 2)<<2)+(PC+4)); 
				break;
			}
			else {
				updatePC(PC+4);
				break;
			}
			
            case 2:
            //j
            updatePC(IR.JI.jumpAddr);  // L�� �̵�
            break;
            case 3:

            case 4:
			// beq
			// ������ �̵�

			// ���� sub�������� �ΰ��� �������Ͱ��� ������ Ȯ���Ͽ��� (������ = 0, �ٸ��� != 0)
			// checkZero�Լ��� 1, 0�� �Ǻ��ϵ��� �Ͽ��µ� ���� �Լ��� ������ �Ǻ��ص� �Ǵ��� Ȥ �ܼ��� if�������� �Ǻ��ص��Ǵ��� �𸣰ڽ��ϴ�.
			
			
			sub = ALU(R[IR.RI.rs], R[IR.RI.rt], 0x9, &Z);   //ALU�� sub����


			if(checkZero(sub)) { // if sub ==0 , 32bit�� sign extension �� immediate ����� << 2 + ( PC + 4 ) 
				updatePC((MEM(R[IR.II.rs] + IR.II.offset, var, 0, 2)<<2)+(PC+4)); 
				break;
			}
			else {  // if sub != 0 , ���� ��ɾ� ������ ���� PC + 4�� ���ش�.
				updatePC(PC+4);
				break;
			}
			

            case 5:
			// bne
			// �ٸ��� �̵�

			// ���� sub�������� �ΰ��� �������Ͱ��� ������ Ȯ���Ͽ��� (������ = 0, �ٸ��� != 0)
			// checkZero�Լ��� 1, 0�� �Ǻ��ϵ��� �Ͽ��µ� ���� �Լ��� ������ �Ǻ��ص� �Ǵ��� Ȥ �ܼ��� if�������� �Ǻ��ص��Ǵ��� �𸣰ڽ��ϴ�.
			
			sub = ALU(R[IR.RI.rs], R[IR.RI.rt], 0x9, &Z);   //ALU�� sub����

			if(!(checkZero(sub))) { 		// if sub !=0 , 32bit�� sign extension �� immediate ����� << 2 + ( PC + 4 )
				updatePC((MEM(R[IR.II.rs] + IR.II.offset, var, 0, 2)<<2)+(PC+4)); 
				break;
			}
			else {  // if sub == 0 , ���� ��ɾ� ������ ���� PC + 4�� ���ش�.
				updatePC(PC+4);
				break;
			}

            case 8:
			// addi
				
				R[IR.RI.rt] = ALU(R[IR.RI.rs], MEM(R[IR.II.rs] + IR.II.offset, var, 0, 2), 0x8, &Z);   //ALU�� addi����
				break;

            case 10:
			// slti
				
				R[IR.RI.rt] = ALU(R[IR.RI.rs], MEM(R[IR.II.rs] + IR.II.offset, var, 0, 2), 0x4, &Z);   // ALU�� checkSetLess����
				break;

            case 12: 
				//andi
				
				R[IR.II.rt] = MEM(R[IR.II.rs] + IR.II.offset, var, 0, 2); //�޸𸮿��� �����i �޾ƿ���
				R[IR.RI.rd] = ALU(R[IR.RI.rs], R[IR.II.rt], 0x8, &Z);//ALU�� addi����
				//R[IR.RI.rt] = ALU(R[IR.RI.rs], MEM(R[IR.II.rs] + IR.II.offset, NULL, 0, 2), 12, &Z);
            case 13:
				//ori
				
				R[IR.II.rt] = MEM(R[IR.II.rs] + IR.II.offset, var, 0, 2); //�޸𸮿��� �����i �޾ƿ���
				R[IR.RI.rd] = ALU(R[IR.RI.rs], R[IR.II.rt], 0xb, &Z);//ALU�� ori����
			case 14:
				//xori
			
				R[IR.II.rt] = MEM(R[IR.II.rs] + IR.II.offset, var, 0, 2); //�޸𸮿��� �����i �޾ƿ���
				R[IR.RI.rd] = ALU(R[IR.RI.rs], R[IR.II.rt], 0xc, &Z);//ALU�� ori����
            case 15:
            case 32:
			// lb

            case 35:
				// lw
				R[IR.II.rt] = MEM(R[IR.II.rs] + IR.II.offset, var, 0, 2);
			break;
            case 36:
            case 40:
            case 43:
				// sw
				MEM(R[IR.II.rs] + IR.II.offset, R[IR.II.rt], 1, 2);
				break;
            default:
				// not found
				break;
        }
    }else {
		// R-Format �� ���
		switch (fct) {
		case 0: {
			// sll
			
			R[IR.RI.rd] = ALU(R[IR.RI.rs], R[IR.RI.rt], 0x1, &Z);
			break; }
		case 2: {
			// srl
			
			R[IR.RI.rd] = ALU(R[IR.RI.rs], R[IR.RI.rt], 0x2, &Z);
			break; }
		case 3: {
			// sra
			
			R[IR.RI.rd] = ALU(R[IR.RI.rs], R[IR.RI.rt], 0x3, &Z);
			break; }
		case 8:
			// jr
			updatePC(R[31]);	// go to $ra
			break;
		case 12:
			// syscall
			continueTask = 0;
			break;
		case 16:
			// mfhi
			break;
		case 18:
			// mflo
			break;
		case 24:
			// mul
			
			break;
		case 32: {
			// add
			
			R[IR.RI.rd] = ALU(R[IR.RI.rs], R[IR.RI.rt], 0x8, &Z);
			break; }
		case 34: {
			// sub
			
			R[IR.RI.rd] = ALU(R[IR.RI.rs], R[IR.RI.rt], 0x9, &Z);
			break; }
		case 36: {
			// and
			
			R[IR.RI.rd] = ALU(R[IR.RI.rs], R[IR.RI.rt], 12, &Z);
			break; }
		case 37: {
			// or
			
			R[IR.RI.rd] = ALU(R[IR.RI.rs], R[IR.RI.rt], 13, &Z);
			break; }
		case 38: {
			// xor
		
			R[IR.RI.rd] = ALU(R[IR.RI.rs], R[IR.RI.rt], 14, &Z);
			break; }
		case 39: {
			// nor
			
			R[IR.RI.rd] = ALU(R[IR.RI.rs], R[IR.RI.rt], 15, &Z);
			break; }
		case 42: {
			// slt
		
			R[IR.RI.rd] = ALU(R[IR.RI.rs], R[IR.RI.rt], 4, &Z);
			break; }
		default:
			// NOT FOUND!
			break;
		}
	}
}

// ex ) add $t1, $t2, $t3
int ALU(int X, int Y, int C, int* Z) {
    // X = 4-bit input number
    // Y = 4-bit input number
    // C = carry into LSB position

    // Z = Zero Flag
    //Zero Flag:  This bit is updated as a result of all operations.
    //If the result of an operation is zero, then Z is asserted.
    //If the result of an operation is not zero, then Z is 0.
	int c32, c10;
	int ret;

	c32 = (C >> 2) & 3;
	c10 = C & 3;
	if (c32 == 0) {
		//shift
		ret = shiftOperation(X, Y, c10);   //ALU control input {0,1,2,3}  -> {0,1,2,3}  >> 2 == 0 -> 0 & 3 == 0
	}
	else if (c32 == 1) {  //ALU control input 4 -> (4 >>2) == 1 => 1 & 3 == 1(001)
		// set less
		ret = checkSetLess(X, Y);
	}
	else if (c32 == 2) {  //ALU control input 8 -> 8>>2 == 2 -> 2 & 3 == 2(010)
		// addsubtract
		ret = addSubtract(X, Y, c10);  // addSubtract�Լ����� 0�� add 1�� subtract
		*Z = checkZero(ret);  // 0 or 1
	}
	else {
		// logic      //ALU control input  15 -> (15>>2) & 3 == 3
		ret = logicOperation(X, Y, c10);
	}
	return ret;
    //ret output
}
int logicOperation(int X, int Y, int C) {
	if (C < 0 || C > 3) {
		printf("error in logic operation\n");
		exit(1);
	}
	if (C == 0) {  //ALU control 0000
		// AND
		return X & Y;
	}
	else if (C == 1) { //ALU control 0001
		// OR
		return X | Y;
	}
	else if (C == 2) { //ALU control 0010
		// XOR
		return X ^ Y;
	}
	else {    //ALU control 1100
		// NOR
		return ~(X | Y);
	}
}

int addSubtract(int X, int Y, int C) {
	int ret;
	if (C < 0 || C > 1) {
		printf("error in add/subtract operation\n");
		exit(1);
	}
	if (C == 0) {
		// add
		ret = X + Y;
	}
	else {
		// subtract
		ret = X - Y;
	}
	return ret;
}

// V is 5 bit shift amount
int shiftOperation(int V, int Y, int C) {
	int ret;
	if (C < 0 || C > 3) {
		printf("error in shift operation\n");
		exit(1);
	}
	if (C == 0) {
		// No shift : �״�� ��ȯ
		ret = V;
	}
	else if (C == 1) {
		// Logical left
		ret = V << Y;
	}
	else if (C == 2) {
		// Logical right
		ret = V >> Y;
	}
	else {
		// Arith right
		ret = V >> Y;
	}
	return ret;
}

// ���Լ��� add �Ǵ� subtract ���� �ø�
// ����Ͽ� Z���� �����Ѵ�.
int checkZero(int S) {
	int ret = 0;
	// check if S is zero,
	// and return 1 if it is zero
	// else return 0
	if (S == 0) {
		ret = 1;
	}
	return ret;
}

int checkSetLess(int X, int Y) {
	int ret;

	// check if X < Y,
	// and return 1 if it is true
	// else return 0
	if (Y > X) {
		ret = 1;
	}
	else {
		ret = 0;
	}
	return ret;
}

// ������
// rtype  0 2 3 8 12 37
// itype  1 4 5  8 10 32
// ������
// rtype  16 18 32 34 36 38
// itype  12 13 14 15 24  34
// ���ؿ�  �������� �������̽� �Լ� �ֱ�
unsigned char* rTypeName(int fct) {
	switch (fct) {
		case 0:
			return "sll"; ////14
		case 2:
			return "srl";  /////
		case 3:
			return "sra"; ////
		case 4:
			return "sllv";
		case 6:
			return "srlv";
		case 7:
			return "srav";
		case 8:
			return "jr";  ///////
		case 9:
			return "jalr";
		case 12:
			return "syscall";  ///////
		case 16:
			return "mfhi";  //////
		case 17:
			return "mthi";
		case 18:
			return "mflo";   //////
		case 19:
			return "mtlo";
		case 24:
			return "mult";
		case 25:
			return "multu";
		case 26:
			return "div";
		case 27:
			return "divu";
		case 32:
			return "add";  ////
		case 33:
			return "addu";
		case 34:
			return "sub";  /////
		case 35:
			return "subu";
		case 36:
			return "and";   //////
		case 37:
			return "or";    /////
		case 38:
			return "xor";   /////
		case 39:
			return "nor"; //////
		case 42:
			return "slt";   //////
		case 43:
			return "sltu";
		default:
			return "ERROR";
	}
}

unsigned char* iTypeName(int opc, int* isImmediate) {
	switch (opc) {
		case 1:
			return "bltz"; //// 15
        case 2:  // j
            return "j";
        case 3: // jal;
            return "jal";
		case 4:
        	*isImmediate = 1;
			return "beq";   /////
		case 5:
        	*isImmediate = 1;
			return "bne";   /////
		case 6:
			return "blez";
		case 7:
			return "bgtz";
		case 8:
        	*isImmediate = 1;
			return "addi";  /////
		case 9:
        	*isImmediate = 1;
			return "addiu";
		case 10:
        	*isImmediate = 1;
			return "slti";   /////
		case 11:
			return "sltiu";
		case 12:
        	*isImmediate = 1;
			return "andi";    /////
		case 13:
        	*isImmediate = 1;
			return "ori";    /////
		case 14:
        	*isImmediate = 1;
			return "xori";   /////
		case 15:
        	*isImmediate = 1;
			return "lui"; ////
		case 24:
			return "mul";     ///////
		case 32:
			return "lb";   /////
		case 33:
			return "lh";
		case 34:
			return "lw";    //////
		case 36:
			return "lbu";    /////
		case 37:
			return "lhu";
		case 40:
			return "sb";   /////
		case 41:
			return "sh";
		case 43:
			return "sw";   //////
		default:
			return "ERROR";
	}
}
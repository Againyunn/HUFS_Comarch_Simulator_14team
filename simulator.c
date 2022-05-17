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
static unsigned int R[32], PC;
static unsigned char progMEM[0x100000], dataMEM[0x100000], stakMEM[0x100000];

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

    //�ùķ����� ���� ���
    printNotice();

    //�������� �ʱ�ȭ �Լ�


    while(1){
        //COMAND ���� �ʱ�ȭ
        lenCode = 0; //��ɾ��� ���� ��
        cmdLen = 0; //��ɾ��� �ڸ���(1~2�ڸ� �ĺ���)
        cmdErr = 0;
        char *cmdArr[10] = {NULL, };




    /*����Է¹ޱ�*/
        printf("Enter a command.\n>>> ");
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
                // load program
				// ptr�� Filename ���ڿ��� ����Ų��.
				ptr = strtok(NULL, " ");
				char* filePath = NULL;
				if (ptr == NULL) {
					printf("Error: Not enough arguments.\n");
					printf("\tex) l C:\\pub\\as_ex01_arith.bin\n");
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
                printf("Terminate program.\n");
                exit(1);
                break;

        /*���ǵ��� ���� ��ɾ� ����ó��: ��ɾ� 1��¥��*/
            default:
                printf("Error: Enter a valid command.");
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
                printf("Error: Enter a valid command.");
            }

        }
    /*���ǵ��� ���� ��ɾ� ����ó��: ��ɾ� �Է�x�� ��� + ���ǵ��� ���� ��ɾ��� ���*/
        else{
            printf("Error: Enter a valid command.");
        }

        printf("\n\n");
    }
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
		return 1;
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


//Memory Access �κ��̴�.
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
    if(opc != 0){
        // I-Format �Ǵ� J-Format �� ���
        switch(opc){
            case 1:
            case 2:
            //j
            updatePC(IR.JI.jumpAddr);  // L�� �̵�
            break;
            case 3:
            case 4:
            case 5:
            case 8:
            case 10:
            case 12: //andi
				//X�� �������� ��, Y�� ���
				unsigned int X;
				memcpy(X, &isImmediate[0], 5);
				unsigned int Y;
				memcpy(Y, &isImmediate[5], 5);

				unsigned int RX = R[X];	//���������� X��ġ���� ����� �� ��������
				unsigned int answer = ALU(RX, Y, 8, 0); //c32 == 2, c10 == 0�����ϱ� ���� C = 8, �����÷��� 0 ����
				return answer;    /////
            case 13:
            case 14:
            case 15:
            case 32:
            case 35:
            case 36:
            case 40:
            case 43:
            default:
				// not found
				break;
        }
    }else{
        // R-Format �� ���
        switch(fct){
            case 0:
            case 2:
            case 3:
            case 8:
            case 12:
            case 16: 
            case 18: 
            //case 24: 
            case 32: //"add"; 
				
			case 34: //"sub";
            case 36: //"and";
			case 37: 
            case 38: //"xor"; 
            case 39: 
            case 42: 
            default: 
            //not found
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

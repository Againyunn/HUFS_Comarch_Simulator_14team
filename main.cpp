#define _CRT_SECURE_NO_WARNINGS 
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>

/*����� ��������*/
unsigned int PC, IR;
FILE* pFile = nullptr;
errno_t err;
int stop = 0; //��ɾ� �ؼ� ���� ���� �ĺ� : syscall 10 �۵��� ���� ����


unsigned int invertEndian(unsigned int data)
{
    unsigned char c[4];

    c[3] = (unsigned char)data; data = data >> 8;
    c[2] = (unsigned char)data; data = data >> 8;
    c[1] = (unsigned char)data; data = data >> 8;
    c[0] = (unsigned char)data;

    return *(unsigned int*)c;
}

const int M_SIZE = 1024;
unsigned char MEM[M_SIZE];

unsigned int memoryRead(unsigned int addr) {
    unsigned int data;

    if ((addr & 3) != 0) { // check unaligned access?
        printf("memoryRead(): Unaligned access error!\n");
        addr &= 0xFFFFFFFC; // make aligned address
    }
    /*
    data = (unsigned int)MEM[addr];
    data |= (unsigned int)MEM[addr + 1] << 8;
    data |= (unsigned int)MEM[addr + 2] << 16;
    data |= (unsigned int)MEM[addr + 3] << 24;
    */
    data = *(unsigned int*)&MEM[addr];
    data = invertEndian(data);

    return data;
}

void memoryWrite(unsigned int addr, unsigned int data) {
    if ((addr & 3) != 0) { // check unaligned access?
        printf("memoryWrite(): Unaligned access error!\n");
        addr &= 0xFFFFFFFC; // ignore the least 2 bits
    }
    /*
    MEM[addr + 3] = (unsigned char)data; data >>= 8;
    MEM[addr + 2] = (unsigned char)data; data >>= 8;
    MEM[addr + 1] = (unsigned char)data; data >>= 8;
    MEM[addr] = (unsigned char)data;
    */
    *(unsigned int*)&MEM[addr] = invertEndian(data);

    return;
}

// R-type ��ɾ� ���ڵ�
void decodeRtype(unsigned int fct)
{
    unsigned int fcth, fctl;

    fctl = fct & 0x7;
    fcth = (fct & 0x38) >> 3;

    if (fcth == 0) {
        if (fctl == 0) printf("sll\n");
        else if (fctl == 2) printf("srl\n");
        else if (fctl == 4) printf("sra\n");
        else printf("Undefined instruction\n");
    }
    else if (fcth == 1) {
        if (fctl == 0) printf("jr\n");
        else if (fctl == 4) { printf("syscall\n"); stop = 1;  return; }
        else printf("Undefined instruction\n");
    }
    else if (fcth == 2) {
        if (fctl == 0) printf("mfhi\n");
        else if (fctl == 2) printf("mflo\n");
        else printf("Undefined instruction\n");
    }
    else if (fcth == 3) {
        if (fctl == 0) printf("mul\n");
        else printf("Undefined instruction\n");
    }
    else if (fcth == 4) {
        if (fctl == 0) printf("add\n");
        else if (fctl == 2) printf("sub\n");
        else if (fctl == 4) printf("and\n");
        else if (fctl == 5) printf("or\n");
        else if (fctl == 6) printf("xor\n");
        else if (fctl == 7) printf("nor\n");
        else printf("Undefined instruction\n");
    }
    else if (fcth == 5) {
        if (fctl == 2) printf("slt\n");
        else printf("Undefined instruction\n");
    }
    else printf("Undefined instruction\n");
}

//ID
void instructionDecode(void)
{
    unsigned int opc, fct;

    unsigned int opch, opcl;

    opc = IR >> 26;
    fct = IR & 0x3f;

    printf("Opc: %2x, Fct: %2x, Inst: ", opc, fct);

    opcl = opc & 0x7;
    opch = (opc & 0x38) >> 3;

    if (opch == 0) {
        if (opcl == 0) {
            decodeRtype(fct);
        }
        else if (opcl == 1) printf("bltz\n");
        else if (opcl == 2) printf("j\n");
        else if (opcl == 3) printf("jal\n");
        else if (opcl == 4) printf("beq\n");
        else if (opcl == 5) printf("bne\n");
        else printf("Undefined instruction\n");
    }
    else if (opch == 1) {
        if (opcl == 0) printf("addi\n");
        else if (opcl == 2) printf("slti\n");
        else if (opcl == 4) printf("andi\n");
        else if (opcl == 5) printf("ori\n");
        else if (opcl == 6) printf("xori\n");
        else if (opcl == 7) printf("lui\n");
        else printf("Undefined instruction\n");
    }
    else if (opch == 4) {
        if (opcl == 0) printf("lb\n");
        else if (opcl == 3) printf("lw\n");
        else if (opcl == 4) printf("lbu\n");
        else printf("Undefined instruction\n");
    }
    else if (opch == 5) {
        if (opcl == 0) printf("sb\n");
        else if (opcl == 3) printf("sw\n");
        else printf("Undefined instruction\n");
    }
    else printf("Undefined instruction\n");
}



//�ùķ�����
void main() {

    //�ùķ����Ϳ��� ����� ����
    char command[100];
    char fileName[100];
    char buf[32];
    /*char* command = malloc(sizeof(char) * 100);
    char* file_name = malloc(sizeof(char) * 100);*/


    //�ùķ����� ����
    while (1) {
        //������ �Է¹��� ��ɾ� �ʱ�ȭ

        //Get command line;
        printf("\t\t\t*��ɾ� �Է�����*\n");
        printf("l<���������̸�>\t\t\t:���������� �ùķ����� �޸𸮿� �ö󰩴ϴ�.\n");
        printf("j<���α׷� ���� ��ġ>\t\t:�Է��� ��ġ�� �ùķ����� ������ �غ��մϴ�.\n");
        printf("g\t\t\t\t:����pc��ġ���� �ùķ����Ͱ� ��ɾ ������ ó���մϴ�.\n");
        printf("s\t\t\t\t:��ɾ� �ϳ��� ó���ϰ� ����� ����� ��� ���·� �����մϴ�.\n");
        printf("m<start><end>\t\t\t:start~end������ �޸� ������ ����մϴ�.\n");
        printf("r\t\t\t\t:���� ���������� ������ ����մϴ�.\n");
        printf("x\t\t\t\t:�ùķ����� ���α׷��� �����մϴ�.\n");
        printf("sr<register number><value>\t:Ư�� ���������� ���� �����մϴ�.\n");
        printf("sm<location><value>\t\t:�޸� Ư�� �ּ��� ���� �����մϴ�.\n");
        printf("---------------------------------------------------------------------------------------------\n");

        scanf("%s", command);

        //��ɾ� l
        if (command[0] == 'l') {
            /*�Է¹��� ���ڿ� �ؼ� �� ���� �̸� ����*/
            int input = 0;
            char s = '<';
            char e = '>';
            int count = 0;
            int err = 0;

            for (int i = 1; i < 100; i++) {
                if (command[i] == NULL || command[i] == '\0') {
                    printf("�Է��� ��ɾ��� ������ Ȯ�����ּ���.\n");
                    err = 1;
                    break;
                }
                //���๮�� > ������ ���
                if (command[i] == e){
                    input = 0;
                    fileName[count] = '\0';
                    break;
                }
                //�����̸� ���
                if (input==1) {
                    fileName[count] = command[i];
                    count++;
                }
                //���๮�� < ������ ��� 
                if (command[i] == s) {
                    input = 1;
                }
            }

            //�ؼ��������� ������ �־��� �� ����
            if (err == 1) {
                //�ٽ� ��ɾ ���� �� ���� ���
                printf("\n");
                continue;
            }

            //���� ��� FD ����
            err = fopen_s(&pFile, fileName, "rb");
            if (err) {
                printf(" '%s' ������ �� �� �����ϴ�.\n", fileName);
                pFile = NULL;
                continue;
            }

            //���α׷�&������ �޸� ���� ����

            //PC&SP �ʱⰪ ����


            //���� ����� ������ ���
            
        }

        //��ɾ� j
        else if (command[0] == 'j') {
            /*�Է¹��� ���ڿ� �ؼ� �� �ּ� �� ����*/
            char address_point[100];

            int input = 0;
            char s = '<';
            char e = '>';
            int count = 0;
            int err = 0;

            for (int i = 1; i < 100; i++) {
                if (command[i] == NULL || command[i] == '\0') {
                    printf("�Է��� ��ɾ��� ������ Ȯ�����ּ���.\n");
                    err = 1;
                    break;
                }
                //���๮�� > ������ ���
                if (command[i] == e) {
                    input = 0;
                    address_point[count] = '\0';
                    break;
                }
                //�ּ� ������ ��ġ ���
                if (input == 1) {
                    address_point[count] = command[i];
                    count++;
                }
                //���๮�� < ������ ��� 
                if (command[i] == s) {
                    input = 1;
                }
            }

            //�ؼ��������� ������ �־��� �� ����
            if (err == 1) {
                //�ٽ� ��ɾ ���� �� ���� ���
                printf("\n");
                continue;
            }

            //PC�� �ش� �ּ� ������ ����
            PC = (unsigned int)address_point; //<- �̺κ� ���� �ʿ�

            //���� ����� ������ ���
            printf("���� PC��: %d\n", PC);

        }

        //��ɾ� g
        else if (command[0] == 'g') {
               
            //������ load�� �����Ͱ� �ִ� �� Ȯ��
            if (&pFile == nullptr || pFile == NULL) {
                printf("��ɾ load�� �� ������ּ���.\n\n");
                continue;
            }
        
        
            //���� PC��ġ �޾ƿ���



            //��ɾ ������ ó��
            int i;
            unsigned int data;
            unsigned int addr;
            unsigned int buf[32];

            unsigned int iCount;	// # of instructions
            unsigned int dCount;	// # of data

            int num;
            //��ɾ buf�� 32���� buf�� �����ϸ� ���� ��ü�� ��ɾ� �о����
            while ((num = fread(&buf, sizeof(char), 32, pFile)) != 0) {


                // read instruction and data numbers
                for (int j = 0; j < 32; j++) {
                    iCount = invertEndian(buf[j]);
                    dCount = invertEndian(buf[j]);                
                    //fread(&data, sizeof(data), 1, buf);
                    //iCount = invertEndian(data);
                    //fread(&data, sizeof(data), 1, buf);
                    //dCount = invertEndian(data);


                    printf("Number of Instructions: %d, Number of Data: %d\n", iCount, dCount);

                    // Load to memory
                    addr = 0;
                    for (i = 0; i < (int)iCount; i++) {
                        fread(&data, sizeof(unsigned int), 1, pFile);
                        data = invertEndian(data);
                        memoryWrite(addr, data);
                        addr += 4;
                    }

                    // Decoding
                    unsigned int PC = 0; // program counter
                    for (i = 0; i < (int)iCount; i++) {
                        IR = memoryRead(PC); // instruction fetch
                        PC += 4;
                        instructionDecode(); // instruction decode
                    }

                    //syscall 10�� ��� ��ɾ� ���� ����
                    if (stop == 1) {
                        fclose(pFile);
                        continue;
                    }
                    fclose(pFile);
                }
            }
        }

        //��ɾ 2�ڸ��� ���(sr, sm)
        else if (command[1] != NULL) {
            
            //��ɾ� sr
            if (command[0] == 's' && command[1] == 'r') {
                /*�Է¹��� ���ڿ� �ؼ� �� register number�� value �� ����*/
                char register_number[100];
                char register_value[100];

                int input = 0;
                char s = '<';
                char e = '>';
                int count = 0;
                int check = 0;
                int err = 0;

                for (int i = 2; i < 100; i++) {
                    if (command[i] == NULL || command[i] == '\0') {
                        printf("�Է��� ��ɾ��� ������ Ȯ�����ּ���.\n");
                        err = 1;
                        break;
                    }

                    //start_address�и�
                    if (check == 0) {
                        //���๮�� > ������ ���
                        if (command[i] == e) {
                            input = 0;
                            register_number[count] = '\0';
                            check = 1;
                            count = 0;
                        }
                        //�ּ� ������ ��ġ ���
                        if (input == 1) {
                            register_number[count] = command[i];
                            count++;
                        }
                        //���๮�� < ������ ��� 
                        if (command[i] == s) {
                            input = 1;
                        }
                    }

                    else {
                        //���๮�� > ������ ���
                        if (command[i] == e) {
                            input = 0;
                            register_value[count] = '\0';
                            break;
                        }
                        //�ּ� ������ ��ġ ���
                        if (input == 1) {
                            register_value[count] = command[i];
                            count++;
                        }
                        //���๮�� < ������ ��� 
                        if (command[i] == s) {
                            input = 1;
                        }
                    }

                }


                //�ؼ��������� ������ �־��� �� ����
                if (err == 1) {
                    //�ٽ� ��ɾ ���� �� ���� ���
                    printf("\n");
                    continue;
                }

                //�׽�Ʈ(����)
                printf("start_address: %s, end_address: %s\n", register_number, register_value);
                //register�Լ� ����

                //��� �� ���
            }

            //��ɾ� sm
            if (command[0] == 's' && command[1] == 'm') {
                /*�Է¹��� ���ڿ� �ؼ� �� location�� value �� ����*/
                char location[100];
                char location_value[100];

                int input = 0;
                char s = '<';
                char e = '>';
                int count = 0;
                int check = 0;
                int err = 0;

                for (int i = 2; i < 100; i++) {

                    if (command[i] == NULL || command[i] == '\0') {
                        printf("�Է��� ��ɾ��� ������ Ȯ�����ּ���.\n");
                        err = 1;
                        break;
                    }

                    //start_address�и�
                    if (check == 0) {
                        //���๮�� > ������ ���
                        if (command[i] == e) {
                            input = 0;
                            location[count] = '\0';
                            check = 1;
                            count = 0;
                        }
                        //�ּ� ������ ��ġ ���
                        if (input == 1) {
                            location[count] = command[i];
                            count++;
                        }
                        //���๮�� < ������ ��� 
                        if (command[i] == s) {
                            input = 1;
                        }
                    }

                    else {
                        //���๮�� > ������ ���
                        if (command[i] == e) {
                            input = 0;
                            location_value[count] = '\0';
                            break;
                        }
                        //�ּ� ������ ��ġ ���
                        if (input == 1) {
                            location_value[count] = command[i];
                            count++;
                        }
                        //���๮�� < ������ ��� 
                        if (command[i] == s) {
                            input = 1;
                        }
                    }

                }

                //�ؼ��������� ������ �־��� �� ����
                if (err == 1) {
                    //�ٽ� ��ɾ ���� �� ���� ���
                    printf("\n");
                    continue;
                }


                //�׽�Ʈ(����)
                printf("start_address: %s, end_address: %s\n", location, location_value);

                //memory�Լ� ����

                //��� �� ���
            }
        }


        //��ɾ� s
        else if (command[0] == 's') {
            //������ load�� �����Ͱ� �ִ� �� Ȯ��
            if (&pFile == nullptr || pFile == NULL) {
                printf("��ɾ load�� �� ������ּ���.\n\n");
                continue;
            }

            //���� PC��ġ �޾ƿ���

            //���� ��ɾ� 1���� ó��
            int i;
            unsigned int data;
            unsigned int addr;
            unsigned int buf[32];

            unsigned int iCount;	// # of instructions
            unsigned int dCount;	// # of data

            int num;

            //��ɾ buf�� 32���� buf�� 1���� ����
            if ((num = fread(&buf, sizeof(char), 32, pFile)) != 0) {

                // read instruction and data numbers
                for (int j = 0; j < 32; j++) {
                    iCount = invertEndian(buf[j]);
                    dCount = invertEndian(buf[j]);
                    //fread(&data, sizeof(data), 1, buf);
                    //iCount = invertEndian(data);
                    //fread(&data, sizeof(data), 1, buf);
                    //dCount = invertEndian(data);


                    printf("Number of Instructions: %d, Number of Data: %d\n", iCount, dCount);

                    // Load to memory
                    addr = 0;
                    for (i = 0; i < (int)iCount; i++) {
                        fread(&data, sizeof(unsigned int), 1, pFile);
                        data = invertEndian(data);
                        memoryWrite(addr, data);
                        addr += 4;
                    }

                    // Decoding
                    unsigned int PC = 0; // program counter
                    for (i = 0; i < (int)iCount; i++) {
                        IR = memoryRead(PC); // instruction fetch
                        PC += 4;
                        instructionDecode(); // instruction decode
                    }

                    //syscall 10�� ��� ��ɾ� ���� ����
                    if (stop == 1) {
                        fclose(pFile);
                        continue;
                    }
                    fclose(pFile);
                }
            }

            //��ɾ ���� ����� register, memory�� �޾Ƽ� ���



        }

        //��ɾ� m
        else if (command[0] == 'm') {
            //�Է¹��� ��ɾ� �ؼ� �� start�� end �� ����

              /*�Է¹��� ���ڿ� �ؼ� �� �ּ� �� ����*/
              char start_address[100];
              char end_address[100];

              int input = 0;
              char s = '<';
              char e = '>';
              int count = 0;
              int check = 0;

              for (int i = 1; i < 100; i++) {
                  if (command[i] == NULL) {
                      printf("�Է��� ��ɾ��� ������ Ȯ�����ּ���.\n");
                      continue;
                  }
                  
                  //start_address�и�
                  if (check == 0) {
                      //���๮�� > ������ ���
                      if (command[i] == e) {
                          input = 0;
                          start_address[count] = '\0';
                          check = 1;
                          count = 0;
                      }
                      //�ּ� ������ ��ġ ���
                      if (input == 1) {
                          start_address[count] = command[i];
                          count++;
                      }
                      //���๮�� < ������ ��� 
                      if (command[i] == s) {
                          input = 1;
                      }
                  }

                  else {
                      //���๮�� > ������ ���
                      if (command[i] == e) {
                          input = 0;
                          end_address[count] = '\0';
                          break;
                      }
                      //�ּ� ������ ��ġ ���
                      if (input == 1) {
                          end_address[count] = command[i];
                          count++;
                      }
                      //���๮�� < ������ ��� 
                      if (command[i] == s) {
                          input = 1;
                      }
                  }
               
              }

            //�׽�Ʈ(����)
            printf("start_address: %s, end_address: %s\n", start_address, end_address);

            //unsigned int unsigned_start_address = (unsigned int)start_address;
            //unsigned int unsigned_end_address = (unsigned int)end_address;
            //printf("start_address: %u, end_address: %u\n", unsigned_start_address, unsigned_end_address);
            
            //memory�Լ� ����
            

            //��� �� ���


        }

        //��ɾ� r
        else if (command[0] == 'r') {
            //register�Լ� ����

            //��� �� ���

        }

        //��ɾ� x
        else if (command[0] == 'x') {
            /*������ ���� �ؾ��ϳ� ?*/

            //�ùķ����� ���� ����
            printf("�ùķ����͸� �����մϴ�.\n");
            break;
        }

        //���ǵ��� ���� ��ɾ�
        else {
        printf("�ùٸ� ��ɾ �Է����ּ���.\n");
        }

        //�ٽ� ��ɾ ���� �� 1�� ����
        printf("\n");
    }

}

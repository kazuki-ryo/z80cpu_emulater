#define DEBUG_VIEW 0

#include"z80.c"


/*
sample�̎��s
NOP
LD C,0
LD A,1
OUT (C),A
OUT (C),C
RET
*/
/**
DDD or SSS
111 A
000 B
001 C
010 D
011 E
100 H
101 L
110 (HL)
RP���W�X�^�y�A
00 BC
01 DE
10 HL
11 SP
*/
unsigned char SampleData[256]={
	/*
	//IO port�e�X�g�p
	0x3e,4,//LD A,4		00 111 110,00000001
	0xd3,1, // out (n),A
	0xdb,1, //IN A,(n)
	0x3e,5,//LD A,5		00 111 110,00000001
	0xd3,2, // out (n),A
	0xdb,2, //IN A,(n)
	0x76,//HALT
	*/
	//�x���`�}�[�N�p�P
	0x11,10,0x00, //LD DE,10
	0x01,0x00,0x00,//LD BC,0
	0x0b,//DEC BC
	0x78,//LD A,B
	0xb1,//OR C
	0xc2,6,0,// JP NZ,0x0006
	0x1b,//DEC DE
	0x7A,//LD A,D
	0xb3,//OR E
	0xc2,3,0,// JP NZ,0x0003
	0x76,//HALT
//	0x21,0x66,0xf3,0x0d,0x7d,0x89,0xae,0xd4,0x3b,0x21,0xa5,0x62,
//	0x5d,0xa2,0xf2,0x72,0x08,0xf5,0xa1,0x2e,0x61,
//	0,0,0,0x76,
	0,//NOP
	0x3e,0x00,//LD A,1		00 111 110,00000001
	0x06,0x00,//LD B,2		00 000 110,00000010
	0x80,//ADD A,B		10000 000
	0x76, //HALT 	01110110
	
	0x21,0x34,0x12, //LD HL,1234h 00 10 0001,00110100,00010010
	0xe5,// PUSH HL 11 10 0101
	0x21,0x78,0x56, //LD HL,5678h 00 10 0001,01111000,01010110
	0xe3, //EX (SP),HL 11100011
	0x76, //HALT 	01110110

	0x06,1,//LD B,1		00 000 110,00000001
	0x0E,2,//LD C,2		00 001 110,00000010
	0x16,3,//LD D,1		00 010 110,00000001
	0x1E,4,//LD E,2		00 011 110,00000010
	0x26,5,//LD H,1		00 100 110,00000001
	0x2E,6,//LD L,2		00 101 110,00000010
	0xd9, //EXX		11011001
	0xeb,  //EX DE,HL		11101011
	0x08, //EX AF,AF'		00001000

	0x76, //HALT 01110110

	0x3, //INC BC		00 00 0011
	0xc, //INC C		00 001 100 
	0xcd,0x0d,0,//CALL 	11001101,00001101,0
	0xc3,0x10,00,//JP 0000	11000011,00010000,00000000
	0,
	0,
	0xc9,//RET
	0,
	0xc5, // PUSH BC		11 00 0101
	0xe1, // POP HL		11 10 0001
	0xe5,//PUSH HL		11 10 0101
	0xf1, //POP AF      11110001
	0xf5,// PUSH AF		11110101
	0xd1, // POP DE		11 01 0001
	0x3e,1,//LD A,1		00 111 110,00000001
	0xed,0x79,//OUT (C),A		11101101,01 111 001
	0xed,0x49,//OUT (C),C		11101101,01 001 001
	0x76, //HALT 01110110
	0xc9//RET
};

/**
���C������
HALT���s�ŃG�~�����[�V������~
*/
int TaskMain()
{
	int i,w;
//	for(i=0;i<1000;i++){
	while(1){
		//�R�[�h�ǂݍ���
		unsigned char code=Memory(PC_REG);
		HL_Flag=0;//�f�t�H���g��HL�g��
		#if DEBUG_VIEW
		printf("code=%x\n",code);
		#endif
		//�R�[�h���
		char State=CodeAnalysis(code,0,HL_REG,H_REG,L_REG);
		if(State==1){
			break;//���f
		}
		//PC�J�E���g�A�b�v
		PC_REG=(PC_REG+1)&0xffff;
		//�E�F�C�g
		//�X�e�[�g�̐��l���ŋ^���E�F�C�g����
//		for(w=0;w<5;w++){
//			Memory(0)=Memory(0);
//		}
		//
		ClockCount=ClockCount+State;
		//�������\���i�f�o�b�O�p)
		#if DEBUG_VIEW

		ViewRegister();
		ViewMemory();
		printf("--\n");
		#endif
	}
	return 0;
}
unsigned char _WorkMemory[65536];//MAIN Memory
unsigned char _IOMemory[256];//I/O AREA


int main(int argc,char* argv[])
{
	int a;
	printf("z80 emulater v0.10\n");
	//�G�~�����[�^������
	InitZ80(_WorkMemory,_IOMemory);
	//�e�X�g�R�[�h���������ɐݒ�
	for(a=0;a<sizeof(SampleData);a++){
		WorkMemory[a]=SampleData[a];
	}
	//���W�X�^������
	InitRegister();
	//�G�~�����[�^�N��
	TaskMain();
	//
	printf("End of Executed z80.\n");
	ViewRegister();
	ViewMemory();

	return 0;
}
/**
VDP�̕����\�������T�|�[�g����ɂ͈ȉ�����������B
out&h99,A0-A7
out&h99,0fxxxxxx  f:1=write 0=read  xxxxx=A8-A13
out&h98,data
*/

#define DEBUG_VIEW 1
#include"z80.c"

/**
１命令テスト実行
initRegs=1の時初期値設定
op=実行する命令（文字列）
code1～4=４バイト分の命令設定
*/
int TestCode(int initRegs,char* op,unsigned char code1,unsigned char code2,unsigned char code3,unsigned char code4)
{
	//レジスタ初期化
	if(initRegs==1){
		InitRegister();
	}
	//実行するコード
	#if DEBUG_VIEW
	printf("OP CODE=%s\n",op);
	#endif
	//メモリにテストデータ設定
	WorkMemory[0]=code1;
	WorkMemory[1]=code2;
	WorkMemory[2]=code3;
	WorkMemory[3]=code4;
	//PC レジスタを初期位置に設定
	PC_REG=0;
	//SP レジスタを初期位置に設定
	SP_REG=0x3ffe;
	//1命令読み込み
	unsigned char code=Memory(PC_REG);
	//コード解析
	int State=CodeAnalysis(code,0,HL_REG,H_REG,L_REG,&HL_REG);
	//PCカウントアップ
	PC_REG=(PC_REG+1)&0xffff;

	//実行結果
	//メモリ表示（デバッグ用)
	#if DEBUG_VIEW
	ViewRegister();
	ViewMemory();
	#endif
	return 0;
}

int ErrorCount=0;//失敗数
int SuccessCount=0;//成功数
int ErrorUnit=0;//失敗数
/**
コード判定
result=レジスタなどの値の結果を入れる
内部で成功・失敗の件数をカウントする。
*/
int CheckCode(int result)
{
	
	if(result!=0){
		SuccessCount++;
		printf(" OK\n");
		return 0;
	}else{
		printf(" NG\n");
		ErrorCount++;
		ErrorUnit++;
		return 1;
	}
}

unsigned char _WorkMemory[65536];//MAIN Memory
unsigned char _IOMemory[256];//I/O AREA

int Test_LD()
{
	ErrorUnit=0;
	//
	printf("[TEST01]8BIT LOAD REGISTER\n");
	/*
	//
	TestCode(1,"LD A,1",0x3e,1,0,0);
	CheckCode(A_REG==1);
	//
	A_REG=1;B_REG=2;
	TestCode(0,"LD A,B",0x78,0,0,0);
	CheckCode(A_REG==2);
	
	//
	A_REG=1;HL_REG=0x0004;WorkMemory[HL_REG]=123;
	TestCode(0,"LD A,(HL)",0x7E,0,0,0);
	CheckCode(A_REG==123);
	//
	A_REG=1;HL_REG=0x0004;WorkMemory[HL_REG]=123;
	TestCode(0,"LD (HL),A",0x77,0,0,0);
	CheckCode(WorkMemory[0x0004]==1);
	//
	A_REG=1;IX_REG=0x0004;WorkMemory[IX_REG+1]=123;
	TestCode(0,"LD A,(IX+d)",0xdd,0x7E,1,0);
	CheckCode(A_REG==123);
	//
	A_REG=1;IY_REG=0x0004;WorkMemory[IY_REG-1]=0;
	TestCode(0,"LD (IY+d),A",0xfd,0x77,0xff,0);
	CheckCode(WorkMemory[0x0003]==1);
	
	//
	BC_REG=0x0004;WorkMemory[BC_REG]=123;
	TestCode(0,"LD A,(BC)",0x0a,0,0,0);
	CheckCode(A_REG==123);
	//
	DE_REG=0x0004;WorkMemory[DE_REG]=100;
	TestCode(0,"LD A,(DE)",0x1a,0,0,0);
	CheckCode(A_REG==100);
	//
	TestCode(0,"LD A,(nn)",0x3a,3,0,2);
	CheckCode(A_REG==2);
	//
	A_REG=1;BC_REG=3;
	TestCode(0,"LD (BC),A",0x02,0,0,0);
	CheckCode(Memory(3)==1);
	//
	A_REG=2;DE_REG=3;
	TestCode(0,"LD (DE),A",0x12,0,0,0);
	CheckCode(Memory(3)==2);
	//
	A_REG=20;
	TestCode(0,"LD (nn),A",0x32,3,0,20);
	CheckCode(Memory(0x0003)==20);
	//
	*/
	printf("[TEST02]16BIT LOAD REGISTER\n");
	//
	TestCode(1,"LD HL,nn",0x21,0x1,0x2,0);
	CheckCode(HL_REG==0x0201);
	//
	TestCode(1,"LD BC,nn",0x01,0x1,0x2,0);
	CheckCode(BC_REG==0x0201);
	//
	TestCode(1,"LD DE,nn",0x11,0x1,0x2,0);
	CheckCode(DE_REG==0x0201);
	//
	TestCode(1,"LD SP,nn",0x31,0x1,0x2,0);
	CheckCode(SP_REG==0x0201);
/*
	//
	TestCode(1,"LD HL,(nn)",0x2A,2,0,0x20);
	CheckCode(HL_REG==0x2000);
	//
	HL_REG=0x0304;
	TestCode(0,"LD (nn),HL",0x22,3,0,0);
	CheckCode(Memory(3)==4 && Memory(4)==3);
	//
	HL_REG=0x0102;
	TestCode(0,"LD SP,HL",0xF9,0,0,0);
	CheckCode(SP_REG==0x0102);

	//
	HL_REG=4;
	TestCode(0,"LD (HL),n",0x36,0x1,0x20,0x30);
	CheckCode(Memory(0x4)==0x1);
	//
	IX_REG=4;
	TestCode(0,"LD (IX+d),n",0xdd,0x36,0x1,0x20);
	CheckCode(Memory(5)==0x20);
	//
	IY_REG=4;
	TestCode(0,"LD (IY+d),n",0xfd,0x36,0x1,0x20);
	CheckCode(Memory(5)==0x20);

	//
	A_REG=1;I_REG=0;R_REG=3;IFF2_REG=1;
	TestCode(0,"LD A,I",0xed,0x57,0x0,0x00);
	CheckCode(A_REG==0);
	//
	A_REG=1;I_REG=2;R_REG=3;
	TestCode(0,"LD I,A",0xed,0x47,0x0,0x00);
	CheckCode(I_REG==1);
	//
	A_REG=1;I_REG=2;R_REG=128;IFF2_REG=1;
	TestCode(0,"LD A,R",0xed,0x5f,0x0,0x00);
	CheckCode(A_REG==128);
	//
	A_REG=1;I_REG=2;R_REG=128;
	TestCode(0,"LD R,A",0xed,0x4f,0x0,0x00);
	CheckCode(R_REG==1);

	//
	IX_REG=0x0102;
	TestCode(0,"LD SP,IX",0xdd,0xF9,0,0);
	CheckCode(SP_REG==0x0102);
	//
	IY_REG=0x0103;
	TestCode(0,"LD SP,IY",0xfd,0xF9,0,0);
	CheckCode(SP_REG==0x0103);
	//
	TestCode(1,"LD IX,(nn)",0xdd,0x2A,0,0);
	CheckCode(IX_REG==0x2Add);
	//
	TestCode(1,"LD IY,(nn)",0xfd,0x2A,0,0);
	CheckCode(IY_REG==0x2Afd);
	//
	IX_REG=0x0304;
	TestCode(0,"LD (nn),IX",0xdd,0x22,4,0);
	CheckCode(Memory(4)==4 && Memory(5)==3);
	//
	IY_REG=0x0506;
	TestCode(0,"LD (nn),IY",0xfd,0x22,4,0);
	CheckCode(Memory(4)==6 && Memory(5)==5);
*/
	//
	TestCode(1,"LD IX,nn",0xdd,0x21,0x1,0x2);
	CheckCode(IX_REG==0x0201);
	//
	TestCode(1,"LD IY,nn",0xfd,0x21,0x3,0x2);
	CheckCode(IY_REG==0x0203);
	//
	BC_REG=0x0102;Memory(4)=0;Memory(5)=0;
	TestCode(0,"LD (nn),BC",0xed,0x43,0x4,0x0);
	CheckCode(Memory(4)==0x02 && Memory(5)==0x01);
	//
	DE_REG=0x0103;Memory(4)=0;Memory(5)=0;
	TestCode(0,"LD (nn),DE",0xed,0x53,0x4,0x0);
	CheckCode(Memory(4)==0x03 && Memory(5)==0x01);
	//
	HL_REG=0x0104;Memory(4)=0;Memory(5)=0;
	TestCode(0,"LD (nn),HL",0xed,0x63,0x4,0x0);
	CheckCode(Memory(4)==0x04 && Memory(5)==0x01);
	//
	SP_REG=0x3ffe;Memory(4)=0;Memory(5)=0;
	TestCode(0,"LD (nn),SP",0xed,0x73,0x4,0x0);
	CheckCode(Memory(4)==0xfe && Memory(5)==0x3f);
	//
	Memory(4)=1;Memory(5)=2;
	TestCode(0,"LD BC,(nn)",0xed,0x4b,0x4,0x0);
	CheckCode(BC_REG==0x0201);
	//
	Memory(4)=1;Memory(5)=3;
	TestCode(0,"LD DE,(nn)",0xed,0x5b,0x4,0x0);
	CheckCode(DE_REG==0x0301);
	//
	Memory(4)=1;Memory(5)=4;
	TestCode(0,"LD HL,(nn)",0xed,0x6b,0x4,0x0);
	CheckCode(HL_REG==0x0401);
	//
	Memory(4)=1;Memory(5)=5;
	TestCode(0,"LD SP,(nn)",0xed,0x7b,0x4,0x0);
	CheckCode(SP_REG==0x0501);
	
	return ErrorUnit;
}
int Test_EX()
{
	ErrorUnit=0;

	//
	printf("[TEST03]SWAP REGISTER\n");
	//
	HL_REG=0x0102;DE_REG=0x0304;
	TestCode(0,"EX DE,HL",0xEB,0,0,0);
	CheckCode(DE_REG==0x0102 && HL_REG==0x0304);
	//
	AF_REG=0x0102;AF2_REG=0x0304;
	TestCode(0,"EX AF,AF'",0x08,0,0,0);
	CheckCode(AF2_REG==0x0102 && AF_REG==0x0304);
	//
	BC_REG=0x01;DE_REG=0x02;HL_REG=0x03;
	BC2_REG=0x04;DE2_REG=0x05;HL2_REG=0x06;
	TestCode(0,"EXX",0xD9,0,0,0);
	CheckCode(BC_REG==0x04 && DE_REG==0x05 && HL_REG==06);
	//
	HL_REG=0x0102;Memory(0x3ffe)=3;Memory(0x3fff)=4;
	TestCode(0,"EX (SP),HL",0xE3,0,0,0);
	CheckCode(Memory(0x3ffe)==2 && Memory(0x3fff)==1);
	//
	IX_REG=0x0102;Memory(0x3ffe)=3;Memory(0x3fff)=4;
	TestCode(0,"EX (SP),IX",0xdd,0xE3,0,0);
	CheckCode(Memory(0x3ffe)==2 && Memory(0x3fff)==1);
	//
	IY_REG=0x0102;Memory(0x3ffe)=3;Memory(0x3fff)=4;
	TestCode(0,"EX (SP),IY",0xfd,0xE3,0,0);
	CheckCode(Memory(0x3ffe)==2 && Memory(0x3fff)==1);

	return ErrorUnit;
}
int Test_PUSHPOP()
{
	ErrorUnit=0;

	//
	printf("[TEST04]PUSH/POP\n");
	//
	HL_REG=0x0201;
	TestCode(0,"PUSH HL",0xE5,0,0,0);
	CheckCode(SP_REG==0x3ffc && Memory(0x3ffc)==1 && Memory(0x3ffd)==2);
	//
	AF_REG=0x0304;
	TestCode(0,"PUSH AF",0xF5,0,0,0);
	CheckCode(SP_REG==0x3ffc && Memory(0x3ffc)==4 && Memory(0x3ffd)==3);
	//
	Memory(0x3ffe)=0x1;Memory(0x3fff)=0x2;
	TestCode(0,"POP HL",0xE1,0,0,0);
	CheckCode(SP_REG==0x4000 && HL_REG==0x0201);
	//
	Memory(0x3ffe)=0x7;Memory(0x3fff)=0x8;
	TestCode(0,"POP AF",0xF1,0,0,0);
	CheckCode(SP_REG==0x4000 && AF_REG==0x0807);
	//
	IX_REG=0x0201;HL_REG=0;
	TestCode(0,"PUSH IX",0xdd,0xE5,0,0);
	CheckCode(SP_REG==0x3ffc && Memory(0x3ffc)==1 && Memory(0x3ffd)==2);
	//
	IY_REG=0x0201;
	TestCode(0,"PUSH IY",0xfd,0xE5,0,0);
	CheckCode(SP_REG==0x3ffc && Memory(0x3ffc)==1 && Memory(0x3ffd)==2);
	//
	Memory(0x3ffe)=0x1;Memory(0x3fff)=0x2;IX_REG=0;IY_REG=0;
	TestCode(0,"POP IX",0xdd,0xE1,0,0);
	CheckCode(SP_REG==0x4000 && IX_REG==0x0201);
	//
	Memory(0x3ffe)=0x1;Memory(0x3fff)=0x2;
	TestCode(0,"POP IY",0xfd,0xE1,0,0);
	CheckCode(SP_REG==0x4000 && IY_REG==0x0201);

	return ErrorUnit;
}
int Test_ROTATE()
{
	ErrorUnit=0;
	/*
	//
	printf("[TEST05]LEFT ROTATE\n");
	//
	A_REG=0x83;
	TestCode(0,"RLCA",0x7,0,0,0);
	CheckCode(A_REG==0x7 && C_FLAG==1);
	//
	C_REG=0x83;C_FLAG=0;
	TestCode(0,"RLC C",0xcb,0x1,0,0);
	CheckCode(C_REG==0x7 && C_FLAG==1);
	//
	D_REG=0xC3;C_FLAG=0;
	TestCode(0,"RLC D",0xcb,0x2,0,0);
	CheckCode(D_REG==0x87 && C_FLAG==1 && S_FLAG==1);
	//
	A_REG=0x43;C_FLAG=1;
	TestCode(0,"RLA",0x17,0x0,0,0);
	CheckCode(A_REG==0x87 && C_FLAG==0);
	//
	A_REG=0x83;
	TestCode(0,"RRCA",0xf,0,0,0);
	CheckCode(A_REG==0xc1 && C_FLAG==1);
	//
	A_REG=0x42;C_FLAG=1;
	TestCode(0,"RRA",0x1f,0x0,0,0);
	CheckCode(A_REG==0xa1 && C_FLAG==0);
*/
	//
	B_REG=0x81;C_FLAG=0;
	TestCode(0,"RLC B",0xCB,0x0,0,0);
	CheckCode(B_REG==0x03 && C_FLAG==1);
	//
	B_REG=0x81;C_FLAG=1;
	TestCode(0,"RL B",0xCB,0x10,0,0);
	CheckCode(B_REG==0x03 && C_FLAG==1);
	//
	B_REG=0x41;C_FLAG=0;
	TestCode(0,"RRC B",0xCB,0x08,0,0);
	CheckCode(B_REG==0xa0 && C_FLAG==1);
	//
	B_REG=0x41;C_FLAG=0;
	TestCode(0,"RR B",0xCB,0x18,0,0);
	CheckCode(B_REG==0x20 && C_FLAG==1);
	return ErrorUnit;
}
int Test_SHIFT()
{
	ErrorUnit=0;
	//
	B_REG=0x81;C_FLAG=0;
	TestCode(0,"SLA B",0xCB,0x20,0,0);
	CheckCode(B_REG==0x02 && C_FLAG==1);
	//
	B_REG=0x81;C_FLAG=0;
	TestCode(0,"SRA B",0xCB,0x28,0,0);
	CheckCode(B_REG==0xc0 && C_FLAG==1);
	//
	B_REG=0x81;C_FLAG=0;
	TestCode(0,"SRL B",0xCB,0x38,0,0);
	CheckCode(B_REG==0x40 && C_FLAG==1);
	//
	A_REG=0x43;HL_REG=3;C_FLAG=0;
	TestCode(0,"RLD",0xED,0x6f,0,0x21);
	CheckCode(A_REG==0x42 && Memory(3)==0x13);
	//
	A_REG=0x43;HL_REG=3;C_FLAG=0;
	TestCode(0,"RRD",0xED,0x67,0,0x21);
	CheckCode(A_REG==0x41 && Memory(3)==0x32);

	return ErrorUnit;
}
int Test_CALC()
{
	ErrorUnit=0;

	//
	printf("[TEST06]ADD 8BIT\n");
	/*
	//
	A_REG=0x1;B_REG=0x5;
	TestCode(0,"ADD A,B",0x80,0,0,0);
	CheckCode(A_REG==0x6 && N_FLAG==1 && PV_FLAG==0);
	//
	A_REG=0x70;
	TestCode(0,"ADD A,0x10",0xC6,0x10,0,0);
	CheckCode(A_REG==0x80 && N_FLAG==1 && PV_FLAG==1 && C_FLAG==0);
	A_REG=0xF0;
	TestCode(0,"ADD A,0x10",0xC6,0x10,0,0);
	CheckCode(A_REG==0x00 && N_FLAG==1 && PV_FLAG==1 && C_FLAG==1 && Z_FLAG==1);
	//
	A_REG=0x70;C_FLAG=1;B_REG=0x10;
	TestCode(0,"ADC A,B",0x88,0x70,0,0);
	CheckCode(A_REG==0x81 && N_FLAG==1 && PV_FLAG==1 && C_FLAG==0);
	A_REG=0xF0;C_FLAG=1;B_REG=0x10;
	TestCode(0,"ADC A,B",0x88,0x10,0,0);
	CheckCode(A_REG==0x01 && N_FLAG==1 && PV_FLAG==1 && C_FLAG==1);
	//
	A_REG=0x70;C_FLAG=1;
	TestCode(0,"ADC A,0x70",0xce,0x10,0,0);
	CheckCode(A_REG==0x81 && N_FLAG==1 && PV_FLAG==1 && C_FLAG==0);
	A_REG=0xF0;C_FLAG=1;
	TestCode(0,"ADC A,0x10",0xce,0x10,0,0);
	CheckCode(A_REG==0x01 && N_FLAG==1 && PV_FLAG==1 && C_FLAG==1);
	//
	A_REG=0x7f;N_FLAG=1;
	TestCode(0,"INC A",0x3c,0x00,0,0);
	CheckCode(A_REG==0x80 && N_FLAG==0 && PV_FLAG==1 && C_FLAG==0 && H_FLAG==0 && S_FLAG==1);
	A_REG=0x0f;N_FLAG=1;
	TestCode(0,"INC A",0x3c,0x00,0,0);
	CheckCode(A_REG==0x10 && N_FLAG==0 && PV_FLAG==0 && C_FLAG==0 && H_FLAG==1 && S_FLAG==0);
	//
	A_REG=0x4;C_FLAG=1;HL_REG=2;
	TestCode(0,"ADC A,(HL)",0x8e,0,1,0);
	CheckCode(A_REG==0x6 && N_FLAG==1 && PV_FLAG==0 && Z_FLAG==0 && C_FLAG==0);
	//
	A_REG=0x4;C_FLAG=1;HL_REG=2;
	TestCode(0,"ADD A,(HL)",0x86,0,1,0);
	CheckCode(A_REG==0x5 && N_FLAG==1 && PV_FLAG==0 && Z_FLAG==0 && C_FLAG==0);
	//
	HL_REG=0x02;
	TestCode(0,"INC (HL)",0x34,0,3,0);
	CheckCode(Memory(2)==0x4 && N_FLAG==0 && PV_FLAG==0 && Z_FLAG==0 && C_FLAG==0);
	//
	A_REG=0x4;C_FLAG=1;IX_REG=4;Memory(4)=1;
	TestCode(0,"ADC A,(IX+d)",0xdd,0x8e,0,0);
	CheckCode(A_REG==0x6 && N_FLAG==1 && PV_FLAG==0 && Z_FLAG==0 && C_FLAG==0);
	//
	A_REG=0x4;C_FLAG=1;IX_REG=4;Memory(4)=1;
	TestCode(0,"ADD A,(IX+d)",0xdd,0x86,0,0);
	CheckCode(A_REG==0x5 && N_FLAG==1 && PV_FLAG==0 && Z_FLAG==0 && C_FLAG==0);
	//
	HL_REG=0x02;IX_REG=4;Memory(4)=3;
	TestCode(0,"INC (IX+d)",0xdd,0x34,0,0);
	CheckCode(Memory(4)==0x4 && N_FLAG==0 && PV_FLAG==0 && Z_FLAG==0 && C_FLAG==0);
	//
	HL_REG=0x02;IY_REG=4;Memory(4)=3;
	TestCode(0,"INC (IY+d)",0xfd,0x34,0,0);
	CheckCode(Memory(4)==0x4 && N_FLAG==0 && PV_FLAG==0 && Z_FLAG==0 && C_FLAG==0);
	//
	printf("[TEST07]SUB 8BIT\n");
	//
	A_REG=0x3;B_REG=0x2;
	TestCode(0,"SUB B",0x90,0,0,0);
	CheckCode(A_REG==0x1 && N_FLAG==1 && PV_FLAG==0);
	//
	A_REG=0x2;B_REG=0x3;
	TestCode(0,"SUB B",0x90,0,0,0);
	CheckCode(A_REG==0xff && N_FLAG==1 && PV_FLAG==0 && S_FLAG==1);
	//
	A_REG=0x2;
	TestCode(0,"SUB 3",0xd6,3,0,0);
	CheckCode(A_REG==0xff && N_FLAG==1 && PV_FLAG==0 && S_FLAG==1 && C_FLAG==1);
	//
	A_REG=0x3;B_REG=0x2;C_FLAG=1;
	TestCode(0,"SBC A,B",0x98,0,0,0);
	CheckCode(A_REG==0x00 && N_FLAG==1 && PV_FLAG==0 && Z_FLAG==1);
	//
	A_REG=0x3;C_FLAG=1;
	TestCode(0,"SBC A,2",0xde,2,0,0);
	CheckCode(A_REG==0x00 && N_FLAG==1 && PV_FLAG==0 && Z_FLAG==1);
	//
	B_REG=0x3;
	TestCode(0,"DEC B",0x5,0,0,0);
	CheckCode(B_REG==0x02 && N_FLAG==1 && PV_FLAG==0 && Z_FLAG==0);
	//
	HL_REG=0x02;
	TestCode(0,"DEC (HL)",0x35,0,3,0);
	CheckCode(Memory(2)==0x2 && N_FLAG==1 && PV_FLAG==0 && Z_FLAG==0 && C_FLAG==0);
	//
	A_REG=0x4;C_FLAG=1;HL_REG=2;
	TestCode(0,"SBC A,(HL)",0x9E,0,1,0);
	CheckCode(A_REG==0x2 && N_FLAG==1 && PV_FLAG==0 && Z_FLAG==0 && C_FLAG==0);
	//
	A_REG=0x4;C_FLAG=1;HL_REG=2;
	TestCode(0,"SUB (HL)",0x96,0,1,0);
	CheckCode(A_REG==0x3 && N_FLAG==1 && PV_FLAG==0 && Z_FLAG==0 && C_FLAG==0);
	
	//
	IX_REG=0x03;
	TestCode(0,"DEC (IX+d)",0xdd,0x35,0,3);
	CheckCode(Memory(3)==0x2 && N_FLAG==1 && PV_FLAG==0 && Z_FLAG==0 && C_FLAG==0);
	//
	A_REG=0x4;C_FLAG=1;IX_REG=3;
	TestCode(0,"SBC A,(IX+d)",0xdd,0x9E,0,1);
	CheckCode(A_REG==0x2 && N_FLAG==1 && PV_FLAG==0 && Z_FLAG==0 && C_FLAG==0);
	//
	A_REG=0x4;C_FLAG=1;IX_REG=3;
	TestCode(0,"SUB (IX+d)",0xdd,0x96,0,1);
	CheckCode(A_REG==0x3 && N_FLAG==1 && PV_FLAG==0 && Z_FLAG==0 && C_FLAG==0);
	
	//
	printf("[TEST08]ADD/SUB 16BIT\n");
	//
	HL_REG=0xfffe;BC_REG=0x0002;
	TestCode(0,"ADD HL,BC",0x09,0,0,0);
	CheckCode(HL_REG==0x0000 && N_FLAG==0 && C_FLAG==1);
	//
	BC_REG=0x0002;C_FLAG=1;
	TestCode(0,"INC BC",0x03,0,0,0);
	CheckCode(BC_REG==0x0003 && C_FLAG==1);
	//
	BC_REG=0x0002;C_FLAG=1;
	TestCode(0,"DEC BC",0x0B,0,0,0);
	CheckCode(BC_REG==0x0001 && C_FLAG==1);
	//
	IX_REG=0xfffe;BC_REG=0x0002;
	TestCode(0,"ADD IX,BC",0xdd,0x09,0,0);
	CheckCode(IX_REG==0x0000 && N_FLAG==0 && C_FLAG==1);
	//
	IX_REG=0x0002;C_FLAG=1;
	TestCode(0,"INC IX",0xdd,0x23,0,0);
	CheckCode(IX_REG==0x0003 && C_FLAG==1);
	//
	IX_REG=0x0002;C_FLAG=1;
	TestCode(0,"DEC IX",0xdd,0x2B,0,0);
	CheckCode(IX_REG==0x0001 && C_FLAG==1);
	*/
	//
	A_REG=255;B_REG=2;
	TestCode(0,"MLT A,B",0xed,0xc1,0,0);
	CheckCode(HL_REG==510 && C_FLAG==1);
	//
	A_REG=2;C_REG=0;
	TestCode(0,"MLT A,C",0xed,0xc9,0,0);
	CheckCode(HL_REG==0 && Z_FLAG==1);
	//
	HL_REG=300;BC_REG=300;
	TestCode(0,"MLT HL,BC",0xed,0xc3,0,0);
	CheckCode(HL_REG==0x5f90 && DE_REG==1 && C_FLAG==1);
	//
	HL_REG=0;SP_REG=0;
	TestCode(0,"MLT HL,BC",0xed,0xF3,0,0);
	CheckCode(HL_REG==0 && DE_REG==0 && Z_FLAG==1);
	//
	A_REG=100;B_REG=21;
	TestCode(0,"DIV A,B",0xed,0xc5,0,0);
	CheckCode(L_REG==4 && H_REG==16 && C_FLAG==0);
	//
	A_REG=100;B_REG=0;
	TestCode(0,"DIV A,B",0xed,0xc5,0,0);
	CheckCode(C_FLAG==1);
	//
	HL_REG=500;BC_REG=21;
	TestCode(0,"DIV HL,BC",0xed,0xc7,0,0);
	CheckCode(HL_REG==23 && DE_REG==17 && C_FLAG==0);
	
	return ErrorUnit;
}
int Test_LOGICAL()
{
	ErrorUnit=0;

	//
	printf("[TEST10]LOGICAL \n");
	//
	A_REG=0x06;B_REG=0x003;C_FLAG=1;
	TestCode(0,"AND B",0xA0,0,0,0);
	CheckCode(A_REG==0x0002 && C_FLAG==0 && Z_FLAG==0 && S_FLAG==0);
	//
	A_REG=0x01;B_REG=0x002;C_FLAG=1;
	TestCode(0,"OR B",0xB0,0,0,0);
	CheckCode(A_REG==0x0003 && C_FLAG==0 && Z_FLAG==0 && S_FLAG==0);
	//
	A_REG=0x01;B_REG=0x0ff;C_FLAG=1;
	TestCode(0,"XOR B",0xA8,0,0,0);
	CheckCode(A_REG==0x00fe && C_FLAG==0 && Z_FLAG==0 && S_FLAG==1);
	//
	A_REG=0xc;
	TestCode(0,"OR n",0xf6,0x3,0x00,0x00);
	CheckCode(A_REG==0x0f && Z_FLAG==0 && C_FLAG==0 && H_FLAG==0 && S_FLAG==0 && PV_FLAG==1);
	//
	A_REG=0xf;
	TestCode(0,"AND n",0xe6,0x3,0x00,0x00);
	CheckCode(A_REG==0x03 && Z_FLAG==0 && C_FLAG==0 && H_FLAG==0 && S_FLAG==0 && PV_FLAG==1);
	//
	A_REG=0x3;HL_REG=0x0002;
	TestCode(0,"XOR (HL)",0xae,0x0,0x01,0x00);
	CheckCode(A_REG==0x02 && Z_FLAG==0 && C_FLAG==0 && H_FLAG==0 && S_FLAG==0 && PV_FLAG==0);
	//
	A_REG=0x3;HL_REG=0x0002;
	TestCode(0,"AND (HL)",0xa6,0x0,0x01,0x00);
	CheckCode(A_REG==0x01 && Z_FLAG==0 && C_FLAG==0 && H_FLAG==0 && S_FLAG==0 && PV_FLAG==0);
	//
	A_REG=0x2;HL_REG=0x0002;
	TestCode(0,"OR (HL)",0xb6,0x0,0x01,0x00);
	CheckCode(A_REG==0x03 && Z_FLAG==0 && C_FLAG==0 && H_FLAG==0 && S_FLAG==0 && PV_FLAG==1);
		//
	A_REG=0x3;IX_REG=0x0003;
	TestCode(0,"XOR (IX+d)",0xdd,0xae,0x0,0x01);
	CheckCode(A_REG==0x02 && Z_FLAG==0 && C_FLAG==0 && H_FLAG==0 && S_FLAG==0 && PV_FLAG==0);
	//
	A_REG=0x3;IX_REG=0x0003;
	TestCode(0,"AND (IX+d)",0xdd,0xa6,0x0,0x01);
	CheckCode(A_REG==0x01 && Z_FLAG==0 && C_FLAG==0 && H_FLAG==0 && S_FLAG==0 && PV_FLAG==0);
	//
	A_REG=0x2;IX_REG=0x0003;
	TestCode(0,"OR (IX+d)",0xdd,0xb6,0x0,0x01);
	CheckCode(A_REG==0x03 && Z_FLAG==0 && C_FLAG==0 && H_FLAG==0 && S_FLAG==0 && PV_FLAG==1);
	//
	A_REG=0x02;H_FLAG=0;N_FLAG=0;
	TestCode(0,"CPL",0x2f,0,0,0);
	CheckCode(A_REG==0x00fd && H_FLAG==1 && N_FLAG==1);

	return ErrorUnit;
}
int Test_BIT()
{
	ErrorUnit=0;

	//
	printf("[TEST11]BIT Operation \n");
	//
	H_FLAG=1;C_FLAG=1;
	TestCode(0,"CCF",0x3f,0,0,0);
	CheckCode(C_FLAG==0 && H_FLAG==1 && N_FLAG==0);
	//
	H_FLAG=1;N_FLAG=1;C_FLAG=0;
	TestCode(0,"SCF",0x37,0,0,0);
	CheckCode(C_FLAG==1 && H_FLAG==0 && N_FLAG==0);

	//
	B_REG=3;
	TestCode(0,"BIT 0,B",0xcb,0x40,0x00,0x00);
	CheckCode(Z_FLAG==1);
	//
	B_REG=8;
	TestCode(0,"BIT 0,B",0xcb,0x40,0x00,0x00);
	CheckCode(Z_FLAG==0);
	//
	HL_REG=3;
	TestCode(0,"BIT b,(HL)",0xcb,0x4e,0x00,0x02);
	CheckCode(Z_FLAG==1);

	//
	B_REG=8;
	TestCode(0,"SET 0,B",0xcb,0xc0,0x00,0x00);
	CheckCode(B_REG==0x09);
	//
	HL_REG=3;
	TestCode(0,"SET b,(HL)",0xcb,0xce,0x00,0x01);
	CheckCode(Memory(3)==3);
	//
	B_REG=7;
	TestCode(0,"RES 0,B",0xcb,0x80,0x00,0x00);
	CheckCode(B_REG==0x06);
	//
	HL_REG=3;
	TestCode(0,"RES b,(HL)",0xcb,0x8e,0x00,0x03);
	CheckCode(Memory(3)==1);
	//
	HL_REG=0;IX_REG=4;Memory(4)=1;
	TestCode(0,"SET b,(IX+n)",0xdd,0xcb,0xce,0x00);
	CheckCode(Memory(4)==3);

	return ErrorUnit;
}
int Test_CP()
{
	ErrorUnit=0;

	//
	printf("[TEST12]CP Operation \n");
/*
	//
	A_REG=1;B_REG=1;
	TestCode(0,"CP B",0xB8,0,0,0);
	CheckCode(Z_FLAG==1 && C_FLAG==0);
	//
	A_REG=1;HL_REG=2;
	TestCode(0,"CP (HL)",0xBE,0,2,0);
	CheckCode(Z_FLAG==0 && C_FLAG==1);
	//
	A_REG=0x001;
	TestCode(0,"CP n",0xfe,0x1,0x00,0x00);
	CheckCode(A_REG==0x1 && Z_FLAG==1 && C_FLAG==0 && N_FLAG==1);
	//
	A_REG=0x001;
	TestCode(0,"CP n",0xfe,0x2,0x00,0x00);
	CheckCode(Z_FLAG==0 && C_FLAG==1 && N_FLAG==1 && H_FLAG==1 && S_FLAG==1);
	//
	A_REG=0x0ff;
	TestCode(0,"CP n",0xfe,0x2,0x00,0x00);
	CheckCode(Z_FLAG==0 && C_FLAG==0 && N_FLAG==1 && H_FLAG==0 && S_FLAG==1);
	//
	A_REG=0x00;
	TestCode(0,"CP n",0xfe,0x2,0x00,0x00);
	CheckCode(Z_FLAG==0 && C_FLAG==1 && N_FLAG==1 && H_FLAG==1 && S_FLAG==1 && PV_FLAG==0);
	//
	A_REG=0x80;
	TestCode(0,"CP n",0xfe,0x2,0x00,0x00);
	CheckCode(Z_FLAG==0 && C_FLAG==0 && N_FLAG==1 && H_FLAG==1 && S_FLAG==0 && PV_FLAG==1);
	//
	A_REG=0xff;
	TestCode(0,"CP n",0xee,0x3,0x00,0x00);
	CheckCode(A_REG==0xfc && Z_FLAG==0 && C_FLAG==0 && H_FLAG==0 && S_FLAG==1 && PV_FLAG==1);
	//
	A_REG=0x1;HL_REG=2;BC_REG=2;
	TestCode(0,"CPI",0xed,0xa1,0x01,0x02);
	CheckCode(HL_REG==3 && BC_REG==1 && Z_FLAG==1 && H_FLAG==0 && S_FLAG==0 && PV_FLAG==1 && N_FLAG==1);
	//
	A_REG=0x2;HL_REG=2;BC_REG=2;
	TestCode(0,"CPIR",0xed,0xb1,0x01,0x02);
	CheckCode(HL_REG==4 && BC_REG==0 && Z_FLAG==1 && H_FLAG==0 && S_FLAG==0 && PV_FLAG==0 && N_FLAG==1);
	//
	A_REG=0x1;HL_REG=3;BC_REG=2;
	TestCode(0,"CPD",0xed,0xa9,0x02,0x01);
	CheckCode(HL_REG==2 && BC_REG==1 && Z_FLAG==1 && H_FLAG==0 && S_FLAG==0 && PV_FLAG==1 && N_FLAG==1);
	//
	A_REG=0x2;HL_REG=3;BC_REG=2;
	TestCode(0,"CPDR",0xed,0xb9,0x03,0x01);
	CheckCode(HL_REG==1 && BC_REG==0 && Z_FLAG==0 && H_FLAG==1 && S_FLAG==1 && PV_FLAG==0 && N_FLAG==1);
	*/

	//
	A_REG=2;IX_REG=2;
	TestCode(0,"CP (IX+1)",0xdd,0xBE,1,2);
	CheckCode(Z_FLAG==1 && C_FLAG==0);
	//
	A_REG=2;IY_REG=2;
	TestCode(0,"CP (IY+1)",0xfd,0xBE,1,3);
	CheckCode(Z_FLAG==0 && C_FLAG==1);
	//

	
	return ErrorUnit;
}
int Test_JUMP()
{
	ErrorUnit=0;

	//
	printf("[TEST13]JUMP Operation \n");
	//
	TestCode(0,"JP 0x0004",0xC3,0x34,0x12,0);
	CheckCode(PC_REG==0x1234);
	//
	Z_FLAG=0;//NZ=0,Z=1,NC=2,C=3
	TestCode(0,"JP NZ,0x1234",0xC2,0x34,0x12,0);
	CheckCode(PC_REG==0x1234);
	//
	Z_FLAG=1;//NZ=0,Z=1,NC=2,C=3
	TestCode(0,"JP Z,0x1234",0xCA,0x34,0x12,0);
	CheckCode(PC_REG==0x1234);
	//
	C_FLAG=0;//NZ=0,Z=1,NC=2,C=3
	TestCode(0,"JP NC,0x1234",0xD2,0x34,0x12,0);
	CheckCode(PC_REG==0x1234);
	//
	C_FLAG=1;//NZ=0,Z=1,NC=2,C=3
	TestCode(0,"JP C,0x1234",0xDA,0x34,0x12,0);
	CheckCode(PC_REG==0x1234);
	//
	HL_REG=0x002;
	TestCode(0,"JP (HL)",0xe9,0x0,0x34,0x12);
	CheckCode(PC_REG==0x1234);
	//
	B_REG=0x002;
	TestCode(0,"DJNZ e",0x10,0xfe,0x00,0x00);
	CheckCode(PC_REG==0x0 && B_REG==1);
	//
	B_REG=0x001;
	TestCode(0,"DJNZ e",0x10,0xfe,0x00,0x00);
	CheckCode(PC_REG==0x2 && B_REG==0);
	//
	C_FLAG=1;
	TestCode(0,"JR C,e",0x38,0xfe,0x00,0x00);
	CheckCode(PC_REG==0x0);
	//
	Z_FLAG=1;
	TestCode(0,"JR Z,e",0x28,0xfe,0x00,0x00);
	CheckCode(PC_REG==0x0);

	return ErrorUnit;
}
int Test_CALL()
{
	ErrorUnit=0;

	//
	printf("[TEST14]CALL Operation \n");
	//
	TestCode(0,"CALL 0x1234",0xCD,0x34,0x12,0);
	CheckCode(PC_REG==0x1234);
	//
	Z_FLAG=0;//NZ=0,Z=1,NC=2,C=3
	TestCode(0,"CALL NZ,0x1234",0xC4,0x34,0x12,0);
	CheckCode(PC_REG==0x1234);
	//
	Z_FLAG=1;//NZ=0,Z=1,NC=2,C=3
	TestCode(0,"CALL Z,0x1234",0xCC,0x34,0x12,0);
	CheckCode(PC_REG==0x1234);
	//
	C_FLAG=0;//NZ=0,Z=1,NC=2,C=3
	TestCode(0,"CALL NC,0x1234",0xD4,0x34,0x12,0);
	CheckCode(PC_REG==0x1234);
	//
	C_FLAG=1;//NZ=0,Z=1,NC=2,C=3
	TestCode(0,"CALL C,0x1234",0xDC,0x34,0x12,0);
	CheckCode(PC_REG==0x1234);
	//
	Memory(0x3ffe)=0;Memory(0x3fff)=0;
	TestCode(0,"RET",0xC9,0x34,0x12,0);
	CheckCode(PC_REG==0x0000 && SP_REG==0x4000);
	//
	Z_FLAG=0;//NZ=0,Z=1,NC=2,C=3
	Memory(0x3ffe)=0;Memory(0x3fff)=0;
	TestCode(0,"RET NZ",0xC0,0x34,0x12,0);
	CheckCode(PC_REG==0x0000 && SP_REG==0x4000);
	//
	Z_FLAG=1;//NZ=0,Z=1,NC=2,C=3
	Memory(0x3ffe)=0;Memory(0x3fff)=0;
	TestCode(0,"RET Z",0xC8,0x34,0x12,0);
	CheckCode(PC_REG==0x0000 && SP_REG==0x4000);
	//
	C_FLAG=0;//NZ=0,Z=1,NC=2,C=3
	Memory(0x3ffe)=0;Memory(0x3fff)=0;
	TestCode(0,"RET NC",0xD0,0x34,0x12,0);
	CheckCode(PC_REG==0x0000 && SP_REG==0x4000);
	//
	C_FLAG=1;//NZ=0,Z=1,NC=2,C=3
	Memory(0x3ffe)=0;Memory(0x3fff)=0;
	TestCode(0,"RET C",0xD8,0x34,0x12,0);
	CheckCode(PC_REG==0x0000 && SP_REG==0x4000);

	//
	TestCode(1,"RST 00h",0xc7,0x0,0x0,0x00);
	CheckCode(PC_REG==0);
	//
	TestCode(1,"RST 10h",0xd7,0x0,0x0,0x00);
	CheckCode(PC_REG==0x10);
	//
	TestCode(1,"RST 20h",0xe7,0x0,0x0,0x00);
	CheckCode(PC_REG==0x20);
	//
	TestCode(1,"RST 30h",0xf7,0x0,0x0,0x00);
	CheckCode(PC_REG==0x30);
	//
	TestCode(1,"RST 08h",0xcf,0x0,0x0,0x00);
	CheckCode(PC_REG==0x08);
	//
	TestCode(1,"RST 18h",0xdf,0x0,0x0,0x00);
	CheckCode(PC_REG==0x18);
	//
	TestCode(1,"RST 28h",0xef,0x0,0x0,0x00);
	CheckCode(PC_REG==0x28);
	//
	TestCode(1,"RST 38h",0xff,0x0,0x0,0x00);
	CheckCode(PC_REG==0x38);

	//
	SP_REG=0x3ffe;IFF2_REG=1;IFF1_REG=0;
	Memory(0x3ffe)=0x34;Memory(0x3fff)=0x12;
	TestCode(0,"RETI",0xED,0x4D,0x0,0);
	CheckCode(PC_REG==0x1234 && SP_REG==0x4000 && IFF1_REG==1);
	//
	SP_REG=0x3ffe;IFF2_REG=0;IFF1_REG=1;
	Memory(0x3ffe)=0x34;Memory(0x3fff)=0x56;
	TestCode(0,"RETD",0xED,0x45,0x0,0);
	CheckCode(PC_REG==0x5634 && SP_REG==0x4000 && IFF1_REG==0);

	
	return ErrorUnit;
}
int Test_OUTIN()
{
	ErrorUnit=0;

	//
	printf("[TEST15]IN/OUT \n");
	//
	A_REG=10;IOMemory[1]=10;
	TestCode(0,"OUT (01),A",0xD3,0x1,0,0);
	CheckCode(IOMemory[1]==10);
	//
	A_REG=10;IOMemory[2]=3;
	TestCode(0,"IN A,(02)",0xDB,0x2,0,0);
	CheckCode(A_REG==3);
	//
	B_REG=30;C_REG=2;IOMemory[2]=0;
	TestCode(0,"OUT (C),B",0xED,0x41,0,0);
	CheckCode(IOMemory[2]==30);
	//
	B_REG=30;C_REG=2;IOMemory[2]=0x82;
	TestCode(0,"IN B,(C)",0xED,0x40,0,0);
	CheckCode(B_REG==0x82 && PV_FLAG==1 && S_FLAG==1);
	/*
	//
	B_REG=2;HL_REG=4;C_REG=2;IOMemory[2]=0;
	Memory(4)=10;Memory(5)=11;Memory(6)=13;
	TestCode(0,"OUTI",0xED,0xa3,0,0);
	CheckCode(IOMemory[2]==10 && B_REG==1);
	TestCode(0,"OUTI",0xED,0xa3,0,0);
	CheckCode(IOMemory[2]==11 && B_REG==0);
	//
	B_REG=2;HL_REG=5;C_REG=2;IOMemory[2]=0;
	Memory(4)=10;Memory(5)=11;Memory(6)=13;
	TestCode(0,"OUTI",0xED,0xab,0,0);
	CheckCode(IOMemory[2]==11 && B_REG==1);
	TestCode(0,"OUTI",0xED,0xab,0,0);
	CheckCode(IOMemory[2]==10 && B_REG==0);
	//
	B_REG=2;HL_REG=4;C_REG=2;IOMemory[2]=0;
	Memory(4)=10;Memory(5)=11;Memory(6)=13;
	TestCode(0,"OUTIR",0xED,0xb3,0,0);
	CheckCode(IOMemory[2]==11 && B_REG==0);
	//
	B_REG=2;HL_REG=5;C_REG=2;IOMemory[2]=0;
	Memory(4)=10;Memory(5)=11;Memory(6)=13;
	TestCode(0,"OUTDR",0xED,0xbb,0,0);
	CheckCode(IOMemory[2]==10 && B_REG==0);
	//
	B_REG=2;HL_REG=5;C_REG=2;IOMemory[2]=0;
	Memory(4)=10;Memory(5)=11;Memory(6)=13;
	TestCode(0,"OUTD",0xED,0xab,0,0);
	CheckCode(IOMemory[2]==11 && B_REG==1);
	//
	B_REG=2;HL_REG=2;C_REG=2;IOMemory[2]=1;
	Memory(2)=0;Memory(3)=0;Memory(4)=0;
	TestCode(0,"INI",0xED,0xa2,0,0);
	CheckCode(Memory(2)==1 && Memory(3)==0 && B_REG==1 && HL_REG==3);
	//
	B_REG=2;HL_REG=2;C_REG=2;IOMemory[2]=1;
	Memory(2)=0;Memory(3)=0;Memory(4)=0;
	TestCode(0,"INIR",0xED,0xb2,0,0);
	CheckCode(Memory(2)==1 && Memory(3)==1 && B_REG==0 && HL_REG==4);
	//
	B_REG=2;HL_REG=4;C_REG=2;IOMemory[2]=1;
	Memory(2)=0;Memory(3)=0;Memory(4)=0;
	TestCode(0,"IND",0xED,0xaa,0,0);
	CheckCode(Memory(4)==1 && Memory(3)==0 && B_REG==1 && HL_REG==3);
	//
	B_REG=2;HL_REG=4;C_REG=2;IOMemory[2]=1;
	Memory(2)=0;Memory(3)=0;Memory(4)=0;
	TestCode(0,"INDR",0xED,0xba,0,0);
	CheckCode(Memory(4)==1 && Memory(3)==1 && B_REG==0 && HL_REG==2);
*/
	return ErrorUnit;
}
int Test_DAA()
{
	ErrorUnit=0;

	//
	printf("[TEST16]DAA \n");

	//
	A_REG=0x0f;H_FLAG=0;N_FLAG=0;C_FLAG=0;
	TestCode(0,"DAA",0x27,0x0,0,0);
	CheckCode(A_REG==0x15);
	//
	A_REG=0x1a;H_FLAG=0;N_FLAG=0;C_FLAG=0;
	TestCode(0,"DAA",0x27,0x0,0,0);
	CheckCode(A_REG==0x20);
	//
	A_REG=0x19;H_FLAG=1;N_FLAG=1;C_FLAG=0;
	TestCode(0,"DAA",0x27,0x0,0,0);
	CheckCode(A_REG==0x13);
	//
	A_REG=0x19;H_FLAG=1;N_FLAG=0;C_FLAG=0;
	TestCode(0,"DAA",0x27,0x0,0,0);
	CheckCode(A_REG==0x1F);
	//
	A_REG=0xA9;H_FLAG=1;N_FLAG=0;C_FLAG=0;
	TestCode(0,"DAA",0x27,0x0,0,0);
	CheckCode(A_REG==0x0F && C_FLAG==1);
	
	//
	A_REG=0xff;H_FLAG=0;N_FLAG=0;C_FLAG=0;
	TestCode(0,"DAA",0x27,0x0,0,0);
	CheckCode(A_REG==0x65 && C_FLAG==1);
	//
	A_REG=0xff;H_FLAG=1;N_FLAG=1;C_FLAG=0;
	TestCode(0,"DAA",0x27,0x0,0,0);
	CheckCode(A_REG==0x99 && C_FLAG==1 && S_FLAG==1 && N_FLAG==1 && PV_FLAG==1);
	
	//
	A_REG=0xf9;H_FLAG=1;N_FLAG=1;C_FLAG=0;
	TestCode(0,"DAA",0x27,0x0,0,0);
	CheckCode(A_REG==0x93 && C_FLAG==1);
	//
	A_REG=0x00;H_FLAG=1;N_FLAG=0;C_FLAG=0;
	TestCode(0,"DAA",0x27,0x0,0,0);
	CheckCode(A_REG==0x06);
	//
	A_REG=0x00;H_FLAG=1;N_FLAG=0;C_FLAG=1;
	TestCode(0,"DAA",0x27,0x0,0,0);
	CheckCode(A_REG==0x66);
	//
	A_REG=0x00;H_FLAG=0;N_FLAG=0;C_FLAG=1;
	TestCode(0,"DAA",0x27,0x0,0,0);
	CheckCode(A_REG==0x60);
	//
	A_REG=0x0A;H_FLAG=0;N_FLAG=0;C_FLAG=0;
	TestCode(0,"DAA",0x27,0x0,0,0);
	CheckCode(A_REG==0x10);
	//
	A_REG=0x00;H_FLAG=1;N_FLAG=1;C_FLAG=0;
	TestCode(0,"DAA",0x27,0x0,0,0);
	CheckCode(A_REG==0xFA);
	//
	A_REG=0x00;H_FLAG=1;N_FLAG=1;C_FLAG=1;
	TestCode(0,"DAA",0x27,0x0,0,0);
	CheckCode(A_REG==0x9A);
	//
	A_REG=0x00;H_FLAG=0;N_FLAG=1;C_FLAG=1;
	TestCode(0,"DAA",0x27,0x0,0,0);
	CheckCode(A_REG==0xA0);
	//
	A_REG=0xA0;H_FLAG=0;N_FLAG=0;C_FLAG=0;
	TestCode(0,"DAA",0x27,0x0,0,0);
	CheckCode(A_REG==0x00 && C_FLAG==1 && Z_FLAG==1);
	
	//
	A_REG=0xFF;H_FLAG=0;N_FLAG=0;C_FLAG=0;
	TestCode(0,"DAA",0x27,0x0,0,0);
	CheckCode(A_REG==0x65 && C_FLAG==1 && H_FLAG==1);
	//
	A_REG=0xFF;H_FLAG=0;N_FLAG=0;C_FLAG=0;
	TestCode(0,"DAA",0x27,0x0,0,0);
	CheckCode(A_REG==0x65 && C_FLAG==1 && H_FLAG==1);
	//
	A_REG=0xFF;H_FLAG=0;N_FLAG=1;C_FLAG=0;
	TestCode(0,"DAA",0x27,0x0,0,0);
	CheckCode(A_REG==0x99 && C_FLAG==1 && S_FLAG==1);
	//
	A_REG=0xFF;H_FLAG=0;N_FLAG=1;C_FLAG=1;
	TestCode(0,"DAA",0x27,0x0,0,0);
	CheckCode(A_REG==0x99 && C_FLAG==1 && S_FLAG==1);
	//
	A_REG=0xFF;H_FLAG=0;N_FLAG=1;C_FLAG=1;
	TestCode(0,"DAA",0x27,0x0,0,0);
	CheckCode(A_REG==0x99 && C_FLAG==1 && S_FLAG==1);
	//
	A_REG=0xFF;H_FLAG=1;N_FLAG=1;C_FLAG=1;
	TestCode(0,"DAA",0x27,0x0,0,0);
	CheckCode(A_REG==0x99 && C_FLAG==1 && S_FLAG==1);
	//
	A_REG=0x0F;H_FLAG=1;N_FLAG=1;C_FLAG=0;
	TestCode(0,"DAA",0x27,0x0,0,0);
	CheckCode(A_REG==0x09 && C_FLAG==0 && S_FLAG==0);
	//
	A_REG=0x0F0;H_FLAG=0;N_FLAG=1;C_FLAG=1;
	TestCode(0,"DAA",0x27,0x0,0,0);
	CheckCode(A_REG==0x090 && C_FLAG==1 && S_FLAG==1);

	return ErrorUnit;
}
int Test_CPU()
{
	ErrorUnit=0;

	//
	F_REG=0xff;
	TestCode(0,"NOP",0x0,0x0,0,0);
	CheckCode(F_REG==0xff );
	//
	TestCode(0,"DI",0xf3,0x0,0,0);
	CheckCode(IFF1_REG==0x0 && IFF2_REG==0x0);
	//
	TestCode(0,"EI",0xfb,0x0,0,0);
	CheckCode(IFF1_REG==0x1 && IFF2_REG==0x1);
	//
	TestCode(0,"IM 0",0xed,0x46,0,0);
	//
	TestCode(0,"IM 1",0xed,0x56,0,0);
	//
	TestCode(0,"IM 2",0xed,0x5e,0,0);
	return ErrorUnit;
}
int Test_BLOCK()
{
	ErrorUnit=0;

	return ErrorUnit;
}
/**
IO処理エミュレーション
*/
void IOTask()
{
	if(IOAccessFlag){
		if(IOAccessFlag==IO_OUT){
			_IOMemory[IOAccessPort]=IOAccessData;
		}else{
			IOAccessData=_IOMemory[IOAccessPort];
		}
		IOAccessFlag=0;
	}
}

int main(int argc,char* argv[])
{
	int a;
	printf("z80 emulater Code Test\n");
	InitZ80(_WorkMemory,_IOMemory);
	//IOコールバック設定
	IOTaskCallback=IOTask;

	int results[20]={0};
	
	/*
	results[0]=Test_LD();
	results[1]=Test_PUSHPOP();
	results[2]=Test_EX();
	results[3]=Test_ROTATE();
	results[4]=Test_SHIFT();
	*/
	results[5]=Test_CALC();
	/*
	results[6]=Test_LOGICAL();
	results[7]=Test_BIT();
	results[8]=Test_CP();
	results[9]=Test_JUMP();
	results[10]=Test_CALL();
	results[11]=Test_OUTIN();
	results[12]=Test_DAA();
	results[13]=Test_CPU();
	results[14]=Test_BLOCK();
*/

	printf("TEST LD Error=%d\n",results[0]);
	printf("TEST PUSHPOP Error=%d\n",results[1]);
	printf("TEST EX Error=%d\n",results[2]);
	printf("TEST ROTATE Error=%d\n",results[3]);
	printf("TEST SHIFT Error=%d\n",results[4]);
	printf("TEST CALC Error=%d\n",results[5]);
	printf("TEST LOGICAL Error=%d\n",results[6]);
	printf("TEST BIT Error=%d\n",results[7]);
	printf("TEST CP Error=%d\n",results[8]);
	printf("TEST JUMP Error=%d\n",results[9]);
	printf("TEST CALL Error=%d\n",results[10]);
	printf("TEST OUTIN Error=%d\n",results[11]);

	printf("TEST DAA Error=%d\n",results[12]);
	printf("TEST CPU Error=%d\n",results[13]);
	printf("TEST BLOCK Error=%d\n",results[14]);

	
	//
	//テストの総合結果
	printf("-----------------------------\n");
	printf("Test Result \n");
	printf("Success Count=%d\n",SuccessCount);
	printf("NG      Count=%d\n",ErrorCount);
	return 0;
}

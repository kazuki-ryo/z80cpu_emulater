#define DEBUG_VIEW 0

#include"z80.c"

unsigned char _WorkMemory[65536];//MAIN Memory
unsigned char _IOMemory[256];//I/O AREA

//配置アドレスは0x0000です
//BDOS(0005H)のfunction 2と9のみ実装
unsigned char BDOS_SYSTEM[256]={
	0xcd,0x00,0x01,//JP 0100h プログラム実行
	0x76,0,//HALT NOP　強制停止
	0x79,0xfe,0x2,0x20,0x4,//LD A,C;CP 2 JR NZ,NEXT1
	0x7b,0xd3,0x98,0xc9,0xfe,0x9,//LD A,E;OUT (0x98),A;RET;CP 9
	0xc0,0x1a,0xfe,0x24,0xc8,//RET NZ;LD A,(DE);CP '$';RET Z
	0xd3,0x98,0x13,0x18,0xf7//OUT (0x98),A;INC DE;JR LOOP1
};
//配置アドレスは0x0100です
unsigned char SampleData[256]={
	//連続IO読み込み＋IO書き込み
	0x21,17,0x01,//LD HL,DATA
	0x06,3,//LD B,3
	0x0e,0x98,//LD C,0x98
	0xed,0xB2,//INIR
	0x21,17,0x01,//LD HL,DATA
	0x06,5,//LD B,3
	0xed,0xb3,//OUTIR
	0xc9,//HALT
	1,2,3,13,10,//DATA
	//連続IO出力テスト
	0x21,0x0e,0x01,//LD HL,DATA
	0x06,5,//LD B,3
	0x0e,0x98,//LD C,0x98
	0xed,0xb3,//OUTIR
	0x0,0x0,//OUTI
	0x0,0x0,//OUTI
	0x76,//HALT
	65,66,67,13,10,//DATA
	//IO読み込みテスト
	0x3e,0x34,0xd3,0x10,//LD A,0x34:OUT (0x10),A
	0xdb,0x10,//IN A,(n)
	0x3e,1,0xd3,0x99,0x3e,0x58,0xd3,0x99,0x3e,65,0xd3,0x98,
	0x3e,1,0xd3,0x99,0x3e,0x18,0xd3,0x99,
	0xdb,0x98,//IN A,(0x98)
	0xdb,0x98,//IN A,(0x98)
	0x76,
	//VRAMアドレス設定と１バイト書き込み
	0x3e,1,0xd3,0x99,0x3e,0x58,0xd3,0x99,0x3e,65,0xd3,0x98,
	0x76,
	//BDOS TEST
	0x1e,0x41,0xe,0x2,0xcd,0x5,0x00,
	0x1e,13,0xe,0x2,0xcd,0x5,0x00,
	0x1e,10,0xe,0x2,0xcd,0x5,0x00,
	0x1e,0x42,0xe,0x2,0xcd,0x5,0x00,
	0x1e,0x43,0xe,0x2,0xcd,0x5,0x00,
	0x11,45,0x01,0xe,0x9,0xcd,0x5,0x00,
	0x76,0xc9,
	13,10,'H','E','L','L','O',13,10,'$'
	/*
	//1文字表示テスト
	0x3e,65,//LD A,65
	0xd3,0x98,//OUT (0x98),A
	0x3e,66,//LD A,66
	0xd3,0x98,//OUT (0x98),A
	0x76,//HALT
	0xc9//RET
	*/
	/*
	//IO portテスト用
	0x3e,4,//LD A,4		00 111 110,00000001
	0xd3,1, // out (n),A
	0xdb,1, //IN A,(n)
	0x3e,5,//LD A,5		00 111 110,00000001
	0xd3,2, // out (n),A
	0xdb,2, //IN A,(n)
	0x76,//HALT
	*/
	/*
	//ベンチマーク用１
	0x11,0xd0,0x7, //LD DE,10
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
	*/
};
//int IOAccessFlag=0;//IOにアクセスするときのフラグ（非同期）、OUT=1、IN=2
//int IOAccessPort=0;//IOアクセスポート番号
//unsigned char IOAccessData=0;//書き込み時の非同期データ

/**
VDP処理エミュレーション
*/
#define VDP_PATTERN_NAME_TABLE 0x1800 //SCREEN1=0x1800 SCREEN0=0x0
int VRAMAddress=VDP_PATTERN_NAME_TABLE;//VRAM読み書きアドレス
unsigned char VDPPort[4]={0,0,0,0};//4つのポートにセットされた値を一時保存する
int VRAMReadWriteFlag=1;//0=Read,1=Write
int VRAMAddressSetting=0;//0,1=アドレス設定（１４ビット連続）
unsigned char VRAMVirtualAddress[256*3]={'M','S','X',' ','B','A','S','I','C'};//仮想画面 SCREEN1 32*24
unsigned char VDPScreenMode=1;//選択されたスクリーンモード

void VDPTask()
{
	switch(IOAccessPort){
		case 0x98://VRAM読み書きモード
		if(VRAMReadWriteFlag==1){
			//出力モード
			printf("%c",VDPPort[0]);//コンソール出力テスト
			if(VRAMAddress>=VDP_PATTERN_NAME_TABLE && VRAMAddress<(VDP_PATTERN_NAME_TABLE+256*3)){
				VRAMVirtualAddress[VRAMAddress-VDP_PATTERN_NAME_TABLE]=VDPPort[0];
			}
			VRAMAddress++;
			VRAMAddressSetting=0;//１回出力ごとにアドレス設定のフラグをリセットする
		}else{
			//読み込みモード
			if(VRAMAddress>=VDP_PATTERN_NAME_TABLE && VRAMAddress<(VDP_PATTERN_NAME_TABLE+256*3)){
				IOAccessData=VRAMVirtualAddress[VRAMAddress-VDP_PATTERN_NAME_TABLE];
			}
			VRAMAddress++;
			VRAMAddressSetting=0;//１回出力ごとにアドレス設定のフラグをリセットする
		}
		break;
		case 0x99://VRAMアドレス設定
			//２回連続書き込みで１４ビットアドレスを設定する
			switch(VRAMAddressSetting){
				case 0:
					VRAMAddressSetting++;
					VRAMAddress=(VRAMAddress & 0xff00) | IOAccessData;
					break;
				case 1:
					VRAMAddressSetting++;
					VRAMAddress=(VRAMAddress & 0xff) | ((IOAccessData& 0x3f)<<8);
					if((IOAccessData & 0x40)!=0){VRAMReadWriteFlag=1;}else{VRAMReadWriteFlag=0;}
					printf("VRAMReadWriteFlag=%d VRAMAddress=%x\n",VRAMReadWriteFlag,VRAMAddress);
					break;
			}
			break;
	}
}
/**
IO処理エミュレーション
*/
void IOTask()
{
	if(IOAccessFlag){
		if(IOAccessPort>=0x98 && IOAccessPort<=0x9b){
			int Port=IOAccessPort-0x98;
			VDPPort[Port]=IOAccessData;
			if(Port==0){
				//入出力モードを自動で設定する（非互換）
				if(IOAccessFlag==IO_OUT){
					VRAMReadWriteFlag=1;
				}else{
					VRAMReadWriteFlag=0;
				}
			}
			VDPTask();
		}else{
			//VDP以外はとりあえずメモリに設定するだけ
			if(IOAccessFlag==IO_OUT){
				_IOMemory[IOAccessPort]=IOAccessData;
			}else{
				IOAccessData=_IOMemory[IOAccessPort];
			}
		}
		IOAccessFlag=0;
	}
}
/**
メイン処理
HALT実行でエミュレーション停止
*/
int TaskMain()
{
	int i,w;
//	for(i=0;i<1000;i++){
	unsigned short *pc_reg,*hl_reg;
	unsigned char *h_reg,*l_reg;
	pc_reg=&regs.PC;
	hl_reg=&regs.HL.SHORT;
	h_reg=&regs.HL.BYTE.HIGH;
	l_reg=&regs.HL.BYTE.LOW;
	//IOコールバック設定
	IOTaskCallback=IOTask;
	while(1){
		//コード読み込み
//		unsigned char code=Memory(*pc_reg);
		unsigned char code=Memory(PC_REG);
		#if DEBUG_VIEW
		printf("code=%x\n",code);
		#endif
		//コード解析
//		char State=CodeAnalysis(code,0,*hl_reg,*h_reg,*l_reg);
		char State=CodeAnalysis(code,0,HL_REG,H_REG,L_REG,&HL_REG);
		if(State==1){
			break;//中断
		}
		//PCカウントアップ
//		*pc_reg=(*pc_reg+1)&0xffff;
		PC_REG=(PC_REG+1)&0xffff;
		//ウェイト
		//ステートの数値分で疑似ウェイト発生
		/*#if DEBUG_VIEW
			for(w=0;w<500000;w++){
				Memory(0)=Memory(0);
			}
		if(ClockCount>10){break;}
		#endif*/
		//
		ClockCount=ClockCount+1;
		//メモリ表示（デバッグ用)
		#if DEBUG_VIEW

		ViewRegister();
		ViewMemory();
		printf("--\n");
		#endif
		//IOTask();
	}
	return 0;
}


int main(int argc,char* argv[])
{
	int a;
	printf("z80 emulater v0.10\n");
	//エミュレータ初期化
	InitZ80(_WorkMemory,_IOMemory);
	//BDOS設定
	for(a=0;a<sizeof(BDOS_SYSTEM);a++){
		WorkMemory[a]=BDOS_SYSTEM[a];
	}
	//テストコードをメモリに設定
	for(a=0;a<sizeof(SampleData);a++){
		WorkMemory[a+256]=SampleData[a];
	}
	//レジスタ初期化
	InitRegister();
	//エミュレータ起動
	TaskMain();
	//
	printf("End of Executed z80.\n");
	ViewRegister();
	ViewMemory();

	return 0;
}
/**
VDPの文字表示だけサポートするには以下を実装する。
out&h99,A0-A7
out&h99,0fxxxxxx  f:1=write 0=read  xxxxx=A8-A13
out&h98,data
*/

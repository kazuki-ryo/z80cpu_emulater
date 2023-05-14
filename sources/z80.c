#include<stdio.h>
#include"z80.h"
/*
Z80命令を翻訳して実行する。
実装：
・Z80命令
・DOSファンクション（文字入力、出力）
・文字表示I/O（VDP)
・文字表示BIOS(A2H)

オリジナルエミュレータをraspberrypi picoで実行し速度を比較する。
*/
int dummy=0;
#if DEBUG_VIEW
#define DEBUG_CODE(mes) printf(mes)
#else
#define DEBUG_CODE(mes) if(0){printf(mes);}
#endif

//z80 virtual memory
//unsigned char WorkMemory[65536];//MAIN Memory
//unsigned char IOMemory[256];//I/O AREA
unsigned char* WorkMemory;//MAIN Memory
unsigned char* IOMemory;//I/O AREA
#define Memory(address) WorkMemory[address]
#define MAX_MEMORY_SIZE 0xFFFF

#define CHAR8TO16(data) data>127?(-(256-data))&0xffff:data



//
REGISTER regs; //全レジスタ

int ClockCount=0;//クロックカウント
int Cycle;// 実行中の命令のステート数

/**
フラグステータス表示
*/
void ViewFlag()
{
	printf("FLAG S,Z,Y,H,X,PV,N,C=%d%d%d%d%d%d%d%d\n",
		regs.AF.BYTE.F.S,
		regs.AF.BYTE.F.Z,
		regs.AF.BYTE.F.Y,
		regs.AF.BYTE.F.H,
		regs.AF.BYTE.F.X,
		regs.AF.BYTE.F.PV,
		regs.AF.BYTE.F.N,
		regs.AF.BYTE.F.C
	);

}
/*
レジスタ全表示
*/
void ViewRegister()
{
	printf("AF   BC   DE   HL   IX   IY   SP   PC   AF2  BC2  DE2  HL2  \n");
	printf("%04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x \n",
		AF_REG,BC_REG,DE_REG,HL_REG,IX_REG,IY_REG,
		SP_REG,PC_REG,
		AF2_REG,BC2_REG,DE2_REG,HL2_REG
	);
	printf("I  R  IFF1 IFF2\n");
	printf("%02x %02x %02x   %02x \n",I_REG,R_REG,IFF1_REG,IFF2_REG);
	ViewFlag();
}
void ViewMemory()
{
	printf("PC[%02x %02x %02x %02x]\n",WorkMemory[PC_REG],WorkMemory[PC_REG+1],WorkMemory[PC_REG+2],WorkMemory[PC_REG+3]);
	printf("SP[%02x %02x]\n",WorkMemory[SP_REG],WorkMemory[(SP_REG+1)&0xffff]);
}
/*
フラグ全クリア
*/
void ClearFlag()
{
	regs.AF.BYTE.F.BYTE=0;
}
/*
起動時のレジスタ初期値
*/
void InitRegister()
{
	AF_REG=0x0000;
	BC_REG=0x0000;
	DE_REG=0x0000;
	HL_REG=0x0000;
	IX_REG=0x0000;
	IY_REG=0x0000;
	PC_REG=0x0000;
	SP_REG=MAX_MEMORY_SIZE;
	I_REG=R_REG=0x00;
	IFF_REG=IFF1_REG=IFF2_REG=0x0000;
	//裏レジスタ
	AF2_REG=0x0000;
	BC2_REG=0x0000;
	DE2_REG=0x0000;
	HL2_REG=0x0000;
}

//コード解析用ワークメモリ
unsigned char* DDD_PTR[8];
unsigned char* SSS_PTR[8];
unsigned short* RP_PTR[4];

/**
コード解析時に参照するレジスタマップ
*/
void SetRegisterAssress()
{
DDD_PTR[0]=SSS_PTR[0]=&B_REG;
DDD_PTR[1]=SSS_PTR[1]=&C_REG;
DDD_PTR[2]=SSS_PTR[2]=&D_REG;
DDD_PTR[3]=SSS_PTR[3]=&E_REG;
DDD_PTR[4]=SSS_PTR[4]=&H_REG;
DDD_PTR[5]=SSS_PTR[5]=&L_REG;
DDD_PTR[6]=SSS_PTR[6]=&F_REG;//ダミー
DDD_PTR[7]=SSS_PTR[7]=&A_REG;
RP_PTR[0]=&BC_REG;
RP_PTR[1]=&DE_REG;
RP_PTR[2]=&HL_REG;
RP_PTR[3]=&SP_REG;
}

/*
初期化、初期値設定
*/
void InitZ80(unsigned char* mem,unsigned char* iomem)
{
	int a;
	//メモリアドレス設定
	WorkMemory=mem;
	IOMemory=iomem;
	//レジスタ初期値
	SetRegisterAssress();
	//IOメモリ初期化
	for(a=0;a<sizeof(IOMemory);a++){
		IOMemory[a]=0;
	}
}
/**
加算実行時のフラグ設定
value=演算後の値(16bit)
bytes=演算前の値
*/
void SetFlagADDSUB16(int value,unsigned short word2,unsigned char sub)
{
	unsigned int word;
	unsigned char P_Count=0,i;
	word=(unsigned short)value & 0xffff;
	N_FLAG=sub;//加算1,減算0
	if(value>0xffff){C_FLAG=1;}else{C_FLAG=0;}
	//bit3からbit4に桁上がりがある場合
	if((word2&0x8000)==0 && (word&0x8000)==0x8000){H_FLAG=1;}else{H_FLAG=0;}
	//Z,P,PVは変化せず
}

/**
加算実行時のフラグ設定
value=演算後の値(16bit)
bytes=演算前の値
*/
void SetFlagADD(short value,unsigned char byte2)
{
	unsigned char byte;
	unsigned char P_Count=0,i;
	byte=(unsigned char)value & 0xff;
	N_FLAG=1;//加算1,減算0
	if(value>255){C_FLAG=1;}else{C_FLAG=0;}
	if(byte==0){Z_FLAG=1;}else{Z_FLAG=0;}
	//if(byte & 0x80){S_FLAG=1;}else{S_FLAG=0;}
	S_FLAG=(byte >>7);

	PV_FLAG=0;
	if(value>127){PV_FLAG=1;}
	if(value<-128){PV_FLAG=1;}
	//bit3からbit4に桁上がりがある場合
	if((byte2&8)==8 && (byte&16)==16){H_FLAG=1;}else{H_FLAG=0;}
	/*
	//論理演算の場合のみ1をカウントする
	for(i=0;i<8;i++){
		if(byte & 1){P_Count++;}
		byte=(byte>>1);
	}
	if(P_Count & 1){PV_FLAG=0;}else{PV_FLAG=1;}
	*/
}
/**
減算実行時のフラグ設定
value=演算後の値(16bit)
bytes=演算前の値
*/
void SetFlagSUB(short value,unsigned char byte2)
{
	unsigned char byte;
	unsigned char P_Count=0,i;
	byte=(unsigned char)value & 0xff;
	N_FLAG=1;//加算0,減算1
	if(value<0){C_FLAG=1;}else{C_FLAG=0;}
	if(byte==0){Z_FLAG=1;}else{Z_FLAG=0;}
	//if(byte & 0x80){S_FLAG=1;}else{S_FLAG=0;}
	S_FLAG=(byte >>7);

	PV_FLAG=0;
	printf("value=%d byte2=%d\n",value,byte2);
	if(value>127 && byte2<128){PV_FLAG=1;}
	if(value<-128 && byte2>127){PV_FLAG=1;}
	if(value<128 && byte2>127){PV_FLAG=1;}
	//bit3からbit4に桁上がりがある場合
//	if((byte2&8)==8 && (byte&16)==16){H_FLAG=1;}else{H_FLAG=0;}
	if((byte2&8)==0 && (byte&16)==16){H_FLAG=1;}else{H_FLAG=0;}
}

/**
加算実行時のフラグ設定
value=演算後の値(16bit)
bytes=演算前の値
*/
void SetFlagINCDEC(short value,unsigned char byte2,unsigned char dec)
{
	unsigned char byte;
	unsigned char P_Count=0,i;
	byte=(unsigned char)value & 0xff;
	N_FLAG=dec;//加算0,減算1
	if(value>255){C_FLAG=1;}else{C_FLAG=0;}
	if(byte==0){Z_FLAG=1;}else{Z_FLAG=0;}
	//if(byte & 0x80){S_FLAG=1;}else{S_FLAG=0;}
	S_FLAG=(byte >>7);
	PV_FLAG=0;
	if(value>127){PV_FLAG=1;}
	if(value<-128){PV_FLAG=1;}
	//bit3からbit4に桁上がりがある場合
	if((byte2&8)==8 && (byte&16)==16){H_FLAG=1;}else{H_FLAG=0;}
//	printf("byte=%x byte2=%x\n",byte,byte2);
//	printf("byte=%x byte2=%x\n",byte&16,byte2&8);
}
/**
1の数を数えて偶数なら1、奇数なら0を返す。
*/
char ParityCheck(unsigned char byte)
{
	unsigned char pp;
	pp=((byte>>4) & 0xf) ^ (byte & 0xf);
	pp=((pp>>2) & 0x3) ^ (pp & 0x3);
	return (((pp>>1) & 0x1) ^ (pp & 0x1))^1;
}
/**
論理演算時のフラグ設定
byte=演算前の値
*/
void SetFlagANDORXOR(unsigned char byte)
{
	//unsigned char P_Count=0,i;
	if(byte==0){Z_FLAG=1;}else{Z_FLAG=0;}
	//if(byte & 0x80){S_FLAG=1;}else{S_FLAG=0;}
	S_FLAG=(byte >>7);
	H_FLAG=0;
	PV_FLAG=0;
	//論理演算の場合のみ1をカウントする
	/*
	for(i=0;i<8;i++){
		if(byte & 1){P_Count++;}
		byte=(byte>>1);
	}
	//if(P_Count & 1){PV_FLAG=0;}else{PV_FLAG=1;}
	PV_FLAG=1-(P_Count & 1);
	*/
	
	PV_FLAG=ParityCheck(byte);
}

/**
ED code
*/
char ED_CodeAnalysis(unsigned char code,char ixiyflag,unsigned short hl_reg,unsigned char h_reg,unsigned char l_reg)
{
	unsigned char mem1,mem2,mem3,mem4;//コードの次のデータ
	unsigned short word1,word2,word3;//
	unsigned short adr1,adr2;//PUSH POP アドレス用
	unsigned short pushpair;//PUSH POP 一時的値代入
	unsigned char up2=code & 0xc0;//上位2ビット
	unsigned char up5=code & 0xf8;//上位5ビット
	unsigned char down3=code & 0x07;//下位3ビット
	unsigned char down4=code & 0x0f;//下位4ビット
	unsigned char DDD=(code>>3) & 0x7;//中3ビット
	unsigned char CCC=(code>>3) & 0x7;//中3ビット
	unsigned char SSS=code & 0x7;//下3ビット
	unsigned char RP=(code>>4)&3;//中2ビット
	short calcvalue;//加算減算結果
	int calcvalue32;//加算減算結果
	//
	#if DEBUG_VIEW
	printf("DDD=%x SSS=%x RP=%x\n",DDD,SSS,RP);
	printf("up2=%x up5=%x \n",up2,up5);
	printf("down3=%x down4=%x \n",down3,down4);
	#endif
	Cycle=0;
	switch(code){
		case 0x6f://RLD
			mem1=A_REG & 0xf0;
			mem2=A_REG & 0x0f;
			mem3=Memory(HL_REG) & 0xf0;
			mem4=Memory(HL_REG) & 0x0f;
			A_REG=mem1 | (mem3>>4);
			Memory(HL_REG)=mem2 | (mem4<<4);
				H_FLAG=N_FLAG=0;
				S_FLAG=(A_REG>>7)&1;
				Z_FLAG=(A_REG==0);
				PV_FLAG=ParityCheck(A_REG);
			DEBUG_CODE("DEBUG RLD\n");
		break;
		case 0x67://RRD
			mem1=A_REG & 0xf0;
			mem2=A_REG & 0x0f;
			mem3=Memory(HL_REG) & 0xf0;
			mem4=Memory(HL_REG) & 0x0f;
			A_REG=mem1 | mem4;
			Memory(HL_REG)=(mem2<<4) | (mem3>>4);
				H_FLAG=N_FLAG=0;
				S_FLAG=(A_REG>>7)&1;
				Z_FLAG=(A_REG==0);
				PV_FLAG=ParityCheck(A_REG);
			DEBUG_CODE("DEBUG RLD\n");
		break;
	}
	return 0;
}

unsigned char BITMASK[8]={1,2,4,8,16,32,64,128};
unsigned char BITMASKNOT[8]={0xfe,0xfd,0xfb,0xf7,0xef,0xdf,0xbf,0x7f};
/**
CB CODE Executed
ixiyflag=次の1バイトを+dのパラメータにするか命令ごとの判断材料にする
*/
char CB_CodeAnalysis(unsigned char code,char ixiyflag,unsigned short hl_reg,unsigned char h_reg,unsigned char l_reg)
{
	unsigned char mem1,mem2,mem3;//コードの次のデータ
	unsigned char temp8;//一時使用
	unsigned short temp16;//一時使用
	unsigned short word1,word2,word3;//
	unsigned short adr1,adr2;//PUSH POP アドレス用
	unsigned short pushpair;//PUSH POP 一時的値代入
	short calcvalue;//加算減算結果
	int calcvalue32;//加算減算結果

	unsigned char up2=code & 0xc0;//上位2ビット
	unsigned char up5=code & 0xf8;//上位5ビット
	unsigned char down3=code & 0x07;//下位3ビット
	unsigned char down4=code & 0x0f;//下位4ビット
	unsigned char BBB=(code>>3) & 0x7;//中3ビット
	unsigned char SSS=code & 0x7;//下3ビット
	unsigned char RP=(code>>4)&3;//中2ビット

	#if DEBUG_VIEW
	printf("CB code \n");

	printf("BBB=%x SSS=%x RP=%x\n",BBB,SSS,RP);
	printf("up2=%x up5=%x \n",up2,up5);
	printf("down3=%x down4=%x \n",down3,down4);
	#endif

	switch(up2){
		case 0x00://RLC r
			switch(up5){
				case 0:
					if(SSS==6){
						if(ixiyflag==1){
							PC_REG++;mem2=Memory(PC_REG);
							hl_reg=hl_reg+mem2;
						}
						temp8=Memory(hl_reg);
						mem1=(temp8>>7) & 1;
						temp8=(temp8<<1) | mem1;
							C_FLAG=mem1;
							H_FLAG=N_FLAG=0;
							S_FLAG=(temp8>>7)&1;
							Z_FLAG=(temp8==0);
							PV_FLAG=ParityCheck(temp8);
						Memory(hl_reg)=temp8;
					}else{
						temp8=*SSS_PTR[SSS];
						mem1=(temp8>>7) & 1;
						temp8=(temp8<<1) | mem1;
							C_FLAG=mem1;
							H_FLAG=N_FLAG=0;
							S_FLAG=(temp8>>7)&1;
							Z_FLAG=(temp8==0);
							PV_FLAG=ParityCheck(temp8);
						*SSS_PTR[SSS]=temp8;
					}
					DEBUG_CODE("DEBUG RLC r\n");
				break;
				case 0x08://RRC r
					if(SSS==6){
						if(ixiyflag==1){
							PC_REG++;mem2=Memory(PC_REG);
							hl_reg=hl_reg+mem2;
						}
						temp8=Memory(hl_reg);
						mem1=temp8 & 1;
						temp8=(temp8>>1) | (mem1<<7);
							C_FLAG=mem1;
							H_FLAG=N_FLAG=0;
							S_FLAG=(temp8>>7)&1;
							Z_FLAG=(temp8==0);
							PV_FLAG=ParityCheck(temp8);
						Memory(hl_reg)=temp8;
					}else{
						temp8=*SSS_PTR[SSS];
						mem1=temp8 & 1;
						temp8=(temp8>>1) | (mem1<<7);
						C_FLAG=mem1;
						H_FLAG=N_FLAG=0;
						S_FLAG=(temp8>>7)&1;
						Z_FLAG=(temp8==0);
						PV_FLAG=ParityCheck(temp8);
						*SSS_PTR[SSS]=temp8;
					}
					DEBUG_CODE("DEBUG RRC r\n");

				break;
				case 0x10://RL r
					if(SSS==6){
						if(ixiyflag==1){
							PC_REG++;mem2=Memory(PC_REG);
							hl_reg=hl_reg+mem2;
						}
						temp8=Memory(hl_reg);
						mem1=(temp8>>7) & 1;
						temp8=(temp8<<1) | C_FLAG;
							C_FLAG=mem1;
							H_FLAG=N_FLAG=0;
							S_FLAG=(temp8>>7)&1;
							Z_FLAG=(temp8==0);
							PV_FLAG=ParityCheck(temp8);
						Memory(hl_reg)=temp8;
					}else{
						temp8=*SSS_PTR[SSS];
						mem1=(temp8>>7) & 1;
						temp8=(temp8<<1) | C_FLAG;
						C_FLAG=mem1;
						H_FLAG=N_FLAG=0;
						S_FLAG=(temp8>>7)&1;
						Z_FLAG=(temp8==0);
						PV_FLAG=ParityCheck(temp8);
						*SSS_PTR[SSS]=temp8;
					}
					DEBUG_CODE("DEBUG RL r\n");
				break;

				case 0x18://RR r
					if(SSS==6){
						if(ixiyflag==1){
							PC_REG++;mem2=Memory(PC_REG);
							hl_reg=hl_reg+mem2;
						}
						temp8=Memory(hl_reg);
						mem1=temp8 & 1;
						temp8=(temp8>>1) | (C_FLAG<<7);
							C_FLAG=mem1;
							H_FLAG=N_FLAG=0;
							S_FLAG=(temp8>>7)&1;
							Z_FLAG=(temp8==0);
							PV_FLAG=ParityCheck(temp8);
						Memory(hl_reg)=temp8;
					}else{
						temp8=*SSS_PTR[SSS];
						mem1=temp8 & 1;
						temp8=(temp8>>1) | (C_FLAG<<7);
						C_FLAG=mem1;
						H_FLAG=N_FLAG=0;
						S_FLAG=(temp8>>7)&1;
						Z_FLAG=(temp8==0);
						PV_FLAG=ParityCheck(temp8);
						*SSS_PTR[SSS]=temp8;
					}
					DEBUG_CODE("DEBUG RR r\n");
				break;
				case 0x20://SLA r
					if(SSS==6){
						if(ixiyflag==1){
							PC_REG++;mem2=Memory(PC_REG);
							hl_reg=hl_reg+mem2;
						}
						temp8=Memory(hl_reg);
							mem1=temp8 & 1;
							C_FLAG=(temp8>>7) & 1;
							temp8=(temp8<<1);
		//					C_FLAG=mem1;
							H_FLAG=N_FLAG=0;
							S_FLAG=(temp8>>7)&1;
							Z_FLAG=(temp8==0);
							PV_FLAG=ParityCheck(temp8);
						Memory(hl_reg)=temp8;
					}else{

					temp8=*SSS_PTR[SSS];
					mem1=temp8 & 1;
					C_FLAG=(temp8>>7) & 1;
					temp8=(temp8<<1);
					H_FLAG=N_FLAG=0;
					S_FLAG=(temp8>>7)&1;
					Z_FLAG=(temp8==0);
					PV_FLAG=ParityCheck(temp8);
					*SSS_PTR[SSS]=temp8;
					}
						DEBUG_CODE("DEBUG SLA r\n");
				break;
				case 0x28://SRA r
					if(SSS==6){
						if(ixiyflag==1){
							PC_REG++;mem2=Memory(PC_REG);
							hl_reg=hl_reg+mem2;
						}
						temp8=Memory(hl_reg);
						mem1=temp8 & 128;
						C_FLAG=temp8 & 1;
						temp8=(temp8>>1) | mem1;
							H_FLAG=N_FLAG=0;
							S_FLAG=(temp8>>7)&1;
							Z_FLAG=(temp8==0);
							PV_FLAG=ParityCheck(temp8);
						Memory(hl_reg)=temp8;
					}else{
						temp8=*SSS_PTR[SSS];
						mem1=temp8 & 128;
						C_FLAG=temp8 & 1;
						temp8=(temp8>>1) | mem1;
						H_FLAG=N_FLAG=0;
						S_FLAG=(temp8>>7)&1;
						Z_FLAG=(temp8==0);
						PV_FLAG=ParityCheck(temp8);
						*SSS_PTR[SSS]=temp8;
					}
						DEBUG_CODE("DEBUG SRA r\n");
				break;
				case 0x38://SRL r
					if(SSS==6){
						if(ixiyflag==1){
							PC_REG++;mem2=Memory(PC_REG);
							hl_reg=hl_reg+mem2;
						}
						temp8=Memory(hl_reg);
						C_FLAG=temp8 & 1;
						temp8=(temp8>>1);

		//					C_FLAG=mem1;
							H_FLAG=N_FLAG=0;
							S_FLAG=(temp8>>7)&1;
							Z_FLAG=(temp8==0);
							PV_FLAG=ParityCheck(temp8);
						Memory(hl_reg)=temp8;
					}else{

						temp8=*SSS_PTR[SSS];
						C_FLAG=temp8 & 1;
						temp8=(temp8>>1);
						H_FLAG=N_FLAG=0;
						S_FLAG=(temp8>>7)&1;
						Z_FLAG=(temp8==0);
						PV_FLAG=ParityCheck(temp8);
						*SSS_PTR[SSS]=temp8;
					}
					DEBUG_CODE("DEBUG SRL r\n");
				break;


				
				
				
			}
		break;
		case 0x40:
			if(SSS==6){
				//BIT b,(HL)
			//	printf("ixiyflag=%d \n",ixiyflag);
				if(ixiyflag==1){
					PC_REG++;mem2=Memory(PC_REG);
					hl_reg=hl_reg+mem2;
				}

				if((BITMASK[BBB] & Memory(hl_reg))!=0){
					Z_FLAG=1;PV_FLAG=1;
				}else{
					Z_FLAG=0;PV_FLAG=0;
				}
				H_FLAG=1;N_FLAG=0;
				DEBUG_CODE("DEBUG BIT b,(HL)\n");
			}else{
				//BIT b,r
				if((BITMASK[BBB] & *SSS_PTR[SSS])!=0){
					Z_FLAG=1;PV_FLAG=1;
				}else{
					Z_FLAG=0;PV_FLAG=0;
				}
				H_FLAG=1;N_FLAG=0;
				DEBUG_CODE("DEBUG BIT b,r\n");
			}
		break;
		case 0x80:
			if(SSS==6){
				//RES b,(HL)
			//	printf("ixiyflag=%d \n",ixiyflag);
				if(ixiyflag==1){
					PC_REG++;mem2=Memory(PC_REG);
					hl_reg=hl_reg+mem2;
				}

//				printf("HL=%d MEM(HL)=%d\n",HL_REG,Memory(HL_REG));
//				printf("MASK=%d\n",BITMASK[BBB]);
				Memory(hl_reg)=Memory(hl_reg) & BITMASKNOT[BBB];
				DEBUG_CODE("DEBUG RES b,(HL)\n");
			}else{
				//RES b,r
				*SSS_PTR[SSS]=*SSS_PTR[SSS] & BITMASKNOT[BBB];
				DEBUG_CODE("DEBUG RES b,r\n");
			}
		break;
		case 0xc0:
			if(SSS==6){
				//SET b,(HL)
				printf("ixiyflag=%d \n",ixiyflag);
				if(ixiyflag==1){
					PC_REG++;mem2=Memory(PC_REG);
					hl_reg=hl_reg+mem2;
				}

			//	printf("HL=%d MEM(HL)=%d\n",hl_reg,Memory(hl_reg));
			//	printf("MASK=%d\n",BITMASK[BBB]);
				Memory(hl_reg)=Memory(hl_reg) | BITMASK[BBB];
				DEBUG_CODE("DEBUG SET b,(HL)\n");
			}else{
				//SET b,r
				*SSS_PTR[SSS]=*SSS_PTR[SSS] | BITMASK[BBB];
			//	printf("MASK=%d\n",BITMASK[BBB]);
				DEBUG_CODE("DEBUG SET b,r\n");
			}
		break;
	}
	return 0;
}
/**
１命令を実行する。
正常実行であれば０を返す、異常であれば１を返す。
code=実行する命令コード
ixiyflag=次の1バイトを+dのパラメータにするか命令ごとの判断材料にする
*/
char CodeAnalysis(unsigned char code,char ixiyflag,unsigned short hl_reg,unsigned char h_reg,unsigned char l_reg)
{
	unsigned char mem1,mem2,mem3;//コードの次のデータ
	unsigned char temp8;//一時的利用
	unsigned short temp16;//一時的利用
	unsigned short word1,word2,word3;//
	unsigned short adr1,adr2;//PUSH POP アドレス用
	unsigned short pushpair;//PUSH POP 一時的値代入
	short calcvalue;//加算減算結果
	int calcvalue32;//加算減算結果
	unsigned char* pmem;//メモリポインタ用
	//
	unsigned char NextCode;
	char Result;

	Cycle=0;

	//優先度高い命令を解析
	switch(code){
		case 0://NOP
			Cycle=4;
			DEBUG_CODE("DEBUG NOP\n");
			return 0;
		break;
			case 0x76://HALT(エミュレータ停止）
			DEBUG_CODE("DEBUG HALT\n");
			return 1;
		break;
		case 0xdd://IX operation
			//NextCodeIX=1;
			//コード読み込み
			PC_REG++;NextCode=Memory(PC_REG);
			//PC_REG++;nstep=Memory(PC_REG);
			ixiyflag=1;//IXIY FLAG
			//コード解析
			#if DEBUG_VIEW
				printf("IX=%0x IY=%0x\n",IX_REG,IY_REG);
			#endif
			Result=CodeAnalysis(NextCode,ixiyflag,IX_REG,IXH_REG,IYH_REG);
			return Result;
		break;
		case 0xfd://IY operation
			//NextCodeIY=1;
			//コード読み込み
			PC_REG++;NextCode=Memory(PC_REG);
			//PC_REG++;nstep=Memory(PC_REG);
			ixiyflag=1;//IXIY FLAG

			//コード解析
			#if DEBUG_VIEW
				printf("IX=%0x IY=%0x\n",IX_REG,IY_REG);
			#endif
			Result=CodeAnalysis(NextCode,ixiyflag,IY_REG,IYH_REG,IYL_REG);
			return Result;
		break;
		case 0xed://
			//コード読み込み
			PC_REG++;NextCode=Memory(PC_REG);
			#if DEBUG_VIEW
				printf("ED code=%0x\n",NextCode);
			#endif
			Result=ED_CodeAnalysis(NextCode,ixiyflag,hl_reg,h_reg,l_reg);
			return Result;
		break;
		case 0xcb://
			//コード読み込み
			PC_REG++;NextCode=Memory(PC_REG);
			Result=CB_CodeAnalysis(NextCode,ixiyflag,hl_reg,h_reg,l_reg);
			return Result;
		break;
	}
	
	

	#if DEBUG_VIEW
	printf("code analysis start!\n");
	#endif
	unsigned char Executed=1;//未実行=0
	//特殊命令の実行
	switch(code){
		case 0x36://LD (HL),n
			PC_REG++;mem1=Memory(PC_REG);
			printf("ixiyflag=%d \n",ixiyflag);
			if(ixiyflag==1){
				hl_reg=hl_reg+(CHAR8TO16(mem1));
			//	printf("d=%0d\n",mem1);
				PC_REG++;mem1=Memory(PC_REG);
			}
			//printf("HL=%0x mem1=%0d\n",hl_reg,mem1);
			Memory(hl_reg)=mem1;
	//	printf("HL Address=%lx *HL=%d \n",(long int)hl_reg,*hl_reg);
	//		Memory(*hl_reg + nstep)=mem1;
	//		Memory(hl_reg)=mem1;
			DEBUG_CODE("DEBUG LD (HL),n\n");
		break;
		case 0x0a://LD A,(BC)
			A_REG=Memory(BC_REG);
			DEBUG_CODE("DEBUG LD A,(BC)\n");
		break;
		case 0x1a://LD A,(DE)
			A_REG=Memory(DE_REG);
			DEBUG_CODE("DEBUG LD A,(DE)\n");
		break;
		case 0x3a://LD A,(nn)
			PC_REG++;mem1=Memory(PC_REG);
			PC_REG++;mem2=Memory(PC_REG);
			pushpair=mem1 | (mem2<<8);
			A_REG=Memory(pushpair);
			DEBUG_CODE("DEBUG LD A,(nn)\n");
		break;
		case 0x02://LD (BC),A
			Memory(BC_REG)=A_REG;
			DEBUG_CODE("DEBUG LD (BC),A\n");
		break;
		case 0x12://LD (DE),A
			Memory(DE_REG)=A_REG;
			DEBUG_CODE("DEBUG LD (DE),A\n");
		break;
		case 0x32://LD (nn),A
			PC_REG++;mem1=Memory(PC_REG);
			PC_REG++;mem2=Memory(PC_REG);
			pushpair=mem1 | (mem2<<8);
			Memory(pushpair)=A_REG;
			DEBUG_CODE("DEBUG LD (nn),A\n");
		break;
		case 0x2a://LD HL,(nn)
			PC_REG++;mem1=Memory(PC_REG);
			PC_REG++;mem2=Memory(PC_REG);
			pushpair=mem1 | (mem2<<8);
			L_REG=Memory(pushpair);pushpair++;
			H_REG=Memory(pushpair);
			DEBUG_CODE("DEBUG LD HL,(nn)\n");
		break;
		case 0x22://LD (nn),HL
			PC_REG++;mem1=Memory(PC_REG);
			PC_REG++;mem2=Memory(PC_REG);
			pushpair=mem1 | (mem2<<8);
			Memory(pushpair)=L_REG;pushpair++;
			Memory(pushpair)=H_REG;
			DEBUG_CODE("DEBUG LD (nn),HL\n");
		break;
		case 0xf9://LD SP,HL
			SP_REG=HL_REG;
			DEBUG_CODE("DEBUG LD SP,HL\n");
		break;
		case 0xeb://EX DE,HL
			pushpair=HL_REG;
			HL_REG=DE_REG;
			DE_REG=pushpair;
			DEBUG_CODE("DEBUG EX DE,HL\n");
		break;
		case 0x08://EX AF,AF'
			pushpair=AF2_REG;
			AF2_REG=AF_REG;
			AF_REG=pushpair;
			DEBUG_CODE("DEBUG EX AF,AF'\n");
		break;
		case 0xd9://EXX
			pushpair=HL2_REG;HL2_REG=HL_REG;HL_REG=pushpair;
			pushpair=DE2_REG;DE2_REG=DE_REG;DE_REG=pushpair;
			pushpair=BC2_REG;BC2_REG=BC_REG;BC_REG=pushpair;
			DEBUG_CODE("DEBUG EXX\n");
		break;
		case 0xe3://EX (SP),HL
			mem1=Memory(SP_REG);
			mem2=Memory((SP_REG+1)& MAX_MEMORY_SIZE);
			pushpair=mem1 | (mem2<<8);
			Memory(SP_REG)=HL_REG & 0xff;
			Memory((SP_REG+1)&0xffff)=(HL_REG>>8) & 0xff;
			HL_REG=pushpair;
			DEBUG_CODE("DEBUG EX (SP),HL\n");
		break;
		case 0xf5://PUSH AF
			SP_REG=(SP_REG-2)& MAX_MEMORY_SIZE;
			adr1=SP_REG;
			adr2=(SP_REG+1)& MAX_MEMORY_SIZE;
			Memory(adr1)=F_REG;//(AF_REG)&255;
			Memory(adr2)=A_REG;//((AF_REG)>>8)&255;
			DEBUG_CODE("DEBUG PUSH AF\n");

		break;
		case 0xf1://POP AF
			adr1=SP_REG;
			adr2=(SP_REG+1)&0xffff;
			mem1=Memory(adr1);
			mem2=Memory(adr2);
			AF_REG=mem1 | (mem2<<8);
			SP_REG=(SP_REG+2)& MAX_MEMORY_SIZE;
			DEBUG_CODE("DEBUG POP AF\n");

		break;
		case 0x07://RLCA
			mem1=(A_REG>>7)&1;
			A_REG=(A_REG<<1) | mem1;
			C_FLAG=mem1;
			H_FLAG=0;N_FLAG=0;
			DEBUG_CODE("DEBUG RLCA\n");
		break;
		case 0x17://RLA
			mem1=C_FLAG;
			C_FLAG=(A_REG>>7)&1;
			A_REG=(A_REG<<1) | mem1;
			H_FLAG=0;N_FLAG=0;
			DEBUG_CODE("DEBUG RLA\n");
		break;
		case 0x0f://RRCA
			mem1=A_REG&1;
			C_FLAG=mem1;
			A_REG=(A_REG>>1) | (mem1<<7);
			H_FLAG=0;N_FLAG=0;
			DEBUG_CODE("DEBUG RRCA\n");
		break;
		case 0x1f://RRA
			mem1=C_FLAG;
			C_FLAG=A_REG & 1;
			A_REG=(A_REG>>1) | (mem1<<7);
			H_FLAG=0;N_FLAG=0;
			DEBUG_CODE("DEBUG RRA\n");

		break;
		case 0xce://ADC A,n
				mem1=A_REG;
				PC_REG++;mem2=Memory(PC_REG);
				calcvalue=(short)A_REG+mem2+C_FLAG;
				A_REG=calcvalue & 0xff;
				SetFlagADD(calcvalue,mem1);
				DEBUG_CODE("DEBUG ADC A,r\n");
		break;
		case 0x86://ADD A,(HL)
			mem1=A_REG;
			if(ixiyflag==1){
				PC_REG++;mem2=Memory(PC_REG);
				hl_reg=hl_reg+(CHAR8TO16(mem2));
			}

			calcvalue=(short)A_REG+Memory(hl_reg);
			A_REG=calcvalue & 0xff;
			SetFlagADD(calcvalue,mem1);
			DEBUG_CODE("DEBUG ADD A,(HL)\n");
		break;
		case 0x8e://ADC A,(HL)
			mem1=A_REG;
			if(ixiyflag==1){
				PC_REG++;mem2=Memory(PC_REG);
				hl_reg=hl_reg+(CHAR8TO16(mem2));
			}

			calcvalue=(short)A_REG+Memory(hl_reg)+C_FLAG;
			A_REG=calcvalue & 0xff;
			SetFlagADD(calcvalue,mem1);
			DEBUG_CODE("DEBUG ADC A,(HL)\n");
		break;
		case 0x34://INC (HL)
			if(ixiyflag==1){
				PC_REG++;mem2=Memory(PC_REG);
				hl_reg=hl_reg+(CHAR8TO16(mem2));
			}
			mem1=Memory(hl_reg);
			calcvalue=mem1+1;
//		printf("mem1=%x calcvalue=%d\n",mem1,calcvalue);
			Memory(hl_reg)=calcvalue & 0xff;
			SetFlagINCDEC(calcvalue,mem1,0);
			DEBUG_CODE("DEBUG INC (HL)\n");

		break;
		case 0xd6://SUB n
				mem1=A_REG;
				PC_REG++;mem2=Memory(PC_REG);
				calcvalue=(short)A_REG-mem2;
		//printf("calcvalue=%x mem1=%x \n",calcvalue,mem1);
				A_REG=calcvalue & 0xff;
				SetFlagSUB(calcvalue,mem1);
				DEBUG_CODE("DEBUG SUB n\n");
		break;
		case 0xde://SBC n
				mem1=A_REG;
				PC_REG++;mem2=Memory(PC_REG);
				calcvalue=(short)A_REG-mem2-C_FLAG;
//		printf("calcvalue=%x mem1=%x \n",calcvalue,mem1);
				A_REG=calcvalue & 0xff;
				SetFlagSUB(calcvalue,mem1);
				DEBUG_CODE("DEBUG SBC A,n\n");
		break;

		case 0x96://SUB (HL)
			if(ixiyflag==1){
				PC_REG++;mem2=Memory(PC_REG);
				hl_reg=hl_reg+(CHAR8TO16(mem2));
			}
			mem1=Memory(hl_reg);
			calcvalue=A_REG-mem1;
//		printf("mem1=%x calcvalue=%d\n",mem1,calcvalue);
			A_REG=calcvalue & 0xff;
			SetFlagSUB(calcvalue,mem1);
			DEBUG_CODE("DEBUG SUB (HL)\n");
		break;
		case 0x9E://SBC A,(HL)
			if(ixiyflag==1){
				PC_REG++;mem2=Memory(PC_REG);
				hl_reg=hl_reg+(CHAR8TO16(mem2));
			}
			mem1=Memory(hl_reg);
			calcvalue=A_REG-mem1-C_FLAG;
//		printf("mem1=%x calcvalue=%d\n",mem1,calcvalue);
			A_REG=calcvalue & 0xff;
			SetFlagSUB(calcvalue,mem1);
			DEBUG_CODE("DEBUG SBC A,(HL)\n");
		break;
		case 0x35://DEC (HL)
			if(ixiyflag==1){
				PC_REG++;mem2=Memory(PC_REG);
				hl_reg=hl_reg+(CHAR8TO16(mem2));
			}
			mem1=Memory(hl_reg);
			calcvalue=mem1-1;
//		printf("mem1=%x calcvalue=%d\n",mem1,calcvalue);
			Memory(hl_reg)=calcvalue & 0xff;
			SetFlagINCDEC(calcvalue,mem1,1);
			DEBUG_CODE("DEBUG DEC (HL)\n");

		break;
		case 0xe6://AND n
			PC_REG++;mem2=Memory(PC_REG);
			A_REG=A_REG & (mem2);
			SetFlagANDORXOR(A_REG);
			C_FLAG=N_FLAG=0;
			DEBUG_CODE("DEBUG AND n\n");
		break;
		case 0xa6://AND (HL)
			if(ixiyflag==1){
				PC_REG++;mem2=Memory(PC_REG);
				hl_reg=hl_reg+(CHAR8TO16(mem2));
			}
			mem2=Memory(hl_reg);
			A_REG=A_REG & (mem2);
			SetFlagANDORXOR(A_REG);
			C_FLAG=N_FLAG=0;
			DEBUG_CODE("DEBUG AND (HL)\n");
		break;
		case 0xf6://OR n
			PC_REG++;mem2=Memory(PC_REG);
			A_REG=A_REG | (mem2);
			SetFlagANDORXOR(A_REG);
			C_FLAG=N_FLAG=0;
			DEBUG_CODE("DEBUG OR n\n");
		break;
		case 0xb6://OR (HL)
			if(ixiyflag==1){
				PC_REG++;mem2=Memory(PC_REG);
				hl_reg=hl_reg+(CHAR8TO16(mem2));
			}
			mem2=Memory(hl_reg);
			A_REG=A_REG | (mem2);
			SetFlagANDORXOR(A_REG);
			C_FLAG=N_FLAG=0;
			DEBUG_CODE("DEBUG OR (HL)\n");
		break;
		case 0xee://XOR n
			PC_REG++;mem2=Memory(PC_REG);
			A_REG=A_REG ^ (mem2);
			SetFlagANDORXOR(A_REG);
			C_FLAG=N_FLAG=0;
			DEBUG_CODE("DEBUG XOR n\n");
		break;
		case 0xae://XOR (HL)
			if(ixiyflag==1){
				PC_REG++;mem2=Memory(PC_REG);
				hl_reg=hl_reg+(CHAR8TO16(mem2));
			}
			mem2=Memory(hl_reg);
			A_REG=A_REG ^ (mem2);
			SetFlagANDORXOR(A_REG);
			C_FLAG=N_FLAG=0;
			DEBUG_CODE("DEBUG XOR n\n");
		break;
		case 0x2f://CPL
			A_REG=A_REG ^ 0xff;
			H_FLAG=N_FLAG=1;
			DEBUG_CODE("DEBUG CPL\n");
		break;
		case 0xfe://CP n
			mem1=A_REG;
			PC_REG++;mem2=Memory(PC_REG);
			calcvalue=(short)(A_REG-mem2);
			SetFlagSUB(calcvalue,mem1);
			DEBUG_CODE("DEBUG CP n\n");
		break;
		case 0xbe://CP (HL)
			if(ixiyflag==1){
				PC_REG++;mem2=Memory(PC_REG);
				hl_reg=hl_reg+(CHAR8TO16(mem2));
			}
			mem2=Memory(hl_reg);
			mem1=A_REG;
			calcvalue=(short)A_REG-mem2;
			SetFlagSUB(calcvalue,mem1);
			DEBUG_CODE("DEBUG CP (HL)\n");
		break;
		case 0x3f://CCF
			H_FLAG=C_FLAG;
			N_FLAG=0;
			C_FLAG=(C_FLAG+1)&1;
			DEBUG_CODE("DEBUG CCF\n");
		break;
		case 0x37://SCF
			N_FLAG=H_FLAG=0;
			C_FLAG=1;
			DEBUG_CODE("DEBUG SCF\n");
		break;
		case 0xc3://JP nn
			mem1=Memory((PC_REG+1)& 0xffff);
			mem2=Memory((PC_REG+2)& 0xffff);
			PC_REG=(mem1 | (mem2<<8))-1;
			DEBUG_CODE("JP nn\n");
		break;
		case 0x18://JR e
			PC_REG++;
			mem1=Memory(PC_REG & 0xffff);
			if(mem1>127){
				pushpair=(-(256-mem1))&0xffff;
			}else{
				pushpair=mem1;
			}
			PC_REG=(PC_REG+pushpair)& 0xffff;
		#if DEBUG_VIEW
		printf("JR e:pushpair=%x \n",pushpair);
		#endif
			DEBUG_CODE("JR e\n");
		break;
		case 0x38://JR C,e
			PC_REG++;
			mem1=Memory(PC_REG);
			if(C_FLAG==1){
				if(mem1>127){
					pushpair=(-(256-mem1))&0xffff;
				}else{
					pushpair=mem1;
				}
				PC_REG=(PC_REG+pushpair)& 0xffff;
			}
			DEBUG_CODE("JR C,e\n");
		break;
		case 0x30://JR NC,e
			PC_REG++;
			mem1=Memory(PC_REG);
			if(C_FLAG==0){
				if(mem1>127){
					pushpair=(-(256-mem1))&0xffff;
				}else{
					pushpair=mem1;
				}
				PC_REG=(PC_REG+pushpair)& 0xffff;
			}
			DEBUG_CODE("JR NC,e\n");
		break;
		case 0x28://JR Z,e
			PC_REG++;
			mem1=Memory(PC_REG);
			if(Z_FLAG==1){
				if(mem1>127){
					pushpair=(-(256-mem1))&0xffff;
				}else{
					pushpair=mem1;
				}
				PC_REG=(PC_REG+pushpair)& 0xffff;
			}
			DEBUG_CODE("JR Z,e\n");
		break;
		case 0x20://JR NZ,e
			PC_REG++;
			mem1=Memory(PC_REG);
			if(Z_FLAG==0){
				if(mem1>127){
					pushpair=(-(256-mem1))&0xffff;
				}else{
					pushpair=mem1;
				}
				PC_REG=(PC_REG+pushpair)& 0xffff;
			}
			DEBUG_CODE("JR NZ,e\n");

		break;
		case 0xe9://JP (HL)
			mem1=Memory(HL_REG);
			mem2=Memory((HL_REG+1)& 0xffff);
			PC_REG=(mem1 | (mem2<<8))-1;
			DEBUG_CODE("JP (HL)\n");

		break;
		case 0x10://DJNZ
			PC_REG++;
			mem1=Memory(PC_REG);
			B_REG=(B_REG-1)& 0xff;
			if(B_REG>0){
				if(mem1>127){
					pushpair=(-(256-mem1))&0xffff;
				}else{
					pushpair=mem1;
				}
				PC_REG=(PC_REG+pushpair)& 0xffff;
			}
		printf("JR e:pushpair=%x \n",pushpair);
		break;
		case 0xcd://CALL nn
			pushpair=(PC_REG+2)&0xffff;
			Memory((SP_REG-1)& 0xffff)=(pushpair>>8)&0xff;
			Memory((SP_REG-2)& 0xffff)=(pushpair)&0xff;
			SP_REG=SP_REG-2;
			mem1=Memory((PC_REG+1)& 0xffff);
			mem2=Memory((PC_REG+2)& 0xffff);
			PC_REG=(mem1 | (mem2<<8))-1;
			DEBUG_CODE("CALL nn\n");
		break;
		case 0xc9://RET
			mem1=Memory((SP_REG)& 0xffff);
			mem2=Memory((SP_REG+1)& 0xffff);
			SP_REG=SP_REG+2;
			PC_REG=(mem1 | (mem2<<8))-1;
			DEBUG_CODE("RET\n");
		break;
		case 0xf3://DI
			IFF_REG=0;Executed=1;
			IFF1_REG=IFF2_REG=0;
			DEBUG_CODE("DEBUG DI\n");
			return 0;
		break;
		case 0xfb://EI
			IFF_REG=1;Executed=1;
			IFF1_REG=IFF2_REG=1;
			DEBUG_CODE("DEBUG EI\n");
			return 0;
		break;
		case 0xdb://IN A,(n)
			PC_REG++;
			mem2=Memory(PC_REG);
			A_REG=IOMemory[mem2];
			DEBUG_CODE("DEBUG IN A,(n)\n");
		break;
		case 0xd3://OUT (n),A
			PC_REG++;
			mem2=Memory(PC_REG);
			IOMemory[mem2]=A_REG;
			DEBUG_CODE("DEBUG OUT (n),A\n");
		break;
		case 0x27://DAA
/*
			mem2=(A_REG>>4) & 0x0f;
			mem1=A_REG & 0x0f;
			if(N_FLAG==1){
				if(H_FLAG==1){
					mem1-=6;
				}
				if(mem2 >0x9){
					mem2=mem2 - 6;
					C_FLAG=1;
				}
				H_FLAG=0;
			}else{
				if(mem1>9){
					mem1=mem1-10;
					mem2++;
					H_FLAG=1;
				}
				if(mem2 >0x9){
					mem2=mem2 - 0xa;
					C_FLAG=1;
				}
			}
			A_REG=(mem2<<4) | mem1;
		*/
			temp8=A_REG;
			if (N_FLAG){
				if ((H_FLAG==1) | ((A_REG&0xf)>9)) temp8-=6;
				if ((C_FLAG==1) | (A_REG>0x99)) temp8-=0x60;
			}else{
				if ((H_FLAG==1) | ((A_REG&0xf)>9)) temp8+=6;
				if ((C_FLAG==1) | (A_REG>0x99)) temp8+=0x60;
			}
			H_FLAG=((A_REG ^ temp8)& 0x10)>>4;
			if(A_REG>0x99){C_FLAG=1;}
			if(temp8==0){Z_FLAG=1;}else{Z_FLAG=0;}
			S_FLAG=(temp8 >>7)&1;
			A_REG = temp8;
			PV_FLAG=ParityCheck(A_REG);
			DEBUG_CODE("DEBUG DAA\n");
		break;
		case 0xC6://ADD A,n
			temp8=A_REG;
			PC_REG++;
			mem2=Memory(PC_REG);
			calcvalue=(short)A_REG+mem2;
			A_REG=calcvalue & 0xff;
			SetFlagADD(calcvalue,temp8);
			DEBUG_CODE("DEBUG ADD A,n\n");
		break;
		default:
		#if DEBUG_VIEW
		DEBUG_CODE("DEBUG 1code not used\n");
		#endif
			Executed=0;
		break;
	}
	#if DEBUG_VIEW
	if(Executed==1){
		DEBUG_CODE("code executed\n");
	}
	#endif

	unsigned char up2=code & 0xc0;//上位2ビット
	unsigned char up5=code & 0xf8;//上位5ビット
	//unsigned char down3=code & 0x07;//下位3ビット
	unsigned char down4=code & 0x0f;//下位4ビット
	unsigned char DDD=(code>>3) & 0x7;//中3ビット
	//unsigned char CCC=(code>>3) & 0x7;//中3ビット
	unsigned char SSS=code & 0x7;//下3ビット
	unsigned char RP=(code>>4)&3;//中2ビット

	#if DEBUG_VIEW
	printf("DDD=%x SSS=%x RP=%x\n",DDD,SSS,RP);
	printf("up2=%x up5=%x \n",up2,up5);
	printf("down3=%x down4=%x \n",SSS,down4);
	#endif

	//その他の命令の合理化
	if(Executed==0){
//		printf("DEBUG up5=%x \n",up5);
		Executed=1;
		switch(up5){
			case 0x70://LD (HL),r
				if(ixiyflag==1){
					PC_REG++;mem2=Memory(PC_REG);
					hl_reg=hl_reg+(CHAR8TO16(mem2));
				}
				mem1=*SSS_PTR[SSS];
				Memory(hl_reg)=mem1;
				DEBUG_CODE("DEBUG LD (HL),r\n");
				break;
			break;
			case 0x80://ADD A,r
				mem1=A_REG;
				calcvalue=(short)A_REG+(*SSS_PTR[SSS]);
				A_REG=calcvalue & 0xff;
				SetFlagADD(calcvalue,mem1);
				DEBUG_CODE("DEBUG ADD A,r\n");
			break;
			case 0x88://ADC A,r
				mem1=A_REG;
				calcvalue=(short)A_REG+(*SSS_PTR[SSS])+C_FLAG;
				A_REG=calcvalue & 0xff;
				SetFlagADD(calcvalue,mem1);
				DEBUG_CODE("DEBUG ADD A,r\n");
			break;
			case 0x90://SUB r
				mem1=A_REG;
				calcvalue=(short)A_REG-(*SSS_PTR[SSS]);
				A_REG=calcvalue & 0xff;
				SetFlagSUB(calcvalue,mem1);
				DEBUG_CODE("DEBUG SUB r\n");
			break;
			case 0x98://SBC A,r
				mem1=A_REG;
				calcvalue=(short)A_REG-(*SSS_PTR[SSS])-C_FLAG;
				A_REG=calcvalue & 0xff;
				SetFlagSUB(calcvalue,mem1);
				DEBUG_CODE("DEBUG SBC A,r\n");
			break;
			case 0xa0://AND r
				A_REG=A_REG & (*SSS_PTR[SSS]);
				SetFlagANDORXOR(A_REG);
				C_FLAG=N_FLAG=0;
				DEBUG_CODE("DEBUG AND r\n");
			break;
			case 0xb0://OR r
				A_REG=A_REG | (*SSS_PTR[SSS]);
				SetFlagANDORXOR(A_REG);
				C_FLAG=N_FLAG=0;
				DEBUG_CODE("DEBUG OR r\n");
				return 0;
			break;
			case 0xa8://XOR r
				A_REG=A_REG ^ (*SSS_PTR[SSS]);
				SetFlagANDORXOR(A_REG);
				C_FLAG=N_FLAG=0;
				DEBUG_CODE("DEBUG XOR r\n");
			break;
			case 0xb8://CP r
				mem1=A_REG;
				calcvalue=(short)A_REG-(*SSS_PTR[SSS]);
				SetFlagSUB(calcvalue,mem1);
				DEBUG_CODE("DEBUG CP r\n");
			break;
			default:
				Executed=0;
			break;
		}
	}
	if(Executed==1){
		return 0;
	}else{
//		printf("DEBUG up2=%x down3=%x down4=%x\n",up2,SSS,down4);
		Executed=1;
		switch(up2){
			case 0x40:
				if(SSS==6){
					//LD r,(HL)
					if(ixiyflag==1){
						PC_REG++;mem2=Memory(PC_REG);
						hl_reg=hl_reg+(CHAR8TO16(mem2));
					}
					*DDD_PTR[DDD]=Memory(hl_reg);
					DEBUG_CODE("DEBUG LD r,(HL)\n");
				}else{
					//LD r1,r2
					*DDD_PTR[DDD]=*SSS_PTR[SSS];
					DEBUG_CODE("DEBUG LD r1,r2\n");
				}
			break;
			case 0x00:
				switch(down4){
					case 0x1://LD rp,nn
						PC_REG++;
						mem1=Memory(PC_REG);
						PC_REG++;
						mem2=Memory(PC_REG);
						*RP_PTR[RP]=mem1 | (mem2<<8);
						DEBUG_CODE("DEBUG LD rp,nn\n");

					break;
					case 0x9://ADD HL,rp
						word1=HL_REG;
						calcvalue32=HL_REG + *RP_PTR[RP];
						HL_REG=(unsigned short)calcvalue32;
						SetFlagADDSUB16(calcvalue32,word1,0);
						DEBUG_CODE("DEBUG ADD HL,rp\n");
					break;
					case 0x3://INC rp
						*RP_PTR[RP]=*RP_PTR[RP]+1;
						DEBUG_CODE("DEBUG INC rp\n");
					break;
					case 0xb://DEC rp
						*RP_PTR[RP]=*RP_PTR[RP]-1;
						DEBUG_CODE("DEBUG DEC rp\n");
					break;
				}
				switch(SSS){
					case 0x6://LD r,n
						PC_REG++;
						mem1=Memory(PC_REG);
						*DDD_PTR[DDD]=mem1;
						DEBUG_CODE("DEBUG LD r,n\n");
					break;
					case 0x4://INC r
						mem1=*DDD_PTR[DDD];
						calcvalue=mem1+1;
//					printf("mem1=%x calcvalue=%d\n",mem1,calcvalue);
						*DDD_PTR[DDD]=calcvalue & 0xff;
						SetFlagINCDEC(calcvalue,mem1,0);
						DEBUG_CODE("DEBUG INC r\n");
					break;
					case 0x5://DEC r
						mem1=*DDD_PTR[DDD];
						calcvalue=mem1-1;
//					printf("mem1=%x calcvalue=%d\n",mem1,calcvalue);
						*DDD_PTR[DDD]=calcvalue & 0xff;
						SetFlagINCDEC(calcvalue,mem1,1);
						DEBUG_CODE("DEBUG DEC r\n");
					break;
				}
			break;
			case 0xc0:
				switch(down4){
					case 0x5://PUSH rp
						pushpair=*RP_PTR[RP];
						SP_REG=(SP_REG-2)& MAX_MEMORY_SIZE;
						adr1=SP_REG;
						adr2=(SP_REG+1)& MAX_MEMORY_SIZE;
						Memory(adr1)=pushpair&255;
						Memory(adr2)=(pushpair>>8)&255;
						DEBUG_CODE("DEBUG PUSH rp\n");
					break;
					case 0x1://POP rp
						adr1=SP_REG;
						adr2=(SP_REG+1)& MAX_MEMORY_SIZE;
						mem1=Memory(adr1);
						mem2=Memory(adr2);
						*RP_PTR[RP]=mem1 | (mem2<<8);
						SP_REG=(SP_REG+2)& MAX_MEMORY_SIZE;
						DEBUG_CODE("DEBUG POP rp\n");
					break;
				}
				switch(SSS){
					case 0x2://JP cc,nn
//						mem1=Memory((PC_REG+1)& 0xffff);
//						mem2=Memory((PC_REG+2)& 0xffff);
						mem1=Memory((unsigned short)(PC_REG+1));
						mem2=Memory((unsigned short)(PC_REG+2));
//						pmem=&WorkMemory[(unsigned short)(PC_REG+1)];
//						mem1=*pmem++;mem2=*pmem;
						switch(DDD){
							case 0:
								if(Z_FLAG==0){
									PC_REG=(mem1 | (mem2<<8))-1;
								}else{
									PC_REG=(PC_REG+2)& 0xffff;
								}
								break;
							case 1:
								if(Z_FLAG==1){
									PC_REG=(mem1 | (mem2<<8))-1;
								}else{
									PC_REG=(PC_REG+2)& 0xffff;
								}
								break;
							case 2:
								if(C_FLAG==0){
									PC_REG=(mem1 | (mem2<<8))-1;
								}else{
									PC_REG=(PC_REG+2)& 0xffff;
								}
								break;
							case 3:
								if(C_FLAG==1){
									PC_REG=(mem1 | (mem2<<8))-1;
								}else{
									PC_REG=(PC_REG+2)& 0xffff;
								}
								break;

						}
						DEBUG_CODE("JP cc,nn\n");

					break;
					case 0x4://CALL cc,nn
						pushpair=(PC_REG+2)&0xffff;
						Memory((SP_REG-1)& 0xffff)=(pushpair>>8)&0xff;
						Memory((SP_REG-2)& 0xffff)=(pushpair)&0xff;
						mem1=Memory((PC_REG+1)& 0xffff);
						mem2=Memory((PC_REG+2)& 0xffff);
						switch(DDD){
							case 0:
								if(Z_FLAG==0){
									PC_REG=(mem1 | (mem2<<8))-1;
									SP_REG=SP_REG-2;
								}else{
									PC_REG=(PC_REG+2)& 0xffff;
								}
								break;
							case 1:
								if(Z_FLAG==1){
									PC_REG=(mem1 | (mem2<<8))-1;
									SP_REG=SP_REG-2;
								}else{
									PC_REG=(PC_REG+2)& 0xffff;
								}
								break;
							case 2:
								if(C_FLAG==0){
									PC_REG=(mem1 | (mem2<<8))-1;
									SP_REG=SP_REG-2;
								}else{
									PC_REG=(PC_REG+2)& 0xffff;
								}
								break;
							case 3:
								if(C_FLAG==1){
									PC_REG=(mem1 | (mem2<<8))-1;
									SP_REG=SP_REG-2;
								}else{
									PC_REG=(PC_REG+2)& 0xffff;
								}
								break;
						}
						DEBUG_CODE("CALL cc,nn\n");

					break;
					case 0x0://RET cc
						mem1=Memory((SP_REG)& MAX_MEMORY_SIZE);
						mem2=Memory((SP_REG+1)& MAX_MEMORY_SIZE);
//					printf("RET DDD=%d Z=%d C=%d\n",DDD,Z_FLAG,C_FLAG);
						switch(DDD){
							case 0:
								if(Z_FLAG==0){
									SP_REG=SP_REG+2;
									PC_REG=(mem1 | (mem2<<8))-1;
								}
								break;
							case 1:
								if(Z_FLAG==1){
									SP_REG=SP_REG+2;
									PC_REG=(mem1 | (mem2<<8))-1;
								}
								break;
							case 2:
								if(C_FLAG==0){
									SP_REG=SP_REG+2;
									PC_REG=(mem1 | (mem2<<8))-1;
								}
								break;
							case 3:
								if(C_FLAG==1){
									SP_REG=SP_REG+2;
									PC_REG=(mem1 | (mem2<<8))-1;
								}
								break;
						}

						DEBUG_CODE("RET cc\n");

					break;
					case 0x7://RST p
						pushpair=(PC_REG+1)&0xffff;
						Memory((SP_REG-1)& 0xffff)=(pushpair>>8)&0xff;
						Memory((SP_REG-2)& 0xffff)=(pushpair)&0xff;
						//pushpair=DDD * 8;
						//mem1=Memory((PC_REG+1)& 0xffff);
						//mem2=Memory((PC_REG+2)& 0xffff);
						PC_REG=DDD*8 -1 ;//(mem1 | (mem2<<8))-1;
						SP_REG=SP_REG-2;
					
						DEBUG_CODE("RST p\n");
					break;
				}
			break;
			case 0x80:
			break;
		}
	}
	//printf("\n");
	return 0;
}

#define DEFAULT_STARTADDRESS 0000
typedef union
{
	unsigned char BYTE;
	struct{
		unsigned char C:1;
		unsigned char N:1;
		unsigned char PV:1;
		unsigned char X:1;
		unsigned char H:1;
		unsigned char Y:1;
		unsigned char Z:1;
		unsigned char S:1;
	};
}FLAG;
typedef struct
{
	FLAG F;
	unsigned char A;
}REGISTER_AF;

typedef struct
{
	unsigned char LOW,HIGH;
}REGISTER_PAIR;

typedef union
{
	unsigned short int SHORT;
	REGISTER_AF BYTE;
}UNION_AF;
typedef union
{
	unsigned short int SHORT;
	REGISTER_PAIR BYTE;
}UNION_BC;
typedef union
{
	unsigned short int SHORT;
	REGISTER_PAIR BYTE;
}UNION_DE;
typedef union
{
	unsigned short int SHORT;
	REGISTER_PAIR BYTE;
}UNION_HL;

typedef union
{
	unsigned short int SHORT;
	REGISTER_PAIR BYTE;
}UNION_IX;
typedef union
{
	unsigned short int SHORT;
	REGISTER_PAIR BYTE;
}UNION_IY;

typedef struct
{
	UNION_AF AF;
	UNION_BC BC;
	UNION_DE DE;
	UNION_HL HL;
	UNION_IX IX;
	UNION_IY IY;
	unsigned short SP;
	unsigned short PC;
	unsigned char I;//使わない
	unsigned char R;//使わない
	unsigned short IFF,IFF1,IFF2;//割り込み（使わない）
	unsigned short AF2,BC2,DE2,HL2;//裏レジスタ
}REGISTER;

#define C_FLAG regs.AF.BYTE.F.C
#define Z_FLAG regs.AF.BYTE.F.Z
#define S_FLAG regs.AF.BYTE.F.S
#define PV_FLAG regs.AF.BYTE.F.PV
#define H_FLAG regs.AF.BYTE.F.H
#define N_FLAG regs.AF.BYTE.F.N
#define X_FLAG regs.AF.BYTE.F.X
#define Y_FLAG regs.AF.BYTE.F.Y

#define A_REG regs.AF.BYTE.A
#define F_REG regs.AF.BYTE.F.BYTE
#define B_REG regs.BC.BYTE.HIGH
#define C_REG regs.BC.BYTE.LOW
#define D_REG regs.DE.BYTE.HIGH
#define E_REG regs.DE.BYTE.LOW
#define H_REG regs.HL.BYTE.HIGH
#define L_REG regs.HL.BYTE.LOW
#define A_PTR &regs.AF.BYTE.HIGH
#define B_PTR &regs.BC.BYTE.HIGH
#define C_PTR &regs.BC.BYTE.LOW
#define D_PTR &regs.DE.BYTE.HIGH
#define E_PTR &regs.DE.BYTE.LOW
#define H_PTR &regs.HL.BYTE.HIGH
#define L_PTR &regs.HL.BYTE.LOW

#define IXH_REG regs.IX.BYTE.HIGH
#define IXL_REG regs.IX.BYTE.LOW
#define IYH_REG regs.IY.BYTE.HIGH
#define IYL_REG regs.IY.BYTE.LOW
#define I_REG regs.I
#define R_REG regs.R
#define IFF_REG regs.IFF
#define IFF1_REG regs.IFF1
#define IFF2_REG regs.IFF2
#define AF_REG regs.AF.SHORT
#define BC_REG regs.BC.SHORT
#define DE_REG regs.DE.SHORT
#define HL_REG regs.HL.SHORT
#define AF2_REG regs.AF2
#define BC2_REG regs.BC2
#define DE2_REG regs.DE2
#define HL2_REG regs.HL2

#define IX_REG regs.IX.SHORT
#define IY_REG regs.IY.SHORT
#define SP_REG regs.SP
#define PC_REG regs.PC

#define BC_PTR &regs.BC.SHORT
#define DE_PTR &regs.DE.SHORT
#define HL_PTR &regs.HL.SHORT
#define SP_PTR &regs.SP


#include "asm.h"
#include "symtab.h"
#include <list>

// 寄存器与变量的映射队列
std::list<RegInfo> regInfoList;
// 剩余可用reg数量:t0-t9，每次函数调用前，应保存寄存器数据，函数内部重新赋值10
static int s_emptyRegNum = 10;
// 当前分析到的四元式序号
static int cur_4var = 0;
// 是否已在寄存器中
static bool isInReg = false;
// 要读取的变量是否全局变量
static bool isGlobal = false;
// 要写入内存的变量是否全局变量
static bool p_isGlobal = false;
// 函数返回类型
static Type s_funcRetType;

void data2asm();
int getRegIndex(char* varname);
void updateSymTab();
void genasm();
int checkRegInfoList(char* varname);
int getAnEmptyReg(char* varname, Symbol* sb);
void insertTempVar(int t);
void function2asm();
void ignoreVarDef();

/*
根据变量名，返回所在的寄存器号；
同时需要更新此寄存器到队列末尾
*/
int checkRegInfoList(char* varname) {
	std::list<RegInfo>::iterator iter = regInfoList.begin();
	for (; iter != regInfoList.end(); ++iter) {
		if ((*iter).varname == varname) {
			break;
		}
	}
	RegInfo nn = (*iter);
	regInfoList.push_back(nn);

	regInfoList.erase(iter);

	return nn.regindex;
}



/*
若有空寄存器，返回下一个寄存器序号；
若无，FIFO原则，退出一个
*/
int getAnEmptyReg(char* varname, Symbol* sb) {
	if (s_emptyRegNum > 0) {					// 有空闲的寄存器：符号表isreg标志置为true，空闲寄存器数量-1，映射队列插入新元素
		if (sb) {
			sb->isreg = true;
		}

		int index = 10 - s_emptyRegNum;
		s_emptyRegNum--;

		RegInfo nn = { index, varname };
		regInfoList.push_back(nn);

		return index;
	}
	else {									// 没有空闲寄存器，需要清理出一个
		// 将一个寄存器释放，送入内存，isreg标志位改变，映射队列pop；新元素isreg为true，映射队列pushback。
		RegInfo nn = regInfoList.front();
		regInfoList.pop_front();

		if (isdigit(nn.varname[0])) {		// 弹出的是立即数，不用送至内存
			//return nn.regindex;
		}
		else {
			Symbol* sb_pop = lookUp_SymTab(nn.varname, p_isGlobal);
			sb_pop->isreg = false;
			if (sb_pop->vec >= 0) {				// 弹出的是数组的基地址，也不用送入内存
				//return nn.regindex;
			}
			else {								// 弹出的是变量：分为全局和局部（临时）
				if (p_isGlobal) {
					fprintf(ASM_FILE, "\tsw\t$t%d,$%s\n",
						nn.regindex,
						nn.varname
					);
				}
				else {
					fprintf(ASM_FILE, "\tsw\t$t%d,%d($fp)\n",
						nn.regindex,
						sb_pop->adress + 8
					);
				}
				//return nn.regindex;
			}
		}
		// 弹出的元素处理完毕
		if (sb) {
			sb->isreg = true;
		}

		RegInfo ttt = { nn.regindex, varname };
		regInfoList.push_back(ttt);

		return nn.regindex;
	}
}

/*
用途：管理寄存器堆。当需要加载操作数时（立即数、变量值、数组基地址），查看是否已在寄存器，或加载到哪个寄存器。
输入：变量名
输出：该变量可以存入的寄存器index，或者是已经存在的寄存器序号, isInReg, isGlobal
首先查找符号表中该变量是否在寄存器中，
	如果在：返回序号，维护寄存器状态列表（对于全局、局部、临时变量都表示值是否在寄存器，数组表示基地址是否在）
	不在：查看是否有空寄存器，若无，FIFO原则维护寄存器堆

	此外还需维护两个全局变量: 表示是否在寄存器中（用于该函数体外），是否全局变量（.data段全局变量可以通过变量名直接获取、存储）
*/
int getRegIndex(char* varname) {
	if (isdigit(varname[0])) {			// 立即数，不查询是否在寄存器，直接获取一个
		return getAnEmptyReg(varname, NULL);
	}
	// 变量
	Symbol* sb = lookUp_SymTab(varname, isGlobal);

	if (sb->isreg) {						// 已经在寄存器堆中
		isInReg = true;
		return checkRegInfoList(varname);
	}
	else {									// 不在寄存器中
		isInReg = false;
		return getAnEmptyReg(varname, sb);
	}
}

// 维护符号表：第一张全局表不动；之后表逆序，按照函数定义的顺序
void updateSymTab() {
	SymTab* st1, * st2;

	st1 = g_symtab->next;
	g_symtab->next = NULL;

	while (st1) {
		st2 = st1->next;

		st1->next = g_symtab->next;
		g_symtab->next = st1;

		st1 = st2;
	}
}

/*
生成MIPS汇编代码；
首先维护符号表（部分逆序）；之后根据四元式生成
*/
void genasm()
{
	updateSymTab();
	// 全局变量
	data2asm();
	fprintf(ASM_FILE, ".text:\n");
	fprintf(ASM_FILE, "\tjal\tmain\n");
	fprintf(ASM_FILE, "#TODO\n");
	// 开始处理函数：先处理栈、再处理局部变量、临时变量、最后逐步处理四元式
	while (cur_4var < NXQ) {
		if (!strcmp(quadvarlist[cur_4var].op, "Func")) {
			// 要记录函数返回类型，便于截断数据
			if (quadvarlist[cur_4var].var1[0] == 'i') {
				s_funcRetType = Type::T_INTEGER;
			}
			else if (quadvarlist[cur_4var].var1[0] == 'c') {
				s_funcRetType = Type::T_CHAR;
			}
			else {
				s_funcRetType = Type::T_VOID;
			}
			fprintf(ASM_FILE, "%s:\n", quadvarlist[cur_4var].var3);
		}
		else if (!strcmp(quadvarlist[cur_4var].op, "Main")) {
			fprintf(ASM_FILE, "main:\n");
		}
		else {						// 不应该进入此分支
			printf("unexpected error in genasm()\n");
		}
		// 处理函数体
		cur_4var++;
		function2asm();

		// 当前函数分析完成，进入下一个函数
		g_symtab->next = g_symtab->next->next;
	}
}

/*
初始化 .data段	+ 全局变量和字符串
*/
void data2asm() {

	fprintf(ASM_FILE, ".data:\n");

	// 全局变量
	while (strcmp("Func", quadvarlist[cur_4var].op)) {
		if (!strcmp("const", quadvarlist[cur_4var].op)) {
			// 无论是int 还是 char 都是存储数值；这里的数值在之前已经全部确定了，溢出的char值，取模
			fprintf(ASM_FILE, "\t$%s:\t.word\t%s\n",
				quadvarlist[cur_4var].var3,
				quadvarlist[cur_4var].var2
			);
		}
		else if (!strcmp("int", quadvarlist[cur_4var].op)) {
			fprintf(ASM_FILE, "\t$%s:\t.word\n",
				quadvarlist[cur_4var].var3
			);
		}
		else if (!strcmp("char", quadvarlist[cur_4var].op)) {	// 事实上这里，char分配的空间也是4
			fprintf(ASM_FILE, "\t$%s:\t.word\n",
				quadvarlist[cur_4var].var3
			);
		}
		else if (!strcmp("intarray", quadvarlist[cur_4var].op)) {
			fprintf(ASM_FILE, "\t$%s:\t.space\t%d\n",
				quadvarlist[cur_4var].var3,
				atoi(quadvarlist[cur_4var].var1) * 4
			);
		}
		else if (!strcmp("chararray", quadvarlist[cur_4var].op)) {
			fprintf(ASM_FILE, "\t$%s:\t.space\t%d\n",
				quadvarlist[cur_4var].var3,
				atoi(quadvarlist[cur_4var].var1) * 4
			);
		}
		else {
			std::cout << "unexpect error in genasm:.data\n";
		}



		cur_4var++;
	}

	// 字符串
	for (int i = 0; i < str_index; i++) {
		fprintf(ASM_FILE, "\t$string%d:\t.asciiz\t\"%s\"\n",
			i, stringlist[i]
		);
	}

	
}


void function2asm() {
	s_emptyRegNum = 10;							// 寄存器清空，但寄存器写入内存由函数调用者实现
	// 栈的变化
	fprintf(ASM_FILE, "\tsw\t$fp, ($sp)\n");		// ($sp) = $fp
	fprintf(ASM_FILE, "\tmove\t$fp, $sp\n");		// $fp = $sp
	fprintf(ASM_FILE, "\tsubi\t$sp, $sp, 8\n");	// $sp -= 8
	fprintf(ASM_FILE, "\tsw\t$ra, 4($sp)\n");		// $ra


	ignoreVarDef();

	insertTempVar(cur_4var);

	// 为函数参数、局部变量、临时变量分配空间
	fprintf(ASM_FILE, "\tsubi\t$sp, $sp, %d\n", g_symtab->next->varsize);

	// 开始分析
	while (strcmp(quadvarlist[cur_4var].op, "endf")) {
		cur_4var++;
	}

	// 恢复状态
	fprintf(ASM_FILE, "\tlw\t$ra, -4($fp)\n");	// 恢复$ra
	fprintf(ASM_FILE, "\tmove\t$sp, $fp\n");		// $sp = $fp
	fprintf(ASM_FILE, "\tlw\t$fp, ($fp)\n");		// $fp = ($fp)

	cur_4var++;
}

// 跳过前面的变量定义，参数
void ignoreVarDef() {
	while (!strcmp(quadvarlist[cur_4var].op, "const")
		|| !strcmp(quadvarlist[cur_4var].op, "int")
		|| !strcmp(quadvarlist[cur_4var].op, "char")
		|| !strcmp(quadvarlist[cur_4var].op, "intarray")
		|| !strcmp(quadvarlist[cur_4var].op, "chararray")
		|| !strcmp(quadvarlist[cur_4var].op, "para")
		) {
		cur_4var++;
	}
}

// 遍历，将临时变量插入符号表
void insertTempVar(int t) {
	// 临时变量定义 只会出现在 var3的位置
	while (strcmp(quadvarlist[t].op, "endf")) {
		if (quadvarlist[t].var3[0] == '$') {
			insertTempVar2SymTab(quadvarlist[t].var3);
		}
		t++;
	}

}


#include "asm.h"
#include "symtab.h"
#include <list>

std::list<RegInfo> regInfoList;
static int emptyRegNum = 10;
static int cur_4var = 0;

void dataasm();

/*
功能：管理寄存器堆，需要时返回一个可用的寄存器序号
输入：变量名
输出：该变量可以存入的寄存器index
首先查找符号表中该变量是否在寄存器中，
	如果在：返回序号，维护寄存器状态列表
	不在：查看是否有空寄存器，若无，FIFO原则维护寄存器堆
*/

int getRegIndex(char* varname) {


	return 0;
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

	dataasm();

	



}

/*
初始化 .data段	+ 全局变量和字符串
*/
void dataasm() {

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




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
// 函数名
static char* s_funcName;

void data2asm();
int getRegIndex(char* varname);
void updateSymTab();
void genasm();
int checkRegInfoList(char* varname);
int getAnEmptyReg(char* varname, Symbol* sb);
void insertTempVar(int t);
void function2asm();
void matchParaType();
void ignoreVarDef();


void add2asm();
void sub2asm();
void mul2asm();
void div2asm();
void callret2asm();
void getarray2asm();
void gre2asm();
void geq2asm();
void lss2asm();
void leq2asm();
void neq2asm();
void eql2asm();
void j2asm();
void jnz2asm();
void para2asm();
void setarray2asm();
void assign2asm();
void lab2asm();
void call2asm();
void vpara2asm();
void scanf2asm();
void print2asm();
void ret2asm();

void saveReg();
// 读取指定遍历到指定寄存器
void mem2reg(char* varname, int reg);
void baseAddr2reg(char* varname, int reg);

/*
根据变量名，返回所在的寄存器号；
同时需要更新此寄存器到队列末尾
*/
int checkRegInfoList(char* varname) {
	std::list<RegInfo>::iterator iter = regInfoList.begin();
	for (; iter != regInfoList.end(); ++iter) {
		if (!strcmp((*iter).varname, varname)) {
			INFO("剩余寄存器：%d，已在寄存器：true，变量名：%s，寄存器序号：%d\n", s_emptyRegNum, varname, std::distance(regInfoList.begin(),iter));
			break;
		}
	}
	if (iter == regInfoList.end()) {
		printf("unpected error in checkRegInfoList()\n");
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

		INFO("剩余寄存器：%d，变量名：%s，寄存器序号：%d\n", s_emptyRegNum, varname, index);
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
					fprintf(ASM_FILE, "\tsw\t\t$t%d,$%s\n",
						nn.regindex,
						nn.varname
					);
				}
				else {
					fprintf(ASM_FILE, "\tsw\t\t$t%d, -%d($fp)\n",
						nn.regindex,
						sb_pop->adress
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

		INFO("剩余寄存器：%d，old变量名：%s，变量名：%s，寄存器序号：%d\n", s_emptyRegNum, nn.varname, varname, nn.regindex);

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
	if (!sb) {
		printf("unexpect error: cannot find symbol\n");
		exit(-1);
	}

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
	fprintf(ASM_FILE, "\tjal\t\tmain\n");
	//fprintf(ASM_FILE, "#TODO\n");
	fprintf(ASM_FILE, "\tli\t\t$v0, 10\n");
	fprintf(ASM_FILE, "\tsyscall\n");
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
			s_funcName = quadvarlist[cur_4var].var3;
			fprintf(ASM_FILE, "%s:\n", quadvarlist[cur_4var].var3);
		}
		else if (!strcmp(quadvarlist[cur_4var].op, "Main")) {
			s_funcName = (char*)"main";
			fprintf(ASM_FILE, "main:\n");
			s_funcRetType = Type::T_VOID;
		}
		else {						// 正常情况不应该进入此分支
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
			fprintf(ASM_FILE, "\t$%s:\t\t.word\t%s\n",
				quadvarlist[cur_4var].var3,
				quadvarlist[cur_4var].var2
			);
		}
		else if (!strcmp("int", quadvarlist[cur_4var].op)) {
			fprintf(ASM_FILE, "\t$%s:\t\t.word\n",
				quadvarlist[cur_4var].var3
			);
		}
		else if (!strcmp("char", quadvarlist[cur_4var].op)) {	// 事实上这里，char分配的空间也是4
			fprintf(ASM_FILE, "\t$%s:\t\t.word\n",
				quadvarlist[cur_4var].var3
			);
		}
		else if (!strcmp("intarray", quadvarlist[cur_4var].op)) {
			fprintf(ASM_FILE, "\t$%s:\t\t.space\t%d\n",
				quadvarlist[cur_4var].var3,
				atoi(quadvarlist[cur_4var].var1) * 4
			);
		}
		else if (!strcmp("chararray", quadvarlist[cur_4var].op)) {
			fprintf(ASM_FILE, "\t$%s:\t\t.space\t%d\n",
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
	s_emptyRegNum = 10;								// 寄存器清空，但寄存器写入内存由函数调用者实现
	regInfoList.clear();							// 清空队列
	// 符号表不处理，因为连函数表都被抛弃了
													// 栈的变化
	fprintf(ASM_FILE, "\tsw\t\t$fp, ($sp)\n");		// ($sp) = $fp
	fprintf(ASM_FILE, "\tmove\t$fp, $sp\n");		// $fp = $sp
	fprintf(ASM_FILE, "\tsubi\t$sp, $sp, 8\n");		// $sp -= 8
	fprintf(ASM_FILE, "\tsw\t\t$ra, 4($sp)\n");		// $ra

	// 有些参数类型需要转换
	matchParaType();

	ignoreVarDef();

	insertTempVar(cur_4var);

	// 为函数参数、局部变量、临时变量分配空间
	fprintf(ASM_FILE, "\tsubi\t$sp, $sp, %d\n", g_symtab->next->varsize - 8);

	// 开始分析
	while (strcmp(quadvarlist[cur_4var].op, "endf")) {
		if (0 == strcmp(quadvarlist[cur_4var].op, "+")) {
			add2asm();
		}
		else if (0 == strcmp(quadvarlist[cur_4var].op, "-")) {
			sub2asm();
		}
		else if (0 == strcmp(quadvarlist[cur_4var].op, "*")) {
			mul2asm();
		}
		else if (0 == strcmp(quadvarlist[cur_4var].op, "/")) {
			div2asm();
		}
		else if (0 == strcmp(quadvarlist[cur_4var].op, "callret")) {
			callret2asm();
		}
		else if (0 == strcmp(quadvarlist[cur_4var].op, "getarray")) {
			getarray2asm();
		}
		else if (0 == strcmp(quadvarlist[cur_4var].op, ">")) {
			gre2asm();
		}
		else if (0 == strcmp(quadvarlist[cur_4var].op, ">=")) {
			geq2asm();
		}
		else if (0 == strcmp(quadvarlist[cur_4var].op, "<")) {
			lss2asm();
		}
		else if (0 == strcmp(quadvarlist[cur_4var].op, "<=")) {
			leq2asm();
		}
		else if (0 == strcmp(quadvarlist[cur_4var].op, "==")) {
			eql2asm();
		}
		else if (0 == strcmp(quadvarlist[cur_4var].op, "!=")) {
			neq2asm();
		}
		else if (0 == strcmp(quadvarlist[cur_4var].op, "j")) {
			j2asm();
		}
		else if (0 == strcmp(quadvarlist[cur_4var].op, "jnz")) {
			jnz2asm();
		}
		else if (0 == strcmp(quadvarlist[cur_4var].op, "para")) {
			para2asm();
		}
		else if (0 == strcmp(quadvarlist[cur_4var].op, "setarray")) {
			setarray2asm();
		}
		else if (0 == strcmp(quadvarlist[cur_4var].op, "assign")) {
			assign2asm();
		}
		else if (0 == strcmp(quadvarlist[cur_4var].op, "lab")) {
			lab2asm();
		}
		else if (0 == strcmp(quadvarlist[cur_4var].op, "call")) {
			call2asm();
		}
		else if (0 == strcmp(quadvarlist[cur_4var].op, "vpara")) {
			vpara2asm();
		}
		else if (0 == strcmp(quadvarlist[cur_4var].op, "scanf")) {
			scanf2asm();
		}
		else if (0 == strcmp(quadvarlist[cur_4var].op, "print")) {
			print2asm();
		}
		else if (0 == strcmp(quadvarlist[cur_4var].op, "ret")) {
			ret2asm();
		}
		cur_4var++;
	}

	// 函数体结束，处理栈的变化等等
	// 这一步也很重要，寄存器的值不要忘了写回
	fprintf(ASM_FILE, "ret_%s:\n", s_funcName);

	saveReg();										// 全局变量需要保存、局部变量已经没有意义
	//s_emptyRegNum = 10;								// 寄存器清空
	//regInfoList.clear();							// 清空队列

	// 恢复状态
	fprintf(ASM_FILE, "\tlw\t\t$ra, -4($fp)\n");	// 恢复$ra
	fprintf(ASM_FILE, "\tmove\t$sp, $fp\n");		// $sp = $fp
	fprintf(ASM_FILE, "\tlw\t\t$fp, ($fp)\n");		// $fp = ($fp)
	
	fprintf(ASM_FILE, "\tjr\t\t$ra\n");
										
	// endf
	cur_4var++;


}

// 跳过前面的变量定义，参数
void ignoreVarDef() {
	while (!strcmp(quadvarlist[cur_4var].op, "const")
		|| !strcmp(quadvarlist[cur_4var].op, "int")
		|| !strcmp(quadvarlist[cur_4var].op, "char")
		|| !strcmp(quadvarlist[cur_4var].op, "intarray")
		|| !strcmp(quadvarlist[cur_4var].op, "chararray")
		//|| !strcmp(quadvarlist[cur_4var].op, "para")
		) {
		cur_4var++;
	}
}

// 匹配参数类型: 有时候传入的实参是int型，形参是char，要转换
void matchParaType() {
	Symbol* sbf = lookUp_SymTab(s_funcName);

	FuncInfo* pf = sbf->pfinfo;
	fprintf(ASM_FILE, "\t#match para type\n");
	for (int i = 0; !strcmp(quadvarlist[cur_4var].op, "para"); i++, cur_4var++) {
		if (pf->paratable[i].ptype == Type::T_CHAR) {
			// 转换
			
			fprintf(ASM_FILE, "\tlw\t\t$a0, -%d($fp)\n", 8 + 4 * i);
			fprintf(ASM_FILE, "\tandi\t$a0, $a0, 0xff\n");
			fprintf(ASM_FILE, "\tsw\t\t$a0, -%d($fp)\n", 8 + 4 * i);
		}
	}
	fprintf(ASM_FILE, "\t#match finished\n");
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


/* 
(+,id/num,id/num,id)
id = id/num + id/num
*/
void add2asm()
{
	/*
	getRegIndex应用举例： 有两种情况：数字或者标识符（数组存的是基地址）
		数字：不查询寄存器是否已经存储，直接获得一个可用寄存器（可用减一 or 将一个寄存器送入内存）
		标识符：先查看是否在寄存器，如果在，返回所在序号，更新队列
				若不在，获得一个可用寄存器（可用减一 or 将一个寄存器送入内存）

		mem2reg：获得了返回的寄存器序号、isInReg，isGlobal状态 之后，数字直接读入寄存器；
				变量：根据isInReg判断是否需要从内存读取，根据isGlobal觉得读取方法
	*/
	int r1, r2, r3;
	//  var1
	r1 = getRegIndex(quadvarlist[cur_4var].var1);
	mem2reg(quadvarlist[cur_4var].var1, r1);

	// var2
	r2 = getRegIndex(quadvarlist[cur_4var].var2);
	mem2reg(quadvarlist[cur_4var].var2, r2);

	// var3 = var1 + var2
	r3 = getRegIndex(quadvarlist[cur_4var].var3);
	fprintf(ASM_FILE, "\tadd\t\t$t%d, $t%d, $t%d\n", r3, r1, r2);
}

/*
(-,id/num,id/num,id)
id = id/num + id/num
*/
void sub2asm()
{
	int r1, r2, r3;
	//  var1
	r1 = getRegIndex(quadvarlist[cur_4var].var1);
	mem2reg(quadvarlist[cur_4var].var1, r1);


	// var2
	r2 = getRegIndex(quadvarlist[cur_4var].var2);
	mem2reg(quadvarlist[cur_4var].var2, r2);

	// var3 = var1 - var2
	r3 = getRegIndex(quadvarlist[cur_4var].var3);
	fprintf(ASM_FILE, "\tsub\t\t$t%d, $t%d, $t%d\n", r3, r1, r2);
}


/*
(*,id/num,id/num,id)
MIPS指令：mul	$s1, $s0, $s2 ==> s1 = s0 * s2 溢出不考虑
*/
void mul2asm()
{
	int r1, r2, r3;
	//  var1
	r1 = getRegIndex(quadvarlist[cur_4var].var1);

	mem2reg(quadvarlist[cur_4var].var1, r1);

	// var2
	r2 = getRegIndex(quadvarlist[cur_4var].var2);

	mem2reg(quadvarlist[cur_4var].var2, r2);

	// var3 = var1 + var2
	r3 = getRegIndex(quadvarlist[cur_4var].var3);
	fprintf(ASM_FILE, "\tmul\t\t$t%d, $t%d, $t%d\n", r3, r1, r2);
}

void div2asm()
{
	int r1, r2, r3;
	//  var1
	r1 = getRegIndex(quadvarlist[cur_4var].var1);
	mem2reg(quadvarlist[cur_4var].var1, r1);


	// var2
	r2 = getRegIndex(quadvarlist[cur_4var].var2);

	mem2reg(quadvarlist[cur_4var].var2, r2);

	// var3 = var1 + var2
	r3 = getRegIndex(quadvarlist[cur_4var].var3);
	fprintf(ASM_FILE, "\tdiv\t\t$t%d, $t%d, $t%d\n", r3, r1, r2);
}

/*
callret,id（函数名）, ,id(ret)
*/
void callret2asm()
{
	saveReg();

	fprintf(ASM_FILE, "\tjal\t\t%s\n", quadvarlist[cur_4var].var1);

	fprintf(ASM_FILE, "\tmove\t$t%d, $v0\n", getRegIndex(quadvarlist[cur_4var].var3));

}

/*
(getarray,id（数组名）,id/num（index）,id)
*/
void getarray2asm()
{
	// 数组名
	int r1 = getRegIndex(quadvarlist[cur_4var].var1);
	// 由于全局数组和局部数组的地址增长方向不一样，要记录
	bool isGlobArray = isGlobal;
	// 因为这里要读取的时数组的基地址；而不是里面的值；不能用mem2reg
	baseAddr2reg(quadvarlist[cur_4var].var1, r1);

	int r2 = getRegIndex(quadvarlist[cur_4var].var2);
	mem2reg(quadvarlist[cur_4var].var2, r2);

	// 计算index地址
	fprintf(ASM_FILE, "\tmul\t\t$s0, $t%d, 4\n", r2);
	if (isGlobArray) {
		fprintf(ASM_FILE, "\tadd\t\t$s0, $t%d, $s0\n", r1);
	}
	else {
		fprintf(ASM_FILE, "\tsub\t\t$s0, $t%d, $s0\n", r1);
	}
	

	int r3 = getRegIndex(quadvarlist[cur_4var].var3);
	fprintf(ASM_FILE, "\tlw\t\t$t%d, ($s0)\n", r3);

}

void gre2asm()
{
}

void geq2asm()
{
}

void lss2asm()
{
}

void leq2asm()
{
}

void neq2asm()
{
}

void eql2asm()
{
}

void j2asm()
{
}

void jnz2asm()
{
}

// matchType已经完成了
void para2asm()
{
	/* do nothing*/
}


/*
(setarray,id / num,index,name)
name[index] = id/num
*/
void setarray2asm()
{
	// 数组名
	int r3 = getRegIndex(quadvarlist[cur_4var].var3);
	// 由于全局数组和局部数组的地址增长方向不一样，要记录
	bool isGlobArray = isGlobal;
	// 因为这里要读取的时数组的基地址；而不是里面的值；不能用mem2reg
	baseAddr2reg(quadvarlist[cur_4var].var3, r3);

	int r2 = getRegIndex(quadvarlist[cur_4var].var2);
	mem2reg(quadvarlist[cur_4var].var2, r2);

	// 计算index地址
	fprintf(ASM_FILE, "\tmul\t\t$s0, $t%d, 4\n", r2);
	if (isGlobArray) {
		fprintf(ASM_FILE, "\tadd\t\t$s0, $t%d, $s0\n", r3);
	}
	else {
		fprintf(ASM_FILE, "\tsub\t\t$s0, $t%d, $s0\n", r3);
	}

	int r1 = getRegIndex(quadvarlist[cur_4var].var1);
	mem2reg(quadvarlist[cur_4var].var1, r1);

	// check type
	Type real_type = lookUp_SymTab(quadvarlist[cur_4var].var3)->valueType;
	if (Type::T_CHAR == real_type) {
		fprintf(ASM_FILE, "\tandi\t$t%d, $t%d, 0xff", r1, r1);
	}

	fprintf(ASM_FILE, "\tsw\t\t$t%d, ($s0)\n", r1);
}

/*
（assign，id/num，，name）
*/
void assign2asm()
{
	int r1 = getRegIndex(quadvarlist[cur_4var].var1);

	mem2reg(quadvarlist[cur_4var].var1, r1);

	int r3 = getRegIndex(quadvarlist[cur_4var].var3);

	fprintf(ASM_FILE, "\tmove\t$t%d, $t%d\n", r3, r1);

	// check type
	Type real_type = lookUp_SymTab(quadvarlist[cur_4var].var3)->valueType;
	if (Type::T_CHAR == real_type) {
		fprintf(ASM_FILE, "\tandi\t$t%d, $t%d, 0xff", r3, r3);
	}
}

void lab2asm()
{
}

/*
（callret,id（函数名）,  ,  ）
*/
void call2asm()
{
	saveReg();

	fprintf(ASM_FILE, "\tjal\t\t%s\n", quadvarlist[cur_4var].var1);
}

// (vpara, , ,id/num)
void vpara2asm()
{
	INFO_ASM("\t#value Para:\n");
	int cnt = 0;
	while(!strcmp(quadvarlist[cur_4var].op, "vpara")) {
		int addr = 8 + 4 * cnt;

		// 先将数据读入寄存器
		int rr = getRegIndex(quadvarlist[cur_4var].var3);
		mem2reg(quadvarlist[cur_4var].var3, rr);
		// 将寄存器的数据送入指定位置
		fprintf(ASM_FILE, "\tsw\t\t$t%d, -%d($sp)\n", rr, addr);

		cnt++;
		cur_4var++;
	}
	cur_4var--;		// 不然会跳过函数调用
	INFO_ASM("\t#End of value Para\n");
}

/*
(scanf,  ,  ,name)
*/
void scanf2asm()
{
	Symbol* sb = lookUp_SymTab(quadvarlist[cur_4var].var3, isGlobal);

	if (Type::T_INTEGER == sb->valueType) {			// 数字
		fprintf(ASM_FILE, "\tli\t\t$v0\t5\n");
		fprintf(ASM_FILE, "\tsyscall\n");
		if (isGlobal) {
			fprintf(ASM_FILE, "\tla\t\t$s0, $%s\n", sb->name);
			fprintf(ASM_FILE, "\tsw\t\t$v0, ($s0)\n");
		}
		else {
			int addr = sb->adress;
			fprintf(ASM_FILE, "\tsw\t\t$v0, -%d($fp)\n", addr);
		}
	}
	else {											// 字符
		fprintf(ASM_FILE, "\tli\t\t$v0\t12\n");
		fprintf(ASM_FILE, "\tsyscall\n");
		if (isGlobal) {
			fprintf(ASM_FILE, "\tla\t\t$s0, $%s\n", sb->name);
			fprintf(ASM_FILE, "\tsw\t\t$v0, ($s0)\n");
		}
		else {
			int addr = sb->adress;
			fprintf(ASM_FILE, "\tsw\t\t$v0, -%d($fp)\n");
		}
	}
}

/*
(print,str_index, , )
(print,  ,id/num, int/char)
这里有点问题， 最后int/char没用；通过id/num来判断的
*/
void print2asm()
{
	if (quadvarlist[cur_4var].var3[0] == ' ') {
		fprintf(ASM_FILE, "\tla\t\t$a0, $string%s\n", quadvarlist[cur_4var].var1);
		fprintf(ASM_FILE, "\tli\t\t$v0, 4\n");
		fprintf(ASM_FILE, "\tsyscall\n");
	}
	else {
		int r2 = getRegIndex(quadvarlist[cur_4var].var2);
		mem2reg(quadvarlist[cur_4var].var2, r2);

		if (Type::T_INTEGER == lookUp_SymTab(quadvarlist[cur_4var].var2)->valueType) {
			fprintf(ASM_FILE, "\tmove\t$a0, $t%d\n", r2);
			fprintf(ASM_FILE, "\tli\t\t$v0, 1\n");
			fprintf(ASM_FILE, "\tsyscall\n");
		}
		else {
			fprintf(ASM_FILE, "\tmove\t$a0, $t%d\n", r2);
			fprintf(ASM_FILE, "\tli\t\t$v0, 11\n");
			fprintf(ASM_FILE, "\tsyscall\n");
		}
	}
}


/*
ret, , ,id/num
ret, , ,
*/
void ret2asm()
{
	
	if (quadvarlist[cur_4var].var3[0] == ' ') {		// == 写成 =了 。。。。。查了几分钟把，还好找出来了
		/* do nothing */
		if (Type::T_VOID != s_funcRetType) {
			printf("return type error ,id: %s\n", quadvarlist[cur_4var].var3);
		}
	}
	else {
		// 保存返回值
		int r3 = getRegIndex(quadvarlist[cur_4var].var3);
		mem2reg(quadvarlist[cur_4var].var3, r3);
		fprintf(ASM_FILE, "\tmove\t$v0, $t%d\n", r3);

		// 判断返回类型
		//Type real_ret_type = lookUp_SymTab(quadvarlist[cur_4var].var3)->valueType;
		if (Type::T_VOID == s_funcRetType) {
			// void : error
			printf("return type error ,id: %s\n", quadvarlist[cur_4var].var3);
		}
		else if (Type::T_CHAR == s_funcRetType) {
			// char: andi 0xff
			fprintf(ASM_FILE, "\tandi\t$v0, $v0, 0xff\n");
		}
		else {
			/* int: do nothing */
		}

	}

	fprintf(ASM_FILE, "\tj\t\tret_%s\n", s_funcName);
}

// 在函数调用前，将寄存器的值写入内存
void saveReg() {
	s_emptyRegNum = 10;

	INFO_ASM("\t#Save RegFile\n");
	while (regInfoList.size()) {
		RegInfo nn = regInfoList.front();
		regInfoList.pop_front();

		if (isdigit(nn.varname[0])) {		// 弹出的是立即数，不用送至内存
			continue;
		}
		else {
			Symbol* sb_pop = lookUp_SymTab(nn.varname, p_isGlobal);
			sb_pop->isreg = false;
			if (sb_pop->vec >= 0) {				// 弹出的是数组的基地址，也不用送入内存
				continue;
			}
			else {								// 弹出的是变量：分为全局和局部（临时）
				if (p_isGlobal) {
					fprintf(ASM_FILE, "\tsw\t\t$t%d, $%s\n",
						nn.regindex,
						nn.varname
					);
				}
				else {
					fprintf(ASM_FILE, "\tsw\t\t$t%d, -%d($fp)\n",
						nn.regindex,
						sb_pop->adress
					);
				}
			}
		}
	}
	INFO_ASM("\t#End of Saving RegFile\n");
}

// 从内存读入寄存器
void mem2reg(char* varname, int reg) {
	// 数字直接读入
	if (isdigit(varname[0])) {
		fprintf(ASM_FILE, "\tli\t\t$t%d, %s\n", reg, varname);
		return;
	}
	// 变量：
	// 不在寄存器，需要读取
	if (!isInReg) {
		if (isGlobal) {						// 读取全局变量
			fprintf(ASM_FILE, "\tlw\t\t$t%d, $%s\n", reg, varname);
		}
		else {								// 读取局部变量
			int addr = lookUp_SymTab(varname)->adress;
			fprintf(ASM_FILE, "\tlw\t\t$t%d, -%d($fp)\n", reg, addr);
		}
	}
}

// 数组的基地址读入内存
void baseAddr2reg(char* varname, int reg) {
	INFO_ASM("\t# Array BaseAddr\n");
	if (!isInReg) {
		if (isGlobal) {						// 全局变量 基地址
			fprintf(ASM_FILE, "\tla\t\t$t%d, $%s\n", reg, varname);
		}
		else {
			// 基地址
			int addr = lookUp_SymTab(varname)->adress;

			//fprintf(ASM_FILE, "\tli\t\t$t%d, %d\n", reg, addr);
			fprintf(ASM_FILE, "\tsubi\t$t%d, $fp, %d\n", reg, addr);
		}
	}
	INFO_ASM("\t# End of Array BaseAddr\n");
}


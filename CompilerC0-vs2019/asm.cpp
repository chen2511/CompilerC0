#include "asm.h"
#include "symtab.h"
#include <list>

// �Ĵ����������ӳ�����
std::list<RegInfo> regInfoList;
// ʣ�����reg����:t0-t9��ÿ�κ�������ǰ��Ӧ����Ĵ������ݣ������ڲ����¸�ֵ10
static int s_emptyRegNum = 10;
// ��ǰ����������Ԫʽ���
static int cur_4var = 0;
// �Ƿ����ڼĴ�����
static bool isInReg = false;
// Ҫ��ȡ�ı����Ƿ�ȫ�ֱ���
static bool isGlobal = false;
// Ҫд���ڴ�ı����Ƿ�ȫ�ֱ���
static bool p_isGlobal = false;
// ������������
static Type s_funcRetType;
// ������
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
// ��ȡָ��������ָ���Ĵ���
void mem2reg(char* varname, int reg);
void baseAddr2reg(char* varname, int reg);

/*
���ݱ��������������ڵļĴ����ţ�
ͬʱ��Ҫ���´˼Ĵ���������ĩβ
*/
int checkRegInfoList(char* varname) {
	std::list<RegInfo>::iterator iter = regInfoList.begin();
	for (; iter != regInfoList.end(); ++iter) {
		if (!strcmp((*iter).varname, varname)) {
			INFO("ʣ��Ĵ�����%d�����ڼĴ�����true����������%s���Ĵ�����ţ�%d\n", s_emptyRegNum, varname, std::distance(regInfoList.begin(),iter));
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
���пռĴ�����������һ���Ĵ�����ţ�
���ޣ�FIFOԭ���˳�һ��
*/
int getAnEmptyReg(char* varname, Symbol* sb) {
	if (s_emptyRegNum > 0) {					// �п��еļĴ��������ű�isreg��־��Ϊtrue�����мĴ�������-1��ӳ����в�����Ԫ��
		if (sb) {
			sb->isreg = true;
		}

		int index = 10 - s_emptyRegNum;
		s_emptyRegNum--;
		
		RegInfo nn = { index, varname };
		regInfoList.push_back(nn);

		INFO("ʣ��Ĵ�����%d����������%s���Ĵ�����ţ�%d\n", s_emptyRegNum, varname, index);
		return index;
	}
	else {									// û�п��мĴ�������Ҫ�����һ��
		// ��һ���Ĵ����ͷţ������ڴ棬isreg��־λ�ı䣬ӳ�����pop����Ԫ��isregΪtrue��ӳ�����pushback��
		RegInfo nn = regInfoList.front();
		regInfoList.pop_front();

		if (isdigit(nn.varname[0])) {		// �������������������������ڴ�
			//return nn.regindex;
		}
		else {
			Symbol* sb_pop = lookUp_SymTab(nn.varname, p_isGlobal);
			sb_pop->isreg = false;
			if (sb_pop->vec >= 0) {				// ������������Ļ���ַ��Ҳ���������ڴ�
				//return nn.regindex;
			}
			else {								// �������Ǳ�������Ϊȫ�ֺ;ֲ�����ʱ��
				if (p_isGlobal) {
					fprintf(ASM_FILE, "\tsw\t\t$t%d,$%s\n",
						nn.regindex,
						nn.varname
					);
				}
				else {
					fprintf(ASM_FILE, "\tsw\t\t$t%d,-%d($fp)\n",
						nn.regindex,
						sb_pop->adress + 8
					);
				}
				//return nn.regindex;
			}
		}
		// ������Ԫ�ش������
		if (sb) {
			sb->isreg = true;
		}

		RegInfo ttt = { nn.regindex, varname };
		regInfoList.push_back(ttt);

		INFO("ʣ��Ĵ�����%d��old��������%s����������%s���Ĵ�����ţ�%d\n", s_emptyRegNum, nn.varname, varname, nn.regindex);

		return nn.regindex;
	}
}

/*
��;������Ĵ����ѡ�����Ҫ���ز�����ʱ��������������ֵ���������ַ�����鿴�Ƿ����ڼĴ���������ص��ĸ��Ĵ�����
���룺������
������ñ������Դ���ļĴ���index���������Ѿ����ڵļĴ������, isInReg, isGlobal
���Ȳ��ҷ��ű��иñ����Ƿ��ڼĴ����У�
	����ڣ�������ţ�ά���Ĵ���״̬�б�����ȫ�֡��ֲ�����ʱ��������ʾֵ�Ƿ��ڼĴ����������ʾ����ַ�Ƿ��ڣ�
	���ڣ��鿴�Ƿ��пռĴ��������ޣ�FIFOԭ��ά���Ĵ�����

	���⻹��ά������ȫ�ֱ���: ��ʾ�Ƿ��ڼĴ����У����ڸú������⣩���Ƿ�ȫ�ֱ�����.data��ȫ�ֱ�������ͨ��������ֱ�ӻ�ȡ���洢��
*/
int getRegIndex(char* varname) {
	if (isdigit(varname[0])) {			// ������������ѯ�Ƿ��ڼĴ�����ֱ�ӻ�ȡһ��
		return getAnEmptyReg(varname, NULL);
	}
	// ����
	Symbol* sb = lookUp_SymTab(varname, isGlobal);

	if (sb->isreg) {						// �Ѿ��ڼĴ�������
		isInReg = true;
		return checkRegInfoList(varname);
	}
	else {									// ���ڼĴ�����
		isInReg = false;
		return getAnEmptyReg(varname, sb);
	}
}

// ά�����ű���һ��ȫ�ֱ�����֮������򣬰��պ��������˳��
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
����MIPS�����룻
����ά�����ű��������򣩣�֮�������Ԫʽ����
*/
void genasm()
{
	updateSymTab();
	// ȫ�ֱ���
	data2asm();

	fprintf(ASM_FILE, ".text:\n");
	fprintf(ASM_FILE, "\tjal\t\tmain\n");
	//fprintf(ASM_FILE, "#TODO\n");
	fprintf(ASM_FILE, "\tli\t\t$v0, 10\n");
	fprintf(ASM_FILE, "\tsyscall\n");
	// ��ʼ���������ȴ���ջ���ٴ���ֲ���������ʱ����������𲽴�����Ԫʽ
	while (cur_4var < NXQ) {
		if (!strcmp(quadvarlist[cur_4var].op, "Func")) {
			// Ҫ��¼�����������ͣ����ڽض�����
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
		}
		else {						// ���������Ӧ�ý���˷�֧
			printf("unexpected error in genasm()\n");
		}
		// ��������
		cur_4var++;
		function2asm();

		// ��ǰ����������ɣ�������һ������
		g_symtab->next = g_symtab->next->next;
	}
}

/*
��ʼ�� .data��	+ ȫ�ֱ������ַ���
*/
void data2asm() {

	fprintf(ASM_FILE, ".data:\n");

	// ȫ�ֱ���
	while (strcmp("Func", quadvarlist[cur_4var].op)) {
		if (!strcmp("const", quadvarlist[cur_4var].op)) {
			// ������int ���� char ���Ǵ洢��ֵ���������ֵ��֮ǰ�Ѿ�ȫ��ȷ���ˣ������charֵ��ȡģ
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
		else if (!strcmp("char", quadvarlist[cur_4var].op)) {	// ��ʵ�����char����Ŀռ�Ҳ��4
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

	// �ַ���
	for (int i = 0; i < str_index; i++) {
		fprintf(ASM_FILE, "\t$string%d:\t.asciiz\t\"%s\"\n",
			i, stringlist[i]
		);
	}

	
}


void function2asm() {
	s_emptyRegNum = 10;								// �Ĵ�����գ����Ĵ���д���ڴ��ɺ���������ʵ��
	regInfoList.clear();							// ��ն���
	// ���ű�������Ϊ����������������
													// ջ�ı仯
	fprintf(ASM_FILE, "\tsw\t\t$fp, ($sp)\n");		// ($sp) = $fp
	fprintf(ASM_FILE, "\tmove\t$fp, $sp\n");		// $fp = $sp
	fprintf(ASM_FILE, "\tsubi\t$sp, $sp, 8\n");		// $sp -= 8
	fprintf(ASM_FILE, "\tsw\t\t$ra, 4($sp)\n");		// $ra

	// ��Щ����������Ҫת��
	matchParaType();

	ignoreVarDef();

	insertTempVar(cur_4var);

	// Ϊ�����������ֲ���������ʱ��������ռ�
	fprintf(ASM_FILE, "\tsubi\t$sp, $sp, %d\n", g_symtab->next->varsize);

	// ��ʼ����
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

	// ���������������ջ�ı仯�ȵ�
	// ��һ��Ҳ����Ҫ���Ĵ�����ֵ��Ҫ����д��
	saveReg();										// ȫ�ֱ�����Ҫ���桢�ֲ������Ѿ�û������
	//s_emptyRegNum = 10;								// �Ĵ������
	//regInfoList.clear();							// ��ն���

	fprintf(ASM_FILE, "ret_%s:\n", s_funcName);

	// �ָ�״̬
	fprintf(ASM_FILE, "\tlw\t\t$ra, -4($fp)\n");	// �ָ�$ra
	fprintf(ASM_FILE, "\tmove\t$sp, $fp\n");		// $sp = $fp
	fprintf(ASM_FILE, "\tlw\t\t$fp, ($fp)\n");		// $fp = ($fp)
	
	fprintf(ASM_FILE, "\tjr\t\t$ra\n");
										
	// endf
	cur_4var++;


}

// ����ǰ��ı������壬����
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

// ƥ���������: ��ʱ�����ʵ����int�ͣ��β���char��Ҫת��
void matchParaType() {
	Symbol* sbf = lookUp_SymTab(s_funcName);

	FuncInfo* pf = sbf->pfinfo;
	fprintf(ASM_FILE, "\t#match para type\n");
	for (int i = 0; !strcmp(quadvarlist[cur_4var].op, "para"); i++, cur_4var++) {
		if (pf->paratable[i].ptype == Type::T_CHAR) {
			// ת��
			
			fprintf(ASM_FILE, "\tlw\t\t$a0, -%d($fp)\n", 8 + 4 * i);
			fprintf(ASM_FILE, "\tandi\t$a0, $a0, 0xff\n");
			fprintf(ASM_FILE, "\tsw\t\t$a0, -%d($fp)\n", 8 + 4 * i);
		}
	}
	fprintf(ASM_FILE, "\t#match finished\n");
}

// ����������ʱ����������ű�
void insertTempVar(int t) {
	// ��ʱ�������� ֻ������� var3��λ��
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
	getRegIndexӦ�þ����� ��������������ֻ��߱�ʶ�����������ǻ���ַ��
		���֣�����ѯ�Ĵ����Ƿ��Ѿ��洢��ֱ�ӻ��һ�����üĴ��������ü�һ or ��һ���Ĵ��������ڴ棩
		��ʶ�����Ȳ鿴�Ƿ��ڼĴ���������ڣ�����������ţ����¶���
				�����ڣ����һ�����üĴ��������ü�һ or ��һ���Ĵ��������ڴ棩

		mem2reg������˷��صļĴ�����š�isInReg��isGlobal״̬ ֮������ֱ�Ӷ���Ĵ�����
				����������isInReg�ж��Ƿ���Ҫ���ڴ��ȡ������isGlobal���ö�ȡ����
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
MIPSָ�mul	$s1, $s0, $s2 ==> s1 = s0 * s2 ���������
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
callret,id����������, ,id(ret)
*/
void callret2asm()
{
	saveReg();

	fprintf(ASM_FILE, "\tjal\t\t%s\n", quadvarlist[cur_4var].var1);

	fprintf(ASM_FILE, "\tmove\t$t%d, $v0\n", getRegIndex(quadvarlist[cur_4var].var3));

}

/*
(getarray,id����������,id/num��index��,id)
*/
void getarray2asm()
{
	// ������
	int r1 = getRegIndex(quadvarlist[cur_4var].var1);
	// ����ȫ������;ֲ�����ĵ�ַ��������һ����Ҫ��¼
	bool isGlobArray = isGlobal;
	// ��Ϊ����Ҫ��ȡ��ʱ����Ļ���ַ�������������ֵ��������mem2reg
	baseAddr2reg(quadvarlist[cur_4var].var1, r1);

	int r2 = getRegIndex(quadvarlist[cur_4var].var2);
	mem2reg(quadvarlist[cur_4var].var2, r2);

	// ����index��ַ
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

// matchType�Ѿ������
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
	// ������
	int r3 = getRegIndex(quadvarlist[cur_4var].var3);
	// ����ȫ������;ֲ�����ĵ�ַ��������һ����Ҫ��¼
	bool isGlobArray = isGlobal;
	// ��Ϊ����Ҫ��ȡ��ʱ����Ļ���ַ�������������ֵ��������mem2reg
	baseAddr2reg(quadvarlist[cur_4var].var3, r3);

	int r2 = getRegIndex(quadvarlist[cur_4var].var2);
	mem2reg(quadvarlist[cur_4var].var2, r2);

	// ����index��ַ
	fprintf(ASM_FILE, "\tmul\t\t$s0, $t%d, 4\n", r2);
	if (isGlobArray) {
		fprintf(ASM_FILE, "\tadd\t\t$s0, $t%d, $s0\n", r3);
	}
	else {
		fprintf(ASM_FILE, "\tsub\t\t$s0, $t%d, $s0\n", r3);
	}


	int r1 = getRegIndex(quadvarlist[cur_4var].var1);
	mem2reg(quadvarlist[cur_4var].var1, r1);
	fprintf(ASM_FILE, "\tsw\t\t$t%d, ($s0)\n", r1);
}

/*
��assign��id/num����name��
*/
void assign2asm()
{
	int r1 = getRegIndex(quadvarlist[cur_4var].var1);

	mem2reg(quadvarlist[cur_4var].var1, r1);

	int r3 = getRegIndex(quadvarlist[cur_4var].var3);

	fprintf(ASM_FILE, "\tmove\t$t%d, $t%d\n", r1, r3);

}

void lab2asm()
{
}

/*
��callret,id����������,  ,  ��
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

		// �Ƚ����ݶ���Ĵ���
		int rr = getRegIndex(quadvarlist[cur_4var].var3);
		mem2reg(quadvarlist[cur_4var].var3, rr);
		// ���Ĵ�������������ָ��λ��
		fprintf(ASM_FILE, "\tsw\t\t$t%d, -%d($sp)\n", rr, addr);

		cnt++;
		cur_4var++;
	}
	cur_4var--;		// ��Ȼ��������������
	INFO_ASM("\t#End of value Para\n");
}

void scanf2asm()
{
}

void print2asm()
{
}


/*
ret, , ,id/num
ret, , ,
*/
void ret2asm()
{
	if (quadvarlist[cur_4var].var3[0] == ' ') {		// == д�� =�� �������������˼����Ӱѣ������ҳ�����
		
	}
	else {

	}



}

// �ں�������ǰ�����Ĵ�����ֵд���ڴ�
void saveReg() {
	s_emptyRegNum = 10;

	INFO_ASM("\t#Save RegFile\n");
	while (regInfoList.size()) {
		RegInfo nn = regInfoList.front();
		regInfoList.pop_front();

		if (isdigit(nn.varname[0])) {		// �������������������������ڴ�
			continue;
		}
		else {
			Symbol* sb_pop = lookUp_SymTab(nn.varname, p_isGlobal);
			sb_pop->isreg = false;
			if (sb_pop->vec >= 0) {				// ������������Ļ���ַ��Ҳ���������ڴ�
				continue;
			}
			else {								// �������Ǳ�������Ϊȫ�ֺ;ֲ�����ʱ��
				if (p_isGlobal) {
					fprintf(ASM_FILE, "\tsw\t\t$t%d, $%s\n",
						nn.regindex,
						nn.varname
					);
				}
				else {
					fprintf(ASM_FILE, "\tsw\t\t$t%d, -%d($fp)\n",
						nn.regindex,
						sb_pop->adress + 8
					);
				}
			}
		}
	}
	INFO_ASM("\t#End of Saving RegFile\n");
}

// ���ڴ����Ĵ���
void mem2reg(char* varname, int reg) {
	// ����ֱ�Ӷ���
	if (isdigit(varname[0])) {
		fprintf(ASM_FILE, "\tli\t\t$t%d, %s\n", reg, varname);
		return;
	}
	// ������
	// ���ڼĴ�������Ҫ��ȡ
	if (!isInReg) {
		if (isGlobal) {						// ��ȡȫ�ֱ���
			fprintf(ASM_FILE, "\tlw\t\t$t%d, $%s\n", reg, varname);
		}
		else {								// ��ȡ�ֲ�����
			int addr = lookUp_SymTab(varname)->adress;
			fprintf(ASM_FILE, "\tlw\t\t$t%d, -%d($fp)\n", reg, addr);
		}
	}
}

// ����Ļ���ַ�����ڴ�
void baseAddr2reg(char* varname, int reg) {
	INFO_ASM("\t# Array BaseAddr\n");
	if (!isInReg) {
		if (isGlobal) {						// ȫ�ֱ��� ����ַ
			fprintf(ASM_FILE, "\tla\t\t$t%d, $%s\n", reg, varname);
		}
		else {
			// ����ַ
			int addr = lookUp_SymTab(varname)->adress;

			fprintf(ASM_FILE, "\tli\t\t$t%d, %d\n", reg, addr);
			fprintf(ASM_FILE, "\tsub\t\t$t%d, $fp, $t%d\n", reg, reg);
		}
	}
	INFO_ASM("\t# End of Array BaseAddr\n");
}


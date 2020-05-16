#include "asm.h"
#include "symtab.h"
#include <list>

// �Ĵ����������ӳ�����
std::list<RegInfo> regInfoList;
// ʣ�����reg����:t0-t9��ÿ�κ�������ǰ��Ӧ����Ĵ������ݣ���ʱ���¸�ֵ10
static int emptyRegNum = 10;
// ��ǰ����������Ԫʽ���
static int cur_4var = 0;
// �Ƿ����ڼĴ�����
static bool isInReg = false;
// Ҫ��ȡ�ı����Ƿ�ȫ�ֱ���
static bool isGlobal = false;
// Ҫд���ڴ�ı����Ƿ�ȫ�ֱ���
static bool p_isGlobal = false;

void dataasm();
int getRegIndex(char* varname);
void updateSymTab();
void genasm();
int checkRegInfoList(char* varname);
int getAnEmptyReg(char* varname, Symbol* sb);

/*
���ݱ��������������ڵļĴ����ţ�
ͬʱ��Ҫ���´˼Ĵ���������ĩβ
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
���пռĴ�����������һ���Ĵ�����ţ�
���ޣ�FIFOԭ���˳�һ��
*/
int getAnEmptyReg(char* varname, Symbol* sb) {
	if (emptyRegNum > 0) {					// �п��еļĴ��������ű�isreg��־��Ϊtrue�����мĴ�������-1��ӳ����в�����Ԫ��
		if (sb) {
			sb->isreg = true;
		}

		int index = 10 - emptyRegNum;
		emptyRegNum--;

		RegInfo nn = { index, varname };
		regInfoList.push_back(nn);

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
		// ������Ԫ�ش������
		if (sb) {
			sb->isreg = true;
		}

		RegInfo ttt = { nn.regindex, varname };
		regInfoList.push_back(ttt);

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
	dataasm();
	// ��ʼ������
	while (strcmp(quadvarlist[cur_4var].op, "Func") || strcmp(quadvarlist[cur_4var].op, "Main")) {
		insertTempVar();
	}



}

/*
��ʼ�� .data��	+ ȫ�ֱ������ַ���
*/
void dataasm() {

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


void insertTempVar() {

}


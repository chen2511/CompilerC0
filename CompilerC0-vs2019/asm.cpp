#include "asm.h"
#include "symtab.h"
#include <list>

std::list<RegInfo> regInfoList;
static int emptyRegNum = 10;
static int cur_4var = 0;

void dataasm();

/*
���ܣ�����Ĵ����ѣ���Ҫʱ����һ�����õļĴ������
���룺������
������ñ������Դ���ļĴ���index
���Ȳ��ҷ��ű��иñ����Ƿ��ڼĴ����У�
	����ڣ�������ţ�ά���Ĵ���״̬�б�
	���ڣ��鿴�Ƿ��пռĴ��������ޣ�FIFOԭ��ά���Ĵ�����
*/

int getRegIndex(char* varname) {


	return 0;
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

	dataasm();

	



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




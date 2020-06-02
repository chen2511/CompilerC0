#include "opt.h"
#include "ir.h"

static int curIndex;




/* 
�����ϲ��� ���������Ϊ��ֵ
ǿ�������� �˷��ͳ�����Ϊ��λ
*/
void constantMergeAndStrengthReduction() 
{
	switch (quadvarlist[curIndex].op[0]) {
	case '+':
		if (isdigit(quadvarlist[curIndex].var1[0]) && isdigit(quadvarlist[curIndex].var2[0])) {
			int num1 = atoi(quadvarlist[curIndex].var1);
			int num2 = atoi(quadvarlist[curIndex].var2);

			quadvarlist[curIndex].var1 = newitoa(num1 + num2);
			quadvarlist[curIndex].var2 = newempty();
			strcpy(quadvarlist[curIndex].op, "assign");
		}
		break;
	case '-':
		if (isdigit(quadvarlist[curIndex].var1[0]) && isdigit(quadvarlist[curIndex].var2[0])) {
			int num1 = atoi(quadvarlist[curIndex].var1);
			int num2 = atoi(quadvarlist[curIndex].var2);

			quadvarlist[curIndex].var1 = newitoa(num1 - num2);
			quadvarlist[curIndex].var2 = newempty();
			strcpy(quadvarlist[curIndex].op, "assign");
		}
		break;
	case '*':
		if (isdigit(quadvarlist[curIndex].var1[0]) && isdigit(quadvarlist[curIndex].var2[0])) {
			int num1 = atoi(quadvarlist[curIndex].var1);
			int num2 = atoi(quadvarlist[curIndex].var2);

			quadvarlist[curIndex].var1 = newitoa(num1 * num2);
			quadvarlist[curIndex].var2 = newempty();
			strcpy(quadvarlist[curIndex].op, "assign");

			return;
		}
		// ���ǳ�����ǿ����������һ��Ϊ������
		if (isdigit(quadvarlist[curIndex].var1[0])) {

		}
		else if (isdigit(quadvarlist[curIndex].var2[0])) {

		}
		break;
	case '/':
		if (isdigit(quadvarlist[curIndex].var1[0]) && isdigit(quadvarlist[curIndex].var2[0])) {
			int num1 = atoi(quadvarlist[curIndex].var1);
			int num2 = atoi(quadvarlist[curIndex].var2);

			quadvarlist[curIndex].var1 = newitoa(num1 / num2);
			quadvarlist[curIndex].var2 = newempty();
			strcpy(quadvarlist[curIndex].op, "assign");
			return;
		}
		// ���ǳ�����ǿ����������һ��Ϊ������
		if (isdigit(quadvarlist[curIndex].var2[0])) {

		}
		break;
	default:
		break;
	}
}


// �Ż�
void optimize()
{
	while (curIndex < NXQ) {
		constantMergeAndStrengthReduction();
		
		
		
		
		curIndex++;
	}

}






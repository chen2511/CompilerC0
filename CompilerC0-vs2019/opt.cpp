#include "opt.h"
#include "ir.h"

static int curIndex;

bool isTheSameOp(int cur, int tocmp);
int isPower2(int num);
bool isTheSame4VarOp(const char* aimOp, int index);
bool isTheSameBasicBlock(int index);
bool checkVarWhetherChanged(int index);

/* 
�����ϲ��� ���������Ϊ��ֵ
�� + - * / ת��Ϊ ��ֵ
*/
void constantMerge() 
{
	// ���ж������������Ƿ������֣�����Ǽ���
	if (isdigit(quadvarlist[curIndex].var1[0]) && isdigit(quadvarlist[curIndex].var2[0]))
	{
		int num1 = atoi(quadvarlist[curIndex].var1);
		int num2 = atoi(quadvarlist[curIndex].var2);
		switch (quadvarlist[curIndex].op[0]) {
		case '+':
			quadvarlist[curIndex].var1 = newitoa(num1 + num2);
			quadvarlist[curIndex].var2 = newempty();
			strcpy(quadvarlist[curIndex].op, "assign");
			break;
		case '-':
			quadvarlist[curIndex].var1 = newitoa(num1 - num2);
			quadvarlist[curIndex].var2 = newempty();
			strcpy(quadvarlist[curIndex].op, "assign");
			break;
		case '*':
			quadvarlist[curIndex].var1 = newitoa(num1 * num2);
			quadvarlist[curIndex].var2 = newempty();
			strcpy(quadvarlist[curIndex].op, "assign");
			break;
		case '/':
			quadvarlist[curIndex].var1 = newitoa(num1 / num2);
			quadvarlist[curIndex].var2 = newempty();
			strcpy(quadvarlist[curIndex].op, "assign");
			break;
			// ���� + - * / �˳�
		default:
			break;
		}
	}
}

/*
ǿ�������� �˷��ͳ�����Ϊ��λ
*/
void strengthReduction()
{
	switch (quadvarlist[curIndex].op[0]) {
	case '*':
		if (isdigit(quadvarlist[curIndex].var1[0]) && isdigit(quadvarlist[curIndex].var2[0])) {
			return;
		}
		// ���ǳ�����ǿ����������һ��Ϊ������
		if (isdigit(quadvarlist[curIndex].var1[0])) {
			// �ж��Ƿ� 2�Ĵη�
			int exp2 = isPower2(atoi(quadvarlist[curIndex].var1));
			//printf("%d\n", exp2);

			strcpy(quadvarlist[curIndex].op, "sll");
			quadvarlist[curIndex].var1 = quadvarlist[curIndex].var2;

			quadvarlist[curIndex].var2 = newitoa(exp2);

		}
		else if (isdigit(quadvarlist[curIndex].var2[0])) {

			// �ж��Ƿ� 2�Ĵη�
			int exp2 = isPower2(atoi(quadvarlist[curIndex].var2));
			//printf("%d\n", exp2);

			strcpy(quadvarlist[curIndex].op, "sll");
			quadvarlist[curIndex].var2 = newitoa(exp2);
		}
		break;
	case '/':
		if (isdigit(quadvarlist[curIndex].var1[0]) && isdigit(quadvarlist[curIndex].var2[0])) {
			return;
		}
		// ���ǳ�����ǿ����������һ��Ϊ������
		if (isdigit(quadvarlist[curIndex].var2[0])) {
			// �ж��Ƿ� 2�Ĵη�
			int exp2 = isPower2(atoi(quadvarlist[curIndex].var2));
			//printf("%d\n", exp2);

			strcpy(quadvarlist[curIndex].op, "sra");
			quadvarlist[curIndex].var2 = newitoa(exp2);
		}
		break;
	default:
		break;
	}

}

// ɾ�������ӱ��ʽ
// �Ľ�5�����ֻ����飬����������������**��DAG
// �Ľ�6�� �Ż��ĵ���һ���������룬����ȼ۵�һ����������������������Ż���ʹ����ǿ
void subexpressElimination() 
{
	// �Ľ�7���������
	if ( isTheSame4VarOp("+", curIndex) || isTheSame4VarOp("-", curIndex) ||
		 isTheSame4VarOp("*", curIndex) || isTheSame4VarOp("/" ,curIndex) )
	{
		// ��鵱ǰ������
		// �Ľ�8���߼����ɶ��ԸĽ���˼��˵��Ե��
		for (int theSameIndex = curIndex + 1; isTheSameBasicBlock(theSameIndex); theSameIndex++) 
		{
			// ����Ƿ�������Ƿ񱻸ı䣺��ֵ����������֮ǰ������1��2�ͱ���ֵ�ı���һ��
			if (checkVarWhetherChanged(theSameIndex))
			{
				return;
			}
			// ɾ�������ӱ��ʽ
			if (isTheSameOp(curIndex, theSameIndex)) 
			{
				// ��ʼ�滻�����Ѿ�������ı��������µ�
				for (int h = theSameIndex + 1; isTheSameBasicBlock(h); h++)
				{
					if (!strcmp(quadvarlist[h].var1, quadvarlist[theSameIndex].var3)) {
						quadvarlist[h].var1 = quadvarlist[curIndex].var3;
					}
					else if (!strcmp(quadvarlist[h].var2, quadvarlist[theSameIndex].var3)) {
						quadvarlist[h].var2 = quadvarlist[curIndex].var3;
					}
				}

				// ɾ�� ͬop
				strcpy(quadvarlist[theSameIndex].op, "null");
			}
		}
	}
}

// �Ż�
void optimize()
{
	while (curIndex < NXQ) {
		constantMerge();
		
		subexpressElimination();
		
		strengthReduction();

		curIndex++;
	}

}


// ����Ƿ�Ϊͬһ��Ԫʽ op1=op2;var1.1=var1.2&&var2.1==var2.2||����;
bool isTheSameOp(int cur, int tocmp)
{
	if (!strcmp(quadvarlist[cur].op, quadvarlist[tocmp].op)) {
		if (!strcmp(quadvarlist[cur].var1, quadvarlist[tocmp].var1) && !strcmp(quadvarlist[cur].var2, quadvarlist[tocmp].var2)
			|| !strcmp(quadvarlist[cur].var1, quadvarlist[tocmp].var2) && !strcmp(quadvarlist[cur].var2, quadvarlist[tocmp].var1)) {
			return true;
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}


}

// �ж���2�ļ��η�
int isPower2(int num)
{
	// ��2�Ĵη�
	if (1 == num || 0 == num) {
		return 0;
	}
	// num & (num - 1) Ӧ�õ���0
	if (0 == (num & (num - 1))) {
		int cnt = 1;
		while (1) {
			if (num == (1 << cnt)) {
				return cnt;
			}
			else {
				cnt++;
			}
		}
	}
	else {
		return 0;
	}
}

// �ж��Ƿ�ͬһ�� op
// ����Ŀ����Ԫʽ op�� ��Ԫʽ index
bool isTheSame4VarOp(const char* aimOp, int index)
{
	if (strcmp(quadvarlist[index].op, aimOp) == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
		
}

// �жϵ�ǰ��Ԫʽ�Ƿ�ͬһ������
// ��Ҫ˼·�� �ж��Ƿ�����j lab call callret Func Main
bool isTheSameBasicBlock(int index)
{
	if (isTheSame4VarOp("+", index) || isTheSame4VarOp("-", index) || isTheSame4VarOp("*", index) || isTheSame4VarOp("/", index)
		|| isTheSame4VarOp("assign", index))
	{
		return true;
	}
	else
	{
		return false;
	}


	//strcmp(quadvarlist[theSameIndex].op, "+") == 0 || strcmp(quadvarlist[theSameIndex].op, "-") == 0
	//	|| strcmp(quadvarlist[theSameIndex].op, "*") == 0 
	//	|| strcmp(quadvarlist[theSameIndex].op, "/") == 0 || strcmp(quadvarlist[theSameIndex].op, "assign") == 0;
}

// ��� �������Ƿ����¸�ֵ���Ƿ��� true
bool checkVarWhetherChanged(int index)
{
	if (isTheSame4VarOp("assign", index))
	{
		if (!strcmp(quadvarlist[index].var3, quadvarlist[curIndex].var1)
			|| !strcmp(quadvarlist[index].var3, quadvarlist[curIndex].var2)) 
		{
			return true;
		}
	}
	return false;
}

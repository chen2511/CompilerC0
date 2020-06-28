#include "opt.h"
#include "ir.h"

static int curIndex;

bool isTheSameOp(int cur, int tocmp);
int isPower2(int num);
bool isTheSame4VarOp(const char* aimOp, int index);
bool isTheSameBasicBlock(int index);
bool checkVarWhetherChanged(int index);

/* 
常量合并： 常量计算改为赋值
将 + - * / 转化为 赋值
*/
void constantMerge() 
{
	// 先判断两个操作数是否都是数字，如果是继续
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
			// 不是 + - * / 退出
		default:
			break;
		}
	}
}

/*
强度削弱： 乘法和除法改为移位
*/
void strengthReduction()
{
	switch (quadvarlist[curIndex].op[0]) {
	case '*':
		if (isdigit(quadvarlist[curIndex].var1[0]) && isdigit(quadvarlist[curIndex].var2[0])) {
			return;
		}
		// 不是常数：强度削弱（有一个为常数）
		if (isdigit(quadvarlist[curIndex].var1[0])) {
			// 判断是否 2的次方
			int exp2 = isPower2(atoi(quadvarlist[curIndex].var1));
			//printf("%d\n", exp2);

			strcpy(quadvarlist[curIndex].op, "sll");
			quadvarlist[curIndex].var1 = quadvarlist[curIndex].var2;

			quadvarlist[curIndex].var2 = newitoa(exp2);

		}
		else if (isdigit(quadvarlist[curIndex].var2[0])) {

			// 判断是否 2的次方
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
		// 不是常数：强度削弱（有一个为常数）
		if (isdigit(quadvarlist[curIndex].var2[0])) {
			// 判断是否 2的次方
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

// 删除公共子表达式
// 改进5：划分基本块，数据流分析，常数**，DAG
// 改进6： 优化文档，一个方面深入，语义等价第一；考虑所有情况；真正的优化，使用性强
void subexpressElimination() 
{
	// 改进7：最多三层
	if ( isTheSame4VarOp("+", curIndex) || isTheSame4VarOp("-", curIndex) ||
		 isTheSame4VarOp("*", curIndex) || isTheSame4VarOp("/" ,curIndex) )
	{
		// 检查当前基本块
		// 改进8：逻辑、可读性改进；思考说清缘由
		for (int theSameIndex = curIndex + 1; isTheSameBasicBlock(theSameIndex); theSameIndex++) 
		{
			// 检查是否操作数是否被改变：赋值操作，并且之前操作数1、2和被赋值的变量一样
			if (checkVarWhetherChanged(theSameIndex))
			{
				return;
			}
			// 删除公共子表达式
			if (isTheSameOp(curIndex, theSameIndex)) 
			{
				// 开始替换：用已经计算过的变量代替新的
				for (int h = theSameIndex + 1; isTheSameBasicBlock(h); h++)
				{
					if (!strcmp(quadvarlist[h].var1, quadvarlist[theSameIndex].var3)) {
						quadvarlist[h].var1 = quadvarlist[curIndex].var3;
					}
					else if (!strcmp(quadvarlist[h].var2, quadvarlist[theSameIndex].var3)) {
						quadvarlist[h].var2 = quadvarlist[curIndex].var3;
					}
				}

				// 删除 同op
				strcpy(quadvarlist[theSameIndex].op, "null");
			}
		}
	}
}

// 优化
void optimize()
{
	while (curIndex < NXQ) {
		constantMerge();
		
		subexpressElimination();
		
		strengthReduction();

		curIndex++;
	}

}


// 检查是否为同一四元式 op1=op2;var1.1=var1.2&&var2.1==var2.2||……;
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

// 判断是2的几次方
int isPower2(int num)
{
	// 是2的次方
	if (1 == num || 0 == num) {
		return 0;
	}
	// num & (num - 1) 应该等于0
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

// 判断是否同一个 op
// 输入目标四元式 op， 四元式 index
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

// 判断当前四元式是否同一基本块
// 主要思路： 判断是否遇到j lab call callret Func Main
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

// 检查 操作数是否被重新赋值，是返回 true
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

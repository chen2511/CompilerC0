#include "opt.h"
#include "ir.h"

static int curIndex;

bool isTheSameOp(int cur, int tocmp);
int isPower2(int num);

/* 
常量合并： 常量计算改为赋值

*/
void constantMerge() 
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
		break;
	default:
		break;
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
			int exp2 = isPower2(atoi(quadvarlist[curIndex].var1));
			//printf("%d\n", exp2);

			strcpy(quadvarlist[curIndex].op, "sll");
			quadvarlist[curIndex].var1 = quadvarlist[curIndex].var2;

			quadvarlist[curIndex].var2 = newitoa(exp2);

		}
		else if (isdigit(quadvarlist[curIndex].var2[0])) {
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
void subexpressElimination() 
{
	if (strcmp(quadvarlist[curIndex].op, "+") == 0 || strcmp(quadvarlist[curIndex].op, "-") == 0 || strcmp(quadvarlist[curIndex].op, "*") == 0
		|| strcmp(quadvarlist[curIndex].op, "/") == 0) 
	{
		// 检查当前基本块
		for (int theSameIndex = curIndex + 1; strcmp(quadvarlist[theSameIndex].op, "+") == 0 || strcmp(quadvarlist[theSameIndex].op, "-") == 0
			|| strcmp(quadvarlist[theSameIndex].op, "*") == 0 || strcmp(quadvarlist[theSameIndex].op, "/") == 0 || strcmp(quadvarlist[theSameIndex].op, "assign") == 0;
			theSameIndex++
			) 
		{
			// 检查是否操作数是否被改变
			if (!strcmp(quadvarlist[theSameIndex].op, "assign"))
			{
				if (!strcmp(quadvarlist[theSameIndex].var3, quadvarlist[curIndex].var1)
					|| !strcmp(quadvarlist[theSameIndex].var3, quadvarlist[curIndex].var2)) {
					return;
				}
			}
			// 删除公共子表达式
			if (isTheSameOp(curIndex, theSameIndex)) 
			{
				// 开始替换
				for (int h = theSameIndex + 1; strcmp(quadvarlist[h].op, "+") == 0 || strcmp(quadvarlist[h].op, "-") == 0
					|| strcmp(quadvarlist[h].op, "*") == 0 || strcmp(quadvarlist[h].op, "/") == 0 || strcmp(quadvarlist[h].op, "assign") == 0;
					h++
					)
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
				// 这里就没有修改多余的临时变量
				//quadvarlist[theSameIndex].var3 = (char *)" ";
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


#include "opt.h"
#include "ir.h"

static int curIndex;




/* 
常量合并： 常量计算改为赋值
强度削弱： 乘法和除法改为移位
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
		// 不是常数：强度削弱（有一个为常数）
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
		// 不是常数：强度削弱（有一个为常数）
		if (isdigit(quadvarlist[curIndex].var2[0])) {

		}
		break;
	default:
		break;
	}
}


// 优化
void optimize()
{
	while (curIndex < NXQ) {
		constantMergeAndStrengthReduction();
		
		
		
		
		curIndex++;
	}

}






#include "global.h"
#include "parser.h"
#include "scan.h"

using namespace std;

static bool b_functionDeclaration = false;
static bool b_mainFunction = false;
static int flashBackIndex;

static void match(TokenType expectToken);

//为每一个非终结符创建一个函数
void program();

void constDeclaration();
void constDefine();
void varDeclaration();
void varDefine();
void typeID();
void functionDefinitionWithReturn();
void DeclarationHead();
void functionDefinitionWithoutReturn();
void paraTable();
void complexStatement();
void mainFunction();

void signedNum();

void statementSequence();
void statement();
void assignStatement();
void ifStatement();
void loopStatement();
void callWithReturn();
void callWithoutReturn();
void valueParaTable();
void readStatement();
void writeStatement();
void returnStatement();

void exp();
void term();
void factor();
void boolExp();
void boolTerm();
void boolFactor();
void conditionFactor();

// 分析开始，调用程序
void parser() {
	getNextToken();
	//语法分析开始
	//测试时：单独调用其他过程
	program();

	

	if (!EOF_flag) {
		cout << "error: not EOF" << endl;
	}
}

// 匹配 期待的 token；否则报错
// 读取下一个token
static void match(TokenType expectToken) {
	if (expectToken == g_token.opType) {
		getNextToken();
	}
	else {
		printf("match error in line %d :\t\texpect Token %d, but %d value: %s \n", g_lineNumber, expectToken, g_token.opType, g_token.value);
	}
}

/*
13.<程序> ::= ［<常量说明>］［<变量说明>］{<有返回值函数定义>|<无返回值函数定义>}<主函数>
这里有很多的公因子，但是为了文法简单，好理解；就在程序上实现，没有提取
*/
void program() {
	// ［<常量说明>］
	if (TokenType::CONST == g_token.opType) {
		constDeclaration();
	}


	// ［<变量说明>］

	// 可选：如果不是 int 和 char 会直接跳过（没有变量定义），但还需要进一步判断有返回值函数定义
	if (TokenType::INT == g_token.opType || TokenType::CHAR == g_token.opType) {
		flashBackIndex = g_lexBegin;
		//因为token获取的值完全依赖于两个指针，将前一个指针保存即可
		//缓存token开始位置，比如识别到int的开始位置，因为末尾要再取一次token
		getNextToken();
		getNextToken();
		//预读两个token，进一步判断
		if (TokenType::LPARENTHES == g_token.opType) {	//有返回值函数定义，左小括号
			b_functionDeclaration = true;
		}
		else {											//变量说明				
			if (TokenType::COMMA == g_token.opType) {			// 逗号

			}
			else if(TokenType::SEMICOLON == g_token.opType) { // 分号

			}
			else if (TokenType::LBRACKET == g_token.opType) {	// 左中括号

			}
			else {											//错误								
				printf("error in program() :in line %d ,tokenType %d value: %s \n", g_lineNumber, g_token.opType, g_token.value);
			}

		}
		// 回溯
		g_forward = flashBackIndex;
		getNextToken();

		if (b_functionDeclaration) {
			//是函数定义：什么都不做，跳过，进入下一部分
		}
		else {
			varDeclaration();			// 进入变量说明
		}	

	}

	// {<有返回值函数定义>|<无返回值函数定义>}
	// 前面的必须按顺序来，错过了就不能定义了。这里可以随机顺序
	while ((TokenType::INT == g_token.opType || TokenType::CHAR == g_token.opType || TokenType::VOID == g_token.opType) && !b_mainFunction) {
		if (TokenType::INT == g_token.opType || TokenType::CHAR == g_token.opType) { //有返回值的定义
			functionDefinitionWithReturn();
		}
		else if (TokenType::VOID == g_token.opType) {		// 有可能是无返回值函数定义，也有可能是 主函数
			flashBackIndex = g_lexBegin;
			getNextToken();

			if (TokenType::MAIN == g_token.opType) {
				b_mainFunction = true;
			}
			// 回溯
			g_forward = flashBackIndex;
			getNextToken();

			// 根据是否主函数标志 选择
			if (b_mainFunction) {
				// b_mainFunction 标志已改变，跳过该循环，进入主函数
			}
			else {
				functionDefinitionWithoutReturn();
			}
		}
	}

	// <主函数>
	mainFunction();

}

/*
14.<常量说明> ::= const<常量定义>;{ const<常量定义>;}
*/
void constDeclaration() {
	match(TokenType::CONST);
	constDefine();
	match(TokenType::SEMICOLON);

	// 多个const 行
	while (TokenType::CONST == g_token.opType) {
		match(TokenType::CONST);
		constDefine();
		match(TokenType::SEMICOLON);
	}
}

//15. < 常量定义 > :: = int<标识符>＝<整数>{, <标识符>＝<整数>} | char<标识符>＝<字符>{, <标识符>＝<字符>}
void constDefine() {
	//int
	if (TokenType::INT == g_token.opType) {
		match(TokenType::INT);
		match(TokenType::IDEN);
		match(TokenType::ASSIGN);
		signedNum();
		// {, <标识符>＝<整数>}
		while (TokenType::COMMA == g_token.opType) {
			match(TokenType::COMMA);
			match(TokenType::IDEN);
			match(TokenType::ASSIGN);
			signedNum();
		}
	}
	//char
	else if (TokenType::CHAR == g_token.opType) {
		match(TokenType::CHAR);
		match(TokenType::IDEN);
		match(TokenType::ASSIGN);
		match(TokenType::LETTER);
		//{, <标识符>＝<字符>}
		while (TokenType::COMMA == g_token.opType) {
			match(TokenType::COMMA);
			match(TokenType::IDEN);
			match(TokenType::ASSIGN);
			match(TokenType::LETTER);
		}
	}
	else {
		printf("error in constDefine() :in line %d ,tokenType %d value: %s \n", g_lineNumber, g_token.opType, g_token.value);
	}
}

//9.<整数> ::= ［＋｜－］<无符号整数>
//只有定义时会遇到，其他时候单独处理
void signedNum() {
	//没写完善。。。。具体语义 后续继续完善
	int sign = 1;
	if (TokenType::MINU == g_token.opType) {		// -
		sign = -1;
		match(TokenType::MINU);
		//计划进行单独处理，即 * -1
	}
	else if (TokenType::PLUS == g_token.opType) {
		match(TokenType::PLUS);
		//不用处理
	}
	else if (TokenType::NUM != g_token.opType) {
		printf("error in signedNum() :in line %d ,tokenType %d value: %s \n", g_lineNumber, g_token.opType, g_token.value);
		return;
	}
	// 如果是负的，应该在这里进行一定处理
	match(TokenType::NUM);

}

//16.<变量说明> ::= <变量定义>;{<变量定义>;}
//这里有可能会和后面的有返回值函数定义冲突
void varDeclaration() {
	varDefine();
	match(TokenType::SEMICOLON);

	// 变量定义判断 重复；一旦开始函数定义，立即退出，不能继续变量定义
	//循环终止条件：不是int、char （无返定义和主函数）或者 开始函数定义
	while ((TokenType::INT == g_token.opType || TokenType::CHAR == g_token.opType) && !b_functionDeclaration) {
		flashBackIndex = g_lexBegin;
		
		getNextToken();
		getNextToken();
		//预读两个token，进一步判断
		if (TokenType::LPARENTHES == g_token.opType) {	//有返回值函数定义，左小括号
			b_functionDeclaration = true;
		}
		else {											//变量说明				
			if (TokenType::COMMA == g_token.opType) {			// 逗号

			}
			else if (TokenType::SEMICOLON == g_token.opType) { // 分号

			}
			else if (TokenType::LBRACKET == g_token.opType) {	// 左中括号

			}
			else {											//错误								
				printf("error in varDeclaration() :in line %d ,tokenType %d value: %s \n", g_lineNumber, g_token.opType, g_token.value);
			}

		}
		// 回溯
		g_forward = flashBackIndex;
		getNextToken();


		if (b_functionDeclaration) {							// 有返回值函数定义：退出
			return;
		}
		else {
			varDefine();			// 进入变量定义
			match(TokenType::SEMICOLON);
		}
	}
	


}

//17.<变量定义> ::= <类型标识符><标识符>[‘[’<无符号整数>‘]’] {,<标识符>[‘[’<无符号整数>‘]’]}
void varDefine() {
	typeID();
	match(TokenType::IDEN);

	if (TokenType::LBRACKET == g_token.opType) {			//数组定义
		match(TokenType::LBRACKET);
		if (TokenType::NUM == g_token.opType) {
			if ('0' == g_token.value[0])
				printf("error in varDefine() :in line %d ,tokenType %d value: %s ;array size is 0 \n", g_lineNumber, g_token.opType, g_token.value);
				


		}
		match(TokenType::NUM);
		match(TokenType::RBRACKET);
	}
	while (TokenType::COMMA == g_token.opType) {			//重复定义
		match(TokenType::COMMA);
		match(TokenType::IDEN);
		if (TokenType::LBRACKET == g_token.opType) {
			match(TokenType::LBRACKET);
			if (TokenType::NUM == g_token.opType) {
				if ("0" == g_token.value)
					printf("error in varDefine() :in line %d ,tokenType %d value: %s \n", g_lineNumber, g_token.opType, g_token.value);
			}
			match(TokenType::NUM);
			match(TokenType::RBRACKET);
		}
	}
}

//18.<类型标识符> ::= int | char
void typeID() {
	if (TokenType::INT == g_token.opType) {
		match(TokenType::INT);
	}
	else if (TokenType::CHAR == g_token.opType) {
		match(TokenType::CHAR);
	}
	else {
		printf("error in typeID() :in line %d ,tokenType %d value: %s \n", g_lineNumber, g_token.opType, g_token.value);
	}
}

//19.<有返回值函数定义> ::= <声明头部>‘(’<参数表>‘)’ ‘{’<复合语句>‘}’
void functionDefinitionWithReturn() {
	DeclarationHead();
	match(TokenType::LPARENTHES);
	paraTable();
	match(TokenType::RPARENTHES);
	match(TokenType::LBRACE);

	complexStatement();

	match(TokenType::RBRACE);
}

//20.<声明头部> ::= int<标识符> | char<标识符>
void DeclarationHead() {
	if (TokenType::INT == g_token.opType) {
		match(TokenType::INT);
	}
	else if (TokenType::CHAR == g_token.opType) {
		match(TokenType::CHAR);
	}
	else {
		printf("error in DeclarationHead() :in line %d ,tokenType %d value: %s \n", g_lineNumber, g_token.opType, g_token.value);
	}
	match(TokenType::IDEN);
}

//21.<无返回值函数定义> ::= void<标识符>‘(’<参数表>‘)’ ‘{’<复合语句>‘}’
void functionDefinitionWithoutReturn() {
	match(TokenType::VOID);
	match(TokenType::IDEN);
	match(TokenType::LPARENTHES);
	paraTable();
	match(TokenType::RPARENTHES);
	match(TokenType::LBRACE);

	complexStatement();

	match(TokenType::RBRACE);
}

//22.<参数表> ::= <类型标识符><标识符>{,<类型标识符><标识符>}| <空>
void paraTable() {
	// <类型标识符><标识符>
	if (TokenType::INT == g_token.opType || TokenType::CHAR == g_token.opType) {
		typeID();
		match(TokenType::IDEN);
		// {,<类型标识符><标识符>}
		while (TokenType::COMMA == g_token.opType) {
			match(TokenType::COMMA);
			typeID();
			match(TokenType::IDEN);
		}
	}
	// <空>
	;
}

////////////////////////////16'.__<变量说明> ::= <变量定义>;{<变量定义>;}
//这里是单纯的 <变量说明>
inline void __varDeclaration() {
	// <变量定义>;
	varDefine();
	match(TokenType::SEMICOLON);
	// {<变量定义>;}
	while (TokenType::INT == g_token.opType || TokenType::CHAR == g_token.opType) {
		varDefine();
		match(TokenType::SEMICOLON);
	}
}

//23.<复合语句> ::= ［<常量说明>］［<变量说明>］<语句列>
void complexStatement() {
	// ［<常量说明>］
	if (TokenType::CONST == g_token.opType) {
		constDeclaration();
	}
	// ［<变量说明>］
	if (TokenType::INT == g_token.opType || TokenType::CHAR == g_token.opType) {
		// 这里没有直接调用"varDeclaration"函数，因为函数中有一些多余的过程 来识别“函数定义”
		__varDeclaration();
	}

	// <语句列>
	statementSequence();

}

//24.<主函数> ::= void main ‘(’ ‘)’ ‘{’<复合语句>‘}’
void mainFunction() {
	match(TokenType::VOID);
	match(TokenType::MAIN);
	match(TokenType::LPARENTHES);
	match(TokenType::RPARENTHES);
	match(TokenType::LBRACE);

	complexStatement();

	match(TokenType::RBRACE);
}

/********************* 以上是语法分析第二次单元测试的内容	*************************/

//25.<语句列> ::= ｛<语句>｝
void statementSequence() {

}

/*
26. < 语句 > :: = <条件语句>｜<循环语句>｜‘{ ’<语句列>‘ }’｜<有返回值的函数调用语句>; ｜<无返回值的函数调用语句>; 
		｜<赋值语句>; ｜<读语句>; ｜<写语句>; ｜<空>; ｜<返回语句>;

!!!!!!注意：这里的  “<空>; ” 和 空语句是不一样的，只有个分号
*/
void statement() {

}


//27. < 赋值语句 > :: = <标识符>[‘[’<算术表达式>‘]’]＝<算术表达式>
void assignStatement() {

}


//28. < 条件语句 > :: = if ‘(’<布尔表达式>‘)’<语句>［else<语句>］
void ifStatement() {

}

/*
29. < 循环语句 > :: = while ‘(’<布尔表达式>‘)’<语句> | for‘(’<赋值语句>; <布尔表达式>; <赋值语句>‘)’<语句>

for循环中的三个表达式：初始化表达式、循环变量测试表达式、循环变量修正表达式
*/
void loopStatement() {

}


//30. < 有返回值的函数调用语句 > :: = <标识符>‘(’<值参数表>‘)’
void callWithReturn() {

}


//31. < 无返回值的函数调用语句 > :: = <标识符>‘(’<值参数表>‘)’
void callWithoutReturn() {

}


/*
32. < 值参数表 > :: = <算术表达式>{ ,<算术表达式> }｜<空>

>>>>>>>> 空语句：这里也有一处空语句，但follow集只有')'一个元素
*/
void valueParaTable() {

}


//33. < 读语句 > :: = scanf ‘(’<标识符>{, <标识符>}‘)’
void readStatement() {

}


/*
34. < 写语句 > :: = printf ‘(’<字符串>, <算术表达式>‘)’ | printf ‘(’<字符串>‘)’ | printf ‘(’<算术表达式>‘)’

定义写语句是以printf为起始的，后接圆括号括起来的字符串或表达式或者两者都有，若两者都存在，则字符串在先，以逗号隔开。
*/
void writeStatement() {

}

//35. < 返回语句 > :: = return[‘(’<算术表达式>‘)’]
void returnStatement() {

}

//36. < 算术表达式 > :: = ［＋｜－］<项>{ <加法运算符><项> }
void exp() {

}


//37. < 项 > :: = <因子>{ <乘法运算符><因子> }
void term() {

}

//38. < 因子 > :: = <标识符>｜<标识符>‘[’<算术表达式>‘]’｜<整数> | <字符>｜<有返回值函数调用语句> | ‘(’<算术表达式>‘)’
void factor() {

}

//39. < 布尔表达式 > :: = <布尔项>{ ‘ || ’ <布尔项> }
void boolExp() {

}

//40. < 布尔项 > :: = <布因子>{ && <布因子> }
void boolTerm() {

}


//41. < 布因子 > :: = false | true | !<布因子> | ‘(‘<布尔表达式>’)’ | <条件因子>[<条件运算符> <条件因子>]
void boolFactor() {

}


//42. < 条件因子 > :: = <标识符>[‘[’<算术表达式>‘]’]｜ <整数> | <字符> | <有返回值函数调用语句>
void conditionFactor() {

}

/*
事实上，这里<条件因子>不像（算术）表达式中一样，这里不支持嵌套，不支持’()’，因为布尔表达式设计起来符合嵌套，
	主要是括号不好处理；暂时这么设计，然后如果真要表达式的话，可以在前面先计算赋值给一个变量
*/





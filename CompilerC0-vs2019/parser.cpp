/*
	递归下降子程序实现语法分析，并生成抽象语法树
*/

#include "global.h"
#include "parser.h"
#include "scan.h"
#include "ast.h"

using namespace std;

static bool b_functionDeclaration = false;
static bool b_mainFunction = false;
static int flashBackIndex;
static int flashBackLineNum;

static bool match(TokenType expectToken);

//为每一个非终结符创建一个函数
TreeNode* program();

TreeNode* constDeclaration();
TreeNode* constDefine();
TreeNode* varDeclaration();
TreeNode* __varDeclaration();
TreeNode* varDefine();
Type typeID();
TreeNode* functionDefinitionWithReturn();
TreeNode* DeclarationHead();
TreeNode* functionDefinitionWithoutReturn();
void paraTable(FuncInfo * finfo);
TreeNode* complexStatement();
TreeNode* mainFunction();

TreeNode* signedNum();

TreeNode* statementSequence();
TreeNode* statement();
TreeNode* assignStatement();
TreeNode* ifStatement();
TreeNode* whileLoopStatement();
TreeNode* forLoopStatement();
TreeNode* callWithReturn();
TreeNode* callWithoutReturn();
TreeNode* valueParaTable();
TreeNode* scanfStatement();
TreeNode* printfStatement();
TreeNode* returnStatement();

TreeNode* exp();
TreeNode* term();
TreeNode* factor();
TreeNode* boolExp();
TreeNode* boolTerm();
TreeNode* boolFactor();
TreeNode* conditionFactor();

// 分析开始，调用程序
TreeNode* parser() {
	TreeNode* synaxTree;

	getNextToken();
	//语法分析开始
	//测试时：单独调用其他过程
	synaxTree = program();

	if (!EOF_flag) {
		cout << "error: not EOF" << endl;
	}

	if (PRINT_AST_TOFILE) {
		AST_File = fopen("./Output/Ast.txt", "w+");
		printAST(synaxTree);
	}

	return synaxTree;
}

// 匹配 期待的 token；否则报错
// 读取下一个token
static bool match(TokenType expectToken) {
	if (expectToken == g_token.opType) {	// 与预期相同，跳过token， true
		getNextToken();
		return true;
	}
	else {								// 提示错误，但在这里不跳读
		printf("match error in line %d :\t\texpect Token %d, but %d value: %s \n", g_lineNumber, expectToken, g_token.opType, g_token.value);
		return false;
	}
}

/*
13.<程序> ::= ［<常量说明>］［<变量说明>］{<有返回值函数定义>|<无返回值函数定义>}<主函数>
这里有很多的公因子，但是为了文法简单，好理解；就在程序上实现，没有提取
*/
TreeNode* program() {
	TreeNode* t = NULL;
	TreeNode* p = NULL;									// 返回t， p中间处理
	
	

	// ［<常量说明>］
	if (TokenType::CONST == g_token.opType) {
		t = constDeclaration();
		p = t;
	}


	// ［<变量说明>］

	// 可选：如果不是 int 和 char 会直接跳过（没有变量定义），但还需要进一步判断有返回值函数定义
	if (TokenType::INT == g_token.opType || TokenType::CHAR == g_token.opType) {
		flashBackIndex = g_lexBegin;
		flashBackLineNum = g_lineNumber;

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
			else if (TokenType::SEMICOLON == g_token.opType) { // 分号

			}
			else if (TokenType::LBRACKET == g_token.opType) {	// 左中括号

			}
			else {											//错误								
				printf("error in program() :in line %d ,tokenType %d value: %s \n", g_lineNumber, g_token.opType, g_token.value);
			}

		}
		// 回溯
		g_forward = flashBackIndex;
		g_lineNumber = flashBackLineNum;
		getNextToken();

		if (b_functionDeclaration) {
			//是函数定义：什么都不做，跳过，进入下一部分
		}
		else {
			// 进入变量说明
			if (NULL == t) {
				t = varDeclaration();
				p = t;
			}
			else {
				TreeNode* q = varDeclaration();
				p->sibling = q;
				p = q;
			}
		}

	}

	// {<有返回值函数定义>|<无返回值函数定义>}
	// 前面的必须按顺序来，错过了就不能定义了。这里可以随机顺序
	while ((TokenType::INT == g_token.opType || TokenType::CHAR == g_token.opType || TokenType::VOID == g_token.opType) && !b_mainFunction) {
		if (TokenType::INT == g_token.opType || TokenType::CHAR == g_token.opType) { //有返回值的定义
			if (NULL == t) {
				t = functionDefinitionWithReturn();
				p = t;
			}
			else {
				TreeNode* q = functionDefinitionWithReturn();
				p->sibling = q;
				p = q;
			}
		}
		else if (TokenType::VOID == g_token.opType) {		// 有可能是无返回值函数定义，也有可能是 主函数
			flashBackIndex = g_lexBegin;
			flashBackLineNum = g_lineNumber;
			getNextToken();

			if (TokenType::MAIN == g_token.opType) {
				b_mainFunction = true;
			}
			// 回溯
			g_forward = flashBackIndex;
			g_lineNumber = flashBackLineNum;
			getNextToken();

			// 根据是否主函数标志 选择
			if (b_mainFunction) {
				// b_mainFunction 标志已改变，跳过该循环，进入主函数
			}
			else {
				if (NULL == t) {
					t = functionDefinitionWithoutReturn();
					p = t;
				}
				else {
					TreeNode* q = functionDefinitionWithoutReturn();
					p->sibling = q;
					p = q;
				}
			}
		}
	}

	// <主函数>
	if (NULL == t) {
		t = mainFunction();
		p = t;
	}
	else {
		TreeNode* q = mainFunction();
		p->sibling = q;
		p = q;
	}

	return t;
}

/*
14.<常量说明> ::= const<常量定义>;{ const<常量定义>;}
*/
TreeNode* constDeclaration() {
	match(TokenType::CONST);

	TreeNode* t = newDecNode(DecKind::Const_DecK);
	TreeNode* p = constDefine();
	t->child[0] = p;

	match(TokenType::SEMICOLON);

	// 多个const 行
	while (TokenType::CONST == g_token.opType) {
		match(TokenType::CONST);

		TreeNode* q = constDefine();
		p->sibling = q;
		p = q;

		match(TokenType::SEMICOLON);
	}

	return t;
}

//15. < 常量定义 > :: = int<标识符>＝<整数>{, <标识符>＝<整数>} | char<标识符>＝<字符>{, <标识符>＝<字符>}
TreeNode* constDefine() {
	TreeNode* t = newDecNode(DecKind::Const_DefK);					// 常量定义节点
	//int
	if (TokenType::INT == g_token.opType) {								// 定义列表节点
		t->type = Type::T_INTEGER;
		TreeNode* p = newDecNode(DecKind::Const_DefK);
		t->child[0] = p;
		match(TokenType::INT);
		
		// 保存标识符
		p->attr.name = copyString(g_token.value);
		match(TokenType::IDEN);
		
		match(TokenType::ASSIGN);
		
		//保存整形数据
		p->child[0] = signedNum();
		// {, <标识符>＝<整数>}
		while (TokenType::COMMA == g_token.opType) {
			match(TokenType::COMMA);

			TreeNode* q = newDecNode(DecKind::Const_DefK);
			p->sibling = q;
			q->attr.name = copyString(g_token.value);
			match(TokenType::IDEN);

			match(TokenType::ASSIGN);
			
			q->child[0] = signedNum();
			p = q;
		}
	}
	//char
	else if (TokenType::CHAR == g_token.opType) {
		t->type = Type::T_CHAR;					// 常量定义 类型
		// 下面是 常量列表
		TreeNode* p = newDecNode(DecKind::Const_DefK);
		t->child[0] = p;
		match(TokenType::CHAR);

		// 保存标识符
		p->attr.name = copyString(g_token.value);
		match(TokenType::IDEN);

		match(TokenType::ASSIGN);

		p->child[0] = newDecNode(DecKind::Const_DefK);
		//保存字符数据
		
		if (g_token.opType == TokenType::LETTER) {
			p->child[0]->attr.cval = g_token.value[0];
			match(TokenType::LETTER);
		}
		else {
			int a = atoi(g_token.value);
			a += 256;
			a = (unsigned char)(a);
			p->child[0]->attr.cval = (unsigned char)(a);
			match(TokenType::NUM);
		}

		
		
		//{, <标识符>＝<字符>}
		while (TokenType::COMMA == g_token.opType) {
			match(TokenType::COMMA);
			TreeNode* q = newDecNode(DecKind::Const_DefK);
			p->sibling = q;
			q->attr.name = copyString(g_token.value);

			match(TokenType::IDEN);
			match(TokenType::ASSIGN);

			q->child[0] = newDecNode(DecKind::Const_DefK);
			//保存字符数据
			//q->child[0]->attr.cval = g_token.value[0];
			//match(TokenType::LETTER);

			if (g_token.opType == TokenType::LETTER) {
				q->child[0]->attr.cval = g_token.value[0];
				match(TokenType::LETTER);
			}
			else {
				int a = atoi(g_token.value);
				a += 256;
				a = (unsigned char)(a);
				q->child[0]->attr.cval = (unsigned char)(a);
				match(TokenType::NUM);
			}

			p = q;
		}
	}
	else {
		printf("error in constDefine() :in line %d ,tokenType %d value: %s \n", g_lineNumber, g_token.opType, g_token.value);
	}

	return t;
}

//9.<整数> ::= ［＋｜－］<无符号整数>
//只有定义时会遇到，其他时候单独处理
TreeNode* signedNum() {
	
	int sign = 1;
	if (TokenType::MINU == g_token.opType) {		// -
		sign = -1;
		match(TokenType::MINU);
	}
	else if (TokenType::PLUS == g_token.opType) {
		match(TokenType::PLUS);
	}
	else if (TokenType::NUM != g_token.opType) {
		printf("error in signedNum() :in line %d ,tokenType %d value: %s \n", g_lineNumber, g_token.opType, g_token.value);
		return nullptr;
	}
	// 如果是负的，应该在这里进行一定处理
	TreeNode* t = newDecNode(DecKind::Const_DefK);
	int valoftoken = atoi(g_token.value);
	valoftoken *= sign;
	t->attr.val = valoftoken;
	match(TokenType::NUM);

	return t;
}

//16.<变量说明> ::= <变量定义>;{<变量定义>;}
//这里有可能会和后面的有返回值函数定义冲突
TreeNode* varDeclaration() {
	TreeNode* t = newDecNode(DecKind::Var_DecK);

	TreeNode* p = varDefine();
	t->child[0] = p;

	match(TokenType::SEMICOLON);

	// 变量定义判断 重复；一旦开始函数定义，立即退出，不能继续变量定义
	//循环终止条件：不是int、char （无返定义和主函数）或者 开始函数定义
	while ((TokenType::INT == g_token.opType || TokenType::CHAR == g_token.opType) && !b_functionDeclaration) {
		flashBackIndex = g_lexBegin;
		flashBackLineNum = g_lineNumber;

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
		g_lineNumber = flashBackLineNum;

		getNextToken();


		if (b_functionDeclaration) {							// 有返回值函数定义：退出
			// bug修复：之前声明之后是一个void函数，退出，不走这个出口
			//return nullptr;
			return t;
		}
		else {
			// 进入变量定义
			TreeNode* q = varDefine();
			p->sibling = q;
			p = q;
			match(TokenType::SEMICOLON);
		}
	}

	return t;
}

//17.<变量定义> ::= <类型标识符><标识符>[‘[’<无符号整数>‘]’] {,<标识符>[‘[’<无符号整数>‘]’]}
TreeNode* varDefine() {
	TreeNode* t = newDecNode(DecKind::Var_DefK);

	Type _vartype = typeID();
	t->type = _vartype;

	TreeNode* p = newDecNode(DecKind::Var_DefK);
	t->child[0] = p;

	p->attr.name = copyString(g_token.value);
	match(TokenType::IDEN);

	if (TokenType::LBRACKET == g_token.opType) {			//数组定义
		match(TokenType::LBRACKET);
		// 数组大小
		if (TokenType::NUM == g_token.opType) {
			p->vec = atoi(g_token.value);
			if ('0' == g_token.value[0])
				printf("error in varDefine() :in line %d ,tokenType %d value: %s ;array size is 0 \n", g_lineNumber, g_token.opType, g_token.value);
		}
		match(TokenType::NUM);
		match(TokenType::RBRACKET);
	}
	while (TokenType::COMMA == g_token.opType) {			//重复定义
		match(TokenType::COMMA);

		TreeNode* q = newDecNode(DecKind::Var_DefK);
		q->attr.name = copyString(g_token.value);

		match(TokenType::IDEN);
		if (TokenType::LBRACKET == g_token.opType) {
			match(TokenType::LBRACKET);
			if (TokenType::NUM == g_token.opType) {
				q->vec = atoi(g_token.value);
				if ("0" == g_token.value)
					printf("error in varDefine() :in line %d ,tokenType %d value: %s \n", g_lineNumber, g_token.opType, g_token.value);
			}
			match(TokenType::NUM);
			match(TokenType::RBRACKET);
		}
		p->sibling = q;
		p = q;
	}

	return t;
}

//18.<类型标识符> ::= int | char
Type typeID() {
	if (TokenType::INT == g_token.opType) {
		match(TokenType::INT);
		return Type::T_INTEGER;
	}
	else if (TokenType::CHAR == g_token.opType) {
		match(TokenType::CHAR);
		return Type::T_CHAR;
	}
	else {
		printf("error in typeID() :in line %d ,tokenType %d value: %s \n", g_lineNumber, g_token.opType, g_token.value);
	}
}

//19.<有返回值函数定义> ::= <声明头部>‘(’<参数表>‘)’ ‘{’<复合语句>‘}’
TreeNode* functionDefinitionWithReturn() {
	//DeclarationHead(); 
	// 方便处理，并不使用
	TreeNode* t = newDecNode(DecKind::Func_DecK);

	Type rettype = typeID();

	t->attr.name = copyString(g_token.value);
	t->pfinfo = new FuncInfo;
	t->pfinfo->rettype = rettype;
	match(TokenType::IDEN);

	match(TokenType::LPARENTHES);
	paraTable(t->pfinfo);
	match(TokenType::RPARENTHES);
	match(TokenType::LBRACE);

	t->child[0] = complexStatement();

	match(TokenType::RBRACE);

	return t;
}

//20.<声明头部> ::= int<标识符> | char<标识符>
// 方便处理，并不使用
TreeNode* DeclarationHead() {
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

	return nullptr;
}

//21.<无返回值函数定义> ::= void<标识符>‘(’<参数表>‘)’ ‘{’<复合语句>‘}’
TreeNode* functionDefinitionWithoutReturn() {
	TreeNode* t = newDecNode(DecKind::Func_DecK);

	match(TokenType::VOID);
	t->pfinfo = new FuncInfo;
	t->pfinfo->rettype = Type::T_VOID;

	t->attr.name = copyString(g_token.value);
	match(TokenType::IDEN);
	match(TokenType::LPARENTHES);
	paraTable(t->pfinfo);
	match(TokenType::RPARENTHES);
	match(TokenType::LBRACE);

	t->child[0] = complexStatement();

	match(TokenType::RBRACE);

	return t;
}

//22.<参数表> ::= <类型标识符><标识符>{,<类型标识符><标识符>}| <空>
void paraTable(FuncInfo* finfo) {
	// <类型标识符><标识符>
	int cnt = 0;
	if (TokenType::INT == g_token.opType || TokenType::CHAR == g_token.opType) {
		Type _t = typeID();
		finfo->paratable[cnt].ptype = _t;
		finfo->paratable[cnt++].pname = copyString(g_token.value);
		match(TokenType::IDEN);
		// {,<类型标识符><标识符>}
		while (TokenType::COMMA == g_token.opType) {
			match(TokenType::COMMA);
			Type _t = typeID();
			finfo->paratable[cnt].ptype = _t;
			finfo->paratable[cnt++].pname = copyString(g_token.value);
			match(TokenType::IDEN);
		}
	}
	// <空>
	;
	finfo->paranum = cnt;
}

////////////////////////////16'.__<变量说明> ::= <变量定义>;{<变量定义>;}
//这里是单纯的 <变量说明>
TreeNode* __varDeclaration() {
	// <变量定义>;
	TreeNode* t = newDecNode(DecKind::Var_DecK);

	TreeNode* p = varDefine();
	t->child[0] = p;

	match(TokenType::SEMICOLON);
	// {<变量定义>;}
	while (TokenType::INT == g_token.opType || TokenType::CHAR == g_token.opType) {
		TreeNode* q = varDefine();
		p->sibling = q;
		p = q;
		match(TokenType::SEMICOLON);
	}
	return t;
}

//23.<复合语句> ::= ［<常量说明>］［<变量说明>］<语句列>
TreeNode* complexStatement() {
	TreeNode* t = NULL;
	TreeNode* p = NULL;
	TreeNode* q = NULL;
	// ［<常量说明>］
	if (TokenType::CONST == g_token.opType) {
		t = constDeclaration();
		p = t;
	}
	// ［<变量说明>］
	if (TokenType::INT == g_token.opType || TokenType::CHAR == g_token.opType) {
		// 这里没有直接调用"varDeclaration"函数，因为函数中有一些多余的过程 来识别“函数定义”
		if (NULL == t) {
			t = __varDeclaration();
			p = t;
		}
		else {
			q = __varDeclaration();
			p->sibling = q;
			p = q;
		}
	}

	// <语句列>
	if (NULL == t) {
		t = statementSequence();
	}
	else {
		q = statementSequence();
		p->sibling = q;
		p = q;
	}
	

	return t;
}

//24.<主函数> ::= void main ‘(’ ‘)’ ‘{’<复合语句>‘}’
TreeNode* mainFunction() {
	TreeNode* t = newDecNode(DecKind::MainFunc_DecK);

	match(TokenType::VOID);
	match(TokenType::MAIN);
	match(TokenType::LPARENTHES);
	match(TokenType::RPARENTHES);
	match(TokenType::LBRACE);

	t->child[0] = complexStatement();

	match(TokenType::RBRACE);

	return t;

}

/********************* 以上是语法分析第二次单元测试的内容	*************************/

//25.<语句列> ::= ｛<语句>｝
TreeNode* statementSequence() {
	TreeNode* t = newStmtNode(StmtKind::Seq_StmtK);
	TreeNode* p = NULL;
	TreeNode* q = NULL;
	// FIRST( <语句> )：； if while for } ID(f) ID scanf printf return 
	while (TokenType::SEMICOLON == g_token.opType || TokenType::IF == g_token.opType || TokenType::WHILE == g_token.opType ||
		TokenType::FOR == g_token.opType || TokenType::LBRACE == g_token.opType || TokenType::IDEN == g_token.opType ||
		TokenType::SCANF == g_token.opType || TokenType::PRINTF == g_token.opType || TokenType::RETURN == g_token.opType) {
			
		if (NULL == p) {					// 第一条语句
			p = statement();
			t->child[0] = p;
		}
		else {								// 之后的语句
			q = statement();
			p->sibling = q;
			p = q;
		}
		

	}

	// FOLLOW(<语句列>): }
	if (TokenType::RBRACE == g_token.opType) {
		// 语句列里面一个语句也没有， 跳过
	}
	else {		// 既不是语句也不是 }
		printf("error in statementSequence() :in line %d ,tokenType %d value: %s \n", g_lineNumber, g_token.opType, g_token.value);
	}

	return t;
}

/*
26. < 语句 > :: = <条件语句>｜<循环语句>｜‘{ ’<语句列>‘ }’｜<有返回值的函数调用语句>; ｜<无返回值的函数调用语句>;
		｜<赋值语句>; ｜<读语句>; ｜<写语句>; ｜<空>; ｜<返回语句>;

!!!!!!注意：这里的  “<空>; ” 和 空语句是不一样的，只有个分号
*/
TreeNode* statement() {
	TreeNode* t = NULL;
	if (TokenType::SEMICOLON == g_token.opType) {
		// <空>;
		match(TokenType::SEMICOLON);
		return nullptr;									// 空语句返回空指针，什么都不做
	}
	else if (TokenType::IF == g_token.opType) {
		// <条件语句>:
		t = ifStatement();
	}
	else if (TokenType::WHILE == g_token.opType) {
		// <循环语句>
		t = whileLoopStatement();
	}
	else if (TokenType::FOR == g_token.opType) {
		// <循环语句>
		t = forLoopStatement();
	}
	else if (TokenType::LBRACE == g_token.opType) {
		//‘{ ’<语句列>‘ }’
		match(TokenType::LBRACE);
		t = statementSequence();
		match(TokenType::RBRACE);
	}
	else if (TokenType::IDEN == g_token.opType) {
		// <有返回值的函数调用语句>;
		// <无返回值的函数调用语句>;
		// <赋值语句>;

		// 此处要判断是 调用还是 赋值； 无论是否有返回值，都是一样的
		flashBackIndex = g_lexBegin;
		flashBackLineNum = g_lineNumber;

		getNextToken();

		bool isAssign = false;
		if (TokenType::ASSIGN == g_token.opType || TokenType::LBRACKET == g_token.opType) {
			// 赋值语句
			isAssign = true;
		}
		else {
			// 函数调用
		}

		g_forward = flashBackIndex;
		g_lineNumber = flashBackLineNum;

		getNextToken();

		if (isAssign) {
			t = assignStatement();
		}
		else {
			// 反正这里的函数调用都是 不利用其 返回值， 等价于无返回值的调用
			t = callWithoutReturn();
		}
		match(TokenType::SEMICOLON);

	}
	else if (TokenType::SCANF == g_token.opType) {
		// <读语句>;
		t = scanfStatement();
		match(TokenType::SEMICOLON);

	}
	else if (TokenType::PRINTF == g_token.opType) {
		// <写语句>;
		t = printfStatement();
		match(TokenType::SEMICOLON);
	}
	else if (TokenType::RETURN == g_token.opType) {
		// <返回语句>;
		t = returnStatement();
		match(TokenType::SEMICOLON);
	}
	else {
		// error
		printf("error in statement() :in line %d ,tokenType %d value: %s \n", g_lineNumber, g_token.opType, g_token.value);
	}
	return t;
}


//27. < 赋值语句 > :: = <标识符>[‘[’<算术表达式>‘]’]＝<算术表达式>
TreeNode* assignStatement() {
	TreeNode* t = newStmtNode(StmtKind::Assign_StmtK);
	// 该节点 有两个个孩子：标识符（存在该节点中）、表达式、数组下标表达式（可选）（在子节点）
	t->attr.name = copyString(g_token.value);
	match(TokenType::IDEN);

	if (TokenType::LBRACKET == g_token.opType) {		//	赋值给数组
		match(TokenType::LBRACKET);
		t->child[1] = exp();							// 第二个孩子是 Index
		match(TokenType::RBRACKET);
	}
	match(TokenType::ASSIGN);
	t->child[0] = exp();								// 第一个孩子，是右端的表达式

	return t;
}


//28. < 条件语句 > :: = if ‘(’<布尔表达式>‘)’<语句>［else<语句>］
TreeNode* ifStatement() {
	TreeNode* t = newStmtNode(StmtKind::If_StmtK);
	// 有三个孩子， bool表达式、then语句、else语句

	match(TokenType::IF);
	match(TokenType::LPARENTHES);
	t->child[0] = boolExp();				// 布尔表达式
	match(TokenType::RPARENTHES);
	t->child[1] = statement();				// then语句

	if (TokenType::ELSE == g_token.opType) {
		match(TokenType::ELSE);
		t->child[2] = statement();			// else语句
	}
	return t;
}

/*
29. < 循环语句 > :: = while ‘(’<布尔表达式>‘)’<语句> | for‘(’<赋值语句>; <布尔表达式>; <赋值语句>‘)’<语句>

for循环中的三个表达式：初始化表达式、循环变量测试表达式、循环变量修正表达式
*/
// while ‘(’<布尔表达式>‘)’<语句>
TreeNode* whileLoopStatement() {
	TreeNode* t = newStmtNode(StmtKind::While_StmtK);
	match(TokenType::WHILE);
	match(TokenType::LPARENTHES);
	t->child[0] = boolExp();
	match(TokenType::RPARENTHES);
	t->child[1] = statement();
	return t;
}

// for‘(’<赋值语句>; <布尔表达式>; <赋值语句>‘)’<语句>
TreeNode* forLoopStatement() {
	TreeNode* t = newStmtNode(StmtKind::For_StmtK);
	TreeNode* p = NULL;

	match(TokenType::FOR);
	match(TokenType::LPARENTHES);
	t->child[1] = assignStatement();
	match(TokenType::SEMICOLON);
	t->child[0] = boolExp();
	match(TokenType::SEMICOLON);
	p = assignStatement();
	match(TokenType::RPARENTHES);
	t->child[2] = statement();
	if (NULL == t->child[2]) {		// for循环是空语句
		t->child[2] = p;					// 把for循环末尾的赋值语句直接填充
	}
	else {
		t->child[2]->sibling = p;			// 把第三个赋值语句放到 循环体末尾
	}
	
	return t;
}


//30. < 有返回值的函数调用语句 > :: = <标识符>‘(’<值参数表>‘)’
// 表达式中用此函数, 将其看做一个特定op，有两个子节点：函数名、参数列表
TreeNode* callWithReturn() {
	TreeNode* t = newExpNode(ExpKind::Op_ExpK);
	t->attr.op = TokenType::CALL;

	t->child[0] = newExpNode(ExpKind::Iden_ExpK);
	t->child[0]->attr.name = copyString(g_token.value);

	match(TokenType::IDEN);
	match(TokenType::LPARENTHES);
	if (TokenType::RPARENTHES != g_token.opType) {
		t->child[1] = valueParaTable();
	}
	match(TokenType::RPARENTHES);
	return t;
}

//31. < 无返回值的函数调用语句 > :: = <标识符>‘(’<值参数表>‘)’
// 单纯调用语句 用此函数
TreeNode* callWithoutReturn() {
	TreeNode* t = newStmtNode(StmtKind::Call_StmtK);

	t->attr.name = copyString(g_token.value);
	match(TokenType::IDEN);

	match(TokenType::LPARENTHES);
	if (TokenType::RPARENTHES != g_token.opType) {
		t->child[0] = valueParaTable();
	}
	match(TokenType::RPARENTHES);
	return t;
}


/*
32. < 值参数表 > :: = <算术表达式>{ ,<算术表达式> }｜<空>

>>>>>>>> 空语句：这里也有一处空语句，但follow集只有')'一个元素
*/
TreeNode* valueParaTable() {
	TreeNode* t = NULL;
	TreeNode* p = NULL;
	TreeNode* q = NULL;
	t = exp();
	p = t;
	while (TokenType::COMMA == g_token.opType) {
		match(TokenType::COMMA);
		q = exp();
		p->sibling = q;
		p = q;
	}
	return t;
}


//33. < 读语句 > :: = scanf ‘(’<标识符>{, <标识符>}‘)’
TreeNode* scanfStatement() {
	TreeNode* t = newStmtNode(StmtKind::Read_StmtK);
	TreeNode* p = NULL;
	TreeNode* q = NULL;
	match(TokenType::SCANF);
	match(TokenType::LPARENTHES);

	p = newStmtNode(StmtKind::Read_StmtK_Idlist);
	p->attr.name = copyString(g_token.value);
	t->child[0] = p;

	match(TokenType::IDEN);
	while (TokenType::COMMA == g_token.opType) {
		match(TokenType::COMMA);
		q = newStmtNode(StmtKind::Read_StmtK_Idlist);
		q->attr.name = copyString(g_token.value);
		p->sibling = q;
		p = q;
		match(TokenType::IDEN);
	}
	match(TokenType::RPARENTHES);
	return t;
}


/*
34. < 写语句 > :: = printf ‘(’<字符串>, <算术表达式>‘)’ | printf ‘(’<字符串>‘)’ | printf ‘(’<算术表达式>‘)’

定义写语句是以printf为起始的，后接圆括号括起来的字符串或表达式或者两者都有，若两者都存在，则字符串在先，以逗号隔开。
*/
TreeNode* printfStatement() {
	TreeNode* t = newStmtNode(StmtKind::Write_StmtK);
	match(TokenType::PRINTF);
	match(TokenType::LPARENTHES);
	if (TokenType::STRING == g_token.opType) {
		t->child[0] = newStmtNode(StmtKind::Write_StmtK_Str);
		t->child[0]->attr.str = copyString(g_token.value);

		match(TokenType::STRING);
		if (TokenType::COMMA == g_token.opType) {
			match(TokenType::COMMA);
			t->child[1] = exp();
		}
	}
	else {
		t->child[1] = exp();
	}
	match(TokenType::RPARENTHES);
	return t;
}

//35. < 返回语句 > :: = return[‘(’<算术表达式>‘)’]
TreeNode* returnStatement() {
	TreeNode* t = newStmtNode(StmtKind::Ret_StmtK);
	match(TokenType::RETURN);
	if (TokenType::LPARENTHES == g_token.opType) {
		match(TokenType::LPARENTHES);
		t->child[0] = exp();
		match(TokenType::RPARENTHES);
	}
	return t;
}


/************************  以上：语法分析第三次测试内容  ***********************/

//36. < 算术表达式 > :: = ［＋｜－］<项>{ <加法运算符><项> }
TreeNode* exp() {
	TreeNode* t = NULL;
	TreeNode* p = NULL;
	bool isNegative = false;

	// ［＋｜－］
	if (TokenType::PLUS == g_token.opType || TokenType::MINU == g_token.opType) {
		if (TokenType::MINU == g_token.opType) {
			isNegative = true;
			match(TokenType::MINU);
		}
		else {
			match(TokenType::PLUS);
		}
	}

	t = term();

	if (isNegative) {
		// 正的 不做处理，负数 会把第一项的值， 变号
		//p = newExpNode(ExpKind::Op_ExpK);
		//p->attr.op = TokenType::NEGATIVE;
		//p->child[0] = t;
		//t = p;

		// 新方法： 开始的负号转换成： 0 - term
		p = newExpNode(ExpKind::Op_ExpK);
		p->attr.op = TokenType::MINU;
		p->child[1] = t;
		t = p;

		p = newExpNode(ExpKind::Num_ExpK);
		p->attr.val = 0;
		t->child[0] = p;
	}
	// { <加法运算符><项> }
	while (TokenType::PLUS == g_token.opType || TokenType::MINU == g_token.opType) {
		if (TokenType::MINU == g_token.opType) {
			p = newExpNode(ExpKind::Op_ExpK);
			p->attr.op = TokenType::MINU;
			p->child[0] = t;
			match(TokenType::MINU);
			t = term();
			p->child[1] = t;
			t = p;
		}
		else {
			p = newExpNode(ExpKind::Op_ExpK);
			p->attr.op = TokenType::PLUS;
			p->child[0] = t;
			match(TokenType::PLUS);
			t = term();
			p->child[1] = t;
			t = p;
		}
	}
	return t;
}


//37. < 项 > :: = <因子>{ <乘法运算符><因子> }
TreeNode* term() {
	TreeNode* t = NULL;
	TreeNode* p = NULL;
	t = factor();

	while (TokenType::MULT == g_token.opType || TokenType::DIV == g_token.opType) {
		if (TokenType::MULT == g_token.opType) {
			p = newExpNode(ExpKind::Op_ExpK);
			p->attr.op = TokenType::MULT;
			p->child[0] = t;
			match(TokenType::MULT);
			t = factor();
			p->child[1] = t;
			t = p;
		}
		else {
			p = newExpNode(ExpKind::Op_ExpK);
			p->attr.op = TokenType::DIV;
			p->child[0] = t;
			match(TokenType::DIV);
			t = factor();
			p->child[1] = t;
			t = p;
		}
	}
	return t;
}

//38. < 因子 > :: = <标识符>｜<标识符>‘[’<算术表达式>‘]’｜<整数> | <字符>｜<有返回值函数调用语句> | ‘(’<算术表达式>‘)’
TreeNode* factor() {
	// <标识符>｜<标识符>‘[’<算术表达式>‘]’｜<有返回值函数调用语句>
	TreeNode* t = NULL;
	TreeNode* p = NULL;

	if (TokenType::IDEN == g_token.opType) {
		// 此处要判断是 调用还是 赋值
		flashBackIndex = g_lexBegin;
		flashBackLineNum = g_lineNumber;
		getNextToken();

		bool isCall = false;
		if (TokenType::LPARENTHES == g_token.opType) {
			// 函数调用
			isCall = true;
		}
		
		g_forward = flashBackIndex;
		g_lineNumber = flashBackLineNum;
		getNextToken();

		// <有返回值函数调用语句>
		if (isCall) {
			t = callWithReturn();
		}
		else {
			// <标识符>｜<标识符>‘[’<算术表达式>‘]’
			t = newExpNode(ExpKind::Iden_ExpK);
			t->attr.name = copyString(g_token.value);
			match(TokenType::IDEN);
			if (TokenType::LBRACKET == g_token.opType) {
				p = newExpNode(ExpKind::Op_ExpK);
				p->attr.op = TokenType::ARRAYAT;
				p->child[0] = t;
				match(TokenType::LBRACKET);
				t = exp();
				match(TokenType::RBRACKET);
				p->child[1] = t;
				t = p;
			}
		}
	}
	else if (TokenType::NUM == g_token.opType) {
		t = newExpNode(ExpKind::Num_ExpK);
		t->attr.val = atoi(g_token.value);
		match(TokenType::NUM);
	}
	else if (TokenType::LETTER == g_token.opType) {
		t = newExpNode(ExpKind::Num_ExpK);
		t->attr.val = g_token.value[0];
		match(TokenType::LETTER);
	}
	else if (TokenType::LPARENTHES == g_token.opType) {
		match(TokenType::LPARENTHES);
		t = exp();
		match(TokenType::RPARENTHES);
	}
	else {
		printf("error in factor() :in line %d ,tokenType %d value: %s \n", g_lineNumber, g_token.opType, g_token.value);
	}

	return t;

}

//39. < 布尔表达式 > :: = <布尔项>{ ‘ || ’ <布尔项> }
TreeNode* boolExp() {
	TreeNode* t = NULL;
	TreeNode* p = NULL;
	t = boolTerm();
	// { ‘ || ’ <布尔项> }
	while (TokenType::OR == g_token.opType) {
		p = newBoolExpNode(BoolExpKind::Op_BoolEK);
		p->attr.op = TokenType::OR;
		p->child[0] = t;
		match(TokenType::OR);
		t = boolTerm();
		p->child[1] = t;
		t = p;
	}
	return t;
}

//40. < 布尔项 > :: = <布因子>{ && <布因子> }
TreeNode* boolTerm() {
	TreeNode* t = NULL;
	TreeNode* p = NULL;
	t = boolFactor();
	// { && <布因子> }
	while (TokenType::AND == g_token.opType) {
		p = newBoolExpNode(BoolExpKind::Op_BoolEK);
		p->attr.op = TokenType::AND;
		p->child[0] = t;
		match(TokenType::AND);
		t = boolFactor();
		p->child[1] = t;
		t = p;
	}
	return t;
}


//41. < 布因子 > :: = false | true | !<布因子> | ‘(‘<布尔表达式>’)’ | <条件因子>[<条件运算符> <条件因子>]
TreeNode* boolFactor() {
	TreeNode* t = NULL;
	TreeNode* p = NULL;

	switch (g_token.opType) {
	case TokenType::FALSE:
		t = newBoolExpNode(BoolExpKind::Const_BoolEK);
		t->attr.bval = false;
		match(TokenType::FALSE);
		break;
	case TokenType::TRUE:
		t = newBoolExpNode(BoolExpKind::Const_BoolEK);
		t->attr.bval = true;
		match(TokenType::TRUE);
		break;
	case TokenType::NOT:
		match(TokenType::NOT);
		t = newBoolExpNode(BoolExpKind::Op_BoolEK);
		t->attr.op = TokenType::NOT;
		t->child[0] = boolFactor();

		break;
	case TokenType::LPARENTHES:
		match(TokenType::LPARENTHES);
		t = boolExp();
		match(TokenType::RPARENTHES);
		break;
	default: // <条件因子>[<条件运算符> <条件因子>]
		//conditionFactor();
		t = exp();
		// [<条件运算符> <条件因子>]
		if (TokenType::LSS == g_token.opType || TokenType::LEQ == g_token.opType ||
			TokenType::GRE == g_token.opType || TokenType::GEQ == g_token.opType ||
			TokenType::NEQ == g_token.opType || TokenType::EQL == g_token.opType
			) {
			p = newBoolExpNode(BoolExpKind::ConOp_BoolEK);
			p->attr.op = g_token.opType;
			p->child[0] = t;
			switch (g_token.opType) {
			case TokenType::LSS:
				match(TokenType::LSS);
				break;
			case TokenType::LEQ:
				match(TokenType::LEQ);
				break;
			case TokenType::GRE:
				match(TokenType::GRE);
				break;
			case TokenType::GEQ:
				match(TokenType::GEQ);
				break;
			case TokenType::NEQ:
				match(TokenType::NEQ);
				break;
			case TokenType::EQL:
				match(TokenType::EQL);
				break;
			}
			//conditionFactor();
			t = exp();
			p->child[1] = t;
			t = p;
		}
	}

	return t;
}


//42. < 条件因子 > :: = <标识符>[‘[’<算术表达式>‘]’]｜ <整数> | <字符> | <有返回值函数调用语句>
// 方便处理，并不使用; 用的是表达式来识别
TreeNode* conditionFactor() {
	// <标识符>｜<标识符>‘[’<算术表达式>‘]’｜<有返回值函数调用语句>
	if (TokenType::IDEN == g_token.opType) {
		// 此处要判断是 调用还是 赋值
		flashBackIndex = g_lexBegin;
		flashBackLineNum = g_lineNumber;
		getNextToken();

		bool isCall = false;
		if (TokenType::LPARENTHES == g_token.opType) {
			// 函数调用
			isCall = true;
		}

		g_forward = flashBackIndex;
		g_lineNumber = flashBackLineNum;
		getNextToken();

		// <有返回值函数调用语句>
		if (isCall) {
			callWithReturn();
		}
		else {
			// <标识符>｜<标识符>‘[’<算术表达式>‘]’
			match(TokenType::IDEN);
			if (TokenType::LBRACKET == g_token.opType) {
				match(TokenType::LBRACKET);
				exp();
				match(TokenType::RBRACKET);
			}
		}
	}
	else if (TokenType::NUM == g_token.opType) {
		match(TokenType::NUM);
	}
	else if (TokenType::LETTER == g_token.opType) {
		match(TokenType::LETTER);
	}
	else {
		printf("error in conditionFactor() :in line %d ,tokenType %d value: %s \n", g_lineNumber, g_token.opType, g_token.value);
	}

	return nullptr;
}

/*
事实上，这里<条件因子>不像（算术）表达式中一样，这里不支持嵌套，不支持’()’，因为布尔表达式设计起来符合嵌套，
	主要是括号不好处理；暂时这么设计，然后如果真要表达式的话，可以在前面先计算赋值给一个变量
*/





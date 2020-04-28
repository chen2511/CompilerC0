#include <string>
#include "semantic.h"


static bool isGlobal = true;

void semanticExp(TreeNode* tree);
int checkParaNum(TreeNode* list);


// 检查 函数调用 参数列表 ，返回个数
int checkParaNum(TreeNode* list) {
	// 参数列表是一个表达式列表，里面的每一个表达式都要检查
	int cnt = 0;
	while (NULL != list) {
		cnt++;
		semanticExp(list);
		list = list->sibling;
	}
	return cnt;
}

void semanticExp(TreeNode* tree) {
	if (ExpKind::Num_ExpK == tree->kind.exp) {
		// 是常量， 什么都不做
	}
	else if (ExpKind::Iden_ExpK == tree->kind.exp) {
		Symbol* lookup = lookUp_SymTab(tree->attr.name);

		if (lookup == NULL) {
			printf("Error in line %d : 标识符未定义：%s\n", tree->lineno, tree->attr.name);
			return;
		}

		if (lookup->vec >= 0) {				// 是数组， 没有下标运算
			printf("Error in line %d : 标识符是数组，需要进行下标运算：%s\n", tree->lineno, tree->attr.name);
			return;
		}

	}
	else {				//ExpKind::Op_ExpK
		if (tree->attr.op == TokenType::CALL) {				// 函数调用 要检查：函数是否声明，以及参数个数是否匹配，是否有返回值
			Symbol* lookup = lookUp_SymTab(tree->child[0]->attr.name);

			if (lookup == NULL) {
				printf("Error in line %d : 函数未定义: %s()\n", tree->lineno, tree->child[0]->attr.name);
				return;
			}

			// 检查返回值
			if (Type::T_VOID == lookup->valueType) {
				printf("Error in line %d : 函数无返回值,  %s()\n", tree->lineno, lookup->name);
				return;
			}

			// 检查参数个数， 同时检查每个参数的表达式是否合法
			if (lookup->pfinfo->paranum != checkParaNum(tree->child[1])) {
				printf("Error in line %d : 函数参数不匹配, %s()\n", tree->lineno, lookup->name);
				return;
			}

		}
		else if (tree->attr.op == TokenType::ARRAYAT) {			// 下标运算，检查标识符是否是数组， 并且检查index位置的表达式；但不检查越界
			Symbol* lookup = lookUp_SymTab(tree->child[0]->attr.name);

			if (lookup == NULL) {
				printf("Error in line %d : 标识符未定义：%s\n", tree->lineno, tree->child[0]->attr.name);
				return;
			}

			if (lookup->vec < 0) {
				printf("Error in line %d : 标识符不是数组，不能进行下标运算：%s\n", tree->lineno, tree->child[0]->attr.name);
				return;
			}

			semanticExp(tree->child[1]);
		}
		else {							// +-*/ 检查左右节点
			semanticExp(tree->child[0]);
			semanticExp(tree->child[1]);
		}
	}
}


void semanticBoolExp(TreeNode* tree) {
	if (tree == NULL)
		return;
	// bool Exp 里面有 4 种节点： 布尔op、布尔常量、关系op、表达式
	if (tree->nodekind == NodeKind::ExpK) {
		semanticExp(tree);
	}
	else {
		if (tree->kind.bexp == BoolExpKind::ConOp_BoolEK) {			// 表达式 ConOp 表达式
			semanticExp(tree->child[0]);
			semanticExp(tree->child[1]);
		}
		else if (tree->kind.bexp == BoolExpKind::Const_BoolEK) {
			// 常量 ，不进行检查		true、false
		}
		else {  // BoolExpKind::Op_BoolEK： 与或非
			semanticBoolExp(tree->child[0]);
			semanticBoolExp(tree->child[1]);
		}
	}
}

// 递归遍历AST，建立符号表，并进行类型检查
void semanticAnalyze(TreeNode* tree)			
{
	if (NULL == tree)
		return;

	Type ttt;
	TreeNode* st;
	SymTab* tmp;
	Symbol* lookup;
	while (NULL != tree) {
		if (NodeKind::DecK == tree->nodekind) {
			switch (tree->kind.dec)
			{
			case DecKind::Const_DecK:
				break;
			case DecKind::Const_DefK:			// 常量定义
				ttt = tree->type;
				st = tree->child[0];			// 这里是 常量列表
				int val;
				while (NULL != st) {
					if (Type::T_INTEGER == ttt) {
						val = st->child[0]->attr.val;
						insert_SymTab(isGlobal, tree->lineno, st->attr.name, IDType::Const_ID, Type::T_INTEGER, val);
					}
					else {
						val = st->child[0]->attr.cval;
						insert_SymTab(isGlobal, tree->lineno, st->attr.name, IDType::Const_ID, Type::T_CHAR, val);
					}
					st = st->sibling;
				}
				// 多个常量定义
				semanticAnalyze(tree->sibling);
				// 返回跳过递归； 事实上是之前抽象语法树这里应该多定义一种类型；就不用上面这两行了，不过代价不大
				return;
			case DecKind::Var_DecK:
				break;
			case DecKind::Var_DefK:					// 变量定义
				ttt = tree->type;
				st = tree->child[0];
				
				while (NULL != st) {
					insert_SymTab(isGlobal, tree->lineno, st->attr.name, IDType::Var_ID, ttt, (int)INFINITY, st->vec);

					st = st->sibling;
				}
				// 多个变量定义
				semanticAnalyze(tree->sibling);
				
				return;
			case DecKind::Func_DecK:					// 函数定义
				// 开始定义函数， 常变量声明只能进入函数表
				isGlobal = false;

				// 处理函数名，插入全局表
				insert_SymTab(true, tree->lineno, tree->attr.name, IDType::Func_ID, tree->pfinfo->rettype, (int)INFINITY, -1, tree->pfinfo);

				// 新建函数表
				tmp = initSimpleSymTable(tree->attr.name);
				tmp->next = g_symtab->next;
				g_symtab->next = tmp;
				// 处理参数表
				for (int i = 0; i < tree->pfinfo->paranum; i++) {
					insert_SymTab(false, tree->lineno, tree->pfinfo->paratable[i].pname, IDType::Para_ID, tree->pfinfo->paratable[i].ptype,
						(int)INFINITY);
				}
				break;
			case DecKind::MainFunc_DecK:
				// 开始定义函数， 常变量声明只能进入函数表
				isGlobal = false;

				insert_SymTab(true, tree->lineno, (char*)("main"), IDType::Func_ID, Type::T_VOID, (int)INFINITY, -1);

				// 新建函数表
				
				tmp = initSimpleSymTable((char*)("main"));
				tmp->next = g_symtab->next;
				g_symtab->next = tmp;
				break;
			default:
				break;
			}
		}
		else if (NodeKind::StmtK == tree->nodekind) {		// 此处有一些重大bug， case里面别写return；会把后面的兄弟节点跳过
			switch (tree->kind.stmt)
			{
			case StmtKind::Assign_StmtK:					// 赋值语句
			{
				lookup = lookUp_SymTab(tree->attr.name);
				if (NULL == lookup) {				// 未定义
					printf("Error in line %d : 标识符未定义：%s\n", tree->lineno, tree->attr.name);
				}
				else {								// 已定义
					// 赋值语句 左边类型 
					tree->type = lookup->valueType;

					if (lookup->type == IDType::Const_ID) {		// 常量
						printf("Error in line %d : 标识符是常量，不能赋值：%s\n", tree->lineno, tree->attr.name);
						// 继续扫描后面，不立马返回
					}
					if (NULL != tree->child[1]) {
						if (lookup->vec <= 0) {				// 不是数组，进行下标运算
							printf("Error in line %d : 标识符不是数组，不能进行下标运算：%s\n", tree->lineno, tree->attr.name);
						}
						// 不管是不是数组，都检查 index表达式
						// semanticExp(tree->child[1]);
					}
					else {
						if (lookup->vec >= 0) {				// 是数组， 没有下标运算
							printf("Error in line %d : 标识符是数组，需要进行下标运算：%s\n", tree->lineno, tree->attr.name);
						}
					}
				}
				// 右侧表达式
				//semanticExp(tree->child[0]);

				// 检查由末尾处 遍历child节点
			}
				break;
			case StmtKind::If_StmtK:
				break;
			case StmtKind::While_StmtK:
				break;
			case StmtKind::For_StmtK:
				break;
			case StmtKind::Call_StmtK:			// 无返回值函数调用
			{
				lookup = lookUp_SymTab(tree->attr.name);
				if (NULL == lookup) {					// 未定义
					printf("Error in line %d : 函数未定义：%s()\n", tree->lineno, tree->attr.name);
					//return;
				}
				else {									// 已定义
					if (IDType::Func_ID != lookup->type) {			// 不是函数
						printf("Error in line %d : 该标识符不是函数：%s()\n", tree->lineno, tree->attr.name);
						//return;
					}
					else {											// 是函数，要检查参数
						// 检查参数个数， 同时检查每个参数的表达式是否合法
						if (lookup->pfinfo->paranum != checkParaNum(tree->child[0])) {
							printf("Error in line %d : 函数参数不匹配: %s()\n", tree->lineno, lookup->name);
							//return;
						}
					}
				}
				//return;
			}
				break;
			case StmtKind::Seq_StmtK:
				break;
			case StmtKind::Read_StmtK: 
				break;
			case StmtKind::Read_StmtK_Idlist:
			{
				lookup = lookUp_SymTab(tree->attr.name);
				if (NULL == lookup) {
					printf("Error in line %d : 标识符未定义：%s\n", tree->lineno, tree->attr.name);
					// return;
				}
				else {
					if (lookup->type == IDType::Const_ID) {		// 常量
						printf("Error in line %d : 标识符是常量，不能读入：%s\n", tree->lineno, tree->attr.name);
						// 继续扫描后面，不立马返回
					}
				}
			}
				break;
			case StmtKind::Write_StmtK:
			{
				if (NULL == tree->child[0]) {			// 只有表达式，没有字符串； 直接跳过以下检查
					break;
				}
				std::string printstring = tree->child[0]->attr.name;
				size_t pos;
				pos = printstring.find("%d");
				if (pos != std::string::npos) {			// 找到 %d
					tree->type = Type::T_INTEGER;

					if (NULL == tree->child[1]) {		// 没有表达式
						printf("Error in line %d : 占用符不匹配: d \n", tree->lineno);
						//return;
					}
					else {								// 检查表达式		改为递归检查，这里不检查
						//	semanticExp(tree->child[1]);
					}

					break;		//不继续找了
				}

				pos = printstring.find("%c");
				if (pos != std::string::npos) {			// 找到 %c
					tree->type = Type::T_CHAR;

					if (NULL == tree->child[1]) {		// 没有表达式
						printf("Error in line %d : 占用符不匹配: c \n", tree->lineno);
						//return;
					}
					else {								// 检查表达式   
						//	semanticExp(tree->child[1]);
					}

					break;		//不继续找了
				}

				// 既没有%c 也没有%d
				if (NULL != tree->child[1]) {		// 却有表达式
					printf("Error in line %d : 缺少占用符 \n", tree->lineno);
					//return;
				}
			}
				break;
			case StmtKind::Write_StmtK_Str:			// 并不会进入这里， 上一级 处理了
				break;
			case StmtKind::Ret_StmtK:
				break;
			default:
				break;
			}
		}
		else if (NodeKind::ExpK == tree->nodekind) {			// 因为表达式单独处理，并且没有兄弟节点，直接返回
			// 单独处理表达式
			semanticExp(tree);
			return;
		}
		else // 布尔表达式
		{
			semanticBoolExp(tree);
			return;
		}

		for (int i = 0; i < MAX_TREENODE_CHILD_NUM; i++) {
			if (NULL != tree->child[i])
				semanticAnalyze(tree->child[i]);
		}
		// 此处加节点 遍历结束 要执行的代码
		tree = tree->sibling;
	}
}

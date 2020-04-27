#include "semantic.h"

static bool isGlobal = true;

// 递归遍历AST，建立符号表，并进行类型检查
void semanticAnalyze(TreeNode* tree)			
{
	if (NULL == tree)
		return;

	Type ttt;
	TreeNode* st;
	SymTab* tmp;
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
		else if (NodeKind::StmtK == tree->nodekind) {
			switch (tree->kind.stmt)
			{
			case StmtKind::Assign_StmtK:
				break;
			case StmtKind::If_StmtK:
				break;
			case StmtKind::While_StmtK:
				break;
			case StmtKind::For_StmtK:
				break;
			case StmtKind::Call_StmtK:
				break;
			case StmtKind::Seq_StmtK:
				break;
			case StmtKind::Read_StmtK:
				break;
			case StmtKind::Read_StmtK_Idlist:
				break;
			case StmtKind::Write_StmtK:
				break;
			case StmtKind::Write_StmtK_Str:
				break;
			case StmtKind::Ret_StmtK:
				break;
			default:
				break;
			}
		}
		else if (NodeKind::ExpK == tree->nodekind) {
			// 好像还是 原来的顺序比较好； 这样的话 输出结果没有括号，有歧义
		}
		else // 布尔表达式
		{

		}

		for (int i = 0; i < MAX_TREENODE_CHILD_NUM; i++) {
			if (NULL != tree->child[i])
				semanticAnalyze(tree->child[i]);
		}
		// 此处加节点 遍历结束 要执行的代码
		tree = tree->sibling;
	}
}

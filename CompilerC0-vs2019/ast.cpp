#include "ast.h"

/*
	AST节点的建立、输出AST到文件的实现
*/


// 创建声明节点：包括常量、变量、函数等等说明定义，传入具体声明类型，返回节点指针
TreeNode* newDecNode(DecKind kind) {
	TreeNode* t = new TreeNode;

	if (NULL == t) {
		std::cout << "Out of memory in alloc new node in line: " << g_lineNumber << std::endl;
	}
	for (int i = 0; i < MAX_TREENODE_CHILD_NUM; ++i) {
		t->child[i] = NULL;
	}
	t->sibling = NULL;
	t->lineno = g_lineNumber;
	//类型信息
	t->nodekind = NodeKind::DecK;
	t->kind.dec = kind;
	// attr type 不做初始化
	t->vec = -1;
	t->pfinfo = NULL;

	return t;
}

// 创建语句节点：传入具体声明类型，返回节点指针
TreeNode* newStmtNode(StmtKind kind) {
	TreeNode* t = new TreeNode;

	if (NULL == t) {
		std::cout << "Out of memory in alloc new node in line: " << g_lineNumber << std::endl;
	}
	for (int i = 0; i < MAX_TREENODE_CHILD_NUM; ++i) {
		t->child[i] = NULL;
	}
	t->sibling = NULL;
	t->lineno = g_lineNumber;
	t->nodekind = NodeKind::StmtK;
	t->kind.stmt = kind;
	// attr type 不做初始化
	t->vec = -1;
	t->pfinfo = NULL;

	return t;
}

// 创建表达式节点：传入具体声明类型，返回节点指针
TreeNode* newExpNode(ExpKind kind) {
	TreeNode* t = new TreeNode;

	if (NULL == t) {
		std::cout << "Out of memory in alloc new node in line: " << g_lineNumber << std::endl;
	}
	for (int i = 0; i < MAX_TREENODE_CHILD_NUM; ++i) {
		t->child[i] = NULL;
	}
	t->sibling = NULL;
	t->lineno = g_lineNumber;
	t->nodekind = NodeKind::ExpK;
	t->kind.exp = kind;
	// attr type 不做初始化
	t->vec = -1;
	t->pfinfo = NULL;

	return t;
}

// 创建布尔表达式节点：传入具体声明类型，返回节点指针
TreeNode* newBoolExpNode(BoolExpKind kind) {
	TreeNode* t = new TreeNode;

	if (NULL == t) {
		std::cout << "Out of memory in alloc new node in line: " << g_lineNumber << std::endl;
	}
	for (int i = 0; i < MAX_TREENODE_CHILD_NUM; ++i) {
		t->child[i] = NULL;
	}
	t->sibling = NULL;
	t->lineno = g_lineNumber;
	t->nodekind = NodeKind::BoolExpK;
	t->kind.bexp = kind;
	// attr type 不做初始化
	t->vec = -1;
	t->pfinfo = NULL;

	return t;
}

// 定义函数时，保存函数信息：返回类型和参数表
//FuncInfo* newFuncInfo(Type t)
//{
//	return nullptr;
//}

char* copyString(char* s)
{
	int n;
	char* t;
	if (s == NULL) 
		return NULL;
	n = strlen(s) + 1;
	t = (char*)malloc(n);
	if (t == NULL)
		std::cout << "Out of memory in alloc copyString in line: " << g_lineNumber << std::endl;
	else strcpy(t, s);
	return t;
	
}

static char op_code[10];
void tran2opcode(TokenType op) {
	switch (op)
	{
	case TokenType::PLUS:
		op_code[0] = '+';
		op_code[1] = '\0';
		break;
	case TokenType::MINU:
		op_code[0] = '-';
		op_code[1] = '\0';
		break;
	case TokenType::MULT:
		op_code[0] = '*';
		op_code[1] = '\0';
		break;
	case TokenType::DIV:
		op_code[0] = '/';
		op_code[1] = '\0';
		break;
	case TokenType::ARRAYAT:
		op_code[0] = '[';
		op_code[1] = ']';
		op_code[2] = '\0';
		break;
	case TokenType::CALL:
		op_code[0] = '(';
		op_code[1] = ')';
		op_code[2] = '\0';
		break;
	case TokenType::NEGATIVE:
		op_code[0] = '%';
		op_code[1] = '\0';
		break;
	case TokenType::AND:
		op_code[0] = '&';
		op_code[1] = '&';
		op_code[2] = '\0';
		break;
	case TokenType::OR:
		op_code[0] = '|';
		op_code[1] = '|';
		op_code[2] = '\0';
		break;
	case TokenType::LSS:
		op_code[0] = '<';
		op_code[1] = '\0';
		op_code[2] = '\0';
		break;
	case TokenType::LEQ:
		op_code[0] = '<';
		op_code[1] = '=';
		op_code[2] = '\0';
		break;
	case TokenType::GRE:
		op_code[0] = '>';
		op_code[1] = '\0';
		op_code[2] = '\0';

		break;
	case TokenType::GEQ:
		op_code[0] = '>';
		op_code[1] = '=';
		op_code[2] = '\0';
		break;
	case TokenType::EQL:
		op_code[0] = '=';
		op_code[1] = '=';
		op_code[2] = '\0';
		break;
	case TokenType::NEQ:
		op_code[0] = '!';
		op_code[1] = '=';
		op_code[2] = '\0';
		break;
	default:
		break;
	}
}

// 中序输出 表达式 ； 之前是先序
void printExp(TreeNode* exp) {
	if (NULL == exp)
		return;
	printExp(exp->child[0]);


	switch (exp->kind.exp)
	{
	case ExpKind::Num_ExpK:
		fprintf(AST_File, " %d ", exp->attr.val);
		break;
	case ExpKind::Iden_ExpK:
		fprintf(AST_File, " %s ", exp->attr.name);
		break;
	case ExpKind::Letter_ExpK:
		fprintf(AST_File, " '%c' ", exp->attr.cval);
		break;
	case ExpKind::Op_ExpK:
		tran2opcode(exp->attr.op);
		fprintf(AST_File, " %s ", op_code);
		break;
	default:
		break;
	}

	printExp(exp->child[1]);

}

void printBoolExp(TreeNode* exp) {
	if (NULL == exp)
		return;
	printBoolExp(exp->child[0]);

	if (exp->nodekind == NodeKind::BoolExpK) {
		printExp(exp);
	}
	else {
		switch (exp->kind.bexp)
		{
		case BoolExpKind::Op_BoolEK:
			tran2opcode(exp->attr.op);
			fprintf(AST_File, " %s ", op_code);
			break;
		case BoolExpKind::Const_BoolEK:
			fprintf(AST_File, " %d ", exp->attr.bval);
			break;
		case BoolExpKind::ConOp_BoolEK:
			tran2opcode(exp->attr.op);
			fprintf(AST_File, " %s ", op_code);
			break;
		default:
			break;
		}
	}

	

	printBoolExp(exp->child[0]);
}

// 输出抽象语法树；还是有必要的，因为后面调试的时候，要是出什么问题，可以通过输出的结果来查找问题；是哪一阶段的问题
void printAST(TreeNode* tree)
{
	if (NULL == tree)
		return;
	
	Type ttt;
	TreeNode* st;
	while (NULL != tree) {
		if (NodeKind::DecK == tree->nodekind) {
			switch (tree->kind.dec)
			{
			case DecKind::Const_DecK:
				fprintf(AST_File, "\n------------ Const Declaration ------------\n\n");
				break;
			case DecKind::Const_DefK:
				//fprintf(AST_File, " >>> Const Defination \n");
				fprintf(AST_File, "Type: %d	(VOID,INT,CHAR)	list: \n", tree->type);
				ttt = tree->type;
				st = tree;
				tree = tree->child[0];
				while (tree != NULL) {
					if (Type::T_INTEGER == ttt) {
						fprintf(AST_File, "ID name:		%s\t\tvalue:	%d\n", tree->attr.name, tree->child[0]->attr.val);
					}
					else {
						fprintf(AST_File, "ID name:		%s\t\tvalue:	%c\n", tree->attr.name, tree->child[0]->attr.cval);
					}
					tree = tree->sibling;
				}
				
				if (NULL != st->sibling) {		// 多个常量定义列表
					st = st->sibling;
					printAST(st);
				}
				return;
				break;
			case DecKind::Var_DecK:
				fprintf(AST_File, "\n------------ Var Declaration ------------\n");
				break;
			case DecKind::Var_DefK:
				//fprintf(AST_File, " >>> Var Defination \n");
				fprintf(AST_File, "Type: %d	(VOID,INT,CHAR)	list: \n", tree->type);
				ttt = tree->type;
				st = tree;
				tree = tree->child[0];
				while (tree != NULL) {
					if (Type::T_INTEGER == ttt) {
						fprintf(AST_File, "ID name: %s\n", tree->attr.name);
					}
					else {
						fprintf(AST_File, "ID name: %s\n", tree->attr.name);
					}
					tree = tree->sibling;
				}
				fprintf(AST_File, "\n");
				st = st->sibling;					// 多个变量定义列表
				if (NULL != st) {
					printAST(st);
				}
				return;
				break;
			case DecKind::Func_DecK:
				fprintf(AST_File, "\n>>>>>>>>>>>> Function Declaration >>>>>>>>>>>>\n");
				fprintf(AST_File, " Function Name: %s \n",tree->attr.name);
				fprintf(AST_File, " Function Info: Return Type(VOID,INT,CHAR):%d \n\t\tParaTable(VOID,INT,CHAR): ", tree->pfinfo->rettype);
				for (int i = 0; i < tree->pfinfo->paranum; ++i) {
					fprintf(AST_File, "%d %s\t", tree->pfinfo->paratable->ptype, tree->pfinfo->paratable->pname);
				}
				fprintf(AST_File, "\n");
				break;
			case DecKind::MainFunc_DecK:
				fprintf(AST_File, "\n***************************************************\n");
				fprintf(AST_File, "************ Main Function Declaration ************\n");
				fprintf(AST_File, "***************************************************\n");
				break;
			default:
				break;
			}
		}
		else if (NodeKind::StmtK == tree->nodekind) {
			switch (tree->kind.stmt)
			{
			case StmtKind::Assign_StmtK:
				fprintf(AST_File, "\nAssign to: %s\n", tree->attr.name);
				break;
			case StmtKind::If_StmtK:
				fprintf(AST_File, "\nIf :\n");
				break;
			case StmtKind::While_StmtK:
				fprintf(AST_File, "\nWhile Stmt:\n");
				break;
			case StmtKind::For_StmtK:
				fprintf(AST_File, "\nFor Stmt:\n");
				break;
			case StmtKind::Call_StmtK:
				fprintf(AST_File, "\nCall Stmt:  Function: %s \n", tree->attr.name);
				break;
			case StmtKind::Seq_StmtK:
				fprintf(AST_File, "\nStmt Sequence>>>   \n");
				break;
			case StmtKind::Read_StmtK:
				fprintf(AST_File, "\nRead Stmt:\n");
				break;
			case StmtKind::Read_StmtK_Idlist:
				fprintf(AST_File, "Read ID: %s\n", tree->attr.name);
				break;
			case StmtKind::Write_StmtK:
				fprintf(AST_File, "\nWrite Stmt:\n");
				break;
			case StmtKind::Write_StmtK_Str:
				fprintf(AST_File, "Write Str: %s\n", tree->attr.name);
				break;
			case StmtKind::Ret_StmtK:
				fprintf(AST_File, "\nRet Stmt:\n");
				break;
			default:
				break;
			}
		}
		else if (NodeKind::ExpK == tree->nodekind) {
			// 好像还是 原来的顺序比较好； 这样的话 输出结果没有括号，有歧义
			printExp(tree);
			return;
		}
		else
		{
			switch (tree->kind.bexp)
			{
			case BoolExpKind::Op_BoolEK:
				tran2opcode(tree->attr.op);
				fprintf(AST_File, " %s ", op_code);
				break;
			case BoolExpKind::Const_BoolEK:
				fprintf(AST_File, " bool(%d) ", tree->attr.bval);
				break;
			case BoolExpKind::ConOp_BoolEK:
				tran2opcode(tree->attr.op);
				fprintf(AST_File, " %s ", op_code);
				break;
			default:
				break;
			}
		}
		if (NULL != tree) {
			for (int i = 0; i < MAX_TREENODE_CHILD_NUM; i++) {
				if (NULL != tree->child[i])
					printAST(tree->child[i]);
			}
			if (tree->nodekind == NodeKind::StmtK && tree->kind.stmt == StmtKind::Seq_StmtK) {
				fprintf(AST_File, "\n<<<Stmt Sequence End  \n");
			}
			tree = tree->sibling;
		}
	}
}







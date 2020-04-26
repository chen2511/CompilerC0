#include "ast.h"
#pragma warning(disable:4996)

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
	// attr vec type 不做初始化
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
	// attr vec type 不做初始化
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
	// attr vec type 不做初始化
	t->pfinfo = NULL;

	return t;
}

// 定义函数时，保存函数信息：返回类型和参数表
FuncInfo* newFuncInfo(Type t)
{
	return nullptr;
}

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




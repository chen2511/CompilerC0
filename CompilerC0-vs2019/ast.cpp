#include "ast.h"
#pragma warning(disable:4996)

// ���������ڵ㣺���������������������ȵ�˵�����壬��������������ͣ����ؽڵ�ָ��
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
	//������Ϣ
	t->nodekind = NodeKind::DecK;
	t->kind.dec = kind;
	// attr type ������ʼ��
	t->vec = -1;
	t->pfinfo = NULL;

	return t;
}

// �������ڵ㣺��������������ͣ����ؽڵ�ָ��
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
	// attr vec type ������ʼ��
	t->pfinfo = NULL;

	return t;
}

// �������ʽ�ڵ㣺��������������ͣ����ؽڵ�ָ��
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
	// attr vec type ������ʼ��
	t->pfinfo = NULL;

	return t;
}

// �����������ʽ�ڵ㣺��������������ͣ����ؽڵ�ָ��
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
	// attr vec type ������ʼ��
	t->pfinfo = NULL;

	return t;
}

// ���庯��ʱ�����溯����Ϣ���������ͺͲ�����
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




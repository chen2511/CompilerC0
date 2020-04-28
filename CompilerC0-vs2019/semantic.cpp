#include <string>
#include "semantic.h"


static bool isGlobal = true;

void semanticExp(TreeNode* tree);
int checkParaNum(TreeNode* list);


// ��� �������� �����б� �����ظ���
int checkParaNum(TreeNode* list) {
	// �����б���һ�����ʽ�б������ÿһ�����ʽ��Ҫ���
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
		// �ǳ����� ʲô������
	}
	else if (ExpKind::Iden_ExpK == tree->kind.exp) {
		Symbol* lookup = lookUp_SymTab(tree->attr.name);

		if (lookup == NULL) {
			printf("Error in line %d : ��ʶ��δ���壺%s\n", tree->lineno, tree->attr.name);
			return;
		}

		if (lookup->vec >= 0) {				// �����飬 û���±�����
			printf("Error in line %d : ��ʶ�������飬��Ҫ�����±����㣺%s\n", tree->lineno, tree->attr.name);
			return;
		}

	}
	else {				//ExpKind::Op_ExpK
		if (tree->attr.op == TokenType::CALL) {				// �������� Ҫ��飺�����Ƿ��������Լ����������Ƿ�ƥ�䣬�Ƿ��з���ֵ
			Symbol* lookup = lookUp_SymTab(tree->child[0]->attr.name);

			if (lookup == NULL) {
				printf("Error in line %d : ����δ����: %s()\n", tree->lineno, tree->child[0]->attr.name);
				return;
			}

			// ��鷵��ֵ
			if (Type::T_VOID == lookup->valueType) {
				printf("Error in line %d : �����޷���ֵ,  %s()\n", tree->lineno, lookup->name);
				return;
			}

			// ������������ ͬʱ���ÿ�������ı��ʽ�Ƿ�Ϸ�
			if (lookup->pfinfo->paranum != checkParaNum(tree->child[1])) {
				printf("Error in line %d : ����������ƥ��, %s()\n", tree->lineno, lookup->name);
				return;
			}

		}
		else if (tree->attr.op == TokenType::ARRAYAT) {			// �±����㣬����ʶ���Ƿ������飬 ���Ҽ��indexλ�õı��ʽ���������Խ��
			Symbol* lookup = lookUp_SymTab(tree->child[0]->attr.name);

			if (lookup == NULL) {
				printf("Error in line %d : ��ʶ��δ���壺%s\n", tree->lineno, tree->child[0]->attr.name);
				return;
			}

			if (lookup->vec < 0) {
				printf("Error in line %d : ��ʶ���������飬���ܽ����±����㣺%s\n", tree->lineno, tree->child[0]->attr.name);
				return;
			}

			semanticExp(tree->child[1]);
		}
		else {							// +-*/ ������ҽڵ�
			semanticExp(tree->child[0]);
			semanticExp(tree->child[1]);
		}
	}
}


void semanticBoolExp(TreeNode* tree) {
	if (tree == NULL)
		return;
	// bool Exp ������ 4 �ֽڵ㣺 ����op��������������ϵop�����ʽ
	if (tree->nodekind == NodeKind::ExpK) {
		semanticExp(tree);
	}
	else {
		if (tree->kind.bexp == BoolExpKind::ConOp_BoolEK) {			// ���ʽ ConOp ���ʽ
			semanticExp(tree->child[0]);
			semanticExp(tree->child[1]);
		}
		else if (tree->kind.bexp == BoolExpKind::Const_BoolEK) {
			// ���� �������м��		true��false
		}
		else {  // BoolExpKind::Op_BoolEK�� ����
			semanticBoolExp(tree->child[0]);
			semanticBoolExp(tree->child[1]);
		}
	}
}

// �ݹ����AST���������ű����������ͼ��
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
			case DecKind::Const_DefK:			// ��������
				ttt = tree->type;
				st = tree->child[0];			// ������ �����б�
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
				// �����������
				semanticAnalyze(tree->sibling);
				// ���������ݹ飻 ��ʵ����֮ǰ�����﷨������Ӧ�öඨ��һ�����ͣ��Ͳ��������������ˣ��������۲���
				return;
			case DecKind::Var_DecK:
				break;
			case DecKind::Var_DefK:					// ��������
				ttt = tree->type;
				st = tree->child[0];
				
				while (NULL != st) {
					insert_SymTab(isGlobal, tree->lineno, st->attr.name, IDType::Var_ID, ttt, (int)INFINITY, st->vec);

					st = st->sibling;
				}
				// �����������
				semanticAnalyze(tree->sibling);
				
				return;
			case DecKind::Func_DecK:					// ��������
				// ��ʼ���庯���� ����������ֻ�ܽ��뺯����
				isGlobal = false;

				// ��������������ȫ�ֱ�
				insert_SymTab(true, tree->lineno, tree->attr.name, IDType::Func_ID, tree->pfinfo->rettype, (int)INFINITY, -1, tree->pfinfo);

				// �½�������
				tmp = initSimpleSymTable(tree->attr.name);
				tmp->next = g_symtab->next;
				g_symtab->next = tmp;
				// ���������
				for (int i = 0; i < tree->pfinfo->paranum; i++) {
					insert_SymTab(false, tree->lineno, tree->pfinfo->paratable[i].pname, IDType::Para_ID, tree->pfinfo->paratable[i].ptype,
						(int)INFINITY);
				}
				break;
			case DecKind::MainFunc_DecK:
				// ��ʼ���庯���� ����������ֻ�ܽ��뺯����
				isGlobal = false;

				insert_SymTab(true, tree->lineno, (char*)("main"), IDType::Func_ID, Type::T_VOID, (int)INFINITY, -1);

				// �½�������
				
				tmp = initSimpleSymTable((char*)("main"));
				tmp->next = g_symtab->next;
				g_symtab->next = tmp;
				break;
			default:
				break;
			}
		}
		else if (NodeKind::StmtK == tree->nodekind) {		// �˴���һЩ�ش�bug�� case�����дreturn����Ѻ�����ֵܽڵ�����
			switch (tree->kind.stmt)
			{
			case StmtKind::Assign_StmtK:					// ��ֵ���
			{
				lookup = lookUp_SymTab(tree->attr.name);
				if (NULL == lookup) {				// δ����
					printf("Error in line %d : ��ʶ��δ���壺%s\n", tree->lineno, tree->attr.name);
				}
				else {								// �Ѷ���
					// ��ֵ��� ������� 
					tree->type = lookup->valueType;

					if (lookup->type == IDType::Const_ID) {		// ����
						printf("Error in line %d : ��ʶ���ǳ��������ܸ�ֵ��%s\n", tree->lineno, tree->attr.name);
						// ����ɨ����棬��������
					}
					if (NULL != tree->child[1]) {
						if (lookup->vec <= 0) {				// �������飬�����±�����
							printf("Error in line %d : ��ʶ���������飬���ܽ����±����㣺%s\n", tree->lineno, tree->attr.name);
						}
						// �����ǲ������飬����� index���ʽ
						// semanticExp(tree->child[1]);
					}
					else {
						if (lookup->vec >= 0) {				// �����飬 û���±�����
							printf("Error in line %d : ��ʶ�������飬��Ҫ�����±����㣺%s\n", tree->lineno, tree->attr.name);
						}
					}
				}
				// �Ҳ���ʽ
				//semanticExp(tree->child[0]);

				// �����ĩβ�� ����child�ڵ�
			}
				break;
			case StmtKind::If_StmtK:
				break;
			case StmtKind::While_StmtK:
				break;
			case StmtKind::For_StmtK:
				break;
			case StmtKind::Call_StmtK:			// �޷���ֵ��������
			{
				lookup = lookUp_SymTab(tree->attr.name);
				if (NULL == lookup) {					// δ����
					printf("Error in line %d : ����δ���壺%s()\n", tree->lineno, tree->attr.name);
					//return;
				}
				else {									// �Ѷ���
					if (IDType::Func_ID != lookup->type) {			// ���Ǻ���
						printf("Error in line %d : �ñ�ʶ�����Ǻ�����%s()\n", tree->lineno, tree->attr.name);
						//return;
					}
					else {											// �Ǻ�����Ҫ������
						// ������������ ͬʱ���ÿ�������ı��ʽ�Ƿ�Ϸ�
						if (lookup->pfinfo->paranum != checkParaNum(tree->child[0])) {
							printf("Error in line %d : ����������ƥ��: %s()\n", tree->lineno, lookup->name);
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
					printf("Error in line %d : ��ʶ��δ���壺%s\n", tree->lineno, tree->attr.name);
					// return;
				}
				else {
					if (lookup->type == IDType::Const_ID) {		// ����
						printf("Error in line %d : ��ʶ���ǳ��������ܶ��룺%s\n", tree->lineno, tree->attr.name);
						// ����ɨ����棬��������
					}
				}
			}
				break;
			case StmtKind::Write_StmtK:
			{
				if (NULL == tree->child[0]) {			// ֻ�б��ʽ��û���ַ����� ֱ���������¼��
					break;
				}
				std::string printstring = tree->child[0]->attr.name;
				size_t pos;
				pos = printstring.find("%d");
				if (pos != std::string::npos) {			// �ҵ� %d
					tree->type = Type::T_INTEGER;

					if (NULL == tree->child[1]) {		// û�б��ʽ
						printf("Error in line %d : ռ�÷���ƥ��: d \n", tree->lineno);
						//return;
					}
					else {								// �����ʽ		��Ϊ�ݹ��飬���ﲻ���
						//	semanticExp(tree->child[1]);
					}

					break;		//����������
				}

				pos = printstring.find("%c");
				if (pos != std::string::npos) {			// �ҵ� %c
					tree->type = Type::T_CHAR;

					if (NULL == tree->child[1]) {		// û�б��ʽ
						printf("Error in line %d : ռ�÷���ƥ��: c \n", tree->lineno);
						//return;
					}
					else {								// �����ʽ   
						//	semanticExp(tree->child[1]);
					}

					break;		//����������
				}

				// ��û��%c Ҳû��%d
				if (NULL != tree->child[1]) {		// ȴ�б��ʽ
					printf("Error in line %d : ȱ��ռ�÷� \n", tree->lineno);
					//return;
				}
			}
				break;
			case StmtKind::Write_StmtK_Str:			// ������������ ��һ�� ������
				break;
			case StmtKind::Ret_StmtK:
				break;
			default:
				break;
			}
		}
		else if (NodeKind::ExpK == tree->nodekind) {			// ��Ϊ���ʽ������������û���ֵܽڵ㣬ֱ�ӷ���
			// ����������ʽ
			semanticExp(tree);
			return;
		}
		else // �������ʽ
		{
			semanticBoolExp(tree);
			return;
		}

		for (int i = 0; i < MAX_TREENODE_CHILD_NUM; i++) {
			if (NULL != tree->child[i])
				semanticAnalyze(tree->child[i]);
		}
		// �˴��ӽڵ� �������� Ҫִ�еĴ���
		tree = tree->sibling;
	}
}

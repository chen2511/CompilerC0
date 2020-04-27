#include "semantic.h"

static bool isGlobal = true;

// �ݹ����AST���������ű����������ͼ��
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
			// ������ ԭ����˳��ȽϺã� �����Ļ� ������û�����ţ�������
		}
		else // �������ʽ
		{

		}

		for (int i = 0; i < MAX_TREENODE_CHILD_NUM; i++) {
			if (NULL != tree->child[i])
				semanticAnalyze(tree->child[i]);
		}
		// �˴��ӽڵ� �������� Ҫִ�еĴ���
		tree = tree->sibling;
	}
}

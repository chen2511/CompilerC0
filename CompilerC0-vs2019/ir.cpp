#include "ir.h"
#include "symtab.h"

// ��һ����Ԫʽ�ĵ�ַ
int NXQ = 0;
// ������ʱ����
int tempvarcnt = 0;
//
int labelcnt = 0;
// ��Ԫʽ�б�
Quadvar quadvarlist[MAX_QUADVAR_NUM];

char* stringlist[MAX_STRINGLIST_SIZE];

int str_index = 0;

static char zero[5] = "0\0";

char* newtemp();
int checkParaNum(TreeNode* list, FuncInfo* pf);
char* newempty();
char* newitoa(int a);
char* newlabel();

// ���� ���У���op��char* ���Ƕ�̬�����; NXQ����
void gen(const char* op, char* s1, char* s2, char* s3) {
	strcpy(quadvarlist[NXQ].op, op);
	//fprintf(IR_FILE, "%d:(%s,%s,%s,%s)\n", NXQ, op, s1, s2, s3);

	quadvarlist[NXQ].var1 = s1;
	quadvarlist[NXQ].var2 = s2;
	quadvarlist[NXQ].var3 = s3;

	NXQ++;
}

void backPatch(int p, char* st) {
	//char* st = newitoa(t);
	int q = p;
	while (q)
	{
		int q1 = atoi(quadvarlist[q].var3);
		quadvarlist[q].var3 = st;
		q = q1;
	}
	return;
}

int merge(int p1, int p2) {
	int p;
	if (!p2)
		return p1;
	else {
		p = p2;
		while (quadvarlist[p].var3[0] != '0')
			p = atoi(quadvarlist[p].var3);
		quadvarlist[p].var3 = newitoa(p1);
		return p2;
	}
}

void printIR() {
	for (int i = 0; i < NXQ; i++) {
		fprintf(IR_FILE, "%d:(%s,%s,%s,%s)\n", i, quadvarlist[i].op, quadvarlist[i].var1, quadvarlist[i].var2, quadvarlist[i].var3);
	}
}

 // ��� �������� �����б� �����ظ���
void insertPara(TreeNode* list) {
	// �����б���һ�����ʽ�б������ÿһ�����ʽ��Ҫ���
	TreeNode* slist = list;
	while (NULL != list) {
		IR_Exp(list);
		
		list = list->sibling;
	}
	
	list = slist;
	while (NULL != list) {
		gen("vpara", newempty(), newempty(), list->place);
		list = list->sibling;
	}
	
}

// malloc int תchar*
char* newitoa(int a) {
	char* ss;
	ss = (char*)malloc(20);
	if (NULL != ss) {
		itoa(a, ss, 10);
	}
	else {
		std::cout << "no enough mem" << std::endl;
		exit(-1);
	}
	
	return ss;
}

// �����ַ�������һ����ɳ�ʼ����֮��ֱ�ӷ���
char* newempty() {
	static char* em;
	if (em != NULL) {
		return em;
	}
	em = (char*)malloc(5);
	if (NULL != em) {
		em[0] = ' ';
		em[1] = '\0';
		return em;
	}
	else {
		std::cout << "no enough mem" << std::endl;
		exit(-1);
	}
	
}

// ����һ����ʱ���� $1����
char* newtemp() {
	char* tempvar = (char*)malloc(20);
	if (NULL != tempvar) {
		sprintf(tempvar, "$_%d", tempvarcnt++);
	}
	else {
		std::cout << "no enough mem" << std::endl;
		exit(-1);
	}
	
	return tempvar;
}

// ����label
char* newlabel() {
	char* label = (char*)malloc(sizeof(char) * 10);
	if (NULL != label) {
		sprintf(label, "Label_%d", labelcnt++);
		return label;
	}
	else {
		std::cout << "no enough mem" << std::endl;
		exit(-1);
	}
	
}

void IR_Exp(TreeNode* tree) {
	/*
		bug: printf��Ҫȷ�����ͣ�Ĭ��int
	*/
	tree->type = Type::T_INTEGER;


	if (ExpKind::Num_ExpK == tree->kind.exp) {
		// int ���� char
		char* num = (char*)malloc(20);
		if (NULL != num) {
			itoa(tree->attr.val, num, 10);
			tree->place = num;
		}
		else {
			std::cout << "no enough mem" << std::endl;
			exit(-1);
		}
	}
	else if (ExpKind::Iden_ExpK == tree->kind.exp) {
		tree->place = tree->attr.name;
		/*
			bug: printf��Ҫȷ������
		*/
		/*Symbol* sb = lookUp_SymTab(tree->attr.name);
		tree->type = sb->valueType;*/

	}
	else {				//ExpKind::Op_ExpK
		// ֻ��Op��λ�òŻ�������Ԫʽ�����Ǽ����м����
		if (tree->attr.op == TokenType::CALL) {				// �������� Ҫ��飺�����Ƿ��������Լ����������Ƿ�ƥ�䣬�Ƿ��з���ֵ
			IR_Exp(tree->child[0]);
			insertPara(tree->child[1]);
			tree->place = newtemp();
			gen("callret", tree->child[0]->place, newempty(), tree->place);
		}
		else if (tree->attr.op == TokenType::ARRAYAT) {			// �±����㣬����ʶ���Ƿ������飬 ���Ҽ��indexλ�õı��ʽ���������Խ��
			IR_Exp(tree->child[0]);
			IR_Exp(tree->child[1]);

			tree->place = newtemp();
			gen("getarray", tree->child[0]->place, tree->child[1]->place, tree->place);

		}
		else {							// +-*/ ������ҽڵ�
			IR_Exp(tree->child[0]);
			IR_Exp(tree->child[1]);
			tree->place = newtemp();
			switch (tree->attr.op)
			{
			case TokenType::PLUS:
				gen("+", tree->child[0]->place, tree->child[1]->place, tree->place);
				break;
			case TokenType::MINU:
				gen("-", tree->child[0]->place, tree->child[1]->place, tree->place);
				break;
			case TokenType::MULT:
				gen("*", tree->child[0]->place, tree->child[1]->place, tree->place);
				break;
			case TokenType::DIV:
				gen("/", tree->child[0]->place, tree->child[1]->place, tree->place);
				break;
			default:
				break;
			}
		}
	}
}


void IR_BoolExp(TreeNode* tree) {
	// bool Exp ������ 4 �ֽڵ㣺 ����op��������������ϵop�����ʽ
	if (tree->nodekind == NodeKind::ExpK) {
		IR_Exp(tree);
		tree->TC = NXQ;
		tree->FC = NXQ + 1;
		gen("jnz", tree->place, newempty(), zero);
		gen("j", newempty(), newempty(), zero);
	}
	else {
		if (tree->kind.bexp == BoolExpKind::ConOp_BoolEK) {			// ���ʽ ConOp ���ʽ
			IR_Exp(tree->child[0]);
			IR_Exp(tree->child[1]);
			tree->TC = NXQ;
			tree->FC = NXQ + 1;
			switch (tree->attr.op)
			{
			case TokenType::LSS:
				gen("<", tree->child[0]->place, tree->child[1]->place, zero);
				break;
			case TokenType::LEQ:
				gen("<=", tree->child[0]->place, tree->child[1]->place, zero);
				break;
			case TokenType::GRE:
				gen(">", tree->child[0]->place, tree->child[1]->place, zero);
				break;
			case TokenType::GEQ:
				gen(">=", tree->child[0]->place, tree->child[1]->place, zero);
				break;
			case TokenType::EQL:
				gen("==", tree->child[0]->place, tree->child[1]->place, zero);
				break;
			case TokenType::NEQ:
				gen("!=", tree->child[0]->place, tree->child[1]->place, zero);
				break;
			default:
				break;
			}
			gen("j", newempty(), newempty(), zero);
		}
		else if (tree->kind.bexp == BoolExpKind::Const_BoolEK) {
			// ���� ,��exp��ͬ		true��false
			char* tt;

			tt = (char*)malloc(5);
			tt[0] = (int)tree->attr.bval + '0';
			tt[1] = 0;

			tree->place = tt;
			tree->TC = NXQ;
			tree->FC = NXQ + 1;
			gen("jnz", tree->place, newempty(), zero);
			gen("j", newempty(), newempty(), zero);
		}
		else {  // BoolExpKind::Op_BoolEK�� ����
			if (TokenType::NOT == tree->attr.op) {
				IR_BoolExp(tree->child[0]);
				tree->FC = tree->child[0]->TC;
				tree->TC = tree->child[0]->FC;
			}
			else if (TokenType::AND == tree->attr.op) {
				IR_BoolExp(tree->child[0]);
				char* ll = newlabel();
				gen("lab", newempty(), newempty(), ll);
				backPatch(tree->child[0]->TC, ll);
				tree->FC = tree->child[0]->FC;

				IR_BoolExp(tree->child[1]);
				tree->TC = tree->child[1]->TC;
				tree->FC = merge(tree->child[0]->FC, tree->child[1]->FC);
			}
			else {
				IR_BoolExp(tree->child[0]);
				char* ll = newlabel();
				gen("lab", newempty(), newempty(), ll);
				// �������˴�������û������ģ���Ϊ������϶�����ǰ����Ԫʽ�������ž���NXQ��
				// ֻ��������newlabel������NXQ��Ҳ����ȫ��ת�����label�ĵط����϶��ǲ������˳���ϵ�����
				backPatch(tree->child[0]->FC, ll);
				tree->TC = tree->child[0]->TC;

				IR_BoolExp(tree->child[1]);
				tree->FC = tree->child[1]->FC;
				tree->TC = merge(tree->child[0]->TC, tree->child[1]->TC);
			}
			
		}
	}
}

// �ݹ����AST���������ű����������ͼ��
// ������ʽ�Ƚϸ��ӣ�
// ���򣺱��ʽ���������ʽ
// ������䣬�к��ӵ����һ��continue
void IR_Analyze(TreeNode* tree)
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
			{
				IR_Analyze(tree->child[0]);
			}
			break;
			case DecKind::Const_DefK:			// ��������
			{
				TreeNode* idlist = tree->child[0];

				char* name;
				int val;

				while (NULL != idlist) {
					name = idlist->attr.name;
					if (Type::T_INTEGER == tree->type) {
						val = idlist->child[0]->attr.val;
						gen("const", (char*)"int", newitoa(val), name);
					}
					else {
						val = idlist->child[0]->attr.cval;
						gen("const", (char*)"char", newitoa(val), name);
					}
					idlist = idlist->sibling;
				}
			}
			break;
			case DecKind::Var_DecK:
			{
				IR_Analyze(tree->child[0]);
			}
			break;
			case DecKind::Var_DefK:					// ��������
			{
				TreeNode* idlist = tree->child[0];
				char* name;
				while (NULL != idlist) {
					name = idlist->attr.name;
					if (-1 == idlist->vec) {		// not vec
						gen((Type::T_INTEGER == tree->type) ? "int" : "char", newempty(), newempty(), name);
					}
					else {						// vec
						gen((Type::T_INTEGER == tree->type) ? "intarray" : "chararray", newitoa(idlist->vec), newempty(), name);
					}
					idlist = idlist->sibling;
				}
			}
			break;
			case DecKind::Func_DecK:					// ��������
			{
				// �˴�bug�� tree->type  >> tree->pfinfo->rettype
				gen("Func", 
					(tree->pfinfo->rettype == Type::T_INTEGER) ? (char*)"int" : (tree->pfinfo->rettype == Type::T_CHAR) ? (char*)"char" : (char*)"void",
					newempty(),
					tree->attr.name
				);

				FuncInfo* pf = tree->pfinfo;
				for (int i = 0; i < pf->paranum; i++) {
					gen("para",
						(Type::T_INTEGER == pf->paratable[i].ptype) ? (char*)"int" : (char*)"char",
						newempty(),
						pf->paratable[i].pname
					);
				}

				IR_Analyze(tree->child[0]);
				gen("endf", newempty(), newempty(), newempty());
			}
			break;
			case DecKind::MainFunc_DecK:
			{
				gen("Main", newempty(), newempty(), newempty());
				IR_Analyze(tree->child[0]);
				gen("endf", newempty(), newempty(), newempty());
			}
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
				if (tree->child[1]) {		// ��������
					IR_Exp(tree->child[0]);
					IR_Exp(tree->child[1]);

					gen("setarray", tree->child[0]->place, tree->child[1]->place, tree->attr.name);
				}
				else {
					IR_Exp(tree->child[0]);
					gen("assign", tree->child[0]->place, newempty(), tree->attr.name);
				}
			}
			break;
			case StmtKind::If_StmtK:
			{
				IR_BoolExp(tree->child[0]);
				char* truelabel = newlabel();
				char* falselabel = newlabel();

				char* conlabel = newlabel();

				gen("lab", newempty(), newempty(), truelabel);
				backPatch(tree->child[0]->TC, truelabel);

				IR_Analyze(tree->child[1]);
				gen("j", newempty(), newempty(), conlabel);

				gen("lab", newempty(), newempty(), falselabel);
				backPatch(tree->child[0]->FC, falselabel);
				IR_Analyze(tree->child[2]);

				gen("lab", newempty(), newempty(), conlabel);
			}
			break;
			case StmtKind::While_StmtK:
			{
				char* looplabel = newlabel();
				gen("lab", newempty(), newempty(), looplabel);
				IR_BoolExp(tree->child[0]);

				char* truelabel = newlabel();
				char* falselabel = newlabel();

				gen("lab", newempty(), newempty(), truelabel);
				backPatch(tree->child[0]->TC, truelabel);
				// ѭ����
				IR_Analyze(tree->child[1]);
				gen("j", newempty(), newempty(), looplabel);


				gen("lab", newempty(), newempty(), falselabel);
				backPatch(tree->child[0]->FC, falselabel);
			}
			break;
			case StmtKind::For_StmtK:
			{
				// ��ʼ��ֵ���
				IR_Analyze(tree->child[1]);
				// ѭ���忪ʼ
				char* looplabel = newlabel();
				gen("lab", newempty(), newempty(), looplabel);

				// �������ʽ
				IR_BoolExp(tree->child[0]);
				
				char* truelabel = newlabel();
				char* falselabel = newlabel();
				gen("lab", newempty(), newempty(), truelabel);
				backPatch(tree->child[0]->TC, truelabel);
				// ѭ�������
				IR_Analyze(tree->child[2]);
				// �����������ж�
				gen("j", newempty(), newempty(), looplabel);

				// ����ѭ����
				gen("lab", newempty(), newempty(), falselabel);
				backPatch(tree->child[0]->FC, falselabel);

			}
			break;
			case StmtKind::Call_StmtK:			// �޷���ֵ��������
			{
				insertPara(tree->child[0]);
				
				gen("call", tree->attr.name, newempty(), newempty());
			}
			break;
			case StmtKind::Seq_StmtK:
				IR_Analyze(tree->child[0]);
				break;
			case StmtKind::Read_StmtK:
			{
				TreeNode* idlist = tree->child[0];
				while (idlist != NULL) {
					gen("scanf", newempty(), newempty(), idlist->attr.name);
					idlist = idlist->sibling;
				}
			}
			break;
			case StmtKind::Read_StmtK_Idlist:
			break;
			case StmtKind::Write_StmtK:
			{
				//char* pstr = newempty();
				if (NULL != tree->child[0]) {		// ��Str
					stringlist[str_index] = tree->child[0]->attr.str;
					if (NULL != tree->child[1]) {		// ��Exp
						IR_Exp(tree->child[1]);
						gen("print", newitoa(str_index), tree->child[1]->place, (tree->type == Type::T_INTEGER) ? (char*)"int" : (char*)"char");
					}
					else {
						gen("print", newitoa(str_index), newempty(), newempty());
					}
					str_index++;
				}
				else {								// û��Str
					if (NULL != tree->child[1]) {		// ��Exp
						IR_Exp(tree->child[1]);
						gen("print", newempty(), tree->child[1]->place, (tree->child[1]->type == Type::T_INTEGER) ? (char*)"int" : (char*)"char");
					}
				}
			}
			break;
			case StmtKind::Write_StmtK_Str:			// ������������ ��һ�� ������
				break;
			case StmtKind::Ret_StmtK:
				if (NULL != tree->child[0]) {
					IR_Exp(tree->child[0]);
					gen("ret", newempty(), newempty(), tree->child[0]->place);
				}
				else {
					gen("ret", newempty(), newempty(), newempty());
				}
				tree = tree->sibling;
				continue;
			default:
				break;
			}
		}
		else if (NodeKind::ExpK == tree->nodekind) {			// ��Ϊ���ʽ������������û���ֵܽڵ㣬ֱ�ӷ���
			// ����������ʽ
			IR_Exp(tree);
			return;
		}
		else // �������ʽ
		{
			IR_BoolExp(tree);
			return;
		}



		//for (int i = 0; i < MAX_TREENODE_CHILD_NUM; i++) {
		//	if (NULL != tree->child[i])
		//		IR_Analyze(tree->child[i]);
		//}

		tree = tree->sibling;
	}
}



#include "ir.h"
#include "symtab.h"

// 下一个四元式的地址
int NXQ = 0;
// 生成临时变量
int tempvarcnt = 0;
//
int labelcnt = 0;
// 四元式列表
Quadvar quadvarlist[MAX_QUADVAR_NUM];

char* stringlist[MAX_STRINGLIST_SIZE];

int str_index = 0;

static char zero[5] = "0\0";

char* newtemp();
int checkParaNum(TreeNode* list, FuncInfo* pf);
char* newempty();
char* newitoa(int a);
char* newlabel();

// 插入 序列；除op，char* 都是动态分配的; NXQ自增
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
	for (int i = 0; i < NXQ - 1; i++) {
		fprintf(IR_FILE, "%d:(%s,%s,%s,%s)\n", i, quadvarlist[i].op, quadvarlist[i].var1, quadvarlist[i].var2, quadvarlist[i].var3);
	}
}

 // 检查 函数调用 参数列表 ，返回个数
void insertPara(TreeNode* list) {
	// 参数列表是一个表达式列表，里面的每一个表达式都要检查
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

// malloc int 转char*
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

// 填充空字符串；第一次完成初始化；之后直接返回
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

// 返回一个临时变量 $1……
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

// 返回label
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
		bug: printf需要确定类型，默认int
	*/
	tree->type = Type::T_INTEGER;


	if (ExpKind::Num_ExpK == tree->kind.exp) {
		// int 或者 char
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
			bug: printf需要确定类型
		*/
		/*Symbol* sb = lookUp_SymTab(tree->attr.name);
		tree->type = sb->valueType;*/

	}
	else {				//ExpKind::Op_ExpK
		// 只有Op的位置才会生成四元式；都是计算中间变量
		if (tree->attr.op == TokenType::CALL) {				// 函数调用 要检查：函数是否声明，以及参数个数是否匹配，是否有返回值
			IR_Exp(tree->child[0]);
			insertPara(tree->child[1]);
			tree->place = newtemp();
			gen("callret", tree->child[0]->place, newempty(), tree->place);
		}
		else if (tree->attr.op == TokenType::ARRAYAT) {			// 下标运算，检查标识符是否是数组， 并且检查index位置的表达式；但不检查越界
			IR_Exp(tree->child[0]);
			IR_Exp(tree->child[1]);

			tree->place = newtemp();
			gen("getarray", tree->child[0]->place, tree->child[1]->place, tree->place);

		}
		else {							// +-*/ 检查左右节点
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
	// bool Exp 里面有 4 种节点： 布尔op、布尔常量、关系op、表达式
	if (tree->nodekind == NodeKind::ExpK) {
		IR_Exp(tree);
		tree->TC = NXQ;
		tree->FC = NXQ + 1;
		gen("jnz", tree->place, newempty(), zero);
		gen("j", newempty(), newempty(), zero);
	}
	else {
		if (tree->kind.bexp == BoolExpKind::ConOp_BoolEK) {			// 表达式 ConOp 表达式
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
			// 常量 ,与exp相同		true、false
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
		else {  // BoolExpKind::Op_BoolEK： 与或非
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
				// 分析：此处回填是没有问题的，因为：回填肯定是填前面四元式，填的序号就是NXQ，
				// 只不过这里newlabel，代替NXQ，也就是全部转到这个label的地方，肯定是不会出现顺序上的问题
				backPatch(tree->child[0]->FC, ll);
				tree->TC = tree->child[0]->TC;

				IR_BoolExp(tree->child[1]);
				tree->FC = tree->child[1]->FC;
				tree->TC = merge(tree->child[0]->TC, tree->child[1]->TC);
			}
			
		}
	}
}

// 递归遍历AST，建立符号表，并进行类型检查
// 遍历方式比较复杂：
// 后序：表达式、布尔表达式
// 中序：语句，有孩子的语句一般continue
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
			case DecKind::Const_DefK:			// 常量定义
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
			case DecKind::Var_DefK:					// 变量定义
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
			case DecKind::Func_DecK:					// 函数定义
			{
				// 此处bug： tree->type  >> tree->pfinfo->rettype
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
		else if (NodeKind::StmtK == tree->nodekind) {		// 此处有一些重大bug， case里面别写return；会把后面的兄弟节点跳过
			switch (tree->kind.stmt)
			{
			case StmtKind::Assign_StmtK:					// 赋值语句
			{
				if (tree->child[1]) {		// 赋给数组
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
				// 循环体
				IR_Analyze(tree->child[1]);
				gen("j", newempty(), newempty(), looplabel);


				gen("lab", newempty(), newempty(), falselabel);
				backPatch(tree->child[0]->FC, falselabel);
			}
			break;
			case StmtKind::For_StmtK:
			{
				// 初始赋值语句
				IR_Analyze(tree->child[1]);
				// 循环体开始
				char* looplabel = newlabel();
				gen("lab", newempty(), newempty(), looplabel);

				// 布尔表达式
				IR_BoolExp(tree->child[0]);
				
				char* truelabel = newlabel();
				char* falselabel = newlabel();
				gen("lab", newempty(), newempty(), truelabel);
				backPatch(tree->child[0]->TC, truelabel);
				// 循环体语句
				IR_Analyze(tree->child[2]);
				// 无条件跳回判断
				gen("j", newempty(), newempty(), looplabel);

				// 跳出循环体
				gen("lab", newempty(), newempty(), falselabel);
				backPatch(tree->child[0]->FC, falselabel);

			}
			break;
			case StmtKind::Call_StmtK:			// 无返回值函数调用
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
				if (NULL != tree->child[0]) {		// 有Str
					stringlist[str_index] = tree->child[0]->attr.str;
					if (NULL != tree->child[1]) {		// 有Exp
						IR_Exp(tree->child[1]);
						gen("print", newitoa(str_index), tree->child[1]->place, (tree->type == Type::T_INTEGER) ? (char*)"int" : (char*)"char");
					}
					else {
						gen("print", newitoa(str_index), newempty(), newempty());
					}
					str_index++;
				}
				else {								// 没有Str
					if (NULL != tree->child[1]) {		// 有Exp
						IR_Exp(tree->child[1]);
						gen("print", newempty(), tree->child[1]->place, (tree->child[1]->type == Type::T_INTEGER) ? (char*)"int" : (char*)"char");
					}
				}
			}
			break;
			case StmtKind::Write_StmtK_Str:			// 并不会进入这里， 上一级 处理了
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
		else if (NodeKind::ExpK == tree->nodekind) {			// 因为表达式单独处理，并且没有兄弟节点，直接返回
			// 单独处理表达式
			IR_Exp(tree);
			return;
		}
		else // 布尔表达式
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



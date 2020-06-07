/*
	�ݹ��½��ӳ���ʵ���﷨�����������ɳ����﷨��
*/

#include "global.h"
#include "parser.h"
#include "scan.h"
#include "ast.h"
#include "errorprocess.h"

using namespace std;

static bool b_functionDeclaration = false;
static bool b_mainFunction = false;
static int flashBackIndex;
static int flashBackLineNum;

static bool match(TokenType expectToken);

//Ϊÿһ�����ս������һ������
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

// ������ʼ�����ó���
TreeNode* parser() {
	TreeNode* synaxTree;

	getNextToken();
	//�﷨������ʼ
	//����ʱ������������������
	synaxTree = program();

	if (!EOF_flag) {
		cout << "error: not EOF" << endl;
	}

	if (PRINT_AST_TOFILE) {
		
		printAST(synaxTree);
	}

	return synaxTree;
}

// ƥ�� �ڴ��� token�����򱨴�
// ��ȡ��һ��token
static bool match(TokenType expectToken) {
	if (expectToken == g_token.opType) {	// ��Ԥ����ͬ������token�� true
		getNextToken();
		return true;
	}
	else {								// ��ʾ���󣬵������ﲻ����
		printf("match error in line %d :\t\texpect Token %d, but %d value: %s \n", g_lineNumber, expectToken, g_token.opType, g_token.value);
		return false;
	}
}

/*
13.<����> ::= ��<����˵��>�ݣ�<����˵��>��{<�з���ֵ��������>|<�޷���ֵ��������>}<������>
�����кܶ�Ĺ����ӣ�����Ϊ���ķ��򵥣�����⣻���ڳ�����ʵ�֣�û����ȡ
*/
TreeNode* program() {
	TreeNode* t = NULL;
	TreeNode* p = NULL;									// ����t�� p�м䴦��
	
	

	// ��<����˵��>��
	if (TokenType::CONST == g_token.opType) {
		t = constDeclaration();
		p = t;
	}


	// ��<����˵��>��

	// ��ѡ��������� int �� char ��ֱ��������û�б������壩��������Ҫ��һ���ж��з���ֵ��������
	if (TokenType::INT == g_token.opType || TokenType::CHAR == g_token.opType) {
		flashBackIndex = g_lexBegin;
		flashBackLineNum = g_lineNumber;

		//��Ϊtoken��ȡ��ֵ��ȫ����������ָ�룬��ǰһ��ָ�뱣�漴��
		//����token��ʼλ�ã�����ʶ��int�Ŀ�ʼλ�ã���ΪĩβҪ��ȡһ��token
		getNextToken();
		getNextToken();
		//Ԥ������token����һ���ж�
		if (TokenType::LPARENTHES == g_token.opType) {	//�з���ֵ�������壬��С����
			b_functionDeclaration = true;
		}
		else {											//����˵��				
			if (TokenType::COMMA == g_token.opType) {			// ����

			}
			else if (TokenType::SEMICOLON == g_token.opType) { // �ֺ�

			}
			else if (TokenType::LBRACKET == g_token.opType) {	// ��������

			}
			else {											//����								
				printf("error in program() :in line %d ,tokenType %d value: %s \n", g_lineNumber, g_token.opType, g_token.value);
			}

		}
		// ����
		g_forward = flashBackIndex;
		g_lineNumber = flashBackLineNum;
		getNextToken();

		if (b_functionDeclaration) {
			//�Ǻ������壺ʲô��������������������һ����
		}
		else {
			// �������˵��
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

	// {<�з���ֵ��������>|<�޷���ֵ��������>}
	// ǰ��ı��밴˳����������˾Ͳ��ܶ����ˡ�����������˳��
	while ((TokenType::INT == g_token.opType || TokenType::CHAR == g_token.opType || TokenType::VOID == g_token.opType) && !b_mainFunction) {
		if (TokenType::INT == g_token.opType || TokenType::CHAR == g_token.opType) { //�з���ֵ�Ķ���
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
		else if (TokenType::VOID == g_token.opType) {		// �п������޷���ֵ�������壬Ҳ�п����� ������
			flashBackIndex = g_lexBegin;
			flashBackLineNum = g_lineNumber;
			getNextToken();

			if (TokenType::MAIN == g_token.opType) {
				b_mainFunction = true;
			}
			// ����
			g_forward = flashBackIndex;
			g_lineNumber = flashBackLineNum;
			getNextToken();

			// �����Ƿ���������־ ѡ��
			if (b_mainFunction) {
				// b_mainFunction ��־�Ѹı䣬������ѭ��������������
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

	// <������>
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
14.<����˵��> ::= const<��������>;{ const<��������>;}
*/
TreeNode* constDeclaration() {
	match(TokenType::CONST);

	TreeNode* t = newDecNode(DecKind::Const_DecK);
	TreeNode* p = constDefine();
	t->child[0] = p;

	if (!match(TokenType::SEMICOLON)) {
		errorProcess(ErrorType::LACK_SEMI_CST);

		p->error = true;
	}
	

	// ���const ��
	while (TokenType::CONST == g_token.opType) {
		match(TokenType::CONST);

		TreeNode* q = constDefine();
		p->sibling = q;
		p = q;

		if (!match(TokenType::SEMICOLON)) {
			errorProcess(ErrorType::LACK_SEMI_CST);

			q->error = true;
		}
	}

	return t;
}

//15. < �������� > :: = int<��ʶ��>��<����>{, <��ʶ��>��<����>} | char<��ʶ��>��<�ַ�>{, <��ʶ��>��<�ַ�>}
TreeNode* constDefine() {
	TreeNode* t = newDecNode(DecKind::Const_DefK);					// ��������ڵ�
	//int
	if (TokenType::INT == g_token.opType) {								// �����б�ڵ�
		t->type = Type::T_INTEGER;
		TreeNode* p = newDecNode(DecKind::Const_DefK);
		t->child[0] = p;
		match(TokenType::INT);
		
		
		// �����ʶ��
		p->attr.name = copyString(g_token.value);
		if (!match(TokenType::IDEN)) 
		{
			errorProcess(ErrorType::LACK_ID_CST);
			t->error = true;
			return t;
		}

		if (!match(TokenType::ASSIGN))
		{
			errorProcess(ErrorType::LACK_ASSIGN_CST);
			t->error = true;
			return t;
		}

		//������������
		p->child[0] = signedNum();

		// {, <��ʶ��>��<����>}
		while (TokenType::COMMA == g_token.opType) 
		{
			match(TokenType::COMMA);

			TreeNode* q = newDecNode(DecKind::Const_DefK);
			p->sibling = q;
			q->attr.name = copyString(g_token.value);
			if (!match(TokenType::IDEN))
			{
				errorProcess(ErrorType::LACK_ID_CST);
				t->error = true;
				return t;
			}

			if (!match(TokenType::ASSIGN))
			{
				errorProcess(ErrorType::LACK_ASSIGN_CST);
				t->error = true;
				return t;
			}

			q->child[0] = signedNum();
			p = q;
		}
		
	}
	//char
	else if (TokenType::CHAR == g_token.opType) {
		t->type = Type::T_CHAR;					// �������� ����
		// ������ �����б�
		TreeNode* p = newDecNode(DecKind::Const_DefK);
		t->child[0] = p;
		match(TokenType::CHAR);

		// �����ʶ��
		p->attr.name = copyString(g_token.value);
		if (!match(TokenType::IDEN))
		{
			errorProcess(ErrorType::LACK_ID_CST);
			t->error = true;
			return t;
		}

		if (!match(TokenType::ASSIGN))
		{
			errorProcess(ErrorType::LACK_ASSIGN_CST);
			t->error = true;
			return t;
		}

		p->child[0] = newDecNode(DecKind::Const_DefK);
		//�����ַ�����
		
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

		
		
		//{, <��ʶ��>��<�ַ�>}
		while (TokenType::COMMA == g_token.opType) {
			match(TokenType::COMMA);
			TreeNode* q = newDecNode(DecKind::Const_DefK);
			p->sibling = q;
			q->attr.name = copyString(g_token.value);

			if (!match(TokenType::IDEN))
			{
				errorProcess(ErrorType::LACK_ID_CST);
				t->error = true;
				return t;
			}

			if (!match(TokenType::ASSIGN))
			{
				errorProcess(ErrorType::LACK_ASSIGN_CST);
				t->error = true;
				return t;
			}

			q->child[0] = newDecNode(DecKind::Const_DefK);
			//�����ַ�����
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
		errorProcess(ErrorType::LACK_TYPE_CST);
		t->error = true;
		return t;
	}

	return t;
}

//9.<����> ::= �ۣ�������<�޷�������>
//ֻ�ж���ʱ������������ʱ�򵥶�����
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
	// ����Ǹ��ģ�Ӧ�����������һ������
	TreeNode* t = newDecNode(DecKind::Const_DefK);
	int valoftoken = atoi(g_token.value);
	valoftoken *= sign;
	t->attr.val = valoftoken;
	match(TokenType::NUM);

	return t;
}

//16.<����˵��> ::= <��������>;{<��������>;}
//�����п��ܻ�ͺ�����з���ֵ���������ͻ
TreeNode* varDeclaration() {
	TreeNode* t = newDecNode(DecKind::Var_DecK);

	TreeNode* p = varDefine();
	t->child[0] = p;

	if (!match(TokenType::SEMICOLON))
	{
		errorProcess(ErrorType::LACK_XXX_VARDEF);
		p->error = true;
	}

	// ���������ж� �ظ���һ����ʼ�������壬�����˳������ܼ�����������
	//ѭ����ֹ����������int��char ���޷������������������ ��ʼ��������
	while ((TokenType::INT == g_token.opType || TokenType::CHAR == g_token.opType) && !b_functionDeclaration) {
		flashBackIndex = g_lexBegin;
		flashBackLineNum = g_lineNumber;

		getNextToken();
		getNextToken();
		//Ԥ������token����һ���ж�
		if (TokenType::LPARENTHES == g_token.opType) {	//�з���ֵ�������壬��С����
			b_functionDeclaration = true;
		}
		else {											//����˵��				
			if (TokenType::COMMA == g_token.opType) {			// ����

			}
			else if (TokenType::SEMICOLON == g_token.opType) { // �ֺ�

			}
			else if (TokenType::LBRACKET == g_token.opType) {	// ��������

			}
			else {											//����								
				printf("error in varDeclaration() :in line %d ,tokenType %d value: %s \n", g_lineNumber, g_token.opType, g_token.value);
			}

		}
		// ����
		g_forward = flashBackIndex;
		g_lineNumber = flashBackLineNum;

		getNextToken();


		if (b_functionDeclaration) {							// �з���ֵ�������壺�˳�
			// bug�޸���֮ǰ����֮����һ��void�������˳��������������
			//return nullptr;
			return t;
		}
		else {
			// �����������
			TreeNode* q = varDefine();
			p->sibling = q;
			p = q;
			if (!match(TokenType::SEMICOLON))
			{
				errorProcess(ErrorType::LACK_XXX_VARDEF);
				p->error = true;
			}
		}
	}

	return t;
}

//17.<��������> ::= <���ͱ�ʶ��><��ʶ��>[��[��<�޷�������>��]��] {,<��ʶ��>[��[��<�޷�������>��]��]}
TreeNode* varDefine() {
	TreeNode* t = newDecNode(DecKind::Var_DefK);

	Type _vartype = typeID();
	if (Type::T_ERROR == _vartype)
	{
		//printf("error in varDefine() :in line %d ,tokenType %d value: %s; expected type desc\n", g_lineNumber, g_token.opType, g_token.value);
		errorProcess(ErrorType::LACK_XXX_VARDEF);
		t->error = true;
		return t;
	}
	t->type = _vartype;

	TreeNode* p = newDecNode(DecKind::Var_DefK);
	t->child[0] = p;

	p->attr.name = copyString(g_token.value);
	if (!match(TokenType::IDEN))
	{
		errorProcess(ErrorType::LACK_XXX_VARDEF);
		t->error = true;
		return t;
	}

	if (TokenType::LBRACKET == g_token.opType) {			//���鶨��
		match(TokenType::LBRACKET);
		// �����С
		if (TokenType::NUM == g_token.opType) {
			p->vec = atoi(g_token.value);
			if ('0' == g_token.value[0])
				printf("error in varDefine() :in line %d ,tokenType %d value: %s ;array size is 0 \n", g_lineNumber, g_token.opType, g_token.value);
		}

		if (!match(TokenType::NUM))
		{
			errorProcess(ErrorType::LACK_XXX_VARDEF);
			t->error = true;
			return t;
		}
		if (!match(TokenType::RBRACKET))
		{
			errorProcess(ErrorType::LACK_XXX_VARDEF);
			t->error = true;
			return t;
		}

	}
	while (TokenType::COMMA == g_token.opType) {			//�ظ�����
		match(TokenType::COMMA);

		TreeNode* q = newDecNode(DecKind::Var_DefK);
		q->attr.name = copyString(g_token.value);

		if (!match(TokenType::IDEN))
		{
			errorProcess(ErrorType::LACK_XXX_VARDEF);
			t->error = true;
			return t;
		}
		if (TokenType::LBRACKET == g_token.opType) {
			match(TokenType::LBRACKET);
			if (TokenType::NUM == g_token.opType) {
				q->vec = atoi(g_token.value);
				if ("0" == g_token.value)
					printf("error in varDefine() :in line %d ,tokenType %d value: %s \n", g_lineNumber, g_token.opType, g_token.value);
			}
			if (!match(TokenType::NUM))
			{
				errorProcess(ErrorType::LACK_XXX_VARDEF);
				t->error = true;
				return t;
			}
			if (!match(TokenType::RBRACKET))
			{
				errorProcess(ErrorType::LACK_XXX_VARDEF);
				t->error = true;
				return t;
			}
		}
		p->sibling = q;
		p = q;
	}

	return t;
}

//18.<���ͱ�ʶ��> ::= int | char
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
		return Type::T_ERROR;
	}
}

//19.<�з���ֵ��������> ::= <����ͷ��>��(��<������>��)�� ��{��<�������>��}��
TreeNode* functionDefinitionWithReturn() {
	//DeclarationHead(); 
	// ���㴦������ʹ��
	TreeNode* t = newDecNode(DecKind::Func_DecK);

	Type rettype = typeID();

	if (Type::T_ERROR == rettype)
	{
		//printf("error in varDefine() :in line %d ,tokenType %d value: %s; expected type desc\n", g_lineNumber, g_token.opType, g_token.value);
		errorProcess(ErrorType::LACK_TYPE_FUN);
		t->error = true;
		return t;
	}


	t->attr.name = copyString(g_token.value);
	t->pfinfo = new FuncInfo;
	t->pfinfo->rettype = rettype;
	if (!match(TokenType::IDEN)) 
	{
		errorProcess(ErrorType::LACK_IDEN_FUN);
		t->error = true;
		return t;
	}

	if (!match(TokenType::LPARENTHES))
	{
		errorProcess(ErrorType::LACK_KUOHAO_FUN);
		t->error = true;
		return t;
	}

	paraTable(t->pfinfo);

	if (!match(TokenType::RPARENTHES))
	{
		errorProcess(ErrorType::LACK_KUOHAO_FUN);
		t->error = true;
		return t;
	}

	if (!match(TokenType::LBRACE))
	{
		errorProcess(ErrorType::LACK_KUOHAO_FUN);
		t->error = true;
		return t;
	}


	t->child[0] = complexStatement();

	if (!match(TokenType::RBRACE))
	{
		errorProcess(ErrorType::LACK_KUOHAO_FUN);
		t->error = true;
		return t;
	}

	return t;
}

//20.<����ͷ��> ::= int<��ʶ��> | char<��ʶ��>
// ���㴦������ʹ��
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

//21.<�޷���ֵ��������> ::= void<��ʶ��>��(��<������>��)�� ��{��<�������>��}��
TreeNode* functionDefinitionWithoutReturn() {
	TreeNode* t = newDecNode(DecKind::Func_DecK);

	match(TokenType::VOID);
	t->pfinfo = new FuncInfo;
	t->pfinfo->rettype = Type::T_VOID;

	t->attr.name = copyString(g_token.value);
	if (!match(TokenType::IDEN))
	{
		errorProcess(ErrorType::LACK_IDEN_FUN);
		t->error = true;
		return t;
	}

	if (!match(TokenType::LPARENTHES))
	{
		errorProcess(ErrorType::LACK_KUOHAO_FUN);
		t->error = true;
		return t;
	}
	paraTable(t->pfinfo);
	if (!match(TokenType::RPARENTHES))
	{
		errorProcess(ErrorType::LACK_KUOHAO_FUN);
		t->error = true;
		return t;
	}

	if (!match(TokenType::LBRACE))
	{
		errorProcess(ErrorType::LACK_KUOHAO_FUN);
		t->error = true;
		return t;
	}

	t->child[0] = complexStatement();

	if (!match(TokenType::RBRACE))
	{
		errorProcess(ErrorType::LACK_KUOHAO_FUN);
		t->error = true;
		return t;
	}

	return t;
}

//22.<������> ::= <���ͱ�ʶ��><��ʶ��>{,<���ͱ�ʶ��><��ʶ��>}| <��>
void paraTable(FuncInfo* finfo) {
	// <���ͱ�ʶ��><��ʶ��>
	int cnt = 0;
	if (TokenType::INT == g_token.opType || TokenType::CHAR == g_token.opType) {
		Type _t = typeID();
		finfo->paratable[cnt].ptype = _t;
		finfo->paratable[cnt++].pname = copyString(g_token.value);
		match(TokenType::IDEN);
		// {,<���ͱ�ʶ��><��ʶ��>}
		while (TokenType::COMMA == g_token.opType) {
			match(TokenType::COMMA);
			Type _t = typeID();
			finfo->paratable[cnt].ptype = _t;
			finfo->paratable[cnt++].pname = copyString(g_token.value);
			match(TokenType::IDEN);
		}
	}
	// <��>
	;
	finfo->paranum = cnt;
}

////////////////////////////16'.__<����˵��> ::= <��������>;{<��������>;}
//�����ǵ����� <����˵��>
TreeNode* __varDeclaration() {
	// <��������>;
	TreeNode* t = newDecNode(DecKind::Var_DecK);

	TreeNode* p = varDefine();
	t->child[0] = p;

	match(TokenType::SEMICOLON);
	// {<��������>;}
	while (TokenType::INT == g_token.opType || TokenType::CHAR == g_token.opType) {
		TreeNode* q = varDefine();
		p->sibling = q;
		p = q;
		match(TokenType::SEMICOLON);
	}
	return t;
}

//23.<�������> ::= ��<����˵��>�ݣ�<����˵��>��<�����>
TreeNode* complexStatement() {
	TreeNode* t = NULL;
	TreeNode* p = NULL;
	TreeNode* q = NULL;
	// ��<����˵��>��
	if (TokenType::CONST == g_token.opType) {
		t = constDeclaration();
		p = t;
	}
	// ��<����˵��>��
	if (TokenType::INT == g_token.opType || TokenType::CHAR == g_token.opType) {
		// ����û��ֱ�ӵ���"varDeclaration"��������Ϊ��������һЩ����Ĺ��� ��ʶ�𡰺������塱
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

	// <�����>
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

//24.<������> ::= void main ��(�� ��)�� ��{��<�������>��}��
TreeNode* mainFunction() {
	TreeNode* t = newDecNode(DecKind::MainFunc_DecK);

	match(TokenType::VOID);
	match(TokenType::MAIN);
	if (!match(TokenType::LPARENTHES))
	{
		errorProcess(ErrorType::LACK_KUOHAO_FUN);
		t->error = true;
		return t;
	}
	
	if (!match(TokenType::RPARENTHES))
	{
		errorProcess(ErrorType::LACK_KUOHAO_FUN);
		t->error = true;
		return t;
	}
	if (!match(TokenType::LBRACE))
	{
		errorProcess(ErrorType::LACK_KUOHAO_FUN);
		t->error = true;
		return t;
	}


	t->child[0] = complexStatement();

	if (!match(TokenType::RBRACE))
	{
		errorProcess(ErrorType::LACK_KUOHAO_FUN);
		t->error = true;
		return t;
	}

	return t;

}

/********************* �������﷨�����ڶ��ε�Ԫ���Ե�����	*************************/

//25.<�����> ::= ��<���>��
TreeNode* statementSequence() {
	TreeNode* t = newStmtNode(StmtKind::Seq_StmtK);
	TreeNode* p = NULL;
	TreeNode* q = NULL;
	// FIRST( <���> )���� if while for } ID(f) ID scanf printf return 
	while (TokenType::SEMICOLON == g_token.opType || TokenType::IF == g_token.opType || TokenType::WHILE == g_token.opType ||
		TokenType::FOR == g_token.opType || TokenType::LBRACE == g_token.opType || TokenType::IDEN == g_token.opType ||
		TokenType::SCANF == g_token.opType || TokenType::PRINTF == g_token.opType || TokenType::RETURN == g_token.opType) {
			
		if (NULL == p) {					// ��һ�����
			p = statement();
			t->child[0] = p;
		}
		else {								// ֮������
			q = statement();
			p->sibling = q;
			p = q;
		}
		

	}

	// FOLLOW(<�����>): }
	if (TokenType::RBRACE == g_token.opType) {
		// ���������һ�����Ҳû�У� ����
	}
	else {		// �Ȳ������Ҳ���� }
		printf("error in statementSequence() :in line %d ,tokenType %d value: %s \n", g_lineNumber, g_token.opType, g_token.value);
	}

	return t;
}

/*
26. < ��� > :: = <�������>��<ѭ�����>����{ ��<�����>�� }����<�з���ֵ�ĺ����������>; ��<�޷���ֵ�ĺ����������>;
		��<��ֵ���>; ��<�����>; ��<д���>; ��<��>; ��<�������>;

!!!!!!ע�⣺�����  ��<��>; �� �� ������ǲ�һ���ģ�ֻ�и��ֺ�
*/
TreeNode* statement() {
	TreeNode* t = NULL;
	if (TokenType::SEMICOLON == g_token.opType) {
		// <��>;
		match(TokenType::SEMICOLON);
		return nullptr;									// ����䷵�ؿ�ָ�룬ʲô������
	}
	else if (TokenType::IF == g_token.opType) {
		// <�������>:
		t = ifStatement();
	}
	else if (TokenType::WHILE == g_token.opType) {
		// <ѭ�����>
		t = whileLoopStatement();
	}
	else if (TokenType::FOR == g_token.opType) {
		// <ѭ�����>
		t = forLoopStatement();
	}
	else if (TokenType::LBRACE == g_token.opType) {
		//��{ ��<�����>�� }��
		match(TokenType::LBRACE);
		t = statementSequence();
		match(TokenType::RBRACE);
	}
	else if (TokenType::IDEN == g_token.opType) {
		// <�з���ֵ�ĺ����������>;
		// <�޷���ֵ�ĺ����������>;
		// <��ֵ���>;

		// �˴�Ҫ�ж��� ���û��� ��ֵ�� �����Ƿ��з���ֵ������һ����
		flashBackIndex = g_lexBegin;
		flashBackLineNum = g_lineNumber;

		getNextToken();

		bool isAssign = false;
		if (TokenType::ASSIGN == g_token.opType || TokenType::LBRACKET == g_token.opType) {
			// ��ֵ���
			isAssign = true;
		}
		else {
			// ��������
		}

		g_forward = flashBackIndex;
		g_lineNumber = flashBackLineNum;

		getNextToken();

		if (isAssign) {
			t = assignStatement();
		}
		else {
			// ��������ĺ������ö��� �������� ����ֵ�� �ȼ����޷���ֵ�ĵ���
			t = callWithoutReturn();
		}
		match(TokenType::SEMICOLON);

	}
	else if (TokenType::SCANF == g_token.opType) {
		// <�����>;
		t = scanfStatement();
		match(TokenType::SEMICOLON);

	}
	else if (TokenType::PRINTF == g_token.opType) {
		// <д���>;
		t = printfStatement();
		match(TokenType::SEMICOLON);
	}
	else if (TokenType::RETURN == g_token.opType) {
		// <�������>;
		t = returnStatement();
		match(TokenType::SEMICOLON);
	}
	else {
		// error
		printf("error in statement() :in line %d ,tokenType %d value: %s \n", g_lineNumber, g_token.opType, g_token.value);
	}
	return t;
}


//27. < ��ֵ��� > :: = <��ʶ��>[��[��<�������ʽ>��]��]��<�������ʽ>
TreeNode* assignStatement() {
	TreeNode* t = newStmtNode(StmtKind::Assign_StmtK);
	// �ýڵ� �����������ӣ���ʶ�������ڸýڵ��У������ʽ�������±���ʽ����ѡ�������ӽڵ㣩
	t->attr.name = copyString(g_token.value);
	match(TokenType::IDEN);

	if (TokenType::LBRACKET == g_token.opType) {		//	��ֵ������
		match(TokenType::LBRACKET);
		t->child[1] = exp();							// �ڶ��������� Index
		match(TokenType::RBRACKET);
	}
	match(TokenType::ASSIGN);
	t->child[0] = exp();								// ��һ�����ӣ����Ҷ˵ı��ʽ

	return t;
}


//28. < ������� > :: = if ��(��<�������ʽ>��)��<���>��else<���>��
TreeNode* ifStatement() {
	TreeNode* t = newStmtNode(StmtKind::If_StmtK);
	// ���������ӣ� bool���ʽ��then��䡢else���

	match(TokenType::IF);
	match(TokenType::LPARENTHES);
	t->child[0] = boolExp();				// �������ʽ
	match(TokenType::RPARENTHES);
	t->child[1] = statement();				// then���

	if (TokenType::ELSE == g_token.opType) {
		match(TokenType::ELSE);
		t->child[2] = statement();			// else���
	}
	return t;
}

/*
29. < ѭ����� > :: = while ��(��<�������ʽ>��)��<���> | for��(��<��ֵ���>; <�������ʽ>; <��ֵ���>��)��<���>

forѭ���е��������ʽ����ʼ�����ʽ��ѭ���������Ա��ʽ��ѭ�������������ʽ
*/
// while ��(��<�������ʽ>��)��<���>
TreeNode* whileLoopStatement() {
	TreeNode* t = newStmtNode(StmtKind::While_StmtK);
	match(TokenType::WHILE);
	match(TokenType::LPARENTHES);
	t->child[0] = boolExp();
	match(TokenType::RPARENTHES);
	t->child[1] = statement();
	return t;
}

// for��(��<��ֵ���>; <�������ʽ>; <��ֵ���>��)��<���>
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
	if (NULL == t->child[2]) {		// forѭ���ǿ����
		t->child[2] = p;					// ��forѭ��ĩβ�ĸ�ֵ���ֱ�����
	}
	else {
		t->child[2]->sibling = p;			// �ѵ�������ֵ���ŵ� ѭ����ĩβ
	}
	
	return t;
}


//30. < �з���ֵ�ĺ���������� > :: = <��ʶ��>��(��<ֵ������>��)��
// ���ʽ���ô˺���, ���俴��һ���ض�op���������ӽڵ㣺�������������б�
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

//31. < �޷���ֵ�ĺ���������� > :: = <��ʶ��>��(��<ֵ������>��)��
// ����������� �ô˺���
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
32. < ֵ������ > :: = <�������ʽ>{ ,<�������ʽ> }��<��>

>>>>>>>> ����䣺����Ҳ��һ������䣬��follow��ֻ��')'һ��Ԫ��
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


//33. < ����� > :: = scanf ��(��<��ʶ��>{, <��ʶ��>}��)��
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
34. < д��� > :: = printf ��(��<�ַ���>, <�������ʽ>��)�� | printf ��(��<�ַ���>��)�� | printf ��(��<�������ʽ>��)��

����д�������printfΪ��ʼ�ģ����Բ�������������ַ�������ʽ�������߶��У������߶����ڣ����ַ������ȣ��Զ��Ÿ�����
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

//35. < ������� > :: = return[��(��<�������ʽ>��)��]
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


/************************  ���ϣ��﷨���������β�������  ***********************/

//36. < �������ʽ > :: = �ۣ�������<��>{ <�ӷ������><��> }
TreeNode* exp() {
	TreeNode* t = NULL;
	TreeNode* p = NULL;
	bool isNegative = false;

	// �ۣ�������
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
		// ���� ������������ ��ѵ�һ���ֵ�� ���
		//p = newExpNode(ExpKind::Op_ExpK);
		//p->attr.op = TokenType::NEGATIVE;
		//p->child[0] = t;
		//t = p;

		// �·����� ��ʼ�ĸ���ת���ɣ� 0 - term
		p = newExpNode(ExpKind::Op_ExpK);
		p->attr.op = TokenType::MINU;
		p->child[1] = t;
		t = p;

		p = newExpNode(ExpKind::Num_ExpK);
		p->attr.val = 0;
		t->child[0] = p;
	}
	// { <�ӷ������><��> }
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


//37. < �� > :: = <����>{ <�˷������><����> }
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

//38. < ���� > :: = <��ʶ��>��<��ʶ��>��[��<�������ʽ>��]����<����> | <�ַ�>��<�з���ֵ�����������> | ��(��<�������ʽ>��)��
TreeNode* factor() {
	// <��ʶ��>��<��ʶ��>��[��<�������ʽ>��]����<�з���ֵ�����������>
	TreeNode* t = NULL;
	TreeNode* p = NULL;

	if (TokenType::IDEN == g_token.opType) {
		// �˴�Ҫ�ж��� ���û��� ��ֵ
		flashBackIndex = g_lexBegin;
		flashBackLineNum = g_lineNumber;
		getNextToken();

		bool isCall = false;
		if (TokenType::LPARENTHES == g_token.opType) {
			// ��������
			isCall = true;
		}
		
		g_forward = flashBackIndex;
		g_lineNumber = flashBackLineNum;
		getNextToken();

		// <�з���ֵ�����������>
		if (isCall) {
			t = callWithReturn();
		}
		else {
			// <��ʶ��>��<��ʶ��>��[��<�������ʽ>��]��
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

//39. < �������ʽ > :: = <������>{ �� || �� <������> }
TreeNode* boolExp() {
	TreeNode* t = NULL;
	TreeNode* p = NULL;
	t = boolTerm();
	// { �� || �� <������> }
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

//40. < ������ > :: = <������>{ && <������> }
TreeNode* boolTerm() {
	TreeNode* t = NULL;
	TreeNode* p = NULL;
	t = boolFactor();
	// { && <������> }
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


//41. < ������ > :: = false | true | !<������> | ��(��<�������ʽ>��)�� | <��������>[<���������> <��������>]
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
	default: // <��������>[<���������> <��������>]
		//conditionFactor();
		t = exp();
		// [<���������> <��������>]
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


//42. < �������� > :: = <��ʶ��>[��[��<�������ʽ>��]��]�� <����> | <�ַ�> | <�з���ֵ�����������>
// ���㴦������ʹ��; �õ��Ǳ��ʽ��ʶ��
TreeNode* conditionFactor() {
	// <��ʶ��>��<��ʶ��>��[��<�������ʽ>��]����<�з���ֵ�����������>
	if (TokenType::IDEN == g_token.opType) {
		// �˴�Ҫ�ж��� ���û��� ��ֵ
		flashBackIndex = g_lexBegin;
		flashBackLineNum = g_lineNumber;
		getNextToken();

		bool isCall = false;
		if (TokenType::LPARENTHES == g_token.opType) {
			// ��������
			isCall = true;
		}

		g_forward = flashBackIndex;
		g_lineNumber = flashBackLineNum;
		getNextToken();

		// <�з���ֵ�����������>
		if (isCall) {
			callWithReturn();
		}
		else {
			// <��ʶ��>��<��ʶ��>��[��<�������ʽ>��]��
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
��ʵ�ϣ�����<��������>�������������ʽ��һ�������ﲻ֧��Ƕ�ף���֧�֡�()������Ϊ�������ʽ�����������Ƕ�ף�
	��Ҫ�����Ų��ô�����ʱ��ô��ƣ�Ȼ�������Ҫ���ʽ�Ļ���������ǰ���ȼ��㸳ֵ��һ������
*/





#include "global.h"
#include "parser.h"
#include "scan.h"

using namespace std;

static bool b_functionDeclaration = false;
static bool b_mainFunction = false;
static int flashBackIndex;
static int flashBackLineNum;

static void match(TokenType expectToken);

//Ϊÿһ�����ս������һ������
void program();

void constDeclaration();
void constDefine();
void varDeclaration();
void varDefine();
void typeID();
void functionDefinitionWithReturn();
void DeclarationHead();
void functionDefinitionWithoutReturn();
void paraTable();
void complexStatement();
void mainFunction();

void signedNum();

void statementSequence();
void statement();
void assignStatement();
void ifStatement();
void whileLoopStatement();
void forLoopStatement();
void callWithReturn();
void callWithoutReturn();
void valueParaTable();
void scanfStatement();
void printfStatement();
void returnStatement();

void exp();
void term();
void factor();
void boolExp();
void boolTerm();
void boolFactor();
void conditionFactor();

// ������ʼ�����ó���
void parser() {
	getNextToken();
	//�﷨������ʼ
	//����ʱ������������������
	program();



	if (!EOF_flag) {
		cout << "error: not EOF" << endl;
	}
}

// ƥ�� �ڴ��� token�����򱨴�
// ��ȡ��һ��token
static void match(TokenType expectToken) {
	if (expectToken == g_token.opType) {
		getNextToken();
	}
	else {
		printf("match error in line %d :\t\texpect Token %d, but %d value: %s \n", g_lineNumber, expectToken, g_token.opType, g_token.value);
	}
}

/*
13.<����> ::= ��<����˵��>�ݣ�<����˵��>��{<�з���ֵ��������>|<�޷���ֵ��������>}<������>
�����кܶ�Ĺ����ӣ�����Ϊ���ķ��򵥣�����⣻���ڳ�����ʵ�֣�û����ȡ
*/
void program() {
	// ��<����˵��>��
	if (TokenType::CONST == g_token.opType) {
		constDeclaration();
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
			varDeclaration();			// �������˵��
		}

	}

	// {<�з���ֵ��������>|<�޷���ֵ��������>}
	// ǰ��ı��밴˳����������˾Ͳ��ܶ����ˡ�����������˳��
	while ((TokenType::INT == g_token.opType || TokenType::CHAR == g_token.opType || TokenType::VOID == g_token.opType) && !b_mainFunction) {
		if (TokenType::INT == g_token.opType || TokenType::CHAR == g_token.opType) { //�з���ֵ�Ķ���
			functionDefinitionWithReturn();
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
				functionDefinitionWithoutReturn();
			}
		}
	}

	// <������>
	mainFunction();

}

/*
14.<����˵��> ::= const<��������>;{ const<��������>;}
*/
void constDeclaration() {
	match(TokenType::CONST);
	constDefine();
	match(TokenType::SEMICOLON);

	// ���const ��
	while (TokenType::CONST == g_token.opType) {
		match(TokenType::CONST);
		constDefine();
		match(TokenType::SEMICOLON);
	}
}

//15. < �������� > :: = int<��ʶ��>��<����>{, <��ʶ��>��<����>} | char<��ʶ��>��<�ַ�>{, <��ʶ��>��<�ַ�>}
void constDefine() {
	//int
	if (TokenType::INT == g_token.opType) {
		match(TokenType::INT);
		match(TokenType::IDEN);
		match(TokenType::ASSIGN);
		signedNum();
		// {, <��ʶ��>��<����>}
		while (TokenType::COMMA == g_token.opType) {
			match(TokenType::COMMA);
			match(TokenType::IDEN);
			match(TokenType::ASSIGN);
			signedNum();
		}
	}
	//char
	else if (TokenType::CHAR == g_token.opType) {
		match(TokenType::CHAR);
		match(TokenType::IDEN);
		match(TokenType::ASSIGN);
		match(TokenType::LETTER);
		//{, <��ʶ��>��<�ַ�>}
		while (TokenType::COMMA == g_token.opType) {
			match(TokenType::COMMA);
			match(TokenType::IDEN);
			match(TokenType::ASSIGN);
			match(TokenType::LETTER);
		}
	}
	else {
		printf("error in constDefine() :in line %d ,tokenType %d value: %s \n", g_lineNumber, g_token.opType, g_token.value);
	}
}

//9.<����> ::= �ۣ�������<�޷�������>
//ֻ�ж���ʱ������������ʱ�򵥶�����
void signedNum() {
	//ûд���ơ��������������� ������������
	int sign = 1;
	if (TokenType::MINU == g_token.opType) {		// -
		sign = -1;
		match(TokenType::MINU);
		//�ƻ����е��������� * -1
	}
	else if (TokenType::PLUS == g_token.opType) {
		match(TokenType::PLUS);
		//���ô���
	}
	else if (TokenType::NUM != g_token.opType) {
		printf("error in signedNum() :in line %d ,tokenType %d value: %s \n", g_lineNumber, g_token.opType, g_token.value);
		return;
	}
	// ����Ǹ��ģ�Ӧ�����������һ������
	match(TokenType::NUM);

}

//16.<����˵��> ::= <��������>;{<��������>;}
//�����п��ܻ�ͺ�����з���ֵ���������ͻ
void varDeclaration() {
	varDefine();
	match(TokenType::SEMICOLON);

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
			return;
		}
		else {
			varDefine();			// �����������
			match(TokenType::SEMICOLON);
		}
	}



}

//17.<��������> ::= <���ͱ�ʶ��><��ʶ��>[��[��<�޷�������>��]��] {,<��ʶ��>[��[��<�޷�������>��]��]}
void varDefine() {
	typeID();
	match(TokenType::IDEN);

	if (TokenType::LBRACKET == g_token.opType) {			//���鶨��
		match(TokenType::LBRACKET);
		if (TokenType::NUM == g_token.opType) {
			if ('0' == g_token.value[0])
				printf("error in varDefine() :in line %d ,tokenType %d value: %s ;array size is 0 \n", g_lineNumber, g_token.opType, g_token.value);



		}
		match(TokenType::NUM);
		match(TokenType::RBRACKET);
	}
	while (TokenType::COMMA == g_token.opType) {			//�ظ�����
		match(TokenType::COMMA);
		match(TokenType::IDEN);
		if (TokenType::LBRACKET == g_token.opType) {
			match(TokenType::LBRACKET);
			if (TokenType::NUM == g_token.opType) {
				if ("0" == g_token.value)
					printf("error in varDefine() :in line %d ,tokenType %d value: %s \n", g_lineNumber, g_token.opType, g_token.value);
			}
			match(TokenType::NUM);
			match(TokenType::RBRACKET);
		}
	}
}

//18.<���ͱ�ʶ��> ::= int | char
void typeID() {
	if (TokenType::INT == g_token.opType) {
		match(TokenType::INT);
	}
	else if (TokenType::CHAR == g_token.opType) {
		match(TokenType::CHAR);
	}
	else {
		printf("error in typeID() :in line %d ,tokenType %d value: %s \n", g_lineNumber, g_token.opType, g_token.value);
	}
}

//19.<�з���ֵ��������> ::= <����ͷ��>��(��<������>��)�� ��{��<�������>��}��
void functionDefinitionWithReturn() {
	DeclarationHead();
	match(TokenType::LPARENTHES);
	paraTable();
	match(TokenType::RPARENTHES);
	match(TokenType::LBRACE);

	complexStatement();

	match(TokenType::RBRACE);
}

//20.<����ͷ��> ::= int<��ʶ��> | char<��ʶ��>
void DeclarationHead() {
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
}

//21.<�޷���ֵ��������> ::= void<��ʶ��>��(��<������>��)�� ��{��<�������>��}��
void functionDefinitionWithoutReturn() {
	match(TokenType::VOID);
	match(TokenType::IDEN);
	match(TokenType::LPARENTHES);
	paraTable();
	match(TokenType::RPARENTHES);
	match(TokenType::LBRACE);

	complexStatement();

	match(TokenType::RBRACE);
}

//22.<������> ::= <���ͱ�ʶ��><��ʶ��>{,<���ͱ�ʶ��><��ʶ��>}| <��>
void paraTable() {
	// <���ͱ�ʶ��><��ʶ��>
	if (TokenType::INT == g_token.opType || TokenType::CHAR == g_token.opType) {
		typeID();
		match(TokenType::IDEN);
		// {,<���ͱ�ʶ��><��ʶ��>}
		while (TokenType::COMMA == g_token.opType) {
			match(TokenType::COMMA);
			typeID();
			match(TokenType::IDEN);
		}
	}
	// <��>
	;
}

////////////////////////////16'.__<����˵��> ::= <��������>;{<��������>;}
//�����ǵ����� <����˵��>
inline void __varDeclaration() {
	// <��������>;
	varDefine();
	match(TokenType::SEMICOLON);
	// {<��������>;}
	while (TokenType::INT == g_token.opType || TokenType::CHAR == g_token.opType) {
		varDefine();
		match(TokenType::SEMICOLON);
	}
}

//23.<�������> ::= ��<����˵��>�ݣ�<����˵��>��<�����>
void complexStatement() {
	// ��<����˵��>��
	if (TokenType::CONST == g_token.opType) {
		constDeclaration();
	}
	// ��<����˵��>��
	if (TokenType::INT == g_token.opType || TokenType::CHAR == g_token.opType) {
		// ����û��ֱ�ӵ���"varDeclaration"��������Ϊ��������һЩ����Ĺ��� ��ʶ�𡰺������塱
		__varDeclaration();
	}

	// <�����>
	statementSequence();

}

//24.<������> ::= void main ��(�� ��)�� ��{��<�������>��}��
void mainFunction() {
	match(TokenType::VOID);
	match(TokenType::MAIN);
	match(TokenType::LPARENTHES);
	match(TokenType::RPARENTHES);
	match(TokenType::LBRACE);

	complexStatement();

	match(TokenType::RBRACE);
}

/********************* �������﷨�����ڶ��ε�Ԫ���Ե�����	*************************/

//25.<�����> ::= ��<���>��
void statementSequence() {
	// FIRST( <���> )���� if while for } ID(f) ID scanf printf return 
	while (TokenType::SEMICOLON == g_token.opType || TokenType::IF == g_token.opType || TokenType::WHILE == g_token.opType ||
		TokenType::FOR == g_token.opType || TokenType::LBRACE == g_token.opType || TokenType::IDEN == g_token.opType ||
		TokenType::SCANF == g_token.opType || TokenType::PRINTF == g_token.opType || TokenType::RETURN == g_token.opType) {

		statement();

	}

	// FOLLOW(<�����>): }
	if (TokenType::RBRACE == g_token.opType) {
		// ���������һ�����Ҳû�У� ����
	}
	else {		// �Ȳ������Ҳ���� }
		printf("error in statementSequence() :in line %d ,tokenType %d value: %s \n", g_lineNumber, g_token.opType, g_token.value);
	}
}

/*
26. < ��� > :: = <�������>��<ѭ�����>����{ ��<�����>�� }����<�з���ֵ�ĺ����������>; ��<�޷���ֵ�ĺ����������>;
		��<��ֵ���>; ��<�����>; ��<д���>; ��<��>; ��<�������>;

!!!!!!ע�⣺�����  ��<��>; �� �� ������ǲ�һ���ģ�ֻ�и��ֺ�
*/
void statement() {
	if (TokenType::SEMICOLON == g_token.opType) {
		// <��>;
		match(TokenType::SEMICOLON);
	}
	else if (TokenType::IF == g_token.opType) {
		// <�������>:
		ifStatement();
	}
	else if (TokenType::WHILE == g_token.opType) {
		// <ѭ�����>
		whileLoopStatement();
	}
	else if (TokenType::FOR == g_token.opType) {
		// <ѭ�����>
		forLoopStatement();
	}
	else if (TokenType::LBRACE == g_token.opType) {
		//��{ ��<�����>�� }��
		match(TokenType::LBRACE);
		statementSequence();
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
			assignStatement();
		}
		else {
			// ��������ĺ������ö��� �������� ����ֵ�� �ȼ����޷���ֵ�ĵ���
			callWithoutReturn();
		}
		match(TokenType::SEMICOLON);

	}
	else if (TokenType::SCANF == g_token.opType) {
		// <�����>;
		scanfStatement();
		match(TokenType::SEMICOLON);

	}
	else if (TokenType::PRINTF == g_token.opType) {
		// <д���>;
		printfStatement();
		match(TokenType::SEMICOLON);
	}
	else if (TokenType::RETURN == g_token.opType) {
		// <�������>;
		returnStatement();
		match(TokenType::SEMICOLON);
	}
	else {
		// error
		printf("error in statement() :in line %d ,tokenType %d value: %s \n", g_lineNumber, g_token.opType, g_token.value);
	}
}


//27. < ��ֵ��� > :: = <��ʶ��>[��[��<�������ʽ>��]��]��<�������ʽ>
void assignStatement() {
	match(TokenType::IDEN);
	if (TokenType::LBRACKET == g_token.opType) {
		match(TokenType::LBRACKET);
		exp();
		match(TokenType::RBRACKET);
	}
	match(TokenType::ASSIGN);
	exp();
}


//28. < ������� > :: = if ��(��<�������ʽ>��)��<���>��else<���>��
void ifStatement() {
	match(TokenType::IF);
	match(TokenType::LPARENTHES);
	boolExp();
	match(TokenType::RPARENTHES);
	statement();

	if (TokenType::ELSE == g_token.opType) {
		match(TokenType::ELSE);
		statement();
	}
}

/*
29. < ѭ����� > :: = while ��(��<�������ʽ>��)��<���> | for��(��<��ֵ���>; <�������ʽ>; <��ֵ���>��)��<���>

forѭ���е��������ʽ����ʼ�����ʽ��ѭ���������Ա��ʽ��ѭ�������������ʽ
*/
// while ��(��<�������ʽ>��)��<���>
void whileLoopStatement() {
	match(TokenType::WHILE);
	match(TokenType::LPARENTHES);
	boolExp();
	match(TokenType::RPARENTHES);
	statement();
}

// for��(��<��ֵ���>; <�������ʽ>; <��ֵ���>��)��<���>
void forLoopStatement() {
	match(TokenType::FOR);
	match(TokenType::LPARENTHES);
	assignStatement();
	match(TokenType::SEMICOLON);
	boolExp();
	match(TokenType::SEMICOLON);
	assignStatement();
	match(TokenType::RPARENTHES);
	statement();
}


//30. < �з���ֵ�ĺ���������� > :: = <��ʶ��>��(��<ֵ������>��)��
// ���ʽ���ô˺���
void callWithReturn() {
	match(TokenType::IDEN);
	match(TokenType::LPARENTHES);
	if (TokenType::RPARENTHES != g_token.opType) {
		valueParaTable();
	}
	match(TokenType::RPARENTHES);
}


//31. < �޷���ֵ�ĺ���������� > :: = <��ʶ��>��(��<ֵ������>��)��
// ����������� �ô˺���
void callWithoutReturn() {
	match(TokenType::IDEN);
	match(TokenType::LPARENTHES);
	if (TokenType::RPARENTHES != g_token.opType) {
		valueParaTable();
	}
	match(TokenType::RPARENTHES);
}


/*
32. < ֵ������ > :: = <�������ʽ>{ ,<�������ʽ> }��<��>

>>>>>>>> ����䣺����Ҳ��һ������䣬��follow��ֻ��')'һ��Ԫ��
*/
void valueParaTable() {
	exp();
	while (TokenType::COMMA == g_token.opType) {
		match(TokenType::COMMA);
		exp();
	}
}


//33. < ����� > :: = scanf ��(��<��ʶ��>{, <��ʶ��>}��)��
void scanfStatement() {
	match(TokenType::SCANF);
	match(TokenType::LPARENTHES);
	match(TokenType::IDEN);
	while (TokenType::COMMA == g_token.opType) {
		match(TokenType::COMMA);
		match(TokenType::IDEN);
	}
	match(TokenType::RPARENTHES);
}


/*
34. < д��� > :: = printf ��(��<�ַ���>, <�������ʽ>��)�� | printf ��(��<�ַ���>��)�� | printf ��(��<�������ʽ>��)��

����д�������printfΪ��ʼ�ģ����Բ�������������ַ�������ʽ�������߶��У������߶����ڣ����ַ������ȣ��Զ��Ÿ�����
*/
void printfStatement() {
	match(TokenType::PRINTF);
	match(TokenType::LPARENTHES);
	if (TokenType::STRING == g_token.opType) {
		match(TokenType::STRING);
		if (TokenType::COMMA == g_token.opType) {
			match(TokenType::COMMA);
			exp();
		}
	}
	else {
		exp();

	}
	match(TokenType::RPARENTHES);
}

//35. < ������� > :: = return[��(��<�������ʽ>��)��]
void returnStatement() {
	match(TokenType::RETURN);
	if (TokenType::LPARENTHES == g_token.opType) {
		match(TokenType::LPARENTHES);
		exp();
		match(TokenType::RPARENTHES);
	}
}


/************************  ���ϣ��﷨���������β�������  ***********************/

//36. < �������ʽ > :: = �ۣ�������<��>{ <�ӷ������><��> }
void exp() {
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

	term();

	if (isNegative) {
		// ���� ������������ ��ѵ�һ���ֵ�� ���
	}
	// { <�ӷ������><��> }
	while (TokenType::PLUS == g_token.opType || TokenType::MINU == g_token.opType) {
		if (TokenType::MINU == g_token.opType) {
			match(TokenType::MINU);
			term();
		}
		else {
			match(TokenType::PLUS);
			term();
		}
	}
}


//37. < �� > :: = <����>{ <�˷������><����> }
void term() {
	factor();

	while (TokenType::MULT == g_token.opType || TokenType::DIV == g_token.opType) {
		if (TokenType::MULT == g_token.opType) {
			match(TokenType::MULT);
			factor();
		}
		else {
			match(TokenType::DIV);
			factor();
		}
	}
}

//38. < ���� > :: = <��ʶ��>��<��ʶ��>��[��<�������ʽ>��]����<����> | <�ַ�>��<�з���ֵ�����������> | ��(��<�������ʽ>��)��
void factor() {
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
	else if (TokenType::LPARENTHES == g_token.opType) {
		match(TokenType::LPARENTHES);
		exp();
		match(TokenType::RPARENTHES);
	}
	else {
		printf("error in factor() :in line %d ,tokenType %d value: %s \n", g_lineNumber, g_token.opType, g_token.value);
	}
}

//39. < �������ʽ > :: = <������>{ �� || �� <������> }
void boolExp() {
	boolTerm();
	// { �� || �� <������> }
	while (TokenType::OR == g_token.opType) {
		match(TokenType::OR);
		boolTerm();
	}
}

//40. < ������ > :: = <������>{ && <������> }
void boolTerm() {
	boolFactor();
	// { && <������> }
	while (TokenType::AND == g_token.opType) {

		match(TokenType::AND);
		boolFactor();
	}
}


//41. < ������ > :: = false | true | !<������> | ��(��<�������ʽ>��)�� | <��������>[<���������> <��������>]
void boolFactor() {
	switch (g_token.opType) {
	case TokenType::FALSE:
		match(TokenType::FALSE);
		break;
	case TokenType::TRUE:
		match(TokenType::TRUE);
		break;
	case TokenType::NOT:
		match(TokenType::NOT);

		boolFactor();

		break;
	case TokenType::LPARENTHES:
		match(TokenType::LPARENTHES);
		boolExp();
		match(TokenType::RPARENTHES);
		break;
	default:
		conditionFactor();
		// [<���������> <��������>]
		if (TokenType::LSS == g_token.opType || TokenType::LEQ == g_token.opType ||
			TokenType::GRE == g_token.opType || TokenType::GEQ == g_token.opType ||
			TokenType::NEQ == g_token.opType || TokenType::EQL == g_token.opType
			) {
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
			conditionFactor();
		}
	}
}


//42. < �������� > :: = <��ʶ��>[��[��<�������ʽ>��]��]�� <����> | <�ַ�> | <�з���ֵ�����������>
void conditionFactor() {
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


}

/*
��ʵ�ϣ�����<��������>�������������ʽ��һ�������ﲻ֧��Ƕ�ף���֧�֡�()������Ϊ�������ʽ�����������Ƕ�ף�
	��Ҫ�����Ų��ô�����ʱ��ô��ƣ�Ȼ�������Ҫ���ʽ�Ļ���������ǰ���ȼ��㸳ֵ��һ������
*/





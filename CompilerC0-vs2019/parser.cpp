#include "global.h"
#include "parser.h"
#include "scan.h"

using namespace std;

static bool b_functionDeclaration = false;
static bool b_mainFunction = false;
static int flashBackIndex;

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
void loopStatement();
void callWithReturn();
void callWithoutReturn();
void valueParaTable();
void readStatement();
void writeStatement();
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
			else if(TokenType::SEMICOLON == g_token.opType) { // �ֺ�

			}
			else if (TokenType::LBRACKET == g_token.opType) {	// ��������

			}
			else {											//����								
				printf("error in program() :in line %d ,tokenType %d value: %s \n", g_lineNumber, g_token.opType, g_token.value);
			}

		}
		// ����
		g_forward = flashBackIndex;
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
			getNextToken();

			if (TokenType::MAIN == g_token.opType) {
				b_mainFunction = true;
			}
			// ����
			g_forward = flashBackIndex;
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

}

/*
26. < ��� > :: = <�������>��<ѭ�����>����{ ��<�����>�� }����<�з���ֵ�ĺ����������>; ��<�޷���ֵ�ĺ����������>; 
		��<��ֵ���>; ��<�����>; ��<д���>; ��<��>; ��<�������>;

!!!!!!ע�⣺�����  ��<��>; �� �� ������ǲ�һ���ģ�ֻ�и��ֺ�
*/
void statement() {

}


//27. < ��ֵ��� > :: = <��ʶ��>[��[��<�������ʽ>��]��]��<�������ʽ>
void assignStatement() {

}


//28. < ������� > :: = if ��(��<�������ʽ>��)��<���>��else<���>��
void ifStatement() {

}

/*
29. < ѭ����� > :: = while ��(��<�������ʽ>��)��<���> | for��(��<��ֵ���>; <�������ʽ>; <��ֵ���>��)��<���>

forѭ���е��������ʽ����ʼ�����ʽ��ѭ���������Ա��ʽ��ѭ�������������ʽ
*/
void loopStatement() {

}


//30. < �з���ֵ�ĺ���������� > :: = <��ʶ��>��(��<ֵ������>��)��
void callWithReturn() {

}


//31. < �޷���ֵ�ĺ���������� > :: = <��ʶ��>��(��<ֵ������>��)��
void callWithoutReturn() {

}


/*
32. < ֵ������ > :: = <�������ʽ>{ ,<�������ʽ> }��<��>

>>>>>>>> ����䣺����Ҳ��һ������䣬��follow��ֻ��')'һ��Ԫ��
*/
void valueParaTable() {

}


//33. < ����� > :: = scanf ��(��<��ʶ��>{, <��ʶ��>}��)��
void readStatement() {

}


/*
34. < д��� > :: = printf ��(��<�ַ���>, <�������ʽ>��)�� | printf ��(��<�ַ���>��)�� | printf ��(��<�������ʽ>��)��

����д�������printfΪ��ʼ�ģ����Բ�������������ַ�������ʽ�������߶��У������߶����ڣ����ַ������ȣ��Զ��Ÿ�����
*/
void writeStatement() {

}

//35. < ������� > :: = return[��(��<�������ʽ>��)��]
void returnStatement() {

}

//36. < �������ʽ > :: = �ۣ�������<��>{ <�ӷ������><��> }
void exp() {

}


//37. < �� > :: = <����>{ <�˷������><����> }
void term() {

}

//38. < ���� > :: = <��ʶ��>��<��ʶ��>��[��<�������ʽ>��]����<����> | <�ַ�>��<�з���ֵ�����������> | ��(��<�������ʽ>��)��
void factor() {

}

//39. < �������ʽ > :: = <������>{ �� || �� <������> }
void boolExp() {

}

//40. < ������ > :: = <������>{ && <������> }
void boolTerm() {

}


//41. < ������ > :: = false | true | !<������> | ��(��<�������ʽ>��)�� | <��������>[<���������> <��������>]
void boolFactor() {

}


//42. < �������� > :: = <��ʶ��>[��[��<�������ʽ>��]��]�� <����> | <�ַ�> | <�з���ֵ�����������>
void conditionFactor() {

}

/*
��ʵ�ϣ�����<��������>�������������ʽ��һ�������ﲻ֧��Ƕ�ף���֧�֡�()������Ϊ�������ʽ�����������Ƕ�ף�
	��Ҫ�����Ų��ô�����ʱ��ô��ƣ�Ȼ�������Ҫ���ʽ�Ļ���������ǰ���ȼ��㸳ֵ��һ������
*/





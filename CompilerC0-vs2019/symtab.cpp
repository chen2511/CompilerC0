#include "symtab.h"


static int g_adress = 0;
static int f_adress = 0;

#define SHIFT 4

// ɢ�к���
static int hash(char* key)
{
	int temp = 0;
	int i = 0;
	while (key[i] != '\0')
	{
		temp = ((temp << SHIFT) + key[i]) % SYMBOL_TABLE_SIZE;
		++i;
	}
	return temp;
}

// ��ʼ��һ�� ����ÿ������һ�ű�ȫ��һ�ű�
SymTab* initSimpleSymTable(char* name)
{
	SymTab* st = new SymTab;
	st->next = NULL;
	st->fname = name;
	for (int i = 0; i < SYMBOL_TABLE_SIZE; i++) {
		st->hashTable[i] = NULL;
	}

	f_adress = 0;		// ÿ����һ���º�����������ָ��仯����Ե�ַ
	
	return st;
}

// �����Ƿ����ɹ���������û���õ�����ֵ���ڲ������ˣ�����ռ䲻�㡢�ظ����壬Ĭ��int vec = -1, FuncInfo* p = NULL
// ���� isGlobal��־ ȷ������ȫ�ֱ��� ������
bool insert_SymTab(bool isGlobal, int lineno, char* name, IDType type, Type valuetype, int value, int vec, FuncInfo* p)
{
	int h = hash(name);
	SymbolList list;
	if (isGlobal) {							// ȫ�ֱ�
		list = g_symtab->hashTable[h];
	}
	else {									// ������
		list = g_symtab->next->hashTable[h];
	}

	// ���ұ����Ƿ�����
	// Ϊ�գ��˳���û�ҵ�ͬ���� ���ֲ�һ�������ң� 
	while ((list != NULL) && (strcmp(name, list->name) != 0)) {
		list = list->next;
	}

	if (NULL == list) {						// δ���壬�ɲ�����ű�
		Symbol* sym = new Symbol;
		if (NULL == sym) {
			printf("Error in line  %d  general SymTab! Not Enough Mem\n", lineno);
			return false;
		}
		sym->name = name;
		sym->type = type;
		sym->valueType = valuetype;
		if (IDType::Const_ID == type) {				// �����ų�ʼ��
			sym->value = value;
		}
		sym->vec = vec;
		sym->pfinfo = p;

		if (isGlobal) {						// �����Ƿ�ȫ�ֱ����õ�ַ��������Ӧ��
			sym->adress = g_adress;
			// ���µ�ַ
			int size;
			if (Type::T_INTEGER == valuetype || IDType::Func_ID == type) {
				size = 4;				// int
			}
			else {
				size = 1;				// �ַ�
			}
			if (-1 != vec) {
				g_adress += size * vec;
			}
			else {
				g_adress += size;
			}
			// ȫ�ֱ�
			sym->next = g_symtab->hashTable[h];
			g_symtab->hashTable[h] = sym;
		}
		else {								// ������
			sym->adress = f_adress;
			// ���µ�ַ
			int size;
			if (Type::T_INTEGER == valuetype) {
				size = 4;				// int
			}
			else {
				size = 1;				// �ַ�
			}
			if (-1 != vec) {
				f_adress += size * vec;
			}
			else {
				f_adress += size;
			}
			// ���뺯����
			sym->next = g_symtab->next->hashTable[h];
			g_symtab->next->hashTable[h] = sym;
		}
		return true;
	}
	else {									// �Ѷ��壬����
		printf("Error in line %d : %s �ض���\n", lineno, name);
		return false;
	}
}

Symbol* lookUp_SymTab(char* name)
{
	int h = hash(name);
	// �ȼ�麯����
	SymbolList list = g_symtab->next->hashTable[h];
	while ((list != NULL) && (strcmp(name, list->name) != 0))
		list = list->next;
	if (NULL != list)
		return list;
	else {				// �ڼ��ȫ�ֱ�
		list = g_symtab->hashTable[h];
		while ((list != NULL) && (strcmp(name, list->name) != 0))
			list = list->next;
		if (NULL != list)
			return list;
	}
	// ��û�У�δ����
	return nullptr;
}

#include "symtab.h"


static int g_adress = 0;
static int f_adress = 0;

#define SHIFT 4
#define CHAR_SIZE 4

// 散列函数
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

// 初始化一张 单表；每个函数一张表、全局一张表
SymTab* initSimpleSymTable(char* name)
{
	SymTab* st = new SymTab;
	st->next = NULL;
	st->fname = name;
	for (int i = 0; i < SYMBOL_TABLE_SIZE; i++) {
		st->hashTable[i] = NULL;
	}
	st->varsize = 0;
	f_adress = 0;		// 每次有一个新函数，函数表指针变化，相对地址
	
	return st;
}

// 返回是否插入成功（后来并没有用到返回值，内部处理了），如空间不足、重复定义，默认int vec = -1, FuncInfo* p = NULL
// 根据 isGlobal标志 确定插入全局表还是 函数表
bool insert_SymTab(bool isGlobal, int lineno, char* name, IDType type, Type valuetype, int value, int vec, FuncInfo* p)
{
	int h = hash(name);
	SymbolList list;
	if (isGlobal) {							// 全局表
		list = g_symtab->hashTable[h];
	}
	else {									// 函数表
		list = g_symtab->next->hashTable[h];
	}

	// 查找表中是否已有
	// 为空，退出，没找到同名； 名字不一样继续找； 
	while ((list != NULL) && (strcmp(name, list->name) != 0)) {
		list = list->next;
	}

	if (NULL == list) {						// 未定义，可插入符号表
		Symbol* sym = new Symbol;
		if (NULL == sym) {
			printf("Error in line  %d  general SymTab! Not Enough Mem\n", lineno);
			return false;
		}
		sym->name = name;
		sym->type = type;
		sym->valueType = valuetype;
		if (IDType::Const_ID == type) {				// 常量才初始化
			sym->value = value;
		}
		sym->vec = vec;
		sym->pfinfo = p;

		// 默认 false
		sym->isreg = false;

		// 计算地址，插入表中
		if (isGlobal) {						// 根据是否全局表，设置地址，插入相应表
			sym->adress = g_adress;
			// 更新地址
			int size;
			if (Type::T_INTEGER == valuetype || IDType::Func_ID == type) {
				size = 4;				// int
			}
			else {
				size = CHAR_SIZE;				// 字符
			}
			if (-1 != vec) {
				g_adress += size * vec;
			}
			else {
				g_adress += size;
			}
			// 全局表
			sym->next = g_symtab->hashTable[h];
			g_symtab->hashTable[h] = sym;
			g_symtab->varsize = g_adress;
		}
		else {								// 函数表
			sym->adress = f_adress;
			// 更新地址
			int size;
			if (Type::T_INTEGER == valuetype) {
				size = 4;				// int
			}
			else {
				size = CHAR_SIZE;				// 字符
			}
			if (-1 != vec) {
				f_adress += size * vec;
			}
			else {
				f_adress += size;
			}
			// 插入函数表
			sym->next = g_symtab->next->hashTable[h];
			g_symtab->next->hashTable[h] = sym;
			g_symtab->next->varsize = f_adress;
		}
		return true;
	}
	else {									// 已定义，报错
		printf("Error in line %d : %s 重定义\n", lineno, name);
		return false;
	}
}

// 插入临时变量：变量名、isreg、地址
void insertTempVar2SymTab(char* name)
{
	int h = hash(name);

	Symbol* sym = new Symbol;
	sym->name = name;
	// 默认 false
	sym->isreg = false;

	// 函数表
	sym->adress = g_symtab->next->varsize;
	// 更新地址
	g_symtab->next->varsize += 4;

	// 插入函数表
	sym->next = g_symtab->next->hashTable[h];
	g_symtab->next->hashTable[h] = sym;
}

Symbol* lookUp_SymTab(char* name)
{
	int h = hash(name);
	// 先检查函数表
	SymbolList list = g_symtab->next->hashTable[h];
	while ((list != NULL) && (strcmp(name, list->name) != 0))
		list = list->next;
	if (NULL != list)
		return list;
	else {				// 在检查全局表
		list = g_symtab->hashTable[h];
		while ((list != NULL) && (strcmp(name, list->name) != 0))
			list = list->next;
		if (NULL != list)
			return list;
	}
	// 都没有，未定义
	return nullptr;
}


Symbol* lookUp_SymTab(char* name, bool& isGlobal)
{
	isGlobal = false;
	int h = hash(name);
	// 先检查函数表
	SymbolList list = g_symtab->next->hashTable[h];
	while ((list != NULL) && (strcmp(name, list->name) != 0))
		list = list->next;
	if (NULL != list)
		return list;
	else {				// 在检查全局表
		isGlobal = true;
		list = g_symtab->hashTable[h];
		while ((list != NULL) && (strcmp(name, list->name) != 0))
			list = list->next;
		if (NULL != list)
			return list;
	}
	// 都没有，未定义
	return nullptr;
}

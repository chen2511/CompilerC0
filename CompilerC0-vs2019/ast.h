#pragma once
#ifndef AST_H
#define AST_H

#include "global.h"

// ���������ڵ㣺���������������������ȵ�˵�����壬��������������ͣ����ؽڵ�ָ��
TreeNode* newDecNode(DecKind kind);

// �������ڵ㣺��������������ͣ����ؽڵ�ָ��
TreeNode* newStmtNode(StmtKind kind);

// �������ʽ�ڵ㣺��������������ͣ����ؽڵ�ָ��
TreeNode* newExpNode(ExpKind kind);

// �����������ʽ�ڵ㣺��������������ͣ����ؽڵ�ָ��
TreeNode* newBoolExpNode(BoolExpKind kind);

// ���庯��ʱ�����溯����Ϣ���������ͺͲ�����
// FuncInfo* newFuncInfo(Type t);

// ����token�е��ַ�������̬����Ŀռ�
char* copyString(char* s);

// ��������﷨�����ļ�
void printAST(TreeNode* tree);

#endif // !AST_H



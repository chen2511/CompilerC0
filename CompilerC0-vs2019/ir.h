#pragma once
#ifndef IR_H
#define IR_H

#include "global.h"

void IR_Analyze(TreeNode* tree);

void IR_Exp(TreeNode* tree);

void printIR();

char* newitoa(int a);

char* newempty();

#endif // !IR_H



#include "errorprocess.h"
#include "scan.h"

void errorProcess(ErrorType e)
{
	g_errorNum++;
	switch (e)
	{
	case ErrorType::LACK_SEMI_CST:
	case ErrorType::LACK_TYPE_CST: 
	case ErrorType::LACK_ID_CST:
	case ErrorType::LACK_ASSIGN_CST: {
		while (g_token.opType != TokenType::CONST && g_token.opType != TokenType::INT && g_token.opType != TokenType::CHAR
			&& g_token.opType != TokenType::VOID && g_token.opType != TokenType::IF
			&& g_token.opType != TokenType::WHILE && g_token.opType != TokenType::FOR
			&& g_token.opType != TokenType::IDEN && g_token.opType != TokenType::RETURN
			&& g_token.opType != TokenType::SCANF && g_token.opType != TokenType::PRINTF
			&& g_token.opType != TokenType::LBRACE && g_token.opType != TokenType::RBRACE
			)
		{
			if (g_token.opType == TokenType::END) 
			{
				exit(0);
			}
			getNextToken();
		}
		break;
	}
	// 以上都是常量定义阶段的错误
	// 变量定义错误：
	case ErrorType::LACK_XXX_VARDEF: {
		while (g_token.opType != TokenType::INT && g_token.opType != TokenType::CHAR
			&& g_token.opType != TokenType::VOID && g_token.opType != TokenType::IF
			&& g_token.opType != TokenType::WHILE && g_token.opType != TokenType::FOR
			&& g_token.opType != TokenType::IDEN && g_token.opType != TokenType::RETURN
			&& g_token.opType != TokenType::SCANF && g_token.opType != TokenType::PRINTF
			&& g_token.opType != TokenType::LBRACE && g_token.opType != TokenType::RBRACE
			)
		{
			if (g_token.opType == TokenType::END)
			{
				exit(0);
			}
			getNextToken();
		}
		break;
	}
	// 函数定义错误
	case ErrorType::LACK_TYPE_FUN:
	case ErrorType::LACK_IDEN_FUN: 
	case ErrorType::LACK_KUOHAO_FUN: {

		while (g_token.opType != TokenType::INT && g_token.opType != TokenType::CHAR
			&& g_token.opType != TokenType::VOID
			)
		{
			if (g_token.opType == TokenType::END)
			{
				exit(0);
			}
			getNextToken();
		}


		break;
	}
	// 语句错误
	case ErrorType::SENTENCE_ERROR: {
		while (g_token.opType != TokenType::IF
			&& g_token.opType != TokenType::WHILE && g_token.opType != TokenType::FOR
			&& g_token.opType != TokenType::IDEN && g_token.opType != TokenType::RETURN
			&& g_token.opType != TokenType::SCANF && g_token.opType != TokenType::PRINTF
			&& g_token.opType != TokenType::LBRACE && g_token.opType != TokenType::RBRACE
			)
		{
			if (g_token.opType == TokenType::END)
			{
				exit(0);
			}
			getNextToken();
		}
		break;
	}
	
	default:
		break;
	}



}

# CompilerC0

[TOC]

## 零、文件说明

>   global.h：全局数据结构、全局变量
>
>   scan.h：局部数据结构、外部调用函数
>
>   scan.cpp：内部函数



## 一、词法分析：

>   理论来自louden书和慕课

>   文件：scan.h;scan.cpp

### 分析思路

理论：

>   文法》正则表达式》NFA》DFA》DFA最小化》编程实现

分为三种类型：保留字、特殊符号（运算符等等）、其他（数字、标识符、字符串、字符）

>   把保留字先看作标识符、再进行识别
>
>   所以，识别特殊符号、无符号数字、标识符、字符串、字符即可

#### 正则表达式

数字：`[1-9][0-9]*|0`

标识符：`[_a-zA-Z][_a-zA-Z0-9]*`

运算符：

```
+ - * / 
&&  '||'  !
< <= > >= != == 
= ： , ; [ ] { } ( ) 
```

字符：`'[<加法运算符>｜<乘法运算符>｜<字母>｜<数字>]'`

字符串：`"｛十进制编码为32,33,35-126的ASCII字符｝"`

#### NFA



#### DFA

>   比较简单。。。。似乎不用什么操作了



### 数据结构

#### Token

```c++
typedef struct{
    TokenType opType;
    char * value;
}Token;
```



#### TokenType

枚举类型

```c++
typedef enum {
    //关键字：
    CHAR, CONST, ELSE, FALSE, FOR,              //0-4
    IF, INT, MAIN, PRINTF, RETURN,              //5-9
    SCANF, TRUE, VOID, WHILE,                   //10-13
    NUM, IDEN, LETTER, STRING,                  //14-17:数字、标识符、字符、字符串
    PLUS, MINU, MULT, DIV,                      //18-21:+ - * /
    AND, OR, NOT,                               //22-24:&& || !
    LSS, LEQ, GRE, GEQ, NEQ, EQL,               //25-30关系运算符: < <= > >= != ==
    ASSIGN, COLON, COMMA, SEMICOLON,            //31-34: =  :  ,  ;
    LBRACE, RBRACE, LBRACKET, RBRACKET,         //35-38:{ } [ ]
    LPARENTHES, RPARENTHES,                     //39-40:( )                                         
}TokenType;
```

#### 保留字表

```c++
const char* reservedWords[] = {
    "case", "char", "const", "default", "else",
    "false", "for", "if", "int", "main",
    "printf", "return", "scanf", "switch", "true",
    "void", "while"
};
```

#### 状态

```c++
typedef enum {
    STATE_START, STATE_NUM, STATE_ID, STATE_CHAR, STATE_STRING, STATE_DONE
}StateType;
```







### DFA编程实现

>   来自louden书的思路

#### 1、隐含状态

注意`advance the input`表示读入一个符号

![image-20200416115119049](README.assets/image-20200416115119049.png)

![image-20200416110131830](README.assets/image-20200416110131830.png)

#### 2、双重case

![image-20200416115645554](README.assets/image-20200416115645554.png)

#### 3、转换表





![image-20200416110233447](README.assets/image-20200416110233447.png)









### 注意问题：

#### 1、读入缓冲

>   缓冲的意思是，不是直接通过指针操作，而是，先读进来一个数组的数据，在数组中操作；
>
>   然后有单缓冲和双缓冲的区别；而北航是多了一个backup数组；
>
>   louden书中的例子，也是有一个缓冲数组lineBuf



#### 2、保留字的识别

>   先看做标识符，再识别保留字；
>
>   保留字表，字典序查询

![image-20200416110914211](README.assets/image-20200416110914211.png)

![image-20200416110407763](README.assets/image-20200416110407763.png)

保留字表；

如果比较大，就用二分查找

#### 3、带符号数处理

>   +1+1
>
>   第二个 +1和第一个+1是有区别的，词法分析阶段识别带符号数，最长字串原则，会有问题

##### 分析：

带符号整数出现于变量定义时，这个地方不会存在问题，语法分析时处理；

另一个是位于表达式中：

>   数学上的形式：其中要么带符号数要么在开头，要么有括号；其他情况数字都是无符号的

因为表达式开头有符号定义，可以忽略这个；

而对于带符号数在表达式内部，肯定实在括号内部，所以又可以递归回去，用开头的符号；

所以总结起来也是语法分析阶段处理

还有就是布尔表达式的地方，也是单独分析

##### 处理方法

放到语法分析中处理带符号数

#### 3、冲突解决



#### 4、错误处理



## 二、语法分析

### 2.1 文法改造

#### 二义性

-   if else

-   表达式

>   未发现其他二义性的地方

#### 左递归

可以较为容易地判断出：没有直接和间接左递归。

#### 回溯

通过改造文法（简单的改造，提取公共左因子；但为了直观、更方便处理，有的不改造了）或者  算法处理（if else 进一步判断，判断后回溯，感觉思想类似LL(n)了，事实上就没有消除回溯）

#### 计算FIRST和FOLLOW集

>   可以用于选择、重复的case判断









#### 使用EBNF

EBNF更适合递归下降分析法

例如：

![image-20200420001326020](README.assets/image-20200420001326020.png)



### 2.2 编程实现

#### 基本方法

![image-20200419225239669](README.assets/image-20200419225239669.png)



#### 重复和选择：使用EBNF

##### 可选结构

![image-20200419161119250](README.assets/image-20200419161119250.png)

##### 重复结构

![image-20200419162417555](README.assets/image-20200419162417555.png)

![image-20200419162522766](README.assets/image-20200419162522766.png)

![image-20200419162535166](README.assets/image-20200419162535166.png)



















### 2.3 数据结构





### 2.4 注意问题

#### 1、公共因子

```
13.<程序> ::= ［<常量说明>］［<变量说明>］{<有返回值函数定义>|<无返回值函数定义>}<主函数>
```

这一部分为了 文法上的简便，没有进行公共因子的提取；

选择类似LL(n)的方法。。。。。一次性往后看好多



分三段，设flag

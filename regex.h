    // 我真诚地保证：
// 我自己独立地完成了整个程序从分析、设计到编码的所有工作。
// 如果在上述过程中，我遇到了什么困难而求教于人，那么，我将在程序实习报告中
// 详细地列举我所遇到的问题，以及别人给我的提示。
// 我的程序里中凡是引用到其他程序或文档之处，
// 例如教材、课堂笔记、网上的源代码以及其他参考书上的代码段,
// 我都已经在程序的注释里很清楚地注明了引用的出处。
// 我从未抄袭过别人的程序，也没有盗用别人的程序，
// 不管是修改式的抄袭还是原封不动的抄袭。
    // 我编写这个程序，从来没有想过要去破坏或妨碍其他计算机系统的正常运转。
    // 兰兆千



/*************************************************
文件名称：   regex.h
项目名称：  mini-vim
创建者：    兰兆千
创建时间：   2013-10-09
最后修改时间： 2013-10-12
功能：     实现正则表达式搜索功能
与其他文件的依赖关系：
    头文件buffer.h  文章缓存存储结构的定义
**************************************************/
#ifndef __MINIVIM_REGEX__
#define __MINIVIM_REGEX__ 1


#define null NULL
#define CHARSET 256
#include "buffer.h"
#include<iostream>
#include<stack>
#include<vector>
#include<queue>
#include<set>
using namespace std;

enum NODE_TYPE{CHAR,OP};        //解析树后缀表达式节点类型
enum RFA_NODE_TYPE{EPSILON = -3,SPLIT = -2,RFA_END = -1};       //RFA状态类型
enum DFA_NODE_TYPE{DFA_DEFAULT = 0,DFA_END = -1};       //DFA状态类型
enum DFA_TYPE{DFA_NORMAL = 1,DFA_ABNORMAL = -1};        //DFA类型


//解析树结点
typedef struct _RENODE
{
    NODE_TYPE type; //类型  字符型和运算符
    char content; //内容
}RENODE;


//RFA状态
typedef struct _RFANODE
{
    static int idcount;//总共有几个结点
    int id;
    int outc;//进行到下一个状态所需要的字符，不过也可以是分支、结束
    _RFANODE* out1;//outc所指示要连接到下一个结点的指针
    _RFANODE* out2;//分支节点则启用，指向分支

    _RFANODE(int outc,_RFANODE* out1,_RFANODE* out2 = null);

}RFANODE;

//RFA
typedef struct _RFA
{
    RFANODE * pstart; //开始结点
    set<RFANODE *> pends; //结束结点
    _RFA(RFANODE * pstart);
}RFA;



//DFA状态
typedef struct _DFANODE
{
    static int idcount;
    int id;
    _DFANODE* next[CHARSET];    //状态转移表
    bool *list; //对应的RFA状态
	DFA_NODE_TYPE type; //状态类型  是否为结束
    _DFANODE(bool *list);

}DFANODE;


//DFA
typedef struct _DFA
{
    DFANODE* pstart;//开始状态
}DFA;



//返回的匹配结果  双向链表
typedef struct _REGEX_RESULT
{
	int line, start , len;//行号 开始索引 长度
	struct _REGEX_RESULT *prev , *next;
	_REGEX_RESULT(int line ,int start,int len, _REGEX_RESULT* prev , _REGEX_RESULT* next);
}REGEX_RESULT;




int gen_tree(const char * word , int len , int pos );

RFA * rfa_patch(RFA *p1 , RFA *p2);
RFA * rfa_merge(RFA *p1 , RFA *p2);
RFA * rfa_oneornone(RFA *p1);
RFA * rfa_anytimes(RFA *p1);
RFA * rfa_morethanonce(RFA * p1);
RFA * gen_rfa();

void rfa_output(RFA * prfa);


bool* gen_eplosure(vector<RFANODE*> & rfans);
vector<RFANODE*> gen_shift(vector<RFANODE*> & rfans,char c);
vector<char>  get_next_chars(vector<RFANODE*> & rfans);
DFANODE* find_dfa(const bool *list);
DFANODE * gen_dfa_node(vector<RFANODE*> & rfans);
void dfa_output(DFA* pdfa);
DFA * gen_dfa(RFA * prfa);

int find_match_in_line(DFA* pdfa , char* c , int len , int line , REGEX_RESULT* &r);




DFA* gen_dfa(char * regex , int len);
int regex_match(DFA* pdfa , line_t* head , REGEX_RESULT* &r);
void regex_release(DFA* pdfa);

//interface
int regex_match(line_t* head ,char * regex , int len , REGEX_RESULT* &r); // return value is num of matched , specially -1 for illegal regex


#endif

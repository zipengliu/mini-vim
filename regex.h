    // ����ϵر�֤��
// ���Լ��������������������ӷ�������Ƶ���������й�����
// ��������������У���������ʲô���Ѷ�������ˣ���ô���ҽ��ڳ���ʵϰ������
// ��ϸ���о��������������⣬�Լ����˸��ҵ���ʾ��
// �ҵĳ������з������õ�����������ĵ�֮����
// ����̲ġ����ñʼǡ����ϵ�Դ�����Լ������ο����ϵĴ����,
// �Ҷ��Ѿ��ڳ����ע����������ע�������õĳ�����
// �Ҵ�δ��Ϯ�����˵ĳ���Ҳû�е��ñ��˵ĳ���
// �������޸�ʽ�ĳ�Ϯ����ԭ�ⲻ���ĳ�Ϯ��
    // �ұ�д������򣬴���û�����Ҫȥ�ƻ���������������ϵͳ��������ת��
    // ����ǧ



/*************************************************
�ļ����ƣ�   regex.h
��Ŀ���ƣ�  mini-vim
�����ߣ�    ����ǧ
����ʱ�䣺   2013-10-09
����޸�ʱ�䣺 2013-10-12
���ܣ�     ʵ��������ʽ��������
�������ļ���������ϵ��
    ͷ�ļ�buffer.h  ���»���洢�ṹ�Ķ���
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

enum NODE_TYPE{CHAR,OP};        //��������׺���ʽ�ڵ�����
enum RFA_NODE_TYPE{EPSILON = -3,SPLIT = -2,RFA_END = -1};       //RFA״̬����
enum DFA_NODE_TYPE{DFA_DEFAULT = 0,DFA_END = -1};       //DFA״̬����
enum DFA_TYPE{DFA_NORMAL = 1,DFA_ABNORMAL = -1};        //DFA����


//���������
typedef struct _RENODE
{
    NODE_TYPE type; //����  �ַ��ͺ������
    char content; //����
}RENODE;


//RFA״̬
typedef struct _RFANODE
{
    static int idcount;//�ܹ��м������
    int id;
    int outc;//���е���һ��״̬����Ҫ���ַ�������Ҳ�����Ƿ�֧������
    _RFANODE* out1;//outc��ָʾҪ���ӵ���һ������ָ��
    _RFANODE* out2;//��֧�ڵ������ã�ָ���֧

    _RFANODE(int outc,_RFANODE* out1,_RFANODE* out2 = null);

}RFANODE;

//RFA
typedef struct _RFA
{
    RFANODE * pstart; //��ʼ���
    set<RFANODE *> pends; //�������
    _RFA(RFANODE * pstart);
}RFA;



//DFA״̬
typedef struct _DFANODE
{
    static int idcount;
    int id;
    _DFANODE* next[CHARSET];    //״̬ת�Ʊ�
    bool *list; //��Ӧ��RFA״̬
	DFA_NODE_TYPE type; //״̬����  �Ƿ�Ϊ����
    _DFANODE(bool *list);

}DFANODE;


//DFA
typedef struct _DFA
{
    DFANODE* pstart;//��ʼ״̬
}DFA;



//���ص�ƥ����  ˫������
typedef struct _REGEX_RESULT
{
	int line, start , len;//�к� ��ʼ���� ����
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

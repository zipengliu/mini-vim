#define null NULL
#define CHARSET 256
#include "buffer.h"
#include<iostream>
#include<stack>
#include<vector>
#include<queue>
#include<set>
using namespace std;


struct _RENODE;
struct _RFANODE;
struct _RFA;
struct _DFANODE;
struct _DFA;
struct _REGEX_RESULT;

typedef struct _RENODE	RENODE;
typedef struct _RFANODE RFANODE;
typedef struct _RFA		RFA;
typedef struct _DFANODE DFANODE;
typedef struct _DFA		DFA;
typedef struct _REGEX_RESULT REGEX_RESULT;


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



//interface
DFA* gen_dfa(char * regex , int len);
int regex_match(DFA* pdfa , line_t* head , REGEX_RESULT* &r);
void regex_release(DFA* pdfa);


int regex_match(line_t* head ,char * regex , int len , REGEX_RESULT* &r);


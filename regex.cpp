#include "regex.h"

struct _RENODE;
struct _RFANODE;
struct _RFA;
struct _DFANODE;
struct _DFA;

enum NODE_TYPE{CHAR,OP};
enum RFA_NODE_TYPE{EPSILON = -3,SPLIT = -2,RFA_END = -1};
enum DFA_NODE_TYPE{DFA_DEFAULT = 0,DFA_END = -1};
enum DFA_TYPE{DFA_NORMAL = 1,DFA_ABNORMAL = -1};

queue<_RENODE*> renodes = queue<_RENODE*>();
stack<_RFA*> rfas = stack<_RFA*>();
queue<_RFANODE*> rfanodes = queue<_RFANODE *>();
set<_DFANODE*> dfanodes = set<_DFANODE*>();


struct _RENODE
{
    NODE_TYPE type;
    char content;
};

struct _RFANODE
{
    static int idcount;
    int id;
    int outc;
    _RFANODE* out1;
    _RFANODE* out2;

    _RFANODE(int outc,_RFANODE* out1,_RFANODE* out2 = null)
    {
        id = idcount++;
        this ->outc = outc;
        this ->out1 = out1;
        this ->out2 = out2;
        rfanodes.push(this);
    }
};

int RFANODE::idcount = 0;



struct _RFA
{
    RFANODE * pstart;
    set<RFANODE *> pends;
    _RFA(RFANODE * pstart)
    {
		pends = set<RFANODE *>();
        this ->pstart = pstart;

    }
};


struct _DFANODE
{
    static int idcount;
    int id;
    _DFANODE* next[CHARSET];
    bool *list;
	DFA_NODE_TYPE type;
    _DFANODE(bool *list)
    {
        id = idcount++;
        this ->list = list;
		if (list[RFANODE::idcount+1])
			type = DFA_END;
		else
			type = DFA_DEFAULT;
		dfanodes.insert(this);
		for(int i = 0 ; i < CHARSET ; i++)
			next[i] = null;
    }

};

int DFANODE::idcount = 0;

struct _DFA
{
    DFANODE* pstart;
};


struct _REGEX_RESULT
{
	int line, start , len;
	struct _REGEX_RESULT *prev , *next;
	_REGEX_RESULT(int line ,int start,int len, _REGEX_RESULT* prev , _REGEX_RESULT* next)
	{
		this ->line = line;
		this ->start = start;
		this ->len = len;
		this ->prev = prev;
		this ->next = next;
	}
};



int gen_tree(const char * word , int len , int pos )
{
    int natom = 0 , nalt = 0;
    RENODE *pnode;
	bool isEscape = false;
    while(pos < len)
    {
		if (isEscape)
		{
			isEscape = false;
			if (natom > 1)
            {
                pnode = new RENODE;
                pnode ->type = OP;
                pnode ->content = '.';
                renodes.push(pnode);
                natom --;
            }
            pnode = new RENODE;
            pnode ->type = CHAR;
            pnode ->content = word[pos];
            renodes.push(pnode);
            natom++;
		}
		else
		{
			switch(word[pos])
			{
			case '\\':
				isEscape = true;
				break;

			case '|':
				while (natom > 1)
				{
					pnode = new RENODE;
					pnode ->type = OP;
					pnode ->content = '.';
					renodes.push(pnode);
					natom -- ;
				}
				natom--;
				nalt ++ ;

				break;


			case '(':
				//cout << natom << endl;
				if (natom > 1)
				{
					pnode = new RENODE;
					pnode ->type = OP;
					pnode ->content = '.';
					renodes.push(pnode);
					natom -- ;
				}
				pos = gen_tree(word,len,++pos);
				natom++;
				break;
			case ')':
				goto out;
				break;


			case '*':case '?':case '+':
				pnode = new RENODE;
				pnode ->type = OP;
				pnode ->content = word[pos];
				renodes.push(pnode);
				break;


			default:
				if (natom > 1)
				{
					pnode = new RENODE;
					pnode ->type = OP;
					pnode ->content = '.';
					renodes.push(pnode);
					natom --;
				}
				pnode = new RENODE;
				pnode ->type = CHAR;
				pnode ->content = word[pos];
				renodes.push(pnode);
				natom++;
				//cout << "plus"<< endl;
				break;
			}
		}
        pos ++ ;
    }
    out:
    for (int i = 0 ; i < natom - 1 ; i++)
    {
        pnode = new RENODE;
        pnode ->type = OP;
        pnode ->content = '.';
        renodes.push(pnode);
    }
    natom -= 2;
    for (int i = 0 ; i < nalt ; i++)
    {
        pnode = new RENODE;
        pnode ->type = OP;
        pnode ->content = '|';
        renodes.push(pnode);
    }

    return pos;
}



RFA * rfa_patch(RFA *p1 , RFA *p2)
{
    for (set<RFANODE *>::iterator it = p1 ->pends.begin(); it != p1 ->pends.end() ; it++)
    {
        (*it) ->out1 = p2 ->pstart;
		if ((*it) ->outc == RFA_END )
			(*it) ->outc = EPSILON;
    }
    p2 ->pstart = p1 ->pstart;
    delete p1;
    return p2;
}


RFA * rfa_merge(RFA *p1 , RFA *p2)
{
    RFANODE* pnode = new RFANODE(SPLIT,p1 ->pstart,p2 ->pstart);
    RFA* p = new RFA(pnode);
    pnode = new RFANODE(RFA_END,null,null);
    for (set<RFANODE *>::iterator it = p1 ->pends.begin(); it != p1 ->pends.end() ; it++)
    {
        (*it) ->out1 = pnode;
		if ((*it) ->outc == RFA_END )
			(*it) ->outc = EPSILON;
    }
    for (set<RFANODE *>::iterator it = p2 ->pends.begin(); it != p2 ->pends.end() ; it++)
    {
        (*it) ->out1 = pnode;
		if ((*it) ->outc == RFA_END )
			(*it) ->outc = EPSILON;
    }
    p ->pends.insert(pnode);
    delete p1;delete p2;
    return p;
}

RFA * rfa_oneornone(RFA *p1)
{
    RFANODE* pnode = new RFANODE(SPLIT,null,p1 ->pstart);
    p1 ->pstart = pnode;
    p1 ->pends.insert(pnode);
    return p1;
}


RFA * rfa_anytimes(RFA *p1)
{
    RFANODE* pnode = new RFANODE(SPLIT,null,p1 ->pstart);
    p1 ->pstart = pnode;
    for (set<RFANODE *>::iterator it = p1 ->pends.begin(); it != p1 ->pends.end() ; it++)
    {
        (*it) ->out1 = pnode;
		if ((*it) ->outc == RFA_END )
			(*it) ->outc = EPSILON;
    }
    p1 ->pends.clear();
    p1 ->pends.insert(pnode);
    return p1;
}


RFA * rfa_morethanonce(RFA * p1)
{
    RFANODE* pnode = new RFANODE(SPLIT,null,p1 ->pstart);
    for (set<RFANODE *>::iterator it = p1 ->pends.begin(); it != p1 ->pends.end() ; it++)
    {
        (*it) ->out1 = pnode;
		if ((*it) ->outc == RFA_END )
			(*it) ->outc = EPSILON;
    }
    p1 ->pends.clear();
    p1 ->pends.insert(pnode);
    return p1;
}

RFA * gen_rfa()
{
    RFA * prfa = null;
    RFA * prfa2 = null;
    RFANODE *prfan = null;
    RENODE *prenode = null;
    while(!renodes.empty())
    {
        prenode = renodes.front();
        renodes.pop();
        if (prenode ->type == CHAR)
        {
            prfan = new RFANODE(prenode ->content,null,null);
            prfa = new RFA(prfan);
            prfa ->pends.insert(prfan);
            rfas.push(prfa);
        }
        else if (prenode ->type == OP)
        {
            switch(prenode ->content)
            {
            case '.':
				if (rfas.size() < 2)
					return null;
                prfa2 = rfas.top();
                rfas.pop();
                prfa = rfas.top();
                rfas.pop();
                prfa = rfa_patch(prfa,prfa2);
                rfas.push(prfa);
                break;
            case '|':
				if (rfas.size() < 2)
					return null;
                prfa2 = rfas.top();
                rfas.pop();
                prfa = rfas.top();
                rfas.pop();
                prfa = rfa_merge(prfa,prfa2);
                rfas.push(prfa);
                break;
            case '?':
				if (rfas.size() < 1)
					return null;
                prfa = rfas.top();
                rfas.pop();
                prfa = rfa_oneornone(prfa);
                rfas.push(prfa);
                break;
            case '*':
				if (rfas.size() < 1)
					return null;
                prfa = rfas.top();
                rfas.pop();
                prfa = rfa_anytimes(prfa);
                rfas.push(prfa);
                break;
            case '+':
				if (rfas.size() < 1)
					return null;
                prfa = rfas.top();
                rfas.pop();
                prfa = rfa_morethanonce(prfa);
                rfas.push(prfa);
                break;
            }
        }
    }
	if (rfas.size() != 1)
		return null;
	prfa = rfas.top();
	rfas.pop();
	RFANODE* pnode = new RFANODE(RFA_END,null,null);
	for (set<RFANODE *>::iterator it = prfa ->pends.begin(); it != prfa ->pends.end() ; it++)
    {
        (*it) ->out1 = pnode;
		if ((*it) ->outc == RFA_END )
			(*it) ->outc = EPSILON;
    }
	prfa ->pends.clear();
    prfa ->pends.insert(pnode);
    return prfa;
}
void rfa_output(RFA * prfa)
{
	cout<< "--------------------RFA info---------------------" << endl;
    cout << "Begin at id: " << prfa ->pstart ->id << endl;
    while(!rfanodes.empty())
    {
        RFANODE * p = rfanodes.front();
        rfanodes.pop();
        cout << "ID:" << p ->id << "  outc: " << p ->outc << "  out1: ";
        if (p ->out1 == null)
        {
            cout << "null";
        }
        else
        {
            cout << p ->out1 ->id;
        }
        cout << "  out2: ";
        if (p ->out2 == null)
        {
            cout << "null";
        }
        else
        {
            cout << p ->out2 ->id;
        }
        cout<<endl;
    }
    cout << "----------------------------------------------------"<<endl << endl;
}


bool* gen_eplosure(vector<RFANODE*> & rfans)
{
    bool* list = new bool[RFANODE::idcount + 2];
    for (int i = 0 ; i < RFANODE::idcount + 2 ; i++)
    {
        list[i] = false;
    }
	for (int i = 0 ; (unsigned)i < rfans.size() ; i++)
    {
		RFANODE* p = rfans[i];
        list[p ->id] = true;
		if(p ->outc == SPLIT || p ->outc == EPSILON)
        {
            if (p ->out1 != null)
				rfans.push_back(p ->out1);
            if (p ->out2 != null)
				rfans.push_back(p ->out2);
        }
		if (p ->out1 == null)
			list[RFANODE::idcount + 1] = true;
    }
    return list;
}



vector<RFANODE*> gen_shift(vector<RFANODE*> & rfans,char c)
{
    vector<RFANODE*> rfans_new = vector<RFANODE*>();
    for (vector<RFANODE *>::iterator it = rfans.begin(); it != rfans.end() &&(*it)!=null; it++)
    {
        if ((*it) ->outc == c)
        {
			if ((*it) ->out1 != null)
				rfans_new.push_back((*it) ->out1);
        }
    }
    return rfans_new;
}

vector<char>  get_next_chars(vector<RFANODE*> & rfans)
{
    vector<char> cset = vector<char>();
    for (vector<RFANODE *>::iterator it = rfans.begin(); it != rfans.end() && (*it)!=null ; it++)
    {
        if ((*it) ->outc >= 0)
        {
			cset.push_back((*it) ->outc);
        }
    }
    return cset;
}


DFANODE* find_dfa(const bool *list)
{
    bool * list2;
    for (set<_DFANODE *>::iterator it = dfanodes.begin(); it != dfanodes.end() ; it++)
    {
		bool notequal = false;
        list2 = (*it) ->list;
        for (int i = 0; i <= RFANODE::idcount ; i++)
        {
            if (list2[i] != list[i])
			{
                notequal = true;
				break;
			}
        }
		if (!notequal)
			return *it;
    }
    return null;
}


DFANODE * gen_dfa_node(vector<RFANODE*> & rfans)
{
    bool* list = gen_eplosure(rfans);
    DFANODE* dfan = find_dfa(list);
    if (dfan != null)
        return dfan;

    dfan = new DFANODE(list);

    vector<char> cset = get_next_chars(rfans);
    vector<RFANODE*> rfan_new;
    for (vector<char>::iterator it = cset.begin(); it != cset.end() ; it++)
    {
        rfan_new = gen_shift(rfans,*it);
        dfan ->next[(int)(*it)] = gen_dfa_node(rfan_new);
    }
	return dfan;

}

void dfa_output(DFA* pdfa)
{
	cout<< "--------------------DFA info---------------------" << endl;
    cout << "Begin at id: " << pdfa ->pstart ->id << endl;
    for (set<_DFANODE *>::iterator it = dfanodes.begin(); it != dfanodes.end() ; it++)
    {
        DFANODE * p = *it;
        cout << "ID:" << p ->id << "  out: " ;
        for(int i = 0 ; i < CHARSET ; i++)
        {
            if (p ->next[i] != NULL)
            {
                cout << "  " <<(char)i <<": "<< p ->next[i] ->id;
            }
        }
		if (p ->type == DFA_END)
			cout << "  END";
        cout<<endl;
    }
    cout << "----------------------------------------------------"<<endl << endl;
}


DFA * gen_dfa(RFA * prfa)
{
    DFA* pdfa = new DFA();
    vector<RFANODE*> rfans = vector<RFANODE*>();
	rfans.push_back(prfa ->pstart);
    pdfa ->pstart = gen_dfa_node(rfans);
    return pdfa;
}



int find_match_in_line(DFA* pdfa , char* c , int len , int line , REGEX_RESULT* &r)
{
	if (c == null)
		return 0;
	int matchcount = 0;
	int p = 0;
	int matchlen = 0,matchpos = 0;
	int matchsuccesslen = 0;
	DFANODE* pnode = pdfa ->pstart;
	bool isfound = false;
	REGEX_RESULT * presult;
	while(p < len)
	{
		if (matchlen != 0 &&pnode ->type == DFA_END)
		{
			isfound = true;
			matchsuccesslen = matchlen;
		}
		if (pnode ->next[(int)(c[p])] != null)
		{
			pnode = pnode ->next[(int)(c[p])];
			matchlen ++;
			p++;
		}
		else
		{
			if (isfound)
			{
				presult = new REGEX_RESULT(line , matchpos , matchsuccesslen , r , r ->next);
				if (r ->next != null)
					r ->next ->prev = presult;
				r ->next = presult;
				r = presult;
				//cout << "Find a match at pos " << matchpos << " and the length is " << matchsuccesslen << endl;
				p = matchpos + matchsuccesslen;
				matchcount++;
			}
			else
			{
				p = matchpos + 1;
			}
			matchlen = 0;
			matchpos = p;
			isfound = false;
			pnode = pdfa ->pstart;
		}
	}
	if (matchlen != 0 &&pnode ->type == DFA_END)
	{
		isfound = true;
		matchsuccesslen = matchlen;
	}
	if (isfound)
	{
		presult = new REGEX_RESULT(line , matchpos , matchsuccesslen , r , r ->next);
		if (r ->next != null)
			r ->next ->prev = presult;
		r ->next = presult;
		matchcount++;
		//cout << "Find a match at pos " << matchpos << " and the length is " << matchlen << endl;
	}
	return matchcount;
}

int regex_match(DFA* pdfa , line_t* head , REGEX_RESULT* &r)
{
	assert(pdfa != null);
	r = new REGEX_RESULT(-1,-1,-1,null,null);
	REGEX_RESULT* presult = r;
	line_t* pline = head ->next;
	int line = 0;
	int matchcount = 0;
	while(pline != null)
	{
		matchcount += find_match_in_line(pdfa,pline ->content,pline ->len,line,presult);
		pline = pline ->next;
		line++;
	}
	return matchcount;
}

DFA* gen_dfa(char * regex , int len)
{
	gen_tree(regex,len,0);

    RFA * prfa = gen_rfa();
	if (prfa == null)
    {
        while(!rfas.empty())
            rfas.pop();
		return null;
    }
    //rfanodes_output(prfa);

    DFA * pdfa = gen_dfa(prfa);
    //dfanodes_output(pdfa);

	delete prfa;

	return pdfa;
}

void regex_release(DFA* pdfa)
{
	while(!rfanodes.empty())
    {
		RFANODE* p = rfanodes.front();
		rfanodes.pop();
		if (p != null)
            delete p;
    }
	for (set<DFANODE *>::iterator it = dfanodes.begin(); it != dfanodes.end() ; it++)
    {
        delete (*it);
    }
	dfanodes.clear();
	while(!renodes.empty())
    {
		RENODE* p = renodes.front();
		rfanodes.pop();
		if (p != null)
            delete p;
    }
	if (pdfa != null)
		delete pdfa;
}


int regex_match(line_t* head ,char * regex , int len , REGEX_RESULT* &r)
{
    DFA* pdfa = gen_dfa(regex ,len);
    int result = -1;
    if (pdfa != null)
        result = regex_match(pdfa , head , r);
    regex_release(pdfa);
    return result;
}

// test block
/*
int main()
{

    char c[6] = {'A','B','|','A','C','\0'};
    char c2[7]  = {'A','B','A','A','A','C'};

    line_t * h = new line_t();
    line_t * h2 = new line_t();
    h2 ->content = c2;
    h2 ->len = 6;
    h ->next = h2;
    REGEX_RESULT* r = null;

    regex_match(h,c,5,r);
    cout << r ->next ->start << r ->next ->next ->start;

}
*/

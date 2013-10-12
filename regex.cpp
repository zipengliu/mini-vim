/*************************************************
文件名称：   regex.cpp
项目名称：  mini-vim
创建者：    兰兆千
创建时间：   2013-10-09
最后修改时间： 2013-10-12
功能：     实现正则表达式搜索功能
与其他文件的依赖关系：
    头文件regex.h
**************************************************/


#include "regex.h"


//全局变量区   均用于生成DFA的过程
queue<_RENODE*> renodes = queue<_RENODE*>();   //构造树状态队列
stack<_RFA*> rfas = stack<_RFA*>();     //RFA栈
queue<_RFANODE*> rfanodes = queue<_RFANODE *>();    //RFA状态栈
set<_DFANODE*> dfanodes = set<_DFANODE*>(); //DFA状态集合



//构造函数
_RFANODE::_RFANODE(int outc,_RFANODE* out1,_RFANODE* out2)
{
    id = idcount++;
    this ->outc = outc;
    this ->out1 = out1;
    this ->out2 = out2;
    rfanodes.push(this);
}


int RFANODE::idcount = 0;


_RFA::_RFA(RFANODE * pstart)
{
    pends = set<RFANODE *>();
    this ->pstart = pstart;

}


_DFANODE::_DFANODE(bool *list)
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
int DFANODE::idcount = 0;



_REGEX_RESULT::_REGEX_RESULT(int line ,int start,int len, _REGEX_RESULT* prev , _REGEX_RESULT* next)
{
    this ->line = line;
    this ->start = start;
    this ->len = len;
    this ->prev = prev;
    this ->next = next;
}


/*************************************************
函数名称：   gen_tree
功能：      根据输入的正则表达式，生成后缀表达式
            （递归调用）
输入参数：正则表达式字符串指针，长度，目前已经匹配到的位置
返回值：  匹配结束后匹配到的位置
*************************************************/
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


/*************************************************
函数名称：   rfa_patch
功能：      将两个rfa顺序连接起来，第一个rfa的
            终止状态是第二个rfa的开始状态
输入参数：两个待操作的rfa指针
返回值：  连接完毕的rfa指针
*************************************************/
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

/*************************************************
函数名称：   rfa_merge
功能：      将两个rfa顺序连接起来，头部相接，尾部相接
        得到一个具有两个分支的rfa
输入参数：两个待操作的rfa指针
返回值：  连接完毕的rfa指针
*************************************************/
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



/*************************************************
函数名称：   rfa_oneornone
功能：      将一个RFA改为能够识别1次或者0次目前该
        RFA能够识别的语言的RFA
        对应的是正则表达式中的‘？’
输入参数：待操作的rfa指针
返回值：  改造完毕的rfa指针
*************************************************/
RFA * rfa_oneornone(RFA *p1)
{
    RFANODE* pnode = new RFANODE(SPLIT,null,p1 ->pstart);
    p1 ->pstart = pnode;
    p1 ->pends.insert(pnode);
    return p1;
}

/*************************************************
函数名称：   rfa_anytimes
功能：      将一个RFA改为能够识别 任意次 目前该
        RFA能够识别的语言 的RFA
        对应的是正则表达式中的‘*’
输入参数：待操作的rfa指针
返回值：  改造完毕的rfa指针
*************************************************/
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

/*************************************************
函数名称：   rfa_morethanonce
功能：      将一个RFA改为能够识别 对于一次 目前该
        RFA能够识别的语言 的RFA
        对应的是正则表达式中的‘+’
输入参数：待操作的rfa指针
返回值：  改造完毕的rfa指针
*************************************************/
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



/*************************************************
函数名称：   gen_rfa
功能：     根据解析树得到的后缀表达式构造RFA
输入参数：无 （但利用到全局变量renodes）
返回值：  建立的RFA指针
*************************************************/
RFA * gen_rfa()
{
    RFA * prfa = null;
    RFA * prfa2 = null;
    RFANODE *prfan = null;
    RENODE *prenode = null;
    while(!renodes.empty()) //未完成构造，后缀表达式里面还有元素
    {
        prenode = renodes.front();  //取出队首元素
        renodes.pop();
        if (prenode ->type == CHAR) //如果是字符，创建一个新的RFA，只接受这一个字符
        {
            prfan = new RFANODE(prenode ->content,null,null);
            prfa = new RFA(prfan);
            prfa ->pends.insert(prfan);
            rfas.push(prfa);
        }
        else if (prenode ->type == OP)  //如果是运算符，取出RFA栈中的若干元素进行修改或者合并操作
        {
            switch(prenode ->content)
            {
            case '.':   //连接运算符
				if (rfas.size() < 2)
					return null;
                prfa2 = rfas.top();
                rfas.pop();
                prfa = rfas.top();
                rfas.pop();
                prfa = rfa_patch(prfa,prfa2);
                rfas.push(prfa);
                break;
            case '|':   //分支运算符
				if (rfas.size() < 2)
					return null;
                prfa2 = rfas.top();
                rfas.pop();
                prfa = rfas.top();
                rfas.pop();
                prfa = rfa_merge(prfa,prfa2);
                rfas.push(prfa);
                break;
            case '?':   //只重复一次或0次
				if (rfas.size() < 1)
					return null;
                prfa = rfas.top();
                rfas.pop();
                prfa = rfa_oneornone(prfa);
                rfas.push(prfa);
                break;
            case '*':   //重复任意次
				if (rfas.size() < 1)
					return null;
                prfa = rfas.top();
                rfas.pop();
                prfa = rfa_anytimes(prfa);
                rfas.push(prfa);
                break;
            case '+':   //重复一次及以上
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

    //最后应该只剩下一个RFA
	if (rfas.size() != 1)
		return null;



	prfa = rfas.top();
	rfas.pop();


	//将RFA的最后添加一个空状态，作为结束状态
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



/*************************************************
函数名称：   rfa_output
功能：     向控制台输出一个RFA的信息
输入参数：要输出的RFA
返回值：  空
*************************************************/
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




/*************************************************
函数名称：   gen_eplosure
功能：    得到一个RFA状态集合的ε闭包
输入参数：RFA状态集合的引用
返回值：  指示状态集合中的是否包含某RFA状态的布尔型数组
*************************************************/
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


/*************************************************
函数名称：   gen_shift
功能：    得到一个RFA状态集合的输入某字符后
            状态转移得到的状态集合
输入参数：RFA状态集合的引用，转移字符
返回值：  状态转移得到的状态集合
*************************************************/
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



/*************************************************
函数名称：   get_next_chars
功能：    得到一个RFA状态集合可以输入那些字符进入下一个状态集合
输入参数：RFA状态集合引用
返回值：  能够进行转移的字符集合
*************************************************/
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



/*************************************************
函数名称：   find_dfa
功能：    根据DFA状态代表的RFA状态集合，寻找DFA状态
输入参数：指示所代表的RFA状态的bool数组
返回值：  寻找到的DFA状态，未找到则返回null
*************************************************/
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



/*************************************************
函数名称：   gen_dfa_node
功能：    根据DFA状态代表的RFA状态集合，生成一个新的DFA状态
输入参数：RFA状态集合
返回值：  建立的DFA状态
*************************************************/
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



/*************************************************
函数名称：   dfa_output
功能：    向控制台输出DFA信息
输入参数：DFA指针
返回值：  无
*************************************************/
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



/*************************************************
函数名称：   gen_dfa
功能：    根据一个RFA生成一个DFA
输入参数：RFA指针
返回值：  生成的DFA指针
*************************************************/
DFA * gen_dfa(RFA * prfa)
{
    DFA* pdfa = new DFA();
    vector<RFANODE*> rfans = vector<RFANODE*>();
	rfans.push_back(prfa ->pstart);
    pdfa ->pstart = gen_dfa_node(rfans);
    return pdfa;
}


/*************************************************
函数名称：   find_match_in_line
功能：    根据一行的字符，寻找匹配，并将结果插入双向链表
输入参数：DFA指针，字符串指针，字符串长度len，行号，结果插入位置
返回值：  找到的匹配个数
*************************************************/
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


/*************************************************
函数名称：   regex_match
功能：    根据DFA和一篇文章，寻找匹配
输入参数：DFA指针，文章按行存储的双向链表头，结果双向链表头引用
返回值：  找到的匹配个数
*************************************************/
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
	    //按行搜索，结果相加
		matchcount += find_match_in_line(pdfa,pline ->content,pline ->len,line,presult);
		pline = pline ->next;
		line++;
	}
	return matchcount;
}



/*************************************************
函数名称：   gen_dfa(char * regex , int len)
功能：    根据正则表达式，得到DFA
        将前面所有的过程封装
输入参数：正则表达式的字符串头指针，长度
返回值：  生成的DFA
*************************************************/
DFA* gen_dfa(char * regex , int len)
{
    //先生成构造树得到后缀表达式
	gen_tree(regex,len,0);

    //然后生成RFA
    RFA * prfa = gen_rfa();
	if (prfa == null)
    {
        while(!rfas.empty())
            rfas.pop();
		return null;
    }
    //rfanodes_output(prfa);

    //再生成DFA
    DFA * pdfa = gen_dfa(prfa);
    //dfanodes_output(pdfa);

	delete prfa;

	return pdfa;
}




/*************************************************
函数名称：   regex_release
功能：    释放所有资源
输入参数：DFA
返回值：  无
*************************************************/
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
    RFANODE::idcount = 0;
    DFANODE::idcount = 0;
}




/*************************************************
函数名称：   regex_match
功能：    根据文章，正则表达式，返回匹配结果
            是对外的接口，之前所有过程的封装
输入参数：文章双向链表，正则表达式头，正则表达式长度，结果引用
返回值：  匹配个数 ， -1则是正则表达式不合法
*************************************************/
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

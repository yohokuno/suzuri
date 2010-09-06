/*-----------------------------------------------------------------------------

  NkScript.cpp

-----------------------------------------------------------------------------*/
#include "stdapp.h"
#include "include.h"

//グラフィック
CObjArray<CNkImage> g_image(SCRIPT_IMAGE_NUM);
//サウンド
CObjArray<CNkSound> g_sound(SCRIPT_SOUND_NUM);
/*-----------------------------------------------------------------------------
	文字列処理
-----------------------------------------------------------------------------*/
//文字列と文字群を比べ、一致しない最初を返す
char* strstr2(char* str,char* chrArray)
{
	for(char* p1=str;p1[0]!='\0';++p1)
	{
		bool ret=true;
		for(char* p2=chrArray;p2[0]!='\0';++p2)
		{
			if(p1[0]==p2[0])
				ret=false;
		}
		if(ret)
			return p1;
	}
	return p1;
}
//文字と文字列を比べる
bool chrstr(int chr,char* chrArray)
{
	for(char* p=chrArray;p[0];++p)
		if(p[0]==chr)
			return true;
	return false;
}
//文字列の最初と最後をポインタで指定してＮｅｗする
char* newstr(char* pFront,char* pRear)
{
	char* ret = new char[pRear-pFront+1];
	strncpy(ret,pFront,pRear-pFront);
	ret[pRear-pFront] = '\0';
	return ret;
}
//区切り文字cで区切られた文字列群str2と最初のｎ文字が一致するか
bool strstrstr(char* str1,char* str2,int c)
{
	char* pFront=str2;
	char* pRear;
	while(pRear = strchr(pFront,c))
	{
		char* buffer=newstr(pFront,pRear);
		if(strstr(str1,buffer) == str1)
		{
			SAFE_DELETE_ARRAY(buffer);
			return true;
		}
		SAFE_DELETE_ARRAY(buffer);
		pFront=pRear+1;
	}
	//最後
	if(strstr(str1,pFront)==str1)
		return true;
	return false;
}
//区切り文字cで区切られた文字列群str2と完全に一致するか
bool strstrstr2(char* str1,char* str2,int c)
{
	char* pFront=str2;
	char* pRear;
	while(pRear = strchr(pFront,c))
	{
		char* buffer=newstr(pFront,pRear);
		if(strcmp(str1,buffer) == 0)
		{
			SAFE_DELETE_ARRAY(buffer);
			return true;
		}
		SAFE_DELETE_ARRAY(buffer);
		pFront=pRear+1;
	}
	//最後
	if(strcmp(str1,pFront)==0)
		return true;
	return false;
}
//文字列が定数をあらわすトークンかどうか
bool IsConst(char* str)
{
	return chrstr(str[0],"0123456789")||chrstr(str[0],"+-.") && chrstr(str[1],"0123456789");
}
//文字列が変数をあらわすトークンかどうか
bool IsVar(char* str)
{
	return chrstr(str[0],"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_") &&
		strstr2(str,"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789")[0]=='\0';
}
//ファイル読み込み
bool ReadFile(char* buffer,int size,const char* lpszFileName)
{
	HANDLE hFile;
	hFile = CreateFile(lpszFileName,
		GENERIC_READ,
		FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if(hFile == INVALID_HANDLE_VALUE)
		return 0;
	//ここから読み込み
	DWORD dwSize;
	int ret = ReadFile(
		hFile,
		buffer,
		size,
		&dwSize,
		NULL);
	ret &= CloseHandle(hFile);
	if(ret)
		buffer[dwSize] = NULL;
	return (ret!=0);
}
//ファイル読み込み・内部バッファ作成バージョン
char* ReadFile(const char* lpszFileName)
{
	HANDLE hFile;
	hFile = CreateFile(lpszFileName,
		GENERIC_READ,
		FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if(hFile == INVALID_HANDLE_VALUE)
		return 0;
	//ファイルサイズを得る
	DWORD size;
	size=GetFileSize(hFile,NULL);
	char *ret=new char[size];
	//ここから読み込み
	DWORD dwSize;
	int flag = ReadFile(
		hFile,
		ret,
		size,
		&dwSize,
		NULL);
	flag &= CloseHandle(hFile);
	if(flag)
	{
		ret[dwSize] = NULL;
		return ret;
	}
	else
	{
		SAFE_DELETE_ARRAY(ret);
		return NULL;
	}
}
//ファイルサイズ
int GetFileSize(const char* lpszFileName)
{
	HANDLE hFile;
	hFile = CreateFile(lpszFileName,
		GENERIC_READ,
		FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if(hFile == INVALID_HANDLE_VALUE)
		return 0;
	int ret=GetFileSize(hFile,NULL);
	CloseHandle(hFile);
	return ret;
}
/*-----------------------------------------------------------------------------
	トークン用クラス・構造体の実装
-----------------------------------------------------------------------------*/
CToken::CToken()
{
	m_pString=NULL;
}
//トークンを切る(文字列へのポインタ、前のトークン）
char* CToken::Cut(char* str,char* former)
{
	if(str==NULL||str[0]=='\0')
	{
		m_pString=NULL;
		return NULL;
	}
	char* pFront = str;
	//strを整文文字のぶん進める
	while(chrstr(pFront[0]," \t\n\r")!=NULL)	//\ｒは改行文字の一種
	{
		pFront++;
		if(pFront[0]=='\0')
			return NULL;
	}
	//Cコメントの場合
	if(strstr(pFront,"/*")==pFront)
	{
		char* p=strstr(pFront,"*/");
		if(p==NULL)
			return NULL;
		return Cut(p+2,former);
	}
	//C++コメントの場合
	else if(strstr(pFront,"//")==pFront)
	{
		char* p=strstr(pFront,"\r\n");
		if(p==NULL)
			return NULL;
		return Cut(p+1,former);
	}
	//定数の場合
	else if(chrstr(pFront[0],"0123456789")||	//最初が数字
		chrstr(pFront[0],"+-")&&chrstr(pFront[1],"0123456789")&&
		(former==NULL||
		!IsConst(former)&&!IsVar(former)&&strcmp(former,")")!=0))	//符号＋数字（演算子の＋－と区別）
	{
		char* pRear = strstr2(pFront+1,".0123456789");
		m_pString = newstr(pFront,pRear);
		return pRear;
	}
	//文字列の場合
	else if(strstr(pFront,"\"")==pFront)
	{
		char* pRear = strchr(pFront+1,'\"');
		if(pRear!=NULL)
			m_pString = newstr(pFront,pRear+1);
		return pRear+1;
	}
	//二文字演算子の場合
	else if(strstrstr(pFront,"+=,-=,++,--,==,!=,<=,>=,||,&&",','))
	{
		m_pString = newstr(pFront,pFront+2);
		return pFront+2;
	}
	//その他の記号の場合
	else if(chrstr(pFront[0],"()+-*=/<>&|!#%'~^[]{}@`?:;,."))
	{
		m_pString = newstr(pFront,pFront+1);
		return pFront+1;
	}
	//関数名や変数名の場合
	else if(chrstr(pFront[0],"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_"))
	{
		char* pRear = strstr2(pFront,"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789");
		m_pString = newstr(pFront,pRear);
		return pRear;
	}
	else
	{
		char buffer[2]={pFront[0],'\0'};
		MBF_ERR("%s:トークンとして認識できません。",buffer);
		return NULL;
	}
}
//デストラクタ
CTokenArray::~CTokenArray()
{
	for(int i=0;i<m_vctpToken.size();++i)
		SAFE_DELETE(m_vctpToken[i]);
}
//トークンへ切り分ける
bool CTokenArray::CutToToken(char* str)
{
	char* p=str;
	do
	{
		char* former=NULL;
		if(m_vctpToken.size()>0)
			former=(*(m_vctpToken.end()-1))->GetStr();
		CToken* pToken = new CToken;
		p = pToken->Cut(p,former);
		if(p!=NULL&&pToken->GetStr()!=NULL)
			m_vctpToken.push_back(pToken);
		else
			SAFE_DELETE(pToken);
	}
	while(p);

	return true;
}
//全printf
void CTokenArray::allprintf()
{
	for(int i=0;i<m_vctpToken.size();++i)
		m_vctpToken[i]->Printf();
}
//（、｛に対応する）、｝を得る
int CTokenArray::GetCorrespondence(int index,const char* kakko1,const char* kakko2)
{
	int i;
	int depth=0;
	for(i=index;i!=m_vctpToken.size() && m_vctpToken[i]!=NULL;++i)
	{
		if(m_vctpToken[i]->GetStr()!=NULL)
		{
			if(*m_vctpToken[i]==kakko2)
			{
				depth--;
				if(depth==0)
					return i;
			}
			else if(*m_vctpToken[i]==kakko1)
				depth++;
		}
	}
	return -1;
}
//文字列を検索する
int CTokenArray::Search(int index,const char* str)
{
	for(int i=index;i!=m_vctpToken.size();++i)
		if(*m_vctpToken[i]==str)
			return i;
	return -1;
}
//カンマを検索する。このさい、（）と｛｝による「深さ」が違っていたら無視する。
int CTokenArray::SearchCommma(int first,int last)
{
	int depth=0;
	for(int i=first;i<last;++i)
	{
		if(*m_vctpToken[i]=="," && depth==0)
			return i;
		if(*m_vctpToken[i]=="(")
			depth++;
		if(*m_vctpToken[i]==")")
			depth--;
	}
	return -1;
}
//区切り文字ｃで区切られた文字列郡ｓｔｒ（演算子）を検索する。
int CTokenArray::SearchEnzansi(char* str,int c,int first,int last)
{
	int depth=0;
	for(int i=first;i<last;++i)
	{
		if(strstrstr2(m_vctpToken[i]->GetStr(),str,c) && depth ==0)
			return i;
		if(*m_vctpToken[i]=="(")
			depth++;
		if(*m_vctpToken[i]==")")
			depth--;
	}
	return -1;
}
//優先順位を返す
int TokenIndex::Get()const
{
	char* p=(*m_pTokenArray)[m_index]->GetStr();
	if(strstrstr2(p,"=,+=,-=",',' ))
		return 0;
	if(strstr(p,"||")==p)
		return 10;
	if(strstr(p,"&&")==p)
		return 20;
	if(strstrstr2(p,"==,!=",','))
		return 30;
	if(strstrstr2(p,"<,<=,>,>=",','))
		return 40;
	if(strstrstr2(p,"+,-",','))
		return 50;
	if(strstrstr2(p,"*,/,%",','))
		return 60;
	//その他
	return -1;
}
/*-----------------------------------------------------------------------------
		ここから本体
-----------------------------------------------------------------------------*/
//文字列から変数を作る
CVar* CLogicalBlock::NewVariable(const char* str)
{
	//pBlockの変数名と照合
	for(int i=0;i<GetVarNum();++i)
	{
		if(strcmp(str,GetVarNameArray()[i]) == 0)
		{
			return new CVariable(this,i);
		}
	}
	//pBlockの親ブロックへ丸投げ
	if(GetBlock())
		return GetBlock()->NewVariable(str);
	return NULL;
}
//ブロックオブジェクトのデストラクタ
CLogicalBlock::~CLogicalBlock()
{
	list<CFunc*>::iterator itr;
	for(itr = m_lstpFunc.begin(); itr != m_lstpFunc.end(); ++itr)
		SAFE_DELETE(*itr);
	m_lstpFunc.clear();
}
//ブロックへトークンから読み込み
bool CLogicalBlock::Read(CBlock* pBlock,CTokenArray* pTokenArray,int first)
{
	m_pBlock=pBlock;
	m_pVar=NULL;
	m_varnum=0;
	for(int i=0;i<VARNUM;++i)
		m_varname[i]=NULL;
	int last = pTokenArray->GetCorrespondence(first,"{","}");
	if(last==-1)
	{
		MB_ERR("{に対応する}がありません。");
		return false;
	}
	for(i=first+1;i!=last;++i)
	{
		if(*(*pTokenArray)[i]=="double" || *(*pTokenArray)[i]=="var")	//変数宣言文
		{
			++i;
			for(;i!=last&&(*pTokenArray)[i]!=NULL;++i)
			{
				if(IsVar((*pTokenArray)[i]->GetStr()))
				{
					m_varname[m_varnum]=_strdup(((*pTokenArray)[i])->GetStr());
					m_varnum++;
					i++;
					if(*(*pTokenArray)[i]==";")
						break;
					else if(*(*pTokenArray)[i]==",")
						continue;
					else
					{
						MBF_ERR("%s:変数宣言文内のトークンとして不適切です。",(*pTokenArray)[i]->GetStr());
						return false;;
					}
				}
				else
				{
					MBF_ERR("%s:変数名として不適切です。",(*pTokenArray)[i]->GetStr());
					return false;
				}
			}
		}
		else//関数文
		{
			int last2=i;
			CFunc* pFunc = NewFunc2(this,pTokenArray,i,last2);
			if(pFunc)
				m_lstpFunc.push_back(pFunc);
			i=last2;
		}
	}
	for(i=0;i<VARNUM;++i)
	{
		if(m_varname[i])
		{
			SAFE_DELETE_ARRAY(m_varname[i]);
		}
	}
	return true;
}
//トークンリストから文を作る(最初指定）
//if,while系の制御文で使う
CFunc* NewFunc2(CBlock* pBlock,CTokenArray* pTokenArray,int first,int& last)
{
	int i=first;
	if(*(*pTokenArray)[i]==";") //空文
	{
		last++;
		return NewFunc2(pBlock,pTokenArray,i+1,last);
	}
	else if(*(*pTokenArray)[i]=="{") //ブロック文
	{
		last = pTokenArray->GetCorrespondence(i,"{","}");
		if(last<0) return NULL;
		CLogicalBlock* pBlock2 = new CLogicalBlock;
		if(pBlock2->Read(pBlock,pTokenArray,i))
			return pBlock2;
		else 
		{
			SAFE_DELETE(pBlock);
			return NULL;
		}
	}
	else if(*(*pTokenArray)[i]=="if") //if
	{
		if(*(*pTokenArray)[i+1]!="(")
		{
			MB_ERR("ifに(がありません。");
			return NULL;
		}
		int last2 = pTokenArray->GetCorrespondence(i+1,"(",")");
		if(last2<0)
		{
			MB_ERR("ifに)がありません。");
			return NULL;
		}
		CFunc* pJoken = NewFunc(pBlock,pTokenArray,i+2,last2);	//条件式
		CFunc* pFunc = NewFunc2(pBlock,pTokenArray,last2+1,last);
		CFunc* pElse = NULL;
		if(*(*pTokenArray)[last+1]=="else")
			pElse = NewFunc2(pBlock,pTokenArray,last+2,last);
		if(pJoken && pFunc)
			return new CIf(pJoken,pFunc,pElse);
		SAFE_DELETE(pJoken);
		SAFE_DELETE(pBlock);
		SAFE_DELETE(pElse);
		return NULL;
	}
	else if(*(*pTokenArray)[i]=="while") //while
	{
		if(*(*pTokenArray)[i+1]!="(")
		{
			MB_ERR("whileに(がありません。");
			return NULL;
		}
		int last2 = pTokenArray->GetCorrespondence(i+1,"(",")");
		if(last2<0)
		{
			MB_ERR("whileに)がありません。");
			return NULL;
		}
		CFunc* pJoken = NewFunc(pBlock,pTokenArray,i+2,last2);	//条件式
		CFunc* pFunc = NewFunc2(pBlock,pTokenArray,last2+1,last);
		if(pJoken && pFunc)
			return new CWhile(pJoken,pFunc);
		SAFE_DELETE(pJoken);
		SAFE_DELETE(pFunc);
		return NULL;
	}
	else if(*(*pTokenArray)[i]=="for") //for
	{
		if(*(*pTokenArray)[i+1]!="(")
		{
			MB_ERR("forに(がありません。");
			return NULL;
		}
		int last2 = pTokenArray->GetCorrespondence(i+1,"(",")");
		if(last2<0)
		{
			MB_ERR("forに)がありません。");
			return NULL;
		}
		int colon1=pTokenArray->Search(i+1,";");
		if(colon1<0)
		{
			MB_ERR("forに;がありません。");
			return NULL;
		}
		int colon2=pTokenArray->Search(colon1+1,";");
		if(colon2<0)
		{
			MB_ERR("forに;がありません。");
			return NULL;
		}
		CFunc* pFirst = NewFunc(pBlock,pTokenArray,i+2,colon1);		//初期化式
		CFunc* pJoken = NewFunc(pBlock,pTokenArray,colon1+1,colon2);//条件式
		CFunc* pStep = NewFunc(pBlock,pTokenArray,colon2+1,last2);	//第三式
		CFunc* pFunc = NewFunc2(pBlock,pTokenArray,last2+1,last);	//ブロック
		if(pFirst && pJoken && pStep && pFunc)
			return new CFor(pFirst,pJoken,pStep,pFunc);
		SAFE_DELETE(pFirst);
		SAFE_DELETE(pJoken);
		SAFE_DELETE(pStep);
		SAFE_DELETE(pFunc);
		return NULL;
	}
	else if(*(*pTokenArray)[i]=="break")
	{
		int gyou=pTokenArray->Search(i,";");
		if(gyou!=i+1)
		{
			MB_ERR("breakに;がありません。");
			return NULL;
		}
		last=gyou;
		return new CBreak();
	}
	else//関数文
	{
		int gyou=pTokenArray->Search(i,";");
		last = gyou;
		if(gyou==-1)
		{
			MB_ERR(";がありません。");
			return NULL;
		}
		CFunc* pFunc = NewFunc(pBlock,pTokenArray,i,gyou);
		if(pFunc)
			return pFunc;	//文の文字列から関数オブジェクトを作る
	}
	return NULL;
}
//トークンリストから関数を作る
CFunc* NewFunc(CBlock* pBlock,CTokenArray* pTokenArray,int first,int last)
{
	CFunc* ret=NULL;
	int i=first;
	//定数
	if(IsConst((*pTokenArray)[i]->GetStr()) && last==i+1)
		return new CConst(atof((*pTokenArray)[i]->GetStr()));
	//カッコ式
	if(*(*pTokenArray)[i]=="(" && pTokenArray->GetCorrespondence(i,"(",")")==last-1)
	{
		return NewFunc(pBlock,pTokenArray,first+1,last-1);
	}
	//演算子
	if(pTokenArray->SearchEnzansi("=,+=,-=,||,&&,==,!=,<,<=,>,>=,+,-,*,/,%",',',first,last) >=0)
	{
		vector<TokenIndex> vctTokenIndex;	//インデックスヴェクター（ソート用）
		int j=pTokenArray->SearchEnzansi("=,+=,-=,||,&&,==,!=,<,<=,>,>=,+,-,*,/,%",',',first,last);
		while(j>=0)
		{
			//printf("%d:%s\n",j,(*pTokenArray)[j]->GetStr());
			TokenIndex tokenIndex;
			tokenIndex.m_index = j;
			tokenIndex.m_pTokenArray = pTokenArray;
			vctTokenIndex.push_back(tokenIndex);	//ヴェクターに追加
			j=pTokenArray->SearchEnzansi("=,+=,-=,||,&&,==,!=,<,<=,>,>=,+,-,*,/,%",',',j+1,last);
		}
		stable_sort(vctTokenIndex.begin(),vctTokenIndex.end());	//同値な並び順は保持してソート
		int index=(vctTokenIndex.end()-1)->m_index;	//最も優先順位の低い演算子へのインデックス
		int H1F=first;
		int H1L=index;
		int H2F=index+1;
		int H2L=last;
		//Ｅｑｕａｌ系演算子
		CVar* pVariable=NULL;
		CFunc* pFunc=NULL;
		if(*(*pTokenArray)[index]=="=")
		{
			pVariable = pBlock->NewVariable((*pTokenArray)[H1F]->GetStr());
			pFunc = NewFunc(pBlock,pTokenArray,H2F,H2L);
			if(pVariable && pFunc)
				return new CEqual(pVariable,pFunc);
		}
		else if(*(*pTokenArray)[index]=="+=")
		{
			pVariable = pBlock->NewVariable((*pTokenArray)[H1F]->GetStr());
			pFunc = NewFunc(pBlock,pTokenArray,H2F,H2L);
			if(pVariable && pFunc)
				return new CPlusEqual(pVariable,pFunc);
		}
		else if(*(*pTokenArray)[index]=="-=")
		{
			pVariable = pBlock->NewVariable((*pTokenArray)[H1F]->GetStr());
			pFunc = NewFunc(pBlock,pTokenArray,H2F,H2L);
			if(pVariable && pFunc)
				return new CMinusEqual(pVariable,pFunc);
		}
		SAFE_DELETE(pVariable);
		SAFE_DELETE(pFunc);
		//通常系２項演算子
		CFunc* pFunc1=NULL;
		CFunc* pFunc2=NULL;
		if(*(*pTokenArray)[index]=="+")
		{
			pFunc1 = NewFunc(pBlock,pTokenArray,H1F,H1L);
			pFunc2 = NewFunc(pBlock,pTokenArray,H2F,H2L);
			if(pFunc1 && pFunc2)
				return new CPlus(pFunc1,pFunc2);
		}
		else if(*(*pTokenArray)[index]=="-")
		{
			pFunc1 = NewFunc(pBlock,pTokenArray,H1F,H1L);
			pFunc2 = NewFunc(pBlock,pTokenArray,H2F,H2L);
			if(pFunc1 && pFunc2)
				return new CMinus(pFunc1,pFunc2);
		}
		else if(*(*pTokenArray)[index]=="*")
		{
			pFunc1 = NewFunc(pBlock,pTokenArray,H1F,H1L);
			pFunc2 = NewFunc(pBlock,pTokenArray,H2F,H2L);
			if(pFunc1 && pFunc2)
				return new CMultiply(pFunc1,pFunc2);
		}
		else if(*(*pTokenArray)[index]=="/")
		{
			pFunc1 = NewFunc(pBlock,pTokenArray,H1F,H1L);
			pFunc2 = NewFunc(pBlock,pTokenArray,H2F,H2L);
			if(pFunc1 && pFunc2)
				return new CDivide(pFunc1,pFunc2);
		}
		else if(*(*pTokenArray)[index]=="%")
		{
			pFunc1 = NewFunc(pBlock,pTokenArray,H1F,H1L);
			pFunc2 = NewFunc(pBlock,pTokenArray,H2F,H2L);
			if(pFunc1 && pFunc2)
				return new CSurplus(pFunc1,pFunc2);
		}
		else if(*(*pTokenArray)[index]=="==")
		{
			pFunc1 = NewFunc(pBlock,pTokenArray,H1F,H1L);
			pFunc2 = NewFunc(pBlock,pTokenArray,H2F,H2L);
			if(pFunc1 && pFunc2)
				return new CEqualEqual(pFunc1,pFunc2);
		}
		else if(*(*pTokenArray)[index]=="!=")
		{
			pFunc1 = NewFunc(pBlock,pTokenArray,H1F,H1L);
			pFunc2 = NewFunc(pBlock,pTokenArray,H2F,H2L);
			if(pFunc1 && pFunc2)
				return new CNotEqual(pFunc1,pFunc2);
		}
		else if(*(*pTokenArray)[index]==">")
		{
			pFunc1 = NewFunc(pBlock,pTokenArray,H1F,H1L);
			pFunc2 = NewFunc(pBlock,pTokenArray,H2F,H2L);
			if(pFunc1 && pFunc2)
				return new CIsLarger(pFunc1,pFunc2);
		}
		else if(*(*pTokenArray)[index]=="<")
		{
			pFunc1 = NewFunc(pBlock,pTokenArray,H1F,H1L);
			pFunc2 = NewFunc(pBlock,pTokenArray,H2F,H2L);
			if(pFunc1 && pFunc2)
				return new CIsSmaller(pFunc1,pFunc2);
		}
		else if(*(*pTokenArray)[index]==">=")
		{
			pFunc1 = NewFunc(pBlock,pTokenArray,H1F,H1L);
			pFunc2 = NewFunc(pBlock,pTokenArray,H2F,H2L);
			if(pFunc1 && pFunc2)
				return new CIsLargerEqual(pFunc1,pFunc2);
		}
		else if(*(*pTokenArray)[index]=="<=")
		{
			pFunc1 = NewFunc(pBlock,pTokenArray,H1F,H1L);
			pFunc2 = NewFunc(pBlock,pTokenArray,H2F,H2L);
			if(pFunc1 && pFunc2)
				return new CIsSmallerEqual(pFunc1,pFunc2);
		}
		else if(*(*pTokenArray)[index]=="&&")
		{
			pFunc1 = NewFunc(pBlock,pTokenArray,H1F,H1L);
			pFunc2 = NewFunc(pBlock,pTokenArray,H2F,H2L);
			if(pFunc1 && pFunc2)
				return new CAndAnd(pFunc1,pFunc2);
		}
		else if(*(*pTokenArray)[index]=="||")
		{
			pFunc1 = NewFunc(pBlock,pTokenArray,H1F,H1L);
			pFunc2 = NewFunc(pBlock,pTokenArray,H2F,H2L);
			if(pFunc1 && pFunc2)
				return new COrOr(pFunc1,pFunc2);
		}
		SAFE_DELETE(pFunc1);
		SAFE_DELETE(pFunc2);
		MBF_ERR("%s: この演算子は未実装です。",(*pTokenArray)[index]->GetStr());
		return NULL;
	}
	//残りは変数か関数
	if(IsVar((*pTokenArray)[i]->GetStr()))	//識別子であることを確認しておく
	{
		//変数
		CVar* pVar = pBlock->NewVariable((*pTokenArray)[i]->GetStr());
		if(pVar!=NULL)
			return pVar;
		//関数
		if(i+1==last)return NULL;
		int corKakko=pTokenArray->GetCorrespondence(i+1,"(",")");//（に対応する）
		if(*(*pTokenArray)[i+1]=="(" && corKakko!=-1)
		{
			if(corKakko!=last-1)
			{
				MBF_ERR("%s:　；がありません。",(*pTokenArray)[i]->GetStr());
				return NULL;
			}
			{//引数なし関数
				int H1L=pTokenArray->GetCorrespondence(first+1,"(",")");//（に対応する）
				if(H1L<0) return NULL;
				if(*(*pTokenArray)[i]=="rnd")
					return new CRnd;
				if(*(*pTokenArray)[i]=="WaitTime")
					return new CWaitTime;
				if(*(*pTokenArray)[i]=="UpdateInput")
					return new CUpdateInput;
				if(*(*pTokenArray)[i]=="DrawList")
					return new CDrawList;
				if(*(*pTokenArray)[i]=="ShowFPS")
					return new CShowFPS;
				if(*(*pTokenArray)[i]=="Flip")
					return new CFlip;
				if(*(*pTokenArray)[i]=="ShotToItem")
					return new CShotToItem;
			}
			{//１引数関数
				CFunc* ret=NULL;
				int H1F=first+2;	//引数1の始まり
				if(H1F>=last) return NULL;
				int H1L=pTokenArray->GetCorrespondence(first+1,"(",")");//（に対応する）
				if(H1L<0) return NULL;

				if(*(*pTokenArray)[i]=="sin")
				{
					CFunc* pFunc = NewFunc(pBlock,pTokenArray,H1F,H1L);
					if(pFunc!=NULL)
						return new CSin(pFunc);
				}
				if(*(*pTokenArray)[i]=="cos")
				{
					CFunc* pFunc = NewFunc(pBlock,pTokenArray,H1F,H1L);
					if(pFunc!=NULL)
						return new CCos(pFunc);
				}
				if(*(*pTokenArray)[i]=="tan")
				{
					CFunc* pFunc = NewFunc(pBlock,pTokenArray,H1F,H1L);
					if(pFunc!=NULL)
						return new CTan(pFunc);
				}
				if(*(*pTokenArray)[i]=="atan")
				{
					CFunc* pFunc = NewFunc(pBlock,pTokenArray,H1F,H1L);
					if(pFunc!=NULL)
						return new CAtan(pFunc);
				}
				if(*(*pTokenArray)[i]=="sqrt")
				{
					CFunc* pFunc = NewFunc(pBlock,pTokenArray,H1F,H1L);
					if(pFunc!=NULL)
						return new CSqrt(pFunc);
				}
				if(*(*pTokenArray)[i]=="abs")
				{
					CFunc* pFunc = NewFunc(pBlock,pTokenArray,H1F,H1L);
					if(pFunc!=NULL)
						return new CAbs(pFunc);
				}
				if(*(*pTokenArray)[i]=="GetInputState")
				{
					CFunc* pFunc = NewFunc(pBlock,pTokenArray,H1F,H1L);
					if(pFunc!=NULL)
						return new CGetInputState(pFunc);
				}
				if(*(*pTokenArray)[i]=="GetInputEvent")
				{
					CFunc* pFunc = NewFunc(pBlock,pTokenArray,H1F,H1L);
					if(pFunc!=NULL)
						return new CGetInputEvent(pFunc);
				}
			}
			{//２引数関数
				int H1F=i+2;	//引数1の始まり
				int H1L=pTokenArray->SearchCommma(i+2,corKakko);	//引数1の最後
				if(H1L!=-1)
				{
					int H2F=H1L+1;
					int H2L=corKakko;
					//通常関数
					CFunc* pFunc1=NULL;
					CFunc* pFunc2=NULL;
					if(*(*pTokenArray)[i]=="Plus")
					{
						pFunc1 = NewFunc(pBlock,pTokenArray,H1F,H1L);
						pFunc2 = NewFunc(pBlock,pTokenArray,H2F,H2L);
						if(pFunc1 && pFunc2)
							return new CPlus(pFunc1,pFunc2);
					}
					else if(*(*pTokenArray)[i]=="Minus")
					{
						pFunc1 = NewFunc(pBlock,pTokenArray,H1F,H1L);
						pFunc2 = NewFunc(pBlock,pTokenArray,H2F,H2L);
						if(pFunc1 && pFunc2)
							return new CMinus(pFunc1,pFunc2);
					}
					else if(*(*pTokenArray)[i]=="atan2")
					{
						pFunc1 = NewFunc(pBlock,pTokenArray,H1F,H1L);
						pFunc2 = NewFunc(pBlock,pTokenArray,H2F,H2L);
						if(pFunc1 && pFunc2)
							return new CAtan2(pFunc1,pFunc2);
					}
					else if(*(*pTokenArray)[i]=="random")
					{
						pFunc1 = NewFunc(pBlock,pTokenArray,H1F,H1L);
						pFunc2 = NewFunc(pBlock,pTokenArray,H2F,H2L);
						if(pFunc1 && pFunc2)
							return new CRandom(pFunc1,pFunc2);
					}
					else if(*(*pTokenArray)[i]=="mod")
					{
						pFunc1 = NewFunc(pBlock,pTokenArray,H1F,H1L);
						pFunc2 = NewFunc(pBlock,pTokenArray,H2F,H2L);
						if(pFunc1 && pFunc2)
							return new CMod(pFunc1,pFunc2);
					}
					else if(*(*pTokenArray)[i]=="PlaySound")
					{
						pFunc1 = NewFunc(pBlock,pTokenArray,H1F,H1L);
						pFunc2 = NewFunc(pBlock,pTokenArray,H2F,H2L);
						if(pFunc1 && pFunc2)
							return new CPlaySound(pFunc1,pFunc2);
					}
					else if(*(*pTokenArray)[i]=="LoadImage")
					{
						pFunc1 = NewFunc(pBlock,pTokenArray,H1F,H1L);
						char* str=newstr((*pTokenArray)[H2L-1]->GetStr()+1,strchr((*pTokenArray)[H2L-1]->GetStr()+1,'\"'));
						if(pFunc1)
							return new CLoadImage(pFunc1,str);
					}
					else if(*(*pTokenArray)[i]=="LoadSound")
					{
						pFunc1 = NewFunc(pBlock,pTokenArray,H1F,H1L);
						char* str=newstr((*pTokenArray)[H2L-1]->GetStr()+1,strchr((*pTokenArray)[H2L-1]->GetStr()+1,'\"'));
						if(pFunc1)
							return new CLoadSound(pFunc1,str);
					}
					else if(*(*pTokenArray)[i]=="Tikei")
					{
						pFunc1 = NewFunc(pBlock,pTokenArray,H1F,H1L);
						pFunc2 = NewFunc(pBlock,pTokenArray,H2F,H2L);
						if(pFunc1 && pFunc2)
							return new CIsTikei(pFunc1,pFunc2);
					}
					SAFE_DELETE(pFunc1);
					SAFE_DELETE(pFunc2);

					if(*(*pTokenArray)[i]=="DrawImage")
					{
						int H2F=H1L+1;
						int H2L=pTokenArray->SearchCommma(H2F,corKakko);
						int H3F=H2L+1;
						int H3L=pTokenArray->SearchCommma(H3F,corKakko);
						int H4F=H3L+1;
						int H4L=corKakko;
						CFunc* pFunc1=NewFunc(pBlock,pTokenArray,H1F,H1L);
						CFunc* pFunc2=NewFunc(pBlock,pTokenArray,H2F,H2L);
						CFunc* pFunc3=NewFunc(pBlock,pTokenArray,H3F,H3L);
						CFunc* pFunc4=NewFunc(pBlock,pTokenArray,H4F,H4L);
						if(pFunc1&&pFunc2&&pFunc3&&pFunc4)
							return new CDraw(pFunc1,pFunc2,pFunc3,pFunc4);
						SAFE_DELETE(pFunc1);
						SAFE_DELETE(pFunc2);
						SAFE_DELETE(pFunc3);
						SAFE_DELETE(pFunc4);
					}
					/*else if(*(*pTokenArray)[i]=="DrawImageAdd")
					{
						int H2F=H1L+1;
						int H2L=pTokenArray->SearchCommma(H2F,corKakko);
						int H3F=H2L+1;
						int H3L=pTokenArray->SearchCommma(H3F,corKakko);
						int H4F=H3L+1;
						int H4L=corKakko;
						CFunc* pFunc1=NewFunc(pBlock,pTokenArray,H1F,H1L);
						CFunc* pFunc2=NewFunc(pBlock,pTokenArray,H2F,H2L);
						CFunc* pFunc3=NewFunc(pBlock,pTokenArray,H3F,H3L);
						CFunc* pFunc4=NewFunc(pBlock,pTokenArray,H4F,H4L);
						if(pFunc1&&pFunc2&&pFunc3&&pFunc4)
							return new CDrawAdd(pFunc1,pFunc2,pFunc3,pFunc4);
						SAFE_DELETE(pFunc1);
						SAFE_DELETE(pFunc2);
						SAFE_DELETE(pFunc3);
						SAFE_DELETE(pFunc4);
					}*/
					else if(*(*pTokenArray)[i]=="LoadImage2")
					{
						int H2F=H1L+1;
						int H2L=pTokenArray->SearchCommma(H2F,corKakko);
						int H3F=H2L+1;
						int H3L=pTokenArray->SearchCommma(H3F,corKakko);
						int H4F=H3L+1;
						int H4L=corKakko;
						CFunc* pFunc1 = NewFunc(pBlock,pTokenArray,H1F,H1L);
						char* str=newstr((*pTokenArray)[H2L-1]->GetStr()+1,strchr((*pTokenArray)[H2L-1]->GetStr()+1,'\"'));
						CFunc* pFunc3=NewFunc(pBlock,pTokenArray,H3F,H3L);
						CFunc* pFunc4=NewFunc(pBlock,pTokenArray,H4F,H4L);
						if(pFunc1&&str&&pFunc3&&pFunc4)
							return new CLoadImage2(pFunc1,str,pFunc3,pFunc4);
						SAFE_DELETE(pFunc1);
						SAFE_DELETE(pFunc3);
						SAFE_DELETE(pFunc4);
					}
				}
			}
			{//引数をvectorで渡す多引数関数
				int start=i+2;
				int final;
				vector<CFunc*> funcarray;//ＮＥＷした関数リスト
				while((final=pTokenArray->SearchCommma(start,corKakko))> 0)
				{
					CFunc* pFunc=NewFunc(pBlock,pTokenArray,start,final);
					if(pFunc==NULL)
					{
						for(int i=0;i<funcarray.size();++i)
							SAFE_DELETE(funcarray[i]);
						return NULL;
					}
					funcarray.push_back(pFunc);
					start=final+1;
				}
				funcarray.push_back(NewFunc(pBlock,pTokenArray,start,corKakko));
				//funcarray初期化完了
				if(*(*pTokenArray)[i]=="Shot")
					return new CShot(funcarray);
				else if(*(*pTokenArray)[i]=="DrawImage2")
					return new CDraw2(funcarray);
				else if(*(*pTokenArray)[i]=="Explode")
					return new CExplode(funcarray);
			}
			MBF_ERR("%s:　存在しない関数です。",(*pTokenArray)[i]->GetStr());
			return NULL;
		}
		else
		{
			MBF_ERR("%s:関数に正しいカッコがありません。",(*pTokenArray)[i]->GetStr());
			return NULL;
		}
	}
	else
	{
		MBF_ERR("%s%s:文が間違ってます。",(*pTokenArray)[i]->GetStr(),(*pTokenArray)[last]->GetStr());
		return NULL;
	}
}
//関数リスト実行
double CLogicalBlock::Get()
{
	double* pVar = NULL;
	if(m_varnum != 0)
		m_pVar = new double[m_varnum];
	list<CFunc*>::iterator itr;
	//全関数を実行
	for(itr = m_lstpFunc.begin(); itr != m_lstpFunc.end(); ++itr)
		if(*itr)
			(*itr)->Get();
	SAFE_DELETE_ARRAY(m_pVar);
	return 0;
}
//メンバ関数実行(引数：thisポインタ
void CMemFunc::Run(CClassInstance* pInstance)
{
	m_pInstance = pInstance;	//CMemVarから参照できるようにメンバに入れておく
	m_pBlock->Get();
}
//CMemVarをnewする
CVar* CMemFunc::NewVariable(const char* str)
{
	//デフォルトメンバ変数
	//（涼璃シュー追加部分）
	if(strcmp(str,"x")==0)
		return new CMemVar(this,-1);
	else if(strcmp(str,"y")==0)
		return new CMemVar(this,-2);
	else if(strcmp(str,"img")==0)
		return new CMemVar(this,-3);
	else if(strcmp(str,"hp")==0)
		return new CMemVar(this,-4);
	else if(strcmp(str,"anm")==0)
		return new CMemVar(this,-5);
	else if(strcmp(str,"px")==0)
		return new CMemVar(this,-10);
	else if(strcmp(str,"py")==0)
		return new CMemVar(this,-11);
	//変数名と照合
	for(int i=0;i<GetVarNum();++i)
	{
		if(strcmp(str,GetVarNameArray()[i]) == 0)
		{
			return new CMemVar(this,i);
		}
	}
	return NULL;
}
//メンバ関数読み込み
bool CMemFunc::Read(CClassDefine *pClassDef,CTokenArray* pTokenArray,int first)
{
	Outputf("%s\n",(*pTokenArray)[first]->GetStr());
	m_pClassDef = pClassDef;
	SAFE_DELETE(m_pBlock);
	m_pBlock = new CLogicalBlock;
	return m_pBlock->Read(this,pTokenArray,first);
}
//インスタンスのコンストラクタ
/*CClassInstance::CClassInstance(CClassDefine* pDefine)
{
	m_pDefine=pDefine;
	m_vararray=new double[pDefine->GetVarNum()];
	for(int i=0;i<pDefine->GetVarNum();++i)
		m_vararray[i]=0;
	if(pDefine->GetConstructer()!=NULL)
		pDefine->GetConstructer()->Run(this);
}*/
//涼璃シュー書き換え
CClassInstance::CClassInstance(CClassDefine* pDefine,CEnemy* pEnemy)
{
	m_pDefine=pDefine;
	m_pEnemy=pEnemy;
	m_vararray=new double[pDefine->GetVarNum()];
	for(int i=0;i<pDefine->GetVarNum();++i)
		m_vararray[i]=0;
	if(pDefine->GetConstructer()!=NULL)
		pDefine->GetConstructer()->Run(this);
}
//クラス定義の読み込み
bool CClassDefine::Read(CTokenArray* pTokenArray,int first,char* str)
{
	m_varnum=0;
	for(int i=0;i<VARNUM;++i)
		m_varname[i]=NULL;
	int last = pTokenArray->GetCorrespondence(first,"{","}");
	if(last==-1)
	{
		MB_ERR("{に対応する}がありません。");
		return false;
	}
	//メンバ変数
	for(i=first+1;i!=last;++i)
	{
		if(*(*pTokenArray)[i]=="double" || *(*pTokenArray)[i]=="var")//メンバ変数
		{
			++i;
			for(;i!=last&&(*pTokenArray)[i]!=NULL;++i)
			{
				if(IsVar((*pTokenArray)[i]->GetStr()))
				{
					m_varname[m_varnum]=_strdup(((*pTokenArray)[i])->GetStr());
					m_varnum++;
					i++;
					if(*(*pTokenArray)[i]==";")
						break;
					else if(*(*pTokenArray)[i]==",")
						continue;
					else
					{
						MBF_ERR("%s:変数宣言文内のトークンとして不適切です。",(*pTokenArray)[i]->GetStr());
						return false;;
					}
				}
				else
				{
					MBF_ERR("%s:変数名として不適切です。",(*pTokenArray)[i]->GetStr());
					return false;
				}
			}
		}
		else if(IsVar((*pTokenArray)[i]->GetStr()) && *(*pTokenArray)[i+1]=="(")//メンバ関数
		{
			int kakko=pTokenArray->GetCorrespondence(i+1,"(",")");
			if(kakko<0)
			{
				MBF_ERR("%s:メンバ関数に)がありません。",(*pTokenArray)[i]->GetStr());
				return false;
			}
			int tyukakko=pTokenArray->GetCorrespondence(kakko+1,"{","}");
			if(tyukakko<0)
			{
				MBF_ERR("%s:メンバ関数に{}がありません。",(*pTokenArray)[i]->GetStr());
				return false;
			}
			i=tyukakko;
		}
		else
		{
			MBF_ERR("%s:文として認識できません。",(*pTokenArray)[i]->GetStr());
			return false;
		}
	}
	int funcname=pTokenArray->Search(first,str);
	if(funcname<0 || funcname>=last)
		m_pConstructer = NULL;
	else
	{
		m_pConstructer = new CMemFunc;
		if(!m_pConstructer->Read(this,pTokenArray,funcname+3))
			SAFE_DELETE(m_pConstructer);
	}
	funcname=pTokenArray->Search(first,"main");
	if(funcname<0 || funcname>=last)
	{
		MBF_ERR("main関数がありません。");
		return false;
	}
	m_pMemFunc = new CMemFunc;
	return m_pMemFunc->Read(this,pTokenArray,funcname+3);
}
/*-----------------------------------------------------------------------------
	ここからNkScript関係
-----------------------------------------------------------------------------*/
CNkScript::CNkScript(const char* filename)
{
	int size=GetFileSize(filename);
	char *buffer= new char [size+1];
	ReadFile(buffer,size,filename);
	if(buffer==NULL)
	{
		MBF_ERR("%sがありません。",filename);
		throw;
	}
	CTokenArray tArray;
	tArray.CutToToken(buffer);
//	tArray.allprintf();
	for(int i=0;i<tArray.GetArray()->size()&&i>=0;++i)
	{
		if(*tArray[i]=="class")
		{
			m_pCDefine.push_back(new CLASSDEFINE(&tArray,i+1));
			i=tArray.GetCorrespondence(i+2,"{","}");
		}
		else if(IsVar(tArray[i]->GetStr()))
		{
			int kakko=tArray.GetCorrespondence(i+1,"(",")");
			if(kakko<0)
			{
				MBF_ERR("%s:関数に)がありません。",tArray[i]->GetStr());
				throw;
			}
			int tyukakko=tArray.GetCorrespondence(kakko+1,"{","}");
			if(tyukakko<0)
			{
				MBF_ERR("%s:関数に{}がありません。",tArray[i]->GetStr());
				throw;
			}
			m_pFunc.push_back(new FUNCDEFINE(&tArray,i));
			i=tyukakko;
		}
		else if(*tArray[i] != ";")
		{
			MBF_ERR("%s:文として認識できません。",tArray[i]->GetStr());
			throw;
		}
	}
	SAFE_DELETE_ARRAY(buffer);
}
CNkScript::~CNkScript()
{
	int i;
	for(i=0;i<m_pCDefine.size();++i)
		SAFE_DELETE(m_pCDefine[i]);
	for(i=0;i<m_pFunc.size();++i)
		SAFE_DELETE(m_pFunc[i]);
	g_image.DeleteAll();
	g_sound.DeleteAll();
}
int CNkScript::SearchClass(const char* name)
{
	for(int i=0;i<m_pCDefine.size();++i)
		if(strcmp(m_pCDefine[i]->m_name,name)==0)
			return i;
	return -1;
}
int CNkScript::SearchFunc(const char* name)
{
	for(int i=0;i<m_pFunc.size();++i)
		if(strcmp(m_pFunc[i]->m_name,name)==0)
			return i;
	return -1;
}

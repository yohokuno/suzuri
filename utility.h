/*-----------------------------------------------------------------------------

  utility.h
  涼璃シュー以外、またはゲーム以外にも使えるようなユーティリティ系をまとめる

-----------------------------------------------------------------------------*/
#ifndef AFX_UTILITY_H
#define AFX_UTILITY_H

//補助マクロ
#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete [] (p);	  (p)=NULL; } }
/*-----------------------------------------------------------------------------
//ベクトル構造体
-----------------------------------------------------------------------------*/
struct VECT
{
	//実体はdouble型の２次元ベクトル
	double x;
	double y;
	//コンストラクタ
	VECT(){};
	VECT(const double x1,const double y1){x=x1;y=y1;};
	VECT(const VECT& vect){x=vect.x;y=vect.y;};
	~VECT(){};
	//演算子
	VECT operator=(const VECT& vect);
	VECT operator+(const VECT& vect)const;
	VECT operator-(const VECT& vect)const;
	VECT operator/(const double& d)const;
	VECT operator*(const double& d)const;
	VECT operator+=(const VECT& vect);
	VECT operator-=(const VECT& vect);
	double operator*(const VECT& vect)const;//内積
	//メンバ関数
	double Distance(const VECT& vect)const;
	double Norn()const;
	void Goto(const VECT& vect,const double velocity);
	double Cos(const VECT& vect)const;
};
VECT operator*(double d,const VECT& vect);

/*-----------------------------------------------------------------------------
//補助系関数
-----------------------------------------------------------------------------*/
inline int Rand(int from,int to)
{
	return (to-from)*rand()/(RAND_MAX+1) + from;
}
char *prfGetString(const char *file, const char *section, const char *key, char *buffer);
int prfGetInt(const char *file, const char *section, const char *key);
double prfGetDouble(const char *file, const char *section, const char *key);
void prfSetString(const char *file, const char *section, const char *key, const char *buffer);
void prfSetInt(const char *file, const char *section, const char *key, int value);
void OutputDebugInt(const int num);
void OutputDebugInt(const int num,const char* cap);
char *strdiv(char *buffer,char *&string,int c,const char *def="DEFAULT");
int strdiv2(char *&string,int c);
int sign(int num);
int round(double n);
int mod(int a,int b);
void MessageInt(const int num,char *caption = "");
void ReplaceFromProfile(char* buffer,const char *file, const char *section);
void Outputf(char* str,...);

/*-----------------------------------------------------------------------------

  CProfile
  プロファイル用文字列クラス
  文字列を','や'/'などで区切ることができる

-----------------------------------------------------------------------------*/
#define BUFFER_NUM 256
class CProfile
{
public:
	//コンストラクタ
	CProfile();
	CProfile(const CProfile& prf);
	CProfile(const char* str);
	~CProfile(){};
	//演算子
	CProfile operator =(const CProfile& prf);
	operator char* () const {return (char*)m_buffer;};
	operator const char*() const {return m_buffer;};
	//メンバ
	char* GetString(char* buffer,int c);				//文字cを探して区切る
	char* GetString(char* buffer,int c1,int c2);		//c2はストッパー文字
	int GetInt(int c);
	int GetInt(int c1,int c2);
	char* Get(){return m_pBuffer;};
protected:
	//実体はバッファとそれへのポインタのセット
	char m_buffer[BUFFER_NUM];
	char* m_pBuffer;
};
/*-----------------------------------------------------------------------------
	
	オブジェクト配列テンプレート
	Ｔ型オブジェクト配列の動的作成、およびデータの追加と削除が簡単にできる。
	動的作成といっても初期化に使う程度。
	データの追加はnewしたアドレスを受け取って保存。
	[]演算子により普通の配列のように使える。

-----------------------------------------------------------------------------*/
template<class T>
class CObjArray
{
public:
	CObjArray();
	CObjArray(int size);
	~CObjArray();
	int Add(T* pObj);
	int StepFrame();
	void DeleteAll();
	bool IsAllNull();
public:
	int GetSize(){return m_size;};
	T*& operator[](int i){return m_ppObj[i];};	//T*配列と同じように扱える
protected:
	T** m_ppObj;
	int m_size;
	int m_index;	//NULLになってる第一候補
};
/*-----------------------------------------------------------------------------
	ここからCObjArray関係の実装
-----------------------------------------------------------------------------*/
template <class T>
CObjArray<T>::CObjArray()
{
	m_ppObj = NULL;
	m_size = 0;
	m_index = 0;
}
template <class T>
CObjArray<T>::CObjArray(int size)
{
	m_ppObj = new T* [size];
	for(int i=0;i<size;++i)
		m_ppObj[i] = NULL;
	m_size = size;
	m_index = 0;
}
template <class T>
CObjArray<T>::~CObjArray()
{
	DeleteAll();
	SAFE_DELETE_ARRAY(m_ppObj);
}
template <class T>
int CObjArray<T>::Add(T* pObj)
{
	if(m_ppObj == NULL) return 0;
	int old_index = m_index;
	for(;;)
	{
		if(m_ppObj[m_index] == NULL)
		{
			//渡されたポインタを保存
			m_ppObj[m_index] = pObj;
			break;
		}
		else
		{
			m_index++;
			if(m_index == m_size)
				m_index = 0;
			if(m_index == old_index)
			{
				delete pObj;
				return 0;
			}
		}
	}
	return 1;
}
template <class T>
int CObjArray<T>::StepFrame()
{
	if(m_ppObj == NULL) return 0;
	for(int i=m_size-1;i>=0;--i)
	{
		if(m_ppObj[i] != NULL)
		{
			//StepFrameを実行
			if(!m_ppObj[i]->StepFrame())
			{
				//StepFrameが0を返したら、ＤＥＬＥＴＥする
				SAFE_DELETE(m_ppObj[i]);
			}
		}
	}
	return 1;
}
template <class T>
void CObjArray<T>::DeleteAll()
{
	for(int i=0;i<m_size;++i)
	{
		SAFE_DELETE(m_ppObj[i]);
	}
}
template <class T>
bool CObjArray<T>::IsAllNull()
{
	for(int i=0;i<m_size;++i)
		if(m_ppObj[i])
			return 0;
	return 1;
}
#endif
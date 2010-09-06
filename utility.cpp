/*-----------------------------------------------------------------------------

  utility.cpp

-----------------------------------------------------------------------------*/
#include "stdapp.h"
#include "utility.h"
/*-----------------------------------------------------------------------------

	VECT構造体関係

-----------------------------------------------------------------------------*/
VECT VECT::operator=(const VECT& vect)
{
	x=vect.x;
	y=vect.y;
	return *this;
}

VECT VECT::operator+(const VECT& vect)const
{
	VECT ret;
	ret.x = x+vect.x;
	ret.y = y+vect.y;
	return ret;
}
VECT VECT::operator-(const VECT& vect)const
{
	VECT ret;
	ret.x = x-vect.x;
	ret.y = y-vect.y;
	return ret;
}
VECT VECT::operator*(const double& d)const
{
	VECT ret;
	ret.x = x*d;
	ret.y = y*d;
	return ret;
}
VECT operator*(double d,const VECT& vect)
{
	VECT ret;
	ret.x = vect.x*d;
	ret.y = vect.y*d;
	return ret;
}
VECT VECT::operator/(const double& d)const
{
	VECT ret;
	ret.x = x/d;
	ret.y = y/d;
	return ret;
}
VECT VECT::operator+=(const VECT& vect)
{
	x += vect.x;
	y += vect.y;
	return *this;
}
VECT VECT::operator-=(const VECT& vect)
{
	x -= vect.x;
	y -= vect.y;
	return *this;
}
double VECT::Distance(const VECT& vect)const
{
	return sqrt((x-vect.x)*(x-vect.x) + (y-vect.y)*(y-vect.y));
}
double VECT::Norn()const
{
	return sqrt(x*x+y*y);
}
double VECT::operator *(const VECT& vect)const
{
	return x*vect.x + y*vect.y;
}
void VECT::Goto(const VECT& vect,const double velocity)
{
	double dst = Distance(vect);
	if(dst < velocity)
	{
		*this = vect;
	}
	else
	{
		*this += (vect - *this)/dst * velocity;
	}
}
double VECT::Cos(const VECT& vect)const
{
	return (*this) * vect / (Norn() * vect.Norn());
}
/*-----------------------------------------------------------------------------
	CProfileの実装
-----------------------------------------------------------------------------*/
CProfile::CProfile()
{
	strcpy(m_buffer,"");
	m_pBuffer = m_buffer;
}
CProfile::CProfile(const CProfile& prf)
{
	strcpy(m_buffer,(const char*)prf);
	m_pBuffer = m_buffer;
}
CProfile::CProfile(const char* str)
{
	strcpy(m_buffer,str);
	m_pBuffer = m_buffer;
}
CProfile CProfile::operator =(const CProfile& prf)
{
	strcpy(m_buffer,(const char*)prf);
	m_pBuffer = m_buffer;
	return *this;
}
//文字列をcで区切って返す
char* CProfile::GetString(char* buffer,int c)
{
	char *temp;
	char def[] = "DEFAULT";

	if((temp = strchr(m_pBuffer, c))==NULL)
	{
		//cが見つからない場合
		temp = strchr(m_pBuffer,NULL);
		strncpy(buffer,m_pBuffer,temp-m_pBuffer);
		m_pBuffer = m_buffer+strlen(m_buffer);//ポインタを文字列末にしておく
		return buffer;
	}
	else if(temp-m_pBuffer == 0)//ちょうどcをさしてるとき
	{
		strcpy( buffer,def);
		m_pBuffer++;
		return buffer;
	}
	else
	{
		//それ以外(通常時）
		strncpy(buffer,m_pBuffer,temp-m_pBuffer);
		buffer[temp-m_pBuffer]=0;
		m_pBuffer = temp;
		m_pBuffer++;
		return buffer;
	}
	return buffer;
}
//c2はストッパー
char* CProfile::GetString(char* buffer,int c1,int c2)
{
	char *temp;
	char def[] = "DEFAULT";

	if((temp = strchr(m_pBuffer, c1))==NULL)
	{
		//c1が見つからない場合
		temp = strchr(m_pBuffer,NULL);
		strncpy(buffer,m_pBuffer,temp-m_pBuffer);
		m_pBuffer = m_buffer+strlen(m_buffer);//ポインタを文字列末にしておく
		return buffer;
	}
	else if(temp-m_pBuffer == 0)//ちょうどc1をさしてるとき
	{
		strcpy( buffer,def);
		m_pBuffer++;
		return buffer;
	}
	else
	{
		char *temp2;
		temp2 = strchr(m_pBuffer, c2);
		if(temp2 == NULL)
		{
			//c2が見つからない場合
			temp = strchr(m_pBuffer,NULL);
			strncpy(buffer,m_pBuffer,temp-m_pBuffer);
			m_pBuffer = m_buffer+strlen(m_buffer);//ポインタを文字列末にしておく
			return buffer;
		}
		else if(temp2-m_pBuffer == 0)//ちょうどc2をさしてるとき
		{
			strcpy( buffer,def);
			return buffer;
		}
		else if(temp2 < temp)//c2のほうがc1より先にあるとき
		{
			strncpy(buffer,m_pBuffer,temp2-m_pBuffer);
			buffer[temp2-m_pBuffer]=NULL;
			m_pBuffer = temp2;//ポインタをc2にあわせておく
			return buffer;
		}
		else//ふつーにc1が見つかった場合
		{
			strncpy(buffer,m_pBuffer,temp-m_pBuffer);
			buffer[temp-m_pBuffer]=0;
			m_pBuffer = temp;
			m_pBuffer++;
			return buffer;
		}
	}
	return buffer;
}
//int返し
int CProfile::GetInt(int c)
{
	char buffer[BUFFER_NUM];
	if(strcmp(GetString(buffer,c),"DEFAULT") == 0)
		return 0;
	else
		return atoi(buffer);
}
//int返し2
int CProfile::GetInt(int c1,int c2)
{
	char buffer[BUFFER_NUM];
	if(strcmp(GetString(buffer,c1,c2),"DEFAULT") == 0)
		return 0;
	else
		return atoi(buffer);
}
/*-----------------------------------------------------------------------------
//補助系関数
-----------------------------------------------------------------------------*/

//符号を返す(正なら1、負なら-1、0なら0)
int sign(int num)
{
	if(num == 0)
		return 0;
	else
		return num/abs(num);
}
//四捨五入
int round(double n)
{
	if(n-0.5<(int)n)
		return (int)n;
	else
		return (int)n+1;
}
//剰余（ＭＯＤ
int mod(int a,int b)
{
	if(a>=0)
		return a%b;
	else
		return a%b +b;
}
//	文字列型プロファイル読み込み
char *prfGetString(const char *file, const char *section, const char *key, char *buffer)
{
	char path[MAX_PATH+sizeof(file)];
	char *temp;
	GetModuleFileName(NULL, path, sizeof(path));
	temp = strrchr(path, '\\');
	if(temp){
		strcpy(temp, "\\");
		strcat(path, file);
	}
	else{
		strcat(path, "\\");
		strcat(path, file);
	}
	GetPrivateProfileString(section, key, "DEFAULT", buffer, 128, path);
	return buffer;
}
//	小数型プロファイル読み込み
double prfGetDouble(const char *file, const char *section, const char *key)
{
	char buffer[256];
	char path[MAX_PATH+sizeof(file)];
	char *temp;
	GetModuleFileName(NULL, path, sizeof(path));
	temp = strrchr(path, '\\');
	if(temp){
		strcpy(temp, "\\");
		strcat(path, file);
	}
	else{
		strcat(path, "\\");
		strcat(path, file);
	}
	GetPrivateProfileString(section, key, "DEFAULT", buffer, 128, path);
	return atof(buffer);
}
//	文字列型プロファイル書き込み
void prfSetString(const char *file, const char *section, const char *key, const char *buffer)
{
	char path[MAX_PATH+sizeof(file)];
	char *temp;
	GetModuleFileName(NULL, path, sizeof(path));
	temp = strrchr(path, '\\');
	if(temp){
		strcpy(temp, "\\");
		strcat(path, file);
	}
	else{
		strcat(path, "\\");
		strcat(path, file);
	}
	WritePrivateProfileString(section, key, buffer, path);
}
void prfSetInt(const char *file, const char *section, const char *key, int value)
{
	char path[MAX_PATH+sizeof(file)];
	char *temp;
	GetModuleFileName(NULL, path, sizeof(path));
	temp = strrchr(path, '\\');
	if(temp){
		strcpy(temp, "\\");
		strcat(path, file);
	}
	else{
		strcat(path, "\\");
		strcat(path, file);
	}
	char buffer[256];
	itoa(value,buffer,10);
	WritePrivateProfileString(section, key, buffer, path);
}
//	整数型プロファイル読み込み
int prfGetInt(const char *file, const char *section, const char *key)
{
	char path[MAX_PATH+sizeof(file)];
	char *temp;

	GetModuleFileName(NULL, path, sizeof(path));
	temp = strrchr(path, '\\');
	if(temp){
		strcpy(temp, "\\");
		strcat(path, file);
	}
	else{
		strcat(path, "\\");
		strcat(path, file);
	}	
	return GetPrivateProfileInt(section, key, 0, path);
}
//プロファイルのセクションから文字列を置き換える
void ReplaceFromProfile(char* buffer,const char *file, const char *section)
{
	char path[MAX_PATH+sizeof(file)];
	char *temp;
	GetModuleFileName(NULL, path, sizeof(path));
	temp = strrchr(path, '\\');
	if(temp){
		strcpy(temp, "\\");
		strcat(path, file);
	}
	else{
		strcat(path, "\\");
		strcat(path, file);
	}
	char str[256*8];
	GetPrivateProfileSection(section,str,256*8,file);
	char* p1=str;
	while(*p1!='\0')
	{
		p1=strchr(p1,'\0');
		*p1 = '\n';
		p1++;
	}
	char* pStr = strtok(str,"=\0");
	while(pStr!=NULL)
	{
		char* p = strstr(buffer,pStr);
		pStr = strtok(NULL,"=\n");
		if(p!=NULL)
		{ 
			strcpy(p,pStr);
		}
		pStr = strtok(NULL,"=\n");
	}
}
/*
	文字列を「,」などで区切って整数値を返し、文字列自身は「,」の後ろのみ残る
	ラムダの文字列同士の引き算(buffer=string-c)に当たる。
*/
char *strdiv(char *buffer,char *&string,int c,const char *def)
{
	char *temp;
	
	if((temp = strchr(string, c))==NULL)
	{
		if((temp = strchr(string,NULL))==NULL)
		{//行末にNULLが無い場合（まさかの時の例外処理
			strcpy( buffer,def);
			return buffer;
		}
		//cが見つからない場合
		strcpy( buffer,def);
		return buffer;
	}
	if(temp-string == 0)//カンマ連続時および文字列終端以降
		strcpy( buffer,"");
	else{//それ以外
		strncpy(buffer,string,temp-string);
		buffer[temp-string]=0;
	}
	string = temp;
	string++;
	return buffer;
}
//切り出し部分のintを返す。
int strdiv2(char *&string,int c)
{
	char buffer[256];
	strdiv(buffer,string,c,"DEFAULT");
	if(strcmp(buffer,"DEFAULT") == 0)
		while(string[0] != NULL)
			string++;
	return atoi(buffer);
}
/*-----------------------------------------------------------------------------
//デバッグ補助関数
-----------------------------------------------------------------------------*/
//	Int型を表示するMessageBox(デバッグ用)
void MessageInt(const int num,char *caption)
{
	char str[32];
	sprintf( str, "%d", num );
	MessageBox(NULL,str,caption,MB_OK);
}
//	Int型を表示するMessageBox(デバッグ用)
void OutputDebugInt(const int num)
{
	char str[32];
	sprintf( str, "%d", num );
	OutputDebugString(str);
}
void OutputDebugInt(const int num,const char* cap)
{
	char str[128];
	sprintf( str, "%s:%d\n", cap, num );
	OutputDebugString(str);
}
//MessageBoxをprintfみたく拡張
int msgf(char *text,char *caption,unsigned int type,...)
{
	char* buffer=new char[strlen(text)+256];
	va_list args;
	va_start(args, type);
	vsprintf(buffer,text,args);
	va_end(args);
	int ret = MessageBox(NULL,buffer,caption,type);
	SAFE_DELETE_ARRAY(buffer);
	return ret;
}
//MessageBoxをprintfみたく拡張
int MBF_ERR(char* str,...)
{
	char* buffer=new char[strlen(str)+256];
	va_list args;
	va_start(args, str);
	vsprintf(buffer,str,args);
	va_end(args);
	int ret = MessageBox(NULL,buffer,"スクリプトエラー",MB_OK|MB_ICONSTOP);
	SAFE_DELETE_ARRAY(buffer);
	return ret;
};
//OuptputDebugStringをpfintf拡張
void Outputf(char* str,...)
{
	char* buffer=new char[strlen(str)+256];\
	va_list args;
	va_start(args, str);
	vsprintf(buffer,str,args);
	va_end(args);
	OutputDebugString(buffer);
	SAFE_DELETE_ARRAY(buffer);
}
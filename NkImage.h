/*-----------------------------------------------------------------------------

	NkImage.h

-----------------------------------------------------------------------------*/

#ifndef AFX_NKIMAGE_H
#define AFX_NKIMAGE_H

/*-----------------------------------------------------------------------------

	CNkImage
	画像クラス
	１つの画像につき１つのインスタンスを持つクラス
	アルファブレンドなどの高度な描画に加え、
	ＣＮｋＬｉｂと連携して描画ソート機能をもつ描画キューを扱える。

-----------------------------------------------------------------------------*/
class CNkImage  
{
public:
	CNkImage();
	CNkImage(char* lpszFileName,int width=0,int height=0);
	virtual ~CNkImage();
	void Release();
	int Load(char* lpszFileName,int width=0,int height=0);
	int Fill( DWORD dwColor);	//色は_RGBマクロで作る。
	//ソートつき描画（通常はこれを使用）
	int DrawLayer(int iDestX,int iDestY,int layer,RECT* prcSrc,bool trans);
	int DrawLayerStretch(LPRECT prcDest,LPRECT prcSrc,int layer,bool trans);
		//ソートなし
		int Draw(int iDestX,int iDestY,RECT *prcSrc,bool trans){return g_pNkLib->BltFast(iDestX,iDestY,m_pdds,prcSrc,trans);}
		int DrawEffect(int iDestX,int iDestY,RECT *prcSrc,CCalculate* pCal){return g_pNkLib->BltEffect(iDestX,iDestY,m_pdds,prcSrc,pCal);}
		int DrawStretch(LPRECT prcDest,LPRECT prcSrc,bool trans){return g_pNkLib->BltStretch(prcDest,m_pdds,prcSrc,trans);}
		//互換用
		int Draw2(int sprLevel,int iDestX,int iDestY,int flameX=0,int flameY=0,bool trans=true)
		{
			flameY=flameY +  flameX / (m_bmpWidth / m_width);flameX =flameX % (m_bmpWidth / m_width);
			RECT rctSrc={flameX*GetWidth(),flameY*GetHeight(),(flameX+1)*GetWidth(),(flameY+1)*GetHeight()};
			return DrawLayer(iDestX,iDestY,sprLevel,&rctSrc,trans);
		}
		//互換用
		int Draw2(int sprLevel,VECT pos,int flameX=0,int flameY=0){ return Draw2(sprLevel,pos.x,pos.y,flameX,flameY);};
		//内部使用
		int LoadBMP(char* lpszFileName);
		int LoadPNG(char* lpszFileName);
		int DrawBitmap( HBITMAP hBMP);
		int SetColorKey( DWORD dwColorKey );
		DWORD ConvertGDIColor( COLORREF dwGDIColor );
public:
	int Reload(){if(m_szFileName[0]!=NULL)return LoadPNG(m_szFileName);return 1;};
	int GetWidth(){return m_width;};
	int GetHeight(){return m_height;};
	void SetWH(int w,int h){m_width=w;m_height=h;}
	VECT GetVect(){return VECT(m_width,m_height);}
	LPDIRECTDRAWSURFACE7 GetSurface(){return m_pdds;}
	//アニメーションフレームを使った書き換え
	int DrawLayer(VECT pos,int layer,RECT* prcSrc,bool trans=true)
	{ return DrawLayer(pos.x,pos.y,layer,prcSrc,trans);}
	int DrawLayer(VECT pos,int layer,int flameX=0,int flameY=0,bool trans=true)
	{
		int flame =flameX + flameY*(m_bmpWidth / m_width);
		flame = mod(flame,(m_bmpWidth / m_width)*(m_bmpHeight / m_height));
		flameX = flame % (m_bmpWidth / m_width);
		flameY = flame / (m_bmpWidth / m_width);
		RECT rcSrc={flameX*GetWidth(),flameY*GetHeight(),(flameX+1)*GetWidth(),(flameY+1)*GetHeight()};
		return DrawLayer(pos,layer,&rcSrc,trans);
	}
	int DrawLayer(int iDestX,int iDestY,int layer,int flameX,int flameY,bool trans)
	{return DrawLayer(VECT(iDestX,iDestY),layer,flameX,flameY,trans);}
	int DrawLayerStretch(int iDestX,int iDestY,double a,int layer,LPRECT prcSrc,bool trans=true)
	{
		RECT rcDest={
			iDestX-(prcSrc->right-prcSrc->left)*a/2,
			iDestY-(prcSrc->bottom-prcSrc->top)*a/2,
			iDestX+(prcSrc->right-prcSrc->left)*a/2,
			iDestY+(prcSrc->bottom-prcSrc->top)*a/2};
		return DrawLayerStretch(&rcDest,prcSrc,layer,trans);
	}
	int DrawLayerStretch(VECT pos,double a,int layer,LPRECT prcSrc,bool trans=true)
	{return DrawLayerStretch(pos.x,pos.y,a,layer,prcSrc,trans);}
	int DrawLayerStretch(VECT pos,double a,int layer,int flameX=0,int flameY=0,bool trans=true)
	{
		RECT rcSrc={flameX*GetWidth(),flameY*GetHeight(),(flameX+1)*GetWidth(),(flameY+1)*GetHeight()};
		return DrawLayerStretch(pos,a,layer,&rcSrc,trans);
	}
	int DrawLayerStretch(int iDestX,int iDestY,double a,int layer,int flameX=0,int flameY=0,bool trans=true)
	{return DrawLayerStretch(VECT(iDestX,iDestY),a,layer,flameX,flameY,trans);}
protected:
	LPDIRECTDRAWSURFACE7 m_pdds;
	int		m_bmpWidth;				//ＢＭＰの幅
	int		m_bmpHeight;			//ＢＭＰの高さ
	int		m_width;				//幅(アニメーション用）
	int		m_height;				//高さ(アニメーション用）
	char	m_szFileName[MAX_PATH]; //BMPファイル名
public:
	/*-----------------------------------------------------------------------------
		エフェクト描画（ソートつき）
		テンプレートメンバ関数の実装(実装をクラス宣言に描かねばならないとは・・
	-----------------------------------------------------------------------------*/
	template<typename T> //CCalculateをテンプレートにする
	int DrawLayerEffect(T cal,int iDestX,int iDestY,int layer,RECT *prcSrc)
	{
		//画面外
		if(iDestX>=SCREEN_WIDTH || iDestY>=SCREEN_HEIGHT || iDestX+m_width<=0 || iDestY+m_height<=0)
			return 0;
		DRAWINFO dInfo;
		dInfo.spr = layer;
		RECT rctSrc;
		if(prcSrc)
		{
			rctSrc=*prcSrc;
		}
		else//NULLのとき
		{
			rctSrc.left=0;
			rctSrc.top=0;
			rctSrc.right=m_width;
			rctSrc.bottom=m_height;
		}
		dInfo.pDrawQue = new CDrawQueEffect<T>(this,iDestX,iDestY,rctSrc,cal);
		g_pNkLib->AddDrawList(dInfo);
		return 1;
	}
	template<typename T>
	int DrawLayerEffect( T cal,int iDestX,int iDestY,int layer=100, int flameX=0,int flameY=0)
	{
		RECT rctSrc={flameX*GetWidth(),flameY*GetHeight(),(flameX+1)*GetWidth(),(flameY+1)*GetHeight()};
		return DrawLayerEffect(cal,iDestX,iDestY,layer,&rctSrc);
	}
	template<typename T>
	int DrawLayerEffect(T cal,VECT pos,int layer=100, int flameX=0,int flameY=0)
	{return DrawLayerEffect(cal,pos.x,pos.y,layer,flameX,flameY);}
};
/*-----------------------------------------------------------------------------
  CDrawQue
  描画キューのインターフェース。
  これさえあればバックバッファに描画可能なすぐれもの。
-----------------------------------------------------------------------------*/
class CDrawQue
{
public:
	virtual ~CDrawQue(){};
	virtual int Draw()=0;
};
/*-----------------------------------------------------------------------------
	通常描画の描画キュー
-----------------------------------------------------------------------------*/
class CDrawNormal : public CDrawQue
{
public:
	CDrawNormal(){}
	CDrawNormal(CNkImage* pimg,int iDestX,int iDestY,RECT rctSrc,bool trans){m_pimg=pimg;m_iDestX=iDestX;m_iDestY=iDestY;m_rctSrc=rctSrc;m_trans=trans;}
	virtual int Draw(){return m_pimg->Draw(m_iDestX,m_iDestY,&m_rctSrc,m_trans);}
private:
	CNkImage* m_pimg;
	int m_iDestX;
	int m_iDestY;
	RECT m_rctSrc;
	bool m_trans;
};
/*-----------------------------------------------------------------------------
	エフェクト描画の描画キュー（CCaluculateのクラスによってテンプレートにする）
-----------------------------------------------------------------------------*/
template<typename T>
class CDrawQueEffect : public CDrawQue
{
public:
	CDrawQueEffect(){}
	CDrawQueEffect(CNkImage* pimg,int iDestX,int iDestY,RECT rctSrc,T cal)
	{
		m_pimg=pimg;m_iDestX=iDestX;m_iDestY=iDestY;m_rctSrc=rctSrc;m_cal=cal;
	}
	virtual int Draw(){return m_pimg->DrawEffect(m_iDestX,m_iDestY,&m_rctSrc,&m_cal);}
private:
	CNkImage* m_pimg;
	int m_iDestX;
	int m_iDestY;
	RECT m_rctSrc;
	T m_cal;
};
/*-----------------------------------------------------------------------------
	拡大縮小描画の描画キュー
-----------------------------------------------------------------------------*/
class CDrawQueStretch : public CDrawQue
{
public:
	CDrawQueStretch(){}
	CDrawQueStretch(CNkImage* pimg,RECT rctDest,RECT rctSrc,bool trans){m_pimg=pimg;m_rctDest=rctDest;m_rctSrc=rctSrc;m_trans=trans;}
	virtual int Draw(){return m_pimg->DrawStretch(&m_rctDest,&m_rctSrc,m_trans);}
private:
	CNkImage* m_pimg;
	RECT m_rctDest;
	RECT m_rctSrc;
	bool m_trans;
};
/*-----------------------------------------------------------------------------
	テキスト描画の描画キュー
-----------------------------------------------------------------------------*/
class CDrawQueText : public CDrawQue
{
public:
	CDrawQueText(){m_text=NULL;}
	CDrawQueText(char* text,int x,int y,int size,int weight,DWORD dwFlag,COLORREF color)
	{m_text=_strdup(text);m_x=x;m_y=y;m_size=size;m_weight=weight;m_dwFlag=dwFlag;m_color=color;}
	~CDrawQueText(){SAFE_DELETE_ARRAY(m_text);}
	virtual int Draw(){return g_pNkLib->TextOutBase(m_text,m_x,m_y,m_size,m_weight,m_dwFlag,m_color);}
private:
	char* m_text;
	int m_x,m_y,m_size,m_weight;
	DWORD m_dwFlag;
	COLORREF m_color;
};
/*-----------------------------------------------------------------------------
	円描画の描画キュー
-----------------------------------------------------------------------------*/
class CDrawQueCircle : public CDrawQue
{
public:
	CDrawQueCircle(){}
	CDrawQueCircle(int width,COLORREF color,int left,int top,int right,int bottom)
	{m_width=width;m_color=color;m_left=left;m_top=top;m_right=right;m_bottom=bottom;}
	virtual int Draw(){return g_pNkLib->DrawCircle(m_width,m_color,m_left,m_top,m_right,m_bottom);}
private:
	int m_width;
	COLORREF m_color;
	int m_left,m_top,m_right,m_bottom;
	int layer;
};
/*=============================================================================
	以下、演算関数となります。
	CDrawQueインターフェースクラスを継承してます。
	２ピクセル３２ビットを同時計算して高速化を図ってます。
=============================================================================*/

// ブレンディング用ビットマスク
#define	_mask01	0x0000ffff	// H off mask
#define	_mask10	0xffff0000	// L off mask
#define	_mask1	0x3def3def	// 0011110111101111 rgb  mask 1
#define	_mask2	0x42104210	// 0100001000010000 rgb  mask 2
#define	_mask3	0xf79ef79e	// 1111011110011110 half mask
#define	_mask4	0xffc0ffc0	// 1111111111000000 565  mask 1
#define	_mask5	0x001f001f	// 0000000000011111 565  mask 2
#define	_mask6	0x7fff7fff	// 0111111111111111 sub  mask
#define COLORKEY1 0xf81ff81f
#define COLORKEY2 0xf81f0000
#define COLORKEY3 0x0000f81f
/*-----------------------------------------------------------------------------
	ピクセル演算のインターフェース
-----------------------------------------------------------------------------*/
class CCalculate
{
public:
	virtual DWORD Get(DWORD dest,DWORD src)=0;
};
/*-----------------------------------------------------------------------------
	テスト用の単純転送
-----------------------------------------------------------------------------*/
class CCalEqual : public CCalculate
{
public:
	virtual DWORD Get(DWORD dest,DWORD src){return src;};
};
/*-----------------------------------------------------------------------------
	NOT
-----------------------------------------------------------------------------*/
class CCalNot : public CCalculate
{
public:
	virtual DWORD Get(DWORD dest,DWORD src){return ~src;};
};
/*-----------------------------------------------------------------------------
	XOR
-----------------------------------------------------------------------------*/
class CCalXOr : public CCalculate
{
public:
	virtual DWORD Get(DWORD dest,DWORD src){return dest^src;};
};
/*-----------------------------------------------------------------------------
	AND
-----------------------------------------------------------------------------*/
class CCalAnd : public CCalculate
{
public:
	virtual DWORD Get(DWORD dest,DWORD src){return dest&src;};
};
/*-----------------------------------------------------------------------------
	Half
-----------------------------------------------------------------------------*/
class CCalHalf : public CCalculate
{
public:
	virtual DWORD Get(DWORD dest,DWORD src){return (dest & src) + (((dest ^ src) & _mask3) >> 1);};
};
/*-----------------------------------------------------------------------------
	TransHalf
-----------------------------------------------------------------------------*/
class CCalTransHalf : public CCalculate
{
public:
	virtual DWORD Get(DWORD dest,DWORD src)
	{
		if(src==COLORKEY1)
			return dest;
		if ((src & _mask10) == COLORKEY2) {
			return (dest & (src | _mask10)) +
				(((dest ^ (src | (dest & _mask10))) & _mask3) >> 1);
		}
		if ((src & _mask01) == COLORKEY3) {
			return (dest & (src | _mask01)) +
				(((dest ^ (src | (dest & _mask01))) & _mask3) >> 1);
		}
		return (dest & src) + (((dest ^ src) & _mask3) >> 1);
	};
};
/*-----------------------------------------------------------------------------
	加算合成
-----------------------------------------------------------------------------*/
class CCalAdd : public CCalculate
{
public:
	virtual DWORD Get(DWORD dest,DWORD src)
	{
		DWORD	c,d;

		dest = to555 (dest);
		src = to555 (src);

		c = ((dest & _mask1) + (src & _mask1)) ^ ((dest ^ src) & _mask2);
		d = (dest + src) - c;
		c = c | (d - (d >> 5));

		return to565 (c);
	};
};
/*-----------------------------------------------------------------------------
	減算合成
-----------------------------------------------------------------------------*/
class CCalSub : public CCalculate
{
public:
	virtual DWORD Get(DWORD dest,DWORD src)
	{
		DWORD	c,d;

		dest = to555 (dest);
		src = to555 (src);

		dest ^= _mask6;
		src ^= _mask6;

		c = ((dest & _mask1) + (src & _mask1)) ^ ((dest ^ src) & _mask2);
		d = dest + src - c;
		c = (c | (d - (d >> 5))) ^ _mask6;

		return to565 (c);
	};
};
/*-----------------------------------------------------------------------------
	透過なしアルファブレンド
	引数:０～３２（大きいほど透明）
-----------------------------------------------------------------------------*/
#define ALPHA_MAX 32
class CCalAlpha : public CCalculate
{
public:
	CCalAlpha(){};
	CCalAlpha(DWORD alpha){if(alpha>ALPHA_MAX)alpha=ALPHA_MAX;m_alpha=ALPHA_MAX-alpha;m_beta=alpha;}
	virtual DWORD Get(DWORD dest,DWORD src);	//実装はNkImage.cppに
private:
	DWORD m_alpha;
	DWORD m_beta;
};
/*-----------------------------------------------------------------------------
	透過つきアルファブレンド
	引数:０～３２（大きいほど透明）
-----------------------------------------------------------------------------*/
class CCalTransAlpha : public CCalculate
{
public:
	CCalTransAlpha(){};
	CCalTransAlpha(DWORD alpha){if(alpha>ALPHA_MAX)alpha=ALPHA_MAX;m_alpha=ALPHA_MAX-alpha;m_beta=alpha;}
	virtual DWORD Get(DWORD dest,DWORD src);	//実装はNkImage.cppに
private:
	DWORD m_alpha;
	DWORD m_beta;
};
#endif

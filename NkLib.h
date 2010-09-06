/*-----------------------------------------------------------------------------

	NkLib.h

-----------------------------------------------------------------------------*/

#ifndef AFX_NKLIB_H
#define AFX_NKLIB_H

//ゲーム情報
#define GAMETITLE "SUZURI SHOOTING VER 1.00β"
#define SCREEN_WIDTH    320
#define SCREEN_HEIGHT   240
#define SCREEN_BPP      16
#define _FPS 16.6
#define FPSSHOW_WIDTH	24
#define FPSSHOW_HEIGHT	16

//補助マクロ
#define MB(str) MessageBox(NULL,str,GAMETITLE,MB_OK)
#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete [] (p);	  (p)=NULL; } }
#define SAFE_ACQUIRE(p)  { if(p) { (p)->Acquire();	} }
#define SAFE_UNACQUIRE(p) { if(p) { (p)->Unacquire();} }

//入力情報
#define INP_NEUTRAL	-1
#define INP_RIGHT	0
#define INP_LEFT	1
#define INP_DOWN	2
#define INP_UP		3
#define INP_B0		4
#define INP_B1		5
#define INP_B2		6
#define INP_B3		7
#define INP_MAX		8

//フォントフラグ
#define FONT_ITALIC		0x01
#define FONT_ULINE		0x02
#define FONT_LINEOUT	0x04

// 565 ⇔ 555 変換マクロ
#define	to555(_a_) (((_a_ & _mask4) >> 1) | (_a_ & _mask5))
#define	to565(_a_) (((_a_ & _mask4) << 1) | (_a_ & _mask5))

class CNkLib;
class CNkImage;
class CDrawQue;
class CCalculate;
extern CNkLib* g_pNkLib;//グローバルなポインタ
/*-----------------------------------------------------------------------------
	WinMain関数
	アプリケーション・エントリーポイント
-----------------------------------------------------------------------------*/
int APIENTRY WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCmdLine, int nCmdShow );
/*-----------------------------------------------------------------------------
	WndProc関数
	ウィンドウプロシージャ
-----------------------------------------------------------------------------*/
LRESULT CALLBACK WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

/*-----------------------------------------------------------------------------
  DRAWINFO構造体
  CDrawQueにスプライトレベルを持たせて、ソートできる。
-----------------------------------------------------------------------------*/
typedef struct tagDRAWINFO
{
	int spr;	//スプライトレベル（大きいほど手前）
	CDrawQue *pDrawQue;
	
	bool operator<(tagDRAWINFO rinfo){return spr<rinfo.spr;}

	tagDRAWINFO(){}
	tagDRAWINFO(int s,CDrawQue *que){spr=s;pDrawQue=que;}
}DRAWINFO, *LPDRAWINFO;

/*-----------------------------------------------------------------------------

	CNkLib
	ウィンドウの管理のほか、ダブルバッファリング・
	キーボード＆コントローラ入力・ＭＩＤＩ再生など、
	１つのアプリケーションにつき１つですむ基本的な機能を実装

-----------------------------------------------------------------------------*/
class CNkLib  
{
public:
	CNkLib(HINSTANCE hInst);
	virtual ~CNkLib();
	//基本機能
	int InitWindow(int nCmdShow);
	int InitDirectDraw(bool bWindow,bool bDouble);
	int WaitTime();
	int Flip();
	int ShowFPS();
	int Text(char *txt,int x,int y,int size,int weight,DWORD dwFlag,COLORREF color,int layer);
	int Circle(int width,COLORREF color,int left,int top,int right,int bottom,int layer);
		int PlusGamma(int red,int green,int blue);	//放棄
		//内部使用
		int UpdateBounds();
		int CreateSurface(LPDDSURFACEDESC2 lpDDSurfaceDesc2,LPDIRECTDRAWSURFACE7* lplpDDSurface);
		int BltFast(int iDestX,int iDestY,LPDIRECTDRAWSURFACE7 pdds,LPRECT prcSrc,bool trans);
		int BltStretch(LPRECT prcDest,LPDIRECTDRAWSURFACE7 pdds,LPRECT prcSrc,bool trans);
		int TextOutBase(char* text,int x,int y,int size,int weight,DWORD dwFlag,COLORREF color);
		int DrawCircle(int width,COLORREF color,int left,int top,int right,int bottom);
		//ブロック転送関係
		int BltEffect(int iDestX,int iDestY,LPDIRECTDRAWSURFACE7 pdds,LPRECT prcSrc,CCalculate* pCal);
		int BltEffectBase(int iDestX,int iDestY,LPDIRECTDRAWSURFACE7 pdds,LPRECT prcSrc,CCalculate* pCal);
		int Blt0To0(DWORD *pDest,DWORD *pSrc,int a_skip,int b_skip,int w,int h,CCalculate* pCal);
		int Blt1To0(DWORD *pDest,DWORD *pSrc,int a_skip,int b_skip,int w,int h,CCalculate* pCal);
		int Blt0To1(DWORD *pDest,DWORD *pSrc,int a_skip,int b_skip,int w,int h,CCalculate* pCal);
		int Blt1To1(DWORD *pDest,DWORD *pSrc,int a_skip,int b_skip,int w,int h,CCalculate* pCal);
		//ウィンドウプロシージャをフレンド関数にしとく
		friend LRESULT CALLBACK WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
public:
	//インライン
	int		GetWindowWidth(){return SCREEN_WIDTH*(1+(int)m_bDouble);};
	int		GetWindowHeight(){return SCREEN_HEIGHT*(1+(int)m_bDouble);};
	int		UpdateDirectDraw(){return InitDirectDraw(m_bWindow,m_bDouble);};
	int		ChangeMode(){m_bWindow=!m_bWindow;m_bDouble=m_bWindow;return InitDirectDraw(m_bWindow,m_bDouble);};
	int		ChangeScreenMode(){m_bWindow=!m_bWindow;return InitDirectDraw(m_bWindow,m_bDouble);};
	int		ChangeScreenSize(){m_bDouble=!m_bDouble;return InitDirectDraw(m_bWindow,m_bDouble);};
	bool	GetWindow(){return m_bWindow;};
	bool	GetDouble(){return m_bDouble;};
	bool	SetWindow(bool bWindow){return m_bWindow = bWindow;};
	bool	SetDouble(bool bDouble){return m_bDouble = bDouble;};
	bool	IsOk(){return m_pDD && m_pddsPrimaryBuffer && m_pddsPrimaryBuffer;};
		//ガンマコントロール（放棄
		int		InitGamma(){return m_pGammaControl->SetGammaRamp(0,&m_gammaInit);}
		int		SetGamma(int red,int green,int blue){InitGamma();return PlusGamma(red,green,blue);}
		int		SetGamma(double red,double green,double blue){return SetGamma(red*60000,green*60000,blue*6000);}
		//イメージリスト関係
		void	AddList(CNkImage* pimg){m_lstpImage.push_front(pimg);};
		void	ClearList(){m_lstpImage.clear();};
		void	RemoveList(CNkImage* pimg){m_lstpImage.remove(pimg);};
		int		ReloadAllImage();
		void 	ReleaseAllImage();
public:
	//描画キュー関係
	int		DrawList();
	void	ClearDrawList(){m_lstDInfo.clear();};	//イメージが破棄されるとき用
		void	AddDrawList(tagDRAWINFO dInfo){m_lstDInfo.push_front(dInfo);};
public:
	//Input関係
	int	InitInput();
	int UpdateInput();
	bool GetInputState2(int inp);
		bool	GetKeyboardState(int inp);
		bool	GetJoystickState(int inp);
		HRESULT CreateDeviceEx(REFGUID rguid);
		HRESULT SetJoystickProperty(REFGUID rguidProp, LPCDIPROPHEADER pdiph);
		HRESULT InitializeJoystick(REFGUID rguid);
		int ClearInput();
public:
	//Input関係のインライン関数
	bool	GetInputEvent(int inp){return m_bPresentState[inp] && !m_bFormerState[inp];};
	bool	GetInputHanasi(int inp){return !m_bPresentState[inp] && m_bFormerState[inp];};
	bool	GetKeyboardState(BYTE key);
		bool	GetInputState(int inp){return m_bPresentState[inp];};
		int		GetLR(){return m_iLR;};
		int		GetUD(){return m_iUD;};
public:
	//Sound関係
	int InitSound();
	int InitMidi();
	int LoadMidi(char* lpszFileName, int num);
	int PlayMidi();
	int StopMidi();
		LPDIRECTSOUND GetDS(){return m_pDS;};
private:
	//ウィンドウ関係データメンバ
	HWND		m_hWnd;		//ウィンドウハンドル
	HINSTANCE	m_hInst;	//インスタンスハンドル
	HACCEL		m_hAccel;	//キーボードアクセラレータ
	bool		m_bWindow;	//ウィンドウモードフラグ
	bool		m_bDouble;	//２倍モードフラグ
	bool		m_bActive;	//アクティブフラグ
	int			m_time;		//１フレ前のtimeGetTimeの値
	RECT		m_rcClient;	//クライアント領域
	RECT		m_rcWindow;	//ウィンドウ領域
	DWORD		m_dwOldTime;//１フレ前のＴｉｍｅ
	DWORD		m_dwFPS;	//ＦＰＳ
	DWORD		m_dwCount;	//フレームカウント
private:
	//Draw関係データメンバ
	LPDIRECTDRAW7			m_pDD;
	LPDIRECTDRAWSURFACE7	m_pddsPrimaryBuffer;	//プライマリバッファ
	LPDIRECTDRAWSURFACE7	m_pddsBackBuffer;		//バックバッファ
	LPDIRECTDRAWSURFACE7	m_pddsFPS;				//FPS表示用

	list<CNkImage*>		m_lstpImage;	//全てのイメージのポインタ
	list<tagDRAWINFO>	m_lstDInfo;	//描画キューリスト

	LPDIRECTDRAWGAMMACONTROL	m_pGammaControl;//ガンマコントロールオブジェクトポインタ
	DDGAMMARAMP					m_gammaInit;     //初期ガンマ値
private:
	//Input関係データメンバ
	LPDIRECTINPUT8			m_pDI;
	LPDIRECTINPUTDEVICE8	m_pdidKeyboard;
	LPDIRECTINPUTDEVICE8	m_pdidJoystick;

	BYTE		m_bKeyState[256];	//キーボード状態
	DIJOYSTATE2	m_diJoyState;		//ジョイスティック状態
	bool		m_bPresentState[INP_MAX];	//現在の統合入力状態
	bool		m_bFormerState[INP_MAX];	//１フレ前の統合入力状態
	int			m_iLR;	//現在アクティブなLR
	int			m_iUD;	//現在アクティブなUD
private:
	//Sound関係データメンバ
	LPDIRECTSOUND m_pDS;
	IDirectMusicLoader*			m_pdml;
	IDirectMusicPerformance*	m_pdmp;
	IDirectMusicSegment*		m_pdms;
};

//Ｉｎｐｕｔ関係コールバック関数。
//アプリ側使用引数・pContextにはthisを渡す。

BOOL CALLBACK	EnumObjectsCallback( const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext );
BOOL CALLBACK	EnumJoysticksCallback2( const DIDEVICEINSTANCE* pdidInstance, VOID* pContext );
BOOL CALLBACK	EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance, VOID* pContext );

#endif
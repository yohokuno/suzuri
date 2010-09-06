/*-----------------------------------------------------------------------------

	NkLib.cpp
	アプリケーションの基本的な構造と
	DirectX関係の、いわゆる唯一のオブジェクト初期化

-----------------------------------------------------------------------------*/
#include "stdapp.h"
#include "utility.h"
#include "NkLib.h"
#include "NkImage.h"
#include "NkSound.h"
/*-----------------------------------------------------------------------------
	CNkLibクラス
	唯一のグローバル変数として作成する
-----------------------------------------------------------------------------*/
CNkLib* g_pNkLib = NULL;

/*-----------------------------------------------------------------------------
	CNkLibクラスのコンストラクタ
-----------------------------------------------------------------------------*/
CNkLib::CNkLib(HINSTANCE hInst)
{
	//メモリリークチェック
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//ウィンドウ関係
	m_hInst = hInst;
	m_hWnd = NULL;
	m_hAccel = NULL;
	m_bWindow = true;
	m_bDouble = false;
	m_bActive = true;
	m_time = 0;
	m_dwOldTime = timeGetTime();
	m_dwFPS = 0;
	m_dwCount = 0;

	//Ｄｒａｗ関係
	m_pDD = NULL;
	m_pddsPrimaryBuffer = NULL;
	m_pddsBackBuffer = NULL;
	m_pddsFPS = NULL;
	m_pGammaControl = NULL;

	//Ｉｎｐｕｔ関係
	m_pDI = NULL;
	m_pdidKeyboard = NULL;
	m_pdidJoystick = NULL;
	for(int i=0;i<256;++i)
		m_bKeyState[i] = 0;
	ZeroMemory(&m_diJoyState, sizeof DIJOYSTATE2);
	for(i=0;i<INP_MAX;++i)
	{
		m_bPresentState[i] = FALSE;
		m_bFormerState[i] = FALSE;
	}
	m_iLR = 0;
	m_iUD = 0;

	//Ｓｏｕｎｄ関係
	CoInitialize(NULL);
	m_pDS = NULL;
	m_pdml = NULL;
	m_pdmp = NULL;
	m_pdms = NULL;
	srand(time(0));
}
/*-----------------------------------------------------------------------------
	CNkLibクラスのデストラクタ
-----------------------------------------------------------------------------*/
CNkLib::~CNkLib()
{
	ReleaseAllImage();
	SAFE_RELEASE(m_pGammaControl);
	SAFE_RELEASE(m_pddsFPS);
	SAFE_RELEASE(m_pddsBackBuffer);
	SAFE_RELEASE(m_pddsPrimaryBuffer);
	SAFE_RELEASE(m_pDD);
	SAFE_UNACQUIRE(m_pdidKeyboard);
	SAFE_UNACQUIRE(m_pdidJoystick);
	SAFE_RELEASE(m_pdidKeyboard);
	SAFE_RELEASE(m_pdidJoystick);
	SAFE_RELEASE(m_pDI);
	SAFE_RELEASE(m_pDS);
	SAFE_RELEASE(m_pdml);
	SAFE_RELEASE(m_pdmp);
	SAFE_RELEASE(m_pdms);
	CoUninitialize();
}
/*-----------------------------------------------------------------------------
	ウィンドウを初期化
-----------------------------------------------------------------------------*/
int CNkLib::InitWindow(int nCmdShow)
{
    WNDCLASSEX wc;

	//ウィンドウクラスの登録
    wc.cbSize        = sizeof(wc);
    wc.lpszClassName = TEXT(GAMETITLE);
    wc.lpfnWndProc   = WndProc;
    wc.style         = CS_VREDRAW | CS_HREDRAW;
    wc.hInstance     = m_hInst;
    wc.hIcon         = LoadIcon( m_hInst, MAKEINTRESOURCE(IDI_MAIN_ICON) );
    wc.hIconSm       = LoadIcon( m_hInst, MAKEINTRESOURCE(IDI_MAIN_ICON) );
    wc.hCursor       = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wc.lpszMenuName  = "";
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;

    if( RegisterClassEx( &wc ) == 0 )
	{
		MB("ウィンドウクラスの登録に失敗しました");
        return 0;
	}

	//キーボードアクセラレーター
    m_hAccel = LoadAccelerators( m_hInst, MAKEINTRESOURCE(IDR_MAIN_ACCEL) );

	//ウィンドウサイズを計算
    DWORD dwFrameWidth    = GetSystemMetrics( SM_CXSIZEFRAME );
    DWORD dwFrameHeight   = GetSystemMetrics( SM_CYSIZEFRAME );
    DWORD dwCaptionHeight = GetSystemMetrics( SM_CYCAPTION );
    DWORD dwWindowWidth   = (DWORD)GetWindowWidth()  + dwFrameWidth * 2;
    DWORD dwWindowHeight  = (DWORD)GetWindowHeight() + dwFrameHeight * 2 + 
                            + dwCaptionHeight;

	//ウィンドウを作成・表示
    DWORD dwStyle = WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX;
    m_hWnd = CreateWindowEx( 0, TEXT(GAMETITLE), 
                           TEXT(GAMETITLE),
                           dwStyle, CW_USEDEFAULT, CW_USEDEFAULT,
  	                       dwWindowWidth, dwWindowHeight, NULL, NULL, m_hInst, NULL );
    if( m_hWnd == NULL )
	{
		MB("ウィンドウ作成に失敗");
		return 0;
	}
    ShowWindow( m_hWnd, nCmdShow );
    UpdateWindow( m_hWnd );
    GetWindowRect( m_hWnd, &m_rcWindow );
	return 1;
}
/*-----------------------------------------------------------------------------
	WaitTime関数
	６０ＦＰＳになるようにしながらメッセージの処理をする
-----------------------------------------------------------------------------*/
int CNkLib::WaitTime()
{
    MSG	   msg;
	while(true)
	{
		while(true)
		{
			if( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
	        {
				if( 0 == GetMessage(&msg, NULL, 0, 0 ) )
				{
				    // WM_QUITを検知
		            return 0;
			    }

	            //メッセージを翻訳
	            if( 0 == TranslateAccelerator( m_hWnd, m_hAccel, &msg ) )
	            {
		            TranslateMessage( &msg ); 
			        DispatchMessage( &msg );
				}
			}
			else
				break;
		}
		//アクティブかどうかを取得
		m_bActive = (GetActiveWindow()!=NULL);

        if( m_bActive )
        {
			int time = (int)timeGetTime();
			int timeDifferent = time - m_time;
			if( timeDifferent > _FPS )
			{
				m_time = time;
				break;
			}
        }
        else
        {
			//非アクティブ
			ClearInput();
            WaitMessage();
            m_time = timeGetTime();
        }
	}
	return 1;
}
/*-----------------------------------------------------------------------------
	InitDirectDraw関数
	DirectDraw初期化
-----------------------------------------------------------------------------*/
int CNkLib::InitDirectDraw(bool bWindow,bool bDouble)
{
	//すでに初期化されてれば、捨てる。
	SAFE_RELEASE(m_pGammaControl);
	SAFE_RELEASE(m_pddsFPS);
	SAFE_RELEASE(m_pddsBackBuffer);
	SAFE_RELEASE(m_pddsPrimaryBuffer);
	if(m_pDD )
	   m_pDD->SetCooperativeLevel(m_hWnd, DDSCL_NORMAL);
	SAFE_RELEASE(m_pDD);
	//メンバを更新
	m_bWindow = bWindow;
	m_bDouble = bDouble;
	//ＤｉｒｅｃｔＤｒａｗオブジェクト生成
	if(FAILED(DirectDrawCreateEx(NULL, (VOID**)&m_pDD,IID_IDirectDraw7,NULL)))
	{
		MB("DirectDrawオブジェクト作成に失敗しました。");
		return 0;
	}
	//ここからはウィンドウモードとフルスクモードで場合分け
	if(bWindow)	//ウィンドウモード
	{
		//協調レベルの設定
		if(FAILED(m_pDD->SetCooperativeLevel(m_hWnd, DDSCL_NORMAL)))
		{
			MB("DirectDrawにWindowモードの協調レベルを設定できませんでした。");
			return 0;
		}
		//ウィンドウスタイルの変更
		DWORD dwStyle;	//ウィンドウスタイル
		dwStyle  = GetWindowStyle(m_hWnd);
		dwStyle &= ~WS_POPUP;
		dwStyle |= WS_OVERLAPPED | WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX;
		SetWindowLong(m_hWnd, GWL_STYLE, dwStyle);
		//ウィンドウサイズ・位置の変更
		RECT rc;	//ウィンドウ領域
		SetRect( &rc, 0, 0, (DWORD)this->GetWindowWidth(), (DWORD)this->GetWindowWidth());
		AdjustWindowRectEx(&rc, GetWindowStyle(m_hWnd), GetMenu(m_hWnd) != NULL, GetWindowExStyle(m_hWnd));
		SetWindowPos(m_hWnd, NULL, 0, 0, rc.right-rc.left, rc.bottom-rc.top,SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
		SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
		//画面外に出てたら直す
		RECT rcWork;	//作業用ＲＥＣＴ
		SystemParametersInfo( SPI_GETWORKAREA, 0, &rcWork, 0 );
		GetWindowRect( m_hWnd, &rc );
		if( rc.left < rcWork.left ) rc.left = rcWork.left;
		if( rc.top	< rcWork.top )	rc.top	= rcWork.top;
		SetWindowPos( m_hWnd, NULL, rc.left, rc.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );
		//プライマリバッファを作成
		DDSURFACEDESC2 ddsd;
		ZeroMemory( &ddsd, sizeof( ddsd ) );
		ddsd.dwSize 		= sizeof( ddsd );
		ddsd.dwFlags		= DDSD_CAPS;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;	//システムメモリに作る
		if( FAILED( m_pDD->CreateSurface( &ddsd, &m_pddsPrimaryBuffer, NULL)))
		{
			MB("プライマリバッファ作成に失敗しました。");
			return 0;
		}
		//バックバッファを作成
		ddsd.dwFlags		= DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;	//システムメモリに作る
		ddsd.dwWidth		= (DWORD)this->GetWindowWidth();
		ddsd.dwHeight		= (DWORD)this->GetWindowHeight();
		
		ddsd.ddpfPixelFormat.dwSize = sizeof(ddsd.ddpfPixelFormat);
		ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;
		ddsd.ddpfPixelFormat.dwRGBBitCount = 16;
		//RGB565カラー
		ddsd.ddpfPixelFormat.dwRBitMask = 0x0000F800 ;
		ddsd.ddpfPixelFormat.dwGBitMask = 0x000007E0 ;
		ddsd.ddpfPixelFormat.dwBBitMask = 0x0000001F ;
		ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = 0x00000000 ;

		if( FAILED(m_pDD->CreateSurface( &ddsd, &m_pddsBackBuffer, NULL)))
		{
			MB("バックバッファ作成に失敗しました。");
			return 0;
		}
		//クリッパーの設定
		LPDIRECTDRAWCLIPPER pcClipper;
		if( FAILED(m_pDD->CreateClipper( 0, &pcClipper, NULL ) ) )
		{
			MB("クリッパー作成に失敗しました。");
			return 0;
		}
		//クリッパーにウィンドウハンドルをセット
		if( FAILED(pcClipper->SetHWnd( 0, m_hWnd)))
		{
			MB("クリッパー作成に失敗しました。");
			pcClipper->Release();
			return E_FAIL;
		}
		//プライマリにクリッパーをセット
		if( FAILED(m_pddsPrimaryBuffer->SetClipper( pcClipper ) ) )
		{
			MB("クリッパー作成に失敗しました。");
			pcClipper->Release();
			return E_FAIL;
		}
		//用済みなのでクリッパーを削除
		pcClipper->Release();
		//メニューをつける
        SetMenu( m_hWnd, NULL );
        dwStyle = GetWindowLong( m_hWnd, GWL_STYLE );
        dwStyle |= WS_SYSMENU;
        SetWindowLong( m_hWnd, GWL_STYLE, dwStyle );
#ifdef _WIN64
        m_hInst = (HINSTANCE) GetWindowLongPtr( m_hWnd, GWLP_HINSTANCE );
#else
        m_hInst = (HINSTANCE) GetWindowLong( m_hWnd, GWL_HINSTANCE );
#endif
        HMENU hMenu = LoadMenu( m_hInst, "");
        SetMenu( m_hWnd, hMenu );
	}
	//フルスクリーンモード
	else
	{
		//協調レベルの設定
		if(FAILED( m_pDD->SetCooperativeLevel(m_hWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN)))
		{
			SetWindow(true);
			int ret = InitDirectDraw(m_bWindow,m_bDouble);
			MB("DirectDrawにFullScreenモードの協調レベルを設定できませんでした。");
			return E_FAIL;
		}
		//ディスプレイモードの設定
		if( FAILED( m_pDD->SetDisplayMode((DWORD)GetWindowWidth(),(DWORD)GetWindowHeight(),SCREEN_BPP, 0, 0)))
		{
			SetWindow(true);
			int ret = InitDirectDraw(m_bWindow,m_bDouble);
			MB("16ビットのフルスクリーンモードを設定できませんでした。");
			return 0;
		}
		//プライマリバッファ作成
		DDSURFACEDESC2 ddsd;
		ZeroMemory( &ddsd, sizeof( ddsd ) );
		ddsd.dwSize 		   = sizeof( ddsd );
		ddsd.dwFlags		   = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps    = DDSCAPS_PRIMARYSURFACE;

		if(FAILED(m_pDD->CreateSurface( &ddsd, &m_pddsPrimaryBuffer,NULL)))
		{
			SetWindow(true);
			int ret = InitDirectDraw(m_bWindow,m_bDouble);
			MB("プライマリバッファ作成に失敗しました。");
			return 0;
		}
		//バックバッファを作成
		ddsd.dwFlags		= DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;	//システムメモリに作る
		ddsd.dwWidth		= (DWORD)this->GetWindowWidth();
		ddsd.dwHeight		= (DWORD)this->GetWindowHeight();

		ddsd.ddpfPixelFormat.dwSize = sizeof(ddsd.ddpfPixelFormat);
		ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;
		ddsd.ddpfPixelFormat.dwRGBBitCount = 16;
		//RGB565カラー
		ddsd.ddpfPixelFormat.dwRBitMask = 0x0000F800 ;
		ddsd.ddpfPixelFormat.dwGBitMask = 0x000007E0 ;
		ddsd.ddpfPixelFormat.dwBBitMask = 0x0000001F ;
		ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = 0x00000000 ;
		if( FAILED(m_pDD->CreateSurface( &ddsd, &m_pddsBackBuffer, NULL)))
		{
			SetWindow(true);
			int ret = InitDirectDraw(m_bWindow,m_bDouble);
			MB("バックバッファ作成に失敗しました。");
			return 0;
		}
		//メニューを消す
        SetMenu( m_hWnd, NULL );
        DWORD dwStyle = GetWindowLong( m_hWnd, GWL_STYLE );
        dwStyle &= ~WS_SYSMENU;
        SetWindowLong( m_hWnd, GWL_STYLE, dwStyle );       
	}
	//FPS表示用サーフェイスを作成
	DDSURFACEDESC2 ddsd;
	ZeroMemory( &ddsd, sizeof( ddsd ) );
	ddsd.dwSize 		   = sizeof( ddsd );
	ddsd.dwFlags		= DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
	ddsd.dwWidth		= FPSSHOW_WIDTH;
	ddsd.dwHeight		= FPSSHOW_HEIGHT;
	ddsd.ddpfPixelFormat.dwSize = sizeof(ddsd.ddpfPixelFormat);
	ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;
	ddsd.ddpfPixelFormat.dwRGBBitCount = 16;
	//RGB565カラー
	ddsd.ddpfPixelFormat.dwRBitMask = 0x0000F800 ;
	ddsd.ddpfPixelFormat.dwGBitMask = 0x000007E0 ;
	ddsd.ddpfPixelFormat.dwBBitMask = 0x0000001F ;
	ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = 0x00000000 ;

	if( FAILED(m_pDD->CreateSurface( &ddsd, &m_pddsFPS, NULL)))
		return 0;
	
	//ガンマコントロールオブジェクト生成
	m_pddsPrimaryBuffer->QueryInterface(IID_IDirectDrawGammaControl,(LPVOID*)&m_pGammaControl);
	m_pGammaControl->GetGammaRamp(0,&m_gammaInit);

	UpdateBounds();
	return 1;
}
/*-----------------------------------------------------------------------------
	UpdateBounds
-----------------------------------------------------------------------------*/
int CNkLib::UpdateBounds()
{
	if(m_bWindow)
	{
		GetClientRect( m_hWnd, &m_rcClient );
		ClientToScreen( m_hWnd, (POINT*)&m_rcClient );
		ClientToScreen( m_hWnd, (POINT*)&m_rcClient+1 );
	}
	else
	{
		SetRect( &m_rcClient, 0, 0, GetSystemMetrics(SM_CXSCREEN),
				 GetSystemMetrics(SM_CYSCREEN) );
	}
	return 1;
}
/*-----------------------------------------------------------------------------
	フリップ
-----------------------------------------------------------------------------*/
int CNkLib::Flip()
{
	HRESULT hr;
	if( NULL == m_pddsPrimaryBuffer || NULL == m_pddsBackBuffer )
		return 0;

	HDC hdcDest,hdcSrc;
	m_pddsPrimaryBuffer->GetDC(&hdcDest);
	m_pddsBackBuffer->GetDC(&hdcSrc);
	int ret=BitBlt(hdcDest,m_rcClient.left,m_rcClient.top,GetWindowWidth(),GetWindowHeight(),hdcSrc,0,0,SRCCOPY);
	m_pddsPrimaryBuffer->ReleaseDC(hdcDest);
	m_pddsBackBuffer->ReleaseDC(hdcSrc);
	return ret;

	while( 1 )
	{
		hr = m_pddsPrimaryBuffer->Blt( &m_rcClient, m_pddsBackBuffer, NULL, DDBLT_WAIT, NULL );
		if( hr == DDERR_SURFACELOST )
		{
			MB("サーフェイスロスト");
			m_pDD->RestoreAllSurfaces();
			m_pddsPrimaryBuffer->Restore();
			m_pddsBackBuffer->Restore();
			m_pddsFPS->Restore();
			return 1;
		}
		else
		{
			if( hr != DDERR_WASSTILLDRAWING )
			{
				return 1;
			}
		}
	}
}
/*-----------------------------------------------------------------------------
	空のサーフェイスを作成
-----------------------------------------------------------------------------*/
int CNkLib::CreateSurface(LPDDSURFACEDESC2 lpDDSurfaceDesc2,LPDIRECTDRAWSURFACE7* lplpDDSurface)
{
	if(FAILED(m_pDD->CreateSurface(lpDDSurfaceDesc2,lplpDDSurface,NULL)))
		return 0;
	return 1;
}
/*-----------------------------------------------------------------------------
	バックバッファに転送
-----------------------------------------------------------------------------*/
int CNkLib::BltFast(int iDestX,int iDestY,LPDIRECTDRAWSURFACE7 pdds,LPRECT prcSrc,bool trans)
{
	//透過フラグ
	DWORD dwTrans = trans ? DDBLTFAST_SRCCOLORKEY : 0;
	if(prcSrc)
	{
		RECT rect=*prcSrc;
		//半分画面外ならそのぶんを切り落とす
		if(iDestX<0) { rect.left += abs(iDestX); iDestX = 0;}
		if(iDestX+rect.right-rect.left > SCREEN_WIDTH)
			rect.right -= (iDestX+rect.right-rect.left - SCREEN_WIDTH);
		if(iDestY<0) { rect.top += abs(iDestY);  iDestY = 0;}
		if(iDestY+rect.bottom-rect.top > SCREEN_HEIGHT)
			rect.bottom -= (iDestY+rect.bottom-rect.top - SCREEN_HEIGHT);
		//倍ウィンドウ時は転送先座標の２倍
		DWORD dwDestX = (DWORD)iDestX * (1+m_bDouble);
		DWORD dwDestY = (DWORD)iDestY * (1+m_bDouble);
		//倍ウィンドウ時は転送元領域の２倍
		rect.left	*= (1+m_bDouble);
		rect.right	*= (1+m_bDouble);
		rect.top	*= (1+m_bDouble);
		rect.bottom	*= (1+m_bDouble);
		if(FAILED(m_pddsBackBuffer->BltFast(dwDestX,dwDestY,pdds,&rect,dwTrans)))
		{
			Outputf("x=%d\n",dwDestX);
			Outputf("y=%d\n",dwDestY);
			return 0;
		}
	}
	else
	{
		if(iDestX<0) iDestX=0;
		if(iDestY<0) iDestY=0;
		//倍ウィンドウ時は転送先座標の２倍
		DWORD dwDestX = (DWORD)iDestX * (1+m_bDouble);
		DWORD dwDestY = (DWORD)iDestY * (1+m_bDouble);
		if(FAILED(m_pddsBackBuffer->BltFast(dwDestX,dwDestY,pdds,prcSrc,dwTrans)))
		{
			Outputf("x=%d\n",dwDestX);
			Outputf("y=%d\n",dwDestY);
			return 0;
		}
	}
	return 1;
}
/*-----------------------------------------------------------------------------
	バックバッファに拡大縮小転送
-----------------------------------------------------------------------------*/
int CNkLib::BltStretch(LPRECT prcDest,LPDIRECTDRAWSURFACE7 pdds,LPRECT prcSrc,bool trans)
{
	//透過フラグ
	DWORD dwTrans = trans ? DDBLT_KEYSRC : 0;

	//とりあえずＮＵＬＬ渡しを吸収
	RECT rcDest,rcSrc;
	LPRECT prSrc,prDest;
	{
		if(prcSrc)
		{
			rcSrc = *prcSrc;
			prSrc = &rcSrc;
		}
		else
			prSrc = NULL;
		if(prcDest)
		{
			rcDest = *prcDest;
			prDest = &rcDest;
		}
		else
		{
			SetRect(&rcDest,0,0,SCREEN_WIDTH,SCREEN_HEIGHT);
			prDest = &rcDest;
		}
	}
	//画面外切り落としをする
	{
		if(rcDest.left<0)
		{
			rcSrc.left +=abs(rcDest.left)*(rcSrc.right-rcSrc.left)/(rcDest.right-rcDest.left);
			rcDest.left=0;
		}
		if(rcDest.top<0)
		{
			rcSrc.top +=abs(rcDest.top)*(rcSrc.bottom-rcSrc.top)/(rcDest.bottom-rcDest.top);
			rcDest.top=0;
		}
		if(rcDest.right > SCREEN_WIDTH)
		{
			rcSrc.right -= (rcDest.right-SCREEN_WIDTH)*(rcSrc.right-rcSrc.left)/(rcDest.right-rcDest.left);
			rcDest.right = SCREEN_WIDTH;
		}
		if(rcDest.bottom > SCREEN_HEIGHT)
		{
			rcSrc.bottom -= (rcDest.bottom-SCREEN_HEIGHT)*(rcSrc.bottom-rcSrc.top)/(rcDest.bottom-rcDest.top);
			rcDest.bottom = SCREEN_HEIGHT;
		}
	}
	//２倍モードを処理
	if(m_bDouble)
	{
		rcDest.left		*= 2;
		rcDest.right	*= 2;
		rcDest.top		*= 2;
		rcDest.bottom	*= 2;
		rcSrc.left		*= 2;
		rcSrc.right		*= 2;
		rcSrc.top		*= 2;
		rcSrc.bottom	*= 2;
	}
	//描画
	if(FAILED(m_pddsBackBuffer->Blt(prDest,pdds,prSrc,dwTrans,NULL)))
		return 0;
	return 1;
}
/*-----------------------------------------------------------------------------
	ブロック転送
-----------------------------------------------------------------------------*/
// オフセットを計算(補助用
int	offset (const DDSURFACEDESC2	&desc,int x,int y)
{
	// 横方向を２の単位に丸める
	x &= 0xfffffffe;

	return y * desc.lPitch / 2 + x;
}
//ブロック転送
//ピクセル演算クラスCCalculateのポインタを渡して加算・減算・アルファ合成可能
int CNkLib::BltEffectBase(int iDestX,int iDestY,LPDIRECTDRAWSURFACE7 pdds,LPRECT prcSrc,CCalculate* pCal)
{
	DDSURFACEDESC2	ddsdDest;
	DDSURFACEDESC2	ddsdSrc;
	DWORD *a,*b;
	int		a_skip,b_skip;
	int		w = (int)(prcSrc->right-prcSrc->left);
	int		h = (int)(prcSrc->bottom-prcSrc->top);
	
	//転送元ロック
	memset (&ddsdSrc,0,sizeof (ddsdSrc));
	ddsdSrc.dwSize = sizeof ddsdSrc;
	if(FAILED(pdds->Lock(NULL,&ddsdSrc,DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR ,NULL)))
		return 0;

	//転送先ロック
	memset (&ddsdDest,0,sizeof (ddsdDest));
	ddsdDest.dwSize = sizeof ddsdDest;
	if(FAILED(m_pddsBackBuffer->Lock(NULL,&ddsdDest,DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR ,NULL)))
		return 0;

	// 転送先アドレスとスキップ数を算出
	a = (DWORD*)ddsdDest.lpSurface;
	a += offset (ddsdDest, iDestX, iDestY) / 2;
	a_skip = (ddsdDest.lPitch / 2 - w) / 2;

	// 転送元アドレスとスキップ数を算出
	b = (DWORD*)ddsdSrc.lpSurface;
	b += offset (ddsdSrc, prcSrc->left,prcSrc->top) / 2;
	b_skip = (ddsdSrc.lPitch / 2 - w) / 2;

	//転送座標の偶数と奇数の組み合わせで場合分け・・・
	if ((iDestX & 1) == 0 && (prcSrc->left & 1) == 0)
		Blt0To0 (a,b, a_skip,b_skip, w, h, pCal);
	if ((iDestX & 1) == 1 && (prcSrc->left & 1) == 0)
		Blt1To0 (a,b, a_skip,b_skip, w, h, pCal);
	if ((iDestX & 1) == 0 && (prcSrc->left & 1) == 1)
		Blt0To1 (a,b, a_skip,b_skip, w, h, pCal);
	if ((iDestX & 1) == 1 && (prcSrc->left & 1) == 1)
		Blt1To1 (a,b, a_skip,b_skip, w, h, pCal);
	//サーフェイスのアンロック
	if(FAILED(m_pddsBackBuffer->Unlock(NULL)))
		return 0;
	if(FAILED(pdds->Unlock(NULL)))
		return 0;
	return 1;
}
/*-----------------------------------------------------------------------------
	ブロック転送のクリップ対応インターフェース
-----------------------------------------------------------------------------*/
int CNkLib::BltEffect(int iDestX,int iDestY,LPDIRECTDRAWSURFACE7 pdds,LPRECT prcSrc,CCalculate* pCal)
{
		RECT rect=*prcSrc;
		//半分画面外ならそのぶんを切り落とす
		if(iDestX<0) { rect.left += abs(iDestX); iDestX = 0;}
		if(iDestX+rect.right-rect.left > SCREEN_WIDTH)
			rect.right -= (iDestX+rect.right-rect.left - SCREEN_WIDTH);
		if(iDestY<0) { rect.top += abs(iDestY);  iDestY = 0;}
		if(iDestY+rect.bottom-rect.top > SCREEN_HEIGHT)
			rect.bottom -= (iDestY+rect.bottom-rect.top - SCREEN_HEIGHT);
		//倍ウィンドウ時は転送先座標の２倍
		int dx = iDestX * (1+m_bDouble);
		int dy = iDestY * (1+m_bDouble);
		//倍ウィンドウ時は転送元領域の２倍
		rect.left	*= (1+m_bDouble);
		rect.right	*= (1+m_bDouble);
		rect.top	*= (1+m_bDouble);
		rect.bottom	*= (1+m_bDouble);
		return BltEffectBase(dx,dy,pdds,&rect,pCal);
}
//ここからBltEffectBase用の補助関数
//偶数から偶数
int CNkLib::Blt0To0(DWORD *pDest,DWORD *pSrc,int a_skip,int b_skip,int w,int h,CCalculate* pCal)
{
	int	wx1 = w & 1;	// 幅が奇数かどうか
	int	wxc = w / 2;	// 横の繰り返す数
	int	i,j;
    DWORD *&a=pDest;
	DWORD *&b=pSrc;
	for (j=0; j<h; j++)
	{
		for (i=0; i<wxc; i++)
		{
			*a = pCal->Get(*a,*b);
			a ++;
			b ++;
		}

		if (wx1)
		{
			*a = (*a & _mask10) | (pCal->Get(*a, *b) & _mask01);
			a ++;
			b ++;
		}

		a += a_skip;
		b += b_skip;
	}
	return 1;
}
//奇数から偶数
int CNkLib::Blt1To0(DWORD *pDest,DWORD *pSrc,int a_skip,int b_skip,int w,int h,CCalculate* pCal)
{
	int	wx1 = w & 1;	// 幅が奇数かどうか
	int	wxc = w / 2;	// 横の繰り返す数
	DWORD	hh,l;
	int	i,j;
    DWORD *&a=pDest;
	DWORD *&b=pSrc;

	if (wx1 == 0) wxc --;

	for (j=0; j<h; j++) {

		l = *b << 16;
		hh = *b >> 16;
	
		*a = (*a & _mask01) | (pCal->Get (*a,l) & _mask10);

		for (i=0; i<wxc; i++) {

			a ++;
			b ++;

			l = *b << 16;
		
			*a = pCal->Get (*a,l | hh);

			hh = *b >> 16;
		}

		a ++;
		b ++;

		if (wx1 == 0) {
			*a = (*a & _mask10) | (pCal->Get (*a,hh) & _mask01);
		}

		a += a_skip;
		b += b_skip;
	}

	return 1;
}
//偶数から奇数
int CNkLib::Blt0To1(DWORD *pDest,DWORD *pSrc,int a_skip,int b_skip,int w,int h,CCalculate* pCal)
{
	int	wx1 = w & 1;	// 幅が奇数かどうか
	int	wxc = w / 2;	// 横の繰り返す数
	DWORD	hh,l;
	int	i,j;
    DWORD *&a=pDest;
	DWORD *&b=pSrc;

	for (j=0; j<h; j++) {
		for (i=0; i<wxc; i++) {

			hh = *b >> 16;

			b ++;

			l = *b << 16;

			*a = pCal->Get (*a,hh | l);

			a ++;
		}

		if (wx1) {
			hh = *b >> 16;
			*a = (*a & _mask10) | (pCal->Get (*a,hh) & _mask01);
			a ++;
			b ++;
		}

		a += a_skip;
		b += b_skip;
	}
	return 1;
}
//奇数から奇数
int CNkLib::Blt1To1(DWORD *pDest,DWORD *pSrc,int a_skip,int b_skip,int w,int h,CCalculate* pCal)
{
	int	wx1 = w & 1;	// 幅が奇数かどうか
	int	wxc = w / 2;	// 横の繰り返す数
	int	i,j;
    DWORD *&a=pDest;
	DWORD *&b=pSrc;

	if (wx1 == 0) wxc --;

	for (j=0; j<h; j++) {

		*a = (*a & _mask01) | (pCal->Get(*a,*b) & _mask10);

		a ++;
		b ++;

		for (i=0; i<wxc; i++) {

			*a = pCal->Get (*a,*b);

			a ++;
			b ++;
		}

		if (wx1 == 0) {
			*a = (*a & _mask10) | (pCal->Get (*a,*b) & _mask01);
		}

		a += a_skip;
		b += b_skip;
	}

	return 1;
}
/*-----------------------------------------------------------------------------
	ガンマ値補正
-----------------------------------------------------------------------------*/
int CNkLib::PlusGamma(int red,int green,int blue)
{
	DDGAMMARAMP Gamma;
	m_pGammaControl->GetGammaRamp(0,&Gamma);
	if(red>0)//レッドプラス
		for(int i=0;i<=255;i++){
			if(Gamma.red[i]<65535-red)	Gamma.red[i] +=red;
			else Gamma.red[i]=65535;
	}
	else if(red<0)//レッドマイナス
		for(int i=0;i<=255;i++){
			if(Gamma.red[i]>0+abs(red))	Gamma.red[i] -=abs(red);
			else Gamma.red[i]=0;
	}
	if(green>0)//グリーンプラス
		for(int i=0;i<=255;i++){
			if(Gamma.green[i]<65535-green)	Gamma.green[i] +=green;
			else Gamma.green[i]=65535;
	}
	else if(green<0)//グリーンマイナス
		for(int i=0;i<=255;i++){
			if(Gamma.green[i]>0+abs(green))	Gamma.green[i] -=abs(green);
			else Gamma.green[i]=0;
	}
	if(blue>0)//ブループラス
		for(int i=0;i<=255;i++){
			if(Gamma.blue[i]<65535-blue)	Gamma.blue[i] +=blue;
			else Gamma.blue[i]=65535;
	}
	else if(blue<0)//ブルーマイナス
		for(int i=0;i<=255;i++){
			if(Gamma.blue[i]>0+abs(blue))	Gamma.blue[i] -=abs(blue);
			else Gamma.blue[i]=0;
	}
	return m_pGammaControl->SetGammaRamp(0,&Gamma);
}
/*-----------------------------------------------------------------------------
	すべてのイメージをロードしなおす
-----------------------------------------------------------------------------*/
int CNkLib::ReloadAllImage()
{
	list<CNkImage*>::iterator itr;
	for(itr = m_lstpImage.begin();itr != m_lstpImage.end(); ++itr)
		if(!(*itr)->Reload())
			return 0;
	return 1;
}
/*-----------------------------------------------------------------------------
	すべてのイメージをReleaseする
-----------------------------------------------------------------------------*/
void CNkLib::ReleaseAllImage()
{
	list<CNkImage*>::iterator itr;
	for(itr = m_lstpImage.begin();itr != m_lstpImage.end(); ++itr)
		(*itr)->Release();
}

/*-----------------------------------------------------------------------------
	すべての描画キューを描画する
-----------------------------------------------------------------------------*/
int CNkLib::DrawList()
{
	list<tagDRAWINFO>::iterator itr;
	m_lstDInfo.sort();
	for(itr = m_lstDInfo.begin();itr != m_lstDInfo.end(); ++itr)
	{
		if(!itr->pDrawQue->Draw())
			return 0;
		SAFE_DELETE(itr->pDrawQue);
	}
	m_lstDInfo.clear();
	return 1;
}
/*-----------------------------------------------------------------------------
	TextOutの内部使用バージョン
	２倍モード処理とフラグの置き換えをしてくれる
-----------------------------------------------------------------------------*/
int CNkLib::TextOutBase(char* text,int x,int y,int size,int weight,DWORD dwFlag,COLORREF color)
{
	if(GetDouble()) {x*=2;y*=2;size*=2;}
	HFONT hFont = CreateFont(
		size,0,0,0,weight,
		dwFlag & FONT_ITALIC,
		dwFlag & FONT_ULINE,
		dwFlag & FONT_LINEOUT,
		SHIFTJIS_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		PROOF_QUALITY,
		FIXED_PITCH | FF_MODERN,
		"HG丸ｺﾞｼｯｸM-PRO");
	HDC hdc;
	m_pddsBackBuffer->GetDC(&hdc);
	SelectObject(hdc, hFont);
	SetTextColor(hdc,color);
	SetBkMode(hdc, TRANSPARENT);
	int ret= (int)TextOut(hdc,x,y,text,strlen(text));
	m_pddsBackBuffer->ReleaseDC(hdc);
	DeleteObject(hFont);
	return ret;
}
/*-----------------------------------------------------------------------------
	Text
	描画キュー対応
-----------------------------------------------------------------------------*/
int CNkLib::Text(char *txt,int x,int y,int size,int weight,DWORD dwFlag,COLORREF color,int layer)
{
	DRAWINFO dInfo;
	dInfo.spr=layer;
	dInfo.pDrawQue = new CDrawQueText(txt,x,y,size,weight,dwFlag,color);
	AddDrawList(dInfo);
	return 0;
}
/*-----------------------------------------------------------------------------
	DrawCircle
	円・楕円を描く
-----------------------------------------------------------------------------*/
int CNkLib::DrawCircle(int width,COLORREF color,int left,int top,int right,int bottom)
{
	if(GetDouble()) {width*=2;left*=2;top*=2;right*=2;bottom*=2;}
	HDC hdc;
	m_pddsBackBuffer->GetDC(&hdc);

	HPEN hPen = CreatePen(PS_SOLID,width,color);
    SelectObject(hdc, hPen);
	HBRUSH hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
    SelectObject(hdc, hBrush);

    int ret=Ellipse(hdc, left, top, right, bottom);
    DeleteObject(hPen);
	m_pddsBackBuffer->ReleaseDC(hdc);
	return ret;
}
/*-----------------------------------------------------------------------------
	Circle
	描画キュー対応晩
-----------------------------------------------------------------------------*/
int CNkLib::Circle(int width,COLORREF color,int left,int top,int right,int bottom,int layer)
{
	DRAWINFO dInfo;
	dInfo.spr=layer;
	dInfo.pDrawQue = new CDrawQueCircle(width,color,left,top,right,bottom);
	AddDrawList(dInfo);
	return 1;
}
/*-----------------------------------------------------------------------------

  ここからＩｎｐｕｔ関連ですよ！

-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
	Input初期化
-----------------------------------------------------------------------------*/
int CNkLib::InitInput()
{
	// DirectInput オブジェクトを作成する。
	if(FAILED(DirectInput8Create(m_hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_pDI, NULL)))
	{
		MB("DirectInput Init Failed.");
		return 0;
	}
	/*---------ここからキーボード初期化---------------*/

	// デバイスを取得する。
	if(FAILED(m_pDI->CreateDevice(GUID_SysKeyboard,&m_pdidKeyboard, NULL)))
		MB("Create Keyboard Failed.");
	// データ形式をセットする。
	m_pdidKeyboard->SetDataFormat(&c_dfDIKeyboard);
	// 協調レベルを設定する。
	m_pdidKeyboard->SetCooperativeLevel(m_hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
	//アクセス権を取得する。
	if(FAILED(m_pdidKeyboard->Acquire()))
		MB("keyboard acquire failed.");
	/*---------ここからジョイスティック初期化---------------*/

	//デバイスの列挙
	if(FAILED(m_pDI->EnumDevices( DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback,(VOID*)this, DIEDFL_ATTACHEDONLY)))
		return 0;
	//ジョイスティックが接続されていれば、EnumJoystickCallbackコールバック関数内で
	//m_pdidJoystickは取得されている。

	if(m_pdidJoystick)
	{
		//データフォーマット
		if(FAILED(m_pdidJoystick->SetDataFormat( &c_dfDIJoystick2 )))
			return 0;
		//協調レベル
		if(FAILED(m_pdidJoystick->SetCooperativeLevel( m_hWnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND )))
			return 0;
		//コールバック関数をセット
		if(FAILED(m_pdidJoystick->EnumObjects( EnumObjectsCallback, (VOID*)this, DIDFT_ALL )))
			return 0;
	}
	return 1;
}
/*-----------------------------------------------------------------------------
	Ｉｎｐｕｔを更新する
-----------------------------------------------------------------------------*/
int CNkLib::UpdateInput()
{
	HRESULT hr;
	//キーボード
	if( FAILED( hr = m_pdidKeyboard->GetDeviceState(256,(LPVOID)&m_bKeyState)))
	{
		MB("keyboard GetDeviceState failed.");
		if( hr == DIERR_INPUTLOST ) 
		{
			if(FAILED(hr = m_pdidKeyboard->Acquire()))
				return 0;
		}
		else
			return 0;
	}
	if(FAILED(m_pdidKeyboard->Acquire()))
	{
		MB("acquire failed.");
		return 0;
	}
	//ジョイスティック
	if(m_pdidJoystick)
	{
		if( FAILED( m_pdidJoystick->Poll()))
		{
			hr = m_pdidJoystick->Acquire();
			while( hr == DIERR_INPUTLOST ) 
				hr = m_pdidJoystick->Acquire();
		}
		if(FAILED(m_pdidJoystick->GetDeviceState( sizeof(DIJOYSTATE2), &m_diJoyState )))
			return 0;
	}
	//統合
	for(int i = 0; i < INP_MAX; ++i)
	{
		m_bFormerState[i] = m_bPresentState[i];
		m_bPresentState[i] = GetKeyboardState(i) || GetJoystickState(i);
	}
	//後押し優先にする
	//左右編
	//押された瞬間である。
	if(GetInputEvent(INP_LEFT))
		m_iLR = INP_LEFT;
	else if(GetInputEvent(INP_RIGHT))
		m_iLR = INP_RIGHT;
	//前からアクティブで、現在も押され続けている。
	else if(m_iLR == INP_LEFT && m_bPresentState[INP_LEFT])
		m_iLR = INP_LEFT;
	else if(m_iLR == INP_RIGHT && m_bPresentState[INP_RIGHT])
		m_iLR = INP_RIGHT;
	//前からアクティブなキーは押されていないが押されているキーがある。
	else if(m_bPresentState[INP_LEFT])
		m_iLR = INP_LEFT;
	else if(m_bPresentState[INP_RIGHT])
		m_iLR = INP_RIGHT;
	else
		m_iLR = INP_NEUTRAL;//なにも押されていない。

	//後押し優先にする
	//上下編
	//押された瞬間である。
	if(GetInputEvent(INP_UP))
		m_iUD = INP_UP;
	else if(GetInputEvent(INP_DOWN))
		m_iUD = INP_DOWN;
	//前からアクティブで、現在も押され続けている。
	else if(m_iUD == INP_UP && m_bPresentState[INP_UP])
		m_iUD = INP_UP;
	else if(m_iUD == INP_DOWN && m_bPresentState[INP_DOWN])
		m_iUD = INP_DOWN;
	//前からアクティブなキーは押されていないが押されているキーがある。
	else if(m_bPresentState[INP_UP])
		m_iUD = INP_UP;
	else if(m_bPresentState[INP_DOWN])
		m_iUD = INP_DOWN;
	else
		m_iUD = INP_NEUTRAL;//なにも押されていない。

	return 1;
}
/*-----------------------------------------------------------------------------
	Ｉｎｐｕｔ関係の初期化補助関数
-----------------------------------------------------------------------------*/

HRESULT CNkLib::CreateDeviceEx(REFGUID rguid)
{
	return m_pDI->CreateDevice(rguid,&m_pdidJoystick, NULL );
}
HRESULT CNkLib::SetJoystickProperty(REFGUID rguidProp, LPCDIPROPHEADER pdiph)
{
	return m_pdidJoystick->SetProperty(rguidProp, pdiph);
}
HRESULT CNkLib::InitializeJoystick(REFGUID rguid)
{
	return m_pdidJoystick->Initialize(m_hInst,DIRECTINPUT_VERSION,rguid);
}
/*-----------------------------------------------------------------------------
	ジョイスティｋック状態
-----------------------------------------------------------------------------*/
bool CNkLib::GetJoystickState(int inp)
{
	switch(inp)
	{
	case INP_RIGHT:
		return m_diJoyState.lX>0;
		break;
	case INP_LEFT:
		return m_diJoyState.lX<0;
		break;
	case INP_DOWN:
		return m_diJoyState.lY>0;
		break;
	case INP_UP:
		return m_diJoyState.lY<0;
		break;
	case INP_B0:
		return (m_diJoyState.rgbButtons[0] & 0x80) !=0;
		break;
	case INP_B1:
		return (m_diJoyState.rgbButtons[1] & 0x80) !=0;
		break;
	case INP_B2:
		return (m_diJoyState.rgbButtons[2] & 0x80) !=0;
		break;
	case INP_B3:
		return (m_diJoyState.rgbButtons[3] & 0x80) !=0;
		break;
	}
	return false;
}
/*-----------------------------------------------------------------------------
	キーボード状態
-----------------------------------------------------------------------------*/
bool CNkLib::GetKeyboardState(int inp)
{
	switch(inp)
	{
	case INP_RIGHT:
		return (m_bKeyState[DIK_RIGHT] & 0x80) !=0;
		break;
	case INP_LEFT:
		return (m_bKeyState[DIK_LEFT] & 0x80) !=0;
		break;
	case INP_DOWN:
		return (m_bKeyState[DIK_DOWN] & 0x80) !=0;
		break;
	case INP_UP:
		return (m_bKeyState[DIK_UP] & 0x80) !=0;
		break;
	case INP_B0:
		return (m_bKeyState[DIK_Z] & 0x80) || (m_bKeyState[DIK_A] & 0x80);
		break;
	case INP_B1:
		return (m_bKeyState[DIK_X] & 0x80) || (m_bKeyState[DIK_A] & 0x80);
		break;
	case INP_B2:
		return (m_bKeyState[DIK_C] & 0x80) !=0;
		break;
	case INP_B3:
		return (m_bKeyState[DIK_V] & 0x80) !=0;
		break;
	}
	return false;
}
/*-----------------------------------------------------------------------------
	キーボード＋ジョイスティック状態
-----------------------------------------------------------------------------*/
bool CNkLib::GetInputState2(int inp)
{
	if(inp==INP_LEFT||inp==INP_RIGHT||inp==INP_UP||inp==INP_DOWN)
	{
		if(inp == m_iLR)
			return true;
		if(inp == m_iUD)
			return true;
		return false;
	}
	return GetInputState(inp);
}
/*-----------------------------------------------------------------------------
	Ｉｎｐｕｔデータをクリアー
-----------------------------------------------------------------------------*/
int CNkLib::ClearInput()
{
	for(int i=0;i<256;++i)
		m_bKeyState[i] = 0;
	ZeroMemory(&m_diJoyState, sizeof DIJOYSTATE2);
	for(i=0;i<INP_MAX;++i)
	{
		m_bPresentState[i] = FALSE;
		m_bFormerState[i] = FALSE;
	}
	m_iLR = 0;
	m_iUD = 0;
	return 1;
}
/*-----------------------------------------------------------------------------

	Inputコールバック通常関数

-----------------------------------------------------------------------------*/
BOOL CALLBACK EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance, VOID* pContext )
{
    HRESULT hr;
	CNkLib *pInput = (CNkLib*)pContext;

    hr = pInput->CreateDeviceEx( pdidInstance->guidInstance);

    if( FAILED(hr) ) 
        return DIENUM_CONTINUE;		//列挙を続ける
    return DIENUM_STOP;				//列挙を終わる
}
BOOL CALLBACK EnumObjectsCallback( const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext )
{
	CNkLib *pInput = (CNkLib*)pContext;

    // For axes that are returned, set the DIPROP_RANGE property for the
    // enumerated axis in order to scale min/max values.
    if( pdidoi->dwType & DIDFT_AXIS )
    {
        DIPROPRANGE diprg; 
        diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
        diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
        diprg.diph.dwHow        = DIPH_BYID; 
        diprg.diph.dwObj        = pdidoi->dwType; // Specify the enumerated axis
        diprg.lMin              = -1000; 
        diprg.lMax              = +1000; 
    
        // Set the range for the axis
        if( FAILED( pInput->SetJoystickProperty( DIPROP_RANGE, &diprg.diph ) ) ) 
            return DIENUM_STOP;
         
    }
    return DIENUM_CONTINUE;
}
/*-----------------------------------------------------------------------------

  ここからSound,MIDI関連ですよ！

-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
	Ｓｏｕｎｄ初期化
-----------------------------------------------------------------------------*/
int CNkLib::InitSound()
{
	if(FAILED(DirectSoundCreate(NULL, &m_pDS, NULL)))
		return 0;
	if(FAILED(m_pDS->SetCooperativeLevel(m_hWnd, DSSCL_NORMAL)))
		return 0;
	return 1;
}
/*-----------------------------------------------------------------------------
	MIDI初期化
-----------------------------------------------------------------------------*/
int CNkLib::InitMidi()
{
	//パフォーマンスの作成
	if(FAILED(CoCreateInstance(
				CLSID_DirectMusicPerformance,
				NULL,
				CLSCTX_INPROC,
				IID_IDirectMusicPerformance2,
				(LPVOID*)&m_pdmp)))
		return 0;
	//パフォーマンスを初期化
	if(FAILED(m_pdmp->Init(NULL, m_pDS, m_hWnd)))
		return 0;
	//ポートを作成
	if(FAILED(m_pdmp->AddPort(NULL)))
		return 0;

	//ローダーの作成
    if(FAILED(CoCreateInstance(
				CLSID_DirectMusicLoader,
				NULL,
				CLSCTX_INPROC,
				IID_IDirectMusicLoader,
				(LPVOID*)&m_pdml)))
		return 0;

	return 1;
}
/*-----------------------------------------------------------------------------
	MIDIロード
-----------------------------------------------------------------------------*/
int CNkLib::LoadMidi(char* lpszFileName, int num)
{
	DMUS_OBJECTDESC odesc;
	ZeroMemory(&odesc, sizeof(DMUS_OBJECTDESC));
	odesc.dwSize		= sizeof(DMUS_OBJECTDESC);
	odesc.guidClass		= CLSID_DirectMusicSegment;	
	odesc.dwValidData	= DMUS_OBJ_CLASS|DMUS_OBJ_FILENAME;
	//UNICODE
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, lpszFileName, -1, odesc.wszFileName, MAX_PATH);	

	if(FAILED(m_pdml->GetObject(&odesc, IID_IDirectMusicSegment, (LPVOID*)&m_pdms)))
        return 0;

	m_pdms->SetParam(GUID_StandardMIDIFile, -1, 0, 0, (LPVOID)m_pdmp);

	m_pdms->SetParam(GUID_Download, -1, 0, 0, (LPVOID)m_pdmp);
	//セグメントのリピート回数を設定
	m_pdms->SetRepeats(num);

	return 1;
}
/*-----------------------------------------------------------------------------
	MIDI再生
-----------------------------------------------------------------------------*/
int CNkLib::PlayMidi()
{
	StopMidi();
	//セグメントの再生
	m_pdmp->PlaySegment(m_pdms, 0, 0, NULL);
	return 1;
}
/*-----------------------------------------------------------------------------
	MIDI停止
-----------------------------------------------------------------------------*/
int CNkLib::StopMidi()
{
	m_pdmp->Stop(NULL, NULL, 0, 0);
	return 1;
}
/*-----------------------------------------------------------------------------
	FPS表示
-----------------------------------------------------------------------------*/
int CNkLib::ShowFPS()
{
	DWORD dwTime=timeGetTime();
	m_dwCount++;
	char str[256];
	if(dwTime-m_dwOldTime >= 1000)
	{
		m_dwFPS = 1000*m_dwCount/(dwTime-m_dwOldTime);
		m_dwCount = 0;
		sprintf(str,"%3d",m_dwFPS);
		m_dwOldTime = timeGetTime();

		//ここからFPS表示
		HDC hDC;
		m_pddsFPS->GetDC( &hDC );
		SetBkColor( hDC, 0 );
		SetTextColor( hDC, RGB(255,255,255) );
		TextOut( hDC, 0, 0, str, strlen(str));
		m_pddsFPS->ReleaseDC( hDC );
	}
	RECT rect={0,0,FPSSHOW_WIDTH,FPSSHOW_HEIGHT};
	m_pddsBackBuffer->BltFast(0,0,m_pddsFPS,&rect,DDBLTFAST_WAIT);
	return 1;
}
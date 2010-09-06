/*-----------------------------------------------------------------------------

  WinMain.cpp

-----------------------------------------------------------------------------*/
#include "stdapp.h"
#define WINMAIN_CPP
#include "include.h"
RESOURCE* g_pResource = NULL;
CONFIG*	g_pConfig = NULL;
SCORE* g_pScore = NULL;
int Title();
void EndStage();
void EndGame();
int Pause();
/*-----------------------------------------------------------------------------
	WinMain関数
	タイトルループ・メインループなどの構造がよくわかる
-----------------------------------------------------------------------------*/
int APIENTRY WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCmdLine, int nCmdShow )
{
//	try
	{
		//NkLib初期化
		g_pConfig = new CONFIG;
		g_pNkLib = new CNkLib(hInst);
		g_pNkLib->InitWindow(nCmdShow);
		g_pNkLib->InitDirectDraw(g_pConfig->fwindow,g_pConfig->fdouble);
		g_pNkLib->InitInput();
		g_pNkLib->InitSound();
		g_pNkLib->InitMidi();
		g_pResource = new RESOURCE;//タイトルで使うのでここで初期化
		//タイトル関数
		while(Title())
		{
			//リソース取得
			if(!g_pConfig->effect)
				g_pResource->LoadEffect2();
			g_pScore = new SCORE;
			//ステージ作成
			g_pStage = new CStage(1);
			//プレイヤー作成
			g_pPlayer = new CPlayer(&g_pResource->imgPlayer,&g_pResource->imgSurasuta,3,800);
			while(true)
			{
				//Wait & Pause
				if(!g_pNkLib->WaitTime()||!Pause())
				{
					EndStage();
					EndGame();
					return 0;
				}
				//UpdateInput
				g_pNkLib->UpdateInput();
				if(!g_pStage->StepFrame())//クリアー
				{
					g_pScore->score += SCORE_CLEAR;
					break;
				}
				if(!g_pPlayer->StepFrame())//ゲームオーバー
					break;
				g_pshot.StepFrame();
				g_enemy.StepFrame();
				g_effect.StepFrame();
				g_item.StepFrame();
				g_enemyShot.StepFrame();
				g_pScore->Draw();
				if(!g_pNkLib->DrawList())//DrawList
				{
					MB("DrawList() failed.");
					return 0;
				}
				g_pNkLib->ShowFPS();//ShowFPS
				if(!g_pNkLib->Flip())//Flip
				{
					MB("Flip failed.");
					break;
				}
			}
			EndStage();
		}
		EndGame();
		return 0;
	}
//	catch(...){MB("例外発生");}
	return 0;
}
/*-----------------------------------------------------------------------------
	タイトル
-----------------------------------------------------------------------------*/
int Title()
{
	g_pNkLib->ClearDrawList();
	int mode=0;
	int fade=0;
	while(true)
	{
		if(!g_pNkLib->WaitTime())
			return 0;
		g_pNkLib->UpdateInput();
		if(fade==0)
		{
			g_pResource->imgTitle.DrawLayer(0,0,0,NULL,false);
			if(mode<1 && g_pNkLib->GetInputEvent(INP_DOWN))
			{
				mode ++;
				g_pResource->sndSentaku.Play(0);
			}
			if(mode>0 && g_pNkLib->GetInputEvent(INP_UP))
			{
				mode --;
				g_pResource->sndSentaku.Play(0);
			}
			if(g_pNkLib->GetInputEvent(INP_B0))
			{
				g_pNkLib->ClearDrawList();
				g_pResource->sndKettei.Play(0);
				if(mode==0)
					fade = 1;
				if(mode==1)
					return 0;
			}
		}
		if(fade>0)
		{
			g_pResource->imgTitle.DrawLayerStretch(160,120,1.0+(double)fade/30,0,0,0,false);
			fade++;
			if(fade>30)
				return 1;
			g_pResource->imgWhite.DrawLayerEffect(CCalAlpha(32-32*fade/30),0,0,1000,0,0);
		}
		g_pNkLib->Text("START",200,180,15+2*(mode==0),500,FONT_ITALIC,RGB(250,250,250),20);
		g_pNkLib->Text("START",201,181,15+2*(mode==0),500+300*(mode==0),FONT_ITALIC,RGB(0,0,0),10);
		g_pNkLib->Text("EXIT",200,200,15+2*(mode==1),500,FONT_ITALIC,RGB(250,250,250),20);
		g_pNkLib->Text("EXIT",201,201,15+2*(mode==1),500+300*(mode==1),FONT_ITALIC,RGB(0,0,0),10);
		g_pNkLib->DrawList();
		g_pNkLib->ShowFPS();
		g_pNkLib->Flip();
	}
}
/*-----------------------------------------------------------------------------
	ステージ終了
-----------------------------------------------------------------------------*/
void EndStage()
{
	try
	{
		g_pshot.DeleteAll();
		g_enemy.DeleteAll();
		g_effect.DeleteAll();
		g_item.DeleteAll();
		g_enemyShot.DeleteAll();
		SAFE_DELETE(g_pPlayer);
		SAFE_DELETE(g_pStage);
		SAFE_DELETE(g_pScore);
	}
	catch(...){MB("ステージを終了します。");}
}
/*-----------------------------------------------------------------------------
	ゲーム終了
-----------------------------------------------------------------------------*/
void EndGame()
{
	try
	{
		SAFE_DELETE(g_pResource);
		SAFE_DELETE(g_pConfig);
		SAFE_DELETE(g_pNkLib);
	}
	catch(...){MB("ゲームを終了します。");}
}
/*-----------------------------------------------------------------------------
	ポーズ
-----------------------------------------------------------------------------*/
int Pause()
{
	if(g_pNkLib->GetInputEvent(INP_B3))
	{
		g_pResource->sndPause.Play(0);
		while(g_pNkLib->WaitTime())
		{
			g_pNkLib->UpdateInput();
			if(g_pNkLib->GetInputEvent(INP_B3))
				return 1;
			g_pResource->imgPause.Draw2(100,VECT(90,90));
			g_pNkLib->DrawList();
			g_pNkLib->Flip();
		}
		return 0;
	}
	return 1;
}
/*-----------------------------------------------------------------------------
	WndProc関数
	ウィンドウプロシージャ
	ウィンドウメッセージを処理
-----------------------------------------------------------------------------*/
LRESULT CALLBACK WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
					HINSTANCE hInst = g_pNkLib->m_hInst;
    switch (msg)
    {
        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
				case IDM_SCREENMODE:
					//フル←→２倍窓
					int reload;
                    if( g_pNkLib->m_bWindow )
                        GetWindowRect( hWnd, &g_pNkLib->m_rcWindow );
					reload= g_pNkLib->m_bWindow==g_pNkLib->m_bDouble;
					g_pNkLib->ChangeMode();
					if(reload)
						g_pNkLib->ReloadAllImage();
					g_pConfig->fwindow = g_pNkLib->GetWindow();
					g_pConfig->fdouble = g_pNkLib->GetDouble();
					return 0L;

                case IDM_TOGGLEFULLSCREEN:
					//フルスクリーン
                    if( g_pNkLib->m_bWindow )
                        GetWindowRect( hWnd, &g_pNkLib->m_rcWindow );
					g_pNkLib->ChangeScreenMode();
					g_pConfig->fwindow = g_pNkLib->GetWindow();
                    return 0L;

				case IDM_DOUBLE:
					//倍モード
                    if( g_pNkLib->m_bWindow )
                        GetWindowRect( hWnd, &g_pNkLib->m_rcWindow );
					g_pNkLib->ChangeScreenSize();
					g_pNkLib->ReloadAllImage();
					g_pConfig->fdouble = g_pNkLib->GetDouble();
					return 0L;
                case IDM_EXIT:
					//終了
            	    PostMessage( hWnd, WM_CLOSE, 0, 0 );
                    return 0L;
            }
            break;
		case WM_PAINT:
			if(g_pNkLib->IsOk())
				g_pNkLib->Flip();
			break;
        case WM_GETMINMAXINFO:
            {
				//ウィンドウサイズ変更を許さない。
                MINMAXINFO* pMinMax = (MINMAXINFO*) lParam;

                DWORD dwFrameWidth    = GetSystemMetrics( SM_CXSIZEFRAME );
                DWORD dwFrameHeight   = GetSystemMetrics( SM_CYSIZEFRAME );
                DWORD dwCaptionHeight = GetSystemMetrics( SM_CYCAPTION );

                pMinMax->ptMinTrackSize.x = (DWORD)g_pNkLib->GetWindowWidth()  + dwFrameWidth * 2;
                pMinMax->ptMinTrackSize.y = (DWORD)g_pNkLib->GetWindowHeight() + dwFrameHeight * 2 + 
                                            dwCaptionHeight;

                pMinMax->ptMaxTrackSize.x = pMinMax->ptMinTrackSize.x;
                pMinMax->ptMaxTrackSize.y = pMinMax->ptMinTrackSize.y;
            }
            return 0L;

        case WM_MOVE:
			g_pNkLib->UpdateBounds();
            return 0L;
        case WM_SIZE:
			g_pNkLib->UpdateBounds();
            break;
        case WM_SETCURSOR:
			//フルスクリーンモードでは、カーソルを隠す
            if( !g_pNkLib->m_bWindow )
            {
                SetCursor( NULL );
                return TRUE;
            }
            break;
        case WM_EXITMENULOOP:
			//メニューを開いてる間、ゲームの進行を止める
            g_pNkLib->m_time = (int)timeGetTime();
            break;
        case WM_EXITSIZEMOVE:
			//サイズを変更しようとしている間、ゲームの進行をとめる
            g_pNkLib->m_time = (int)timeGetTime();
            break;
        case WM_SYSCOMMAND:
            switch( wParam )
            {
                case SC_MOVE:
                case SC_SIZE:
                case SC_MAXIMIZE:
                case SC_MONITORPOWER:
                    if( !g_pNkLib->m_bWindow )
                        return TRUE;
            }
            break;           
        case WM_DESTROY:
            PostQuitMessage( 0 );
            return 0L;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}
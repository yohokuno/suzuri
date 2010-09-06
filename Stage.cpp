/*-----------------------------------------------------------------------------

	Stage.cpp

-----------------------------------------------------------------------------*/
#include "stdapp.h"
#include "include.h"
CStage* g_pStage = NULL;
/*-----------------------------------------------------------------------------

	CStageクラス

-----------------------------------------------------------------------------*/
CStage::CStage(int n)
{
	m_scroll = 0;
	//敵
	for(int i=0;i<ENEMY_ALLNUM;++i)
		m_enemyData[i]=EnemyData(0,NULL);
	//スクリプト読み込み
	char buffer[256];
	sprintf(buffer,"stg\\stage%d.txt",n);
	m_pScript=new CNkScript(buffer);
	int initfunc=m_pScript->SearchFunc("Init");
	if(initfunc<0)
		return;
	(*m_pScript->GetFunc())[initfunc]->m_func.Get();
	//ここからINIファイル読み込み利用初期化
	char file[32];
	sprintf(file,"stg\\stage%d.ini",n);
	m_speed = prfGetDouble(file,"speed","bg");	//スクロールスピード
	m_index = prfGetInt(file,"enemy","start");	//敵のインデックスの開始地点
	m_time = prfGetInt(file,"enemy","time");
	m_imgBg.Load(prfGetString(file,"image","bg",buffer));//背景画像読みこみ
	m_imgStart.Load(prfGetString(file,"image","start",buffer));
	m_imgTikei.Load(prfGetString(file,"image","tikei",buffer),20,20);
	//ステージの曲を流す
	#ifndef _DEBUG
	g_pNkLib->LoadMidi(prfGetString(file,"midi","stage",buffer),10);
	g_pNkLib->PlayMidi();
	#endif
	//地形
	m_tikeiIndex = prfGetInt(file,"tikei","scroll");
	m_tikeiIndexTable = prfGetInt(file,"tikei","index");
	m_tikeiScroll = prfGetInt(file,"tikei","scroll");
	m_tikeiSpeed = prfGetDouble(file,"speed","tikei");	//スクロールスピード
	for(i=0;i<17;++i)
		m_pTikei[i] = NULL;
	for(i=0;i<TIKEI_ALLNUM;++i)
		m_pTikeiTable[i] = NULL;
	//ステージファイル読み込み
	StageRead(file);

}
CStage::~CStage()
{
	#ifndef _DEBUG
	g_pNkLib->StopMidi();
	#endif
	End();
}
int CStage::End()
{
	SAFE_DELETE(m_pScript);
	for(int i=0;i<ENEMY_ALLNUM;++i)
		SAFE_DELETE(m_enemyData[i].pEnemy);
	for(i=0;i<TIKEI_ALLNUM;++i)
		SAFE_DELETE(m_pTikeiTable[i]);
	return 1;
}
int CStage::StepFrame()
{
	if(!EnemyApear() && g_enemy.IsAllNull())
		return 0;
	DrawTikei();
	m_scroll += m_speed;
	m_imgBg.Draw2(0,(-(int)m_scroll)%320,0);
	m_imgBg.Draw2(0,(-(int)m_scroll)%320+320,0);
	//グラデーション
	if(m_time<60)
		g_pResource->imgWhite.DrawLayerEffect(CCalAlpha(32*m_time/60),0,0,1000,0,0);
	//ＳＴＡＧＥ文字
	if(m_time>30)
	if(m_time<50)
		m_imgStart.DrawLayerEffect(CCalTransAlpha(32-32*(m_time-30)/20),60,100*(m_time-20)/30,1000,0,0);
	else if(m_time<160)
		m_imgStart.Draw2(1000,60,100);
	else if(m_time<180)
		m_imgStart.DrawLayerEffect(CCalTransAlpha(32*(m_time-160)/20),60,100*m_time/30-433,1000,0,0);
	return 1;
}
int CStage::EnemyApear()
{
	m_time++;
	while(m_enemyData[m_index].pEnemy!=NULL && m_enemyData[m_index].time==m_time)
	{
		g_enemy.Add(m_enemyData[m_index].pEnemy);	//稼動側に登録して動き出す
		m_enemyData[m_index].pEnemy=NULL;			//ステージ側からは削除
		do
		{
			m_index++;
			if(m_index == ENEMY_ALLNUM)		//全部敵出た＝クリアーしたとき
				return 0;
		}while(m_enemyData[m_index].pEnemy==NULL || m_enemyData[m_index].time<m_time);
	}
	return 1;
}
int CStage::StageRead(char* lpszFileName)
{
	//敵読み込み
	for(int i=0;i<ENEMY_ALLNUM;++i)
	{
		char buffer[256];
		strcpy(buffer,"");
		char numstr[4];
		strcpy(numstr,"");
		itoa(i,numstr,10);
		prfGetString(lpszFileName,"enemy",numstr,buffer);
		if(strcmp(buffer,"DEFAULT") != 0)
		{
			//プロファイル文字列クラス
			CProfile prf(buffer);
			//基本出現情報
			int time = prf.GetInt(',' , '/');
			int x = prf.GetInt(',' , '/');
			int y = prf.GetInt('/');
			int index=m_pScript->SearchClass(prf.Get());
			CClassDefine* pDefine=NULL;
			if(index>=0)
			{
				pDefine=&(*m_pScript->GetClass())[index]->m_define;
				m_enemyData[i]=EnemyData(time,new CEnemy(x,y,pDefine));
			}
			else
				m_enemyData[i]=EnemyData(time,NULL);
		}
	}
	//地形読み込み
	for(i=0;i<TIKEI_ALLNUM;++i)
	{
		char buffer[256];
		char numstr[4];
		itoa(i,numstr,10);
		prfGetString(lpszFileName,"tikei",numstr,buffer);
		if(strcmp(buffer,"DEFAULT") != 0)
		{
			//プロファイル文字列クラス
			CProfile prf(buffer);
			//地形ポインタ
			Tikei* p;
			p = new Tikei;	//メモリを確保
			p->n = prf.GetInt(',');
			for(int j=0;j<12;++j)
				p->anm[j] = prf.GetInt(',');
			m_pTikeiTable[i] = p;//ポインタをテーブルに登録
		}
	}
	return 1;
}
int CStage::DrawTikei()
{
	m_tikeiScroll += m_tikeiSpeed;
	int x=-((int)m_tikeiScroll%20);
	if(abs(x) < m_tikeiSpeed)//次の地形出現
	{
		for(int i=0;i<16;++i)
			m_pTikei[i] = m_pTikei[i+1];
		m_pTikei[16] = m_pTikeiTable[m_tikeiIndexTable];
		//次のテーブルに進む
		m_tikeiIndex++;
		int index = m_tikeiIndexTable+1;
		while(index < TIKEI_ALLNUM && m_pTikeiTable[index] == NULL)
			index++;
		if(index < TIKEI_ALLNUM && m_pTikeiTable[index])
			if(m_pTikeiTable[index]->n == m_tikeiIndex)
				m_tikeiIndexTable = index;
	}
	for(int i=0;i<17;++i)
	{
		int y=0,w=20;
		for(int j=0;j<12;++j)
		{
			if(m_pTikei[i] && m_pTikei[i]->anm[j])
			{
				m_imgTikei.DrawLayer(x,y,40,m_pTikei[i]->anm[j]-1,0,true);
				y += 20;
			}
			else
				y += 20;
		}
		x += w;
	}
	return 1;
}
int CStage::CollTikei(VECT pos)
{
	int i = (pos.x+(int)m_tikeiScroll%20)/20;
	int j = pos.y/20;
	if(i<0 || m_pTikei[i] == NULL)
		return 0;
	else if(m_pTikei[i]->anm[j])
		return 1;
	else 
		return 0;
}
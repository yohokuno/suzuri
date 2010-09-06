/*-----------------------------------------------------------------------------

  Player.cpp

-----------------------------------------------------------------------------*/
#include "stdapp.h"
#include "include.h"

#define PLAYER_MUTEKI 120
//グローバル変数
CPlayer* g_pPlayer = NULL;
CObjArray<CPlayerShot> g_pshot(PSHOT_NUM);

bool GetInputState(int inp){return g_pNkLib->GetInputState2(inp);}
bool GetInputEvent(int inp){return g_pNkLib->GetInputEvent(inp);}
bool GetInputHanasi(int inp){return g_pNkLib->GetInputHanasi(inp);}

CPlayer::CPlayer(CNkImage* pimg, CNkImage* pimgSurasuta, int zanki,int item):m_surasuta(pimgSurasuta,VECT(-40,105))
{
	m_pimg = pimg;
	m_zanki = zanki;
	m_pos = VECT(-40,105);
	m_shottime = 0;
	m_anmtime = 0;
	m_state = 1;	//出現アニメーションする、ということ。
	m_muteki = PLAYER_MUTEKI;
	m_item = 0;
	m_power = 0;
	for(m_item=0;m_item<=item;++m_item)
		if(m_item%(80+150*m_power) == 0 && m_power<10)
			m_power++;
}
CSurasuta::CSurasuta(CNkImage* pimg,VECT pos)
{
	m_pimg = pimg;
	m_posSurasuta = pos;
	m_posGun = pos;
	for(int i=0;i<SURASUTA_LATE;++i)
		m_playerpos[i] = pos;
	m_index = 0;
	m_anmtime = 0;
}

CPlayer::~CPlayer()
{
}
#define PLAYER_NANAME (GetInputState(INP_UP)||GetInputState(INP_DOWN))&&(GetInputState(INP_LEFT)||GetInputState(INP_RIGHT))
#define PLAYER_SPEED (3-GetInputState(INP_B0))/(PLAYER_NANAME?1.414:1)
#define PLAYER_ANMCYCLE 15
int CPlayer::StepFrame()
{
	int i;
	VECT former = m_pos;
	switch(m_state)
	{
	case 0://通常状態
		//移動処理
		if(GetInputState(INP_RIGHT) && GetRight() < SCREEN_WIDTH)
			m_pos.x += PLAYER_SPEED;
		while(CollTikei())
		{
			m_pos.x --;
			if(m_pos.x < 0 && m_muteki==0)
			{
				Damage();
				break;
			}
		}
		if(GetInputState(INP_LEFT) && GetLeft() > 0)
			m_pos.x -= PLAYER_SPEED;
		if(CollTikei())
			m_pos.x = former.x;
		if(GetInputState(INP_DOWN) && GetBottom() < SCREEN_HEIGHT)
			m_pos.y += PLAYER_SPEED;
		if(GetInputState(INP_UP) && GetTop() > 0)
			m_pos.y -= PLAYER_SPEED;
		if(CollTikei())
			m_pos.y = former.y;
		//ショット
		if(GetInputState(INP_B0))
		{
			m_shottime++;
			if(m_shottime%(8-m_power/2) == 0)
			{
				g_pResource->sndShot.Play(0);
				g_pshot.Add(new CPlayerShot(&g_pResource->imgPlayerShot, m_surasuta.GetPos()-VECT(10,5), VECT(20,0)));
				g_pshot.Add(new CPlayerShot(&g_pResource->imgPlayerShot, m_surasuta.GetPos()+VECT(-10,5), VECT(20,0)));
			}
			if(m_shottime%(10) == 0)
			{
				if(m_power>0)
				{
					g_pshot.Add(new CPlayerShot(&g_pResource->imgSubShot2, GetCenter()+VECT(0,5), VECT(10,3)));
					g_pshot.Add(new CPlayerShot(&g_pResource->imgSubShot1, GetCenter()-VECT(0,5), VECT(10,-3)));
				}
				if(m_power>3)
				{
					g_pshot.Add(new CPlayerShot(&g_pResource->imgSubShot2, GetCenter()+VECT(0,5), VECT(10,1.5)));
					g_pshot.Add(new CPlayerShot(&g_pResource->imgSubShot1, GetCenter()-VECT(0,5), VECT(10,-1.5)));
				}
				if(m_power>5)
				{
					g_pshot.Add(new CPlayerShot(&g_pResource->imgSubShot2, GetCenter()+VECT(0,5), VECT(10,7)));
					g_pshot.Add(new CPlayerShot(&g_pResource->imgSubShot1, GetCenter()-VECT(0,5), VECT(10,-7)));
				}
				if(m_power>7)
				{
					g_pshot.Add(new CPlayerShot(&g_pResource->imgSubShot2, GetCenter()+VECT(0,5), VECT(10,5)));
					g_pshot.Add(new CPlayerShot(&g_pResource->imgSubShot1, GetCenter()-VECT(0,5), VECT(10,-5)));
				}
			}
		}
		//無敵処理
		if(m_muteki>0)
			m_muteki--;
		//吸い込み
		if(GetInputState(INP_B1))
		{
			for(i=0; i<ITEM_NUM; ++i)
				if(g_item[i])
					g_item[i]->Pull();
			for(i=0; i<ENEMYSHOT_NUM; ++i)
				if(g_enemyShot[i])
					g_enemyShot[i]->Pull(GetCenter());
		}
		//当たり判定：アイテム
		if(m_anmtime%2)
		for(i=0; i<ITEM_NUM; ++i){
			if(g_item[i]){
				if(this->IsColl(g_item[i])){
					g_pResource->sndItem.Play(0);
					g_pScore->item ++;
					g_pScore->score += SCORE_ITEM;
					m_item++;
					if(m_item%(80+150*m_power) == 0 && m_power<10)
						m_power++;
					SAFE_DELETE(g_item[i]);
					break;
				}
			}
		}
		//当たり判定：敵
		if(m_muteki==0){
			for(i=0; i<ENEMY_NUM; ++i){
				if(g_enemy[i]&&g_enemy[i]->GetHP()!=20){
					if(this->IsColl2(g_enemy[i])){
						Damage();
						break;
					}
				}
			}
		}
		//当たり判定：敵弾
		if(m_muteki==0){
			for(i=0; i<ENEMYSHOT_NUM; ++i){
				if(g_enemyShot[i]){
					if(this->IsColl2(g_enemyShot[i])){
						Damage();
						break;
					}
				}
			}
		}
		//アニメ処理
		m_anmtime++;
		if(m_anmtime==PLAYER_ANMCYCLE)
			m_anmtime=0;
		//描画
		//本体（無敵でない・無敵中点滅オン・ポーズ寸前）
		if(m_muteki==0||m_anmtime%5||GetInputEvent(INP_B3))
			m_pimg->Draw2(100, m_pos.x, m_pos.y, m_anmtime*3/PLAYER_ANMCYCLE, GetInputState(INP_LEFT));
		//スラスタ
		if(GetInputState(INP_B0) && GetInputState(INP_B1))	//吸い込み＋ショット
			m_surasuta.StepFrame(m_pos,3);
		else if(GetInputState(INP_B0))//ショット（変形
			m_surasuta.StepFrame(m_pos,2);
		else if(GetInputState(INP_B1))//吸い込み
			m_surasuta.StepFrame(m_pos,1);
		else//なんもなし
			m_surasuta.StepFrame(m_pos,0);
		break;
	case 1://出現モーション中
		m_anmtime++;
		m_pos.x += (30.0-m_anmtime)/5;
		m_pimg->Draw2(100, m_pos);					//本体
		m_surasuta.StepFrame(m_pos,4);
		if(m_anmtime > 40)
		{
			m_anmtime = 0;
			m_muteki = PLAYER_MUTEKI;
			m_state = 0;//通常状態に戻す
		}
		break;
	case 2://死亡モーション（墜落）中
		m_anmtime++;
		m_pos.y += (double)m_anmtime/5;
		m_pimg->Draw2(100, m_pos, 0, 2);
		m_surasuta.StepFrame(m_pos,4);
		if(m_anmtime > 40)
		{
			//ゲームオーバーチェックは墜落が終わってから
			if(m_zanki<0)
				return 0;
			m_pos = VECT(-40,105);
			m_anmtime = 0;
			m_state = 1;
		}
		break;
	}
	//残機数描画
	for(i=0;i<m_zanki;++i)
		g_pResource->imgZanki.Draw2(70,130+i*20,215);
	return 1;
}
#define PLAYERCOLL_LEFT 14
#define PLAYERCOLL_TOP 16
#define PLAYERCOLL_RIGHT 18
#define PLAYERCOLL_BOTTOM 19
int CPlayer::IsColl2(CObj* pobj)
{
	if(m_pos.x+PLAYERCOLL_LEFT > pobj->GetPos().x+pobj->GetImage()->GetWidth())
		return 0;
	if(m_pos.y+PLAYERCOLL_TOP > pobj->GetPos().y+pobj->GetImage()->GetHeight())
		return 0;
	if(m_pos.x+PLAYERCOLL_RIGHT < pobj->GetPos().x)
		return 0;
	if(m_pos.y+PLAYERCOLL_BOTTOM < pobj->GetPos().y)
		return 0;
	return 1;
}
int CPlayer::IsColl2(CEnemy* pEnemy)
{
	return IsColl2(&CObj(g_image[pEnemy->GetImage()],pEnemy->GetPos()));
}
int CPlayer::Damage()
{
	g_pResource->sndDamage.Play(0);
	if(g_pConfig->effect)
	{
		g_effect.Add(new CEffect(&g_pResource->imgExplode1,GetCenter(),1,150));
		g_effect.Add(new CEffect(&g_pResource->imgSmoke1,GetCenter(),2,140));
	}
	else
	{
		g_effect.Add(new CEffect(&g_pResource->imgExplode1,GetCenter(),0,150));
	}
	m_zanki--;
	m_anmtime = 0;
	m_state = 2;
	if(m_zanki<0)
		return 0;
	return 1;
}
//地形判定
#define PL_OFFSET 5
int CPlayer::CollTikei()
{
	return
		g_pStage->CollTikei(VECT(GetLeft()+PL_OFFSET,GetTop()+PL_OFFSET))||
		g_pStage->CollTikei(VECT(GetLeft()+PL_OFFSET,GetBottom()-PL_OFFSET))||
		g_pStage->CollTikei(VECT(GetRight()-PL_OFFSET,GetTop()+PL_OFFSET))||
		g_pStage->CollTikei(VECT(GetRight()-PL_OFFSET,GetBottom()-PL_OFFSET))||
		g_pStage->CollTikei(VECT(GetLeft()+GetWidth()/2,GetTop()))||
		g_pStage->CollTikei(VECT(GetLeft()+GetWidth()/2,GetBottom()))||
		g_pStage->CollTikei(VECT(GetLeft(),GetTop()+GetHeight()/2))||
		g_pStage->CollTikei(VECT(GetRight(),GetTop()+GetHeight()/2));
}
/*-----------------------------------------------------------------------------
	スラスタ関係
-----------------------------------------------------------------------------*/
const VECT c_surasuta_pos = VECT( 8, 15);
const VECT c_gun_pos = VECT( 4, 20);
const VECT c_gattai_pos = VECT( 15, 20);
const VECT c_suikomi_pos = VECT(20,15);
#define SURASUTA_VEL 5
int CSurasuta::StepFrame(VECT pos,int state)
{
	VECT vect;
	m_anmtime++;
	if(m_state != state)
		m_anmtime = 0;
	switch(state)
	{
	case 0://normal
		m_posGun.Goto(m_playerpos[m_index] + c_gun_pos + VECT(0,sin(3.14*2*50*m_anmtime) * 2),SURASUTA_VEL);
		m_pimg->Draw2(110, m_posGun, 0, 1);	//武器
		m_posSurasuta.Goto(m_playerpos[m_index] + c_surasuta_pos + VECT(0,sin(3.14*2*50*(m_anmtime-20)) * 2),SURASUTA_VEL);
		m_pimg->Draw2(120, m_posSurasuta, 0, 0);		//スラスタ
		break;
	case 1://suikomi
		m_posGun.Goto(m_playerpos[m_index] + c_gun_pos + VECT(0,sin(3.14*2*50*m_anmtime) * 2),SURASUTA_VEL);
		m_pimg->Draw2(110, m_posGun, 0, 1);	//武器
		m_posSurasuta.Goto(m_playerpos[m_index] + c_suikomi_pos+ VECT(0,sin(3.14*2*50*(m_anmtime-20)) * 2),SURASUTA_VEL);
		m_pimg->Draw2(120, m_posSurasuta, 0, 0);		//スラスタ
		break;
	case 2://shot
		m_posSurasuta.Goto(m_playerpos[m_index] + c_gattai_pos + VECT(0,sin(3.14*2*50*(m_anmtime-20) * 2)),SURASUTA_VEL);
		m_posGun.Goto(m_playerpos[m_index] + c_gattai_pos + VECT(0,sin(3.14*2*50*m_anmtime)*2),SURASUTA_VEL);
		m_pimg->Draw2(110, m_posGun, 0, 3);//合体
		break;
	case 3://suikomi+shot
		m_posSurasuta.Goto(m_playerpos[m_index] + c_suikomi_pos+ VECT(0,sin(3.14*2*50*(m_anmtime-20)) * 2),SURASUTA_VEL);
		m_posGun.Goto(m_playerpos[m_index] + c_gattai_pos+ VECT(0,sin(3.14*2*50*m_anmtime) * 2),SURASUTA_VEL);
		m_pimg->Draw2(110, m_posSurasuta, 0, 0);//スラスタ
		m_pimg->Draw2(110, m_posGun, 0, 2);//ショット
		break;
	case 4://apear
		m_posSurasuta = pos+c_surasuta_pos;
		m_posGun = pos+c_gun_pos;
		m_pimg->Draw2(120, m_posSurasuta,  0, 0);		//スラスタ
		m_pimg->Draw2(120, m_posGun, 0, 1);		//スラスタ
		break;
	}
	//吸い込み効果音
	if((state==1 || state==3) && (m_state!=1 && m_state !=3))//開始
		g_pResource->sndSuikomi.Play(DSBPLAY_LOOPING );
	if((state!=1 && state!=3) && (m_state==1 || m_state==3))//停止
		g_pResource->sndSuikomi.Stop();
	m_state = state;
	//吸い込みエフェクト描画
	if(state==1||state==3)
	{
		g_pResource->imgSuikomi.Draw2(200,
			m_posSurasuta.x+m_pimg->GetWidth(),
			m_posSurasuta.y+m_pimg->GetHeight()/2-g_pResource->imgSuikomi.GetHeight()/2,
			m_anmtime/5%8,0);
	}
	//スラスタの遅れ処理
	m_playerpos[m_index] = pos;
	m_index++;
	if(m_index==SURASUTA_LATE)
		m_index=0;
	return 1;
}
/*-----------------------------------------------------------------------------
	自ショット関係
-----------------------------------------------------------------------------*/
int CPlayerShot::StepFrame()
{
	m_pos += m_vel;
	if(g_pStage->CollTikei(GetCenter())||!m_pimg->DrawLayerEffect(CCalAdd(),m_pos,150))
		return 0;
	return 1;
}
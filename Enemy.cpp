/*-----------------------------------------------------------------------------

  Enemy.cpp

-----------------------------------------------------------------------------*/
#include "stdapp.h"
#include "include.h"

CObjArray<CEnemy> g_enemy(ENEMY_NUM);

CEnemy::CEnemy(double x,double y,CClassDefine* pDefine)
{
	m_x=x;m_y=y;
	m_pInstance = new CClassInstance(pDefine,this);
}
CEnemy::~CEnemy(){SAFE_DELETE(m_pInstance);}
int CEnemy::Damage()
{
	g_pResource->sndHit.Play(0);
	g_pScore->score += SCORE_SHOT;
	m_hp--;
	g_item.Add(new CItem(&g_pResource->imgItem,GetCenter()+VECT(Rand(-10,10),Rand(-10,10)),VECT(Rand(-4,-2),Rand(-2,2))));
	return 1;
}
int CEnemy::ShotHantei()
{
	//“–‚½‚è”»’èF©’e
	if(m_hp!=-20)
		for(int i=0; i<PSHOT_NUM; ++i)
			if(g_pshot[i])
				if(g_pshot[i]->IsColl(m_x,m_y,GetWidth(),GetHeight()))
				{
					SAFE_DELETE(g_pshot[i]);
					if(m_hp!=-10)
						if(!Damage())
							return 0;
				}
	return 1;
}
int CEnemy::StepFrame()
{
	if(!ShotHantei())
		return 0;
	m_pInstance->Run();
	if(m_hp<=0 && m_hp!=-10 && m_hp!=-20)
	{
		g_pResource->sndExplode.Play(0);
		g_pScore->kill ++;
		g_pScore->score += SCORE_KILL;
		if(g_pConfig->effect)
		{
			g_effect.Add(new CEffect(&g_pResource->imgExplode2,GetCenter(),1,150));
			g_effect.Add(new CEffect(&g_pResource->imgSmoke2,GetCenter(),2,140));
		}
		else
		{
			g_effect.Add(new CEffect(&g_pResource->imgExplode2,GetCenter(),0,150));
		}
		return 0;
	}
	return Draw(150);
}
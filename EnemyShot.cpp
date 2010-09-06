/*-----------------------------------------------------------------------------

  EnemyShot.cpp

-----------------------------------------------------------------------------*/
#include "stdapp.h"
#include "include.h"

CObjArray<CEnemyShot> g_enemyShot(ENEMYSHOT_NUM);
CEnemyShot::CEnemyShot(CNkImage* pimg,VECT pos,VECT vel,int anm)
{
	m_pimg=pimg;
	SetCenter(pos);
	m_vel=vel;
	m_velOrg=vel;
	m_pul=VECT(0,0);
	m_anm=anm;
};
int CEnemyShot::StepFrame()
{
	m_pos += m_vel + m_pul;
	m_vel = m_velOrg;
		m_pul -= m_pul/20;
	if(!m_pimg->DrawLayer(m_pos,300,m_anm,0,true))
		return 0;
	if(g_pStage->CollTikei(m_pos+m_pimg->GetVect()/2))
		return 0;
	return 1;
}
#define PULLSPEED 0.004
int CEnemyShot::Pull(VECT pos)
{
	double dst=m_pos.Distance(pos);
	double kakudo = atan2(m_pos.y-pos.y, m_pos.x-pos.x);
	if(m_velOrg.Cos(pos-m_pos) > 1.0/2)
	if(m_velOrg.Cos(pos-m_pos) > 1.0/2)
	if(kakudo>-3.14/2 && kakudo<3.14/2)
		m_pul = PULLSPEED*(pos-m_pos)/dst *(1000-dst)*3.14/4 - m_velOrg/2;
	return 0;
}

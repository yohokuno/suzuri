/*-----------------------------------------------------------------------------

  Effect.cpp

-----------------------------------------------------------------------------*/
#include "stdapp.h"
#include "include.h"

CObjArray<CEffect> g_effect(EFFECT_NUM);

#define ANM_EXPLODETIME 3
CEffect::CEffect(CNkImage* pimg, VECT pos, int fDraw, int spr)
{
	m_pimg = pimg;
	SetCenter(pos);
	m_fDraw = fDraw;
	m_time = 0;
	m_spr = spr;
}
int CEffect::StepFrame()
{
	m_time++;
	if(m_time/ANM_EXPLODETIME >= 8)
		return 0;
	switch(m_fDraw)
	{
	case 0:
		m_pimg->Draw2(m_spr,m_pos,m_time/ANM_EXPLODETIME);
		break;
	case 1:
		m_pimg->DrawLayerEffect(CCalAdd(),m_pos,m_spr,m_time/ANM_EXPLODETIME);
		break;
	case 2:
		m_pimg->DrawLayerEffect(CCalSub(),m_pos,m_spr,m_time/ANM_EXPLODETIME);
		break;
	}
	return 1;
}
/*-----------------------------------------------------------------------------

  Item.cpp

-----------------------------------------------------------------------------*/
#include "stdapp.h"
#include "include.h"

CObjArray<CItem> g_item(ITEM_NUM);
#define ITEMPULL_SPEED 5

int CItem::StepFrame()
{
	m_pos += m_vel;
	if(m_pull)
	{
		m_pos += (g_pPlayer->GetCenter()-m_pos)/m_pos.Distance(g_pPlayer->GetCenter())*ITEMPULL_SPEED;
		m_pos -= m_vel/2;
		if(g_pPlayer->GetState()!=0)
			m_pull=false;
	}
	if(!m_pimg->Draw2(140,m_pos.x,m_pos.y))
		return 0;
	return 1;
}
int CItem::Pull()
{
	m_pull = true;
	return 1;
}

//全ての敵弾をアイテムに！
void ShotToItem()
{
	for(int i=0;i<ENEMYSHOT_NUM;++i)
	{
		if(g_enemyShot[i])
		{
			g_item.Add(new CItem(&g_pResource->imgItem,g_enemyShot[i]->GetCenter(),g_enemyShot[i]->GetVel()));
			SAFE_DELETE(g_enemyShot[i]);
		}
	}
}
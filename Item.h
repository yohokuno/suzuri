/*-----------------------------------------------------------------------------

	Item.h

-----------------------------------------------------------------------------*/
#ifndef AFX_ITEM_H
#define AFX_ITEM_H

#define ITEM_NUM 50

class CItem;
extern CObjArray<CItem> g_item;
class CItem : public CObj
{
public:
	CItem(CNkImage* pimg, VECT pos, VECT vel) { m_pimg = pimg; SetCenter(pos); m_vel = vel;m_pull=false;};
	int StepFrame();
	int Pull();
protected:
	VECT m_vel;
	bool m_pull;	//吸い込まれたフラグ
};
//全ての敵弾をアイテムに！
void ShotToItem();
#endif

/*-----------------------------------------------------------------------------

  Obj.h

-----------------------------------------------------------------------------*/
#ifndef AFX_OBJ_H
#define AFX_OBJ_H

class CObj;
class CPlayerShot;
template<class T>
class CObjArray;
//プレイヤーショットの配列
extern CObjArray<CPlayerShot> g_pshot;
/*-----------------------------------------------------------------------------
	
	オブジェクトクラス
	インターフェイスの統一＋当たり判定の機能つき

-----------------------------------------------------------------------------*/
class CObj  
{
public:
	CObj(){ m_pimg = NULL; m_pos = VECT(0,0);};
	CObj(CNkImage* pimg, VECT pos=VECT(0,0)) { m_pimg = pimg; m_pos = pos;};
	//当たり判定
	int IsColl(CObj* pobj)
	{
		if(m_pos.x > pobj->GetPos().x+pobj->GetImage()->GetWidth())
			return 0;
		if(m_pos.y > pobj->GetPos().y+pobj->GetImage()->GetHeight())
			return 0;
		if(m_pos.x+m_pimg->GetWidth() < pobj->GetPos().x)
			return 0;
		if(m_pos.y+m_pimg->GetHeight() < pobj->GetPos().y)
			return 0;
		return 1;
	};
	int IsColl(double x,double y,int w,int h)
	{
		if(m_pos.x > x + w)
			return 0;
		if(m_pos.y > y + h)
			return 0;
		if(m_pos.x+m_pimg->GetWidth() < x)
			return 0;
		if(m_pos.y+m_pimg->GetHeight() < y)
			return 0;
		return 1;
	};
public:
	CNkImage* GetImage(){return m_pimg;};
	VECT GetPos(){return m_pos;};
	double &GetX(){return m_pos.x;}
	double &GetY(){return m_pos.y;}
	int GetWidth(){return m_pimg->GetWidth();};
	int GetHeight(){return m_pimg->GetHeight();};
	int GetLeft(){return m_pos.x;};
	int GetTop(){return m_pos.y;};
	int GetRight(){return m_pos.x+m_pimg->GetWidth();};
	int GetBottom(){return m_pos.y+m_pimg->GetHeight();};
	VECT GetCenter(){return m_pos+VECT(GetWidth()/2,GetHeight()/2);};
	void SetCenter(VECT pos){m_pos=pos-VECT(GetWidth()/2,GetHeight()/2);};
protected:
	CNkImage*	m_pimg;
	VECT		m_pos;
};

#endif
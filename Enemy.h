/*-----------------------------------------------------------------------------

	Enemy.h

-----------------------------------------------------------------------------*/
#ifndef AFX_ENEMY_H
#define AFX_ENEMY_H

#define ENEMY_NUM 50
class CEnemy;
class CClassInstance;
class CClassDefine;
extern CObjArray<CEnemy> g_enemy;
extern CObjArray<CNkImage> g_image;
extern CObjArray<CNkSound> g_sound;

class CEnemy
{
public:
	CEnemy(double x,double y,CClassDefine* pDefine);
	~CEnemy();
	int StepFrame();
	int Damage();
	int ShotHantei();
	int PlayerHantei();
public:
	double& GetX(){return m_x;};
	double& GetY(){return m_y;};
	double& GetImage(){return m_img;};
	double& GetHP(){return m_hp;};
	double& GetAnm(){return m_anm;};
	VECT GetPos(){return VECT(m_x,m_y);};
	VECT GetVect(){return VECT(GetWidth(),GetHeight());};
	int GetWidth(){return g_image[m_img]->GetWidth();};
	int GetHeight(){return g_image[m_img]->GetHeight();};
	VECT GetCenter(){return VECT(m_x,m_y)+VECT(GetWidth()/2,GetHeight()/2);};
	int Draw(int spr){return g_image[m_img]->DrawLayer(m_x,m_y,spr,m_anm,0,true);};
private:
	double m_x,m_y;
	double m_img;
	double m_hp;
	double m_anm;

	CClassInstance* m_pInstance;
};
#endif

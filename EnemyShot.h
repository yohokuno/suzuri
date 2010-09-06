/*-----------------------------------------------------------------------------

	EnemyShot.h

-----------------------------------------------------------------------------*/
#ifndef AFX_ENEMYSHOT_H
#define AFX_ENEMYSHOT_H

#define ENEMYSHOT_NUM 1000
class CEnemyShot;
extern CObjArray<CEnemyShot> g_enemyShot;

class CEnemyShot : public CObj
{
public:
	CEnemyShot(CNkImage* pimg,VECT pos,VECT vel,int anm=0);
	virtual int StepFrame();
	int Pull(VECT pos);
	VECT GetVel(){return m_vel;}
protected:
	VECT m_vel;		//最終速度
	VECT m_velOrg;	//元の速度
	VECT m_pul;		//吸い込みの速度
	int m_anm;
};
#endif

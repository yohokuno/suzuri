/*-----------------------------------------------------------------------------

	Effect.h

-----------------------------------------------------------------------------*/
#ifndef AFX_EFFECT_H
#define AFX_EFFECT_H

#define EFFECT_NUM 50
class CEffect;
extern CObjArray<CEffect> g_effect;
class CEffect : public CObj
{
public:
	CEffect(){};
	CEffect(CNkImage* pimg, VECT pos, int fDraw, int spr);
	virtual ~CEffect(){};
	virtual int StepFrame();
protected:
	int m_time;
	int m_fDraw;
	int m_spr;
};
#endif

/*-----------------------------------------------------------------------------

  Player.h

-----------------------------------------------------------------------------*/
#ifndef AFX_PLAYER_H
#define AFX_PLAYER_H

class CPlayer;
class CSurasuta;
//グローバル変数
extern CPlayer* g_pPlayer;

//スラスタの遅れ
#define SURASUTA_LATE 5
//スラスタクラス
class CSurasuta
{
public:
	CSurasuta(CNkImage* pimg,VECT pos);
	~CSurasuta(){};
	int StepFrame(VECT pos,int state);	//state: 0:normal,1;suikomi,2:shot,3:gattai,4:apear
	VECT GetPos(){return m_posGun;};
	VECT GetSurasutaPos(){return m_posSurasuta;};
protected:
	CNkImage* m_pimg;
	VECT m_posSurasuta;
	VECT m_posGun;
	VECT m_playerpos[SURASUTA_LATE];	//プレイヤー位置の保存
	int m_index;		//現在のスラスタのアニメ位置（添え字）
	int m_anmtime;		//ふわふわアニメ用タイマー
	int m_state;		//１フレーム前のstate
};

//CPlayerプレイヤークラス
class CPlayer  : public CObj
{
public:
	CPlayer(CNkImage* pimg, CNkImage* pimgSurasuta, int zanki,int item);
	virtual ~CPlayer();
	virtual int StepFrame();
	int Damage();
	int IsColl2(CObj* pobj);	//チート用（見た目より小さい判定を実現]
	int IsColl2(CEnemy* pEnemy);	//チート用（見た目より小さい判定を実現]
	int CollTikei();
	int GetState(){return m_state;}
protected:
	CSurasuta m_surasuta;	//スラスタ
	int m_shottime;		//ショット用タイマー
	int m_anmtime;		//アニメ用タイマー
	int m_zanki;		//残奇数
	int m_state;		//状態フラグ(0:normal,1:apear,2:fall
	int m_muteki;		//無敵時間
	int m_item;			//とったアイテムの数
	int m_power;		//ショットのパワー(0ならサブショットなし、以後は連射アップ）
};

//PlayerShotオブジェクト
#define PSHOT_NUM 100
class CPlayerShot : public CObj  
{
public:
	CPlayerShot():CObj(){};
	CPlayerShot(CNkImage* pimg, VECT pos, VECT vel):CObj(pimg,pos){m_vel=vel;};
	int StepFrame();
protected:
	VECT m_vel;	//速度
};
#endif
/*-----------------------------------------------------------------------------

	Stage.h

-----------------------------------------------------------------------------*/
#ifndef AFX_STAGE_H
#define AFX_STAGE_H

class CStage;
class CNkScript;
extern CStage* g_pStage;
//１ステージの敵の総数最大
#define ENEMY_ALLNUM 400
//１ステージの敵に使う画像数
#define ENEMYIMAGE_NUM 10
#define TIKEI_ALLNUM 100
//敵データ構造体
//敵の出現は、出現時刻を用いて管理する。
//現れる前にCEnemyをnewしておき、g_enemyに登録した時点で
//実際に動き出し、表示もされる。
struct EnemyData
{
	int time;		//出現時刻
	CEnemy* pEnemy;	//敵オブジェクトへのポインタ
	EnemyData(){};
	EnemyData(int t,CEnemy* p){time=t;pEnemy=p;};//コンストラクタ
};
//地形構造体
struct Tikei
{
	int anm[12];	//アニメーション
	int n;	//列番号
};
//ステージクラス。
//ステージファイルを要するデータを全て持つ。
//敵の出現や背景・地形を管理する。
class CStage
{
public:
	CStage(int n);
	virtual ~CStage();
	virtual int StepFrame();
	int End();
	int EnemyApear();
	int DrawTikei();
	int StageRead(char* lpszFileName);
	int CollTikei(VECT pos);
	int CollTikei(double x,double y){return CollTikei(VECT(x,y));}
protected:
	//敵
	EnemyData m_enemyData[ENEMY_ALLNUM];	//敵出現テーブル
	int m_index;
	int m_time;
	CNkScript* m_pScript;	//スクリプト
	//地形
	CNkImage m_imgTikei;	//地形イメージ
	Tikei* m_pTikei[17];					//アクティブな地形
	Tikei* m_pTikeiTable[TIKEI_ALLNUM];		//地形テーブル
	double m_tikeiSpeed;
	double m_tikeiScroll;
	int m_tikeiIndex;		//アクティブ側のインデックス（テーブルの重複使用も数える)
	int m_tikeiIndexTable;	//テーブル側のインデックス
	//背景
	CNkImage m_imgBg;		//背景一枚絵
	double m_scroll;
	double m_speed;
	//ステージ開始の文字
	CNkImage m_imgStart;
};
#endif

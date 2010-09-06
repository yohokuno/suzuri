/*-----------------------------------------------------------------------------

  struct.h

-----------------------------------------------------------------------------*/
struct RESOURCE;
struct CONFIG;
struct SCORE;

extern RESOURCE* g_pResource;
extern CONFIG* g_pConfig;
extern SCORE* g_pScore;

#define SCORE_SHOT 10
#define SCORE_ITEM 100
#define SCORE_KILL 1000
#define SCORE_CLEAR 10000
#define SCORE_LIFE 10000

/*-----------------------------------------------------------------------------

	リソース構造体
	ステージに関係なく使う画像・効果音データをこの構造体にまとめておき、
	グローバル変数g_pResourceを使ってどこからでもアクセスできるようにする

-----------------------------------------------------------------------------*/
struct RESOURCE
{
	CNkImage imgPlayer;
	CNkImage imgSurasuta;
	CNkImage imgPlayerShot;
	CNkImage imgExplode1;
	CNkImage imgExplode2;
	CNkImage imgSmoke1;
	CNkImage imgSmoke2;
	CNkImage imgItem;
	CNkImage imgZanki;
	CNkImage imgSuikomi;
	CNkImage imgFont;
	CNkImage imgSubShot1;
	CNkImage imgSubShot2;
	CNkImage imgPause;
	CNkImage imgTitle;
	CNkImage imgWhite;

	CNkSound sndShot;
	CNkSound sndHit;
	CNkSound sndExplode;
	CNkSound sndDamage;
	CNkSound sndItem;
	CNkSound sndSuikomi;
	CNkSound sndSentaku;
	CNkSound sndKettei;
	CNkSound sndPause;
	//コンストラクタ：メンバのコンストラクタ, ...
	RESOURCE():

	imgPlayer("img\\suzuri.png",32,36),
	imgSurasuta("img\\surasuta.png",24,12),
	imgPlayerShot("img\\myshot.png"),
	imgExplode1("img\\explode1.png",160,160),
	imgExplode2("img\\explode2.png",64,64),
	imgSmoke1("img\\smoke1.png",160,160),
	imgSmoke2("img\\smoke2.png",64,64),
	imgItem("img\\item.png"),
	imgZanki("img\\zanki.png"),
	imgSuikomi("img\\suikomi.png",60,120),
	imgFont("img\\font1.png",16,16),
	imgSubShot1("img\\subshot1.png"),
	imgSubShot2("img\\subshot2.png"),
	imgPause("img\\PAUSE.png"),
	imgTitle("img\\title.png"),
	imgWhite("img\\white.png"),

	sndShot("snd\\shot.wav"),
	sndHit("snd\\hit.wav"),
	sndExplode("snd\\explode.wav"),
	sndDamage("snd\\damage.wav"),
	sndItem("snd\\item.wav"),
	sndSuikomi("snd\\suikomi.wav"),
	sndSentaku("snd\\sentaku.wav"),
	sndKettei("snd\\kettei.wav"),
	sndPause("snd\\pause.wav")
	{
	};
	void LoadEffect2()
	{
		imgExplode1.Load("img\\explode1b.png",160,160);
		imgExplode2.Load("img\\explode2b.png",64,64);
	}
};
/*-----------------------------------------------------------------------------
	コンフィグ構造体
-----------------------------------------------------------------------------*/
struct CONFIG
{
	int effect;
	bool fwindow;
	bool fdouble;
	//コンストラクタ
	CONFIG()
	{
		effect = prfGetInt("config.ini","config","effect");
		fwindow = prfGetInt("config.ini","config","window")==1;
		fdouble = prfGetInt("config.ini","config","double")==1;
	}
	//デストラクタ
	~CONFIG()
	{
		prfSetInt("config.ini","config","effect",effect);
		prfSetInt("config.ini","config","window",fwindow);
		prfSetInt("config.ini","config","double",fdouble);
	}
};

/*-----------------------------------------------------------------------------
	スコア構造体
-----------------------------------------------------------------------------*/
struct SCORE
{
	int score;
	int highscore;
	int item;		//アイテムゲットの数
	int kill;		//殺した敵の数
	//コンストラクタ
	SCORE()
	{
		char buffer[32];
		highscore = atoi(prfGetString("config.ini","score","score",buffer));
		score = item = kill = 0;
	}
	//デストラクタ
	~SCORE()
	{
		if(score>highscore)
		{
			highscore = score;
			prfSetInt("config.ini","score","score",highscore);
		}
	}
	//描画
	void Draw()
	{
		for(int i=0;i<8;++i)
			g_pResource->imgFont.Draw2(50,320-110+i*13,3,(int)(score/pow(10,(7-i)))%10);
	}
};

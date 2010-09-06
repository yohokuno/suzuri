/*-----------------------------------------------------------------------------

	NkSound.cpp

-----------------------------------------------------------------------------*/
#include "stdapp.h"
#include "utility.h"
#include "NkLib.h"
#include "NkSound.h"

/*-----------------------------------------------------------------------------

	CNkSoundクラス

-----------------------------------------------------------------------------*/
CNkSound::CNkSound()
{
	for(int i=0;i<SOUNDBUFFER_NUM;++i)
		m_pdsb[i] = NULL;
	m_pDS = g_pNkLib->GetDS();
	m_index=0;
}
CNkSound::CNkSound(char* lpszFileName)
{
	for(int i=0;i<SOUNDBUFFER_NUM;++i)
		m_pdsb[i] = NULL;
	m_pDS = g_pNkLib->GetDS();
	m_index=0;
	Load(lpszFileName);
}
CNkSound::~CNkSound()
{
	Release();
}
int CNkSound::Release()
{
	for(int i=0;i<SOUNDBUFFER_NUM;++i)
		SAFE_RELEASE(m_pdsb[i]);
	return 0;
}
int CNkSound::Play(DWORD dwFlags)
{
//	m_pdsb[m_index]->Stop();
	m_pdsb[m_index]->Play(0,0,dwFlags);
	m_index++;
	if(m_index==SOUNDBUFFER_NUM)
		m_index=0;
	return 0;
}
int CNkSound::Stop()
{
	DWORD dwFlags;
	for(int i=0;i<SOUNDBUFFER_NUM;++i)
	{
		if(FAILED(m_pdsb[i]->GetStatus(&dwFlags)))
			return 0;
		else
			if(dwFlags & DSBSTATUS_PLAYING)
				m_pdsb[i]->Stop();
	}
	return 1;
}
int CNkSound::Load(char* lpszFileName)
{
	//RIFF構造のファイルを読み込むのに必要な構造体です。
	//わかりやすい名前を付けました。
	MMCKINFO oya_chunk, ko_chunk;
	
	//waveファイルのオープン
	HMMIO hmmio = mmioOpen(lpszFileName, NULL, MMIO_READ|MMIO_ALLOCBUF);
	if(!hmmio) {
		return false;
	}
	
	//waveファイルかどうか調べる
	oya_chunk.fccType = mmioFOURCC('W', 'A', 'V', 'E');
	if(mmioDescend(hmmio, &oya_chunk, NULL, MMIO_FINDRIFF) != MMSYSERR_NOERROR) {
		mmioClose(hmmio, 0);
		return false;
	}
	
	//fmtチャンクへ移動する
	ko_chunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
	if(mmioDescend(hmmio, &ko_chunk, &oya_chunk, MMIO_FINDCHUNK) != MMSYSERR_NOERROR) {
		mmioClose(hmmio, 0);
		return false;
	}
	
	//fmtチャンクを読み取ります。
	//WAVEFORMATEX構造体に必要な情報がコピーされます。
	//サウンドバッファを作る時に必要です。
	//画像で言えばまぁ、幅、高さ、ビット深度とかそこらへんを取得したんだと思ってください。
	WAVEFORMATEX wfex;
	if(mmioRead(hmmio, (HPSTR)&wfex, (LONG)ko_chunk.cksize) != (LONG)ko_chunk.cksize) {
		mmioClose(hmmio, 0);
		return false;
	}
	
	//fmtサブチャンクの外部に移動する
	//チャンクを潜ったので外に出ます。
	//ああ、なんてセンスの無い処理なんでしょう(^^;
	//もうちょっとスマートに出来るハズだろうと言ってやりたいですね。
	mmioAscend(hmmio, &ko_chunk, 0);
	
	//dataチャンクに移動
	ko_chunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
	if(mmioDescend(hmmio, &ko_chunk, &oya_chunk, MMIO_FINDCHUNK)) {
		mmioClose(hmmio, 0);
		return false;
	}
	
	//必要最小限のメモリを確保する
	LPBYTE pdata = (LPBYTE)malloc(ko_chunk.cksize);
	
	//dataチャンクを読み取る
	//ここが画像ファイルで言うと一番欲しい画像の部分ですね。
	//丸ごとごっそりいただく事にします。
	if(mmioRead(hmmio, (HPSTR)pdata, (LONG)ko_chunk.cksize) != (LONG)ko_chunk.cksize){
		free(pdata);
		mmioClose(hmmio, 0);
		return false;
	}
	
	//必要なデータがそろったので､ウェーブファイルを閉じる
	//センスの感じられない読み込みから脱出です。(^^)
	mmioClose(hmmio, 0);

//ここから下がサウンドバッファの作成工程になります。
	DSBUFFERDESC dsbdesk;
		
	//DirectXで構造体を使うような時のお約束ですね(^^)
	ZeroMemory(&dsbdesk, sizeof(DSBUFFERDESC));
	dsbdesk.dwSize = sizeof(DSBUFFERDESC);
	
	//サウンドバッファがどのようなバッファなのか指定します。
	//今回は必要最低限の物を選択しました。
	//DSBCAPS_CTRLPAN		パン操作が可能
	//						パンってのは簡単に言えば音の左右の調節の事です。
	//DSBCAPS_CTRLVOLUME	ボリュームの操作が可能
	//DSBCAPS_GLOBALFOCUS	アプリケーションがカレントを失っても
	//						サウンドバッファは失われません。
	//						このフラグはもっと深い意味がありますが
	//						まぁ、今回は説明しません。
	dsbdesk.dwFlags = DSBCAPS_CTRLPAN|DSBCAPS_CTRLVOLUME|DSBCAPS_GLOBALFOCUS ;

	//バッファのサイズを指定
	//これもさっき読みました。
	dsbdesk.dwBufferBytes = ko_chunk.cksize;

	//さっきwaveファイルから読み込んだ情報をここで教えてやります。
	dsbdesk.lpwfxFormat = &wfex; 
	
	for(int i=0;i<SOUNDBUFFER_NUM;i++)
	{
		//オフスクリーンバッファの生成
		if(FAILED(m_pDS->CreateSoundBuffer(&dsbdesk, &m_pdsb[i], NULL))){		
			return false;
		}

		//先ほど読み込んだwaveのデータ部分をサウンドバッファにコピーします。
		LPVOID pmem;
		DWORD size;
		
		//サウンドバッファをロック
		//ロックしたサイズとバッファの先頭アドレスが取得出来ます。
		m_pdsb[i]->Lock(0, ko_chunk.cksize, &pmem, &size, NULL, 0, 0);
		
		//読み込み済みのwaveデータをサウンドバッファにコピーします。
		//読み込んだサイズ分コピー
		memcpy(pmem, pdata, size);
		
		//バッファをアンロック
		//Drawの時と同じです。
		m_pdsb[i]->Unlock(pmem, size, NULL, 0);
	}
	
	//waveデータの読み込みに使った領域を開放します。
	//後始末は確りしましょう。
	free(pdata);

	return true;
}
/*
 *  FM-7 EMULATOR "XM7"  Copyright (C) 1999-2003
 * ＰＩ．(ytanaka@ipc-tokai.or.jp) Copyright (C) 2001-2003 Ryu
 * Takegami Copyright (C) 2004 GIMONS  [ XWIN
 * コンフィギュレーション ] 
 */  
    
#ifdef _XWIN
    
#ifndef _xw_cfg_h_
#define _xw_cfg_h_
    
#ifdef __cplusplus
extern          "C" {
 
#endif				/*  */
/*
 * ここで、configdat_t型を指定する
 * …ToolKitを外に追い出すための布石。
 */
        typedef struct {
                int  fm7_ver;	/* ハードウェアバージョン */
                BOOL cycle_steal;	/* サイクルスチールフラグ 
					 */
                DWORD main_speed;	/* メインCPUスピード */
                DWORD mmr_speed;	/* メインCPU(MMR)スピード */
                   
#if XM7_VER >= 3
                DWORD fmmr_speed;	/* メインCPU(高速MMR)スピード 
					 */
                   
#endif				/*  */
                DWORD sub_speed;	/* サブCPUスピード */
                DWORD uTimerResolution;	/* マルチメディアタイマー精度 
                                         */
                BOOL bTapeFull;	/* テープモータ時の速度フラグ */
                BOOL bCPUFull;	/* 全力駆動フラグ */
                BOOL bSpeedAdjust;	/* 自動速度調整 */
                BOOL bTapeMode;	/* テープモータ速度制御タイプ 
					 */
                int nSampleRate;	/* サンプリングレート */
                int nSoundBuffer;	/* サウンドバッファサイズ 
					 */
                int nBeepFreq;	/* BEEP周波数 */
                BOOL bFMHQmode;	/* FM高品質合成 */
                int nStereoOut;	/* 出力モード */
                BOOL bForceStereo;	/* 強制ステレオ出力 */
                BOOL bTapeMon;	/* テープ音モニタ */
                  
	// BYTE KeyMap[256]; /* キーマップ */
                BOOL bKbdReal;	/* 擬似リアルタイムキースキャン */
                BOOL bTenCursor;	/* 方向キーをテンキーに対応 
					 */
                BOOL bArrow8Dir;	/* テンキー変換
					 * 8方向モード */
                  
	/*
	 * JOYSTICK 
	 */            
                int nJoyType[2];
                int nJoyRapid[2][2];
                int nJoyCode[2][7];
                BOOL bFullScan;	/* 400ラインタイミングモード 
					 */
                BOOL bFullScanFS;
                WORD uWidth;	/* 表示サイズ(横) */
                WORD uHeight;	/* 表示サイズ(縦) */
                BOOL bOPNEnable;	/* OPN有効フラグ(7 only) */
                BOOL bWHGEnable;	/* WHG有効フラグ */
                BOOL bTHGEnable;	/* THG有効フラグ */
                BOOL bDigitizeEnable;	/* ディジタイズ有効フラグ */
#if ((XM7_VER >= 3) || defined(FMTV151))
                BOOL bExtRAMEnable;	/* 拡張RAM有効フラグ */
#endif				/*  */
#ifdef MOUSE
                BOOL bMouseCapture;	/* マウスキャプチャフラグ  */
                BYTE nMousePort;	/* マウス接続ポート */
                BYTE nMidBtnMode;	/* 中央ボタン状態取得モード */
                   
#endif				/*  */
                   
#ifdef FDDSND
                BOOL bFddWait;	/* FDDウェイト */
                BOOL bFddSound;	/* FDDシークサウンド */
         
#endif
                int nFMVolume;                                          /* FM音源ボリューム */
                int nPSGVolume;                                         /* PSGボリューム */
                int nBeepVolume;                                        /* BEEP音ボリューム */
                int nCMTVolume;                                         /* CMT音モニタボリューム */
                int nWaveVolume;                                        /* 各種効果音ボリューム */
                UINT uChSeparation;
        } configdat_t;

extern configdat_t configdat;	/* コンフィグ用データ */    
#define BST_CHECKED     TRUE
#define BST_UNCHECKED   FALSE
    
	/*
	 *  主要エントリ 
	 */ 
        void    LoadCfg(void);
                   
	/*
	 * 設定ロード 
	 */            
        void    SaveCfg(void);
                   
	/*
	 * 設定セーブ 
	 */           
        void    ApplyCfg(void);
                   
	/*
	 * 設定適用 
	 */            
        void    SetMachineVersion(void);
                   
	/*
	 * 動作機種再設定 
	 */            
#ifdef USE_GTK        
        void    OnConfig(GtkWidget *widget, gpointer data);
        void    OnCancelPressed(GtkWidget * widget, gpointer data);


                   
/*
 * 設定ダイアログ 
 */            
        void    OnConfig_OK(GtkWidget *widget, gpointer data);
        
/*
 * 設定ダイアログ(OKアクション) 
 */            
        void    OnGP_CPUDEFAULTClicked(GtkWidget *widget, gpointer data);
                   
/*
 * CPU Defaultボタンアクション 
 */            
	
        void    OnGP_CPUCOMBOChanged(GtkWidget *widget, gpointer data);
                   
/*
 * CPU コンボボックスアクション 
 */            
        void    OnGP_ChSepVolumeChanged(GtkWidget *widget, gpointer data);
        void    OnGP_FMVolumeChanged(GtkWidget *widget, gpointer data);
        void    OnGP_PSGVolumeChanged(GtkWidget *widget, gpointer data);
        void    OnGP_BEEPVolumeChanged(GtkWidget *widget, gpointer data);
        void    OnGP_CMTVolumeChanged(GtkWidget *widget, gpointer data);
        void    OnGP_WAVVolumeChanged(GtkWidget *widget, gpointer data);

#endif /* USE_GTK */	
	/*
	 *  主要ワーク 
	 */            
#ifdef __cplusplus
}             
#endif  /*  */
               
#endif	/* _xw_cfg_h_ */
#endif	/* _XWIN */

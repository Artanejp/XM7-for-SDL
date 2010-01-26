/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2010 ＰＩ．(yasushi@tanaka.net)
 *	Copyright (C) 2001-2010 Ryu Takegami
 *
 *	[ デバイス依存部 ]
 */

#ifndef _device_h_
#define _device_h_

/*
 *	定数定義
 */
#define OPEN_R		1					/* 読み込みモード */
#define OPEN_W		2					/* 書き込みモード */
#define OPEN_RW		3					/* 読み書きモード */

#define SOUND_STOP			255			/* サウンド停止 */
#define SOUND_CMTMOTORON	0			/* CMTモータ制御リレーON */
#define SOUND_CMTMOTOROFF	1			/* CMTモータ制御リレーOFF */
#define SOUND_FDDSEEK		2			/* FDDヘッドシーク */
#define SOUND_FDDHEADUP		3			/* FDDヘッドアップ(未使用) */
#define SOUND_FDDHEADDOWN	4			/* FDDヘッドダウン(未使用) */

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	主要エントリ
 */

/* 描画 */
void FASTCALL vram_notify(WORD addr, BYTE dat);
										/* VRAM書き込み通知 */
void FASTCALL tvram_notify(WORD addr, BYTE dat);
										/* テキストVRAM書き込み通知 */
void FASTCALL ttlpalet_notify(void);
										/* デジタルパレット変更通知 */
void FASTCALL apalet_notify(void);
										/* アナログパレット変更通知 */
void FASTCALL display_notify(void);
										/* 画面無効通知(グラフィック画面) */
void FASTCALL tvram_redraw_notify(void);
										/* 画面無効通知(テキスト画面) */
void FASTCALL digitize_notify(void);
										/* ディジタイズ通知 */
void FASTCALL vsync_notify(void);
										/* VSYNC通知 */
#if XM7_VER >= 3
void FASTCALL window_notify(void);
										/* ハードウェアウィンドウ変更通知 */
#endif

/* サウンド */
void FASTCALL opn_notify(BYTE reg, BYTE dat);
										/* OPN出力通知 */
void FASTCALL whg_notify(BYTE reg, BYTE dat);
										/* WHG出力通知 */
void FASTCALL thg_notify(BYTE reg, BYTE dat);
										/* THG出力通知 */
void FASTCALL beep_notify(void);
										/* BEEP出力通知 */
void FASTCALL tape_notify(BOOL flag);
										/* テープ出力通知 */
void FASTCALL midi_notify(BYTE mes);
										/* MIDI出力通知 */
void FASTCALL midi_reset_notify(void);
										/* MIDIリセット通知 */
void FASTCALL wav_notify(BYTE no);
										/* 各種効果音出力通知 */
void FASTCALL dac_notify(BYTE dat);
										/* DAC出力通知 */

/* ジョイスティック・マウス */
BYTE FASTCALL joy_request(BYTE port);
										/* ジョイスティック要求 */
#ifdef MOUSE
void FASTCALL mospos_request(BYTE *move_x, BYTE *move_y);
										/* マウス移動距離要求 */
BYTE FASTCALL mosbtn_request(void);
										/* マウスボタン状態要求 */
#endif

/* シリアル */
void FASTCALL rs232c_reset_notify(void);
										/* RS-232Cリセット通知 */
void FASTCALL rs232c_senddata(BYTE dat);
										/* RS-232Cデータ送信 */
BYTE FASTCALL rs232c_receivedata(void);
										/* RS-232Cデータ受信 */
BYTE FASTCALL rs232c_readstatus(void);
										/* ステータスレジスタリード */
void FASTCALL rs232c_writemodecmd(BYTE dat);
										/* モードコマンドレジスタライト */
void FASTCALL rs232c_writecommand(BYTE dat);
										/* コマンドレジスタライト */
void FASTCALL rs232c_setbaudrate(BYTE dat);
										/* ボーレートレジスタライト */

/* ファイル */
BOOL FASTCALL file_load(char *fname, BYTE *buf, int size);
										/* ファイルロード(ROM専用) */
BOOL FASTCALL file_load2(char *fname, BYTE *buf, int ptr, int size);
										/* 任意位置からのファイルロード */
BOOL FASTCALL file_save(char *fname, BYTE *buf, int size);
										/* ファイルセーブ(学習RAM専用) */
int FASTCALL file_open(char *fname, int mode);

										/* ファイルオープン */
void FASTCALL file_close(int handle);
										/* ファイルクローズ */
DWORD FASTCALL file_getsize(int handle);
										/* ファイルレングス取得 */
BOOL FASTCALL file_seek(int handle, DWORD offset);
										/* ファイルシーク */
BOOL FASTCALL file_read(int handle, BYTE *ptr, DWORD size);
										/* ファイル読み込み */
BOOL FASTCALL file_write(int handle, BYTE *ptr, DWORD size);
										/* ファイル書き込み */

/* ファイルサブ(プラットフォーム非依存。実体はsystem.cにある) */
BOOL FASTCALL file_byte_read(int handle, BYTE *dat);
										/* バイト読み込み */
BOOL FASTCALL file_word_read(int handle, WORD *dat);
										/* ワード読み込み */
BOOL FASTCALL file_dword_read(int handle, DWORD *dat);
										/* ダブルワード読み込み */
BOOL FASTCALL file_bool_read(int handle, BOOL *dat);
										/* ブール読み込み */
BOOL FASTCALL file_byte_write(int handle, BYTE dat);
										/* バイト書き込み */
BOOL FASTCALL file_word_write(int handle, WORD dat);
										/* ワード書き込み */
BOOL FASTCALL file_dword_write(int handle, DWORD dat);
										/* ダブルワード書き込み */
BOOL FASTCALL file_bool_write(int handle, BOOL dat);
										/* ブール書き込み */
#ifdef __cplusplus
}
#endif

#endif	/* _device_h_ */

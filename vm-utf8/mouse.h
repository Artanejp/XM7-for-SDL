/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2009 ＰＩ．(yasushi@tanaka.net)
 *	Copyright (C) 2001-2009 Ryu Takegami
 *
 *	[ インテリジェントマウス ]
 */

#ifdef MOUSE

#ifndef _mouse_h_
#define _mouse_h_

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	主要エントリ
 */
BOOL FASTCALL mos_init(void);
										/* 初期化 */
void FASTCALL mos_cleanup(void);
										/* クリーンアップ */
void FASTCALL mos_reset(void);
										/* リセット */
void FASTCALL mos_strobe_signal(BOOL strb);
										/* ストローブ信号処理 */
BYTE FASTCALL mos_readdata(BYTE trigger);
										/* データ読み込み */
BOOL FASTCALL mos_save(int fileh);
										/* セーブ */
BOOL FASTCALL mos_load(int fileh, int ver);
										/* ロード */

/*
 *	主要ワーク
 */
extern BYTE mos_port;
										/* マウス接続ポート */
extern BOOL mos_capture;
										/* マウスキャプチャフラグ */
#ifdef __cplusplus
}
#endif

#endif	/* _opn_h_ */

#endif	/* MOUSE */

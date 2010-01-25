/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2010 ＰＩ．(yasushi@tanaka.net)
 *	Copyright (C) 2001-2010 Ryu Takegami
 *
 *	[ イベントID ]
 */

#ifndef _event_h_
#define _event_h_

/*
 *	イベント定義
 */
#define EVENT_MAINTIMER			0		/* メインCPU 2.03msタイマ */
#define EVENT_SUBTIMER			1		/* サブCPU 20msタイマ */
#define EVENT_OPN_A				2		/* OPN タイマA */
#define EVENT_OPN_B				3		/* OPN タイマB */
#define EVENT_KEYBOARD			4		/* キーボード リピートタイマ */
#define EVENT_BEEP				5		/* BEEP音 単音タイマ */
#define EVENT_VSYNC				6		/* VSYNC */
#define EVENT_BLANK				7		/* BLANK */
#define EVENT_LINE				8		/* 直線補間LSI(V2/V3) */
#define	EVENT_TEXT_BLINK		8		/* テキストブリンキング(V1) */
#define EVENT_RTC				9		/* 時計 1sec(V2/V3) */
#define	EVENT_SUB_RESET			9		/* モード切り換えサブリセット(V1) */
#define EVENT_WHG_A				10		/* WHG タイマA */
#define EVENT_WHG_B				11		/* WHG タイマB */
#define EVENT_THG_A				12		/* THG タイマA */
#define EVENT_THG_B				13		/* THG タイマB */
#define EVENT_FDC_M				14		/* FDC マルチセクタ */
#define EVENT_FDC_L				15		/* FDC ロストデータ */
#define	EVENT_FDD_SEEK			16		/* FDD シーク動作ウェイト */
#define	EVENT_HOT_RESET			16		/* 旧ホットリセットイベント */
#define	EVENT_TAPEMON			17		/* テープ音声サンプリングタイマ */
#define	EVENT_MOUSE				18		/* マウスデータロスト */
#define	EVENT_FDC_DMA			19		/* FDC DMA転送開始(V3) */
#define	EVENT_FDC_NMI			19		/* FDC NMI(V1) */
#define	EVENT_KEYENC_ACK		20		/* キーエンコーダ ACK送信 */
#define	EVENT_RS_TXTIMING		21		/* RS-232C 送信タイミング */
#define	EVENT_RS_RXTIMING		22		/* RS-232C 受信タイミング */


#endif	/* _event_h_ */

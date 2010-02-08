/*
 *  FM-7 EMULATOR "XM7"
 *  Copyright (C) 1999-2003 ＰＩ．(ytanaka@ipc-tokai.or.jp)
 *  Copyright (C) 2001-2010 Ryu Takegami
 *  Copyright (C) 2004 GIMONS
 *  Copyright (C) 2010 K.Ohta  
 * [ XWIN/SDLキーボード設定プログラム - GTK+パート] 
 */ 
#ifndef _gtk_propkeyboard_h_
 
#ifdef __cplusplus
extern "C" {
#endif

        extern BOOL     SnoopedOnKeyPressedCallback(SDL_Event *event);
        //extern void     SnoopKeyEvent(BOOL flag);
        //extern void     StartGetKeycodeForProp(Uint16 keysym, BYTE code);
        extern void     StartGetKeycodeForProp(GtkWidget *widget, gpointer data);

#ifdef __cplusplus
}
#endif


#endif /* _gtk_propkeyboard_h_ */


/*
 * XM7 : 断片化VRAMでの書き込み
 * 2011 (C) K.Ohta <whatithis.sowhat@gmail.com>
 */

 void PutVram_AG_SP(SDL_Surface *p, int x, int y, int w, int h,  Uint32 mpage)
{
	int xx, yy;
	int hh, ww;
	int addr;
	int ofset;
	int size;
	Uint32 c[8];
	Uint32 *pp;

	// Test

    if(pVirtualVram == NULL) return;
    pp = &(pVirtualVram->pVram[0][0]);

    if(pp == NULL) return;
	if((vram_pb == NULL) || (vram_pg == NULL) || (vram_pr == NULL)) return;

    if(bClearFlag) {
        LockVram();
        memset(pp, 0x00, 640 * 400 * sizeof(Uint32)); // モードが変更されてるので仮想VRAMクリア
        bClearFlag = FALSE;
        UnlockVram();
    }
	switch (bMode) {
	case SCR_400LINE:
        CreateVirtualVram8(pp, x, y, w, h, bMode);
		break;
	case SCR_262144:
        CreateVirtualVram256k(pp, x, y, w, h, bMode, mpage);
		break;
	case SCR_4096:
        CreateVirtualVram4096(pp, x, y, w, h, bMode, mpage);
		break;
	case SCR_200LINE:
        CreateVirtualVram8(pp, x, y, w, h, bMode);
		break;
	}
}

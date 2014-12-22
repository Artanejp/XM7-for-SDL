// JULIET.H
// Programmed by ROMEOユーザー数名 / and GORRY.

#ifdef ROMEO

#ifdef __cplusplus
extern "C"
{
#endif

	BOOL juliet_load(void);
	void juliet_unload(void);

	BOOL juliet_prepare(void);

	void juliet_YM2151Reset(void);
	int juliet_YM2151IsEnable(void);
	int juliet_YM2151IsBusy(void);
	void juliet_YM2151Mute(BOOL mute);
	void juliet_YM2151W(BYTE addr, BYTE data);

// byうさ
	void juliet_YM2151BW(BYTE addr, BYTE data);
	void juliet_YM2151EXEC(DWORD Wait);

	void juliet_YMF288Reset(void);
	int juliet_YMF288Enable(void);
	int juliet_YMF288IsBusy(void);
	void juliet_YMF288Mute(BOOL mute);
	void juliet_YMF288A(BYTE addr, BYTE data);
	void juliet_YMF288B(BYTE addr, BYTE data);

// byうさ
	void juliet_YMF288A_B(BYTE addr, BYTE data);
	void juliet_YMF288B_B(BYTE addr, BYTE data);
	void juliet_YMF288EXEC(DWORD Wait);


	void juliet2_reset(void);
	void juliet2_sync(DWORD delayclock);
	void juliet2_exec(void);
	void juliet2_YM2151W(BYTE addr, BYTE data, DWORD clock);
	void juliet2_YMF288A(BYTE addr, BYTE data, DWORD clock);
	void juliet2_YMF288B(BYTE addr, BYTE data, DWORD clock);

#ifdef __cplusplus
}
#endif

#endif

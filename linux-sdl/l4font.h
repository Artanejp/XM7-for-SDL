/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2010 ＰＩ．(yasushi@tanaka.net)
 *	Copyright (C) 2001-2010 Ryu Takegami
 *
 *	[ 400ラインモード用内蔵フォント ]
 */

static const BYTE subcg_internal[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x78, 0x80, 0x80, 0x70, 0x08, 0x08, 0xF0, 0x00, 
	0x22, 0x22, 0x22, 0x3E, 0x22, 0x22, 0x22, 0x00, 
	0x78, 0x80, 0x80, 0x70, 0x08, 0x08, 0xF0, 0x00, 
	0x22, 0x22, 0x14, 0x08, 0x14, 0x22, 0x22, 0x00, 
	0xF8, 0x80, 0x80, 0xF0, 0x80, 0x80, 0xF8, 0x00, 
	0x22, 0x22, 0x14, 0x08, 0x14, 0x22, 0x22, 0x00, 
	0xF8, 0x80, 0x80, 0xF0, 0x80, 0x80, 0xF8, 0x00, 
	0x3E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 
	0xF8, 0x80, 0x80, 0xF0, 0x80, 0x80, 0xF8, 0x00, 
	0x1C, 0x22, 0x22, 0x22, 0x2A, 0x24, 0x1A, 0x00, 
	0x20, 0x20, 0x50, 0x50, 0xF8, 0x88, 0x88, 0x00, 
	0x22, 0x24, 0x28, 0x30, 0x28, 0x24, 0x22, 0x00, 
	0xF0, 0x88, 0x88, 0xF0, 0x88, 0x88, 0xF0, 0x00, 
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x3E, 0x00, 
	0xF0, 0x88, 0x88, 0xF0, 0x88, 0x88, 0xF0, 0x00, 
	0x1E, 0x20, 0x20, 0x1C, 0x02, 0x02, 0x3C, 0x00, 
	0x88, 0x88, 0x88, 0xF8, 0x88, 0x88, 0x88, 0x00, 
	0x3E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xF8, 0x00, 
	0x3E, 0x20, 0x20, 0x3C, 0x20, 0x20, 0x20, 0x00, 
	0x88, 0x88, 0x88, 0xF8, 0x88, 0x88, 0x88, 0x00, 
	0x22, 0x36, 0x2A, 0x22, 0x22, 0x22, 0x22, 0x00, 
	0x70, 0x88, 0x80, 0x80, 0x80, 0x88, 0x70, 0x00, 
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x3E, 0x00, 
	0x70, 0x88, 0x80, 0x80, 0x80, 0x88, 0x70, 0x00, 
	0x3C, 0x22, 0x22, 0x3C, 0x28, 0x24, 0x22, 0x00, 
	0x78, 0x80, 0x80, 0x70, 0x08, 0x08, 0xF0, 0x00, 
	0x1C, 0x22, 0x22, 0x22, 0x22, 0x22, 0x1C, 0x00, 
	0x78, 0x80, 0x80, 0x70, 0x08, 0x08, 0xF0, 0x00, 
	0x1C, 0x08, 0x08, 0x08, 0x08, 0x08, 0x1C, 0x00, 
	0xF0, 0x88, 0x88, 0x88, 0x88, 0x88, 0xF0, 0x00, 
	0x3E, 0x20, 0x20, 0x3C, 0x20, 0x20, 0x3E, 0x00, 
	0xF0, 0x88, 0x88, 0x88, 0x88, 0x88, 0xF0, 0x00, 
	0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 
	0xF0, 0x88, 0x88, 0x88, 0x88, 0x88, 0xF0, 0x00, 
	0x1C, 0x22, 0x02, 0x04, 0x08, 0x10, 0x3E, 0x00, 
	0xF0, 0x88, 0x88, 0x88, 0x88, 0x88, 0xF0, 0x00, 
	0x1C, 0x22, 0x02, 0x1C, 0x02, 0x22, 0x1C, 0x00, 
	0xF0, 0x88, 0x88, 0x88, 0x88, 0x88, 0xF0, 0x00, 
	0x04, 0x0C, 0x14, 0x14, 0x3E, 0x04, 0x04, 0x00, 
	0x88, 0x88, 0xC8, 0xA8, 0x98, 0x88, 0x88, 0x00, 
	0x22, 0x24, 0x28, 0x30, 0x28, 0x24, 0x22, 0x00, 
	0x78, 0x80, 0x80, 0x70, 0x08, 0x08, 0xF0, 0x00, 
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x3E, 0x00, 
	0xF8, 0x80, 0x80, 0xF0, 0x80, 0x80, 0xF8, 0x00, 
	0x3C, 0x22, 0x22, 0x3C, 0x22, 0x22, 0x3C, 0x00, 
	0x70, 0x88, 0x80, 0x80, 0x80, 0x88, 0x70, 0x00, 
	0x22, 0x22, 0x32, 0x2A, 0x26, 0x22, 0x22, 0x00, 
	0xF8, 0x80, 0x80, 0xF0, 0x80, 0x80, 0xF8, 0x00, 
	0x22, 0x36, 0x2A, 0x22, 0x22, 0x22, 0x22, 0x00, 
	0x78, 0x80, 0x80, 0x70, 0x08, 0x08, 0xF0, 0x00, 
	0x3C, 0x22, 0x22, 0x3C, 0x22, 0x22, 0x3C, 0x00, 
	0xF8, 0x80, 0x80, 0xF0, 0x80, 0x80, 0xF8, 0x00, 
	0x1C, 0x22, 0x20, 0x20, 0x20, 0x22, 0x1C, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x04, 
	0xFE, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x40, 
	0xFE, 0x40, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x10, 0x38, 0x54, 0x10, 0x10, 0x10, 
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 
	0x00, 0x00, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 
	0x10, 0x10, 0x10, 0x54, 0x38, 0x10, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 
	0x10, 0x00, 0x00, 0x00, 0x10, 0x10, 0x00, 0x00, 
	0x00, 0x6C, 0x6C, 0x24, 0x48, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x04, 0x24, 0x26, 0x3C, 0x64, 0x24, 0x24, 
	0x26, 0x3C, 0x64, 0x24, 0x24, 0x20, 0x00, 0x00, 
	0x00, 0x10, 0x10, 0x7C, 0x92, 0x90, 0x50, 0x38, 
	0x14, 0x12, 0x92, 0x7C, 0x10, 0x10, 0x00, 0x00, 
	0x00, 0x00, 0x40, 0xA0, 0xA2, 0x44, 0x08, 0x10, 
	0x20, 0x44, 0x8A, 0x0A, 0x04, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x70, 0x88, 0x80, 0x80, 0x80, 0x40, 
	0x60, 0x90, 0x8A, 0x84, 0x8C, 0x72, 0x00, 0x00, 
	0x00, 0x18, 0x18, 0x08, 0x10, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x02, 0x04, 0x08, 0x08, 0x10, 0x10, 0x10, 
	0x10, 0x10, 0x08, 0x08, 0x04, 0x02, 0x00, 0x00, 
	0x00, 0x80, 0x40, 0x20, 0x20, 0x10, 0x10, 0x10, 
	0x10, 0x10, 0x20, 0x20, 0x40, 0x80, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x10, 0x92, 0x54, 0x38, 
	0x54, 0x92, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x10, 0xFE, 
	0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x60, 0x60, 0x20, 0x40, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x60, 0x60, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x02, 0x04, 0x08, 0x10, 
	0x20, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x7C, 0x82, 0x82, 0x86, 0x8A, 0x8A, 0x92, 
	0xA2, 0xA2, 0xC2, 0x82, 0x82, 0x7C, 0x00, 0x00, 
	0x00, 0x10, 0x30, 0x50, 0x10, 0x10, 0x10, 0x10, 
	0x10, 0x10, 0x10, 0x10, 0x10, 0x7C, 0x00, 0x00, 
	0x00, 0x38, 0x44, 0x82, 0x82, 0x02, 0x04, 0x08, 
	0x10, 0x20, 0x40, 0x80, 0x80, 0xFE, 0x00, 0x00, 
	0x00, 0x38, 0x44, 0x82, 0x82, 0x02, 0x04, 0x38, 
	0x04, 0x02, 0x82, 0x82, 0x44, 0x38, 0x00, 0x00, 
	0x00, 0x04, 0x0C, 0x0C, 0x14, 0x14, 0x24, 0x24, 
	0x44, 0x44, 0xFE, 0x04, 0x04, 0x04, 0x00, 0x00, 
	0x00, 0xFE, 0x80, 0x80, 0x80, 0xB8, 0xC4, 0x82, 
	0x02, 0x02, 0x02, 0x82, 0x44, 0x38, 0x00, 0x00, 
	0x00, 0x38, 0x44, 0x82, 0x80, 0x80, 0xB8, 0xC4, 
	0x82, 0x82, 0x82, 0x82, 0x44, 0x38, 0x00, 0x00, 
	0x00, 0xFE, 0x82, 0x82, 0x04, 0x04, 0x08, 0x08, 
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 
	0x00, 0x38, 0x44, 0x82, 0x82, 0x82, 0x44, 0x38, 
	0x44, 0x82, 0x82, 0x82, 0x44, 0x38, 0x00, 0x00, 
	0x00, 0x38, 0x44, 0x82, 0x82, 0x82, 0x82, 0x46, 
	0x3A, 0x02, 0x02, 0x82, 0x44, 0x38, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 
	0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 
	0x00, 0x00, 0x18, 0x18, 0x08, 0x10, 0x00, 0x00, 
	0x00, 0x00, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 
	0x20, 0x10, 0x08, 0x04, 0x02, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 
	0x00, 0x7E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 
	0x04, 0x08, 0x10, 0x20, 0x40, 0x00, 0x00, 0x00, 
	0x00, 0x38, 0x44, 0x82, 0x82, 0x82, 0x44, 0x08, 
	0x10, 0x10, 0x00, 0x00, 0x10, 0x10, 0x00, 0x00, 
	0x00, 0x38, 0x44, 0x82, 0x92, 0xAA, 0xAA, 0xAA, 
	0xAA, 0xAA, 0x9C, 0x80, 0x42, 0x3C, 0x00, 0x00, 
	0x00, 0x10, 0x10, 0x28, 0x28, 0x28, 0x44, 0x44, 
	0x44, 0x7C, 0x82, 0x82, 0x82, 0x82, 0x00, 0x00, 
	0x00, 0xF8, 0x44, 0x42, 0x42, 0x42, 0x44, 0x78, 
	0x44, 0x42, 0x42, 0x42, 0x44, 0xF8, 0x00, 0x00, 
	0x00, 0x38, 0x44, 0x82, 0x82, 0x80, 0x80, 0x80, 
	0x80, 0x80, 0x82, 0x82, 0x44, 0x38, 0x00, 0x00, 
	0x00, 0xF8, 0x44, 0x42, 0x42, 0x42, 0x42, 0x42, 
	0x42, 0x42, 0x42, 0x42, 0x44, 0xF8, 0x00, 0x00, 
	0x00, 0xFE, 0x80, 0x80, 0x80, 0x80, 0x80, 0xFC, 
	0x80, 0x80, 0x80, 0x80, 0x80, 0xFE, 0x00, 0x00, 
	0x00, 0xFE, 0x80, 0x80, 0x80, 0x80, 0x80, 0xFC, 
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 
	0x00, 0x38, 0x44, 0x82, 0x82, 0x80, 0x80, 0x9E, 
	0x82, 0x82, 0x82, 0x82, 0x46, 0x3A, 0x00, 0x00, 
	0x00, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0xFE, 
	0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x00, 0x00, 
	0x00, 0x38, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 
	0x10, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00, 0x00, 
	0x00, 0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 
	0x04, 0x04, 0x04, 0x84, 0x88, 0x70, 0x00, 0x00, 
	0x00, 0x82, 0x82, 0x84, 0x88, 0x88, 0x90, 0xE0, 
	0x90, 0x88, 0x88, 0x84, 0x82, 0x82, 0x00, 0x00, 
	0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
	0x80, 0x80, 0x80, 0x80, 0x80, 0xFE, 0x00, 0x00, 
	0x00, 0x82, 0x82, 0xC6, 0xC6, 0xAA, 0xAA, 0x92, 
	0x92, 0x82, 0x82, 0x82, 0x82, 0x82, 0x00, 0x00, 
	0x00, 0x82, 0xC2, 0xC2, 0xA2, 0xA2, 0x92, 0x92, 
	0x8A, 0x8A, 0x86, 0x86, 0x82, 0x82, 0x00, 0x00, 
	0x00, 0x38, 0x44, 0x82, 0x82, 0x82, 0x82, 0x82, 
	0x82, 0x82, 0x82, 0x82, 0x44, 0x38, 0x00, 0x00, 
	0x00, 0xF8, 0x84, 0x82, 0x82, 0x82, 0x84, 0xF8, 
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 
	0x00, 0x38, 0x44, 0x82, 0x82, 0x82, 0x82, 0x82, 
	0x82, 0x82, 0xBA, 0xC6, 0x44, 0x3A, 0x00, 0x00, 
	0x00, 0xF8, 0x84, 0x82, 0x82, 0x82, 0x84, 0xF8, 
	0x88, 0x88, 0x84, 0x84, 0x82, 0x82, 0x00, 0x00, 
	0x00, 0x78, 0x84, 0x80, 0x80, 0x40, 0x20, 0x10, 
	0x08, 0x04, 0x02, 0x02, 0x82, 0x7C, 0x00, 0x00, 
	0x00, 0xFE, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 
	0x00, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 
	0x82, 0x82, 0x82, 0x82, 0x44, 0x38, 0x00, 0x00, 
	0x00, 0x82, 0x82, 0x82, 0x82, 0x44, 0x44, 0x44, 
	0x44, 0x28, 0x28, 0x28, 0x10, 0x10, 0x00, 0x00, 
	0x00, 0x82, 0x92, 0x92, 0x92, 0x92, 0xAA, 0xAA, 
	0xAA, 0xAA, 0x44, 0x44, 0x44, 0x44, 0x00, 0x00, 
	0x00, 0x82, 0x82, 0x44, 0x44, 0x28, 0x28, 0x10, 
	0x28, 0x28, 0x44, 0x44, 0x82, 0x82, 0x00, 0x00, 
	0x00, 0x82, 0x82, 0x44, 0x44, 0x28, 0x28, 0x10, 
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 
	0x00, 0xFE, 0x02, 0x04, 0x04, 0x08, 0x08, 0x10, 
	0x20, 0x20, 0x40, 0x40, 0x80, 0xFE, 0x00, 0x00, 
	0x00, 0x1E, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 
	0x10, 0x10, 0x10, 0x10, 0x10, 0x1E, 0x00, 0x00, 
	0x00, 0x82, 0x82, 0x44, 0x44, 0x28, 0xFE, 0x10, 
	0x10, 0xFE, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 
	0x00, 0xF0, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 
	0x10, 0x10, 0x10, 0x10, 0x10, 0xF0, 0x00, 0x00, 
	0x00, 0x10, 0x28, 0x44, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0x00, 0x00, 
	0x00, 0x20, 0x30, 0x18, 0x08, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x04, 
	0x3A, 0x46, 0x82, 0x82, 0x46, 0x3A, 0x00, 0x00, 
	0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0xB8, 0xC4, 
	0x82, 0x82, 0x82, 0x82, 0xC4, 0xB8, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x44, 
	0x82, 0x80, 0x80, 0x82, 0x44, 0x38, 0x00, 0x00, 
	0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x3A, 0x46, 
	0x82, 0x82, 0x82, 0x82, 0x46, 0x3A, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x44, 
	0x82, 0xFE, 0x80, 0x82, 0x44, 0x38, 0x00, 0x00, 
	0x00, 0x0C, 0x12, 0x20, 0x20, 0x20, 0xFC, 0x20, 
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3A, 0x46, 
	0x82, 0x82, 0x82, 0x46, 0x3A, 0x02, 0x44, 0x38, 
	0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0xB8, 0xC4, 
	0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x00, 0x00, 
	0x00, 0x00, 0x10, 0x10, 0x00, 0x00, 0x30, 0x10, 
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 
	0x00, 0x00, 0x04, 0x04, 0x00, 0x00, 0x0C, 0x04, 
	0x04, 0x04, 0x04, 0x04, 0x04, 0x44, 0x48, 0x30, 
	0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x84, 0x88, 
	0x90, 0xA0, 0xD0, 0x88, 0x84, 0x82, 0x00, 0x00, 
	0x00, 0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEC, 0x92, 
	0x92, 0x92, 0x92, 0x92, 0x92, 0x92, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB8, 0xC4, 
	0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x44, 
	0x82, 0x82, 0x82, 0x82, 0x44, 0x38, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB8, 0xC4, 
	0x82, 0x82, 0x82, 0xC4, 0xB8, 0x80, 0x80, 0x80, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3A, 0x46, 
	0x82, 0x82, 0x82, 0x46, 0x3A, 0x02, 0x02, 0x02, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9C, 0xA2, 
	0xC0, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x84, 
	0x40, 0x30, 0x0C, 0x02, 0x82, 0x7C, 0x00, 0x00, 
	0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0xFC, 0x20, 
	0x20, 0x20, 0x20, 0x20, 0x22, 0x1C, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x82, 0x82, 
	0x82, 0x82, 0x82, 0x82, 0x46, 0x3A, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x82, 0x82, 
	0x44, 0x44, 0x28, 0x28, 0x10, 0x10, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x82, 0x92, 
	0x92, 0xAA, 0xAA, 0x44, 0x44, 0x44, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x82, 0x44, 
	0x28, 0x10, 0x10, 0x28, 0x44, 0x82, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x82, 0x82, 
	0x44, 0x44, 0x28, 0x28, 0x10, 0x10, 0x20, 0xC0, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0x04, 
	0x08, 0x10, 0x20, 0x40, 0x80, 0xFE, 0x00, 0x00, 
	0x00, 0x06, 0x08, 0x08, 0x08, 0x08, 0x08, 0x10, 
	0x08, 0x08, 0x08, 0x08, 0x08, 0x06, 0x00, 0x00, 
	0x00, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 
	0x00, 0xC0, 0x20, 0x20, 0x20, 0x20, 0x20, 0x10, 
	0x20, 0x20, 0x20, 0x20, 0x20, 0xC0, 0x00, 0x00, 
	0x00, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xF0, 0x88, 0x88, 0x88, 0x88, 0xF0, 0x00, 
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x3E, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
	0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 
	0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 
	0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 
	0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 
	0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 
	0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 
	0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 
	0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 
	0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 
	0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 
	0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 
	0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 
	0xFF, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 
	0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0xFF, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 
	0xF0, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 
	0x1F, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 
	0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x1F, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0xF0, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 
	0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 
	0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x03, 0x04, 0x08, 0x10, 0x10, 0x20, 0x20, 0x20, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x80, 0x40, 0x20, 0x10, 0x10, 0x08, 0x08, 0x08, 
	0x20, 0x20, 0x20, 0x20, 0x10, 0x10, 0x08, 0x04, 
	0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x08, 0x08, 0x08, 0x08, 0x10, 0x10, 0x20, 0x40, 
	0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x60, 0x90, 0x90, 0x60, 0x00, 0x00, 
	0x00, 0x1E, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 
	0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 
	0x10, 0x10, 0x10, 0x10, 0x10, 0xF0, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x80, 0x40, 0x20, 0x10, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 
	0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xFE, 0x02, 0x02, 0x02, 0xFE, 0x02, 
	0x02, 0x02, 0x04, 0x04, 0x18, 0x60, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0x02, 
	0x14, 0x18, 0x10, 0x20, 0x20, 0x40, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x04, 0x08, 
	0x18, 0x68, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0xFE, 0x82, 
	0x82, 0x82, 0x04, 0x04, 0x08, 0x30, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0x10, 
	0x10, 0x10, 0x10, 0x10, 0x10, 0xFE, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x04, 0x04, 0xFE, 0x04, 
	0x0C, 0x14, 0x24, 0x44, 0x84, 0x0C, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x20, 0x20, 0x3E, 0xE2, 
	0x22, 0x24, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0x04, 
	0x04, 0x04, 0x04, 0x04, 0x04, 0xFE, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0x02, 
	0x02, 0xFE, 0x02, 0x02, 0x02, 0xFE, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x92, 0x92, 
	0x92, 0x02, 0x04, 0x04, 0x18, 0x60, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x7E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xFE, 0x02, 0x02, 0x14, 0x18, 0x10, 
	0x10, 0x10, 0x10, 0x20, 0x20, 0x40, 0x00, 0x00, 
	0x00, 0x02, 0x02, 0x04, 0x04, 0x08, 0x18, 0x28, 
	0xC8, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00, 
	0x00, 0x10, 0x10, 0xFE, 0x82, 0x82, 0x82, 0x02, 
	0x02, 0x02, 0x04, 0x04, 0x08, 0x30, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xFE, 0x10, 0x10, 0x10, 0x10, 
	0x10, 0x10, 0x10, 0x10, 0x10, 0xFE, 0x00, 0x00, 
	0x00, 0x04, 0x04, 0xFE, 0x0C, 0x0C, 0x14, 0x14, 
	0x24, 0x24, 0x44, 0x84, 0x04, 0x0C, 0x00, 0x00, 
	0x00, 0x10, 0x10, 0xFE, 0x12, 0x12, 0x12, 0x12, 
	0x12, 0x22, 0x22, 0x42, 0x42, 0x84, 0x00, 0x00, 
	0x00, 0x10, 0x10, 0x1E, 0xF0, 0x10, 0x10, 0x10, 
	0x1E, 0xE8, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00, 
	0x00, 0x20, 0x20, 0x3E, 0x42, 0x42, 0x82, 0x02, 
	0x02, 0x04, 0x04, 0x08, 0x10, 0x60, 0x00, 0x00, 
	0x00, 0x40, 0x40, 0x7E, 0x48, 0x48, 0x88, 0x08, 
	0x08, 0x08, 0x10, 0x10, 0x20, 0x40, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xFE, 0x02, 0x02, 0x02, 0x02, 
	0x02, 0x02, 0x02, 0x02, 0x02, 0xFE, 0x00, 0x00, 
	0x00, 0x44, 0x44, 0x44, 0xFE, 0x44, 0x44, 0x44, 
	0x04, 0x04, 0x08, 0x08, 0x10, 0x60, 0x00, 0x00, 
	0x00, 0x00, 0x60, 0x10, 0x02, 0xC2, 0x22, 0x02, 
	0x02, 0x04, 0x04, 0x08, 0x10, 0xE0, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xFE, 0x02, 0x02, 0x04, 0x04, 
	0x08, 0x08, 0x10, 0x28, 0x44, 0x82, 0x00, 0x00, 
	0x00, 0x40, 0x40, 0xFE, 0x42, 0x42, 0x44, 0x48, 
	0x40, 0x40, 0x40, 0x40, 0x20, 0x1E, 0x00, 0x00, 
	0x00, 0x00, 0x82, 0x42, 0x42, 0x22, 0x22, 0x02, 
	0x02, 0x04, 0x04, 0x08, 0x10, 0x60, 0x00, 0x00, 
	0x00, 0x20, 0x20, 0x3E, 0x42, 0x42, 0xA2, 0x12, 
	0x0A, 0x04, 0x04, 0x08, 0x10, 0x60, 0x00, 0x00, 
	0x00, 0x00, 0x0C, 0x70, 0x10, 0x10, 0x10, 0xFE, 
	0x10, 0x10, 0x10, 0x20, 0x20, 0x40, 0x00, 0x00, 
	0x00, 0x00, 0x92, 0x92, 0x92, 0x92, 0x02, 0x02, 
	0x02, 0x04, 0x04, 0x08, 0x10, 0x60, 0x00, 0x00, 
	0x00, 0x00, 0x7C, 0x00, 0x00, 0x00, 0xFE, 0x10, 
	0x10, 0x10, 0x10, 0x20, 0x20, 0x40, 0x00, 0x00, 
	0x00, 0x40, 0x40, 0x40, 0x40, 0x40, 0x60, 0x50, 
	0x48, 0x44, 0x40, 0x40, 0x40, 0x40, 0x00, 0x00, 
	0x00, 0x10, 0x10, 0x10, 0xFE, 0x10, 0x10, 0x10, 
	0x10, 0x10, 0x10, 0x20, 0x20, 0x40, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x7C, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xFE, 0x02, 0x02, 0x04, 0x04, 
	0x48, 0x28, 0x10, 0x28, 0x44, 0x80, 0x00, 0x00, 
	0x00, 0x10, 0x10, 0xFE, 0x02, 0x04, 0x04, 0x08, 
	0x10, 0x34, 0xD2, 0x10, 0x10, 0x10, 0x00, 0x00, 
	0x00, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x04, 
	0x04, 0x08, 0x08, 0x10, 0x20, 0xC0, 0x00, 0x00, 
	0x00, 0x00, 0x08, 0x04, 0x02, 0x42, 0x42, 0x42, 
	0x42, 0x42, 0x42, 0x42, 0x42, 0x82, 0x00, 0x00, 
	0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0xFE, 0x80, 
	0x80, 0x80, 0x80, 0x80, 0x80, 0x7E, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xFE, 0x02, 0x02, 0x02, 0x02, 
	0x02, 0x04, 0x04, 0x08, 0x10, 0x60, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x20, 0x50, 0x50, 0x88, 0x88, 
	0x04, 0x04, 0x04, 0x02, 0x02, 0x02, 0x00, 0x00, 
	0x00, 0x10, 0x10, 0x10, 0xFE, 0x10, 0x10, 0x54, 
	0x54, 0x52, 0x92, 0x10, 0x10, 0x10, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xFE, 0x02, 0x02, 0x02, 0x04, 
	0x04, 0x48, 0x28, 0x10, 0x08, 0x04, 0x00, 0x00, 
	0x00, 0x00, 0x70, 0x0C, 0x00, 0x00, 0x00, 0x70, 
	0x0C, 0x00, 0x00, 0x00, 0xF8, 0x06, 0x00, 0x00, 
	0x00, 0x00, 0x10, 0x10, 0x10, 0x20, 0x20, 0x20, 
	0x44, 0x44, 0x44, 0x84, 0x9A, 0xE2, 0x00, 0x00, 
	0x00, 0x00, 0x02, 0x02, 0x02, 0x02, 0x04, 0x24, 
	0x14, 0x08, 0x0C, 0x12, 0x20, 0xC0, 0x00, 0x00, 
	0x00, 0x00, 0xFE, 0x20, 0x20, 0x20, 0x20, 0xFE, 
	0x20, 0x20, 0x20, 0x20, 0x20, 0x1E, 0x00, 0x00, 
	0x00, 0x20, 0x20, 0x3E, 0xE2, 0x22, 0x24, 0x20, 
	0x20, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x7C, 0x04, 0x04, 0x04, 0x04, 
	0x04, 0x04, 0x04, 0x04, 0x04, 0xFE, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xFE, 0x02, 0x02, 0x02, 0x02, 
	0xFE, 0x02, 0x02, 0x02, 0x02, 0xFE, 0x00, 0x00, 
	0x00, 0x00, 0x7C, 0x00, 0x00, 0xFE, 0x02, 0x02, 
	0x02, 0x04, 0x04, 0x08, 0x10, 0x60, 0x00, 0x00, 
	0x00, 0x02, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 
	0x02, 0x02, 0x04, 0x04, 0x08, 0x30, 0x00, 0x00, 
	0x00, 0x00, 0x10, 0x50, 0x50, 0x50, 0x50, 0x50, 
	0x50, 0x52, 0x52, 0x54, 0x58, 0x90, 0x00, 0x00, 
	0x00, 0x00, 0x40, 0x40, 0x40, 0x40, 0x40, 0x42, 
	0x42, 0x44, 0x44, 0x48, 0x50, 0x60, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xFE, 0x82, 0x82, 0x82, 0x82, 
	0x82, 0x82, 0x82, 0x82, 0x82, 0xFE, 0x00, 0x00, 
	0x00, 0x00, 0xFE, 0x82, 0x82, 0x82, 0x02, 0x02, 
	0x02, 0x02, 0x04, 0x04, 0x08, 0x30, 0x00, 0x00, 
	0x00, 0x00, 0xC0, 0x22, 0x02, 0x02, 0x02, 0x02, 
	0x02, 0x04, 0x04, 0x08, 0x10, 0xE0, 0x00, 0x00, 
	0x00, 0x20, 0x90, 0x40, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x60, 0x90, 0x90, 0x60, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 
	0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x10, 0x10, 0x10, 0x10, 0x10, 0x1F, 0x10, 0x10, 
	0x10, 0x10, 0x1F, 0x10, 0x10, 0x10, 0x10, 0x10, 
	0x10, 0x10, 0x10, 0x10, 0x10, 0xFF, 0x10, 0x10, 
	0x10, 0x10, 0xFF, 0x10, 0x10, 0x10, 0x10, 0x10, 
	0x08, 0x08, 0x08, 0x08, 0x08, 0xF8, 0x08, 0x08, 
	0x08, 0x08, 0xF8, 0x08, 0x08, 0x08, 0x08, 0x08, 
	0x01, 0x01, 0x03, 0x03, 0x07, 0x07, 0x0F, 0x0F, 
	0x1F, 0x1F, 0x3F, 0x3F, 0x7F, 0x7F, 0xFF, 0xFF, 
	0x80, 0x80, 0xC0, 0xC0, 0xE0, 0xE0, 0xF0, 0xF0, 
	0xF8, 0xF8, 0xFC, 0xFC, 0xFE, 0xFE, 0xFF, 0xFF, 
	0xFF, 0xFF, 0x7F, 0x7F, 0x3F, 0x3F, 0x1F, 0x1F, 
	0x0F, 0x0F, 0x07, 0x07, 0x03, 0x03, 0x01, 0x01, 
	0xFF, 0xFF, 0xFE, 0xFE, 0xFC, 0xFC, 0xF8, 0xF8, 
	0xF0, 0xF0, 0xE0, 0xE0, 0xC0, 0xC0, 0x80, 0x80, 
	0x00, 0x10, 0x38, 0x38, 0x7C, 0x7C, 0xFE, 0xFE, 
	0xFE, 0xFE, 0x7C, 0x10, 0x38, 0x7C, 0x00, 0x00, 
	0x00, 0x6C, 0x6C, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 
	0x7C, 0x7C, 0x7C, 0x38, 0x38, 0x10, 0x00, 0x00, 
	0x00, 0x10, 0x10, 0x38, 0x38, 0x7C, 0x7C, 0xFE, 
	0x7C, 0x7C, 0x38, 0x38, 0x10, 0x10, 0x00, 0x00, 
	0x00, 0x00, 0x10, 0x38, 0x38, 0x38, 0x54, 0xFE, 
	0xFE, 0xFE, 0x54, 0x10, 0x38, 0x7C, 0x00, 0x00, 
	0x00, 0x00, 0x38, 0x7C, 0x7C, 0xFE, 0xFE, 0xFE, 
	0xFE, 0xFE, 0xFE, 0x7C, 0x7C, 0x38, 0x00, 0x00, 
	0x00, 0x00, 0x38, 0x44, 0x44, 0x82, 0x82, 0x82, 
	0x82, 0x82, 0x82, 0x44, 0x44, 0x38, 0x00, 0x00, 
	0x01, 0x01, 0x02, 0x02, 0x04, 0x04, 0x08, 0x08, 
	0x10, 0x10, 0x20, 0x20, 0x40, 0x40, 0x80, 0x80, 
	0x80, 0x80, 0x40, 0x40, 0x20, 0x20, 0x10, 0x10, 
	0x08, 0x08, 0x04, 0x04, 0x02, 0x02, 0x01, 0x01, 
	0x81, 0x81, 0x42, 0x42, 0x24, 0x24, 0x18, 0x18, 
	0x18, 0x18, 0x24, 0x24, 0x42, 0x42, 0x81, 0x81, 
	0x00, 0x00, 0x00, 0xFE, 0x92, 0x92, 0x92, 0xFE, 
	0x82, 0x82, 0x82, 0x82, 0x82, 0x86, 0x00, 0x00, 
	0x00, 0x00, 0x40, 0x40, 0x7E, 0x88, 0x88, 0x7E, 
	0x48, 0x48, 0xFE, 0x08, 0x08, 0x08, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x7E, 0x42, 0x42, 0x7E, 0x42, 
	0x42, 0x7E, 0x42, 0x42, 0x82, 0x86, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x7E, 0x42, 0x42, 0x42, 0x42, 
	0x7E, 0x42, 0x42, 0x42, 0x42, 0x7E, 0x00, 0x00, 
	0x00, 0x00, 0x02, 0x02, 0xEF, 0xA2, 0xBF, 0xA2, 
	0xFF, 0xA2, 0xAA, 0xAA, 0xE2, 0x06, 0x00, 0x00, 
	0x00, 0x00, 0x2C, 0x24, 0x24, 0x42, 0x42, 0xBD, 
	0x95, 0x14, 0x14, 0x24, 0x24, 0x4C, 0x00, 0x00, 
	0x00, 0x00, 0x24, 0x44, 0xD5, 0x55, 0x55, 0xE4, 
	0x4D, 0xE1, 0xE2, 0xC2, 0x44, 0x48, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xFE, 0x00, 0x00, 0xFE, 0x10, 
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 
	0x00, 0x00, 0x10, 0x10, 0xFE, 0x10, 0x10, 0xFE, 
	0x92, 0x92, 0x92, 0x96, 0x10, 0x10, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xFE, 0x80, 0x84, 0xA4, 0x98, 
	0x88, 0x94, 0xA0, 0x80, 0xFE, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xF8, 0xA8, 0xAF, 0xAA, 0xFA, 0xAA, 
	0xAA, 0xAA, 0xFA, 0x02, 0x02, 0x06, 0x00, 0x00, 
	0x00, 0x00, 0x22, 0x22, 0x22, 0xFF, 0x22, 0x72, 
	0x6A, 0xAA, 0xAA, 0xA2, 0x22, 0x26, 0x00, 0x00, 
	0x00, 0x00, 0x10, 0x10, 0x10, 0x10, 0x10, 0x20, 
	0x30, 0x28, 0x48, 0x44, 0x84, 0x82, 0x00, 0x00, 
	0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 
	0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};
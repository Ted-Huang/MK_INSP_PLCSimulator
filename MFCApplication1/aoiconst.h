#pragma once
#pragma pack(push, 1)
//---------------------------------------------------------------
// For UTF file format,can't use char > 128 for Zh-TW System
// unsigned char NewParam = 129;   (<== C2001 Error in OS(TW),but OS(ENG) Success)
// unsigned char NewParam = (unsigned char)129; (always success)
//										Eric Chao 20150804
//---------------------------------------------------------------

#define SYSBTN_HEIGHT       65
#define INFODLG_HEIGHT      112
#define SLAVECTRLDLG_HEIGHT 500
#define SHEET_WEIGHT        130

const COLORREF CLR_BACKGROUND1 = RGB(0x30, 0x30, 0x30);
const COLORREF CLR_BACKGROUND2 = RGB(0xd9, 0xd9, 0xd9);
const COLORREF CLR_BACKGROUND3 = RGB(0xcc, 0xcc, 0xcc);
const COLORREF CLR_BACKGROUND4 = RGB( 0x49, 0x49, 0x49 );
const COLORREF CLR_WHITE = RGB(0xff, 0xff, 0xff);
const COLORREF CLR_WHITE2 = RGB(0xf0, 0xf0, 0xf0);
const COLORREF CLR_SELECTED = RGB(0xa9, 0xa9, 0xa9);
const COLORREF CLR_FONT = RGB(0x5d, 0x5d, 0x5d);
const COLORREF CLR_FONT2 = RGB(0x80, 0x80, 0x80);
const COLORREF CLR_HOVER = RGB(0x69, 0x69, 0x69);
const COLORREF CLR_BLUE = RGB(0x00, 0x5c, 0xb9);
const COLORREF CLR_DARK = RGB(0x49, 0x49, 0x49);
const COLORREF CLR_ORANGE = RGB(0xff, 0x63, 0x47);
const COLORREF CLR_RED = RGB(0xff, 0, 0);

const BYTE ctMonoColorTable[1024]={
	(BYTE)0x00,(BYTE)0x00,(BYTE)0x00,(BYTE)0x00,(BYTE)0x01,(BYTE)0x01,(BYTE)0x01,(BYTE)0x00,(BYTE)0x02,(BYTE)0x02,(BYTE)0x02,(BYTE)0x00,(BYTE)0x03,(BYTE)0x03,(BYTE)0x03,(BYTE)0x00,(BYTE)0x04,(BYTE)0x04,(BYTE)0x04,(BYTE)0x00,(BYTE)0x05,(BYTE)0x05,(BYTE)0x05,(BYTE)0x00,(BYTE)0x06,(BYTE)0x06,(BYTE)0x06,(BYTE)0x00,(BYTE)0x07,(BYTE)0x07,(BYTE)0x07,(BYTE)0x00,(BYTE)0x08,(BYTE)0x08,(BYTE)0x08,(BYTE)0x00,(BYTE)0x09,(BYTE)0x09,(BYTE)0x09,(BYTE)0x00,(BYTE)0x0A,(BYTE)0x0A,(BYTE)0x0A,(BYTE)0x00,(BYTE)0x0B,(BYTE)0x0B,(BYTE)0x0B,(BYTE)0x00,(BYTE)0x0C,(BYTE)0x0C,(BYTE)0x0C,(BYTE)0x00,(BYTE)0x0D,(BYTE)0x0D,(BYTE)0x0D,(BYTE)0x00,(BYTE)0x0E,(BYTE)0x0E,(BYTE)0x0E,(BYTE)0x00,(BYTE)0x0F,(BYTE)0x0F,(BYTE)0x0F,(BYTE)0x00,\
	(BYTE)0x10,(BYTE)0x10,(BYTE)0x10,(BYTE)0x00,(BYTE)0x11,(BYTE)0x11,(BYTE)0x11,(BYTE)0x00,(BYTE)0x12,(BYTE)0x12,(BYTE)0x12,(BYTE)0x00,(BYTE)0x13,(BYTE)0x13,(BYTE)0x13,(BYTE)0x00,(BYTE)0x14,(BYTE)0x14,(BYTE)0x14,(BYTE)0x00,(BYTE)0x15,(BYTE)0x15,(BYTE)0x15,(BYTE)0x00,(BYTE)0x16,(BYTE)0x16,(BYTE)0x16,(BYTE)0x00,(BYTE)0x17,(BYTE)0x17,(BYTE)0x17,(BYTE)0x00,(BYTE)0x18,(BYTE)0x18,(BYTE)0x18,(BYTE)0x00,(BYTE)0x19,(BYTE)0x19,(BYTE)0x19,(BYTE)0x00,(BYTE)0x1A,(BYTE)0x1A,(BYTE)0x1A,(BYTE)0x00,(BYTE)0x1B,(BYTE)0x1B,(BYTE)0x1B,(BYTE)0x00,(BYTE)0x1C,(BYTE)0x1C,(BYTE)0x1C,(BYTE)0x00,(BYTE)0x1D,(BYTE)0x1D,(BYTE)0x1D,(BYTE)0x00,(BYTE)0x1E,(BYTE)0x1E,(BYTE)0x1E,(BYTE)0x00,(BYTE)0x1F,(BYTE)0x1F,(BYTE)0x1F,(BYTE)0x00,\
	(BYTE)0x20,(BYTE)0x20,(BYTE)0x20,(BYTE)0x00,(BYTE)0x21,(BYTE)0x21,(BYTE)0x21,(BYTE)0x00,(BYTE)0x22,(BYTE)0x22,(BYTE)0x22,(BYTE)0x00,(BYTE)0x23,(BYTE)0x23,(BYTE)0x23,(BYTE)0x00,(BYTE)0x24,(BYTE)0x24,(BYTE)0x24,(BYTE)0x00,(BYTE)0x25,(BYTE)0x25,(BYTE)0x25,(BYTE)0x00,(BYTE)0x26,(BYTE)0x26,(BYTE)0x26,(BYTE)0x00,(BYTE)0x27,(BYTE)0x27,(BYTE)0x27,(BYTE)0x00,(BYTE)0x28,(BYTE)0x28,(BYTE)0x28,(BYTE)0x00,(BYTE)0x29,(BYTE)0x29,(BYTE)0x29,(BYTE)0x00,(BYTE)0x2A,(BYTE)0x2A,(BYTE)0x2A,(BYTE)0x00,(BYTE)0x2B,(BYTE)0x2B,(BYTE)0x2B,(BYTE)0x00,(BYTE)0x2C,(BYTE)0x2C,(BYTE)0x2C,(BYTE)0x00,(BYTE)0x2D,(BYTE)0x2D,(BYTE)0x2D,(BYTE)0x00,(BYTE)0x2E,(BYTE)0x2E,(BYTE)0x2E,(BYTE)0x00,(BYTE)0x2F,(BYTE)0x2F,(BYTE)0x2F,(BYTE)0x00,\
	(BYTE)0x30,(BYTE)0x30,(BYTE)0x30,(BYTE)0x00,(BYTE)0x31,(BYTE)0x31,(BYTE)0x31,(BYTE)0x00,(BYTE)0x32,(BYTE)0x32,(BYTE)0x32,(BYTE)0x00,(BYTE)0x33,(BYTE)0x33,(BYTE)0x33,(BYTE)0x00,(BYTE)0x34,(BYTE)0x34,(BYTE)0x34,(BYTE)0x00,(BYTE)0x35,(BYTE)0x35,(BYTE)0x35,(BYTE)0x00,(BYTE)0x36,(BYTE)0x36,(BYTE)0x36,(BYTE)0x00,(BYTE)0x37,(BYTE)0x37,(BYTE)0x37,(BYTE)0x00,(BYTE)0x38,(BYTE)0x38,(BYTE)0x38,(BYTE)0x00,(BYTE)0x39,(BYTE)0x39,(BYTE)0x39,(BYTE)0x00,(BYTE)0x3A,(BYTE)0x3A,(BYTE)0x3A,(BYTE)0x00,(BYTE)0x3B,(BYTE)0x3B,(BYTE)0x3B,(BYTE)0x00,(BYTE)0x3C,(BYTE)0x3C,(BYTE)0x3C,(BYTE)0x00,(BYTE)0x3D,(BYTE)0x3D,(BYTE)0x3D,(BYTE)0x00,(BYTE)0x3E,(BYTE)0x3E,(BYTE)0x3E,(BYTE)0x00,(BYTE)0x3F,(BYTE)0x3F,(BYTE)0x3F,(BYTE)0x00,\
	(BYTE)0x40,(BYTE)0x40,(BYTE)0x40,(BYTE)0x00,(BYTE)0x41,(BYTE)0x41,(BYTE)0x41,(BYTE)0x00,(BYTE)0x42,(BYTE)0x42,(BYTE)0x42,(BYTE)0x00,(BYTE)0x43,(BYTE)0x43,(BYTE)0x43,(BYTE)0x00,(BYTE)0x44,(BYTE)0x44,(BYTE)0x44,(BYTE)0x00,(BYTE)0x45,(BYTE)0x45,(BYTE)0x45,(BYTE)0x00,(BYTE)0x46,(BYTE)0x46,(BYTE)0x46,(BYTE)0x00,(BYTE)0x47,(BYTE)0x47,(BYTE)0x47,(BYTE)0x00,(BYTE)0x48,(BYTE)0x48,(BYTE)0x48,(BYTE)0x00,(BYTE)0x49,(BYTE)0x49,(BYTE)0x49,(BYTE)0x00,(BYTE)0x4A,(BYTE)0x4A,(BYTE)0x4A,(BYTE)0x00,(BYTE)0x4B,(BYTE)0x4B,(BYTE)0x4B,(BYTE)0x00,(BYTE)0x4C,(BYTE)0x4C,(BYTE)0x4C,(BYTE)0x00,(BYTE)0x4D,(BYTE)0x4D,(BYTE)0x4D,(BYTE)0x00,(BYTE)0x4E,(BYTE)0x4E,(BYTE)0x4E,(BYTE)0x00,(BYTE)0x4F,(BYTE)0x4F,(BYTE)0x4F,(BYTE)0x00,\
	(BYTE)0x50,(BYTE)0x50,(BYTE)0x50,(BYTE)0x00,(BYTE)0x51,(BYTE)0x51,(BYTE)0x51,(BYTE)0x00,(BYTE)0x52,(BYTE)0x52,(BYTE)0x52,(BYTE)0x00,(BYTE)0x53,(BYTE)0x53,(BYTE)0x53,(BYTE)0x00,(BYTE)0x54,(BYTE)0x54,(BYTE)0x54,(BYTE)0x00,(BYTE)0x55,(BYTE)0x55,(BYTE)0x55,(BYTE)0x00,(BYTE)0x56,(BYTE)0x56,(BYTE)0x56,(BYTE)0x00,(BYTE)0x57,(BYTE)0x57,(BYTE)0x57,(BYTE)0x00,(BYTE)0x58,(BYTE)0x58,(BYTE)0x58,(BYTE)0x00,(BYTE)0x59,(BYTE)0x59,(BYTE)0x59,(BYTE)0x00,(BYTE)0x5A,(BYTE)0x5A,(BYTE)0x5A,(BYTE)0x00,(BYTE)0x5B,(BYTE)0x5B,(BYTE)0x5B,(BYTE)0x00,(BYTE)0x5C,(BYTE)0x5C,(BYTE)0x5C,(BYTE)0x00,(BYTE)0x5D,(BYTE)0x5D,(BYTE)0x5D,(BYTE)0x00,(BYTE)0x5E,(BYTE)0x5E,(BYTE)0x5E,(BYTE)0x00,(BYTE)0x5F,(BYTE)0x5F,(BYTE)0x5F,(BYTE)0x00,\
	(BYTE)0x60,(BYTE)0x60,(BYTE)0x60,(BYTE)0x00,(BYTE)0x61,(BYTE)0x61,(BYTE)0x61,(BYTE)0x00,(BYTE)0x62,(BYTE)0x62,(BYTE)0x62,(BYTE)0x00,(BYTE)0x63,(BYTE)0x63,(BYTE)0x63,(BYTE)0x00,(BYTE)0x64,(BYTE)0x64,(BYTE)0x64,(BYTE)0x00,(BYTE)0x65,(BYTE)0x65,(BYTE)0x65,(BYTE)0x00,(BYTE)0x66,(BYTE)0x66,(BYTE)0x66,(BYTE)0x00,(BYTE)0x67,(BYTE)0x67,(BYTE)0x67,(BYTE)0x00,(BYTE)0x68,(BYTE)0x68,(BYTE)0x68,(BYTE)0x00,(BYTE)0x69,(BYTE)0x69,(BYTE)0x69,(BYTE)0x00,(BYTE)0x6A,(BYTE)0x6A,(BYTE)0x6A,(BYTE)0x00,(BYTE)0x6B,(BYTE)0x6B,(BYTE)0x6B,(BYTE)0x00,(BYTE)0x6C,(BYTE)0x6C,(BYTE)0x6C,(BYTE)0x00,(BYTE)0x6D,(BYTE)0x6D,(BYTE)0x6D,(BYTE)0x00,(BYTE)0x6E,(BYTE)0x6E,(BYTE)0x6E,(BYTE)0x00,(BYTE)0x6F,(BYTE)0x6F,(BYTE)0x6F,(BYTE)0x00,\
	(BYTE)0x70,(BYTE)0x70,(BYTE)0x70,(BYTE)0x00,(BYTE)0x71,(BYTE)0x71,(BYTE)0x71,(BYTE)0x00,(BYTE)0x72,(BYTE)0x72,(BYTE)0x72,(BYTE)0x00,(BYTE)0x73,(BYTE)0x73,(BYTE)0x73,(BYTE)0x00,(BYTE)0x74,(BYTE)0x74,(BYTE)0x74,(BYTE)0x00,(BYTE)0x75,(BYTE)0x75,(BYTE)0x75,(BYTE)0x00,(BYTE)0x76,(BYTE)0x76,(BYTE)0x76,(BYTE)0x00,(BYTE)0x77,(BYTE)0x77,(BYTE)0x77,(BYTE)0x00,(BYTE)0x78,(BYTE)0x78,(BYTE)0x78,(BYTE)0x00,(BYTE)0x79,(BYTE)0x79,(BYTE)0x79,(BYTE)0x00,(BYTE)0x7A,(BYTE)0x7A,(BYTE)0x7A,(BYTE)0x00,(BYTE)0x7B,(BYTE)0x7B,(BYTE)0x7B,(BYTE)0x00,(BYTE)0x7C,(BYTE)0x7C,(BYTE)0x7C,(BYTE)0x00,(BYTE)0x7D,(BYTE)0x7D,(BYTE)0x7D,(BYTE)0x00,(BYTE)0x7E,(BYTE)0x7E,(BYTE)0x7E,(BYTE)0x00,(BYTE)0x7F,(BYTE)0x7F,(BYTE)0x7F,(BYTE)0x00,\
	(BYTE)0x80,(BYTE)0x80,(BYTE)0x80,(BYTE)0x00,(BYTE)0x81,(BYTE)0x81,(BYTE)0x81,(BYTE)0x00,(BYTE)0x82,(BYTE)0x82,(BYTE)0x82,(BYTE)0x00,(BYTE)0x83,(BYTE)0x83,(BYTE)0x83,(BYTE)0x00,(BYTE)0x84,(BYTE)0x84,(BYTE)0x84,(BYTE)0x00,(BYTE)0x85,(BYTE)0x85,(BYTE)0x85,(BYTE)0x00,(BYTE)0x86,(BYTE)0x86,(BYTE)0x86,(BYTE)0x00,(BYTE)0x87,(BYTE)0x87,(BYTE)0x87,(BYTE)0x00,(BYTE)0x88,(BYTE)0x88,(BYTE)0x88,(BYTE)0x00,(BYTE)0x89,(BYTE)0x89,(BYTE)0x89,(BYTE)0x00,(BYTE)0x8A,(BYTE)0x8A,(BYTE)0x8A,(BYTE)0x00,(BYTE)0x8B,(BYTE)0x8B,(BYTE)0x8B,(BYTE)0x00,(BYTE)0x8C,(BYTE)0x8C,(BYTE)0x8C,(BYTE)0x00,(BYTE)0x8D,(BYTE)0x8D,(BYTE)0x8D,(BYTE)0x00,(BYTE)0x8E,(BYTE)0x8E,(BYTE)0x8E,(BYTE)0x00,(BYTE)0x8F,(BYTE)0x8F,(BYTE)0x8F,(BYTE)0x00,\
	(BYTE)0x90,(BYTE)0x90,(BYTE)0x90,(BYTE)0x00,(BYTE)0x91,(BYTE)0x91,(BYTE)0x91,(BYTE)0x00,(BYTE)0x92,(BYTE)0x92,(BYTE)0x92,(BYTE)0x00,(BYTE)0x93,(BYTE)0x93,(BYTE)0x93,(BYTE)0x00,(BYTE)0x94,(BYTE)0x94,(BYTE)0x94,(BYTE)0x00,(BYTE)0x95,(BYTE)0x95,(BYTE)0x95,(BYTE)0x00,(BYTE)0x96,(BYTE)0x96,(BYTE)0x96,(BYTE)0x00,(BYTE)0x97,(BYTE)0x97,(BYTE)0x97,(BYTE)0x00,(BYTE)0x98,(BYTE)0x98,(BYTE)0x98,(BYTE)0x00,(BYTE)0x99,(BYTE)0x99,(BYTE)0x99,(BYTE)0x00,(BYTE)0x9A,(BYTE)0x9A,(BYTE)0x9A,(BYTE)0x00,(BYTE)0x9B,(BYTE)0x9B,(BYTE)0x9B,(BYTE)0x00,(BYTE)0x9C,(BYTE)0x9C,(BYTE)0x9C,(BYTE)0x00,(BYTE)0x9D,(BYTE)0x9D,(BYTE)0x9D,(BYTE)0x00,(BYTE)0x9E,(BYTE)0x9E,(BYTE)0x9E,(BYTE)0x00,(BYTE)0x9F,(BYTE)0x9F,(BYTE)0x9F,(BYTE)0x00,\
	(BYTE)0xA0,(BYTE)0xA0,(BYTE)0xA0,(BYTE)0x00,(BYTE)0xA1,(BYTE)0xA1,(BYTE)0xA1,(BYTE)0x00,(BYTE)0xA2,(BYTE)0xA2,(BYTE)0xA2,(BYTE)0x00,(BYTE)0xA3,(BYTE)0xA3,(BYTE)0xA3,(BYTE)0x00,(BYTE)0xA4,(BYTE)0xA4,(BYTE)0xA4,(BYTE)0x00,(BYTE)0xA5,(BYTE)0xA5,(BYTE)0xA5,(BYTE)0x00,(BYTE)0xA6,(BYTE)0xA6,(BYTE)0xA6,(BYTE)0x00,(BYTE)0xA7,(BYTE)0xA7,(BYTE)0xA7,(BYTE)0x00,(BYTE)0xA8,(BYTE)0xA8,(BYTE)0xA8,(BYTE)0x00,(BYTE)0xA9,(BYTE)0xA9,(BYTE)0xA9,(BYTE)0x00,(BYTE)0xAA,(BYTE)0xAA,(BYTE)0xAA,(BYTE)0x00,(BYTE)0xAB,(BYTE)0xAB,(BYTE)0xAB,(BYTE)0x00,(BYTE)0xAC,(BYTE)0xAC,(BYTE)0xAC,(BYTE)0x00,(BYTE)0xAD,(BYTE)0xAD,(BYTE)0xAD,(BYTE)0x00,(BYTE)0xAE,(BYTE)0xAE,(BYTE)0xAE,(BYTE)0x00,(BYTE)0xAF,(BYTE)0xAF,(BYTE)0xAF,(BYTE)0x00,\
	(BYTE)0xB0,(BYTE)0xB0,(BYTE)0xB0,(BYTE)0x00,(BYTE)0xB1,(BYTE)0xB1,(BYTE)0xB1,(BYTE)0x00,(BYTE)0xB2,(BYTE)0xB2,(BYTE)0xB2,(BYTE)0x00,(BYTE)0xB3,(BYTE)0xB3,(BYTE)0xB3,(BYTE)0x00,(BYTE)0xB4,(BYTE)0xB4,(BYTE)0xB4,(BYTE)0x00,(BYTE)0xB5,(BYTE)0xB5,(BYTE)0xB5,(BYTE)0x00,(BYTE)0xB6,(BYTE)0xB6,(BYTE)0xB6,(BYTE)0x00,(BYTE)0xB7,(BYTE)0xB7,(BYTE)0xB7,(BYTE)0x00,(BYTE)0xB8,(BYTE)0xB8,(BYTE)0xB8,(BYTE)0x00,(BYTE)0xB9,(BYTE)0xB9,(BYTE)0xB9,(BYTE)0x00,(BYTE)0xBA,(BYTE)0xBA,(BYTE)0xBA,(BYTE)0x00,(BYTE)0xBB,(BYTE)0xBB,(BYTE)0xBB,(BYTE)0x00,(BYTE)0xBC,(BYTE)0xBC,(BYTE)0xBC,(BYTE)0x00,(BYTE)0xBD,(BYTE)0xBD,(BYTE)0xBD,(BYTE)0x00,(BYTE)0xBE,(BYTE)0xBE,(BYTE)0xBE,(BYTE)0x00,(BYTE)0xBF,(BYTE)0xBF,(BYTE)0xBF,(BYTE)0x00,\
	(BYTE)0xC0,(BYTE)0xC0,(BYTE)0xC0,(BYTE)0x00,(BYTE)0xC1,(BYTE)0xC1,(BYTE)0xC1,(BYTE)0x00,(BYTE)0xC2,(BYTE)0xC2,(BYTE)0xC2,(BYTE)0x00,(BYTE)0xC3,(BYTE)0xC3,(BYTE)0xC3,(BYTE)0x00,(BYTE)0xC4,(BYTE)0xC4,(BYTE)0xC4,(BYTE)0x00,(BYTE)0xC5,(BYTE)0xC5,(BYTE)0xC5,(BYTE)0x00,(BYTE)0xC6,(BYTE)0xC6,(BYTE)0xC6,(BYTE)0x00,(BYTE)0xC7,(BYTE)0xC7,(BYTE)0xC7,(BYTE)0x00,(BYTE)0xC8,(BYTE)0xC8,(BYTE)0xC8,(BYTE)0x00,(BYTE)0xC9,(BYTE)0xC9,(BYTE)0xC9,(BYTE)0x00,(BYTE)0xCA,(BYTE)0xCA,(BYTE)0xCA,(BYTE)0x00,(BYTE)0xCB,(BYTE)0xCB,(BYTE)0xCB,(BYTE)0x00,(BYTE)0xCC,(BYTE)0xCC,(BYTE)0xCC,(BYTE)0x00,(BYTE)0xCD,(BYTE)0xCD,(BYTE)0xCD,(BYTE)0x00,(BYTE)0xCE,(BYTE)0xCE,(BYTE)0xCE,(BYTE)0x00,(BYTE)0xCF,(BYTE)0xCF,(BYTE)0xCF,(BYTE)0x00,\
	(BYTE)0xD0,(BYTE)0xD0,(BYTE)0xD0,(BYTE)0x00,(BYTE)0xD1,(BYTE)0xD1,(BYTE)0xD1,(BYTE)0x00,(BYTE)0xD2,(BYTE)0xD2,(BYTE)0xD2,(BYTE)0x00,(BYTE)0xD3,(BYTE)0xD3,(BYTE)0xD3,(BYTE)0x00,(BYTE)0xD4,(BYTE)0xD4,(BYTE)0xD4,(BYTE)0x00,(BYTE)0xD5,(BYTE)0xD5,(BYTE)0xD5,(BYTE)0x00,(BYTE)0xD6,(BYTE)0xD6,(BYTE)0xD6,(BYTE)0x00,(BYTE)0xD7,(BYTE)0xD7,(BYTE)0xD7,(BYTE)0x00,(BYTE)0xD8,(BYTE)0xD8,(BYTE)0xD8,(BYTE)0x00,(BYTE)0xD9,(BYTE)0xD9,(BYTE)0xD9,(BYTE)0x00,(BYTE)0xDA,(BYTE)0xDA,(BYTE)0xDA,(BYTE)0x00,(BYTE)0xDB,(BYTE)0xDB,(BYTE)0xDB,(BYTE)0x00,(BYTE)0xDC,(BYTE)0xDC,(BYTE)0xDC,(BYTE)0x00,(BYTE)0xDD,(BYTE)0xDD,(BYTE)0xDD,(BYTE)0x00,(BYTE)0xDE,(BYTE)0xDE,(BYTE)0xDE,(BYTE)0x00,(BYTE)0xDF,(BYTE)0xDF,(BYTE)0xDF,(BYTE)0x00,\
	(BYTE)0xE0,(BYTE)0xE0,(BYTE)0xE0,(BYTE)0x00,(BYTE)0xE1,(BYTE)0xE1,(BYTE)0xE1,(BYTE)0x00,(BYTE)0xE2,(BYTE)0xE2,(BYTE)0xE2,(BYTE)0x00,(BYTE)0xE3,(BYTE)0xE3,(BYTE)0xE3,(BYTE)0x00,(BYTE)0xE4,(BYTE)0xE4,(BYTE)0xE4,(BYTE)0x00,(BYTE)0xE5,(BYTE)0xE5,(BYTE)0xE5,(BYTE)0x00,(BYTE)0xE6,(BYTE)0xE6,(BYTE)0xE6,(BYTE)0x00,(BYTE)0xE7,(BYTE)0xE7,(BYTE)0xE7,(BYTE)0x00,(BYTE)0xE8,(BYTE)0xE8,(BYTE)0xE8,(BYTE)0x00,(BYTE)0xE9,(BYTE)0xE9,(BYTE)0xE9,(BYTE)0x00,(BYTE)0xEA,(BYTE)0xEA,(BYTE)0xEA,(BYTE)0x00,(BYTE)0xEB,(BYTE)0xEB,(BYTE)0xEB,(BYTE)0x00,(BYTE)0xEC,(BYTE)0xEC,(BYTE)0xEC,(BYTE)0x00,(BYTE)0xED,(BYTE)0xED,(BYTE)0xED,(BYTE)0x00,(BYTE)0xEE,(BYTE)0xEE,(BYTE)0xEE,(BYTE)0x00,(BYTE)0xEF,(BYTE)0xEF,(BYTE)0xEF,(BYTE)0x00,\
	(BYTE)0xF0,(BYTE)0xF0,(BYTE)0xF0,(BYTE)0x00,(BYTE)0xF1,(BYTE)0xF1,(BYTE)0xF1,(BYTE)0x00,(BYTE)0xF2,(BYTE)0xF2,(BYTE)0xF2,(BYTE)0x00,(BYTE)0xF3,(BYTE)0xF3,(BYTE)0xF3,(BYTE)0x00,(BYTE)0xF4,(BYTE)0xF4,(BYTE)0xF4,(BYTE)0x00,(BYTE)0xF5,(BYTE)0xF5,(BYTE)0xF5,(BYTE)0x00,(BYTE)0xF6,(BYTE)0xF6,(BYTE)0xF6,(BYTE)0x00,(BYTE)0xF7,(BYTE)0xF7,(BYTE)0xF7,(BYTE)0x00,(BYTE)0xF8,(BYTE)0xF8,(BYTE)0xF8,(BYTE)0x00,(BYTE)0xF9,(BYTE)0xF9,(BYTE)0xF9,(BYTE)0x00,(BYTE)0xFA,(BYTE)0xFA,(BYTE)0xFA,(BYTE)0x00,(BYTE)0xFB,(BYTE)0xFB,(BYTE)0xFB,(BYTE)0x00,(BYTE)0xFC,(BYTE)0xFC,(BYTE)0xFC,(BYTE)0x00,(BYTE)0xFD,(BYTE)0xFD,(BYTE)0xFD,(BYTE)0x00,(BYTE)0xFE,(BYTE)0xFE,(BYTE)0xFE,(BYTE)0x00,(BYTE)0xFF,(BYTE)0xFF,(BYTE)0xFF,(BYTE)0x00
};

//for RGB332 Color use
const unsigned char ctGreenColor		=		(unsigned char)28; //RGB332 Green
const unsigned char ctP2GreenColor		=		(unsigned char)61; //Replace RGB(50,255,50)
const unsigned char ctRedColor			=		(unsigned char)224; //RGB332 Red
const unsigned char ctP2RedColor		=		(unsigned char)229;
const unsigned char ctBlueColor			=		(unsigned char)3; //RGB332 Blue
const unsigned char ctP2BlueColor		=		(unsigned char)39; //Replace RGB(50,50,255)
const unsigned char ctDiffMaskColor		=		(unsigned char)243; //Replace RGB(255,128,255)
const unsigned char ctP2InspectionColor =		(unsigned char)233; //Inspection Red Replace RGB(255,60,60)
const unsigned char ctEmbossColor_one	=		(unsigned char)31; // 單向陰影
const unsigned char ctEmbossColor_one_center =	(unsigned char)224; // 質心
const unsigned char ctEmbossColor_two	=		(unsigned char)227; // 雙向
const unsigned char ctTargetColor		=		(unsigned char)243; //Replace RGB(255,128,255)
const unsigned char ctBlendColor		=		(unsigned char)235; // Replace RGB(255,64,255)
const unsigned char ctOutlineColor		=		(unsigned char)253; // Replace RGB(128,255,0)

//For DefectMap Color RGB332-------------------
const unsigned char ctDFLAG_SERIOUS			=	(unsigned char)255; //RGB(255, 255, 255);
const unsigned char ctDFLAG_BAND_PASS		=	(unsigned char)192;
const unsigned char ctDFLAG_BAND_STOP		=	(unsigned char)128;  //RGB(64, 0, 0);
const unsigned char ctDFLAG_BAND_EXIST		=	(unsigned char)2; //navy blue RGB(0, 0, 128);
const unsigned char ctDFLAG_COLORDIFF		=	(unsigned char)252; //yellow RGB(255, 255, 0);
const unsigned char ctDFLAG_LIGHT			=	(unsigned char)224; //red
const unsigned char ctDFLAG_INK_V			=	(unsigned char)28; //green RGB(0, 255, 0)
const unsigned char ctDFLAG_COLORDERIVATIVE	=	(unsigned char)16; //green
const unsigned char ctDFLAG_INNERDEFECT		=	(unsigned char)13; //RGB(0, 128, 128);
const unsigned char ctDFLAG_SHADOW			=	(unsigned char)221; //RGB(192, 128, 255);
const unsigned char ctDFLAG_EDGEDEFECT		=	(unsigned char)221; //RGB(225, 255, 64);

//eric chao 20150605 for defect piece
const int ctDEFECT_IMGRECT_W				= 224;
const int ctDEFECT_IMGRECT_H				= 256;
// 24-bit RGB color.
const COLORREF ctRgbWhite = RGB(255, 255, 255);	// White

//For Page Mask //eric chao 20131217
const unsigned char ctPAGE1_MK = 1;
const unsigned char ctPAGE2_MK = 2;
const unsigned char ctPAGE3_MK = 4;
const unsigned char ctPAGE4_MK = 8;
const unsigned char ctPAGE5_MK = 16;
const unsigned char ctPAGE6_MK = 32;

//For Camera Type-----------------------------
const unsigned char ctCAMERA_2K				= 0;
const unsigned char ctCAMERA_4K				= 1;
const unsigned char ctCAMERA_8K				= 2;
const unsigned char ctCAMERA_AREA			= 4;

//For Global
const int ctMAX_CAMERA_NUM = 8;
const int ctMAX_MATRIX_UNIT_OF_COLUMN = 8;
const int ctMAX_MACHINE_NUM = 8;
const int ctMAX_EXTRA_DEVICE = 4;
const int ctMAX_LIGHT_DEVICE = 16;
const int ctMAX_WEB_COLUMN = 16;
const int ctMAX_INKJET_NUM = 8;

//For Insp Error Process
const int ctERROR_MAX_INSP_TIME = 500; //ms  //eric chao 20131225

//For OpenGl Display Limit-- 660,760,960
const int ctOPENGL_TEXTURE_MAX_HEIGHT = 16384;


//For Anchor
const int ctMAX_UNIT_ANCHOR_NUM = 2;
const int ctDEFAULT_ANCHOR_SHAPE_WIDTH  = 200;
const int ctDEFAULT_ANCHOR_SHAPE_HEIGHT = 200;

//For Page3 Error Message
enum {	ERR_MSG_BEGIN=0,
		ERR_MSG_IO_MISS_BLANK = 0, 
		ERR_MSG_IMAGE_MISS=1,
		ERR_MSG_FIND_START=2,
		ERR_MSG_CAPTURE_OVERWRITE=3,
		ERR_MSG_KICK_MISS=4,
		ERR_MSG_PAGETRIG_MISS=5,
		ERR_MSG_MISSTRIG_MISS=6,
		ERR_MSG_CANBUS_IOCARD =7, //eric chao 20141009
		ERR_MSG_CANBUS_TRANSCARD =8, //eric chao 20141009
		ERR_MSG_BARCODEDUPKICATED =9,	// added by eric at 20150915
		ERR_MSG_BARCODENOEXISTED =10,	// added by eric at 20150915
		ERR_MSG_FIND_START_DONE =11,	//seanchen 20151023-01
		ERR_MSG_IO_SOLENOID_MEMFULL =12,	//seanchen 20170328-01
		ERR_MSG_END
	};
//For Normal Function Operate //eric chao 20140718
enum FUN_OP_CODE
{
		OP_FUN_INIT		=0,
		OP_FUN_CREATE	=1,
		OP_FUN_DESTROY	=2,
		OP_FUN_START	=3,
		OP_FUN_STOP		=4,
		OP_FUN_EXPORT	=5,
		OP_FUN_CHECK	=6,
		OP_FUN_COMMIT	=7,
		OP_FUN_RESET	=8,
		OP_FUN_NEWBATCH
};

enum {	OP_FUN_DATA_ADD = 0,
		OP_FUN_DATA_REMOVE = 1,
		OP_FUN_DATA_CHECK = 2,
		OP_FUN_DATA_MARK = 3,
		OP_FUN_DATA_CLEAR = 4,
		OP_FUN_DATA_DUMP =5,
		OP_FUN_DATA_SAVE =6,
		OP_FUN_DATA_LOAD =7,
		OP_FUN_DATA_COMMIT =8,
		OP_FUN_DATA_EXIST =9,
		OP_FUN_DATA_FIRST,
		OP_FUN_DATA_SIZE
};
// added by eric at 20150626
enum COLORCHANNEL
{
	TYPE_RGB = 0,
	TYPE_RED,
	TYPE_GREEN,
	TYPE_BLUE
};

#define MASK_CLIENT(x)	(1<<x)
#define MSG_CLIENT_MAP(x)  (~(1<<x))

#pragma pack(pop)
#define MAX_INSPECT_DEFECT  32	//eric chao 20160929
#define MAX_UI_DEFECT	4 //eric chao 20160929
#define MAX_DIGITS 10	//OCR Save Number
#define _JJ_MAGIC_NO		10 //用來預留拍攝的紙長邊緣(2cm)
#define RUNWINDOW_SIZE		100 //次級品儲存的最大數目
#define DIMG_MEMORY_SIZE		(1024*1048576LL)	// 1024MB
#define DIMG_MEMORY_SIZE_MASTER	(2*1024*1048576LL)	// 2048MB
//eric chao 20130924
#define LIGHT_MAX_TEMP _T("70,70,70,70")
const int ctLIGHT_MAX_TEMP = 70;
const int ctDefaultT2C = 2000; //2000 Encoder
const int ctDefaultMaxT2C = 150; //For Testing Page Trigger...150 mm
const int ctDefaultT2S = 3000;
const double ctP_TRIG_LEN1 = 230.0; //eric chao 20140422
const double ctP_TRIG_LEN2 = 120.0; //eric chao 20140422

const int ctMAX_BARCODE_DIGITS = 20; //eric chao 20140415

const int ctMIN_COLOR_DERIVATE_LENGTH = 3; //eric chao 20140418
const int ctMAX_COLOR_DERIVATE_LENGTH_2K = 2000;
const int ctMAX_COLOR_DERIVATE_LENGTH_4K = 4000; 
const int ctMAX_COLOR_DERIVATE_LENGTH_8K = 7000; //7296 & 8192

//For Defect Object Id Sel
const int ctDefault_ObjectIdFile = 10; //eric chao 20150428
const int ctDefault_ObjectIdFile_Big = 30; 
#ifdef USE_DUMMY_GRABBER
const int ctDefault_ObjectMinSamples = 10; //eric chao 20150608
#else //USE_DUMMY_GRABBER
const int ctDefault_ObjectMinSamples = 100; //eric chao 20150608
#endif //USE_DUMMY_GRABBER
const int ctDefault_ObjectSelPosPercent = 95; //eric chao 20150608
const int ctDefault_ObjectPercent_Range = 10; //eric chao 20150428
const int ctDefault_ObjectPercent_START = 100 - ctDefault_ObjectPercent_Range; //eric chao 20150428
const int ctDefault_ObjectPercent_END = 100 + ctDefault_ObjectPercent_Range; //eric chao 20150428

//For Page2 DefaultValue
const int ctDefault_PaperShrink = 2;
#define MAX_PAGE2_SAMPLES_MULTIMODEL	 16
#define MAX_PAGE2_SAMPLES_NORMAL		 10

//For Multi Channel Process
const int ctCHANNEL_MIN_WIDTH	=1280;
const int ctCHANNEL_MIN_HEIGHT	= 960;
const int ctMAX_WEB_FRAGMENT_LEN = 3000; //eric chao 20170525
const int ctGoldenSubKey = 0x10003; // 0x10003 ==> UnitType=DECODE_SAMPLE(3), UnitId=1

static void OnGetDefaultPageLength(int nCameraType,int &nPageLength) //eric chao 20131007
{
	switch(nCameraType){
		case ctCAMERA_4K:
			nPageLength = 1000;
			break;
		case ctCAMERA_8K:
			nPageLength = 2000;
			break;
		default:
			nPageLength = 550;
			break;
	}
}
static void OnGetCameraType(int iImgSize, int &iCameraType)	// added by eric at 20130524
{
	if (iImgSize >= 7000) {//7296 & 8192
		iCameraType = ctCAMERA_8K;
	} else if (iImgSize >= 4096){
		iCameraType = ctCAMERA_4K;
	} else if (iImgSize >= 2048){
		iCameraType = ctCAMERA_2K;
	} else {
		iCameraType = ctCAMERA_AREA; 
	}
}

static void OnGetImgSize(int iCameraType, int &iImgSize)	// added by eric at 20161217
{
	if (iCameraType == ctCAMERA_2K) {
		iImgSize = 2048;
	} else if (iCameraType == ctCAMERA_4K) {
		iImgSize = 4096;
	} else if (iCameraType == ctCAMERA_8K) {
		iImgSize = 8192;
	} else if (iCameraType == ctCAMERA_AREA) {
		iImgSize = 1024;
	}
}

static void OnGetAdvancedHeavyVal(int iCameraType, int &iHeavyVal)	// added by eric at 20130524
{
	if (iCameraType == ctCAMERA_2K) {
		iHeavyVal = 10;
	} else if (iCameraType == ctCAMERA_4K) {
		iHeavyVal = 40;
	} else if (iCameraType == ctCAMERA_8K) {
		iHeavyVal = 160;
	} else if (iCameraType == ctCAMERA_AREA) {
		iHeavyVal = 10;
	}
};

static void OnGetMaxPnts(int iCameraType, int &iMaxPnts)	// added by eric at 20130524
{
	if (iCameraType == ctCAMERA_2K) {
		iMaxPnts = 100;
	} else if (iCameraType == ctCAMERA_4K) {
		iMaxPnts = 400;
	} else if (iCameraType == ctCAMERA_8K) {
		iMaxPnts = 1600;
	} else if (iCameraType == ctCAMERA_AREA) {
		iMaxPnts = 100;
	}
};

static void OnGetColorDerivatePnts(int iCameraType, int &iPnts)	// added by eric at 20130524
{
	if (iCameraType == ctCAMERA_2K) {
		iPnts = 30;
	} else if (iCameraType == ctCAMERA_4K) {
		iPnts = 120;
	} else if (iCameraType == ctCAMERA_8K) {
		iPnts = 400;
	} else if (iCameraType == ctCAMERA_AREA) {
		iPnts = 30;
	}
};

static void OnGetColorDerivateMaxPnts(int iCameraType, int &iMaxPnts)	// added by eric at 20130524
{
	if (iCameraType == ctCAMERA_2K) {
		iMaxPnts = 16*16;
	} else if (iCameraType == ctCAMERA_4K) {
		iMaxPnts = 32*32;
	} else if (iCameraType == ctCAMERA_8K) {
		iMaxPnts = 64*64;
	} else if (iCameraType == ctCAMERA_AREA) {
		iMaxPnts = 16*16;
	}
};

static void OnGetAnchorSize(int iCameraType, int &iWidth, int &iHeight)	// added by eric at 20130524
{
	if (iCameraType == ctCAMERA_2K) {
		iWidth = iHeight = 32;
	} else if (iCameraType == ctCAMERA_4K) {
		iWidth = iHeight = 64;
	} else if (iCameraType == ctCAMERA_8K) {
		iWidth = iHeight = 96;
	} else if (iCameraType == ctCAMERA_AREA) {
		iWidth = iHeight = 64;
	}
};
// added by eric at 20150427
static void OnGetAnchorMaxSize(int iCameraType, int &iWidth, int &iHeight)
{
	if (iCameraType==ctCAMERA_8K) {
		iWidth=iHeight=400;
	} else {
		iWidth=iHeight=200;
	}
}
// added by eric at 20150427, half size
static void OnGetAnchorStandardShapeSize(int iCameraType, int &iSize)
{
	if (iCameraType == ctCAMERA_2K) {
		iSize = 16;
	} else if (iCameraType == ctCAMERA_4K) {
		iSize = 32;
	} else if (iCameraType == ctCAMERA_8K) {
		iSize = 48;
	} else if (iCameraType == ctCAMERA_AREA) {
		iSize = 32;
	}
}

static void OnGetAnchorShiftPos(int iCameraType, int &iShiftX, int &iShiftY)	// added by eric at 20130524
{
	if (iCameraType == ctCAMERA_2K) {
		if (iShiftX == 0 && iShiftY == 0)
			iShiftX = iShiftY = 8;
	} else if (iCameraType == ctCAMERA_4K) {
		if (iShiftX == 0 && iShiftY == 0)
			iShiftX = iShiftY = 15;
	} else if (iCameraType == ctCAMERA_8K) {
		if (iShiftX == 0 && iShiftY == 0)
			iShiftX = iShiftY = 30;
	} else if (iCameraType == ctCAMERA_AREA) {
		if (iShiftX == 0 && iShiftY == 0)
			iShiftX = iShiftY = 15;
	}
}

static void OnGetAnchorSearchRange(int iCameraType, BOOL bDistanceMode, int &iLower, int &iHigher, int &iSearchRange)	// added by eric at 20130524
{
	iLower = 2;
	if (iCameraType == ctCAMERA_2K) {
		iHigher = 15;
		if (iSearchRange == 0) {
			if (bDistanceMode)
				iSearchRange = 50;
			else
				iSearchRange = 100;
		}
	} else if (iCameraType == ctCAMERA_4K) {
		iHigher = 30;
		if (iSearchRange == 0) {
			if (bDistanceMode)
				iSearchRange = 100;
			else
				iSearchRange = 200;
		}
	} else if (iCameraType == ctCAMERA_8K) {
		iHigher = 60;
		if (iSearchRange == 0) {
			if (bDistanceMode)
				iSearchRange = 200;
			else
				iSearchRange = 400;
		}
	} else if (iCameraType == ctCAMERA_AREA) {
		iHigher = 60;
		if (iSearchRange == 0) {
			if (bDistanceMode)
				iSearchRange = 100;
			else
				iSearchRange = 200;
		}
	}
};

static void OnGetColorDerivateLength(int iCameraType, int &iLength)	// added by eric at 20130524
{
	if (iCameraType == ctCAMERA_2K) {
		iLength = 3;
	} else if (iCameraType == ctCAMERA_4K) {
		iLength = 6;
	} else if (iCameraType == ctCAMERA_8K) {
		iLength = 12;
	} else if (iCameraType == ctCAMERA_AREA) {
		iLength = 3;
	}
};

static void OnGetColorDerivateBoxSize(int iCameraType, POINT &ptGeneral, POINT &ptHorizontal, POINT &ptVertical)	// added by eric at 20130923, 20131004
{
	if (iCameraType == ctCAMERA_2K) {
		ptGeneral.x = ptGeneral.y = 16;
		ptHorizontal.x = 100;
		ptHorizontal.y = 3;
		ptVertical.x = 3;
		ptVertical.y = 100;
	} else if (iCameraType == ctCAMERA_4K) {
		ptGeneral.x = ptGeneral.y = 32;
		ptHorizontal.x = 200;
		ptHorizontal.y = 6;
		ptVertical.x = 6;
		ptVertical.y = 200;
	} else if (iCameraType == ctCAMERA_8K) {
		ptGeneral.x = ptGeneral.y = 64;
		ptHorizontal.x = 300;
		ptHorizontal.y = 10;
		ptVertical.x = 10;
		ptVertical.y = 300;
	} else if (iCameraType == ctCAMERA_AREA) {
		ptGeneral.x = ptGeneral.y = 16;
		ptHorizontal.x = 100;
		ptHorizontal.y = 3;
		ptVertical.x = 3;
		ptVertical.y = 100;
	}
}
// added by eric at 20140610
static void OnGetPatternMapDefaultSize(int iCameraType, int &width, int &height)
{
	if (iCameraType == ctCAMERA_2K) {
		width = height = 50;
	} else if (iCameraType == ctCAMERA_4K) {
		width = height = 100;
	} else if (iCameraType == ctCAMERA_8K) {
		width = height = 200;
	} else if (iCameraType == ctCAMERA_AREA) {
		width = height = 50;
	}
}

static void OnGetInspEdgeStrengthPnts(int iCameraType, int &iPnts)	// added by eric at 20150702
{
	if (iCameraType == ctCAMERA_2K) {
		iPnts = 30;
	} else if (iCameraType == ctCAMERA_4K) {
		iPnts = 120;
	} else if (iCameraType == ctCAMERA_8K) {
		iPnts = 400;
	} else if (iCameraType == ctCAMERA_AREA) {
		iPnts = 30;
	}
};

// 分切機使用
#define AOI_SPLITTER_PRJ_FOLDER _T("\\SPLITTER\\PRJ") //eric chao 20140815 
#define AOI_SPLITTER_CACHE_FOLDER _T("\\SPLITTER\\CACHE\\UPLOAD") //eric chao 20140808
#define AOI_SPLITTER_DOWNLOAD_FOLDER _T("\\SPLITTER\\CACHE\\DOWNLOAD") //eric chao 20140814
#define AOI_SPLITTER_CLIENT_FOLDER _T("\\CUTTER") //eric chao 20140815

#define AOI_DEF_DUMP_FOLDER _T("\\DEFECT_DUMP\\")

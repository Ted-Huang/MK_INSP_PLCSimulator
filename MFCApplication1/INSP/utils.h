#pragma once

int getHistogramGT(unsigned char *luminances, int w, int h, int color, int *blackCenter, int *whiteCenter);
int fixedGTBinary(unsigned char *src, int width, int height, int nColor, unsigned char *dst, unsigned char GT);
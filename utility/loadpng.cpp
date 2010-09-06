#define STRICT
#include <vector>
#include <string>
#include <windows.h>
#include "png.h"
using namespace std;

#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "libpng.lib")

#define for if (0) ; else for

//---------------------------------------------------------------------------
// LoadPng用ファイル読み込みプロシージャ
void PngReadProc(png_structp png_ptr, png_bytep data, png_size_t length) {
  ReadFile(png_get_io_ptr(png_ptr), data, length, (DWORD*)&length, NULL);
}

//---------------------------------------------------------------------------
// LoadPng用4色形式→16色形式変換関数
void to4bpp(png_structp png_ptr, png_row_infop row_info, png_bytep data) {
  static const png_byte pix[] = {
    0x00, 0x01, 0x02, 0x03, 0x10, 0x11, 0x12, 0x13,
    0x20, 0x21, 0x22, 0x23, 0x30, 0x31, 0x32, 0x33,
  };
  png_uint_32 rowb;
  png_byte *p, *q, c;

  rowb = (row_info->width + 1) / 2;
  q = data + rowb;
  p = data + rowb / 2;

  if (rowb % 2 == 1) {
    c = *p;
    *(--q) = pix[c >> 4];
  }
  while (p > data) {
    c = *(--p);
    *(--q) = pix[c & 0x0f];
    *(--q) = pix[c >> 4];
  }
  row_info->bit_depth   = 4;
  row_info->pixel_depth = 4;
  row_info->rowbytes    = rowb;
}

//---------------------------------------------------------------------------
// PNGファイルを読み込む
HBITMAP LoadPng(const string &strFilename) {
  HANDLE hFile = CreateFile(strFilename.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
  if (hFile == INVALID_HANDLE_VALUE)
    return NULL;

  // PNG読み込み開始
  png_struct *png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr) {
    CloseHandle(hFile);
    return NULL;
  }

  png_info *info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    png_destroy_read_struct(&png_ptr, NULL, NULL);
    CloseHandle(hFile);
    return NULL;
  }
  png_info *end_info = png_create_info_struct(png_ptr);
  if (!end_info) {
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    CloseHandle(hFile);
    return NULL;
  }

  png_set_read_fn(png_ptr, hFile, PngReadProc);
  png_uint_32 nWidth, nHeight;
  int nDepth, nPal;
  int nPngDepth, nColorType, nInterlaceType, nCompType, nFilterType;

  png_read_info(png_ptr, info_ptr);
  png_get_IHDR(png_ptr, info_ptr, &nWidth, &nHeight, &nPngDepth, &nColorType, &nInterlaceType, &nCompType, &nFilterType);

  if (nColorType == PNG_COLOR_TYPE_RGB || nColorType == PNG_COLOR_TYPE_RGB_ALPHA) {
    nPngDepth = 24;
    nDepth = 24;
    nPal = 0;
  } else {
    switch (nPngDepth) {
      case 2:  nDepth = 4; break;
      case 16: nDepth = 8; break;
      default: nDepth = nPngDepth; break;
    }
    nPal = 1 << nDepth;
  }

  vector <png_color> vPalette;
  if (nPal > 0)
    vPalette.resize(nPal);
  int nRowBytes = (nWidth * nDepth + 31) / 32 * 4;
  int nImgBytes = nRowBytes * nHeight;
  BYTE *pImgPtr = (BYTE*)GlobalAlloc(GMEM_FIXED, nImgBytes);
  vector<BYTE*> vRowPtr;
  vRowPtr.reserve(nHeight);
  for (int y = nHeight - 1; y >= 0; --y)
    vRowPtr.push_back(pImgPtr + y * nRowBytes);

  if (nColorType & PNG_COLOR_MASK_ALPHA)
    png_set_strip_alpha(png_ptr);
  if (nPngDepth == 2)
    png_set_read_user_transform_fn(png_ptr, to4bpp);
  else if (nPngDepth == 16)
    png_set_strip_16(png_ptr);
  if (nColorType== PNG_COLOR_TYPE_RGB || nColorType == PNG_COLOR_TYPE_RGB_ALPHA)
    png_set_bgr(png_ptr);
  png_read_update_info(png_ptr, info_ptr);

  if (nPal > 0) {
    if (nColorType == PNG_COLOR_TYPE_PALETTE) {
      png_color *palette;
      int num_palette;
      png_get_PLTE(png_ptr, info_ptr, &palette, &num_palette);
      if (num_palette > nPal)
        num_palette = nPal;
      memset(vPalette.begin(),       0, nPal * sizeof png_color);
      memcpy(vPalette.begin(), palette, num_palette * sizeof png_color);
    } else {
      int depth = nPngDepth == 16 ? 8 : nPngDepth;
      png_build_grayscale_palette(depth, vPalette.begin());
    }
  }

  png_read_image(png_ptr, vRowPtr.begin());
  png_read_end(png_ptr, end_info);
  png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

  CloseHandle(hFile);

  BITMAPINFO* bi = (BITMAPINFO*)GlobalAlloc(GMEM_FIXED, sizeof BITMAPINFOHEADER + nPal * sizeof RGBQUAD);

  memset(bi, 0, sizeof BITMAPINFOHEADER);
  bi->bmiHeader.biSize        = sizeof BITMAPINFOHEADER;
  bi->bmiHeader.biWidth        = nWidth;
  bi->bmiHeader.biHeight      = nHeight;
  bi->bmiHeader.biPlanes      = 1;
  bi->bmiHeader.biBitCount    = nDepth;
  bi->bmiHeader.biCompression = BI_RGB;
  bi->bmiHeader.biSizeImage    = nImgBytes;
  bi->bmiHeader.biClrUsed      = nPal;

  for (int i = 0; i < nPal; ++i) {
    bi->bmiColors[i].rgbRed   = vPalette[i].red;
    bi->bmiColors[i].rgbGreen = vPalette[i].green;
    bi->bmiColors[i].rgbBlue  = vPalette[i].blue;
  }

  HWND hwnd = GetDesktopWindow();
  HDC hdc = GetDC(hwnd);
  char *pBits;
  HBITMAP hBitmap = CreateDIBSection(hdc, bi, DIB_RGB_COLORS, (void**)&pBits, NULL, 0);
  if (pBits)
    memcpy(pBits, pImgPtr, nImgBytes);
  ReleaseDC(hwnd, hdc);

  GlobalFree(pImgPtr);
  GlobalFree(bi);

  return hBitmap;
}

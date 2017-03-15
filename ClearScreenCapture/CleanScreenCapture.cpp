#include <Windows.h>

PBITMAPINFO CreateBitmapInfoStruct(HBITMAP hBmp)
{
	BITMAP bmp;
	PBITMAPINFO pbmi;
	WORD cClrBits;

	GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp);

	cClrBits = (WORD)(bmp.bmPlanes*bmp.bmBitsPixel);
	if (cClrBits == 1)
	{
		cClrBits = 1;
	}
	else
	{
		if (cClrBits <= 4)
		{
			cClrBits = 4;
		}
		else
		{
			if (cClrBits <= 8)
			{
				cClrBits = 8;
			}
			else
			{
				if (cClrBits <= 16)
				{
					cClrBits = 16;
				}
				else
				{
					if (cClrBits <= 24)
					{
						cClrBits = 24;
					}
					else
					{
						cClrBits = 32;
					}
				}
			}
		}
	}

	if (cClrBits != 24)
	{
		pbmi = (PBITMAPINFO)LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER)+sizeof(RGBQUAD)*(1<<cClrBits));
	}
	else
	{
		pbmi = (PBITMAPINFO)LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER));
	}

	pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pbmi->bmiHeader.biWidth = bmp.bmWidth;
	pbmi->bmiHeader.biHeight = bmp.bmHeight;
	pbmi->bmiHeader.biPlanes = bmp.bmPlanes;
	pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel;
	if (cClrBits < 24) pbmi->bmiHeader.biClrUsed = (1 << cClrBits);

	pbmi->bmiHeader.biCompression = BI_RGB;

	pbmi->bmiHeader.biSizeImage = ((pbmi->bmiHeader.biWidth*cClrBits + 31)&~31) / 8 * pbmi->bmiHeader.biHeight;
	pbmi->bmiHeader.biClrImportant = 0;
	return pbmi;
}

void CreateBMPFile(LPTSTR pszFile, PBITMAPINFO pbi, HBITMAP hBMP, HDC hDC)
{
	HANDLE hf;
	BITMAPFILEHEADER hdr;
	PBITMAPINFOHEADER pbih;
	LPBYTE lpBits;
	DWORD dwTotal;
	DWORD cb;
	BYTE *hp;
	DWORD dwTmp;

	pbih = (PBITMAPINFOHEADER)pbi;
	lpBits = (LPBYTE)GlobalAlloc(GMEM_FIXED, pbih->biSizeImage);

	GetDIBits(hDC, hBMP, 0, (WORD)pbih->biHeight, lpBits, pbi, DIB_RGB_COLORS);

	hf = CreateFile(pszFile, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	hdr.bfType = 0x4d42;

	hdr.bfSize = (DWORD)(sizeof(BITMAPFILEHEADER) + pbih->biSize + pbih->biClrUsed * sizeof(RGBQUAD) + pbih->biSizeImage);
	hdr.bfReserved1 = 0;
	hdr.bfReserved2 = 0;

	hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + pbih->biSize + pbih->biClrUsed * sizeof(RGBQUAD);

	WriteFile(hf, (LPVOID)&hdr, sizeof(BITMAPFILEHEADER), (LPDWORD)&dwTmp, NULL);

	WriteFile(hf, (LPVOID)pbih, sizeof(BITMAPINFOHEADER) + pbih->biClrUsed * sizeof(RGBQUAD), (LPDWORD)&dwTmp, NULL);

	dwTotal = cb = pbih->biSizeImage;
	hp = lpBits;
	WriteFile(hf, (LPSTR)hp, (int)cb, (LPDWORD)&dwTmp, NULL);

	CloseHandle(hf);

	GlobalFree((HGLOBAL)lpBits);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
	int sx = GetSystemMetrics(SM_CXSCREEN),
		sy = GetSystemMetrics(SM_CYSCREEN);
	HDC hDC = GetDC(GetDesktopWindow());
	HDC MyHDC = CreateCompatibleDC(hDC);
	HBITMAP hBMP = CreateCompatibleBitmap(hDC, sx, sy);
	SelectObject(MyHDC, hBMP);
	LOGBRUSH MyBrush;
	MyBrush.lbStyle = BS_SOLID;
	MyBrush.lbColor = 0xFF0000;
	HBRUSH hBrush = CreateBrushIndirect(&MyBrush);
	RECT MyRect = { 0,0,sx,sy };
	FillRect(MyHDC, &MyRect, hBrush);
	BitBlt(MyHDC, 0, 0, sx, sy, hDC, 0, 0, SRCCOPY);
	CreateBMPFile(L"myscreen.bmp", CreateBitmapInfoStruct(hBMP), hBMP, MyHDC);
	return 0;
}
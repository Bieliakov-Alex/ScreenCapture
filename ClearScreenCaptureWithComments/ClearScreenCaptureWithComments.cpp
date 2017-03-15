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
		pbmi = (PBITMAPINFO)LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*(1 << cClrBits));
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
	//Размеры окна пользователя
	int sx = GetSystemMetrics(SM_CXSCREEN),
		sy = GetSystemMetrics(SM_CYSCREEN);

	/*
	The GetDC function retrieves a handle to a device context (DC) for the client area of a specified window or for the entire screen. You can use the returned handle in subsequent GDI functions to draw in the DC. The device context is an opaque data structure, whose values are used internally by GDI.
	https://msdn.microsoft.com/ru-ru/library/windows/desktop/dd144871(v=vs.85).aspx
	*/

	/*
	A device context is a structure that defines a set of graphic objects and their associated attributes, as well as the graphic modes that affect output. The graphic objects include a pen for line drawing, a brush for painting and filling, a bitmap for copying or scrolling parts of the screen, a palette for defining the set of available colors, a region for clipping and other operations, and a path for painting and drawing operations. The remainder of this section is divided into the following three areas.
	https://msdn.microsoft.com/en-us/library/windows/desktop/dd183553(v=vs.85).aspx
	*/

	/*
	An object is a data structure that represents a system resource, such as a file, thread, or graphic image. An application cannot directly access object data or the system resource that an object represents. Instead, an application must obtain an object handle, which it can use to examine or modify the system resource. Each handle has an entry in an internally maintained table. These entries contain the addresses of the resources and the means to identify the resource type.
	https://msdn.microsoft.com/en-us/library/windows/desktop/ms724457(v=vs.85).aspx
	*/

	/*
	Retrieves a handle to the desktop window. The desktop window covers the entire screen. The desktop window is the area on top of which other windows are painted.
	https://msdn.microsoft.com/ru-ru/library/windows/desktop/ms633504(v=vs.85).aspx
	*/

	HDC hDC = GetDC(GetDesktopWindow());

	/*
	The CreateCompatibleDC function creates a memory device context (DC) compatible with the specified device
	https://msdn.microsoft.com/ru-ru/library/windows/desktop/dd183489(v=vs.85).aspx
	*/

	HDC MyHDC = CreateCompatibleDC(hDC);

	/*
	The CreateCompatibleBitmap function creates a bitmap compatible with the device that is associated with the specified device context.
	https://msdn.microsoft.com/ru-ru/library/windows/desktop/dd183488(v=vs.85).aspx
	*/

	HBITMAP hBMP = CreateCompatibleBitmap(hDC, sx, sy);

	/*
	The SelectObject function selects an object into the specified device context (DC). The new object replaces the previous object of the same type.
	https://msdn.microsoft.com/ru-ru/library/windows/desktop/dd162957(v=vs.85).aspx
	*/

	SelectObject(MyHDC, hBMP);

	LOGBRUSH MyBrush;
	MyBrush.lbStyle = BS_SOLID;
	MyBrush.lbColor = 0xFF0000;
	HBRUSH hBrush = CreateBrushIndirect(&MyBrush);
	RECT MyRect = { 0,0,sx,sy };

	/*
	The FillRect function fills a rectangle by using the specified brush. This function includes the left and top borders, but excludes the right and bottom borders of the rectangle.
	https://msdn.microsoft.com/ru-ru/library/windows/desktop/dd162719(v=vs.85).aspx
	*/

	FillRect(MyHDC, &MyRect, hBrush);

	/*
	The BitBlt function performs a bit-block transfer of the color data corresponding to a rectangle of pixels from the specified source device context into a destination device context.
	https://msdn.microsoft.com/ru-ru/library/windows/desktop/dd183370(v=vs.85).aspx
	*/

	BitBlt(MyHDC, 0, 0, sx, sy, hDC, 0, 0, SRCCOPY);

	CreateBMPFile(L"myscreen.bmp", CreateBitmapInfoStruct(hBMP), hBMP, MyHDC);
	return 0;
}
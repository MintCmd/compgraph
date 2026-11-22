// Raster.h : interface of CRaster class
// (C) Alexey Polyakov 2002-2003
/////////////////////////////////////////////////////////////////////////////
#ifndef _RASTER_INCLUDED
#define _RASTER_INCLUDED
// макрос для определения количества байт в выровненной по DWORD строки пикселов в DIB 
// Width - длина строки в пикселах; BPP - бит на пиксел
#define BYTESPERLINE(Width, BPP) ((WORD)((((DWORD)(Width) * \
(DWORD)(BPP) + 31) >> 5)) << 2) 

// Режимы масштабирования изображения (выбираются в меню View / Вид)
enum ZoomMode {
	ZOOM_NEAREST    = 0,	// "по соседним" пикселям (без интерполяции),
							// режим растрирования COLORONCOLOR
	ZOOM_LINEAR     = 1,	// линейная интерполяция значений пикселов
	ZOOM_SPLINE     = 2		// сплайновая (кубическая Catmull-Rom) интерполяция
};

class CRaster
{
	LPBITMAPINFO		m_pBMI;   		//указатель на описание изображения
	PBYTE				m_pData;		//указатель на начало растровых данных
public:
	CRaster();
	~CRaster();
	void Clear(); //очистка памяти
	// Возвращает
	LPBITMAPINFO GetBMInfoPtr(){return m_pBMI;}	//указатель на заголовок растра
	RGBQUAD* GetBMColorTabPtr();				//указатель на таблицу цветов
	LONG GetBMWidth(); 							//ширину в пикселах;
	LONG GetBMHeight();							//высоту в пикселах;
	
	BYTE* GetBMDataPtr(){return m_pData;};		//указатель на растровые данные
	BYTE* GetPixPtr(LONG x, LONG y);			//указатель на пиксел 	

	// Загружает из файла
	BOOL LoadBMP(CString FileName);
	// Выводит DIB на контекст pDC
	// x, y - позиция левого верхнего угла области назначения
	// cx, cy - размер области назначения
	// x0, y0 - позиция левого верхнего угла выводимой части изображения
	// cx0, cy0 - размер выводимой части изображения
	// str_mode - режим масштабирования
	// rop - растровая операция определяет способ наложения изображения
	void DrawBitmap(CDC *pDC, LONG x=0, LONG y=0, LONG cx=0, LONG cy=0,
								 	LONG x0=0, LONG y0=0, LONG cx0=0, LONG cy0=0, int str_mode=COLORONCOLOR, DWORD rop=SRCCOPY);
	// Выводит DIB на контекст pDC с позиции (x,y) в масштабе scale
	void DrawBitmap(CDC *pDC, LONG x, LONG y, double scale, int str_mode=COLORONCOLOR, DWORD rop=SRCCOPY);
	// Выводит DIB на контекст pDC с позиции (x,y) в масштабе scale,
	// используя выбранный режим масштабирования (см. enum ZoomMode).
	// ZOOM_NEAREST - встроенное растрирование GDI ("по соседним"),
	// ZOOM_LINEAR / ZOOM_SPLINE - интерполяционное масштабирование
	// с предварительной пересчеткой растра (метод Stretch).
	void DrawZoomed(CDC *pDC, LONG x, LONG y, double scale, int zoom_mode, DWORD rop=SRCCOPY);

	// Масштабирует изображение с указанным режимом интерполяции.
	// Результат пишется в pDest; если pDest==NULL - в этот объект.
	// Поддерживаются 8/24/32-битные растры (черно-белые, индексируемые и цветные).
	BOOL Stretch(CRaster *pDest, double scale, int zoom_mode);
	BOOL Stretch(CRaster *pDest, LONG dstw, LONG dsth, int zoom_mode);
	int GetPixelDepth();	// разрядность пиксела: 1, 8 или 24 (32 -> 24+альфа)
	
	// Записывает BMP в файл
	BOOL SaveBMP(CString FileName);
	// Создает копию
	BOOL CreateCopy(CRaster *pOrg);
	// Создает растр заданного размера, совместимый с параметрами BITMAPINFO 
	BOOL CreateCompatible(LPBITMAPINFO pBMI, LONG width=0, LONG height=0);
	// Возвращает гисторамму изображения
	BOOL GetHistogham(DWORD *pHist, int Range);
};
#endif
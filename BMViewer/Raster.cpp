// Raster.cpp : implementation of the CRaster class
// (C) Alexey Polyakov 2002
////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Raster.h"

CRaster::CRaster()
{
	m_pData=NULL;
	m_pBMI=NULL;
}

CRaster::~CRaster()
{
	Clear();
};

void CRaster::Clear()
{

	if(m_pData!=NULL) delete[] m_pData; 
	m_pData=NULL;

	if(m_pBMI!=NULL) delete[] m_pBMI; 
	m_pBMI=NULL;
	
}; 

RGBQUAD* CRaster::GetBMColorTabPtr()
{
	return(LPRGBQUAD)(((BYTE*)(m_pBMI))+sizeof(BITMAPINFOHEADER));
};


LONG CRaster::GetBMWidth()
{
	if(m_pBMI==NULL) return 0;
	return m_pBMI->bmiHeader.biWidth;
};

LONG CRaster::GetBMHeight()
{
	if(m_pBMI==NULL) return 0;
	return m_pBMI->bmiHeader.biHeight;
};

BYTE* CRaster::GetPixPtr(LONG x, LONG y)
{
	if(	x<0 || x>= m_pBMI->bmiHeader.biWidth ||
		y<0 || y>= m_pBMI->bmiHeader.biHeight ||
		m_pData == NULL)
		return NULL;
	return (m_pData+(BYTESPERLINE(m_pBMI->bmiHeader.biWidth, m_pBMI->bmiHeader.biBitCount)*y +
			x*m_pBMI->bmiHeader.biBitCount/8));
};

BOOL CRaster::LoadBMP(CString FileName)
{
	//Очистим
	Clear();
	//Открываем файл
	CFile File;
	if(!File.Open(FileName, CFile::modeRead)) return FALSE; 
	
	////////////////////////////////////////////////////////
	//Загружаем изображение
	//прочитать заголовок файла. Это дает его размер и положение
	//начала данных
	BITMAPFILEHEADER	FI;
	File.Read(&FI, sizeof(BITMAPFILEHEADER));
	
	//Проверяем Windows Bitmap изображение ?
	if(FI.bfType!=0x4D42)
	{ File.Close(); return FALSE;}

	//Смещаем позицию
	File.Seek(sizeof(BITMAPFILEHEADER), CFile::begin);
	
	//Считаем, что все от заголовка файла до начала растровых данных
	//есть BITMAPINFO
	//выделяем память под заголовок
	m_pBMI=(LPBITMAPINFO)new BYTE[FI.bfOffBits-sizeof(BITMAPFILEHEADER)];
	if(m_pBMI==NULL) { File.Close(); return FALSE;}
	//Читаем BITMAPINFO 
	File.Read(m_pBMI, FI.bfOffBits-sizeof(BITMAPFILEHEADER));
	
	//Умеем работать только с несжатыми данными
	if(m_pBMI->bmiHeader.biCompression!=0) 
		{ File.Close(); return FALSE;}

	//переход к началу данных
	File.Seek(FI.bfOffBits, CFile::begin);
	//выделяем память под данные
	//расчет размера
	if(m_pBMI->bmiHeader.biSizeImage==0)
		m_pBMI->bmiHeader.biSizeImage=BYTESPERLINE(	m_pBMI->bmiHeader.biWidth,
													m_pBMI->bmiHeader.biBitCount)*
													m_pBMI->bmiHeader.biHeight;	

	m_pData= new BYTE[m_pBMI->bmiHeader.biSizeImage]; 
	if(m_pData==NULL) { File.Close(); return FALSE;}
	//Читаем данные 
	File.Read(m_pData, m_pBMI->bmiHeader.biSizeImage);

	File.Close();
	return TRUE;
};


void CRaster::DrawBitmap(CDC *pDC,	LONG x/*=0*/, LONG y/*=0*/, LONG cx/*=0*/, LONG cy/*=0*/,
									LONG x0/*=0*/, LONG y0/*=0*/, LONG cx0/*=0*/, LONG cy0/*=0*/,
									int str_mode/*=COLORONCOLOR*/, DWORD rop /*=SRCCOPY*/)
{
	if(m_pBMI==NULL || m_pData==NULL) return;
	//размеры не заданы - габариты в пикселях
	if(cx==0) cx=GetBMWidth();
	if(cy==0) cy=GetBMHeight();
	
	if(cx0==0) cx0=GetBMWidth();
	if(cy0==0) cy0=GetBMHeight();
	HDC  hdc=pDC->GetSafeHdc();
	if(hdc==NULL) return;
	// Установка режима масштабирования
	int oldStretchMode=pDC->SetStretchBltMode(str_mode);
	::StretchDIBits(hdc,
		x, 	y,	cx,	cy,			// область назначения
		x0, y0,	cx0, cy0,		// начало и размер выводимого изображения
		m_pData,				// данные
		m_pBMI,					// заголовок
		DIB_RGB_COLORS,			// опции
		rop);					// код РОП
	if(oldStretchMode!=0)
		pDC->SetStretchBltMode(oldStretchMode);
};

void CRaster::DrawBitmap(CDC *pDC,	LONG x, LONG y, double scale,
						 int str_mode/*=COLORONCOLOR*/, DWORD rop /*=SRCCOPY*/)
{
	if(m_pBMI==NULL || m_pData==NULL) return;
	
	LONG x0=0, y0=0;
	
	LONG cx0=GetBMWidth();
	LONG cy0=GetBMHeight();

	LONG cx=static_cast<LONG>(scale*cx0+0.5);
	LONG cy=static_cast<LONG>(scale*cy0+0.5);

	DrawBitmap(pDC,	x, y, cx, cy, x0, y0, cx0, cy0, str_mode, rop);
};

int CRaster::GetPixelDepth()
{
	if(m_pBMI==NULL) return 0;
	switch(m_pBMI->bmiHeader.biBitCount){
		case 1:	return 1;
		case 4:	return 4;
		case 8:	return 8;
		case 24: return 24;
		case 32: return 32;
	}
	return 0;
};

//////////////////////////////////////////////////////////////////////////
// Интерполяторы, используемые при масштабировании.
// Все они работают с "каналами" - целочисленными значениями яркости
// (для цветного изображения каждый канал R,G,B обрабатывается отдельно),
// поэтому одинаково пригодны как для цветных, так и для черно-белых картинок.

// Линейная интерполяция между двумя отсчетами
static inline double InterpLinear(double p0, double p1, double t)
{
	return p0 + (p1-p0)*t;
}

// Кубический сплайн Catmull-Rom по четырем соседним отсчетам
static inline double InterpSpline(double p_1, double p0, double p1, double p2, double t)
{
	return p0 + 0.5*t*(p1 - p_1 + t*(2.0*p_1 - 5.0*p0 + 4.0*p1 - p2 +
		   t*(3.0*(p0 - p1) + p2 - p_1)));
}

// Количество каналов, обрабатываемых при интерполяции
static inline int ChannelsOf(int depth)
{
	switch(depth){
		case 8: return 1;
		case 24: return 3;
		case 32: return 4;
	}
	return 0;
}

// Выборки исходного растра с граничным отражением индексов
static inline long ReflectIndex(long i, long n)
{
	if(i<0)	i=-i;				// отражение от границы
	if(i>=n) i=2*(n-1)-i;
	if(i<0) i=0;
	return i;
}

// Класс выборки значений из масштабируемого растра.
// Данные предварительно переводятся в "канальное" представление (массив
// double по 4 канала на пиксел), что позволяет одинаково просто
// интерполировать и цветные (24/32 бита), и черно-белые/индексированные
// (8 бит) изображения.
class SrcSampler
{
	const CRaster *m_pSrc;
	double *m_pBuf;		// [(y*m_w + x)*4 + channel]
	LONG m_w, m_h;
	int m_nch;			// число значимых каналов (1, 3 или 4)
public:
	SrcSampler(const CRaster *pSrc): m_pSrc(pSrc), m_pBuf(NULL),
		m_w(0), m_h(0), m_nch(0){}
	~SrcSampler(){ if(m_pBuf) delete[] m_pBuf; }

	BOOL Init()
	{
		if(m_pSrc==NULL) return FALSE;
		m_w=m_pSrc->GetBMWidth();
		m_h=m_pSrc->GetBMHeight();
		if(m_w<=0 || m_h<=0) return FALSE;
		int depth=m_pSrc->GetPixelDepth();
		m_nch=ChannelsOf(depth);
		if(m_nch==0) return FALSE;
		m_pBuf=new double[(DWORD)m_w*(DWORD)m_h*4];
		if(m_pBuf==NULL) return FALSE;
		// число байт на канал в исходном растре
		int bytes=(depth==8)?1:(depth==24?3:4);
		for(LONG y=0; y<m_h; y++){
			BYTE *pLine=m_pSrc->GetPixPtr(0, y);
			if(pLine==NULL) return FALSE;
			for(LONG x=0; x<m_w; x++){
				double *d=m_pBuf+((DWORD)y*m_w+x)*4;
				d[0]=d[1]=d[2]=d[3]=0.0;
				for(int ch=0; ch<m_nch; ch++) d[ch]=(double)pLine[x*bytes+ch];
			}
		}
		return TRUE;
	}

	// Выборка канала с отражением индексов за границами изображения
	inline const double* Pix(LONG x, LONG y) const
	{
		return m_pBuf+((DWORD)ReflectIndex(y, m_h)*(DWORD)m_w + ReflectIndex(x, m_w))*4;
	}

	// Линейная (билинейная) интерполяция канала ch в точке (sx,sy)
	double Linear(double sx, double sy, int ch) const
	{
		long x0=(long)floor(sx);
		long y0=(long)floor(sy);
		double tx=sx-x0, ty=sy-y0;
		double v00=Pix(x0,   y0  )[ch], v10=Pix(x0+1, y0  )[ch];
		double v01=Pix(x0,   y0+1)[ch], v11=Pix(x0+1, y0+1)[ch];
		double vx0=InterpLinear(v00, v10, tx);
		double vx1=InterpLinear(v01, v11, tx);
		return InterpLinear(vx0, vx1, ty);
	}

	// Бикубическая сплайновая (Catmull-Rom) интерполяция канала ch
	double Spline(double sx, double sy, int ch) const
	{
		long x0=(long)floor(sx);
		long y0=(long)floor(sy);
		double tx=sx-x0, ty=sy-y0;
		double rows[4];
		for(int j=-1; j<=2; j++){
			double r[4];
			for(int i=-1; i<=2; i++)
				r[i+1]=Pix(x0+i, y0+j)[ch];
			rows[j+1]=InterpSpline(r[0], r[1], r[2], r[3], tx);
		}
		return InterpSpline(rows[0], rows[1], rows[2], rows[3], ty);
	}
};

// Запись значения канала в пиксел dst (x,y). Значение округляется и
// ограничивается диапазоном [0,255]
static void PutChannelValue(CRaster *dst, LONG x, LONG y, int ch, double v)
{
	BYTE *p = dst->GetPixPtr(x, y);
	if(p==NULL) return;
	if(v<0.0) v=0.0;
	if(v>255.0) v=255.0;
	BYTE b=(BYTE)(v+0.5);
	int depth = dst->GetPixelDepth();
	switch(depth){
		case 8:	p[0]=b; break;
		case 24: case 32: p[ch]=b; break;
	}
}


//////////////////////////////////////////////////////////////////////////
// Масштабирование ("растягивание") растра с указанным режимом интерполяции
BOOL CRaster::Stretch(CRaster *pDest, double scale, int zoom_mode)
{
	if(scale<=0.0) return FALSE;
	LONG dstw=static_cast<LONG>(GetBMWidth()*scale+0.5);
	LONG dsth=static_cast<LONG>(GetBMHeight()*scale+0.5);
	if(dstw<1) dstw=1;
	if(dsth<1) dsth=1;
	return Stretch(pDest, dstw, dsth, zoom_mode);
}

BOOL CRaster::Stretch(CRaster *pDest, LONG dstw, LONG dsth, int zoom_mode)
{
	CRaster *pd = (pDest==NULL) ? this : pDest;
	if(pd==NULL) return FALSE;
	if(m_pBMI==NULL || m_pData==NULL) return FALSE;
	if(dstw<1 || dsth<1) return FALSE;

	int depth=GetPixelDepth();
	// Для 1/4-битных (двуцветных) растров интерполяция значений пикселов
	// не имеет смысла - используется режим "по соседним"
	if(depth!=8 && depth!=24 && depth!=32)
		zoom_mode=ZOOM_NEAREST;

	// Режим "по соседним" реализуем средствами GDI (режим растрирования
	// COLORONCOLOR - выбор ближайшего пиксела источника без смешивания)
	if(zoom_mode==ZOOM_NEAREST){
		HBITMAP hBmp=CreateCompatibleBitmap(NULL, dstw, dsth);
		if(hBmp==NULL) return FALSE;
		HDC memDC=CreateCompatibleDC(NULL);
		if(memDC==NULL){ DeleteObject(hBmp); return FALSE; }
		HBITMAP hOld=(HBITMAP)SelectObject(memDC, hBmp);
		SetStretchBltMode(memDC, COLORONCOLOR);
		BOOL ok=StretchDIBits(memDC, 0, 0, dstw, dsth,
							  0, 0, GetBMWidth(), GetBMHeight(),
							  m_pData, m_pBMI, DIB_RGB_COLORS, SRCCOPY)!=0;
		SelectObject(memDC, hOld);
		DeleteDC(memDC);
		if(!ok){ DeleteObject(hBmp); return FALSE; }
		// Вытащим результат обратно в CRaster через GetDIBits
		BITMAPINFO dstBMI=*m_pBMI;
		dstBMI.bmiHeader.biWidth=dstw;
		dstBMI.bmiHeader.biHeight=dsth;
		dstBMI.bmiHeader.biSizeImage=BYTESPERLINE(dstw, dstBMI.bmiHeader.biBitCount)*dsth;
		dstBMI.bmiHeader.biCompression=BI_RGB;
		if(!pd->CreateCompatible(&dstBMI)) { DeleteObject(hBmp); return FALSE; }
		ok=GetDIBits(GetDC(NULL), hBmp, 0, dsth, pd->GetBMDataPtr(), &dstBMI, DIB_RGB_COLORS)!=0;
		DeleteObject(hBmp);
		return ok;
	}

	// Интерполяционные режимы: линейный и сплайновый
	// Подготовка: получаем выходной формат - совместимый растр того же типа
	BITMAPINFO dstBMI=*m_pBMI;
	dstBMI.bmiHeader.biWidth=dstw;
	dstBMI.bmiHeader.biHeight=dsth;
	dstBMI.bmiHeader.biSizeImage=BYTESPERLINE(dstw, dstBMI.bmiHeader.biBitCount)*dsth;
	dstBMI.bmiHeader.biCompression=BI_RGB;
	if(!pd->CreateCompatible(&dstBMI)) return FALSE;

	SrcSampler sampler(this);
	if(!sampler.Init()) return FALSE;

	int nch=ChannelsOf(depth);
	double srcw=(double)GetBMWidth();
	double srch=(double)GetBMHeight();
	// центрирование пиксельных сеток
	for(LONG y=0; y<dsth; y++){
		double sy=((double)y + 0.5)*srch/(double)dsth - 0.5;
		for(LONG x=0; x<dstw; x++){
			double sx=((double)x + 0.5)*srcw/(double)dstw - 0.5;
			for(int ch=0; ch<nch; ch++){
				double v;
				if(zoom_mode==ZOOM_LINEAR){
					v=sampler.Linear(sx, sy, ch);
				} else { // ZOOM_SPLINE
					v=sampler.Spline(sx, sy, ch);
				}
				PutChannelValue(pd, x, y, ch, v);
			}
		}
	}
	// для 32-битных растров альфа-канал уже обработан (ch==3)
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// Вывод с учетом выбранного режима масштабирования
void CRaster::DrawZoomed(CDC *pDC, LONG x, LONG y, double scale, int zoom_mode, DWORD rop /*=SRCCOPY*/)
{
	if(m_pBMI==NULL || m_pData==NULL) return;
	if(pDC==NULL) return;

	// Масштаб 1:1 или режим "по соседним" - рисуем напрямую без пересчетки
	if(zoom_mode==ZOOM_NEAREST || scale==1.0){
		DrawBitmap(pDC, x, y, scale, COLORONCOLOR, rop);
		return;
	}

	// Интерполяция: сначала масштабируем растр, затем выводим 1:1
	CRaster Scaled;
	if(Scaled.Stretch(this, scale, zoom_mode)){
		Scaled.DrawBitmap(pDC, x, y, 1.0, COLORONCOLOR, rop);
		return;
	}
	// Если что-то пошло не так - fallback на GDI
	DrawBitmap(pDC, x, y, scale, HALFTONE, rop);
};


BOOL CRaster::SaveBMP(CString FileName)
{
	//Открываем файл
	CFile File;
	if(!File.Open(FileName, CFile::modeCreate|CFile::modeWrite)) return FALSE; 
	
	////////////////////////////////////////////////////////
	//Записываем изображение
	//Вычислим размер заголовка растра вместе с таблицей цветов
	DWORD SizeOfBMI= (DWORD)m_pBMI->bmiHeader.biSize + 
	m_pBMI->bmiHeader.biClrUsed*sizeof(RGBQUAD);
	
	// Заголовок файла
	BITMAPFILEHEADER	FI;
	// Идентификатор типа файла BMP: 0x42 = "B" 0x4d = "M"  
    FI.bfType = 0x4d42;        
    // Размер всего файла вместе с заголовками и данными
    FI.bfSize = (DWORD) sizeof(BITMAPFILEHEADER) + SizeOfBMI + 
				m_pBMI->bmiHeader.biSizeImage; 
    FI.bfReserved1 = 0; 
    FI.bfReserved2 = 0; 
    // Вычисляем смещение до начала растровых данных
    FI.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + SizeOfBMI; 
	// Записываем заголовок файла 
	File.Write(&FI, sizeof(BITMAPFILEHEADER));
	
		
	//Записываем BITMAPINFO вместе с таблицей цветов
	File.Write(m_pBMI, SizeOfBMI);
	
	//Данные 
	File.Write(m_pData, m_pBMI->bmiHeader.biSizeImage);
	
	File.Close();
	return TRUE;
};

BOOL CRaster::CreateCopy(CRaster *pOrg)
{
	Clear();
	if(!pOrg) return FALSE;

	LPBITMAPINFO pOrgBMI=pOrg->GetBMInfoPtr();
	//Вычислим размер заголовка растра вместе с таблицей цветов
	DWORD SizeOfBMI= (DWORD)pOrgBMI->bmiHeader.biSize + 
	pOrgBMI->bmiHeader.biClrUsed*sizeof(RGBQUAD);

	// Выделим память под заголовок растра
	m_pBMI=(LPBITMAPINFO)new BYTE[SizeOfBMI];
	if(!m_pBMI) return FALSE;
	// Копируем заголовок растра
	memcpy(m_pBMI, pOrg->GetBMInfoPtr(), SizeOfBMI);

	// Расчет размера памяти под данные
	if(m_pBMI->bmiHeader.biSizeImage==0)
		m_pBMI->bmiHeader.biSizeImage=BYTESPERLINE(	m_pBMI->bmiHeader.biWidth,
													m_pBMI->bmiHeader.biBitCount)*
													m_pBMI->bmiHeader.biHeight;	
	// Выделяем память под данные
	m_pData= new BYTE[m_pBMI->bmiHeader.biSizeImage]; 
	if(!m_pData) return FALSE;

	// Копируем данные
	memcpy(m_pData, pOrg->GetBMDataPtr(), m_pBMI->bmiHeader.biSizeImage);

	return TRUE;
};

BOOL CRaster::CreateCompatible(LPBITMAPINFO pBMI,	LONG width/*=0*/,
													LONG height/*=0*/)
{
	if(!pBMI) return FALSE;

	if(width==0)	width=pBMI->bmiHeader.biWidth;
	if(height==0)	height=pBMI->bmiHeader.biHeight;

	// Проверяем может существующий растр и так совместим
	if( m_pBMI!=NULL &&							// существует
		m_pBMI->bmiHeader.biWidth==width &&		// такого же размера
		m_pBMI->bmiHeader.biHeight==height &&	// и глубина цвета совпадает
		m_pBMI->bmiHeader.biBitCount==pBMI->bmiHeader.biBitCount
		)
		return TRUE; // растр и так совместим
	////////////////////////////////////////////////////////
	// Создаем совместимый растр
	Clear();
	//Вычислим размер заголовка растра вместе с таблицей цветов
	DWORD SizeOfBMI= (DWORD) pBMI->bmiHeader.biSize + 
					  pBMI->bmiHeader.biClrUsed*sizeof(RGBQUAD);

 	// Выделим память под заголовок растра
	m_pBMI=(LPBITMAPINFO)new BYTE[SizeOfBMI];
	if(!m_pBMI) return FALSE;
	
	// Копируем заголовок растра
	memcpy(m_pBMI, pBMI, SizeOfBMI);

	// Устанавливаем размер 
	m_pBMI->bmiHeader.biWidth=width;
	m_pBMI->bmiHeader.biHeight=height;

	// Расчет размера памяти под данные
	m_pBMI->bmiHeader.biSizeImage=BYTESPERLINE(	m_pBMI->bmiHeader.biWidth,
												m_pBMI->bmiHeader.biBitCount)*
												m_pBMI->bmiHeader.biHeight;	
	// Выделяем память под данные
	m_pData= new BYTE[m_pBMI->bmiHeader.biSizeImage]; 
	if(!m_pData) return FALSE;

	return TRUE;
};

BOOL CRaster::GetHistogham(DWORD *pHist, int Range)
{
	// Умеет работать только с данными RGB888
	if(m_pBMI->bmiHeader.biBitCount!=24) return FALSE;
	
	// Обнулим таблицу
	for(int i=0; i<Range; i++)
		pHist[i]=0;

	LONG DataStrLength=BYTESPERLINE(m_pBMI->bmiHeader.biWidth, m_pBMI->bmiHeader.biBitCount);
	BYTE *pCurPix=NULL;
	BYTE Brightness=0;
	for(int y=0, x=0; y<m_pBMI->bmiHeader.biHeight; y++)
		for(x=0; x<m_pBMI->bmiHeader.biWidth; x++)
	{
		// Адрес пикселя
		pCurPix=m_pData+y*DataStrLength+x*3;
	
		// Яркость рассчитывается как 0.3*Red+0.59*Green+0.11*Blue,
		// но пиксельные данные хранятся в файле BMP, в порядке BGR 
		Brightness=(BYTE)((	0.11*(*pCurPix) + 
							0.59*(*(pCurPix+1))	+
							0.3*(*(pCurPix+2)))*Range/256);
		pHist[Brightness]+=1;
	}
	return TRUE;
};


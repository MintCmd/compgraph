// ���� Shapes.cpp
////////////////////////////////////////
//���������� �������
#include "stdafx.h"
#include "shapes.h"
#include "global.h"
#include <atlimage.h>
#include <math.h>

////////////////////////////////////////
// ���������� ������� ������ CBasePoint
CBasePoint::CBasePoint(): CPoint(0, 0)
{
	m_wSize=1;
	
	m_iPenStyle=PS_SOLID;
	m_iPenWidth=1;
	m_rgbPenColor=RGB(0,0,0);
	m_iBrushStyle=-1; // �� ���������� ���������
	m_rgbBrushColor=RGB(0,0,0);
	m_dwPattern_ID=0; // ��� ������� �������
};

CRaster::CRaster(): CBasePoint()
{
	m_pImage = NULL;
	m_ImageWidth = 0;
	m_ImageHeight = 0;
	m_ImagePath.Empty();
}

CRaster::~CRaster()
{
	if(m_pImage!=NULL)
	{
		delete m_pImage;
		m_pImage = NULL;
	}
}

IMPLEMENT_SERIAL(CRaster, CBasePoint, 1)
void CRaster::Serialize(CArchive &ar)
{
	CBasePoint::Serialize(ar);
	if(ar.IsStoring())
	{
		ar << m_ImagePath;
		ar << m_ImageWidth;
		ar << m_ImageHeight;
	}
	else
	{
		ar >> m_ImagePath;
		ar >> m_ImageWidth;
		ar >> m_ImageHeight;
		if(!m_ImagePath.IsEmpty())
			LoadFromFile(m_ImagePath);
	}
}

BOOL CRaster::CreateSampleImage(int width, int height)
{
	if(width<=0 || height<=0) return FALSE;
	if(m_pImage==NULL) m_pImage = new ATL::CImage();
	if(FAILED(m_pImage->Create(width, height, 32))) return FALSE;

	for(int yy=0; yy<height; yy++)
		for(int xx=0; xx<width; xx++)
		{
			COLORREF color = (((xx/8)+(yy/8))%2==0) ? RGB(60,120,220) : RGB(245,220,80);
			m_pImage->SetPixel(xx, yy, color);
		}

	m_ImagePath.Empty();
	m_ImageWidth = width;
	m_ImageHeight = height;
	return TRUE;
}

BOOL CRaster::LoadFromFile(LPCTSTR fileName)
{
	if(fileName==NULL || fileName[0]==0) return FALSE;
	if(m_pImage==NULL) m_pImage = new ATL::CImage();
	if(FAILED(m_pImage->Load(fileName))) return FALSE;

	m_ImagePath = fileName;
	m_ImageWidth = m_pImage->GetWidth();
	m_ImageHeight = m_pImage->GetHeight();
	return TRUE;
}

void CRaster::SetSize(int width, int height)
{
	if(width>0) m_ImageWidth = width;
	if(height>0) m_ImageHeight = height;
}

void CRaster::Show(CDC *pDC)
{
	if(pDC==NULL || m_pImage==NULL || m_pImage->IsNull()) return;
	int width = m_ImageWidth>0 ? m_ImageWidth : m_pImage->GetWidth();
	int height = m_ImageHeight>0 ? m_ImageHeight : m_pImage->GetHeight();
	if(width<=0 || height<=0) return;

	int oldMode = pDC->SetStretchBltMode(COLORONCOLOR);
	m_pImage->Draw(pDC->GetSafeHdc(), x, y, width, height);
	pDC->SetStretchBltMode(oldMode);
}

void CRaster::GetRegion(CRgn &Rgn)
{
	int width = m_ImageWidth;
	int height = m_ImageHeight;
	if(m_pImage!=NULL && !m_pImage->IsNull())
	{
		if(width<=0) width = m_pImage->GetWidth();
		if(height<=0) height = m_pImage->GetHeight();
	}
	Rgn.CreateRectRgn(x, y, x+width, y+height);
}

IMPLEMENT_SERIAL(CVariant6, CBasePoint, 1)
void CVariant6::Serialize(CArchive &ar)
{
	// делегируем базовой сериализации
	CBasePoint::Serialize(ar);
}

CVariant6::CVariant6(): CBasePoint()
{
	// радиус по умолчанию
	m_wSize = 10;
}

void CVariant6::Show(CDC* pDC)
{
	// вычисления позиций
	int r = (m_wSize>1) ? (int)m_wSize : 1;
	const double sqrt3 = sqrt(3.0);
	double shiftY = (2.0*sqrt3/5.0) * (double)r;
	double topY = (double)y - shiftY;
	double bottomY = topY + sqrt3 * r;

	// координаты центров
	CPoint centers[5];
	centers[0] = CPoint( (LONG)(x - 2.0*r + 0.5), (LONG)(bottomY + 0.5) ); // синий
	centers[1] = CPoint( (LONG)(x + 0.5),                 (LONG)(bottomY + 0.5) ); // красный
	centers[2] = CPoint( (LONG)(x + 2.0*r + 0.5), (LONG)(bottomY + 0.5) ); // жёлтый
	centers[3] = CPoint( (LONG)(x - 1.0*r + 0.5), (LONG)(topY + 0.5) ); // зелёный
	centers[4] = CPoint( (LONG)(x + 1.0*r + 0.5), (LONG)(topY + 0.5) ); // розовый

	if(HBRUSH(m_Brush) != NULL)
	{
		if(!PrepareDC(pDC)) return;
		for(int i=0;i<5;i++)
		{
			int cx = centers[i].x; int cy = centers[i].y;
			pDC->Ellipse(cx - r, cy - r, cx + r, cy + r);
		}
		RestoreDC(pDC);
		return;
	}

	COLORREF cols[5] = {
		RGB(0,0,255),   // синий
		RGB(255,0,0),   // красный
		RGB(255,255,0), // жёлтый
		RGB(0,128,0),   // зелёный
		RGB(255,0,255)  // розовый
	};

	for(int i=0;i<5;i++)
	{
		CBrush br;
		br.CreateSolidBrush(cols[i]);
		CPen pen(PS_SOLID, 1, RGB(0,0,0));
		CBrush* pOldBr = pDC->SelectObject(&br);
		CPen* pOldPen = pDC->SelectObject(&pen);
		int cx = centers[i].x; int cy = centers[i].y;
		pDC->Ellipse(cx - r, cy - r, cx + r, cy + r);
		pDC->SelectObject(pOldPen);
		pDC->SelectObject(pOldBr);
	}
}

void CVariant6::GetRegion(CRgn &Rgn)
{
	double r = (double)m_wSize;
	const double sqrt3 = sqrt(3.0);
	double shiftY = (2.0*sqrt3/5.0) * r;
	double topY = (double)y - shiftY;
	double bottomY = topY + sqrt3 * r;

	int left = (int)(x - 2.0*r - r + 0.5); // левый край: центр левого верхнего - r
	int right = (int)(x + 2.0*r + r + 0.5); // правый край: центр правого верхнего + r
	int top = (int)(topY - r + 0.5);
	int bottom = (int)(bottomY + r + 0.5);
	Rgn.CreateRectRgn(left, top, right, bottom);
}

CBasePoint::CBasePoint(int x, int y, WORD s):CPoint(x, y)
{
	m_wSize=s;

	m_iPenStyle=PS_SOLID;
	m_iPenWidth=1;
	m_rgbPenColor=RGB(0,0,0);
	m_iBrushStyle=-1; // �� ���������� ���������
	m_rgbBrushColor=RGB(0,0,0);
	m_dwPattern_ID=0; // ��� ������� �������
};

IMPLEMENT_SERIAL(CBasePoint, CObject , VERSIONABLE_SCHEMA|1)
void CBasePoint::Serialize(CArchive &ar)
{
	if(ar.IsStoring()) // ����������
	{
		// ��������� ��������� �������
		ar<<x;
		ar<<y;
		ar<<m_wSize;
		ar<<m_iPenStyle;
		ar<<m_iPenWidth;
		ar<<m_rgbPenColor;
		ar<<m_iBrushStyle;
		ar<<m_rgbBrushColor;
		ar<<m_dwPattern_ID;
	}
	else	// ������
	{
		// �������� ������ �������
		int Version=ar.GetObjectSchema();
		// � ����������� �� ������
		// ����� ��������� ��������� �������� ��������
		// ��������� ��������� �������
		ar>>x;
		ar>>y;
		ar>>m_wSize;
		ar>>m_iPenStyle;
		ar>>m_iPenWidth;
		ar>>m_rgbPenColor;
		ar>>m_iBrushStyle;
		ar>>m_rgbBrushColor;
		ar>>m_dwPattern_ID;

		SetPen(m_rgbPenColor, m_iPenWidth, m_iPenStyle);
		SetBrush(m_rgbBrushColor, m_dwPattern_ID, m_iBrushStyle );
	}
};
BOOL CBasePoint::SetPen(COLORREF color, int width /*=1*/, int style/*=PS_SOLID*/)
{
	m_iPenStyle=style;
	m_iPenWidth=width;
	m_rgbPenColor=color;
	if(HPEN(m_Pen)!=NULL)	// ���� ���� ��� ����������
		if(!m_Pen.DeleteObject()) return FALSE; // ������� ������ ����
	// ������� ����� ���� � ���������� ���������
	return m_Pen.CreatePen( m_iPenStyle, m_iPenWidth, m_rgbPenColor);
};
BOOL CBasePoint::SetBrush(COLORREF color, DWORD pattern /*=0*/, int style/*=-1*/)
{
	m_iBrushStyle=style;
	m_dwPattern_ID=pattern;
	m_rgbBrushColor=color;
	int res=1;
	if(HBRUSH(m_Brush)!=NULL)	// ���� ����� ��� ����������
		if(!m_Brush.DeleteObject()) return FALSE; // ������� ������ �����
	if(m_dwPattern_ID>0)	// ���� ������ �������
	{
		CBitmap Pattern;
		if(!Pattern.LoadBitmap(m_dwPattern_ID)) return FALSE;
		return m_Brush.CreatePatternBrush(&Pattern);
	}
	if(m_iBrushStyle>=0)	// ������ ����� ���������
		return m_Brush.CreateHatchBrush( m_iBrushStyle, m_rgbBrushColor);
	// ������� �������� ����� � ���������� ���������
	return m_Brush.CreateSolidBrush(m_rgbBrushColor);
};



BOOL CBasePoint::PrepareDC(CDC *pDC)
{
	// ��������� ��������� ��������� ����������
	if(!pDC->SaveDC()) return FALSE;
	// ������������� ���� � ����� 
	if(HPEN(m_Pen)!=NULL)
		pDC->SelectObject(&m_Pen);
	if(HBRUSH(m_Brush)!=NULL)
		pDC->SelectObject(&m_Brush);
	return TRUE;
};

BOOL CBasePoint::RestoreDC(CDC *pDC)
{
	// ��������������� ��������� ��������� ����������
	return pDC->RestoreDC(-1);
};

void CBasePoint::Show(CDC* pDC) 
{
	// ������������� ����� � �����
	PrepareDC(pDC);
	// ������ ������, ������������ �����
	pDC->Ellipse(x-m_wSize, y-m_wSize, x+m_wSize, y+m_wSize);
	// ��������������� ��������
	RestoreDC(pDC);
} 

void CBasePoint::GetRegion(CRgn &Rgn)
{
	Rgn.CreateEllipticRgn(x-m_wSize, y-m_wSize, x+m_wSize, y+m_wSize);
}

void CBasePoint::Transform(const CPoint &point0, double ang, int a, int b)
{
	CPoint res=::Transform(CPoint(x, y), CPoint(0,0), 0, a, b);
	 x=res.x; y=res.y;
};

BOOL CBasePoint::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags, UINT nMyFlags) 
{
	BOOL res=TRUE;
	if(nMyFlags & SHIFT_HOLD) //�������
		switch(nChar)
		{
			case 37: 
				Transform(CPoint(0,0), -ROTATE_STEP, 0, 0);
				break;
			case 39:
				Transform(CPoint(0,0), ROTATE_STEP, 0, 0);
				break;
			default:
				res=FALSE;
		}
	else //�������
		switch(nChar)
		{
			case 38: // �����
				Transform(CPoint(0,0), 0, 0, MOVE_STEP);
				break; 
			case 40: // ����
				Transform(CPoint(0,0), 0, 0, -MOVE_STEP); 
				break;
			case 37: // �����
				Transform(CPoint(0,0), 0, -MOVE_STEP, 0);
				break;
			case 39: // ������
				Transform(CPoint(0,0), 0, MOVE_STEP, 0);
				break;
			default:
				res=FALSE;
		}	
	return res;
}

////////////////////////////////////////
// ���������� ������� ������ CSquare

CSquare::CSquare(int x, int y, WORD s): CBasePoint(x, y, s)
{
	m_wSize=s;
} 

CSquare::CSquare(): CBasePoint()
{
	m_wSize=40;
} 

IMPLEMENT_SERIAL(CSquare, CObject, 1)
void CSquare::Serialize(CArchive &ar)
{
	CBasePoint::Serialize(ar);
}
void CSquare::Show(CDC* pDC) 
{
	int s=m_wSize/2;
	// ������������� ����� � �����
	PrepareDC(pDC);
	// ������ �������
	pDC->Rectangle(x-s, y-s, x+s, y+s);
	// ��������������� ��������
	RestoreDC(pDC);
} 

void CSquare::GetRegion(CRgn &Rgn)
{
	int s=m_wSize/2;
	Rgn.CreateRectRgn(x-s, y-s, x+s, y+s);
}


////////////////////////////////////////
// ���������� ������� ������ CPolygon

CPolygon::CPolygon(): CBasePoint()
{
	m_wSize=0;
	m_bPolygon=FALSE;
} 

CPolygon::~CPolygon()
{
	m_PointsArray.RemoveAll( );
} 


IMPLEMENT_SERIAL(CPolygon, CObject, 1)
void CPolygon::Serialize(CArchive &ar)
{
	if(ar.IsStoring()) // ����������
	{
		// ��������� ��������� �������
		ar<<m_bPolygon;
	}
	else	// ������
	{
		// �������� ������ �������
		int Version=ar.GetObjectSchema();
		// � ����������� �� ������
		// ����� ��������� ��������� �������� ��������
		// ��������� ��������� �������
		ar>>m_bPolygon;
	}	
	m_PointsArray.Serialize(ar);
	CBasePoint::Serialize(ar);
}
void CPolygon::Show(CDC* pDC) 
{
	// ������������� ����� � �����
	PrepareDC(pDC);
	// ������ 
	if(m_bPolygon)
		pDC->Polygon(m_PointsArray.GetData(), m_PointsArray.GetSize());
	else
		pDC->Polyline( m_PointsArray.GetData(), m_PointsArray.GetSize());
	// ��������������� ��������
	RestoreDC(pDC);
} 

void CPolygon::GetRegion(CRgn &Rgn)
{
	Rgn. CreatePolygonRgn(m_PointsArray.GetData(), m_PointsArray.GetSize(), ALTERNATE);
}

void CPolygon::Transform(const CPoint &point0, double ang, int a, int b)
{
	for(int i=0; i<m_PointsArray.GetSize(); i++)
		m_PointsArray[i]=::Transform(m_PointsArray[i], m_PointsArray[0], ang, a, b);
};

// Вспомогательная функция Catmull-Rom интерполяции
static CPoint CatmullRomPoint(const CPoint &P0, const CPoint &P1, const CPoint &P2, const CPoint &P3, double t)
{
	double t2 = t*t;
	double t3 = t2*t;
	double x = 0.5*( (2.0*P1.x) + (-P0.x + P2.x)*t + (2.0*P0.x - 5.0*P1.x + 4.0*P2.x - P3.x)*t2 + (-P0.x + 3.0*P1.x - 3.0*P2.x + P3.x)*t3 );
	double y = 0.5*( (2.0*P1.y) + (-P0.y + P2.y)*t + (2.0*P0.y - 5.0*P1.y + 4.0*P2.y - P3.y)*t2 + (-P0.y + 3.0*P1.y - 3.0*P2.y + P3.y)*t3 );
	return CPoint((int)floor(x+0.5), (int)floor(y+0.5));
}

IMPLEMENT_SERIAL(CSpline, CPolygon, 1)
void CSpline::Serialize(CArchive &ar)
{
	CPolygon::Serialize(ar);
}

void CSpline::Show(CDC *pDC)
{
	// Если нет базовых точек - ничего не рисуем
	int n = m_PointsArray.GetSize();
	if(n<2) return;

	// Соберём точки для Catmull-Rom: дублируем концы
	CArray<CPoint,CPoint> pts;
	pts.SetSize(n+2);
	pts[0]=m_PointsArray[0];
	for(int i=0;i<n;i++) pts[i+1]=m_PointsArray[i];
	pts[n+1]=m_PointsArray[n-1];

	// Подготовка DC (перо/кисть объекта)
	BOOL prepared = FALSE;
	prepared = this->PrepareDC(pDC);

	// Собираем сэмплированные точки
	CArray<CPoint,CPoint> samp;
	const int STEPS = 12; // точек на сегмент
	for(int i=0;i<n-1;i++)
	{
		CPoint P0 = pts[i];
		CPoint P1 = pts[i+1];
		CPoint P2 = pts[i+2];
		CPoint P3 = pts[i+3];
		for(int s=0;s<STEPS;s++)
		{
			double t = (double)s / (double)STEPS;
			samp.Add(CatmullRomPoint(P0,P1,P2,P3,t));
		}
	}
	// Добавить последний базовый
	samp.Add(m_PointsArray[n-1]);

	// Рисуем сплайн как полилинию
	pDC->Polyline(samp.GetData(), samp.GetSize());

	if(prepared) this->RestoreDC(pDC);
}

void CSpline::GetRegion(CRgn &Rgn)
{
	// приближённый ограничивающий прямоугольник
	if(m_PointsArray.GetSize()==0) { Rgn.CreateRectRgn(0,0,0,0); return; }
	CRect rc(m_PointsArray[0].x, m_PointsArray[0].y, m_PointsArray[0].x, m_PointsArray[0].y);
	for(int i=1;i<m_PointsArray.GetSize();i++) rc |= CRect(m_PointsArray[i].x, m_PointsArray[i].y, m_PointsArray[i].x, m_PointsArray[i].y);
	Rgn.CreateRectRgn(rc.left-5, rc.top-5, rc.right+5, rc.bottom+5);
}

CPatternPolygon::CPatternPolygon(): CPolygon()
{
	m_bPatternCreated = FALSE;
	SetPolygon(TRUE);
}

IMPLEMENT_SERIAL(CPatternPolygon, CPolygon, 1)
void CPatternPolygon::Serialize(CArchive &ar)
{
	CPolygon::Serialize(ar);
	if(!ar.IsStoring())
		m_bPatternCreated = FALSE;
}

BOOL CPatternPolygon::CreatePatternBrush(CDC *pDC)
{
	if(pDC==NULL) return FALSE;
	if(HBITMAP(m_PatternBitmap)!=NULL) m_PatternBitmap.DeleteObject();
	if(HBRUSH(m_PatternBrush)!=NULL) m_PatternBrush.DeleteObject();

	const int patternSize = 12;
	if(!m_PatternBitmap.CreateCompatibleBitmap(pDC, patternSize, patternSize)) return FALSE;
	CDC patternDC;
	if(!patternDC.CreateCompatibleDC(pDC)) return FALSE;
	CBitmap *pOldBitmap = patternDC.SelectObject(&m_PatternBitmap);

	patternDC.FillSolidRect(0, 0, patternSize, patternSize, RGB(255,235,180));
	CBrush darkBrush(RGB(65,140,210));
	CBrush lightBrush(RGB(120,200,240));
	for(int yy=0; yy<patternSize; yy+=6)
		for(int xx=0; xx<patternSize; xx+=6)
		{
			CBrush *pOldBrush = patternDC.SelectObject(((xx/6+yy/6)%2)==0 ? &darkBrush : &lightBrush);
			patternDC.Rectangle(xx, yy, xx+6, yy+6);
			patternDC.SelectObject(pOldBrush);
		}

	patternDC.SelectObject(pOldBitmap);
	m_bPatternCreated = m_PatternBrush.CreatePatternBrush(&m_PatternBitmap);
	return m_bPatternCreated;
}

void CPatternPolygon::Show(CDC *pDC)
{
	if(pDC==NULL || m_PointsArray.GetSize()<2) return;
	if(!m_bPatternCreated) CreatePatternBrush(pDC);

	if(!PrepareDC(pDC)) return;
	if(HBRUSH(m_PatternBrush)!=NULL)
		pDC->SelectObject(&m_PatternBrush);
	pDC->Polygon(m_PointsArray.GetData(), m_PointsArray.GetSize());
	RestoreDC(pDC);
}

void CPatternPolygon::GetRegion(CRgn &Rgn)
{
	CPolygon::GetRegion(Rgn);
}

/////////////////////////////////////////////////
//3D Polygon

IMPLEMENT_SERIAL(C3DPolygon, CPolygon, -1)
void C3DPolygon::Serialize(CArchive &ar)
{
	CPolygon::Serialize(ar);
	
	if(ar.IsStoring())
	{
	}
	else
	{
	}
	m_3DPointsArray.Serialize(ar);
	
};

void C3DPolygon::MakeProjection(Perspective P)
{
	// ������� � �������
	P.theta=P.theta*atan(1.0)/45.0; P.phi=P.phi*atan(1.0)/45.0;
	// ������ ������������� ������� ��������������
	// ���� ���������� ����� ����������� MM_TEXT, ��� ������� ������ ���������
	// � ������� ����� ����, ��������� ���� �������� �� ��������������� ���� 
	// � ������������� ������� ������� ������� �������������� (����������� ��� Y)

	double  st=sin(P.theta), ct=cos(P.theta), sp=sin(P.phi), cp=cos(P.phi),
	v11=-st,	v12=-cp*ct,	v13=-sp*ct,
	v21=ct,		v22=-cp*st,	v23=-sp*st,
				v32=sp,		v33=-cp,
	v41=P.dx,	v42=P.dy,	v43=P.rho;
	double x, y, z;
	double TempZ=0;
	//������ ������� ��������� �����
	 m_PointsArray.SetSize(m_3DPointsArray.GetSize());
	for(int i=0; i<m_3DPointsArray.GetSize(); i++)
	{
		x=m_3DPointsArray[i].x-P.O.x;
		y=m_3DPointsArray[i].y-P.O.y;
		z=m_3DPointsArray[i].z-P.O.z;

		TempZ=v13*x+v23*y+v33*z+v43;
		m_PointsArray[i].x=(LONG)(v11*x+v21*y+v41+0.5);
		m_PointsArray[i].y=(LONG)(v12*x+v22*y+v32*z+v42+0.5);

		// ������������� ��������������
		if(P.with_perspective)
		{
			m_PointsArray[i].x=(LONG)(P.d*m_PointsArray[i].x/TempZ +0.5);
			m_PointsArray[i].y=(LONG)(P.d*m_PointsArray[i].y/TempZ +0.5);
		}
		
		m_PointsArray[i].x+=(LONG)(P.O.x +0.5);
		m_PointsArray[i].y+=(LONG)(P.O.y +0.5);
	}
};

//////////////////////////////////////////////////////
C3DShape::C3DShape(): CBasePoint()
{
	m_Percpective.O.x=0;
	m_Percpective.O.y=0;
	m_Percpective.O.z=0;
	m_Percpective.rho=50000; //50 ��. � ������ MM_HIMETRIC
	m_Percpective.theta=30;  
	m_Percpective.phi=30;
	m_Percpective.d=25000; //25 ��. � ������ MM_HIMETRIC
	m_Percpective.with_perspective=TRUE;
	m_Percpective.dx=0;
	m_Percpective.dy=0;
}

C3DShape::~C3DShape()
{
	while(m_PtrPolygonList.GetCount()>0)
		delete m_PtrPolygonList.RemoveHead();
	
};

IMPLEMENT_SERIAL(C3DShape, CBasePoint, -1)
void C3DShape::Serialize(CArchive &ar)
{
	if(ar.IsStoring())
	{
		ar << m_Percpective.O.x;
		ar << m_Percpective.O.y;
		ar << m_Percpective.O.z;
		ar << m_Percpective.rho;
		ar << m_Percpective.theta;
		ar << m_Percpective.phi;
		ar << m_Percpective.d;
		ar << m_Percpective.with_perspective;
		ar << m_Percpective.dx;
		ar << m_Percpective.dy;
	}
	else
	{
		ar >> m_Percpective.O.x;
		ar >> m_Percpective.O.y;
		ar >> m_Percpective.O.z;
		ar >> m_Percpective.rho;
		ar >> m_Percpective.theta;
		ar >> m_Percpective.phi;
		ar >> m_Percpective.d;
		ar >> m_Percpective.with_perspective;
		ar >> m_Percpective.dx;
		ar >> m_Percpective.dy;

	}
	m_PtrPolygonList.Serialize(ar);
};

void C3DShape::Show(CDC *pDC)
{
	// ����� ���� ���������
	POSITION Pos=NULL;
	if(m_PtrPolygonList.GetCount()>0)
		Pos=m_PtrPolygonList.GetHeadPosition();
	
	while(Pos!=NULL)
		m_PtrPolygonList.GetNext(Pos)->Show(pDC);
};


void C3DShape::GetRegion(CRgn &Rgn)
{
	// ������������  ������� ������� C3DShape,
	// � ���� ��������������, ������������� ����������� 
	// ������ �� ������
	CRect Frame; // ������������ �������������
	POSITION Pos=NULL;
	int i=0;
	CPolygon *pPolygon=NULL;
	if(m_PtrPolygonList.GetCount()>0)
		Pos=m_PtrPolygonList.GetHeadPosition();
	// �������������� ������������� ���������� ������ ����� ������� ��������
	if(Pos!=NULL && (pPolygon=m_PtrPolygonList.GetAt(Pos))!=NULL && pPolygon->m_PointsArray.GetSize()>0)
	{
		Frame.left=Frame.right=pPolygon->m_PointsArray[0].x;
		Frame.top=Frame.bottom=pPolygon->m_PointsArray[0].y;
	}
	else return;
	// �������� �������� ������	
	while(Pos!=NULL)
	{
		pPolygon=m_PtrPolygonList.GetNext(Pos);
		for(i=0; i<pPolygon->m_PointsArray.GetSize(); i++)
		{
			if(pPolygon->m_PointsArray[i].x<Frame.left) Frame.left=pPolygon->m_PointsArray[i].x;
			if(pPolygon->m_PointsArray[i].x>Frame.right) Frame.right=pPolygon->m_PointsArray[i].x;
			if(pPolygon->m_PointsArray[i].y>Frame.bottom) Frame.bottom=pPolygon->m_PointsArray[i].y;
			if(pPolygon->m_PointsArray[i].y<Frame.top) Frame.top=pPolygon->m_PointsArray[i].y;
		};
	}
	// ������� �������
	Rgn.CreateRectRgn(Frame.left, Frame.top, Frame.right, Frame.bottom);
}

	
void C3DShape::AddPolygon(C3DPolygon *pPolygon)
{
	m_PtrPolygonList.AddTail(pPolygon);	//�������� � ������
	// ������ ������
	POSITION Pos=NULL;
	C3DPolygon* pCurPolygon=NULL;
	WORD Count=0, i=0;
	if(m_PtrPolygonList.GetCount()>0)
		Pos=m_PtrPolygonList.GetHeadPosition();
	while(Pos!=NULL)
	{
		pCurPolygon=(C3DPolygon*)m_PtrPolygonList.GetNext(Pos);
		for(i=0; i<pCurPolygon->m_3DPointsArray.GetSize(); i++)
		{
			m_Percpective.O.x+=pCurPolygon->m_3DPointsArray[i].x;	
			m_Percpective.O.y+=pCurPolygon->m_3DPointsArray[i].y;	
			m_Percpective.O.z+=pCurPolygon->m_3DPointsArray[i].z;	
		}
		Count+=i;
	}
	m_Percpective.O.x/=Count;
	m_Percpective.O.y/=Count;
	m_Percpective.O.z/=Count;
};


void C3DShape::MakeProjection()
{
	POSITION Pos=NULL;
	if(m_PtrPolygonList.GetCount()>0)
		Pos=m_PtrPolygonList.GetHeadPosition();
	while(Pos!=NULL)
		((C3DPolygon*)m_PtrPolygonList.GetNext(Pos))->MakeProjection(m_Percpective);
};

BOOL C3DShape::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags, UINT nMyFlags)
{
	
	BOOL res=TRUE;
	if(nMyFlags & SHIFT_HOLD)
	switch(nChar)
	{
		case 38: m_Percpective.phi-=ROTATE_STEP; break;	//Up ����� ���������� ����
		case 40: m_Percpective.phi+=ROTATE_STEP; break;	//Down ����� ���������� ����
		case 37: m_Percpective.theta-=ROTATE_STEP; break; //Left ����� ���������� �����
		case 39: m_Percpective.theta+=ROTATE_STEP; break; //Right ����� ���������� ������
		default: res=FALSE;
	}
	else
	if(nMyFlags & CTRL_HOLD)	 
	switch(nChar)
	{
		case 38: m_Percpective.d+=MOVE_STEP; break; //Up ����� ������
		case 40: m_Percpective.d-=MOVE_STEP; break; //Down ����� �����
		default: res=FALSE;
	}
	else
	{
		//�������
		switch(nChar)
		{
			case 38:	m_Percpective.dy+= MOVE_STEP; break; // �����
			case 40: 	m_Percpective.dy-= MOVE_STEP; break; // ����
			case 37: 	m_Percpective.dx-= MOVE_STEP; break; // �����
			case 39:    m_Percpective.dx+= MOVE_STEP; break; // ������
			// ������� P ���/���� ������������� ��������������
			case 80:    m_Percpective.with_perspective=!m_Percpective.with_perspective;
						break; 
			default:	res=FALSE;
		}	
	}
	if(res)
		// ������ ��������
		MakeProjection();
	return res;
};


/////////////////////////////////////////////////////////////////////////////////
// ���������� ������� ��� ������ � �������������

int AddRsection(C3DShape *pShape, POINT3D *pSur, int x_size, int y_size, double level, COLORREF color)
{
	if(x_size<2 || y_size<2) return 0;
	// ������� ��� ���������� �������� ����� ����� ������
	C3DPolygon *pTempPolygon=new C3DPolygon();
	if(pTempPolygon==NULL) return 0; 
	// ��������� ����������� �� ������������ �
	// ������� ����� ����������� ��� ������� ������������ � ��������� level.
	// ����� ����������� ��������� � pTempPolygon
	for(int x=0, y=0; y<y_size-1; y++)
		for(int x=0;  x<x_size-1; x++)
		{
			AddTriangleSection(pTempPolygon, &pSur[y*x_size+x], &pSur[(y+1)*x_size+x+1], &pSur[y*x_size+x+1], level);
			AddTriangleSection(pTempPolygon, &pSur[y*x_size+x], &pSur[(y+1)*x_size+x], &pSur[(y+1)*x_size+x+1], level);
		}
	// �� ����������� ������ ����� ������� ���������� �����������
	// ��� ��������� ������ � ������� ������� ������ �� ������
	// ��� ������� �� ������ ����������� ��������� ���, ���� ������ :)
	CArray <POINT3D, POINT3D> &TempPointsArray=pTempPolygon->m_3DPointsArray;
	
	int pos=0, posmin=0;
	POINT3D EndSegPoint;
	double D=0, dcur=0, dmin=0; // ���������� ����� �������
	C3DPolygon *pSeg;
	BOOL fContinueSeg=TRUE; // ���� "���������� ������� �������"
	// ��������� ��������� ���������� ����� ������� - ��������� ����� �� ���������
	POINT3D P1=pSur[0], P2=pSur[x_size+1];	P1.z=P2.z=0;
	D=Dist(&P1, &P2);  
	// ���� �� ��������� ������� �������� ���� �� ���� �����
	// c������� �� ������� �������� �������
	while(TempPointsArray.GetSize()-1>0)
	{
		// ����� ������� - �������
		pSeg=new C3DPolygon();	fContinueSeg=TRUE;
		if(pSeg==NULL) return 0;
		// ��������� ����
		pSeg->SetPen(color);
		// ������ �����-������ � ����� ��������
		pSeg->AddPoint(TempPointsArray[0]);
		EndSegPoint=TempPointsArray[0];
		// ������� ����� �� ������ ������� �����
		TempPointsArray.RemoveAt(0);
		// ���������� �������
		while(fContinueSeg )
		{	
			posmin=0;
			dmin=D*2;
			// � ������ �������
			// �������� ��������� � ����� �������� ����� 
			for(pos=0; pos<TempPointsArray.GetSize(); pos++)
			{
			  dcur=Dist(&EndSegPoint, &TempPointsArray[pos]);
			  if(dcur<dmin) 
			    {dmin=dcur; posmin=pos;}//���������� �������(�����) ��������� �����
			}
			if(dmin<=D) //���������� �� ��������� ����� ������ ����������,
			{	
				//�� ���-���� ����� �� ��������� � ������ ��������,
				//������� ������� �� � �������
				if(dmin>D/1000) 
				{
					//��������� ���� � �������
					pSeg->AddPoint(TempPointsArray[posmin]);
					//����� ����� ����������� ������ ��������
					EndSegPoint=TempPointsArray[posmin];
				}

				//������� ��� �����
				TempPointsArray.RemoveAt(posmin);
			}
			else //�� ����� ������� � ����� �����- ��������� �������
				fContinueSeg=FALSE;
		} ;
		//��������, ����� ����� �������� �������
		if(pSeg->m_3DPointsArray.GetSize()>2)
			if(Dist(&pSeg->m_3DPointsArray[0], &pSeg->m_3DPointsArray[pSeg->m_3DPointsArray.GetSize()-1])<D)
				pSeg->AddPoint(pSeg->m_3DPointsArray[0]);
		
			
		//��������� ������� � ������ 
		pShape->AddPolygon(pSeg);
	} 	
	//��������� ������� ��� ������ �� �����
	delete pTempPolygon;
	return 1;
}



void AddTriangleSection(C3DPolygon *p3DPolygon, POINT3D *pP1, POINT3D *pP2, POINT3D *pP3, double level)
{
	int  f1,f2,f3;
	double x1,x2,x3,y1,y2,y3;
	POINT3D P1, P2;
	if(	!((pP1->z==level)&&(pP2->z==level)&&(pP3->z==level)) && //����������� � ���������
   		!((pP1->z>level)&&(pP2->z>level)&&(pP3->z>level)) &&   //����������� � ��������� �� ������������
		!((pP1->z<level)&&(pP2->z<level)&&(pP3->z<level)) )	   //����������� � ��������� �� ������������

   	if((pP1->z==level)&&(pP2->z==level))	//������� � ��������� - ���������
   	{
		p3DPolygon->AddPoint(*pP1);
		p3DPolygon->AddPoint(*pP2);
   	}
	else
		if((pP2->z==level)&&(pP3->z==level)) //������� � ��������� - ���������
   		{ 
   			p3DPolygon->AddPoint(*pP2);
			p3DPolygon->AddPoint(*pP3);
		}
   		else
   			if((pP3->z==level)&&(pP1->z==level)) //������� � ��������� - ���������
   		    { 
	   			p3DPolygon->AddPoint(*pP3);
				p3DPolygon->AddPoint(*pP1);
			}
   			else
   			{	// ������� 	����������� ������ ������� ������������ � ����������
				 f1=CutCross(level,pP1, pP2, x1, y1); 
				 f2=CutCross(level,pP2, pP3, x2, y2);
				 f3=CutCross(level,pP3, pP1, x3, y3);
				 if(f1&&f2) 
				 { 
					P1.x=x1; P1.y=y1; P1.z=level; 
					P2.x=x2; P2.y=y2; P2.z=level; 
					p3DPolygon->AddPoint(P1);
					p3DPolygon->AddPoint(P2);
			     }
				 if(f2&&f3)
				 { 
					P1.x=x2; P1.y=y2; P1.z=level; 
					P2.x=x3; P2.y=y3; P2.z=level; 
					p3DPolygon->AddPoint(P1);
					p3DPolygon->AddPoint(P2);
				 }
				 if(f1&&f3)
				 { 
					P1.x=x1; P1.y=y1; P1.z=level; 
					P2.x=x3; P2.y=y3; P2.z=level; 
					p3DPolygon->AddPoint(P1);
					p3DPolygon->AddPoint(P2);
				 }
   }//~else
}

int CutCross(double level, POINT3D *pP1, POINT3D *pP2, double &x, double &y)
{ if( (pP1->z<level && pP2->z<level) || //������� ��� ���������� level
      (pP1->z>level && pP2->z>level) ||	//������� ��� ���������� level
	  (pP1->z==pP2->z)  ) 	 //������� � ��������� level
		{x=pP1->x; y=pP1->y; return 0;} 
  else
    {
	 x=pP2->x-(pP1->x-pP2->x)*(level-pP2->z)/(pP2->z-pP1->z);
	 y=pP2->y-(pP1->y-pP2->y)*(level-pP2->z)/(pP2->z-pP1->z);
	 return 1;
	}
}

double Dist(POINT3D *pP1, POINT3D* pP2)
{
	if(pP1==NULL||pP2==NULL) return 0;
	return sqrt(pow(pP1->x-pP2->x, 2)+pow(pP1->y-pP2->y, 2)+pow(pP1->z-pP2->z, 2));
};
// ���� Shapes.h
////////////////////////////////////////////////////
//����� ������� �����
#define MOVE_STEP 100
#define ROTATE_STEP 5
#define SHIFT_HOLD	1
#define CTRL_HOLD	2

#include <atlimage.h>

class CBasePoint: public CPoint, public CObject
{
 	DECLARE_SERIAL(CBasePoint)
	CPen	m_Pen;	// ����
	CBrush	m_Brush;// �����
protected:
	// ����� ������������
	virtual void Serialize(CArchive& ar);
	// �������������� �������� ����������
	virtual BOOL PrepareDC(CDC *pDC);
	// ������������� �������� ����������
	virtual BOOL RestoreDC(CDC *pDC);
public:
	// ������
	WORD		m_wSize;			//������ ������
	int			m_iPenStyle;		//����� �����
	int			m_iPenWidth;		//������ �����
	COLORREF	m_rgbPenColor;		//���� �����
	int			m_iBrushStyle;		//����� �������
	COLORREF	m_rgbBrushColor;	//���� �������
	DWORD		m_dwPattern_ID;		//������������� ������� �������
public:		
	// ������������
	CBasePoint();				//����������� ��� ����������
	CBasePoint(int x, int y, WORD s);	//����������� � �����������
	~CBasePoint(){};			//����������
	// ������
	// ���������� ������ �� ������
	virtual void Show(CDC *pDC);
	// �������� ������� �������
	virtual void GetRegion(CRgn &Rgn);
	// ������������� ��������� �����
	virtual BOOL SetPen(COLORREF color, int width =1, int style=PS_SOLID);
	// ������������� ��������� �������
	virtual BOOL SetBrush(COLORREF color, DWORD pattern =0, int style=-1);
	// ��������� �������������� �� ���������
	virtual void Transform(const CPoint &point0, double ang, int a, int b);
	// ������� �� ������� �������
	virtual BOOL OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags, UINT nMyFlags); 
};

////////////////////////////////////////////////////
// Растровый графический объект
class CRaster: public CBasePoint
{
	DECLARE_SERIAL(CRaster)
protected:
	ATL::CImage *m_pImage;
	CString m_ImagePath;
	int m_ImageWidth;
	int m_ImageHeight;
	void Serialize(CArchive &ar);
public:
	CRaster();
	~CRaster();
	void Show(CDC *pDC);
	void GetRegion(CRgn &Rgn);
	BOOL CreateSampleImage(int width, int height);
	BOOL LoadFromFile(LPCTSTR fileName);
	void SetSize(int width, int height);
};

////////////////////////////////////////////////////
// ����� ��������� ������ (������� 6)
class CVariant6: public CBasePoint
{
	DECLARE_SERIAL(CVariant6)
protected:
	// ������������
	void Serialize(CArchive& ar);
public:
	CVariant6();
	~CVariant6(){};
	// ���������� ������ �� ������
	void Show(CDC *pDC);
	// �������� ������� �������
	void GetRegion(CRgn &Rgn);
};


////////////////////////////////////////////////////
//����� �������
class CSquare: public CBasePoint
{
 	DECLARE_SERIAL(CSquare)   
 protected:  
	 // ����� ������������
	void Serialize(CArchive& ar);
 public:		
	// ������������
	CSquare(int x, int y, WORD s);
	CSquare();
	~CSquare(){};
//������
	// ���������� ������ �� ������
	void Show(CDC *pDC);
	// �������� ������� �������
	void GetRegion(CRgn &Rgn);
};

////////////////////////////////////////////////////
//����� �������
class CPolygon: public CBasePoint
{
 	DECLARE_SERIAL(CPolygon)   
	BOOL	m_bPolygon;	// ����� ���������: 
	// TRUE - ����������� �������,
	// FALSE - ������� ������. ���, ������� ������. ���, ������� �����. ��! 

 protected:  
	 // ����� ������������
	void Serialize(CArchive& ar);
 public:
	 // ������������ ������ �����-������
	 CArray <CPoint, CPoint> m_PointsArray;
		
	// ������������
	 CPolygon();
	~CPolygon();
//������
	// ���������� ������ �� ������
	void Show(CDC *pDC);
	// �������� ������� �������
	void GetRegion(CRgn &Rgn);
	// ������������� ����� ��������� ��������
	void SetPolygon(BOOL p) {m_bPolygon=p;};
	// ��������� �������������� �� ���������
	void Transform(const CPoint &point0, double ang, int a, int b);
};

////////////////////////////////////////////////////
// Сплайновая фигура (Catmull-Rom -> аппроксимация)
class CSpline: public CPolygon
{
	DECLARE_SERIAL(CSpline)
protected:
	void Serialize(CArchive& ar);
public:
	CSpline(){};
	~CSpline(){};
	void Show(CDC *pDC);
	void GetRegion(CRgn &Rgn);
};

////////////////////////////////////////////////////
// Полигон с растровой шаблонной кистью
class CPatternPolygon: public CPolygon
{
	DECLARE_SERIAL(CPatternPolygon)
	CBitmap m_PatternBitmap;
	CBrush m_PatternBrush;
	BOOL m_bPatternCreated;
protected:
	void Serialize(CArchive &ar);
public:
	CPatternPolygon();
	~CPatternPolygon(){};
	BOOL CreatePatternBrush(CDC *pDC);
	void Show(CDC *pDC);
	void GetRegion(CRgn &Rgn);
};

////////////////////////////////////////////////////
// 3D �����
struct POINT3D
{
	double x, y, z;
}; 

//���������  ���������� �����
struct  Perspective
{
	POINT3D	O;					// �����, ������ ������� ��������� �������
	double	rho, theta, phi,	// �������� ���������� ����� ���������� (E)
			d;					// ���������� �� E �� ������
	WORD	with_perspective;   // 1- ���. ������������� ��������������; 0-����.
	LONG	dx, dy;				// �������� �������� �� ������
};

////////////////////////////////////////////////////
//����� C3DPolygon
//
class C3DPolygon: public CPolygon
{
 	DECLARE_SERIAL(C3DPolygon)   
 protected:  	// ����� ������������	
	void Serialize(CArchive& ar);
 public:		
 	// ������������
	 C3DPolygon(){};
	 ~C3DPolygon(){};
// ������
	// ������������ ������ �����-������ � ������� �����������
	CArray <POINT3D, POINT3D> m_3DPointsArray;	
// ������		
	// �������� �����
	void AddPoint(POINT3D point) {m_3DPointsArray.Add(point);};
	// ������ �������� ���������
	void MakeProjection(Perspective P); 
};


////////////////////////////////////////////////////
//����� C3DShape
class C3DShape: public CBasePoint
{
 	DECLARE_SERIAL(C3DShape)   
 protected:  	// ����������� ����� ������������	
	void Serialize(CArchive& ar);
	
 public:		
	//������������
	C3DShape();
	~C3DShape();
//������
	Perspective m_Percpective; //��������� ������
	CTypedPtrList<CObList, C3DPolygon*> m_PtrPolygonList; // ������ ���������� �� ��������
//������
	// ������ ��������
	void MakeProjection();
	// ����������� ������ �� ������
	void Show(CDC *pDC);
	// �������� ������� �������
	void GetRegion(CRgn &Rgn);
	// �������� �������
	void AddPolygon(C3DPolygon *pPolygon);
	// ������� �� ������� �������
	BOOL OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags, UINT nMyFlags);
};

/////////////////////////////////////////////////////////////////////////////////
// ���������� ������� ��� ������ � �������������,
// �� �������, ��-��������, ���� �� ������� ����� C3DSurface

// ��������� ����� ����������� (����� ������) �����������,
// �������� �������� ����� POINT3D � ����������,
// �������� ��������� level (������� �� Z)
// ���������:
// pShape - ��������� �� ���������� ������, ���� ����� ��������� �������
// pSur - ��������� �� ������ �����, �������� ����������
// x_size, y_size - ������ ������� �����
// level - ������� ������� ���������
// color - ���� ����� �����������  
// ������������ ��������:
// 0 - �������
// 1 - �����
int AddRsection(C3DShape *pShape, POINT3D *pSur, int x_size, int y_size, double level,  COLORREF color);

// ��������� ����� ����������� ����������,
// ��������� ����� ������� � ����������,
// �������� ��������� level (������� �� Z)
// ���������:
// p3DPolygon - ��������� �� �������, ���� ����� ��������� ����� �����������
// pP1, pP2, pP3 - ��������� �� ������, �������� �����������
// level - ������� ������� ���������
void AddTriangleSection(C3DPolygon *p3DPolygon, POINT3D *pP1, POINT3D *pP2, POINT3D *pP3, double level);

// � ��� ������� ������ ����� �������� �����������

// ������������ ���������� x,y ����� ����������� ������� pP1, pP2 � ������������ level
int CutCross(double level, POINT3D *pP1, POINT3D *pP2, double &x, double &y);
// ������������ ���������� ����� ����� �������
double Dist(POINT3D *pP1, POINT3D* pP2);
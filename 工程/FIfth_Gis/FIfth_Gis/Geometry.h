#pragma once
#include "framework.h"
#include "FIfth_Gis.h"
#include <json/json.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>
#include <errno.h>
#include <string.h>
#include <vector>
#include <cmath>
using namespace std;

// ����Ҫ���࣬��ͬ����ID
class Geometry
{
protected:
	string ID;
public:

	Geometry(string id)
	{
		ID = id;
	}
	string GetID()
	{
		return ID;
	}
};

// ��Ҫ����(Point)
class Point : public Geometry
{
protected:
	float x;
	float y;
public:
	// ���캯������ʼ��
	Point(float px = (rand() % 800), float py = (rand() % 500), string id = std::to_string(rand() % 100)) :Geometry(id)
	{
		x = px;
		y = py;
	}
	// ��ȡX����
	float GetX()
	{
		return x;
	}
	// ��ȡY����
	float GetY()
	{
		return y;
	}
	// ��GeoJson����
	void InitialFromJSON(Json::Value root)
	{
		Json::Value properties, geometry, coordinates, xy;
		// ������ȡJson��Ҫ������
		properties = root["properties"];
		// ������ȡҪ��ID
		string pID = properties["id"].asString();
		// ������ȡҪ�صĵ�������Ϣ
		geometry = root["geometry"];
		coordinates = geometry["coordinates"];
		float px = 0;
		float py = 0;
		xy = coordinates[0];
		x = xy[0].asFloat();
		y = xy[1].asFloat();
		ID = pID;
	}
	// ���ƺ���
	void Draw(HDC hdc)
	{
		Ellipse(hdc, x * 10 - 1, y * 10 - 1, x * 10 + 1, y * 10 + 1);
	}

};

// ��Ҫ���ࣨLineString��
class PolyLine : public Geometry
{
public:
	std::vector<Point> pts;

	// ���캯������ʼ��
	PolyLine(string id = std::to_string(rand() % 100)) :Geometry(id)
	{
		ID = id;
	}

	// ��GeoJson���ɣ���Ҫ�ظ�����GeoJson��ʽ�ص���в�ͬ��Json������Ҫ�����ɣ���������Coordinates����֯�ṹ��ͬ���������΢���죬��ͬ
	void InitialFromGeoJson(Json::Value root)
	{
		Json::Value properties, geometry, coordinates, xy;
		properties = root["properties"];
		string pID = properties["id"].asString();
		geometry = root["geometry"];
		coordinates = geometry["coordinates"];
		float px = 0;
		float py = 0;
		ID = pID;
		for (int i = 0; i < coordinates.size(); ++i) {
			xy = coordinates[i];
			px = xy[0].asFloat();
			py = xy[1].asFloat();
			Point pt(px, py, ID);
			pts.push_back(pt);
		}
	}
	// GeoJson��ʼ��/���/������Ҫ�ص������
	void AddCoorFromGeoJson(Json::Value coordinates)
	{
		Json::Value p_xy, xy;
		float px = 0;
		float py = 0;
		for (int i = 0; i < coordinates.size(); ++i) {
			xy = coordinates[i];
			px = xy[0].asFloat();
			py = xy[1].asFloat();
			Point pt(px, py, ID);
			pts.push_back(pt);
		}
	}
	// ���ƺ���
	void Draw(HDC hdc)
	{
		POINT* point = new POINT[pts.size()];
		for (int i = 0; i < pts.size(); ++i)
		{
			point[i].x = (pts[i].GetX() * 10);
			point[i].y = (pts[i].GetY() * 10);
		}

		Polyline(hdc, point, pts.size());

		delete[] point;
	}
	static PolyLine CreateRandomPolyLine()
	{
		PolyLine polyline(std::to_string(rand() % 100));
		int ptn = rand() % 500 + 2;
		for (int i = 0; i < ptn; ++i)
		{
			polyline.pts.push_back(Point::Point());
		}
		return polyline;
	}
};

//  ��Ҫ���ࣨPolygon��
class PolyGon : public Geometry
{
public:
	std::vector<Point> pts;

	// ���캯������ʼ��
	PolyGon(string id = std::to_string(rand() % 100)) :Geometry(id)
	{
		ID = id;
	}

	// ��GeoJson����
	void InitialFromGeoJson(Json::Value root)
	{
		Json::Value properties, geometry, coordinates, p_xy, xy;
		properties = root["properties"];
		string pID = properties["id"].asString();
		geometry = root["geometry"];
		coordinates = geometry["coordinates"];
		float px = 0;
		float py = 0;
		ID = pID;
		//
		// Json���������ݸ�ʽΪ��
		// [
		//		[
		//			[122.12345, 23.234212],
		//			[122.83571, 24.342112],
		//			[113.42612, 22.738014],
		//			[113.447808, 22.735836],
		//			[113.467964, 22.728504],
		//      ]
		// ]
		//
		for (int i = 0; i < coordinates.size(); ++i) {
			p_xy = coordinates[i];
			for (int j = 0; j < p_xy.size(); ++j)
			{
				xy = p_xy[j];
				px = xy[0].asFloat();
				py = xy[1].asFloat();
				Point pt(px, py, ID);
				pts.push_back(pt);
			}
		}
	}

	// GeoJson��ʼ��/���/������Ҫ�ص������
	void AddCoorFromGeoJson(Json::Value coordinates)
	{
		Json::Value p_xy, xy;
		float px = 0;
		float py = 0;
		for (int i = 0; i < coordinates.size(); ++i) {
			p_xy = coordinates[i];
			for (int j = 0; j < p_xy.size(); ++j)
			{
				xy = p_xy[j];
				px = xy[0].asFloat();
				py = xy[1].asFloat();
				Point pt(px, py, ID);
				pts.push_back(pt);
			}
		}
	}

	// ���ƺ���
	void Draw(HDC hdc)
	{
		POINT* point = new POINT[pts.size() + 1];
		for (int i = 0; i < pts.size(); ++i)
		{
			/*point[i].x = (fabs(pts[i].GetX() - int(pts[i].GetX() / 10 * 10) + int(pts[i].GetX()) % 100) * 20);
			point[i].y = (fabs(pts[i].GetY() - int(pts[i].GetY() / 10 * 10) + int(pts[i].GetY()) % 100) * 20);*/
			point[i].x = (pts[i].GetX() * 10);
			point[i].y = ((90.0 - pts[i].GetY()) * 10);
		}
		/*point[pts.size()].x = (fabs(pts[0].GetX() - int(pts[0].GetX() / 10 * 10) + int(pts[0].GetX()) % 100) * 20);
		point[pts.size()].y = (fabs(pts[0].GetY() - int(pts[0].GetY() / 10 * 10) + int(pts[0].GetY()) % 100) * 20);*/
		point[pts.size()].x = (pts[0].GetX() * 10);
		point[pts.size()].y = ((90.0 - pts[0].GetY()) * 10);
		Polygon(hdc, point, pts.size());

		delete[] point;
	}

	static PolyGon CreateRandomPolyGon()
	{
		PolyGon polygon(std::to_string(rand() % 100));
		int ptn = rand() % 10 + 3;
		for (int i = 0; i < ptn; ++i)
		{
			polygon.pts.push_back(Point::Point());
		}
		return polygon;
	}
};

// ��Ҫ��ͼ���ࣨMultiPoint��
class PointLayer
{
public:
	string layer_id;
	// ���ͼ��ĵ�Ҫ��
	std::vector<Point> element;

	// ���캯������ʼ��
	PointLayer(string layer_id = "PointLayer_1")
	{

		int elementCount = rand() % 5 + 1;
		for (int i = 0; i < elementCount; ++i)
		{
			element.push_back(Point::Point(0, 0, "id"));
		}
	}

	// ��GeoJson����
	void InitialFromGeoJson(Json::Value root)
	{
		Json::Value properties, geometry, coordinates, xy;
		properties = root["properties"];
		string pID = properties["id"].asString();
		geometry = root["geometry"];
		coordinates = geometry["coordinates"];
		float px = 0;
		float py = 0;
		layer_id = pID;
		for (int i = 0; i < coordinates.size(); ++i) {
			xy = coordinates[i];
			px = xy[0].asFloat();
			py = xy[1].asFloat();
			Point pt(px, py, std::to_string(i));
			element.push_back(pt);
		}
	}
	// ���ƺ���
	void Draw(HDC hdc)
	{
		for (int i = 0; i < element.size(); ++i)
		{
			element[i].Draw(hdc);
		}
	}
};

// ��Ҫ��ͼ���ࣨMultiLineString��
class PolyLineLayer
{
public:
	std::string layer_id;
	// ���ͼ�����Ҫ��
	std::vector<PolyLine> element;

	int LineWidth;
	COLORREF color;
	int style;

	// ���캯������ʼ��
	PolyLineLayer(string layer_id = "PolylineLayer_1")
	{
		style = PS_SOLID;
		LineWidth = 1.0;
		color = RGB(0, 255, 0);
	}

	// ��GeoJson����
	void InitialFromGeoJson(Json::Value root)
	{
		style = PS_SOLID;
		LineWidth = 1.0;
		color = RGB(rand() % 256, rand() % 256, rand() % 256);

		Json::Value properties, geometry, coordinates, p_xy, xy;
		properties = root["properties"];
		string pID = properties["id"].asString();
		geometry = root["geometry"];
		coordinates = geometry["coordinates"];
		float px = 0;
		float py = 0;
		layer_id = pID;
		//
		// Json�е�������֯��ʽ��
		//[
		//	[
		//		[105.6005859375, 30.65681556429287],
		//		[107.95166015624999, 31.98944183792288],
		//		[109.3798828125, 30.031055426540206],
		//		[107.7978515625, 29.935895213372444]
		//	] ,
		//	[
		//		[109.3798828125, 30.031055426540206],
		//		[107.1978515625, 31.235895213372444]
		//	]
		//]
		//
		for (int i = 0; i < coordinates.size(); ++i) {
			p_xy = coordinates[i];
			string line_name = "Line_";
			string line_no = std::to_string(i);
			string line_id = line_name + line_no;
			PolyLine polyline(line_id);
			polyline.AddCoorFromGeoJson(p_xy);
			element.push_back(polyline);
		}
	}
	// ���ƺ���
	void Draw(HDC hdc)
	{
		HPEN hPen1 = CreatePen(style, LineWidth, color);

		HPEN hOld = (HPEN)SelectObject(hdc, hPen1);

		for (int i = 0; i < element.size(); ++i)
		{
			element[i].Draw(hdc);
		}

		SelectObject(hdc, hOld);
	}

};

// ��Ҫ��ͼ���ࣨMultiPolygon��
class PolyGonLayer
{
public:
	std::string layer_id;
	// ���ͼ�����Ҫ��
	std::vector<PolyGon> element;
	int style;
	int LineWidth;
	COLORREF LineColor;
	COLORREF FillColor;

	// ���캯������ʼ��
	PolyGonLayer(string layer_id = "PolygonLayer")
	{
		style = PS_SOLID;
		LineWidth = 1.0;
		LineColor = RGB(255, 0, 0);
		FillColor = RGB(0, 0, 255);
	}
	// ��GeoJson����
	void InitialFromGeoJson(Json::Value root)
	{
		style = PS_SOLID;
		LineWidth = 1.0;
		LineColor = RGB(255, 0, 0);
		FillColor = RGB(0, 0, 255);

		Json::Value properties, geometry, coordinates, p_xy, xy;
		properties = root["properties"];
		string pID = properties["id"].asString();
		geometry = root["geometry"];
		coordinates = geometry["coordinates"];
		float px = 0;
		float py = 0;
		layer_id = pID;
		//
		// Json�е�������֯��ʽ��
		/*[
			[
				[
					[109.2041015625, 30.088107753367257],
					[115.02685546875, 30.088107753367257],
					[115.02685546875, 32.7872745269555],
					[109.2041015625, 32.7872745269555],
					[109.2041015625, 30.088107753367257]


				]
			] ,
			[
				[
					[112.9833984375, 26.82407078047018],
					[116.69677734375, 26.82407078047018],
					[116.69677734375, 29.036960648558267],
					[112.9833984375, 29.036960648558267],
					[112.9833984375, 26.82407078047018]
				]
			]
		  ]*/
		  //
		for (int i = 0; i < coordinates.size(); ++i) {
			p_xy = coordinates[i];
			string poly_name = "Polygon_";
			string poly_no = std::to_string(i);
			string poly_id = poly_name + poly_no;
			PolyGon polyGon(poly_id);
			polyGon.AddCoorFromGeoJson(p_xy);
			element.push_back(polyGon);
		}
	}
	// ���ƺ���
	void Draw(HDC hdc)
	{
		HPEN hPen1 = CreatePen(style, LineWidth, LineColor);
		HBRUSH hBrush = CreateSolidBrush(FillColor);
		HPEN hOldPen = (HPEN)SelectObject(hdc, hPen1);
		HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
		for (int i = 0; i < element.size(); ++i)
		{
			element[i].Draw(hdc);
		}

		SelectObject(hdc, hOldPen);
		SelectObject(hdc, hOldBrush);
	}
};

// �ࣨ���ӣ�Ҫ�ؼ�ͼ��/��ͼ��(FeatureCollection��������ΪFeature)
class FeatureCollection
{
public:
	std::string layer_id;
	// ���ͼ��/��ͼ�ĵ�Ҫ�ؼ�/ͼ��
	std::vector<PolyGonLayer> Poly_element;
	// ���ͼ��/��ͼ����Ҫ�ؼ�/ͼ��
	std::vector<PolyLineLayer> Line_element;
	// ���ͼ��/��ͼ����Ҫ�ؼ�/ͼ��
	std::vector<PointLayer> Point_element;
	int style;
	int LineWidth;
	COLORREF LineColor;
	COLORREF FillColor;

	// ���캯������ʼ��
	FeatureCollection()
	{
		layer_id = "Map";
		style = PS_SOLID;
		LineWidth = 1.0;
		LineColor = RGB(255, 0, 0);
		FillColor = RGB(0, 0, 255);
	}

	// ��GeoJson����
	void InitialFromGeoJson(Json::Value root)
	{
		style = PS_SOLID;
		LineWidth = 1.0;
		LineColor = RGB(255, 0, 0);
		FillColor = RGB(0, 0, 255);

		Json::Value features, feature, properties, geometry, coordinates, p_xy, xy;
		features = root["features"];
		for (int i = 0; i < features.size(); ++i) {
			feature = features[i];
			properties = feature["properties"];
			geometry = feature["geometry"];
			string pID = properties["name"].asString();
			string l_type = geometry["type"].asString();
			// �ж�Ҫ�����ͣ��㡢�ߡ��棬�����ɶ�ӦҪ�أ�������
			if (l_type == "MultiPolygon")
			{
				PolyGonLayer pgl(pID);
				pgl.InitialFromGeoJson(feature);
				Poly_element.push_back(pgl);
			}
			else if (l_type == "MultiLineString")
			{
				PolyLineLayer pll(pID);
				pll.InitialFromGeoJson(feature);
				Line_element.push_back(pll);
			}
			else if (l_type == "MultiPoint")
			{
				PointLayer pl(pID);
				pl.InitialFromGeoJson(feature);
				Point_element.push_back(pl);
			}
		}
	}

	// ���ƺ���
	void Draw(HDC hdc)
	{
		HPEN hPen1 = CreatePen(style, LineWidth, LineColor);
		HBRUSH hBrush = CreateSolidBrush(FillColor);
		HPEN hOldPen = (HPEN)SelectObject(hdc, hPen1);
		HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
		for (int i = 0; i < Poly_element.size(); ++i)
		{
			Poly_element[i].Draw(hdc);
		}
		for (int i = 0; i < Line_element.size(); ++i)
		{
			Line_element[i].Draw(hdc);
		}
		for (int i = 0; i < Point_element.size(); ++i)
		{
			Point_element[i].Draw(hdc);
		}

		SelectObject(hdc, hOldPen);
		SelectObject(hdc, hOldBrush);
	}
};
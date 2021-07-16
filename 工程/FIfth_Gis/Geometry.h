#pragma once
#include "framework.h"
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

// 地理要素类，共同特征ID
class Geometry
{
protected:
	string ID;
	float x_max = 0.0;
	float x_min = 180.0;
	float y_max = 0.0;
	float y_min = 90.0;
public:

	Geometry(string id = "go_1")
	{
		ID = id;
	}
	string GetID()
	{
		return ID;
	}
	virtual void InitialFromGeoJson(Json::Value root)
	{
	}
	virtual void Draw(HDC hdc)
	{
	}
	float GetMaxX()
	{
		return x_max;
	}
	float GetMinX()
	{
		return x_min;
	}
	float GetMaxY()
	{
		return y_max;
	}
	float GetMinY()
	{
		return y_min;
	}

};

// 点要素类(Point)
class Point : public Geometry
{
protected:
	float x;
	float y;
public:
	// 构造函数，初始化
	Point(float px = (rand() % 800), float py = (rand() % 500), string id = std::to_string(rand() % 100)) :Geometry(id)
	{
		x = px;
		y = py;
	}
	// 获取X坐标
	float GetX()
	{
		return x;
	}
	// 获取Y坐标
	float GetY()
	{
		return y;
	}
	// 由GeoJson生成
	void InitialFromGeoJson(Json::Value root)
	{
		Json::Value properties, geometry, coordinates, xy;
		// 解析获取Json中要素属性
		properties = root["properties"];
		// 解析获取要素ID
		string pID = properties["id"].asString();
		// 解析获取要素的地理几何信息
		geometry = root["geometry"];
		coordinates = geometry["coordinates"];
		float px = 0;
		float py = 0;
		xy = coordinates[0];
		x = xy[0].asFloat();
		y = xy[1].asFloat();
		ID = pID;
	}
	// 绘制函数
	void Draw(HDC hdc)
	{
		Ellipse(hdc, x * 10 - 1, y * 10 - 1, x * 10 + 1, y * 10 + 1);
	}
	void MultiDraw(HDC hdc, double xrate, double yrate, double xmax, double xmin, double ymax, double ymin)
	{
		Ellipse(hdc, ((this->GetX() - xmin) / (xmax - xmin) * min(xrate, yrate) * 0.8 * 2 + 100 - 1), ((this->GetY() - ymin) / (ymax - ymin) * min(xrate, yrate) * 0.8 + 100 - 1), 
			((this->GetX() - xmin) / (xmax - xmin) * min(xrate, yrate) * 0.8 * 2 + 100 + 1), ((this->GetY() - ymin) / (ymax - ymin) * min(xrate, yrate) * 0.8 + 100 + 1));
	}

};

// 线要素类（LineString）
class PolyLine : public Geometry
{
public:
	std::vector<Point> pts;

	// 构造函数，初始化
	PolyLine(string id = std::to_string(rand() % 100)) :Geometry(id)
	{
		ID = id;
	}

	// 由GeoJson生成，各要素根据其GeoJson格式特点进行不同的Json解析和要素生成，这里坐标Coordinates的组织结构不同，因此有稍微差异，下同
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
			if (px > x_max)
				x_max = px;
			if (px < x_min)
				x_min = px;
			if (py > y_max)
				y_max = py;
			if (py < y_min)
				y_min = py;
			Point pt(px, py, ID);
			pts.push_back(pt);
		}
	}
	// GeoJson初始化/添加/更改线要素的坐标点
	void AddCoorFromGeoJson(Json::Value coordinates)
	{
		Json::Value p_xy, xy;
		float px = 0;
		float py = 0;
		for (int i = 0; i < coordinates.size(); ++i) {
			xy = coordinates[i];
			px = xy[0].asFloat();
			py = xy[1].asFloat();
			if (px > x_max)
				x_max = px;
			if (px < x_min)
				x_min = px;
			if (py > y_max)
				y_max = py;
			if (py < y_min)
				y_min = py;
			Point pt(px, py, ID);
			pts.push_back(pt);
		}
	}
	// 绘制函数
	void Draw(HDC hdc, double xrate, double yrate)
	{
		POINT* point = new POINT[pts.size()];
		for (int i = 0; i < pts.size(); ++i)
		{
			point[i].x = (pts[i].GetX() - this->GetMinX()) / (this->GetMaxX() - this->GetMinX()) * min(xrate, yrate) * 0.8 * 2 + 100;
			point[i].y = yrate - (pts[i].GetY() - this->GetMinY()) / (this->GetMaxY() - this->GetMinY()) * min(xrate, yrate) * 0.8 - 100;
		}

		Polyline(hdc, point, pts.size());

		delete[] point;
	}
	void MultiDraw(HDC hdc, double xrate, double yrate, double xmax, double xmin, double ymax, double ymin)
	{
		POINT* point = new POINT[pts.size() + 1];
		for (int i = 0; i < pts.size(); ++i)
		{


			point[i].x = (pts[i].GetX() - xmin) / (xmax - xmin) * min(xrate, yrate) * 0.8 * 2 + 100;
			point[i].y = yrate - (pts[i].GetY() - ymin) / (ymax - ymin) * min(xrate, yrate) * 0.8 - 100;

		}

		Polyline(hdc, point, pts.size());

		delete[] point;
	}
};

//  面要素类（Polygon）
class PolyGon : public Geometry
{
public:
	std::vector<Point> pts;

	// 构造函数，初始化
	PolyGon(string id = std::to_string(rand() % 100)) :Geometry(id)
	{
		ID = id;
	}

	// 由GeoJson生成
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
		// Json里坐标数据格式为：
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
				if (px > x_max)
					x_max = px;
				if (px < x_min)
					x_min = px;
				if (py > y_max)
					y_max = py;
				if (py < y_min)
					y_min = py;
				Point pt(px, py, ID);
				pts.push_back(pt);
			}
		}
	}

	// GeoJson初始化/添加/更改线要素的坐标点
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
				if (px > x_max)
					x_max = px;
				if (px < x_min)
					x_min = px;
				if (py > y_max)
					y_max = py;
				if (py < y_min)
					y_min = py;
				Point pt(px, py, ID);
				pts.push_back(pt);
			}
		}
	}

	// 绘制函数
	void Draw(HDC hdc, double xrate, double yrate)
	{
		POINT* point = new POINT[pts.size() + 1];
		for (int i = 0; i < pts.size(); ++i)
		{
			point[i].x = (pts[i].GetX() - this->GetMinX()) / (this->GetMaxX() - this->GetMinX()) * min(xrate, yrate) * 0.8 * 2 + 100;
			point[i].y = yrate - (pts[i].GetY() - this->GetMinY()) / (this->GetMaxY() - this->GetMinY()) * min(xrate, yrate) * 0.8 + 100;

		}

		point[pts.size()].x = (pts[0].GetX() - this->GetMinX()) / (this->GetMaxX() - this->GetMinX()) * min(xrate, yrate) * 0.8 * 2 + 100;
		point[pts.size()].y = yrate - (pts[0].GetY() - this->GetMinY()) / (this->GetMaxY() - this->GetMinY()) * min(xrate, yrate) * 0.8 + 100;

		Polygon(hdc, point, pts.size());

		delete[] point;
	}

	void MultiDraw(HDC hdc, double xrate, double yrate, double xmax, double xmin, double ymax, double ymin)
	{
		POINT* point = new POINT[pts.size() + 1];
		for (int i = 0; i < pts.size(); ++i)
		{


			point[i].x = (pts[i].GetX() - xmin) / (xmax - xmin) * min(xrate, yrate) * 0.8 * 2 + 100;
			point[i].y = yrate - (pts[i].GetY() - ymin) / (ymax - ymin) * min(xrate, yrate) * 0.8 - 100;

		}

		point[pts.size()].x = (pts[0].GetX() - xmin) / (xmax - xmin) * min(xrate, yrate) * 0.8 * 2 + 100;
		point[pts.size()].y = yrate - (pts[0].GetY() - ymin) / (ymax - ymin) * min(xrate, yrate) * 0.8 - 100;

		Polygon(hdc, point, pts.size());

		delete[] point;
	}
};

class Layer
{
protected:
	string layer_id;
	float x_max = 0.0;
	float x_min = 180.0;
	float y_max = 0.0;
	float y_min = 90.0;
public:
	Layer(string l_id = "Layer_1")
	{
		layer_id = l_id;
	}
	string GetID()
	{
		return layer_id;
	}
	virtual void InitialFromGeoJson(Json::Value root)
	{
	}
	virtual void Draw(HDC hdc, double xrate, double yrate)
	{
	}
	float GetMaxX()
	{
		return x_max;
	}
	float GetMinX()
	{
		return x_min;
	}
	float GetMaxY()
	{
		return y_max;
	}
	float GetMinY()
	{
		return y_min;
	}
};

// 点要素图层类（MultiPoint）
class PointLayer : public Layer
{
public:
	// 组成图层的点要素
	std::vector<Point> element;

	// 构造函数，初始化
	PointLayer(string l_id = "PointLayer_1") :Layer(l_id)
	{

		int elementCount = rand() % 5 + 1;
		for (int i = 0; i < elementCount; ++i)
		{
			element.push_back(Point::Point(0, 0, "id"));
		}
	}

	// 由GeoJson生成
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
			if (px > x_max)
				x_max = px;
			if (px < x_min)
				x_min = px;
			if (py > y_max)
				y_max = py;
			if (py < y_min)
				y_min = py;
			Point pt(px, py, std::to_string(i));
			element.push_back(pt);
		}
	}
	// 绘制函数
	void Draw(HDC hdc, double xrate, double yrate)
	{
		for (int i = 0; i < element.size(); ++i)
		{
			element[i].MultiDraw(hdc, xrate, yrate, this->GetMaxX(), this->GetMinX(), this->GetMaxY(), this->GetMinY());
		}
	}
	void MultiDraw(HDC hdc, double xrate, double yrate, double xmax, double xmin, double ymax, double ymin)
	{
		for (int i = 0; i < element.size(); ++i)
		{
			element[i].MultiDraw(hdc, xrate, yrate, xmax, xmin, ymax, ymin);
		}
	}
};

// 线要素图层类（MultiLineString）
class PolyLineLayer : public Layer
{
public:
	// 组成图层的线要素
	std::vector<PolyLine> element;

	int LineWidth;
	COLORREF color;
	int style;


	// 构造函数，初始化
	PolyLineLayer(string l_id = "PolylineLayer_1") :Layer(l_id)
	{
		style = PS_SOLID;
		LineWidth = 1.0;
		color = RGB(0, 255, 0);
	}

	// 由GeoJson生成
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
		// Json中的数据组织格式：
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
			if (polyline.GetMaxX() > x_max)
				x_max = polyline.GetMaxX();
			if (polyline.GetMinX() < x_min)
				x_min = polyline.GetMinX();
			if (polyline.GetMaxY() > y_max)
				y_max = polyline.GetMaxY();
			if (polyline.GetMinY() < y_min)
				y_min = polyline.GetMinY();
			element.push_back(polyline);
		}
	}
	// 绘制函数
	void Draw(HDC hdc, double xrate, double yrate)
	{
		HPEN hPen1 = CreatePen(style, LineWidth, color);

		HPEN hOld = (HPEN)SelectObject(hdc, hPen1);

		for (int i = 0; i < element.size(); ++i)
		{
			element[i].MultiDraw(hdc, xrate, yrate, this->GetMaxX(), this->GetMinX(), this->GetMaxY(), this->GetMinY());
		}

		SelectObject(hdc, hOld);
	}
	void MultiDraw(HDC hdc, double xrate, double yrate, double xmax, double xmin, double ymax, double ymin)
	{
		HPEN hPen1 = CreatePen(style, LineWidth, color);

		HPEN hOld = (HPEN)SelectObject(hdc, hPen1);

		for (int i = 0; i < element.size(); ++i)
		{
			element[i].MultiDraw(hdc, xrate, yrate, xmax, xmin, ymax, ymin);
		}
		SelectObject(hdc, hOld);
	}

};

// 面要素图层类（MultiPolygon）
class PolyGonLayer : public Layer
{
public:
	std::string layer_id;
	// 组成图层的面要素
	std::vector<PolyGon> element;
	int style;
	int LineWidth;
	COLORREF LineColor;
	COLORREF FillColor;


	// 构造函数，初始化
	PolyGonLayer(string l_id = "PolygonLayer", COLORREF L_Color = RGB(rand() % 256, rand() % 256, rand() % 256), COLORREF F_Color = RGB(rand() % 256, rand() % 256, rand() % 256)) :Layer(l_id)
	{
		style = PS_SOLID;
		LineWidth = 1.0;
		LineColor = L_Color;
		FillColor = F_Color;
	}
	// 由GeoJson生成
	void InitialFromGeoJson(Json::Value root)
	{

		Json::Value properties, geometry, coordinates, p_xy, xy;
		properties = root["properties"];
		string pID = properties["id"].asString();
		geometry = root["geometry"];
		coordinates = geometry["coordinates"];
		float px = 0;
		float py = 0;
		layer_id = pID;
		//
		// Json中的数据组织格式：
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
			if (polyGon.GetMaxX() > x_max)
				x_max = polyGon.GetMaxX();
			if (polyGon.GetMinX() < x_min)
				x_min = polyGon.GetMinX();
			if (polyGon.GetMaxY() > y_max)
				y_max = polyGon.GetMaxY();
			if (polyGon.GetMinY() < y_min)
				y_min = polyGon.GetMinY();
			element.push_back(polyGon);
		}
	}
	// 绘制函数
	void Draw(HDC hdc, double xrate, double yrate)
	{
		HPEN hPen1 = CreatePen(style, LineWidth, LineColor);
		HBRUSH hBrush = CreateSolidBrush(FillColor);
		HPEN hOldPen = (HPEN)SelectObject(hdc, hPen1);
		HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
		for (int i = 0; i < element.size(); ++i)
		{
			element[i].MultiDraw(hdc, xrate, yrate, this->GetMaxX(), this->GetMinX(), this->GetMaxY(), this->GetMinY());
		}

		SelectObject(hdc, hOldPen);
		SelectObject(hdc, hOldBrush);
	}
	void MultiDraw(HDC hdc, double xrate, double yrate, double xmax, double xmin, double ymax, double ymin)
	{
		HPEN hPen1 = CreatePen(style, LineWidth, LineColor);
		HBRUSH hBrush = CreateSolidBrush(FillColor);
		HPEN hOldPen = (HPEN)SelectObject(hdc, hPen1);
		HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
		for (int i = 0; i < element.size(); ++i)
		{
			element[i].MultiDraw(hdc, xrate, yrate, xmax, xmin, ymax, ymin);
		}

		SelectObject(hdc, hOldPen);
		SelectObject(hdc, hOldBrush);
	}
};

class Map
{
protected:
	string map_id;
	float x_max = 0.0;
	float x_min = 180.0;
	float y_max = 0.0;
	float y_min = 90.0;
public:
	Map(string m_id = "Map_1")
	{
		map_id = m_id;
	}
	string GetID()
	{
		return map_id;
	}
	virtual void InitialFromGeoJson(Json::Value root)
	{
	}
	virtual void Draw(HDC hdc, double xrate, double yrate)
	{
	}

	float GetMaxX()
	{
		return x_max;
	}
	float GetMinX()
	{
		return x_min;
	}
	float GetMaxY()
	{
		return y_max;
	}
	float GetMinY()
	{
		return y_min;
	}

};

// 多（复杂）要素集图层/地图类(FeatureCollection，以上类为Feature)
class FeatureCollection : public Map
{
public:
	// 组成图层/地图的点要素集/图层
	std::vector<PolyGonLayer> Poly_element;
	// 组成图层/地图的线要素集/图层
	std::vector<PolyLineLayer> Line_element;
	// 组成图层/地图的面要素集/图层
	std::vector<PointLayer> Point_element;
	int style;
	int LineWidth;
	COLORREF LineColor;
	COLORREF FillColor;

	// 构造函数，初始化
	FeatureCollection(string l_id = "Map_1") :Map(l_id)
	{
		style = PS_SOLID;
		LineWidth = 1.0;
		LineColor = RGB(rand() % 256, rand() % 256, rand() % 256);
		FillColor = RGB(rand() % 256, rand() % 256, rand() % 256);
	}

	// 由GeoJson生成
	void InitialFromGeoJson(Json::Value root)
	{

		Json::Value features, feature, properties, geometry, coordinates, p_xy, xy;
		features = root["features"];
		for (int i = 0; i < features.size(); ++i) {
			feature = features[i];
			properties = feature["properties"];
			geometry = feature["geometry"];
			string pID = properties["name"].asString();
			string l_type = geometry["type"].asString();
			// 判断要素类型：点、线、面，以生成对应要素（集）类
			if (l_type == "MultiPolygon")
			{
				PolyGonLayer pgl(pID, LineColor, FillColor);
				pgl.InitialFromGeoJson(feature);
				Poly_element.push_back(pgl);
				if (x_max < pgl.GetMaxX())
					x_max = pgl.GetMaxX();
				if (x_min > pgl.GetMinX())
					x_min = pgl.GetMinX();
				if (y_max < pgl.GetMaxY())
					y_max = pgl.GetMaxY();
				if (y_min > pgl.GetMinY())
					y_min = pgl.GetMinY();
			}
			else if (l_type == "MultiLineString")
			{
				PolyLineLayer pll(pID);
				pll.InitialFromGeoJson(feature);
				if (x_max < pll.GetMaxX())
					x_max = pll.GetMaxX();
				if (x_min > pll.GetMinX())
					x_min = pll.GetMinX();
				if (y_max < pll.GetMaxY())
					y_max = pll.GetMaxY();
				if (y_min > pll.GetMinY())
					y_min = pll.GetMinY();
				Line_element.push_back(pll);
			}
			else if (l_type == "MultiPoint")
			{
				PointLayer pl(pID);
				pl.InitialFromGeoJson(feature);
				if (x_max < pl.GetMaxX())
					x_max = pl.GetMaxX();
				if (x_min > pl.GetMinX())
					x_min = pl.GetMinX();
				if (y_max < pl.GetMaxY())
					y_max = pl.GetMaxY();
				if (y_min > pl.GetMinY())
					y_min = pl.GetMinY();
				Point_element.push_back(pl);
			}
		}
	}

	// 绘制函数
	void Draw(HDC hdc, double xrate, double yrate)
	{
		for (int i = 0; i < Poly_element.size(); ++i)
		{
			Poly_element[i].MultiDraw(hdc, xrate, yrate, this->GetMaxX(), this->GetMinX(), this->GetMaxY(), this->GetMinY());
		}
		for (int i = 0; i < Line_element.size(); ++i)
		{
			Line_element[i].MultiDraw(hdc, xrate, yrate, this->GetMaxX(), this->GetMinX(), this->GetMaxY(), this->GetMinY());
		}
		for (int i = 0; i < Point_element.size(); ++i)
		{
			Point_element[i].Draw(hdc, xrate, yrate);
		}
	}
};
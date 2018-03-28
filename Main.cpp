#include "RenderManager.h"
#include <cmath>
#include<string>
#include<iostream>
#include<cstdio>
#include<vector>
#include<stdlib.h>
using std::cin;
using std::cout;
using std::endl;
using std::string;
#pragma  region 扫描线
int ymax = 0, ymin = 0;//多边形y的最大值和最小值
int line = 0;
AColor policolor;
class EDGE {

public:
	double xi;//边的下端点x坐标，在活化链表（AET）中，表示扫描线与边的交点x坐标
	double dx;//是个常量（直线斜率的倒数）（x + dx, y + 1）
	int ymax;//边的上顶点的y值
	friend bool operator <(EDGE a, EDGE b)//重载排列关系
	{
		return (abs(a.xi - b.xi) < 1 ? a.dx < b.dx : a.xi < b.xi);
	}
	friend bool operator >(EDGE a, EDGE b)//重载排列关系
	{
		return (abs(a.xi - b.xi) < 1 ? a.dx > b.dx : a.xi > b.xi);
	}
	friend bool operator ==(EDGE a, EDGE b)//重载等于号
	{
		return (abs(a.xi - b.xi) < 1 && a.dx == b.dx && a.ymax == b.ymax);
	}
	friend bool operator !=(EDGE a, EDGE b)//重载不等于号
	{
		return (abs(a.xi - b.xi) > 1 || a.dx != b.dx || a.ymax != b.ymax);
	}
};
vector<EDGE> allaet;
void My_Sort(vector<EDGE> &list)
{
	EDGE d;
	for (int i = 0; i < list.size() - 1; i++)
	{
		for (int j = i + 1; j < list.size(); j++)
		{
			if (list[j] < list[i])
			{
				d = list[j];
				list[j] = list[i];
				list[i] = d;
			}
		}
	}
}
void RemoveNonActiveEdgeFromAet(vector<EDGE> &AET, int y)
{
		line = y;
		for (auto it = AET.begin(); it != AET.end();)
		{
			if (line == (*it).ymax) 
				it = AET.erase(it);
			else 
				++it;
		}
}
/// <summary>
/// 更新边表中每项的xi值，就是根据扫描线的连贯性用dx对其进行修正，
/// 并且根据xi从小到大的原则对更新后的aet表重新排序
void UpdateAndResortAet(vector<EDGE> &AET)
{
	//更新xi
	
	for (auto& e : AET)//遍历stl需要&
	{
		e.xi += e.dx;
	}

	My_Sort(AET);
}
/// 扫描线填充处理过程
/// 开始对每条扫描线进行处理，对每条扫描线的处理有四个操作
void FillAetScanLine(vector<EDGE> &AET, int y)
{
	if (AET.size() < 2)
		return;
	for (int i = 0; i < AET.size(); i+=2)
	{
		int from = ((int)AET[i].xi + 1);
		int to = ((int)(AET[i + 1].xi + 1));
		while (from < to)
		{
			Vertex v;
			v.position = Vector3(from, y, 0);
			RenderManager::Instance().draw_pixel(v, policolor);
			from += 1;
		}
	}
}
void InsertNetListToAet(vector<EDGE> list, vector<EDGE> &AET)
{
	if (list.size() == 0) 
		return;
	if (AET.size() == 0)
	{
		AET = list;
		return;
	}
	vector<EDGE> temp;
	int i = 0, j = 0;
	while (i < list.size() && j < AET.size())
	{
		if (list[i] == AET[j])
		{
			i++;
			temp.push_back(AET[j]);
			j++;
			continue;
		}
		if (list[i] < AET[j])
		{
			temp.push_back(list[i]);
			i++;
			continue;
		}
		if (list[i] > AET[j])
		{
			temp.push_back(AET[j]);
			j++;
			continue;
		}
	}
	while (i < list.size())
	{
		temp.push_back(list[i]);
		i++;
	}
	while (j < AET.size())
	{
		temp.push_back(AET[j]);
		j++;
	}
	AET = temp;
}
void GetPolygonMinMax(vector<Vertex> vs)
{
	ymax = -1;
	ymin = 1000;
	for (int i = 0; i < vs.size(); i++)
	{
		if (vs[i].position.y > ymax)
			ymax = vs[i].position.y;
		if (vs[i].position.y < ymin)
			ymin = vs[i].position.y;
	}
	return;
}
void InitScanLineNewEdgeTable(vector<EDGE>* NET, vector<Vertex> vs, int ymin, int ymax)
{
	vector<int> temp;
	EDGE e;
	for (int i = 0; i < vs.size(); i++)
	{
		Vertex ps = vs[i];
		Vertex pe = vs[(i + 1) % vs.size()];
		Vertex pss = vs[(i - 1 + vs.size()) % vs.size()];
		Vertex pee = vs[(i + 2) % vs.size()];
		if (pe.position.y != ps.position.y)//不处理平行线
		{
			e.dx = (double)(pe.position.x - ps.position.x) / (double)(pe.position.y - ps.position.y);
			if (pe.position.y > ps.position.y)
			{
				e.xi = ps.position.x;
				if (pee.position.y >= pe.position.y)
					e.ymax = pe.position.y - 1;
				else
					e.ymax = pe.position.y;
				NET[(int)ps.position.y - ymin].push_back(e);//加入对应的NET里
				temp.push_back(ps.position.y - ymin);
			}
			else
			{
				e.xi = pe.position.x;
				if (pss.position.y >= ps.position.y)
					e.ymax = ps.position.y - 1;
				else
					e.ymax = ps.position.y;
				NET[(int)pe.position.y - ymin].push_back(e);//加入对应的NET里
				temp.push_back(pe.position.y - ymin);
			}
		}
	}
	for (int i = 0; i < temp.size(); i++)
	{
		My_Sort(NET[temp[i]]);
	}
}
void ProcessScanLineFill(vector<EDGE>* NET, int ymin, int ymax)
{
	vector<EDGE> AET;//扫描线
	for (int y = ymin; y < ymax; y += 1)
	{
		InsertNetListToAet(NET[y - ymin], AET);
		FillAetScanLine(AET, y);
		RemoveNonActiveEdgeFromAet(AET, y);//删除非活动边
		UpdateAndResortAet(AET);//更新活动边表中每项的xi值，并根据xi重新排序
	}
	allaet = AET;
}
void ScanLinePolygonFill(vector<Vertex> vs)
{
	vector<EDGE>* NET = new vector<EDGE>[500];//定义新边表
	GetPolygonMinMax(vs);//计算更新ymax和ymin(ok)
	InitScanLineNewEdgeTable(NET, vs, ymin, ymax);//初始化新边表
	ProcessScanLineFill(NET, ymin, ymax);
	delete[]NET;
}

#pragma endregion
void initwindow()
{
	const int WIDTH = 800;
	const int HEIGHT = 800;

	RenderManager& Mywindow = RenderManager::Instance();
	//Mywindow.game_start(WIDTH,HEIGHT);

	RenderWindow myWindow("Render", "Render", WIDTH, HEIGHT);
	//Camera,初始化屏幕
	Camera::instance(WIDTH, HEIGHT);
	//DirectX
	DX::instance().initialDirectX(GetModuleHandle(nullptr), myWindow.get_hwnd(), WIDTH, HEIGHT);
	
	return;

}
void DrawBzier(vector<Vertex> vs);
void DrawCube();
int main()
{
	policolor = AColor(0, 255, 255, 255);
	bool ifget = false;
	while (1)
	{
		cout << "请选择要画的图形：" << endl;
		cout << "1, 线" << endl;
		cout << "2, 圆" << endl;
		cout << "3, 椭圆" << endl;
		cout << "4, 多边形及填充裁剪" << endl;
		cout << "5, Bezier曲线" << endl;
		cout << "6, 渲染Cube" << endl;
		char choice;
		cin >> choice;

		//画线
		if (choice == '1')
		{
			
			Vertex v1, v2;
			cout << "请输入第1个点坐标，格式如(x,y): ";
			cin >> v1;
			cout << "请输入第2个点坐标，格式如(x,y): ";
			cin >> v2;
			initwindow();

#pragma region MessageLoop
			//时间初始化
			DWORD curTime = GetTickCount();
			DWORD preTime = GetTickCount();
			//2.消息循环
			MSG msg = { 0 };
			while (msg.message != WM_QUIT)
			{
				//获取消息
				if (PeekMessage(&msg, 0, NULL, NULL, PM_REMOVE)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				else
				{
					curTime = GetTickCount();
					//cout<<1000.0f/(curTime-preTime)<<endl;
					if (curTime - preTime > 30)
					{
						preTime = curTime;
						//GameManager::Instance().game_update();
						DX &directx = DX::instance();
						directx.fillSurface();
						directx.lockSurface();

						
						RenderManager::Instance().draw_lines(v1, v2, AColor(0, 255, 255, 255));

						directx.unlockSurface();
						directx.flipSurface();
					}
				}
			}
#pragma endregion
		}
		//画圆
		else if (choice == '2')
		{
			Vertex v; int r;
			cout << "请输入圆心坐标，格式(x,y): ";
			cin >> v;
			cout << "请输入半径长度: ";
			cin >> r;
			initwindow();
#pragma region MessageLoop
			//时间初始化
			DWORD curTime = GetTickCount();
			DWORD preTime = GetTickCount();
			//2.消息循环
			MSG msg = { 0 };
			while (msg.message != WM_QUIT)
			{
				//获取消息
				if (PeekMessage(&msg, 0, NULL, NULL, PM_REMOVE)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				else
				{
					curTime = GetTickCount();
					//cout<<1000.0f/(curTime-preTime)<<endl;
					if (curTime - preTime > 30)
					{
						preTime = curTime;
						//GameManager::Instance().game_update();
						DX &directx = DX::instance();
						directx.fillSurface();
						directx.lockSurface();

						RenderManager::Instance().draw_circle(v, r);

						directx.unlockSurface();
						directx.flipSurface();
					}
				}
			}
#pragma endregion
		}
		//画椭圆
		else if (choice == '3')
		{
			Vertex v;
			int lr, sr;
			cout << "请输入椭圆中心点坐标，格式(x,y): ";
			cin >> v;
			cout << "请输入椭圆长轴(x轴方向): ";
			cin >> lr;
			cout << "请输入椭圆短轴(y轴方向): ";
			cin >> sr;
			initwindow();
#pragma region MessageLoop
			//时间初始化
			DWORD curTime = GetTickCount();
			DWORD preTime = GetTickCount();
			//2.消息循环
			MSG msg = { 0 };
			while (msg.message != WM_QUIT)
			{
				//获取消息
				if (PeekMessage(&msg, 0, NULL, NULL, PM_REMOVE)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				else
				{
					curTime = GetTickCount();
					//cout<<1000.0f/(curTime-preTime)<<endl;
					if (curTime - preTime > 30)
					{
						preTime = curTime;
						//GameManager::Instance().game_update();
						DX &directx = DX::instance();
						directx.fillSurface();
						directx.lockSurface();

						RenderManager::Instance().draw_ellipse(v, lr, sr);

						directx.unlockSurface();
						directx.flipSurface();
					}
				}
			}
#pragma endregion
		}
		//画多边形
		else if (choice == '4')
		{
			int wxl, wxr, wyb, wyt;
			cout << "请输入多边形的边数，以便输入: ";
			int n;
			cin >> n;
			cout << "请按连接顺序输入每一个点的坐标，格式(x,y): " << endl;
			vector<Vertex> vs;
			vector<Vertex> vs2;
			for (int i = 0; i < n; i++)
			{
				Vertex temp;
				cout << "第" << i + 1 << "个点坐标: ";
				cin >> temp;
				vs.push_back(temp);
			}

			cout << "是否需要填充颜色，需要按1，否则按2: ";
			char choice2;
			char choice3;
			cin >> choice2;
			if (choice2 == '1')
			{
				cout << "请输入颜色的rgb值(0~255)，格式如 255 255 255："<<endl;
				int r, g, b;
				cin >> r >> g >> b;
				policolor.r_ = r;
				policolor.g_ = g;
				policolor.b_ = b;
				cout << "是否需要裁剪，需要按1，否则按2: ";
				cin >> choice3;
				if (choice3 == '1')
				{
					cout << "分别输入窗口的四条边界坐标："<<endl;
					cout << "wxl=";
					cin >> wxl;
					cout << "wxr=";
					cin >> wxr;
					cout << "wyb=";
					cin >> wyb;
					cout << "wyt=";
					cin >> wyt;
					Vertex v1, v2, v3, v4;
					v1.position = Vector3(wxl, wyt, 0);
					v2.position = Vector3(wxr, wyt, 0);
					v3.position = Vector3(wxr, wyb, 0);
					v4.position = Vector3(wxl, wyb, 0);
					vs2.push_back(v1);
					vs2.push_back(v2);
					vs2.push_back(v3);
					vs2.push_back(v4);
				}

			}

			initwindow();
#pragma region MessageLoop
			//时间初始化
			DWORD curTime = GetTickCount();
			DWORD preTime = GetTickCount();
			//2.消息循环
			MSG msg = { 0 };
			while (msg.message != WM_QUIT)
			{
				//获取消息
				if (PeekMessage(&msg, 0, NULL, NULL, PM_REMOVE)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				else
				{
					curTime = GetTickCount();
					//cout<<1000.0f/(curTime-preTime)<<endl;
					if (curTime - preTime > 30)
					{
						preTime = curTime;
						//GameManager::Instance().game_update();
						DX &directx = DX::instance();
						directx.fillSurface();
						directx.lockSurface();

						RenderManager::Instance().draw_poligon(vs, AColor(0, 255, 255, 255));
						if (choice2 == '1')
						{
							ScanLinePolygonFill(vs);
							if (choice3 == '1')
							{
								RenderManager::Instance().draw_poligon(vs2, AColor(0, 255, 255, 255));
								for (int i = 0; i < 800; i++)
									for (int j = 0; j < 800; j++)
										if (i<wxl || i>wxr || j<wyt || j>wyb)
											DX::instance().drawPixel(i, j, AColor(0, 0, 0, 0));
							}
						}

						directx.unlockSurface();
						directx.flipSurface();
					}
				}
			}
#pragma endregion
		}
		else if (choice == '5')
		{
			
			cout << "请输入点的数量: ";
			int n;
			cin >> n;
			cout << "请按连接顺序输入每一个点的坐标，格式(x,y): " << endl;
			vector<Vertex> vs;
			for (int i = 0; i < n; i++)
			{
				Vertex temp;
				cout << "第" << i + 1 << "个点坐标: ";
				cin >> temp;
				vs.push_back(temp);
			}
			/*
			Vertex v1,v2,v3,v4;
			v1.position = Vector3(100, 120, 0);
			v2.position = Vector3(200, 100, 0);
			v3.position = Vector3(300, 200, 0);
			v4.position = Vector3(250, 150, 0);
			vector<Vertex> vs;
			vs.push_back(v1);
			vs.push_back(v2);
			vs.push_back(v3);
			vs.push_back(v4);*/

			initwindow();
#pragma region MessageLoop
			//时间初始化
			DWORD curTime = GetTickCount();
			DWORD preTime = GetTickCount();
			//2.消息循环
			MSG msg = { 0 };
			while (msg.message != WM_QUIT)
			{
				//获取消息
				if (PeekMessage(&msg, 0, NULL, NULL, PM_REMOVE)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				else
				{
					curTime = GetTickCount();
					//cout<<1000.0f/(curTime-preTime)<<endl;
					if (curTime - preTime > 30)
					{
						preTime = curTime;
						//GameManager::Instance().game_update();
						DX &directx = DX::instance();
						directx.fillSurface();
						directx.lockSurface();

						DrawBzier(vs);

						directx.unlockSurface();
						directx.flipSurface();
					}
				}
			}
#pragma endregion
		}
		else if (choice == '6')
		{
			RenderManager::Instance().Cube_start(800,800);
		}
		system("cls");
	}
	return 0;
}
//Bezier曲线
void DrawBzier(vector<Vertex> vs)
{
	int n = vs.size()-1;
	if (n <= 0)
		return;
	if ((vs[n].position.x < vs[0].position.x + 1) && (vs[n].position.x > vs[0].position.x - 1) 
		&& (vs[n].position.y < vs[0].position.y + 1) && (vs[n].position.y > vs[0].position.y - 1))
	{
		RenderManager::Instance().draw_pixel(vs[0], AColor(0, 255, 255, 255));
		return;
	}
	vector<Vertex> p1(n+1);
	int i, j;
	p1[0] = vs[0];
	for (i = 1; i <= n; i++)
	{
		for (j = 0; j <= n - i; j++)
		{
			vs[j].position.x = (vs[j].position.x + vs[j + 1].position.x) / 2;
			vs[j].position.y = (vs[j].position.y + vs[j + 1].position.y) / 2;
		}
		p1[i] = vs[0];
	}
	DrawBzier(vs);
	DrawBzier(p1);
}
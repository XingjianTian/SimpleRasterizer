#include<cmath>
#include<vector>
#include"Vertex.h"
#include"GameManager.h"
int ymax = 0, ymin = 0;//多边形y的最大值和最小值
class EDGE{

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
void InitScanLineNewEdgeTable(vector<EDGE>* NET,vector<Vertex> vs, int ymin, int ymax)
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
					e.ymax = ps.position.y-1;
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
void My_Sort(vector<EDGE> &list)
{
	EDGE d;
	for (int i = 0; i < list.size() - 1; i++)
	{
		for (int j = i + 1; j < list.size(); j++)//瞎！for (int j = i+1; i < list.Count; i++)
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

int line = 0;
void RemoveNonActiveEdgeFromAet(vector<EDGE> &AET, int y)
{
	line = y;
	for (auto it = AET.begin(); it !=AET.end(); it++)
	{
		if ((*it).ymax == line)
			AET.erase(it);
	}
}
/// <summary>
/// 更新边表中每项的xi值，就是根据扫描线的连贯性用dx对其进行修正，
/// 并且根据xi从小到大的原则对更新后的aet表重新排序
/// </summary>
/// <param name="AET"></param>

void UpdateAndResortAet(vector<EDGE> &AET)
{
	//更新xi
	for(auto e:AET)
	{
		e.xi += e.dx;;
	}
	My_Sort(AET);
}

/// <summary>
/// 扫描线填充处理过程
/// 开始对每条扫描线进行处理，对每条扫描线的处理有四个操作
/// </summary>
/// <param name="NET"></param>
/// <param name="ymin"></param>
/// <param name="ymax"></param>
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
}
void FillAetScanLine(vector<EDGE> &AET, int y)
{
	if (AET.size()< 2) 
		return;
	for (int i = 0; i < AET.size(); i += 1)
	{
		int from = ((int)AET[i].xi + 1);
		int to = ((int)(AET[i + 1].xi + 1 / 2));
		while (from < to)
		{
			Vertex v;
			v.position = Vector3(from, y, 0);
			GameManager::Instance().draw_pixel(v, AColor(0, 255, 255, 255));
			from += 1;
		}
	}
}
void InsertNetListToAet(vector<EDGE> list, vector<EDGE> &AET)
{
	if (list.size()== 0) return;
	if (AET.size() == 0)
	{
		AET = list;
		return;
	}//刚开始这里写成if（）AET=list;return;一直出错！下次一定要规范！！！
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


#ifndef RENDER_VERTEX_H_
#define RENDER_VERTEX_H_
#include "Vector3.h"
#include "MyMath.h"
#include "VColor.h"
#include<string>
class Vertex
{
public:
	Vertex(const Vector3 &position_,const VColor &color_,float u_,float v_,Vector3 normal_=(Vector3())):position(position_),color(color_),u(u_),v(v_),inv_w(1.0f),normal(normal_){}
	Vertex():position(),color(0.0f,0.0f,0.0f,0.0f),u(0.0f),v(0.0f){}
	Vertex interp(const Vertex &v,float factor);
	bool isxy_same(const Vertex &v)
	{
		return position.x==v.position.x&&position.y==v.position.y;
	}
	void set_normal(Vector3 n)
	{
		normal = n;
	}

	Vector3 position;//位置
	Vector3 normal;//法线
	VColor color;//颜色
	VColor light;//光照模型实际上是三种光照模型的加和！！！
	float u;
	float v;
	float inv_w;

	friend istream& operator>>(istream & is, Vertex& v)
	{
		string s;
		is >> s;
		s.erase(0, 1);
		s.erase(s.length() - 1, 1);
		int i;
		for (i = 0; i < s.length(); i++)
		{
			if (s[i] == ',')
				break;
		}
		Vector3 p;
		p.x = atoi(s.substr(0, i).c_str());
		p.y = atoi(s.substr(i + 1, s.length()).c_str());
		p.z = 0;
		v.position = p;

		return is;
	}
};
#endif
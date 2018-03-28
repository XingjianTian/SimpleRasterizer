#include "RenderManager.h"
#include <stdlib.h>
RenderManager& RenderManager::Instance()
{
	static RenderManager gameManager;
	return gameManager;
}

void RenderManager::Cube_start(const int WIDTH, const int HEIGHT)
{
	//Window
	RenderWindow myWindow("Render", "Renderer", WIDTH, HEIGHT);
	//Camera
	Camera::instance(WIDTH, HEIGHT);
	//DirectX
	DX::instance().initialDirectX(GetModuleHandle(nullptr), myWindow.get_hwnd(), WIDTH, HEIGHT);
	//Cube
	pcube = new Cube(1, "Yu.png", Vector3(0, 0, 6));
	//Rasterizer
	rasterizer.set_texture(&pcube->texture);
	//Light
	light.position = Vector3(-1000, 1000, 7);
	light.diffuse = VColor(0.0f, 0.7f, 0.7f, 0.7f);
	light.specular = VColor(0.0f, 1.0f, 1.0f, 1.0f);
	light.ambient = VColor(0.0f, 0.4f, 0.4f, 0.4f);
	//Message Loop

	myWindow.message_dispatch();//enter msg loop（call draw_update call draw_cube）
}

void RenderManager::Draw_update()
{
	
	DX &directX = DX::instance();
	directX.fillSurface();
	directX.lockSurface();

	draw_cube();
	//draw_pixels();
	//draw_lines();
	//draw_circle(Vertex(400,400),200);
	//draw_ellipse(400,400,200,100);

	directX.unlockSurface();
	directX.flipSurface();
}
void RenderManager::Draw_end()
{
	cout<<"Display end!"<<endl;
}

void RenderManager::fill_point(int cx,int cy,int x,int y)
{
	DX &directX = DX::instance();
	AColor color(0,255,255,255);
	
	directX.drawPixel(x+cx,y+cy,color);
	directX.drawPixel(-x+cx,y+cy,color);
	directX.drawPixel(x+cx,-y+cy,color);
	directX.drawPixel(-x+cx,-y+cy,color);
}

void RenderManager::draw_pixel(Vertex v, AColor color)
{
	DX &directX = DX::instance();
	directX.drawPixel(v.position.x, v.position.y, color);
}
AColor RenderManager::get_pixel(Vertex v)
{
	return DX::instance().getPixel(v.position.x, v.position.y);
	
}

void RenderManager::draw_pixels()
{
	DX &directX = DX::instance();
	for (int i=0;i<100;++i) {
		for (int j=0;j<100;++j) {
			directX.drawPixel(i,j,AColor(0,255,255,255));
		}
	}
}

//线
void RenderManager::draw_lines(Vertex v1, Vertex v2, AColor color)
{
	rasterizer.drawline_bresenham(v1.position.x, v1.position.y, v2.position.x, v2.position.y,color);
}

//多边形
void RenderManager::draw_poligon(vector<Vertex> vs, AColor color)
{
	for (int i = 0; i < vs.size() - 1; i++)
	{
		rasterizer.drawline_bresenham(vs[i].position.x, vs[i].position.y, vs[i + 1].position.x, vs[i + 1].position.y, color);
	}
	rasterizer.drawline_bresenham(vs[vs.size() - 1].position.x, vs[vs.size() - 1].position.y, vs[0].position.x, vs[0].position.y, color);
}

//中点画圆法
void RenderManager::draw_circle(Vertex v,int r)
{
	int x = 0,y = r;
	int p = 1-r;
	while (x<=y)
	{
		fill_point(v.position.x,v.position.y,x,y);
		fill_point(v.position.x,v.position.y,y,x);
		if (p<0)
		{
			p += 2*x+3;
		}
		else
		{
			p += 2*(x-y)+5;
			--y;
		}
		++x;
	}
}

//中点画椭圆
void RenderManager::draw_ellipse(Vertex v,int rx,int ry)
{
	int A = rx*rx,B = ry*ry,C = A*B;
	int px = B-A*ry+A/4;
	int x = 0,y = ry;
	while (x*B<y*A)
	{
		fill_point(v.position.x,v.position.y,x,y);
		if (px<0)
		{
			px += B*(2*x+3);
		}
		else
		{
			px += B*(2*x+3)+A*(3-2*y);
			--y;
		}
		++x;
	}
	int py = B*(x*x+x)+B/4+A*(y*y+1-2*y)-C;
	while (y>=0)
	{
		fill_point(v.position.x, v.position.y,x,y);
		if (py<0)
		{
			py += A*(3-2*y)+B*(2+2*x);
			++x;
		}
		else
		{
			py += A*(3-2*y);
		}
		--y;
	}
}

//3dcube
void RenderManager::draw_cube()
{ 
	/*1，坐标转换（局部，世界）
	  2，光照
	  3，投影
	  4，背面消影
	  5，裁剪、屏幕坐标转换（视口坐标系）

	  6，光栅化 
		a，扫描转换
		b，纹理贴图
		c，可见性测试
		d，帧缓冲
	*/
	Camera &camera = Camera::instance();
	//1.模型空间————(模型变换)————>世界空间
	Vector3 rotate_vector(1.0f, 1.0f, 1.0f);
	rotate_vector.normalize();
	Matrix cube_rotate_matrix;
	cube_rotate_matrix.identify();
	cube_rotate_matrix.setRotate(rotate_vector, 1);


	Matrix model_move_matrix;
	model_move_matrix.identify();
	model_move_matrix.setTranslation(pcube->position);//按位置位移

	for (int i = 0; i < pcube->trans_vertexes.size(); ++i)
	{
		//局部坐标系
		pcube->local_vertexes[i].position = pcube->local_vertexes[i].position*cube_rotate_matrix;//物体旋转
		pcube->trans_vertexes[i] = pcube->local_vertexes[i];//旋转后
		pcube->trans_vertexes[i].position = pcube->trans_vertexes[i].position*model_move_matrix;//物体移动
		pcube->trans_vertexes[i].normal = pcube->trans_vertexes[i].position - pcube->position;//法线方向
		pcube->trans_vertexes[i].normal.normalize();//规范化

		//光照处理
		VColor light_color;
		//Diffuse
		Vector3 l = light.position - pcube->trans_vertexes[i].position;
		l.normalize();
		float nl = pcube->trans_vertexes[i].normal*l;
		pcube->trans_vertexes[i].light += /*表示法线*/max(nl, 0)*light.diffuse*pcube->material.diffuse;
		//Specular
		if (nl > 0)
		{
			Vector3 r = 2 * pcube->trans_vertexes[i].normal*nl - l;//R=2*dot(N,L)*N-L.Phong光照模型
			r.normalize();
			Vector3 v = camera.get_position() - pcube->trans_vertexes[i].position;
			v.normalize();
			pcube->trans_vertexes[i].light += /*表示法线*/max(r*v, 0)*light.specular*pcube->material.specular;
		}

		//Ambient
		pcube->trans_vertexes[i].light += light.ambient*pcube->material.ambient;
	}
	//2.世界空间————(观察变换)————>相机空间
	camera.eye_transform(pcube->trans_vertexes);
	//背面消隐剔除
	vector<Triangle> triangles = Camera::instance().get_front_triangle(pcube->trans_vertexes);

	/*3.相机空间————(投影变换)————>裁剪空间（CVV裁剪）
	顶点从模型空间到世界空间到相机空间到齐次裁剪空间，对应的主要是三个矩阵world view proj
	图元装配，也就是将顶点以一定顺序组装成三角形，本项目中采用顶点索引的方式来组织顶点
	并且三角形顺时针顺序对应的法线朝向外面。之所以规定好顺序，是为了法线方向的一致性
	只有这样才能进行后面的背面消隐。
	投影后的坐标再进行透视除法，让顶点坐标位于cvv中(x[-1, 1], y[-1, 1], z[0, 1])
	裁剪过后再将cvv坐标转化为屏幕中对应的坐标。*/
	triangles = Camera::instance().cvv_clip(pcube->trans_vertexes, triangles);

	//4.裁剪空间————(屏幕映射)————>屏幕空间
	camera.screen_map(pcube->trans_vertexes);
	//5.光栅化，对每个三角形单元显示
	for (auto &triangle : triangles)
	{
		rasterizer.draw_triangle(pcube->trans_vertexes[triangle.index1], pcube->trans_vertexes[triangle.index2], pcube->trans_vertexes[triangle.index3]);
	}
}
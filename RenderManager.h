#ifndef RENDER_GAMEMANAGER_H_
#define RENDER_GAMEMANAGER_H_
#include "RenderWindow.h"
#include "Rasterizer.h"
#include "Cube.h"
#include "Matrix.h"
#include "Camera.h"
class RenderManager
{
public:
	int wid = 800;
	int hei= 800;
	static RenderManager& Instance();
	Rasterizer rasterizer;
	void Cube_start(const int WIDTH, const int HEIGHT);
	void Draw_update();
	void Draw_end();

	void fill_point(int cx,int cy,int x,int y);
	void draw_pixel(Vertex v, AColor color);
	void draw_pixels();
	void draw_lines(Vertex v1, Vertex v2, AColor color);
	void draw_cube();
	void draw_circle(Vertex v,int r);
	void draw_ellipse(Vertex v,int rx,int ry);
	void draw_poligon(vector<Vertex> v, AColor color);
	AColor get_pixel(Vertex v);
private:
	RenderManager():pcube(nullptr){}
	~RenderManager()
	{
		if (pcube!=nullptr)
			delete pcube;
	}
	
	Cube *pcube;
	Light light;
	
};
#endif
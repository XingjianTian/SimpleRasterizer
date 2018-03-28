#ifndef DIRECTXLIB_H_
#define DIRECTXLIB_H_

#include <d3d9.h>
#include <d3dx9.h>
#include <windows.h>
#include <iostream>
#include "Enum.h"
#include "MyMath.h"
using namespace std;
class DX
{
public:
	//单例获取
	static DX& instance()//static函数
	{
		static DX* pDirectX = new DX();
		return *pDirectX;
	}
	//初始化DirectX
	bool initialDirectX(HINSTANCE hInstance, HWND hWnd, int width, int height);
	void fillSurface();
	void lockSurface();
	void unlockSurface();
	//绘制像素到缓冲区
	void drawPixel(int x, int y, AColor color);//,float depth=0.0f);
	//读取像素
	AColor DX::getPixel(int x, int y);
	//缓冲区翻转显示
	void flipSurface();
	DWORD ARGB(int a,int r,int g,int b){return DWORD((a<<24)+(r<<16)+(g<<8)+b);}
	//析构
	~DX();

private:
	DX():pD3DXDevice(nullptr),pD3DSurface(nullptr){}
	IDirect3DDevice9* pD3DXDevice;//设备
	IDirect3DSurface9* pD3DSurface;//表面
	D3DLOCKED_RECT LockRect;//指向表面存储区
	float *z_buffer_;//vertexbuffer
	int width_;
	int height_;
	int buffer_size_;
};


#endif
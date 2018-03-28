#include "DirectXLib.h"
bool DX::initialDirectX(HINSTANCE hInstance, HWND hWnd, int width, int height)
{
	//1.创建接口 物理硬件
	IDirect3D9* d3d9 = Direct3DCreate9(D3D_SDK_VERSION);
	//2.获取硬件信息，确定顶点处理方式
	D3DCAPS9 caps;//检查设备性能
	int vertexProcessing;
	d3d9->GetDeviceCaps(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,&caps);    //(当前显卡，硬件设备)
	if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
		vertexProcessing = D3DCREATE_HARDWARE_VERTEXPROCESSING;//硬件顶点计算
	else
		vertexProcessing = D3DCREATE_SOFTWARE_VERTEXPROCESSING;//软件
	//3.填写D3D参数
	D3DPRESENT_PARAMETERS d3dpp;
	d3dpp.BackBufferWidth            = width;
	d3dpp.BackBufferHeight           = height;
	d3dpp.BackBufferFormat           = D3DFMT_A8R8G8B8;
	d3dpp.BackBufferCount            = 2;//所需使用的后台缓存个数
	d3dpp.MultiSampleType            = D3DMULTISAMPLE_NONE;
	d3dpp.MultiSampleQuality         = 0;
	d3dpp.SwapEffect                 = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow              = hWnd;//窗口句柄
	d3dpp.Windowed                   = true;//是否窗口化
	d3dpp.EnableAutoDepthStencil     = true;
	d3dpp.AutoDepthStencilFormat     = D3DFMT_D24S8;
	d3dpp.Flags                      = 0;
	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	d3dpp.PresentationInterval       = D3DPRESENT_INTERVAL_IMMEDIATE;

	//4.创建设备
	d3d9->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_REF,///type
		hWnd,
		vertexProcessing,
		&d3dpp,
		&pD3DXDevice);//返回创建的设备

	d3d9->Release();//刷新
	//创建一个绘制表面
	pD3DXDevice->CreateOffscreenPlainSurface(width, height, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &pD3DSurface, 0);
	//保存屏幕宽、高、Z缓冲区大小
	width_ = width;
	height_ = height;
	buffer_size_ = width_*height_;//缓冲区大小
	//z_buffer开辟空间
	z_buffer_ = new float[buffer_size_];
	return true;
}

//屏幕填充刷新，Z缓冲初始化
void DX::fillSurface()
{
	pD3DXDevice->ColorFill(pD3DSurface,NULL,D3DCOLOR_XRGB(0,0,0));//全黑
	fill(z_buffer_,z_buffer_+buffer_size_,FLT_MAX);
}

void DX::lockSurface()
{
	// 创建并初始化锁定区域
	memset(&LockRect, 0, sizeof(LockRect));
	// 锁定
	pD3DSurface->LockRect(&LockRect,NULL,D3DLOCK_DISCARD);
}

void DX::unlockSurface()
{
	// 解锁
	pD3DSurface->UnlockRect();
}

void DX::drawPixel(int x,int y, AColor color)//,float depth)
{
	//像素着色
	int index = y*(width_)+x;
	//if (z_buffer_[index]>depth)
	//{
		DWORD* pBits = (DWORD*)LockRect.pBits;
		pBits[x + y * (LockRect.Pitch >> 2)] = 
			DWORD((color.a_<<24)+(color.r_<<16)+(color.g_<<8)+color.b_);
		//z_buffer_[index] = depth;
//	}
}
AColor DX::getPixel(int x, int y)
{
	DWORD* pBits = (DWORD*)LockRect.pBits;
	//LockRect.Pitch表示WIDTH所占的字节数
	DWORD c;
	c = pBits[x + y * (LockRect.Pitch >> 2)];
	AColor color = AColor((c >> 24) & 0xff, (c >> 16) & 0xff, (c >> 8) & 0xff, c & 0xff);
	return color;
	
}


//Swap FrameBuffer
void DX::flipSurface()
{
	// 获取后台缓存
	IDirect3DSurface9* backBuffer = 0;
	pD3DXDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backBuffer);
	// 使用自定义表面填充后台缓存
	pD3DXDevice->StretchRect(pD3DSurface, 0, backBuffer, 0, D3DTEXF_NONE);
	// 减少引用计数，否则会内存泄露
	backBuffer->Release();

	// 将交换链中的后台缓存显示
	pD3DXDevice->Present(NULL, NULL, NULL, NULL);
}

DX::~DX()
{
	if (pD3DSurface!=NULL)
	{
		pD3DSurface->Release();
	}
	if (pD3DXDevice!=NULL)
	{
		pD3DXDevice->Release();
	}
}
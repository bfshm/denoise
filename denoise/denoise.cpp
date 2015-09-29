// denoise.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
namespace fs = std::experimental::filesystem;

uint32_t	g_threshold = 140;
uint32_t	g_out_width = 600;

class UsingGdiplus
{
public:
	UsingGdiplus()
	{
		m_gdiplus_startup_input = new Gdiplus::GdiplusStartupInput();
		Gdiplus::GdiplusStartup(&m_gdiplus_token, m_gdiplus_startup_input, nullptr);
	}

	~UsingGdiplus()
	{
		Gdiplus::GdiplusShutdown(m_gdiplus_token);
		delete m_gdiplus_startup_input;
	}

private:
	Gdiplus::GdiplusStartupInput*	m_gdiplus_startup_input;
	ULONG_PTR						m_gdiplus_token;
};


int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	using namespace Gdiplus;

	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);

	if(size == 0)
		return -1;  // Failure

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if(pImageCodecInfo == nullptr)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for(UINT j = 0; j < num; ++j)
	{
		if(wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}

void Denoise(std::unique_ptr<Gdiplus::Bitmap>& bmp)
{
	auto width = bmp->GetWidth();
	auto height = bmp->GetHeight();

	Gdiplus::Rect rc(0, 0, width, height);
	Gdiplus::BitmapData data;

	ATLASSERT(PixelFormat24bppRGB == bmp->GetPixelFormat());
	bmp->LockBits(&rc, Gdiplus::ImageLockModeRead | Gdiplus::ImageLockModeWrite, PixelFormat24bppRGB, &data);

	auto step = data.Stride;
	auto offset = step - width * 3;
	auto p = (uint8_t *)data.Scan0;

	for(auto y = 0U; y < height; ++y)
	{
		for(auto x = 0U; x < width; ++x)
		{
			auto r = p[0];
			auto g = p[1];
			auto b = p[2];

			if(r > g_threshold && g > g_threshold && b > g_threshold)
			{
				p[0] = p[1] = p[2] = 0xFF;
			}

			p += 3;
		}

		p += offset;
	}

	bmp->UnlockBits(&data);

}

std::unique_ptr<Gdiplus::Bitmap> Convert(std::unique_ptr<Gdiplus::Bitmap> bmp)
{
	auto factor = bmp->GetWidth() / (double)g_out_width;
	auto height = (uint32_t)(bmp->GetHeight() / factor);

	std::unique_ptr<Gdiplus::Bitmap> out(new Gdiplus::Bitmap(g_out_width, height, PixelFormat24bppRGB));
	out->SetResolution(Gdiplus::REAL(bmp->GetHorizontalResolution() / factor), Gdiplus::REAL(bmp->GetVerticalResolution() / factor));

	{
		std::unique_ptr<Gdiplus::Graphics> graph(Gdiplus::Graphics::FromImage(out.get()));
		graph->DrawImage(bmp.get(), 0, 0, g_out_width, height);
	}
	
	return out;
}

int wmain(int argc, wchar_t* argv[])
{
	_wsetlocale(LC_ALL, L"chs");

	UsingGdiplus using_gdiplus;

	wprintf_s(L"======================图像去噪工具===========================\n");
	wprintf_s(L"本工具专用于处理tif/jpg/png格式图片，其它格式不支持\n");
	wprintf_s(L"将本工具放在要处理的图片文件夹内 \n");
	wprintf_s(L"处理后的图像将被存放到output子文件 \n\n");
	wprintf_s(L"命令行参数说明：\n");
	wprintf_s(L"> -t 去噪阈值\n");
	wprintf_s(L"> -w 输出图像尺寸，默认600p\n");
	wprintf_s(L"============================================================\n\n");
	
	try
	{
		for(auto i = 0; i < argc; ++i)
		{
			if(0 == wcscmp(argv[i], L"-t") && i + 1 < argc)
			{
				g_threshold = std::stoul(argv[i + 1]);
				continue;
			}
			if(0 == wcscmp(argv[i], L"-o") && i + 1 < argc)
			{
				g_out_width = std::stoul(argv[i + 1]);
				continue;
			}
		}

	}
	catch(std::exception&)
	{
		wprintf_s(L"命令行参数错误，请重新输入！\n");
		return 0;
	}
	
	wprintf_s(L"当前设置值：t = %d, w = %d\n", g_threshold, g_out_width);
	wprintf_s(L"扫描当前目录所有图像：%s\n", fs::current_path().c_str());

	auto outdir = fs::current_path() / fs::path(L"output");
	fs::create_directories(outdir);

	for(fs::directory_iterator iter(fs::current_path()); iter != fs::directory_iterator(); ++iter)
	{
		auto path = iter->path();
		auto ext = path.extension().wstring();

		if(!(ext == L".tif" || ext == L".png" || ext == L".jpg"))
		{
			continue;
		}

		auto outpath = (outdir / fs::path(path.filename())).replace_extension(L"png");

		wprintf_s(L"处理 %s -> %s\n", path.c_str(), outpath.c_str());

		std::unique_ptr<Gdiplus::Bitmap> bmp(Gdiplus::Bitmap::FromFile(path.c_str()));

		bmp = Convert(std::move(bmp));
		Denoise(bmp);

		CLSID clsid;
		GetEncoderClsid(L"image/png", &clsid);

		bmp->Save(outpath.c_str(), &clsid);
	}

    return 0;
}


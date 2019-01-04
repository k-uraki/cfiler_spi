#include "spi.h"
#include <stdio.h>
#include "../../cterm/ctermcore.h"

//---- cfiler_native.cpp から流用＋α
#ifdef DEBUG
	struct FuncTrace
	{
		FuncTrace( const char * _funcname, unsigned int _lineno )
		{
			funcname = _funcname;
			lineno   = _lineno;

			printf( "FuncTrace : Enter : %s(%)\n", funcname, lineno );
		}

		~FuncTrace()
		{
			printf( "FuncTrace : Leave : %s(%)\n", funcname, lineno );
		}

		const char * funcname;
		unsigned int lineno;
	};
	#define FUNC_TRACE FuncTrace functrace(__FUNCTION__,__LINE__)
	#define _TRACE(...)		printf(__VA_ARGS__)
#else
	#define FUNC_TRACE
	#define _TRACE(...)		((void)0)
#endif
//---- cfiler_native.cpp から流用＋α


//---- from pythonutil.hから流用
namespace StringUtil
{
	std::wstring MultiByteToWideChar( const char * str, int len );
	std::string WideCharToMultiByte( const wchar_t * str, int len );
};
std::wstring StringUtil::MultiByteToWideChar( const char * str, int len )
{
	int buf_size = len+2;
	wchar_t * buf = new wchar_t[ buf_size ];
	int write_len = ::MultiByteToWideChar( 0, 0, str, len, buf, buf_size );
	buf[write_len] = '\0';

	std::wstring ret = buf;

	delete [] buf;

	return ret;
}

std::string StringUtil::WideCharToMultiByte( const wchar_t * str, int len )
{
	int buf_size = len*2+2;
	char * buf = new char[ buf_size ];
	int write_len = ::WideCharToMultiByte( 0, 0, str, len, buf, buf_size, NULL, NULL );
	buf[write_len] = '\0';

	std::string ret = buf;

	delete [] buf;

	return ret;
}

bool PythonUtil::PyStringToString( const PyObject * pystr, std::string * str )
{
	if( PyUnicode_Check(pystr) )
	{
		*str = StringUtil::WideCharToMultiByte( (const wchar_t*)PyUnicode_AS_UNICODE(pystr), PyUnicode_GET_SIZE(pystr) );
		return true;
	}
	else if( PyString_Check(pystr) )
	{
		*str = PyString_AS_STRING(pystr);
		return true;
	}
	else
	{
		PyErr_SetString( PyExc_TypeError, "must be string or unicode." );
		*str = "";
		return false;
	}
}

bool PythonUtil::PyStringToWideString( const PyObject * pystr, std::wstring * str )
{
	if( PyUnicode_Check(pystr) )
	{
		*str = (wchar_t*)PyUnicode_AS_UNICODE(pystr);
		return true;
	}
	else if( PyString_Check(pystr) )
	{
		*str = StringUtil::MultiByteToWideChar( (const char*)PyString_AS_STRING(pystr), PyString_GET_SIZE(pystr) );
		return true;
	}
	else
	{
		PyErr_SetString( PyExc_TypeError, "must be string or unicode." );
		*str = L"";
		return false;
	}
}
//---- from pythonutil.hから流用


CSPI::CSPI() : 
	ref_count(0),
	_hDll(0),
	_funcGetPluginInfo	(NULL),
	_funcIsSupported	(NULL),
	_funcGetPictureInfo	(NULL),
	_funcGetPicture		(NULL)
{
	
}
CSPI::~CSPI()
{
	term();
}

int CSPI::loadDll(const WCHAR *dll_fname)
{
	if(_hDll) term();
	
	_hDll = ::LoadLibrary(dll_fname);
	if(_hDll == NULL)
	{
		// ロード失敗
		return 0;
	}
	
	_funcGetPluginInfo		= reinterpret_cast<GET_PLUGIN_INFO >(::GetProcAddress(_hDll, "GetPluginInfo"));
	_funcIsSupported		= reinterpret_cast<IS_SUPPORTED    >(::GetProcAddress(_hDll, "IsSupported"));
	_funcGetPictureInfo		= reinterpret_cast<GET_PICTURE_INFO>(::GetProcAddress(_hDll, "GetPictureInfo"));
	_funcGetPicture			= reinterpret_cast<GET_PICTURE     >(::GetProcAddress(_hDll, "GetPicture"));
	
	// 関数取得できない
	if(_funcGetPluginInfo		== NULL ||
	   _funcIsSupported			== NULL ||
	   _funcGetPictureInfo		== NULL ||
	   _funcGetPicture			== NULL)
		return 0;
	
	return 1;
}

void CSPI::term()
{
	_TRACE("CSPI::term\n");
	if(_hDll)
	{
		_TRACE("  free DLL\n");
		FreeLibrary(_hDll);
		_hDll = NULL;
	}
}


// GetPluginInfo系
bool CSPI::getPluginInfoAPIVer(std::string *s)
{
	int		ret;
	char	buf[256+1];
	
	if(_funcGetPluginInfo)
	{
		ret = _funcGetPluginInfo(0, buf, 256);
		if(ret)
		{
			*s = buf;
			return true;
		}
	}
	return false;
}
bool CSPI::getPluginInfoAbout (std::string *s)
{
	int		ret;
	char	buf[256+1];
	
	if(_funcGetPluginInfo)
	{
		ret = _funcGetPluginInfo(1, buf, 256);
		if(ret)
		{
			*s = buf;
			return true;
		}
	}
	return false;
}
bool CSPI::getPluginInfoExt   (int no, std::string *s)
{
	int		ret;
	char	buf[256+1];
	
	if(_funcGetPluginInfo)
	{
		ret = _funcGetPluginInfo(2 + no * 2, buf, 256);
		if(ret)
		{
			*s = buf;
			return true;
		}
	}
	return false;
}

// IsSupported系
bool CSPI::isSupported(LPCSTR filename)
{
	int	ret;
	unsigned char	buf[1024*2];
	
	if(_funcIsSupported)
	{
		FILE	*fp = fopen(filename, "rb");
		if(fp == NULL) return false;
		long	read_len = fread(buf, 1, sizeof(buf), fp);
		if(read_len < sizeof(buf)) memset(buf + read_len, 0, sizeof(buf)-read_len);
		fclose(fp);
		
		ret = _funcIsSupported(NULL, (DWORD)buf);
		
		return ret ? true : false;
	}
	return false;
}
bool CSPI::isSupportedMem(const void *fimg)
{
	int	ret;
	
	if(_funcIsSupported)
	{
		ret = _funcIsSupported(NULL, (DWORD)fimg);
		
		return ret ? true : false;
	}
	return false;
}

// GetPictureInfo系
bool CSPI::getPictureInfo(LPCSTR filename, PictureInfo *info)
{
	int	ret;
	
	if(_funcGetPictureInfo)
	{
		ret = _funcGetPictureInfo((LPSTR)filename, 0, 0, info);
		return ret == 0 ? true : false;
	}
	return false;
}
bool CSPI::getPictureInfoMem(const void *fimg, unsigned long size, PictureInfo *info)
{
	int	ret;
	
	if(_funcGetPictureInfo)
	{
		ret = _funcGetPictureInfo((CHAR*)fimg, size, 1, info);
		return ret == 0 ? true : false;
	}
	return false;
}
// GetPicture系
bool CSPI::getPicture(LPCSTR filename, HLOCAL *hbminfo, HLOCAL *hbm)
{
	int	ret;
	HLOCAL	hbi = NULL, hb = NULL;
	
	
	if(_funcGetPicture)
	{
		ret = _funcGetPicture((LPSTR)filename, 0, 0, &hbi, &hb, NULL, 0);
		if(ret != 0 || hbminfo == NULL || hbm == NULL)
		{
			if(hbi) LocalFree(hbi);
			if(hb) LocalFree(hb);
			return false;
		}
		
		*hbminfo = hbi;
		*hbm = hb;
		return true;
	}
	return false;
}
bool CSPI::getPictureMem(const void *fimg, unsigned long size, HLOCAL *hbminfo, HLOCAL *hbm)
{
	int	ret;
	HLOCAL	hbi = NULL, hb = NULL;
	
	
	if(_funcGetPicture)
	{
		ret = _funcGetPicture((CHAR*)fimg, size, 1, &hbi, &hb, NULL, 0);
		if(ret != 0 || hbminfo == NULL || hbm == NULL)
		{
			if(hbi) LocalFree(hbi);
			if(hb) LocalFree(hb);
			return false;
		}
		
		*hbminfo = hbi;
		*hbm = hb;
		return true;
	}
	return false;
}

/*
void CSPI::test()
{
	int	ret;
	char	buf[256+1];
	
	if(_funcGetPluginInfo)
	{
		int	no = 0;
		while((ret = _funcGetPluginInfo(no, buf, 256)) != 0)
		{
			printf("%d ret=%d %s\n", no, ret, buf);
			no++;
		}
	}
	else
	{
		printf("_funcGetPluginInfo=%p\n", _funcGetPluginInfo);
	}
}

*/

//-----------------------------------------------------------------------------
// Pythonインターフェース実装
// cfilerのソースとPythonヘルプ見ながら見よう見まね

static PyObject * SPI_fromPath( PyObject * self, PyObject * args )
{
	FUNC_TRACE;
	
	PyObject * pyfilename;
	if( ! PyArg_ParseTuple(args, "O", &pyfilename ) )
        return NULL;

    std::wstring filename;
    if( !PythonUtil::PyStringToWideString( pyfilename, &filename ) )
    {
    	return NULL;
    }

	CSPI	*spi = new CSPI;
	int	ret = spi->loadDll(filename.c_str());
	if(ret == 0)
	{
		delete spi;
		
		std::string msg;
		if( !PythonUtil::PyStringToString( pyfilename, &msg ) )
		{
			return NULL;
		}
		msg += ": spi load error.";
		PyErr_SetString( PyExc_ValueError, msg.c_str() );
		return NULL;
	}
	
	SPI_Object * pyspi;
	pyspi = PyObject_New( SPI_Object, &SPI_Type );
	pyspi->p = spi;
	spi->AddRef();

	return (PyObject*)pyspi;
}
static PyObject * SPI_getExtList( PyObject * self, PyObject * args )
{
	FUNC_TRACE;
	
	if( ! PyArg_ParseTuple(args, "" ) )
		return NULL;
	
	CSPI *spi = ((SPI_Object*)self)->p;
	if( ! spi )
	{
		PyErr_SetString( PyExc_ValueError, "already destroyed." );
		return NULL;
	}
	
	std::string	ext_list, ext;
	const char	*delim = ";";
	int	no = 0;
	while(spi->getPluginInfoExt(no, &ext))
	{
		if(ext_list.length()) ext_list += delim;
		ext_list += ext;
		no++;
	}
	
	// あえて文字列で返す。文字列のほうがマッチング楽なこともあるかも…。
	PyObject * pyret = Py_BuildValue( "s", ext_list.c_str() );
	return pyret;
}
void __log(const char *fmt, ...)
{
	FILE	*fp = fopen("log.txt", "wt+");
	
	va_list	args;
	va_start(args, fmt);
	vfprintf(fp, fmt, args);
	va_end(args);
	fclose(fp);
}

// Imageオブジェクトを作るのが困難なので、サイズ返せるようにする
// ((w,h),depth) = getSize(filename)
static PyObject * SPI_getSizeDepth( PyObject * self, PyObject * args )
{
	PyObject *pyfilename;
	if( ! PyArg_ParseTuple(args, "O", &pyfilename ) )
		return NULL;
	
	CSPI	*spi = ((SPI_Object*)self)->p;
	
	std::string filename;
	if( !PythonUtil::PyStringToString( pyfilename, &filename ) )
	{
		return NULL;
	}
	
	if(!spi->isSupported(filename.c_str()))
	{
		PyErr_SetString( PyExc_ValueError, "not supported image." );
		return NULL;
	}
	
	PictureInfo	pi;
	pi.hInfo = NULL;
	if(!spi->getPictureInfo(filename.c_str(), &pi))
	{
		PyErr_SetString( PyExc_ValueError, "can't get pictureinfo." );
		return NULL;
	}
	// 一応解放
	if(pi.hInfo) LocalFree(pi.hInfo);
	// タプルを返す
	return Py_BuildValue( "(ii)i", pi.width, pi.height, pi.colorDepth );
}
// ((w,h),depth) = getSize(fileimage)
static PyObject * SPI_getSizeDepthMem( PyObject * self, PyObject * args )
{
	const char	*fimg;
	unsigned int	fimg_size;
	if( ! PyArg_ParseTuple(args, "s#", &fimg, &fimg_size ) )
		return NULL;
	
	CSPI	*spi = ((SPI_Object*)self)->p;
	
	if(!spi->isSupportedMem(fimg))
	{
		PyErr_SetString( PyExc_ValueError, "not supported image." );
		return NULL;
	}
	
	PictureInfo	pi;
	pi.hInfo = NULL;
	if(!spi->getPictureInfoMem(fimg, fimg_size, &pi))
	{
		PyErr_SetString( PyExc_ValueError, "can't get pictureinfo." );
		return NULL;
	}
	// 一応解放
	if(pi.hInfo) LocalFree(pi.hInfo);
	// タプルを返す
	return Py_BuildValue( "(ii)i", pi.width, pi.height, pi.colorDepth );
}

static void *_dib2rgba(BITMAPINFO *info, void *src_, unsigned long *pdst_size)
{
	int	w = info->bmiHeader.biWidth, h = info->bmiHeader.biHeight;
	int	dst_size = 4 * w * h;
	if(pdst_size) *pdst_size = dst_size;
	unsigned char	*dst = (unsigned char*)malloc(dst_size);
	
// _TRACE("biSize          =%d\n", info->bmiHeader.biSize          );
// _TRACE("biWidth         =%d\n", info->bmiHeader.biWidth         );
// _TRACE("biHeight        =%d\n", info->bmiHeader.biHeight        );
// _TRACE("biPlanes        =%d\n", info->bmiHeader.biPlanes        );
// _TRACE("biBitCount      =%d\n", info->bmiHeader.biBitCount      );
// _TRACE("biCompression   =%d\n", info->bmiHeader.biCompression   );
// _TRACE("biSizeImage     =%d\n", info->bmiHeader.biSizeImage     );
// _TRACE("biXPelsPerMeter =%d\n", info->bmiHeader.biXPelsPerMeter );
// _TRACE("biYPelsPerMeter =%d\n", info->bmiHeader.biYPelsPerMeter );
// _TRACE("biClrUsed       =%d\n", info->bmiHeader.biClrUsed       );
// _TRACE("biClrImportant  =%d\n", info->bmiHeader.biClrImportant  );
	
	// BMPのストライドは4バイト境界である必要があった気がする
	typedef unsigned long u32;
	typedef unsigned char u8;
	u8	*src = (u8*)src_;
	
	// ビット数変換		まじめに作るとキリないので超適当
	switch(info->bmiHeader.biBitCount)
	{
	case 32:
		{
			u32	*dst_u32 = (u32*)dst;
			for( u32*src_u32 = (u32*)src, *src_end = src_u32 + (w * h);
					src_u32 != src_end; src_u32++ )	// 4バイト境界が保証される
			{
				// BGRA => RGBA
				u32	c = *src_u32;
				*dst_u32 = 
					((c & 0x000000FF) << 16) |	// B
					 (c & 0x0000FF00)        |	// G
					((c & 0x00FF0000) >> 16) |	// R
					 (c & 0xFF000000);
				dst_u32++;
				src_u32++;
			}
		}
		break;
	case 24:
		// src=24bit
		{
			int	wb = w * 3;
			wb = (wb + 3) & (~3);	// 4バイト境界
			u32	*dst_u32 = (u32*)dst;
			for( int yy = h-1;yy>=0;yy-- )
			{
				u8	*src_u8 = src + (yy*wb);
				for( int x=0 ; x<w ; x++ )
				{
					// BGRx => RGBA
					u32	c = *((u32*)src_u8);	// x86はアドレスが境界でなくても大丈夫だったはず
					*dst_u32 = 
						((c & 0x000000FF) << 16) |	// B
						 (c & 0x0000FF00)        |	// G
						((c & 0x00FF0000) >> 16) |	// R
						0xFF000000;
					dst_u32++;
					src_u8 += 3;
				}
			}
		}
		break;
	case 8:
		{
			// 先にパレットの並びを置換
			int	plt_num = info->bmiHeader.biClrUsed;
			if(plt_num == 0) plt_num = 256;	// 0なことがある…
			u32	*plt =(u32*)malloc(sizeof(u32)*plt_num);
			{
				u32	*src_plt = (u32*)info->bmiColors;
				for(u32	*p = plt, *p_end = plt + plt_num; p != p_end; p++, src_plt++)
				{
					u32	c = *src_plt;
					*p = ((c & 0x000000FF) << 16) |	// B
						   c & 0x0000FF00         |	// G
						 ((c & 0x00FF0000) >> 16) |	// R
						 0xFF000000;
				}
			}
			
			int	wb = w;
			wb = (w + 3) & (~3);	// 4バイト境界
			u32	*dst_u32 = (u32*)dst;
			for( int yy = h-1;yy>=0;yy-- )
			{
				u8	*src_u8 = src + (yy*wb);
				for( int x=0 ; x<w ; x++ )
				{
					*dst_u32 = plt[*src_u8];
					dst_u32++;
					src_u8++;
				}
			}
			free(plt);
		}
		break;
	case 4:
		{
			// 先にパレットの並びを置換
			int	plt_num = info->bmiHeader.biClrUsed;
			if(plt_num == 0) plt_num = 16;	// 0なことがある…
			u32	*plt =(u32*)malloc(sizeof(u32)*plt_num);
			{
				u32	*src_plt = (u32*)info->bmiColors;
				for(u32	*p = plt, *p_end = plt + plt_num; p != p_end; p++, src_plt++)
				{
					u32	c = *src_plt;
					*p = ((c & 0x000000FF) << 16) |	// B
						   c & 0x0000FF00         |	// G
						 ((c & 0x00FF0000) >> 16) |	// R
						 0xFF000000;
				}
			}
			
			int	aw = (w+1) & (~1);	// タプル数をバイト境界にあわせるためアライン2
			int	wh = aw / 2;		// バイト数算出
			int	wb = wh;			// ストライド
			wb = (wb + 3) & (~3);	// 4バイト境界
//			__log("w=%d aw=%d wh=%d wb=%d\n", w, aw, wh, wb);
			u32	*dst_u32 = (u32*)dst;
			for( int yy = h-1;yy>=0;yy-- )
			{
				u8	*src_u8 = src + (yy*wb);
				for( int x=0 ; x<wh ; x++ )
				{
					u8	c = *src_u8++;
					*dst_u32 = plt[c >> 4];
					dst_u32++;
					*dst_u32 = plt[c & 0x0F];
					dst_u32++;
				}
			}
		}
		break;
// ToDo: 1bit, 2bit
	}
	
	return dst;
}
// string = loadImage(filename)
static PyObject * SPI_loadImage( PyObject * self, PyObject * args )
{
	PyObject *pyfilename;
	if( ! PyArg_ParseTuple(args, "O", &pyfilename ) )
		return NULL;
	
	CSPI	*spi = ((SPI_Object*)self)->p;
	
	std::string filename;
	if( !PythonUtil::PyStringToString( pyfilename, &filename ) )
	{
		return NULL;
	}
	
	if(!spi->isSupported(filename.c_str()))
	{
		PyErr_SetString( PyExc_ValueError, "not supported image." );
		return NULL;
	}
	
	HLOCAL	hbminfo = NULL, hbm = NULL;
	if(!spi->getPicture(filename.c_str(), &hbminfo, &hbm))
	{
		if(hbminfo) LocalFree(hbminfo);
		if(hbm)     LocalFree(hbm);
		PyErr_SetString( PyExc_ValueError, "load error." );
		return NULL;
	}
	
	BITMAPINFO	*info = (BITMAPINFO*)LocalLock(hbminfo);
	unsigned char		*src = (unsigned char*)LocalLock(hbm);
	
	_TRACE("bitcount=%d\n", info->bmiHeader.biBitCount);
	
	unsigned long	dst_size;
	void *dst = _dib2rgba(info, src, &dst_size);
	
	PyObject *ret = Py_BuildValue("s#", dst, dst_size);
	
	free(dst);
	
	if(hbminfo) LocalFree(hbminfo);
	if(hbm)     LocalFree(hbm);
	
	return ret;
}
// string = loadImageFromStr(fileimage)
static PyObject * SPI_loadImageMem( PyObject * self, PyObject * args )
{
	const char	*fimg;
	unsigned int	fimg_size;
	if( ! PyArg_ParseTuple(args, "s#", &fimg, &fimg_size ) )
		return NULL;
	
	CSPI	*spi = ((SPI_Object*)self)->p;
	
	if(!spi->isSupportedMem(fimg))
	{
		PyErr_SetString( PyExc_ValueError, "not supported image." );
		return NULL;
	}
	
	HLOCAL	hbminfo = NULL, hbm = NULL;
	if(!spi->getPictureMem(fimg, fimg_size, &hbminfo, &hbm))
	{
		if(hbminfo) LocalFree(hbminfo);
		if(hbm)     LocalFree(hbm);
		PyErr_SetString( PyExc_ValueError, "load error." );
		return NULL;
	}
	
	BITMAPINFO	*info = (BITMAPINFO*)LocalLock(hbminfo);
	unsigned char		*src = (unsigned char*)LocalLock(hbm);
	
	_TRACE("bitcount=%d\n", info->bmiHeader.biBitCount);
	
	unsigned long	dst_size;
	void *dst = _dib2rgba(info, src, &dst_size);
	
	PyObject *ret = Py_BuildValue("s#", dst, dst_size);
	
	free(dst);
	
	if(hbminfo) LocalFree(hbminfo);
	if(hbm)     LocalFree(hbm);
	
	return ret;
}

static void SPI_dealloc(PyObject* self)
{
	FUNC_TRACE;

    CSPI * spi = ((SPI_Object*)self)->p;
    spi->Release();
	self->ob_type->tp_free(self);
}

static PyMethodDef SPI_methods[] = {
	{ "getSizeDepth",	SPI_getSizeDepth,	METH_VARARGS, "" },
	{ "loadImage",		SPI_loadImage,		METH_VARARGS, "" },
	{ "getSizeDepthMem",SPI_getSizeDepthMem,METH_VARARGS, "" },
	{ "loadImageMem",	SPI_loadImageMem,	METH_VARARGS, "" },
	{ "getExtList",		SPI_getExtList,		METH_VARARGS, "" },
	{ "fromPath",		SPI_fromPath,		METH_STATIC|METH_VARARGS, "" },
	{NULL,NULL}
};

PyTypeObject SPI_Type = {
	PyObject_HEAD_INIT(NULL)
	0,
	"SPI",			/* tp_name */
	sizeof(SPI_Object), /* tp_basicsize */
	0,					/* tp_itemsize */
	SPI_dealloc,		/* tp_dealloc */
	0,					/* tp_print */
	0,					/* tp_getattr */
	0,					/* tp_setattr */
	0,					/* tp_compare */
	0, 					/* tp_repr */
	0,					/* tp_as_number */
	0,					/* tp_as_sequence */
	0,					/* tp_as_mapping */
	0,					/* tp_hash */
	0,					/* tp_call */
	0,					/* tp_str */
	PyObject_GenericGetAttr,/* tp_getattro */
	PyObject_GenericSetAttr,/* tp_setattro */
	0,					/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,/* tp_flags */
	"",					/* tp_doc */
	0,					/* tp_traverse */
	0,					/* tp_clear */
	0,					/* tp_richcompare */
	0,					/* tp_weaklistoffset */
	0,					/* tp_iter */
	0,					/* tp_iternext */
	SPI_methods,		/* tp_methods */
	0,					/* tp_members */
	0,					/* tp_getset */
	0,					/* tp_base */
	0,					/* tp_dict */
	0,					/* tp_descr_get */
	0,					/* tp_descr_set */
	0,					/* tp_dictoffset */
	0,					/* tp_init */
	0,					/* tp_alloc */
	PyType_GenericNew,	/* tp_new */
	0,					/* tp_free */
};

#define MODULE_NAME "spi"

static PyMethodDef cterm_funcs[] =
{
    {NULL, NULL, 0, NULL}
};

static PyObject * Error;

extern "C" void __stdcall initspi(void)
{
    if( PyType_Ready(&SPI_Type)<0 ) return;

    PyObject *m, *d;

    m = Py_InitModule3( MODULE_NAME, cterm_funcs, "spi module." );

    Py_INCREF(&SPI_Type);
    PyModule_AddObject( m, "SPI", (PyObject*)&SPI_Type );

    d = PyModule_GetDict(m);

    Error = PyErr_NewException( MODULE_NAME".Error", NULL, NULL);
    PyDict_SetItemString( d, "Error", Error );

    if( PyErr_Occurred() )
    {
        Py_FatalError( "can't initialize module "MODULE_NAME );
    }
}

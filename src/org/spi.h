#ifndef __SPI_H__
#define __SPI_H__

#include "windows.h"
//#include <vector>
#include <string>

//---- from pythonutil.hから流用
#include "python.h"

namespace PythonUtil
{
    bool PyStringToString( const PyObject * pystr, std::string * str );
	bool PyStringToWideString( const PyObject * pystr, std::wstring * str );
};
//---- from pythonutil.hから流用


// SPI画像情報
struct PictureInfo
{
	long left,top;		// 画像を展開する位置
	long width;			// 画像の幅(pixel)
	long height;		// 画像の高さ(pixel)
	WORD x_density;		// 画素の水平方向密度
	WORD y_density;		// 画素の垂直方向密度
	short colorDepth;	// １画素当たりのbit数
	HLOCAL hInfo;		// 画像内のテキスト情報
};

// SPI操作クラス
// 1インスタンスが1つのSPIに対応する
class CSPI
{
public:
	CSPI();
	~CSPI();
	
	int loadDll(const WCHAR *dll_fname);
	
	void term();
	
	void AddRef() { ref_count++; /* printf("Image::AddRef : %d\n", ref_count ); */ }
	void Release() { ref_count--; /* printf("Image::Release : %d\n", ref_count ); */ if(ref_count==0) delete this; }
	
	// GetPluginInfo系
	bool getPluginInfoAPIVer(std::string *s);
	bool getPluginInfoAbout (std::string *s);
	bool getPluginInfoExt   (int no, std::string *s);	// no=0～
	
	// IsSupported系
	bool isSupported(LPCSTR filename);
	bool isSupportedMem(const void *fimg);
	
	// GetPictureInfo系
	bool getPictureInfo(LPCSTR filename, PictureInfo *info);
	bool getPictureInfoMem(const void *fimg, unsigned long size, PictureInfo *info);
	
	// GetPicture系
	bool getPicture(LPCSTR filename, HLOCAL *hbminfo, HLOCAL *hbm);
	bool getPictureMem(const void *fimg, unsigned long size, HLOCAL *hbminfo, HLOCAL *hbm);
	
private:
	int	ref_count;
	
	HMODULE	_hDll;
	
	// ToDo: 00AMプラグイン対応 内骨格向けだと要らないかな…
/*
  ・GetPluginInfo - Plug-inに関する情報を得る

    Prototype:
      extern "C" int _export PASCAL GetPluginInfo(int infono,
                                                LPSTR buf,int buflen);
    Parameter:
      int infono : 取得する情報番号
                      0   : Plug-in APIバージョン
                      1   : Plug-in名、バージョン及び copyright
                            (SusieのAbout..に表示されます)
                      2n+2: 代表的な拡張子 ("*.JPG" "*.RGB;*.Q0" など)
                      2n+3: ファイル形式名
                            (ファイルタイプ名として使われます)
      LPSTR buf    : 情報を納めるバッファ
      int buflen : バッファ長(byte)

    Return:
      バッファに書き込んだ文字数を返します。
      情報番号が無効の場合、0を返します。

    解説:
      情報番号0と1はすべてのバージョンで共通とします。
      2以降は二つづつ組みでSusieのOPENダイアログで用いる情報です。
      一つのplug-inで複数の画像フォーマットに対応している場合は
      その数だけ拡張子とファイル形式名を用意します。
*/
	typedef int (PASCAL *GET_PLUGIN_INFO)(int , LPSTR , int );
	GET_PLUGIN_INFO	_funcGetPluginInfo;
	
/*
  ・IsSupported - 展開可能な(対応している)ファイル形式か調べる。
    Prototype:
      extern "C" int _export PASCAL IsSupported(LPSTR filename,DWORD dw);

    Parameter:
      LPSTR filename : ファイルネーム
      DWORD dw       : 上位ワードが  0  のとき:
                           ファイルハンドル
                       上位ワードが 非0 のとき:
                           ファイル先頭部(2Kbyte以上)を読み込んだバッファへの
                           ポインタ
                           ファイルサイズが2Kbyte以下の場合もバッファは2Kbyte
                           確保し、余分は 0 で埋めること

    Return:
      対応している画像フォーマットであれば非0を返す

    解説:
      各Plug-inは基本的に渡されたファイルのヘッダを調べ、自分の対応したファイル
      フォーマットであるかどうかを調べる。
      まれにファイル名(拡張子)を判断材料として必要としたり、複数のファイルで
      構成されている場合があるので、ファイル名(フルパス)も引数に加えた。
      今回配布のPlug-inではfilenameは使われていない。
*/
	typedef int (PASCAL *IS_SUPPORTED)(LPSTR ,DWORD );
	IS_SUPPORTED	_funcIsSupported;
	
/*
  ・GetPictureInfo - 画像ファイルに関する情報を得る
    Prototype:
      extern "C" int _export PASCAL GetPictureInfo(
         LPSTR buf,long len,unsigned int flag,struct PictureInfo *lpInfo);

    Parameter:
      LPSTR buf : 入力がファイルの場合 ファイル名
                        メモリーの場合 ファイルイメージへのポインタ
      long len  : 入力がファイルの場合 読込み開始オフセット(MacBin対応のため)
                        メモリーの場合 データサイズ
      unsigned int flag : 追加情報 xxxx xxxx xxxx xSSS
                  SSS : 入力形式
                      000 : ディスクファイル
                      001 : メモリ上のイメージ
      struct PictureInfo *lpInfo :
                  struct PictureInfo
                  {
                    long left,top;    画像を展開する位置
                    long width;       画像の幅(pixel)
                    long height;      画像の高さ(pixel)
                    WORD x_density;   画素の水平方向密度
                    WORD y_density;   画素の垂直方向密度
                    short colorDepth; １画素当たりのbit数
                    HLOCAL hInfo;    画像内のテキスト情報
                  };
                  hInfoには必要に応じてPlug-inが確保したGlobalメモリーの
                  ハンドルが格納される。

    Return:
      エラーコード。0なら正常終了。
*/
	typedef int (PASCAL *GET_PICTURE_INFO)(LPSTR ,long ,unsigned int ,PictureInfo *lpInfo);
	GET_PICTURE_INFO	_funcGetPictureInfo;
	
/*
  ・GetPicture - 画像を展開する
    Prototype:
      extern "C" int _export PASCAL GetPicture(
              LPSTR buf,long len,unsigned int flag,
              HANDLE *pHBInfo,HANDLE *pHBm,
              FARPROC lpPrgressCallback,long lData);

    Parameter:
      LPSTR buf : 入力がファイルの場合 ファイル名
                        メモリーの場合 ファイルイメージへのポインタ
      long len  : 入力がファイルの場合 読込み開始オフセット(MacBin対応のため)
                        メモリーの場合 データサイズ
      unsigned int flag : 追加情報 xxxx xxxx xxxx xSSS
                  SSS : 入力形式
                      000 : ディスクファイル
                      001 : メモリ上のイメージ
      HLOCAL *pHBInfo : BITMAPINFO 構造体が納められたメモリハンドルが
                             返される。
      HLOCAL *pHBm    : ビットマップデータ本体のメモリハンドルが返される
      FARPROC lpPrgressCallback :
                途中経過を表示するコールバック関数へのポインタ。
                NULLの場合、plug-inは処理が終了するまでプロセスを占有し、
                中断も出来ません。
                コールバック関数のprototype:
                  int PASCAL ProgressCallback(
                                        int nNum,int nDenom,long lData);
                  まず nNum==0 でコールされ、nNum==nDenom になるまで
                  定期的に呼ばれる。
                  戻値が 非0 の時、Plug-inは処理を中断する。
      long lData : コールバック関数に渡すlongデータ。
                  ポインタ等を必要に応じて受け渡せる。

    Return:
      エラーコード。0なら正常終了。

    解説:
      プラグインはLocalAllocによって必要なメモリーを確保し、そのハンドルを
      返す。
      アプリケーションはLocalFreeによってメモリーを開放する必要がある。
*/
	typedef int (PASCAL *GET_PICTURE)(LPSTR ,long ,unsigned int ,HANDLE *,HANDLE *,FARPROC ,long );
	GET_PICTURE	_funcGetPicture;
	
/*
  ・GetPreview - プレビュー・カタログ表示用画像縮小展開ルーティン
    Prototype:
      extern "C" int _export PASCAL GetPreview(
              LPSTR buf,long len,unsigned int flag,
              HANDLE *pHBInfo,HANDLE *pHBm,
              FARPROC lpPrgressCallback,long lData);

    Parameter:
      GETPICTURE参照。

    Return:
      エラーコード。0なら正常終了。
      この関数はオプションであり、未対応の場合は -1 を返す。

    解説:
      プレビュー等で用いる縮小された画像をファイルから作成する。
      JPEGの様に、アルゴリズムの関係で縮小されたサイズでは高速に展開出来る
      ときにこの関数をインプリメントする。
      今回配布のPlug-inでは IFJPEG.PLG のみ対応(1/4サイズで展開)している。
      未対応の場合、Susieは通常の展開ルーティンを用いて展開した後
      縮小処理を行う。
      （対応していても縮小ロードされた画像を更にサイズ調整している）
      プラグインはLocalAllocによって必要なメモリーを確保し、そのハンドルを
      返す。
      アプリケーションはLocalFreeによってメモリーを開放する必要がある。

  ・エラーコード
    0 : 正常終了
   -1 : その機能はインプリメントされていない
    1 : コールバック関数が非0を返したので展開を中止した
    2 : 未知のフォーマット
    3 : データが壊れている
    4 : メモリーが確保出来ない
    5 : メモリーエラー（Lock出来ない、等）
    6 : ファイルリードエラー
    7 : （予約）
    8 : 内部エラー
*/
	// 省略
};

// Pythonインターフェース
extern PyTypeObject SPI_Type;
#define SPI_Check(op) PyObject_TypeCheck(op, &SPI_Type)

struct SPI_Object
{
    PyObject_HEAD
    CSPI * p;
};




#endif	// __SPI_H__

/*
Susie 32bit Plug-in 仕様 rev5

１．はじめに
  Susie 32bit Plug-in は Windows の DLL であり、後述の API により Susie 以外の
  ソフトウェアからも簡単に使う事が出来ます。
  また、この仕様通りにPlug-inを作れば Susie を新しい画像フォーマットに
  対応させる事が可能です。

２．Plug-in APIのバージョン
  今後の拡張性を持たせるため、Plug-inにAPIのバージョン番号がつきます。
  このバージョン番号はすべてのバージョンに共通である関数'GETPLUGININFO'によって
  取得出来ます。
  バージョン番号は基本的に4byteのコードで以下の意味を持ちます。

      00 I N
      ~T T T
       | | +-- N : Normal,  M : Multi-picture
       | +---- I : Import filter, X : Export filter, A : Archive extractor
       +------ Virsion No.

３．共通関数
  ・GetPluginInfo - Plug-inに関する情報を得る

    Prototype:
      extern "C" int _export PASCAL GetPluginInfo(int infono,
                                                LPSTR buf,int buflen);
    Parameter:
      int infono : 取得する情報番号
                      0   : Plug-in APIバージョン
                      1   : Plug-in名、バージョン及び copyright
                            (SusieのAbout..に表示されます)
                      2n+2: 代表的な拡張子 ("*.JPG" "*.RGB;*.Q0" など)
                      2n+3: ファイル形式名
                            (ファイルタイプ名として使われます)
      LPSTR buf    : 情報を納めるバッファ
      int buflen : バッファ長(byte)

    Return:
      バッファに書き込んだ文字数を返します。
      情報番号が無効の場合、0を返します。

    解説:
      情報番号0と1はすべてのバージョンで共通とします。
      2以降は二つづつ組みでSusieのOPENダイアログで用いる情報です。
      一つのplug-inで複数の画像フォーマットに対応している場合は
      その数だけ拡張子とファイル形式名を用意します。

４．'00IN'の関数
  ・IsSupported - 展開可能な(対応している)ファイル形式か調べる。
    Prototype:
      extern "C" int _export PASCAL IsSupported(LPSTR filename,DWORD dw);

    Parameter:
      LPSTR filename : ファイルネーム
      DWORD dw       : 上位ワードが  0  のとき:
                           ファイルハンドル
                       上位ワードが 非0 のとき:
                           ファイル先頭部(2Kbyte以上)を読み込んだバッファへの
                           ポインタ
                           ファイルサイズが2Kbyte以下の場合もバッファは2Kbyte
                           確保し、余分は 0 で埋めること

    Return:
      対応している画像フォーマットであれば非0を返す

    解説:
      各Plug-inは基本的に渡されたファイルのヘッダを調べ、自分の対応したファイル
      フォーマットであるかどうかを調べる。
      まれにファイル名(拡張子)を判断材料として必要としたり、複数のファイルで
      構成されている場合があるので、ファイル名(フルパス)も引数に加えた。
      今回配布のPlug-inではfilenameは使われていない。


  ・GetPictureInfo - 画像ファイルに関する情報を得る
    Prototype:
      extern "C" int _export PASCAL GetPictureInfo(
         LPSTR buf,long len,unsigned int flag,struct PictureInfo *lpInfo);

    Parameter:
      LPSTR buf : 入力がファイルの場合 ファイル名
                        メモリーの場合 ファイルイメージへのポインタ
      long len  : 入力がファイルの場合 読込み開始オフセット(MacBin対応のため)
                        メモリーの場合 データサイズ
      unsigned int flag : 追加情報 xxxx xxxx xxxx xSSS
                  SSS : 入力形式
                      000 : ディスクファイル
                      001 : メモリ上のイメージ
      struct PictureInfo *lpInfo :
                  struct PictureInfo
                  {
                    long left,top;    画像を展開する位置
                    long width;       画像の幅(pixel)
                    long height;      画像の高さ(pixel)
                    WORD x_density;   画素の水平方向密度
                    WORD y_density;   画素の垂直方向密度
                    short colorDepth; １画素当たりのbit数
                    HLOCAL hInfo;    画像内のテキスト情報
                  };
                  hInfoには必要に応じてPlug-inが確保したGlobalメモリーの
                  ハンドルが格納される。

    Return:
      エラーコード。0なら正常終了。

  ・GetPicture - 画像を展開する
    Prototype:
      extern "C" int _export PASCAL GetPicture(
              LPSTR buf,long len,unsigned int flag,
              HANDLE *pHBInfo,HANDLE *pHBm,
              FARPROC lpPrgressCallback,long lData);

    Parameter:
      LPSTR buf : 入力がファイルの場合 ファイル名
                        メモリーの場合 ファイルイメージへのポインタ
      long len  : 入力がファイルの場合 読込み開始オフセット(MacBin対応のため)
                        メモリーの場合 データサイズ
      unsigned int flag : 追加情報 xxxx xxxx xxxx xSSS
                  SSS : 入力形式
                      000 : ディスクファイル
                      001 : メモリ上のイメージ
      HLOCAL *pHBInfo : BITMAPINFO 構造体が納められたメモリハンドルが
                             返される。
      HLOCAL *pHBm    : ビットマップデータ本体のメモリハンドルが返される
      FARPROC lpPrgressCallback :
                途中経過を表示するコールバック関数へのポインタ。
                NULLの場合、plug-inは処理が終了するまでプロセスを占有し、
                中断も出来ません。
                コールバック関数のprototype:
                  int PASCAL ProgressCallback(
                                        int nNum,int nDenom,long lData);
                  まず nNum==0 でコールされ、nNum==nDenom になるまで
                  定期的に呼ばれる。
                  戻値が 非0 の時、Plug-inは処理を中断する。
      long lData : コールバック関数に渡すlongデータ。
                  ポインタ等を必要に応じて受け渡せる。

    Return:
      エラーコード。0なら正常終了。

    解説:
      プラグインはLocalAllocによって必要なメモリーを確保し、そのハンドルを
      返す。
      アプリケーションはLocalFreeによってメモリーを開放する必要がある。

  ・GetPreview - プレビュー・カタログ表示用画像縮小展開ルーティン
    Prototype:
      extern "C" int _export PASCAL GetPreview(
              LPSTR buf,long len,unsigned int flag,
              HANDLE *pHBInfo,HANDLE *pHBm,
              FARPROC lpPrgressCallback,long lData);

    Parameter:
      GETPICTURE参照。

    Return:
      エラーコード。0なら正常終了。
      この関数はオプションであり、未対応の場合は -1 を返す。

    解説:
      プレビュー等で用いる縮小された画像をファイルから作成する。
      JPEGの様に、アルゴリズムの関係で縮小されたサイズでは高速に展開出来る
      ときにこの関数をインプリメントする。
      今回配布のPlug-inでは IFJPEG.PLG のみ対応(1/4サイズで展開)している。
      未対応の場合、Susieは通常の展開ルーティンを用いて展開した後
      縮小処理を行う。
      （対応していても縮小ロードされた画像を更にサイズ調整している）
      プラグインはLocalAllocによって必要なメモリーを確保し、そのハンドルを
      返す。
      アプリケーションはLocalFreeによってメモリーを開放する必要がある。

  ・エラーコード
    0 : 正常終了
   -1 : その機能はインプリメントされていない
    1 : コールバック関数が非0を返したので展開を中止した
    2 : 未知のフォーマット
    3 : データが壊れている
    4 : メモリーが確保出来ない
    5 : メモリーエラー（Lock出来ない、等）
    6 : ファイルリードエラー
    7 : （予約）
    8 : 内部エラー

５．'00AM'の関数 (暫定)

  ・IsSupported - 展開可能な(対応している)ファイル形式か調べる。
    Prototype:
      extern "C" int _export PASCAL IsSupported(LPSTR filename,DWORD dw);

    Parameter:
      LPSTR filename : ファイルネーム
      DWORD dw       : 上位ワードが  0  のとき:
                           ファイルハンドル
                       上位ワードが 非0 のとき:
                           ファイル先頭部(2Kbyte以上)を読み込んだバッファへの
                           ポインタ
                           ファイルサイズが2Kbyte以下の場合もバッファは2Kbyte
                           確保し、余分は 0 で埋めること

    Return:
      対応している画像フォーマットであれば非0を返す

    解説:
      詳しくは'00IN'のISSUPPORTED関数を参照の事。
      引数dwで渡すバッファサイズ2Kbyte以上は自己解凍型LHa対応のため。

  ・GetArchiveInfo - アーカイブ内のすべてのファイルの情報を取得する
    Prototype:
      extern "C" errcode _export PASCAL GetArchiveInfo(LPSTR buf,long len,
            unsigned int flag,HLOCAL *lphInf);

    Parameter:
      LPSTR buf : 入力がファイルの場合 ファイル名
                        メモリーの場合 ファイルイメージへのポインタ
      long len  : 入力がファイルの場合 読込み開始オフセット(MacBin対応のため)
                        メモリーの場合 データサイズ
      unsigned int flag : 追加情報 xxxx xxxx xxxx xSSS
                  SSS : 入力形式
                      000 : ディスクファイル
                      001 : メモリ上のイメージ
      HLOCAL *lphInf
                : ファイル情報の入ったハンドルを受け取る変数へのポインタ。
                  Plug-in内で確保されたLOCALメモリーに次の構造体配列が
                  書き込まれ、そのハンドルが返される。
                  method[0]=='\0'で終端。
                        typedef struct
                        {
                            unsigned char method[8];    圧縮法の種類
                            unsigned long position;     ファイル上での位置
                            unsigned long compsize;     圧縮されたサイズ
                            unsigned long filesize;     元のファイルサイズ
                            time_t timestamp;           ファイルの更新日時
                            char path[200];             相対パス
                            char filename[200];         ファイルネーム
                            unsigned long crc;         CRC
                        } fileInfo;

    Return:
      エラーコード。0なら正常終了。

  ・GetFileInfo - アーカイブ内の指定したファイルの情報を取得する
    Prototype:
      extern "C" errcode _export PASCAL GetFileInfo(LPSTR buf,long len,
                    LPSTR filename, unsigned int flag,fileInfo *lpInfo);
    Parameter:
      LPSTR buf : 入力がファイルの場合 ファイル名
                        メモリーの場合 ファイルイメージへのポインタ
      long len  : 入力がファイルの場合 読込み開始オフセット(MacBin対応のため)
                        メモリーの場合 データサイズ
      LPSTR filename : 情報を取得するファイルのファイルネーム
                        アーカイブ内の相対パスを含めて指定
      unsigned int flag : 追加情報 xxxx xxxx Ixxx xSSS
                  SSS : 入力形式
                      000 : ディスクファイル
                      001 : メモリ上のイメージ
                  I : 0 : ファイル名の大文字小文字を区別する
                      1 : ファイル名の大文字小文字を同一視する。
      fileInfo *lpInfo
                : 情報を受け取るfileInfo構造体へのポインタ

    Return:
      エラーコード。0なら正常終了。

  ・GetFile - アーカイブ内のファイルを取得する
    Prototype:
      extern "C" errcode _export PASCAL GetFile(LPSTR src,long len,
                LPSTR dest,unsigned int flag,
                FARPROC prgressCallback,long lData);

    Parameter:
      LPSTR src  : 入力がファイルの場合 ファイル名
                        メモリーの場合 ファイルイメージへのポインタ
      long len   : 入力がファイルの場合 読込み開始オフセット
                        メモリーの場合 データサイズ
      void far *dest : 出力先がファイルの場合
                              出力先ディレクトリ
                               (書庫内の相対パスは無視される)
                        メモリーの場合 
                              ファイルの入ったLOCALメモリーハンドルを受け取る
                              変数へのポインタ。
      unsigned int flag  : 追加情報 xxxx xDDD xxxx xSSS
                  SSS : 入力形式
                      000 : ディスクファイル
                      001 : メモリ上のイメージ
                  DDD : 出力形式
                      000 : ディスクファイル
                      001 : メモリ上のイメージ
      FARPROC lpPrgressCallback :
                途中経過を表示するコールバック関数へのポインタ。
                NULLの場合、plug-inは処理が終了するまでプロセスを占有し、
                中断も出来ません。
                コールバック関数のprototype:
                  int PASCAL ProgressCallback(
                                        int nNum,int nDenom,long lData);
                  まず nNum==0 でコールされ、nNum==nDenom になるまで
                  定期的に呼ばれる。
                  戻値が 非0 の時、Plug-inは処理を中断する。
      long lData : コールバック関数に渡すlongデータ。
                  ポインタ等を必要に応じて受け渡せる。

    Return:
      エラーコード。0なら正常終了。

    解説:
      プラグインはLocalAllocによって必要なメモリーを確保し、そのハンドルを
      返す。
      アプリケーションはLocalFreeによってメモリーを開放する必要がある。

６．Plug-inの使い方
  Plug-inはDLLですから、通常のDLLと同じ用に次の２つの方法でアプリケーションに
  リンク出来ます。

  1) DLLからインポートライブラリを作ってリンクする
    implib.exe や implibw.exe を使ってPlug-inからインポートライブラリを
    作って、これをアプリケーションにリンクします。
    この方法は簡単ですが、特定のPlug-inしか使えません。
  2) LoadLibrary で必要に応じてリンクする。
    この方法は少々手間がかかりますが、検索して見つかったPlug-inを動的に
    用いることができます。

  通常は1)の方法が用いられますが、複数のフォーマットに対応する必要がある
  場合には2)の方法をおすすめします。
  2)の方法を用いるものとして全体の流れを説明します。

  1.Plug-inを検索する。
    Plug-inのあるディレクトリを"*.plg"で検索し、見つかったものを
    LoadLibrary でロードします。
    GetProcAddress で GETPLUGININFO 関数へのポインタを取得し、
    GETPLUGININFO 関数にて情報番号0のPlug-inバージョンを確かめます。
    対応しているバージョンならPlug-inリストに加えます。
    対応していないものならFreeLibraryで忘れずに開放します。

  2.画像ファイルに合ったPlug-inを探す。
    画像ファイルをロードする必要が生じたならまずそのファイルを_lopen等で
    オープンします。
    次に Plug-inリストにしたがって順に ISSUPPORTED 関数を呼び、対応した
    Plug-inを探します。MacBinary が付いている可能性があるので、offset=0で
    だめな場合は offset=128 でもう一度探すと良いでしょう。

  3.画像を展開する。
    対応したPlug-inが見つかったらそのPlug-inの GETPICTURE 関数でロードします。
    CALLBACK関数内で PeekMessage を使う事で他のプロセスに(そして自分にも)
    実行の機会を与えるとスマートです。

  4.Plug-inを開放する。
    アプリケーションを終了する時には忘れずに LoadLibrary したPlug-inすべてを
    FreeLibrary で開放しましょう。


７．Plug-inの仕様と使用に関して
  Plug-inを作りたい、もしくは使いたいがこれではよくわからん、という方は
  下記IDまでお問い合わせ下さい。なにかしら助言出来ると思います。（返事が
  遅くなっても怒らないでね(^_^;)）
  また、APIの仕様に関しての御意見もお待ちしております。APIバージョンアップ時
  に参考にさせていただきます。このバージョンは Susie の内部クラスのI/Fの
  ほとんどそのままなので汎用性に欠けますし(^_^;)
  転載等に関しては plugin.txt を参照して下さい。

	Nifty-serve GGB01506   竹村嘉人 (たけちん)
*/

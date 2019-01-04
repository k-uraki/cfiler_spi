内骨格でSPIを使うためのファイル

■概要
内骨格で無理やりSPI対応する為のもの。
個人的に使ってるだけのものなんで非常に出来は悪いです。


■注意
・本家のcfiler_imageviewer.pyを改変して使っている関係で、
  バージョンアップの際に使えなくなる可能性が高いです。
  ＃実際、何度か使えなくなってはソース改変しなおしてます…orz

・内骨格がバージョンアップしてPythonのバージョンが上がると
  pydファイルの再ビルドが必要になります。

・Pythonがそこそこわかる人でないと導入難しいかもしれません。
  面倒なので簡単にするつもりもないです。

・一部、本家ソースをパクっています。
  問題があればご一報下さい。


■必要ファイル
spi.pyd
  SPI対応の為のDLL
cfiler_imageviewer_mod.py
  本家cfiler_imageviewer.pyの改造品
spi_config.py
  SPI対応設定の補助ソース


■導入手順
1. 以下のファイルを cfiler/extension へコピーします。
  spi.pyd
  cfiler_imageviewer_mod.py
  spi_config.py

2. config.py へ、以下のような記述を追加します

#--------------
    import spi_config
    
    spi_list = [
        "C:/SusiePlugin/IFTIM.SPI",
        
        "C:/SusiePlugin/ifgdip.spi",
        "C:/SusiePlugin/ifDIB.spi",
        "C:/SusiePlugin/ifgif.spi",
        "C:/SusiePlugin/ifPNG.spi",
        "C:/SusiePlugin/ifpsd.spi",
        "C:/SusiePlugin/ifTGA.spi",
        "C:/SusiePlugin/IFJPEGX.SPI",
    ]
    
    spi_config.spi_config(window, spi_list, False)
#--------------

  ・spi_list には、SPIファイルのフルパスを記述します。
    同じ拡張子に対応するSPIが混在している場合、後ろに書かれているものが優先されます。
  ・spi_config.spi_config 関数の第3引数は
    「PILでの表示を優先するか」を指定します。
    例えば、JPEG表示に対応したSPIを登録する場合、
      Trueの場合は、SPIのJPEG対応は無視され、オリジナルと同じくPILによる表示を行う
      Falseの場合は、SPIのJPEG対応が優先される
    ようになります。

3. 内骨格を再起動
  ログにconfig関連でエラーが表示されていないことを確認します。

以上で、導入完了です。

画像が表示されるかテストしてみてください。


■削除方法

1. config.py へ追記した、導入手順 2. の記述を削除します。
2. 内骨格を終了します。
3. extensionへ導入したファイルを削除します。


■spi.pydのビルドについて
spi.pydを改変される方への情報です。

ビルドにはVisualStudioが必要です。

vcmake.bat の以下の部分を適宜改変してください。

・vcvarsall.bat のパス
　※使用するVisualStudioのバージョンに合わせる
・PYTHON_ROOT Pythonのインストール先
・PYTHON_LIB_FILENAME のファイル名
　※内骨格が使用しているPythonと同じバージョンとすること

改変後、vcmake.bat を実行すれば、spi.pyd が生成されます。


■再配布について
本家ソースをパクっているのでいいのか悪いのか…
そもそも、これ自体配布してよいものか、よくわからないです。

以上


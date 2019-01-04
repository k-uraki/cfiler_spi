#######################################
# 
# SPI対応 imageviewer 設定スクリプト
# 
#######################################

import os
import spi
import cfiler_imageviewer_mod

from cfiler import *

import ckit
from ckit.ckit_const import *

import string

# --------------------------------------------------------------------
# SPI登録
# window : メインウィンドウ
# spi_list : SPIのpathリスト
# keep_pil_ext : PIL表示を優先するか
def spi_config(window, spi_list, keep_pil_ext):
	
	# ビュワーで読む拡張子リスト(オリジナル)
	# MainWindow.image_file_ext_list = ( ".bmp", ".gif", ".jpg", ".jpeg", ".png", ".psd", ".tga", ".tif", ".tiff" )
	
	# SPI辞書
	# None の場合はPIL表示
	window.image_spi_list = {}
	
	# オリジナルの拡張子を登録
	for e in window.image_file_ext_list:
		window.image_spi_list[e] = None
	
	# spiオブジェクトを拡張子をキーとした辞書へ登録
	print(">>>> begin SPI")
	for s in spi_list:
		p = spi.SPI.fromPath(s)
		support_exts = p.getExtList()
		exts = support_exts.split(';')
		for e in exts:
			e = e.replace('*', '').lower()
			window.image_spi_list[e] = p
		print("load SPI %s : %s" % (s, support_exts))
	print("<<<< end SPI")
	
	# PIL表示を優先する場合、PIL拡張子のSPIをはNoneにする
	if keep_pil_ext:
		for e in window.image_file_ext_list:
			window.image_spi_list[e] = None
	
	# MainWindowの画像拡張子リストを上書き
	window.image_file_ext_list = window.image_spi_list.keys()
	
	# SPI対応のimageviewer呼び出し関数
	def callMyImageViewer(pane):
		items = []
		selection = 0

		for i in range(pane.file_list.numItems()):
			item = pane.file_list.getItem(i)
			ext = os.path.splitext(item.name)[1]
			ext = ext.lower()
			if ext in window.image_file_ext_list:
				items.append(item)
				if i==pane.cursor:
					selection = len(items)-1

		if len(items) :
			def onCursorMove(item):
				for i in range(pane.file_list.numItems()):
					if pane.file_list.getItem(i) is item:
						pane.cursor = i
						pane.scroll_info.makeVisible( pane.cursor, window.fileListItemPaneHeight(), 1 )
						window.paint(PAINT_UPPER)
						return
		
			def onSelect(item):
				for i in range(pane.file_list.numItems()):
					if pane.file_list.getItem(i) is item:
						pane.file_list.selectItem(i)
						window.paint(PAINT_UPPER)
						return
		
			pos = window.centerOfWindowInPixel()
			viewer = cfiler_imageviewer_mod.ImageViewerMod(
						pos[0], pos[1],
						window.width(), window.height(),
						window, window.ini,
						"image viewer",
						items, selection,
						onCursorMove, onSelect,
						window.image_spi_list )
			
#### old
# 			pos = window.centerOfWindowInPixel()
# 			viewer = cfiler_imageviewer_mod.ImageViewerMod( pos[0], pos[1],
# 															window.width(), window.height(),
# 															window, window.ini, u"image viewerMOD", items, selection, window.image_spi_list )
			return True
		
		return False
	
	# check spi list
# 	def checkSPIList():
# 		spi_list = window.image_spi_list
# 		print(">>>> ImageViewer condition:")
# 		for k in window.image_file_ext_list:
# 			p = spi_list[k]
# 			if p == None:
# 				print("ImageLib: ", k)
# 			else:
# 				print("SPI: ", k)
# 		print(">>>> end of ImageViewer condition")
# 	
# 	window.keymap[  KeyEvent( VK_F11, MODKEY_CTRL|MODKEY_SHIFT ) ] = checkSPIList
	
	# --------------------------------------------------------------------
	# Enter キーを押したときの動作をカスタマイズするためのフック
	
	def hook_Enter():
		pane = window.activePane()
		item = pane.file_list.getItem(pane.cursor)
		ext = os.path.splitext(item.name)[1].lower()
		
		# 画像ビュワー乗っ取り
		if ext in window.image_file_ext_list:
			# hook から True を返すと、デフォルトの動作がスキップされます
			return callMyImageViewer(pane)
		
		# 該当しないものはデフォルト動作
		return False

	#### フック関数設定
	window.enter_hook = hook_Enter


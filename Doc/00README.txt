****
* FM-7/77/AV エミュレータ、XM7 V3.4L30 for SDL/Linux
*  Version 0.1r15 (αレベル)
*
*  Copyright (C) 1999-2003 ＰＩ．(ytanaka@ipc-tokai.or.jp) 
*  Copyright (C) 2001-2003 Ryu Takegami
*  Copyright (C) 2004 GIMONS
*  Copyright (C) 2010 K.Ohta 
*
* 2010.01.31 Artane.
* HP:
* http://sky.geocities.jp/artanejp/
* 仮設SVN:
* http://xm7-sdl.unfuddle.com/

1.はじめに
 非常に古いマイコンであるFM-7のエミュレータに、XM-7(*0)と言う物があります。
 このエミュレータ自体はWin32上で動くものですが、基本的にLinuxしか使って
いない私にはあれこれもどかしい物がありました。
 そういう事もあって、SDLへの移植をこの一年ちょっと目論んでいたのですが、
GIMONS氏がX11/GTKに移植されていて(*1)、そのコードをベースに弄り始めたの
でした。
(*0) http://retropc.net/ryu/xm7/
(*1) http://www.geocities.jp/kugimoto0715/ 

2.SDL版の特徴
・XM7 V3.4L30ベースです(但し、描画まわりやダイアログが出来ていない所も
 ある)
・SDLのダイレクト描画機能を使っている上に多少細工してあるのでそこそこ
　高速です。
・SDL版オリジナル機能として、1280x800ドット描画機能を追加してあります。(*2)

(*2) PC-88エミュレータであるQuasi88に同等の機能があったので…

3.ビルド
 以下の物が、必要です。
・GTK2/GLIB2
・SDL1.2以上
・U*ix系OSの場合にはX Window Systemと開発環境、FontConfigなどなど。
・GCC 4
・SDL_mixer ( http://www.libsdl.org/cgi/docwiki.cgi/SDL_mixer )
・NASM
・GNU MAKE
以下は、デバッグ用に使っています。
・libmemwatch (http://www.linkdata.se/sourcecode/memwatch )

現状では6809エミュレーションをXM7添付のアセンブラコアで
やっているので、ia32でしかビルドできません。（最終的にはCコア
に移行する予定）
amd64環境の方はクロスビルドする必要がありますが、そのままmake出来
ちゃいます。多分。

libmemwatchはメモリリークをチェックする為の物なので…

4.つかいかた
a.xm7の実行型式があるディレクトリィに、以下の物を入れます。
・FM-77AVEXのROMイメージ、もしくはAppolo氏製の互換ROM
・FDDシークの為のWAV

b.
$ cd [インストールしたディレクトリィ]
$ ./xm7

c.
 手持ちのFD・テープをD77・T77形式に変換した物を動かして見てください。

*** 動くことを願っています(ぉぃ ***

5.TIPS

a.GNU/Linuxの場合、サウンドドライバとしてALSAを使っていると音飛びなどが
目立つと思います。
　これは、カーネルのタスクスケジューラの優先度の問題なので、
/etc/security/limits.confを変えてやるとほぼ解消出来ます（重い処理の時
には厳しいですが。)
(例)
---
hoge      -        rtprio          99
hoge      -        nice            -2
---
hogeはユーザ名など。
詳しくは、man limits.conf してみてください。

b.memwatchなどのビルドでクロスビルドが必要になるかもしれません。
　詳しくはmakefile.sdl.amd64を読んで見てください(無責任…)

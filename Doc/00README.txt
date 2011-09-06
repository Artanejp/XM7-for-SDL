****
* FM-7/77/AV エミュレータ、XM7 V3.4L31 for SDL/Linux(amd64)
*  Version 0.1r346 (βレベル / Agar / OpenGL)
*
*  Copyright (C) 1999-2003 ＰＩ．(ytanaka@ipc-tokai.or.jp) 
*  Copyright (C) 2001-2003 Ryu Takegami
*  Copyright (C) 2004 GIMONS
*  Copyright (C) 2010 K.Ohta 
*
* 2011.09.06 Artane.
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
・XM7 V3.4L31ベースです(但し、描画まわりやダイアログが出来ていない所も
 ある)
・OpenGLの描画機能を使っているので、拡大してもそこそこ高速です。
・
・SDL版オリジナル機能として、1280x800ドット描画機能を追加してあります。(*2)
・とうとう、AMD64ベースでのビルドが可能になりました＼(^o^)／
　まだまだバグ取らないといけないですが(；´Д｀)
・CPUコアをアセンブラベースから、sdlmess0136 (*3) にあるCコードベース
　のものに変更。とはいえ、内部構造がかなり違うので元々のアセンブラコード
　からCに移植した部分もかなりあるし、APIが全く違うのでかなり手を入れて
　います。
　messのライセンスについてはDoc/mess/license.txtに入れておきました
・OpenGLで描画しています（試験版）
・GUIにAgar toolkit ( http://www.libagar.org/ )を使用しています。
・仮想キーボードがつきます(r254以降)

(*2) PC-88エミュレータであるQuasi88に同等の機能があったので…
(*3) SDLMESS http://rbelmont.mameworld.info/?page_id=163

3.ビルド
 以下の物が、必要です。
・SDL1.2以上
・U*ix系OSの場合にはX Window Systemと開発環境、FontConfigなどなど。
・GCC 4
・SDL_ttf
・SDL_mixer ( http://www.libsdl.org/cgi/docwiki.cgi/SDL_mixer )
・AGAR ( http://www.libagar.org/ )
・IPA ゴシック一式(ビルド時に変更可能)
・GNU MAKE
・PKGCONFIG
・libtool
・OpenGLのライブラリ
などなど。

以下は、デバッグ用に使っています。
・Code::Blocks (統合開発環境/  http://www.codeblocks.org/ )

※rr151あたりから、描画機能をlibemugrphに分離しています。(ライセンスを別にしたいので）
　仕様が未だ固まっていない状態ですが…
　r254から、libemugrphをstatic linkするようにしています。
※rr300前後から、XM7のエミュレーションコアをvm/　に、Agar依存部分をui_agar/に、本体をsdl/にそれぞれ分離しました。

-- 以下、過去の話 --
現状では6809エミュレーションをXM7添付のアセンブラコアで
やっているので、ia32でしかビルドできません。（最終的にはCコア
に移行する予定）
amd64環境の方はクロスビルドする必要がありますが、そのままmake出来
ちゃいます。多分。
-- ここまで --


4.つかいかた
a.xm7の実行型式があるディレクトリィに、以下の物を入れます。

a. bin に移動して、
$sudo install ./xm7_debug ./xm7 /usr/local/bin

b. resource に移動して、
$sudo install -m 0777 -d /usr/local/share/xm7/
$sudo install -m 0644 ./* /usr/local/share/xm7/

c. libagarのインストール。AMD64についてはコンパイル済みバイナリを
   libagar.r9027/ 以下に添付してあります。これを、/usr/local 以下にインストールしてください

d. 以下のものを~/.xm7 に入れます。
・FM-77AVEXのROMイメージ、もしくはAppolo氏製の互換ROM
・FDDシークの為のWAV


e. xm7を起動します（カレントディレクトリになくてもよいです。てか、カレントディレクトリから削除した方がいいかも)

e.
 手持ちのFD・テープをD77・T77形式に変換した物を動かして見てください。

*** 動くことを願っています(ぉぃ ***

5. SVNのおさそい
現在、SVNリポジトリを使用してソースコードを管理しています。
但し、管理権限の関係があるので公開されてるかどうかわかりません（いいかげんでごめんなさい）
$ svn co http://xm7-sdl.unfuddle.com/svn/xm7-sdl_xm7-sdl-linux/linux-sdl

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


c.WAVファイル、ROMファイルを~/.xm7/にコピーしたのに動かないときは、
　これらファイルの大文字小文字を見直してください。

d.Agar使用に伴い、フォントを検索します。が、IPAゴシックのipag*.ttfが必要なのは同じです＿|￣|●
  かえる必要があるときはMakefileもしくはbuildconfig.hを弄ってください

-- 過去の情報 ---
d.r42から、UI定義をGTKBUILDERベースにしました。
　/usr/local/share/xm7/gtk_prop.ui などがないと
　動きませんΣ（￣□￣；）！！
e.現状では、フォントを決め打ちしています。
  (/usr/share/fonts/truetype/ipafont-jisx0208/ipagui0208_for_legacy_compatibility.ttf )
  かえる必要があるときは MakefileのUIFONT = の行、もしくはXM7.INIを弄ってください（Todo)
--- 過去の情報 ---

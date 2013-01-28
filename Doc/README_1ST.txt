XM7/SDL snapshot Jan 28,2013(or later).
              K.Ohta <whatisthis.sowhat@gmail.com>

1. At first...
 This is snapshot of XM7/SDL.
 GIT's revision is e89e20a99ddb4231670f8dc5c7182b7c4286c5c3;

>commit e89e20a99ddb4231670f8dc5c7182b7c4286c5c3
>Author: K.Ohta <whatisthis.sowhat@gmail.com>
>Date:   Mon Jan 28 04:04:32 2013 +0900
>
>    [SND] Fix wrong rendersize.


 This programs should start from terminal or batch.

      c:\>.\xm7.exe -d <Driver> [options]

 Now, default driver (wgl) is terrible buggy.
 Drivers are below: 
 sdlfb : Basic driver, use directdraw via SDL.
 sdlgl : SDL GL driver. Use texture via SDL and opengl32.
         Unfortunally, this is slower then sdlfb, differ
         to linux.
 wgl   : WinGL driver, now buggy of libagar's issue.
 glx   : X window GL driver, disabled on Windows.

 This xm7 was built with original build libagar, SVN 9549.
 And more in windows, I build SDL.dll and SDL_mixrer.dll.
 Now, buggy a little (except default driver).

2. Install
  1. On Windows, place ROMS and WAVS and FONTS and below of resource\ to .\xm7.
     You *should not* place I18N files, C/ and ja/ .
     On *nix, place F-Fonts (and IPA Gothic P) and below of resource and I18N files,
     and ROMS and WAVS  to ~/.xm7 .
     Font needs "IPA Gothic P".You can download from IPA's site.
     See, http://ossipedia.ipa.go.jp/ipafont/index.html#en .
  2. Start DOS console.
  3. Start .\xm7.exe [options] via DOS console.
 
3. Options (Both of Linux and Windows). 
 -d <driver>   : select display driver.
 -f            : Force full-screen, not test yet.
 -W            : Force window (default).
 -T <fontpath> : Set an alternative font path.
 -F <fontface> : Set an alternative font face for GUI.
 -S <fontsize> : Set an alternative font size for GUI.
 -o <fontface> : Set an alternative font path for OSD.
 -O <fontsize> : Set an alternative font size for OSD.
 -l <path>     : Set load path of configuration of agar.
 -s <path>     : Set save path of configuration of agar.
 -i <path>     : Set save and load path of configuration of agar.
 -t <fontspec> : Set alternative default fontspec.
 -?            : Mini help to console

4.Build from source
 1. You can get sourcecodes from GITHUB or my web page.
  GitHub (Latest or snapshot)
    $ git clone git://github.com/Artanejp/XM7-for-SDL.git

  WEB pages (Release)
    Primary site: http://sky.geocities.jp/artanejp/XM7-SDL/index.en.html
    Mirror site:  https://sites.google.com/site/artanejp/s/xm7-sdl-en

  Japanese WEB pages (Release)
    Primary site: http://sky.geocities.jp/artanejp/XM7-SDL/index.html
    Mirror site:  https://sites.google.com/site/artanejp/s

 2. After you got source, you should build MinGW32 on windows.
    If you use *nix OS, you can build native(maybe).
    You need libSDL and libSDL_mixer, http://www.libsdl.org/ .
    And you should build libAgar and this needs.
    See, http://libagar.org/ .
    More addtion on windows, libwinpthread if you can.
    https://github.com/pfarq/libwinpthread
 
 3. Edit config.mak, and make.
 4. Good luck!
                                                   -- K.Ohta.

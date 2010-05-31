#!/bin/sh
IA32BOOTSTRAPED=/emul/ia32-bootstrap
ADDLDFLAGS="-Wl,-rpath,/emul/ia32-bootstrap/usr/lib32,-rpath,/emul/ia32-bootstrap/usr/local/lib32,-rpath,/emul/ia32-bootstrap/lib32
-L/emul/ia32-bootstrap/usr/lib32 -L/emul/ia32-bootstrap/usr/local/lib32 -L/emul/ia32-bootstrap/lib32 "

ADDLIBS= "-lX11 -lXt -lXext -lgio-2.0  -lgthread-2.0 -lglib-2.0" 
ADDLIBS1="-lgtk-x11-2.0 -lgdk-x11-2.0 -latk-1.0 -lpangoft2-1.0 -lgdk_pixbuf-2.0  -lpangocairo-1.0  -lcairo -lpango-1.0 -lfreetype -lfontconfig -lgobject-2.0 -lgmodule-2.0 -lpcre -lpcreposix -lrt -ldl"
ADDLIBS2="-lSDL -lSDL_mixer -lSDL_ttf -lartsc -lesd -lcaca -laa -lgpm -lpulsecommon-0.9.21 -lpulse-simple"

#LIBS += -lmemwatch
g++ -m32 $LDFLAGS $ADDLIBS $ADDLIBS1 $ADDLIBS2 $@
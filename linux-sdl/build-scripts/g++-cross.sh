#!/bin/sh
IA32BOOTSTRAPED=/emul/ia32-bootstrap
ADDCFLAGS="-I/emul/ia32-bootstrap/usr/include -I/emul/ia32-bootstrap/usr/local/include -I/emul/ia32-bootstrap/usr/X11R6/include -I. -D_REENTRANT -pthread -I/emul/ia32-bootstrap/usr/include/gtk-2.0                 -I/emul/ia32-bootstrap/usr/lib/gtk-2.0/include  -I/emul/ia32-bootstrap/usr/include/atk-1.0 -I/emul/ia32-bootstrap/usr/include/cairo -I/emul/ia32-bootstrap/usr/include/pango-1.0 -I/emul/ia32-bootstrap/usr/include/pixman-1 -I/emul/ia32-bootstrap/usr/include/freetype2 -I/emul/ia32-bootstrap/usr/include/directfb -I/emul/ia32-bootstrap/usr/include/libpng12 -I/emul/ia32-bootstrap/usr/include/glib-2.0 -I/emul/ia32-bootstrap/usr/lib/glib-2.0/include -I/emul/ia32-bootstrap/usr/include/libglade-2.0/ -pthread -I/usr/include/glib-2.0 -I/emul/ia32-bootstrap/usr/lib/glib-2.0/include"
g++ -m32 $ADDCFLAGS $@
## Process this file with automake to produce Makefile.in

AM_CXXFLAGS = @X_CFLAGS@
INCLUDES = -I$(top_srcdir)/include -I$(top_builddir)/include
RESWRAP  = $(top_builddir)/utils/reswrap$(EXEEXT)
SUFFIXES = .gif .bmp
LDADD = $(top_builddir)/lib/libFOX-1.7.la -lm

noinst_PROGRAMS	= \
scan \
format \
console \
thread \
calendar \
expression \
dctest \
codecs \
dialog \
groupbox \
scribble \
wizard \
layout \
rex \
table \
memmap \
minheritance \
bitmapviewer \
imageviewer \
button \
datatarget \
dirlist \
foursplit \
header \
hello \
hello2 \
iconlist \
image \
mditest \
shutter \
splitter \
switcher \
tabbook \
ratio \
half \
gltest \
glviewer

format_SOURCES          = format.cpp
scan_SOURCES            = scan.cpp
console_SOURCES         = console.cpp
thread_SOURCES          = thread.cpp
calendar_SOURCES        = calendar.cpp
expression_SOURCES      = expression.cpp
wizard_SOURCES	        = wizard.cpp
rex_SOURCES	        = rex.cpp
layout_SOURCES	        = layout.cpp
minheritance_SOURCES	= minheritance.cpp
memmap_SOURCES	        = memmap.cpp
gltest_SOURCES	        = gltest.cpp
glviewer_SOURCES        = glviewer.cpp
mditest_SOURCES		= mditest.cpp
table_SOURCES		= table.cpp
groupbox_SOURCES	= groupbox.cpp
foursplit_SOURCES	= foursplit.cpp
datatarget_SOURCES	= datatarget.cpp
bitmapviewer_SOURCES	= bitmapviewer.cpp
imageviewer_SOURCES	= imageviewer.cpp
scribble_SOURCES	= scribble.cpp
splitter_SOURCES	= splitter.cpp
shutter_SOURCES		= shutter.cpp
image_SOURCES		= image.cpp
header_SOURCES		= header.cpp
hello_SOURCES		= hello.cpp
hello2_SOURCES		= hello2.cpp
switcher_SOURCES	= switcher.cpp
iconlist_SOURCES	= iconlist.cpp
tabbook_SOURCES		= tabbook.cpp
ratio_SOURCES		= ratio.cpp
dialog_SOURCES		= dialog.cpp
dirlist_SOURCES		= dirlist.cpp
button_SOURCES		= button.cpp
codecs_SOURCES		= codecs.cpp
dctest_SOURCES		= dctest.cpp dippy.h
half_SOURCES		= half.cpp



EXTRA_DIST = Makefile.bc \
             Makefile.wc \
             Makefile.dmc

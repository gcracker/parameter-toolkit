CUDD_HOME := ../../cudd-2.4.2
HOOPS_INSTALL_DIR := ../HOOPS-1500

PKGS := glib-2.0 gobject-2.0

PKG_CFLAGS := `pkg-config $(PKGS) --cflags`
PKG_LDFLAGS := `pkg-config $(PKGS) --libs`

CUDD_CFLAGS := -I$(CUDD_HOME)/include

CUDD_LIBS := -L$(CUDD_HOME)/dddmp -ldddmp -L$(CUDD_HOME)/cudd -lcudd \
             -L$(CUDD_HOME)/mtr -lmtr -L$(CUDD_HOME)/st -lst \
             -L$(CUDD_HOME)/util -lutil -L$(CUDD_HOME)/epd -lepd \
	     -L$(CUDD_HOME)/extra20 -lextra \
	     -lm -lbfd -lopcodes -liberty

#HOOPS_LIBS := $(HOOPS_LIBS) -L$(HOOPS_INSTALL_DIR)/Dev_Tools/hoops_3dgs/lib/linux_x86_64/ \
#			  -lhoops1500

INC_PATH = $(QT_INC) $(HOOPS_INC) $(X11_INC)
LIBS = $(HOOPS_LIBS) $(QT_LIBS) $(X11_LIBS) $(CUDD_LIBS)


SRCS = \
	HQApplication.cpp\
	HQDeleter.cpp\
	SimpleHQWidget.cpp\
	bddwork.cpp\
	zddwork.cpp\
	bfdwork.cpp\
	HQWidget.cpp\
	main.cpp\
	moc_HQApplication.cpp\
	moc_HQDeleter.cpp\
	moc_SimpleHQWidget.cpp\
	moc_HQWidget.cpp\
	PMZoom.cpp\
	PMPan.cpp\
	PMDinRdyGraph.cpp\
	PMBDDCollect.cpp\
	PMSelectArea.cpp\
	imagework.cpp




OBJS = $(SRCS:.cpp=.o)

.SUFFIXES:
.SUFFIXES: .o .cpp

.cpp.o:
	$(CCP) $(CPPFLAGS) $(PKG_CFLAGS) $(CUDD_CFLAGS) $(INC_PATH) -c $<



default:
	@echo "targets:"
	@echo "   make linux"
	@echo "   make linux_x86_64"

TARGET = ParaMeter

BIN_PLATFORM_PATH = $(PLATFORM)

$(TARGET): moc_files $(OBJS)
	$(CCP) -O3 -o $(TARGET) $(OBJS) $(PKG_CFLAGS) $(CUDD_CFLAGS) $(LIBS) $(LFLAGS) $(PKG_LDFLAGS) 
#	mkdir -p ../../../bin/$(BIN_PLATFORM_PATH)
#	cp $(TARGET) ../../../bin/$(BIN_PLATFORM_PATH)/$(TARGET)


moc_files: moc_SimpleHQWidget.cpp moc_HQWidget.cpp moc_HQApplication.cpp moc_HQDeleter.cpp

moc_SimpleHQWidget.cpp: SimpleHQWidget.h
	$(MOC) SimpleHQWidget.h -o moc_SimpleHQWidget.cpp

moc_HQWidget.cpp: HQWidget.h
	$(MOC) HQWidget.h -o moc_HQWidget.cpp

moc_HQApplication.cpp: HQApplication.h
	$(MOC) HQApplication.h -o moc_HQApplication.cpp

moc_HQDeleter.cpp: HQDeleter.h
	$(MOC) HQDeleter.h -o moc_HQDeleter.cpp

clean: $(MAKE)_clean

make_clean:
	-rm -f moc_*
	-rm -f *.obj
	-rm -f $(TARGET)
	-rm -f $(TARGET).exe
	-rm -f $(TARGET).lib
	-rm -f $(TARGET).exp
	-rm -f *~
	-rm -f *.o
	-rm -f core
	-rm -Rf SunWS_cache


HOOPS_VERSION = 1500
HOOPS_PATH = $(HOOPS_INSTALL_DIR)/Dev_Tools/hoops_3dgs
HOOPS_MVO_BASE = $(HOOPS_INSTALL_DIR)/Dev_Tools/hoops_mvo
HOOPS_STREAM_BASE = $(HOOPS_INSTALL_DIR)/Dev_Tools/hoops_stream
HOOPS_NET_BASE = $(HOOPS_INSTALL_DIR)/Dev_Tools/hoops_net
HOOPS_UTILITY = $(HOOPS_INSTALL_DIR)/Dev_Tools/utility
QTDIR = /usr/share/qt4
LINUX_QT_BASE = $(QTDIR)


####Linux####

linux:
	$(MAKE) $(MFLAGS) $(TARGET) \
	"MOC = $(LINUX_QT_BASE)/bin/moc" \
	"QT_INC = -I$(LINUX_QT_BASE)/include -I$(LINUX_QT_BASE)/include/Qt -I$(LINUX_QT_BASE)/include/QtCore -I$(LINUX_QT_BASE)/include/QtGui" \
	"HOOPS_INC = -I$(HOOPS_MVO_BASE)/source -I$(HOOPS_NET_BASE)/source -I$(HOOPS_STREAM_BASE)/source -I$(HOOPS_STREAM_BASE)/source/stream_common  -I$(HOOPS_PATH)/source -I$(HOOPS_UTILITY)/source" \
	"X11_INC = -I/usr/X11R6/include" \
	"QT_LIBS = -L$(LINUX_QT_BASE)/lib/ -lQtCore -lQtGui " \
	"HOOPS_LIBS = -lfreetype -L$(HOOPS_MVO_BASE)/lib/linux -lhoops_mvo -L$(HOOPS_STREAM_BASE)/lib/linux -lhoops_stream -L$(HOOPS_PATH)/lib/linux -lhoops$(HOOPS_VERSION) -L$(HOOPS_NET_BASE)/lib/linux -lhoops_net -L$(HOOPS_UTILITY)/lib/linux -lhoops_utils" \
	"X11_LIBS = -L/usr/X11R6/lib -lXmu -lXext -lX11 -lm -ldl -lpthread" \
	"CUDD_LIBS = $(CUDD_LIBS)" \
	"CCP = g++" \
	"CPPFLAGS = -c -pipe -O2 -Wall -DUNIX_SYSTEM -DLINUX_SYSTEM -DNDEBUG -DIS_X11 -DIS_QT -DUSE_GLX_VISUAL" \
	"LFLAGS = -Wl,-rpath,$(HOOPS_PATH)/lib/linux" \
	"CUDD_CFLAGS = $(CUDD_CFLAGS)" \
	"PLATFORM = linux"

linux-debug:
	$(MAKE) $(MFLAGS) $(TARGET) \
	"MOC = $(LINUX_QT_BASE)/bin/moc" \
	"QT_INC = -I$(LINUX_QT_BASE)/include -I$(LINUX_QT_BASE)/include/Qt -I$(LINUX_QT_BASE)/include/QtCore -I$(LINUX_QT_BASE)/include/QtGui" \
	"HOOPS_INC = -I$(HOOPS_MVO_BASE)/source -I$(HOOPS_NET_BASE)/source -I$(HOOPS_STREAM_BASE)/source -I$(HOOPS_STREAM_BASE)/source/stream_common  -I$(HOOPS_PATH)/source -I$(HOOPS_UTILITY)/source" \
	"X11_INC = -I/usr/X11R6/include" \
	"QT_LIBS = -L$(LINUX_QT_BASE)/lib/ -lQtCore -lQtGui" \
	"HOOPS_LIBS = -lfreetype -L$(HOOPS_MVO_BASE)/lib/linux -lhoops_mvo -L$(HOOPS_STREAM_BASE)/lib/linux -lhoops_stream -L$(HOOPS_PATH)/lib/linux -lhoops$(HOOPS_VERSION) -L$(HOOPS_NET_BASE)/lib/linux -lhoops_net -L$(HOOPS_UTILITY)/lib/linux/ -lhoops_utils" \
	"X11_LIBS = -L/usr/X11R6/lib -lXmu -lXext -lX11 -lm -ldl -lpthread" \
	"CUDD_LIBS = $(CUDD_LIBS)" \
	"CCP = g++" \
	"CPPFLAGS = -c -pipe -g -pg  -Wall -DUNIX_SYSTEM -DLINUX_SYSTEM -DIS_X11 -DIS_QT -DUSE_GLX_VISUAL" \
	"LFLAGS = -Wl,-rpath,$(HOOPS_PATH)/lib/linux" \
	"PLATFORM = linux"

####Linux x64####

linux_x86_64:
	$(MAKE) $(MFLAGS) $(TARGET) \
	"MOC = $(LINUX_QT_BASE)/bin/moc" \
	"QT_INC = -I$(LINUX_QT_BASE)/include -I$(LINUX_QT_BASE)/include/Qt -I$(LINUX_QT_BASE)/include/QtCore -I$(LINUX_QT_BASE)/include/QtGui" \
	"HOOPS_INC = -I$(HOOPS_MVO_BASE)/source -I$(HOOPS_NET_BASE)/source -I$(HOOPS_STREAM_BASE)/source -I$(HOOPS_STREAM_BASE)/source/stream_common  -I$(HOOPS_PATH)/source -I$(HOOPS_UTILITY)/source" \
	"X11_INC = -I/usr/X11R6/include" \
	"QT_LIBS = -L$(LINUX_QT_BASE)/lib/ -lQtCore -lQtGui " \
	"HOOPS_LIBS = -lfreetype -L$(HOOPS_MVO_BASE)/lib/linux_x86_64 -lhoops_mvo -L$(HOOPS_STREAM_BASE)/lib/linux_x86_64 -lhoops_stream -L$(HOOPS_PATH)/lib/linux_x86_64 -lhoops$(HOOPS_VERSION) -L$(HOOPS_NET_BASE)/lib/linux_x86_64 -lhoops_net -L$(HOOPS_UTILITY)/lib/linux_x86_64 -lhoops_utils" \
	"X11_LIBS = -L/usr/X11R6/lib64 -lXmu -lXext -lX11 -lm -ldl -lpthread" \
	"CUDD_LIBS = $(CUDD_LIBS)" \
	"CCP = g++" \
	"CPPFLAGS = -c -pipe -O2 -Wall -DUNIX_SYSTEM -DLINUX_SYSTEM -DNDEBUG -DIS_X11 -DIS_QT -DUSE_GLX_VISUAL" \
	"LFLAGS = -Wl,-rpath,$(HOOPS_PATH)/lib/linux_x86_64" \
	"CUDD_CFLAGS = $(CUDD_CFLAGS)" \
	"PLATFORM = linux_x86_64"

linux_x86_64-debug:

	$(MAKE) $(MFLAGS) $(TARGET) \
	"MOC = $(LINUX_QT_BASE)/bin/moc" \
	"QT_INC = -I$(LINUX_QT_BASE)/include -I$(LINUX_QT_BASE)/include/Qt -I$(LINUX_QT_BASE)/include/QtCore -I$(LINUX_QT_BASE)/include/QtGui" \
	"HOOPS_INC = -I$(HOOPS_MVO_BASE)/source -I$(HOOPS_NET_BASE)/source -I$(HOOPS_STREAM_BASE)/source -I$(HOOPS_STREAM_BASE)/source/stream_common  -I$(HOOPS_PATH)/source -I$(HOOPS_UTILITY)/source" \
	"X11_INC = -I/usr/X11R6/include" \
	"QT_LIBS = -L$(LINUX_QT_BASE)/lib/ -lQtCore -lQtGui " \
	"HOOPS_LIBS = -lfreetype -L$(HOOPS_MVO_BASE)/lib/linux_x86_64 -lhoops_mvo -L$(HOOPS_STREAM_BASE)/lib/linux_x86_64 -lhoops_stream -L$(HOOPS_PATH)/lib/linux_x86_64 -lhoops$(HOOPS_VERSION) -L$(HOOPS_NET_BASE)/lib/linux_x86_64 -lhoops_net -L$(HOOPS_UTILITY)/lib/linux_x86_64 -lhoops_utils" \
	"X11_LIBS = -L/usr/X11R6/lib64 -lXmu -lXext -lX11 -lm -ldl -lpthread" \
	"CUDD_LIBS = $(CUDD_LIBS)" \
	"CCP = g++" \
	"CPPFLAGS = -c -pipe -g -pg -Wall -DUNIX_SYSTEM -DLINUX_SYSTEM -DIS_X11 -DIS_QT -DUSE_GLX_VISUAL" \
	"LFLAGS = -Wl,-rpath,$(HOOPS_PATH)/lib/linux_x86_64" \
	"PLATFORM = linux_x86_64"

####Linux icc####

linux_icc:
	$(MAKE) $(MFLAGS) $(TARGET) \
	"MOC = $(LINUX_QT_BASE)/bin/moc" \
	"QT_INC = -I$(LINUX_QT_BASE)/include/qt4 -I$(LINUX_QT_BASE)/include/qt4/Qt -I$(LINUX_QT_BASE)/include/qt4/QtCore -I$(LINUX_QT_BASE)/include/qt4/QtGui" \
	"HOOPS_INC = -I$(HOOPS_MVO_BASE)/source -I$(HOOPS_NET_BASE)/source -I$(HOOPS_STREAM_BASE)/source -I$(HOOPS_STREAM_BASE)/source/stream_common  -I$(HOOPS_PATH)/source -I$(HOOPS_UTILITY)/source" \
	"X11_INC = -I/usr/X11R6/include" \
	"QT_LIBS = -L$(LINUX_QT_BASE)/lib/ -lQtCore -lQtGui" \
	"HOOPS_LIBS = -lfreetype -L$(HOOPS_MVO_BASE)/lib/linux_icc -lhoops_mvo -L$(HOOPS_STREAM_BASE)/lib/linux_icc -lhoops_stream -L$(HOOPS_PATH)/lib/linux_icc -lhoops$(HOOPS_VERSION) -L$(HOOPS_NET_BASE)/lib/linux_icc -lhoops_net -L$(HOOPS_UTILITY)/lib/linux_icc -lhoops_utils" \
	"X11_LIBS = -L/usr/X11R6/lib -lXmu -lXext -lX11 -lm -ldl -lpthread" \
	"CUDD_LIBS = $(CUDD_LIBS)" \
	"CCP = icc" \
	"CPPFLAGS = -c -O2 -Wall -DUNIX_SYSTEM -DLINUX_SYSTEM -DNDEBUG -DIS_X11 -DIS_QT -DUSE_GLX_VISUAL" \
	"LFLAGS = " \
	"PLATFORM = linux_icc"

linux_icc-debug:
	$(MAKE) $(MFLAGS) $(TARGET) \
	"MOC = $(LINUX_QT_BASE)/bin/moc" \
	"QT_INC = -I$(LINUX_QT_BASE)/include/qt4 -I$(LINUX_QT_BASE)/include/qt4/Qt -I$(LINUX_QT_BASE)/include/qt4/QtCore -I$(LINUX_QT_BASE)/include/qt4/QtGui" \
	"HOOPS_INC = -I$(HOOPS_MVO_BASE)/source -I$(HOOPS_NET_BASE)/source -I$(HOOPS_STREAM_BASE)/source -I$(HOOPS_STREAM_BASE)/source/stream_common  -I$(HOOPS_PATH)/source -I$(HOOPS_UTILITY)/source" \
	"X11_INC = -I/usr/X11R6/include" \
	"QT_LIBS = -L$(LINUX_QT_BASE)/lib/ -lQtCore -lQtGui" \
	"HOOPS_LIBS = -lfreetype -L$(HOOPS_MVO_BASE)/lib/linux_icc -lhoops_mvo -L$(HOOPS_STREAM_BASE)/lib/linux_icc -lhoops_stream -L$(HOOPS_PATH)/lib/linux_icc -lhoops$(HOOPS_VERSION) -L$(HOOPS_NET_BASE)/lib/linux_icc -lhoops_net -L$(HOOPS_UTILITY)/lib/linux_icc/ -lhoops_utils" \
	"X11_LIBS = -L/usr/X11R6/lib -lXmu -lXext -lX11 -lm -ldl -lpthread" \
	"CUDD_LIBS = $(CUDD_LIBS)" \
	"CCP = icc" \
	"CPPFLAGS = -c -g -Wall -DUNIX_SYSTEM -DLINUX_SYSTEM -DIS_X11 -DIS_QT -DUSE_GLX_VISUAL" \
	"LFLAGS = " \
	"PLATFORM = linux_icc"


####Linux mdi####

linux-mdi:
	$(MAKE) $(MFLAGS) $(TARGET) \
	"MOC = $(LINUX_QT_BASE)/bin/moc" \
	"QT_INC = -I$(LINUX_QT_BASE)/include/qt4 -I$(LINUX_QT_BASE)/include/qt4/Qt -I$(LINUX_QT_BASE)/include/qt4/QtCore -I$(LINUX_QT_BASE)/include/qt4/QtGui" \
	"HOOPS_INC = -I$(HOOPS_MVO_BASE)/source -I$(HOOPS_NET_BASE)/source -I$(HOOPS_STREAM_BASE)/source -I$(HOOPS_STREAM_BASE)/source/stream_common  -I$(HOOPS_PATH)/source -I$(HOOPS_UTILITY)/source" \
	"X11_INC = -I/usr/X11R6/include" \
	"QT_LIBS = -L$(LINUX_QT_BASE)/lib/ -lQtCore -lQtGui" \
	"HOOPS_LIBS = -lfreetype -L$(HOOPS_MVO_BASE)/lib/linux -lhoops_mvo -L$(HOOPS_STREAM_BASE)/lib/linux -lhoops_stream -L$(HOOPS_PATH)/lib/linux -lhoops$(HOOPS_VERSION) -L$(HOOPS_NET_BASE)/lib/linux -lhoops_net -L$(HOOPS_UTILITY)/lib/linux -lhoops_utils" \
	"X11_LIBS = -L/usr/X11R6/lib -lXmu -lXext -lX11 -lm -ldl -lpthread" \
	"CUDD_LIBS = $(CUDD_LIBS)" \
	"CCP = g++" \
	"CPPFLAGS = -c -pipe -O2 -Wall -DUNIX_SYSTEM -DLINUX_SYSTEM -DNDEBUG -DIS_X11 -DIS_QT -DUSE_GLX_VISUAL -DUSE_MDI" \
	"LFLAGS = " \
	"PLATFORM = linux"

linux-debug-mdi:
	$(MAKE) $(MFLAGS) $(TARGET) \
	"MOC = $(LINUX_QT_BASE)/bin/moc" \
	"QT_INC = -I$(LINUX_QT_BASE)/include/qt4 -I$(LINUX_QT_BASE)/include/qt4/Qt -I$(LINUX_QT_BASE)/include/qt4/QtCore -I$(LINUX_QT_BASE)/include/qt4/QtGui" \
	"HOOPS_INC = -I$(HOOPS_MVO_BASE)/source -I$(HOOPS_NET_BASE)/source -I$(HOOPS_STREAM_BASE)/source -I$(HOOPS_STREAM_BASE)/source/stream_common  -I$(HOOPS_PATH)/source -I$(HOOPS_UTILITY)/source" \
	"X11_INC = -I/usr/X11R6/include" \
	"QT_LIBS = -L$(LINUX_QT_BASE)/lib/ -lQtCore -lQtGui" \
	"HOOPS_LIBS = -lfreetype -L$(HOOPS_MVO_BASE)/lib/linux -lhoops_mvo -L$(HOOPS_STREAM_BASE)/lib/linux -lhoops_stream -L$(HOOPS_PATH)/lib/linux -lhoops$(HOOPS_VERSION) -L$(HOOPS_NET_BASE)/lib/linux -lhoops_net -L$(HOOPS_UTILITY)/lib/linux/ -lhoops_utils" \
	"X11_LIBS = -L/usr/X11R6/lib -lXmu -lXext -lX11 -lm -ldl -lpthread" \
	"CUDD_LIBS = $(CUDD_LIBS)" \
	"CCP = g++" \
	"CPPFLAGS = -c -pipe -g  -Wall -DUNIX_SYSTEM -DLINUX_SYSTEM -DIS_X11 -DIS_QT -DUSE_GLX_VISUAL -DUSE_MDI" \
	"LFLAGS = " \
	"PLATFORM = linux"



####NT_I386####

nt_i386:
	MSDEV qt_simple.dsp /MAKE "qt_simple - Win32 Release"

nt_i386-debug:
	MSDEV qt_simple.dsp /MAKE "qt_simple - Win32 Debug"

nt_i386_sln:
	devenv /build Release qt_simple.vcproj

nt_i386_sln-debug:
	devenv /build Debug qt_simple.vcproj

nmake_clean:
	-MSDEV qt_simple.dsp /MAKE "qt_simple - Win32 Release" /CLEAN
	-MSDEV qt_simple.dsp /MAKE "qt_simple - Win32 Debug" /CLEAN
	-devenv /clean Release qt_simple.vcproj
	-devenv /clean Debug qt_simple.vcproj





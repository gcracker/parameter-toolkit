/*#*STARTLICENCE*#
 Copyright (c) 2005-2009, Regents of the University of Colorado

 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.

 Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.

 Neither the name of the University of Colorado nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
  #*ENDLICENCE*#*/


// HQtMain.cpp 
//  
// HOOPS/MVO and HOOPS/QT global variables are declared and initialized 
// One Qt QApplication object is instanced and configured 
// One initial HQApplication object is instanced 
// The Qt event processing loop is launched 
// Qt Headers 
#include <QApplication>
#include <QMessageBox>
#include <QObject>

// CUDD Headers
#include "dddmp.h"
#include <cudd.h>
#include <getopt.h>
//#include <cuddInt.h>

// HOOPS/MVO Headers 
#include "HDB.h" 

// HOOPS/Qt Headers 
#include "HQApplication.h" 
#include "HQDeleter.h" 
#include "SimpleHQWidget.h"
#include "HTManager.h"

// HOOPS Headers 
#include "hc.h" 


// Create Global pointer to HOOPS/MVO class HDB 
HDB * m_pHDB=0; 

// Create a global pointer to HOOPS/Qt class HQDeleter 
HQDeleter * deleter=0; 

// A few other global variables
char * DDstartupFile = NULL;
int x_res = 0;
int y_res= 0;
int depth_clip = 0;
guint64 x_scale = 0;
guint64 y_scale = 0;
guint64 x_start = 0;
guint64 x_stop = 0;
guint64 x_cap = 0;
bool invst = false;
guint64 y_start = 0;
guint64 y_stop = 0;
guint64 g_start_sin = 0;
bool g_render = true;
bool dieondone = false;  
float min_res_time = 0.0;
float max_res_time = 0.0;
bool use_res_time = true;

static struct option long_options[] =
  {
    {"xres",  required_argument, 0, 'x'},
    {"yres",  required_argument, 0, 'y'},
    {"ddfile",required_argument, 0, 'l'},
    {"invst", optional_argument, 0, 'i'},
    {"dieondone", optional_argument, 0, 'd'},
    {"xscale",required_argument, 0, 's'},
    {"xscale",required_argument, 0, 'r'},
    {"yscale",required_argument, 0, 'w'},
    {"xstart",required_argument, 0, 'm'},
    {"xstop",required_argument,0,'q'},
    {"xcap",required_argument, 0, 'p'},
    {"startsin",required_argument, 0, 'k'},
    {"depthclip",required_argument, 0, 'c'},
    {"minrestime",required_argument, 0, 'e'},
    {"maxrestime",required_argument, 0, 't'},
    {"norestime",optional_argument, 0, 'u'},
    {0, 0, 0, 0}
  };
  

class timer : public QObject {
public:
  timer( int ms ) {
    id = startTimer( ms );
  };
  ~timer() {
    killTimer(id);
  };

  void timerEvent( QTimerEvent * ) {
    float time;

    HC_Show_Time( &time );
    HDB::GetHTManager()->Tick( time );
  };
private:
  int id;
};

int main( int argc, char **argv ) 
{  
  int c = 0;
  bool helpOnly = false;
  

  const char * shortoptions = "c:k:s:w:p:l:x:y:q:e:t:urdh";
  int option_index = 0;


  //! Grab command line options
  c = getopt_long (argc, argv, shortoptions,
		   long_options, &option_index);     

  while (c != -1)
    {

      switch (c) 
	{
        case 'e':
	  min_res_time = (float)g_ascii_strtod(optarg, (char **)NULL);
	  break;
        case 't':
	  max_res_time = (float)g_ascii_strtod(optarg, (char **)NULL);
	  break;
        case 'u':
	  use_res_time = false;
	  break;
            
        case 'c':
	  depth_clip = atoi(optarg);
	  break;

        case 'd':
	  //! die when done rendering the DD
	  //! mostly for gathering timing data
	  dieondone = true;
	  break;

	  // load the DD at startup
	case 'l':
	  DDstartupFile = optarg;
	  break;
		  
		  
	  //! set the x resolution	  
	case 'x':
	  x_res = atoi(optarg);
	  break;
		  
		
	  //! turn rendering on or off
	case 'r':
	  g_render = false;
	  break;

        case 'p':
	  x_cap = g_ascii_strtoull(optarg,(char **)NULL, 10);
	  break;

	case 'i':
	  invst = true;
	  break;

        case'm':
	  x_start = g_ascii_strtoull(optarg,(char **)NULL, 10);
	  break;

        case 's':
	  x_scale = g_ascii_strtoull(optarg,(char **)NULL, 10);
	  break;

        case 'w':
	  y_scale = g_ascii_strtoull(optarg,(char **)NULL, 10);
	  break;
            
        case 'q':
	  x_stop = g_ascii_strtoull(optarg,(char **)NULL, 10);
	  break;

        case 'k':
	  g_start_sin = g_ascii_strtoull(optarg,(char **)NULL, 10);
	  break;

        case 'h':
	  g_print("Enter -h for this help guide\n");
	  g_print("--xres <num> for X axis resolution\n");
	  g_print("--yres <num> for Y axis resolution\n");
	  g_print("--ddfile <file> to specify a file to load on startup\n");
	  g_print("--norender to not render any graph\n");
	  g_print("--dieondone To shutdown when the DD graph is done being drawn\n");
	  g_print("--xstart To shift the start of the X axis\n");
	  g_print("--xstop To force a finish searching of the X axis\n");
	  g_print("--xcap To stop after grabbing so many DINs\n");
	  g_print("--xscale To scale the X axis\n");
	  g_print("--yscale To scake the Y axis\n");
	  g_print("--startsin To give the initial SIN number");

	  helpOnly = true;
	  break;
	}		  
		

      //! get the next option
      c = getopt_long (argc, argv, shortoptions,
		       long_options, &option_index);
    }

  if(true == invst)
    {
      g_render = false;
      dieondone = true;
    }


  if (!helpOnly)
    {

      // Initialize the global HOOPS Database Object 
      m_pHDB = new HDB(); 
      m_pHDB->Init();
          
          
      // Create an HQDeleter object and initialize the global pointer 
      deleter = new HQDeleter();

      QApplication::setColorSpec( QApplication::ManyColor );
          
      // Create the one QApplication object 
      QApplication * a = new QApplication(argc,argv); 
      a->setQuitOnLastWindowClosed(true);
          
      if(argc == 2){
              
	HQApplication * ha = new HQApplication(a, argv[1]);
	//NOP
	ha=ha;
      }else{
              
	HQApplication * ha = new HQApplication(a);
	//NOP
	ha=ha;
      }
          
      if(g_render || true)
	{
	  // Launch the Qt event processing loop 
	  a->exec(); 
	}
      // Clean up 
      delete m_pHDB;
    }
  return EXIT_SUCCESS;

} 






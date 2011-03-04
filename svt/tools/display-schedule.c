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

#include <svt.h>
#include <glib.h>
#include <math.h>
#include <ctype.h>
#include <stdlib.h>
#include <getopt.h>

/* All constraints are of the form:
 *
 * constant[0] < (sin_coeff * sin) + (din_coeff * din) + (cycle_coeff * cycle) < constant[1]
 *
 */

#define LOWER_BOUND_INDEX 0
#define UPPER_BOUND_INDEX 1

typedef struct {
  double sin_coeff;
  double din_coeff;
  double cycle_coeff;
  double constant[2];
} LinearConstraint_t;

static inline gboolean evaluate_constraint(LinearConstraint_t *constraint, 
					   guint64 din, guint64 sin, 
					   guint64 cycle)
{
  double value = 
    (constraint->sin_coeff * sin) +
    (constraint->din_coeff * din) +
    (constraint->cycle_coeff * cycle);

  return (constraint->constant[LOWER_BOUND_INDEX] < value) &&
    (value < constraint->constant[UPPER_BOUND_INDEX]);
}

static inline void find_minimums(LinearConstraint_t *constraint,
				 guint64 *min_din,
				 guint64 *min_sin,
				 guint64 *min_cycle)
{
  int index;
  double min;

  *min_sin = 0;
  *min_din = 0;
  *min_cycle = 0;

  if((constraint->sin_coeff > 0 && constraint->din_coeff <= 0 && constraint->cycle_coeff <= 0) ||
     (constraint->sin_coeff < 0 && constraint->din_coeff >= 0 && constraint->cycle_coeff >= 0)) {
    index = constraint->sin_coeff > 0 ? 0 : 1;
    min = constraint->constant[index] / constraint->sin_coeff;
    *min_sin = ceil(min);
  }

  if((constraint->din_coeff > 0 && constraint->sin_coeff <= 0 && constraint->cycle_coeff <= 0) ||
     (constraint->din_coeff < 0 && constraint->sin_coeff >= 0 && constraint->cycle_coeff >= 0)) {
    index = constraint->din_coeff > 0 ? 0 : 1;
    min = constraint->constant[index] / constraint->din_coeff;
    *min_din = ceil(min);
  }

  if((constraint->cycle_coeff > 0 && constraint->din_coeff <= 0 && constraint->sin_coeff <= 0) ||
     (constraint->cycle_coeff < 0 && constraint->din_coeff >= 0 && constraint->sin_coeff >= 0)) {
    index = constraint->cycle_coeff > 0 ? 0 : 1;
    min = constraint->constant[index] / constraint->cycle_coeff;
    *min_cycle = ceil(min);
  }
}

static inline void find_maximums(LinearConstraint_t *constraint,
				 guint64 *max_din,
				 guint64 *max_sin,
				 guint64 *max_cycle)
{
  int index;
  double max;

  *max_sin = G_MAXUINT64;
  *max_din = G_MAXUINT64;
  *max_cycle = G_MAXUINT64;

  if((constraint->sin_coeff > 0 && constraint->din_coeff >= 0 && constraint->cycle_coeff >= 0) ||
     (constraint->sin_coeff < 0 && constraint->din_coeff <= 0 && constraint->cycle_coeff <= 0)) {
    index = constraint->sin_coeff > 0 ? 1 : 0;
    max = constraint->constant[index] / constraint->sin_coeff;
    *max_sin = floor(max);
  }

  if((constraint->din_coeff > 0 && constraint->sin_coeff >= 0 && constraint->cycle_coeff >= 0) ||
     (constraint->din_coeff < 0 && constraint->sin_coeff <= 0 && constraint->cycle_coeff <= 0)) {
    index = constraint->din_coeff > 0 ? 1 : 0;
    max = constraint->constant[index] / constraint->din_coeff;
    *max_din = floor(max);
  }

  if((constraint->cycle_coeff > 0 && constraint->din_coeff >= 0 && constraint->sin_coeff >= 0) ||
     (constraint->cycle_coeff < 0 && constraint->din_coeff <= 0 && constraint->sin_coeff <= 0)) {
    index = constraint->cycle_coeff > 0 ? 1 : 0;
    max = constraint->constant[index] / constraint->cycle_coeff;
    *max_cycle = floor(max);
  }
}

LinearConstraint_t *parse_constraint(char *str)
{
  char *orig_str = str;
  char *ptr;
  LinearConstraint_t *constraint;
  double coeff;
  double sign = 1;
  gboolean sin, din, cycle, star_seen;

  sin = din = cycle = FALSE;
  constraint = g_new0(LinearConstraint_t, 1);

  /* Grab lower bound constant */
  constraint->constant[LOWER_BOUND_INDEX] = strtod(str, &ptr);
  if(ptr == str) g_error("Lower bound constant missing in constraint %s", 
			 orig_str);
  str = ptr;
  
  /* Eat the less than sign */
  while(isspace(*str)) str++;
  if(*str != '<') {
    g_error("Expecting < in constraint %s", orig_str);
  }
  str++;

  /* Swallow all the terms */
  do {
    coeff = sign * strtod(str, &ptr);
    if(ptr == str) coeff = sign;
    str = ptr;

    star_seen = FALSE;
    while(isspace(*str) || (!star_seen && *str == '*')) {
      if(*str == '*') star_seen = TRUE;
      str++;
    }
    if(!g_ascii_strncasecmp(str, "SIN", 3)) {
      if(sin)
	g_error("Constraint %s refers to SIN more than once", orig_str);
      sin = TRUE;
      constraint->sin_coeff = coeff;
      str += 3;
    } else if(!g_ascii_strncasecmp(str, "DIN", 3)) {
      if(din)
	g_error("Constraint %s refers to DIN more than once", str);
      din = TRUE;
      constraint->din_coeff = coeff;
      str += 3;
    } else if(!g_ascii_strncasecmp(str, "CYCLE", 5)) {
      if(cycle)
	g_error("Constraint %s refers to CYCLE more than once", str);
      cycle = TRUE;
      constraint->cycle_coeff = coeff;
      str += 5;
    } else {
      g_error("Unrecognized term starting \"%s\" in constraint %s", str,
	      orig_str);
    }

    sign = 1.0;
    /* Look for a + or - operator followed by a space.  
       Use these to set sign. */
    while(isspace(*str)) str++;
    if(*str == '+' && isspace(*(str+1))) { sign = 1.0; str += 2; }
    if(*str == '-' && isspace(*(str+1))) { sign = -1.0; str += 2; }
  } while(*str != '<');

  /* Eat the less than sign */
  str++;

  /* Grab lower bound constant */
  constraint->constant[UPPER_BOUND_INDEX] = strtod(str, &ptr);
  if(ptr == str) g_error("Upper bound constant missing in constraint %s", 
			 orig_str);
  str = ptr;
  
  return constraint;
}

void print_constraint(LinearConstraint_t *constraint)
{
  printf("%f < %f*DIN + %f*SIN + %f*CYCLE < %f\n",
	 constraint->constant[LOWER_BOUND_INDEX],
	 constraint->din_coeff,
	 constraint->sin_coeff,
	 constraint->cycle_coeff,
	 constraint->constant[UPPER_BOUND_INDEX]);
}

typedef struct {
  GArray *constraints;
  gboolean quiet_mode;
  SVTInvsreadyConfig sin_vs_ready;
  SVTInvsreadyConfig din_vs_ready;
} Config_t;

Config_t *parse_arguments(int argc, char *argv[], int *index)
{
  Config_t *config = g_new0(Config_t, 1);
  SVTInvsreadyConfig *invsready;
  int option;
  GArray *constraints = g_array_new(FALSE, FALSE, sizeof(LinearConstraint_t*));
  LinearConstraint_t *constraint;

  config->constraints = constraints;
  config->quiet_mode = FALSE;

  enum {
    OPTION_CONSTRAINT,
    OPTION_QUIET,
    OPTION_PLOTSINVSRDY,
    OPTION_PLOTDINVSRDY,
    OPTION_UNKNOWN
  };

  char *short_options = "qc:";
  struct option long_options[] = {
    { "constraint",   required_argument, NULL, OPTION_CONSTRAINT },
    { "quiet",        no_argument,       NULL, OPTION_QUIET },
    { "plotdinvsrdy", required_argument, NULL, OPTION_PLOTDINVSRDY },
    { "plotsinvsrdy", required_argument, NULL, OPTION_PLOTSINVSRDY },
  };

  while((option = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
    invsready = NULL;

    switch(option) {
    case 'c':
    case OPTION_CONSTRAINT:
      constraint = parse_constraint(optarg);
      g_array_append_val(constraints, constraint);
      break;
    case 'q':
    case OPTION_QUIET:
      config->quiet_mode = TRUE;
      break;
    case OPTION_PLOTDINVSRDY:
      invsready=&config->din_vs_ready;
    case OPTION_PLOTSINVSRDY:
      if(!invsready) {
	invsready=&config->sin_vs_ready;
      }
      invsready->doit=1;
      {
	gchar **args;
	int i;
	args=g_strsplit(optarg, ",", 0);
	for(i=0;args[i];i++); /* Count args */
	if((i < 3)  || (i > 5)) {
	  g_error("Must pass at least 3 arguments to" 
		  " --plot%cinvsrdy.  filename, ready_bin_size, din_bin_size\n"
		  "You may optionally pass a 4th maxopcount argument\n"
		  "Or a 4th minopcount argument and 5th maxopcount argument\n",
		  (option==OPTION_PLOTDINVSRDY)?'d':'s');
	}
	invsready->outfilename=
	  g_strdup(args[0]);
	invsready->ready_bin_size=
	  g_ascii_strtoull(args[1],NULL,0);
	invsready->din_bin_size=
	  g_ascii_strtoull(args[2],NULL,0);
	if(i == 4) {
	  invsready->maxopcount=
	    g_ascii_strtoull(args[3],NULL,0);
	} else if(i==5) {
	  invsready->minopcount=
	    g_ascii_strtoull(args[3],NULL,0);
	  invsready->maxopcount=
	    g_ascii_strtoull(args[4],NULL,0);
	  if(invsready->maxopcount) {
	    if(invsready->minopcount >
	       invsready->maxopcount) {
	      g_error("--plot%cvsrdy argument minopcount must be "
		      "<= maxopcount", (option==OPTION_PLOTDINVSRDY)?'d':'s');
	    } 
	  }
	}
	g_strfreev(args);
      }
      break;      
    case '?':
    case ':':
      g_error("Bad usage");
    }
  }

  if(optind == argc)
    g_error("Bad usage");

  *index = optind;
  return config;
}

int main(int argc, char *argv[])
{
  char *filename;
  int filename_index,i;
  LinearConstraint_t *constraint;
  Config_t *config;

  SVTInvsready sin_vs_ready, din_vs_ready;
  guint64 min_sin, min_din, min_cycle;
  guint64 max_sin, max_din, max_cycle;

  min_sin = min_din = min_cycle = 0;
  max_sin = max_din = max_cycle = G_MAXUINT64;

  SVTContext *ctxt;
  guint64 sin,din,cycle;

  /***********************/
  /* Parse our arguments */
  /***********************/
  config = parse_arguments(argc, argv, &filename_index);
  filename = argv[filename_index];
  if(config->sin_vs_ready.doit)
    svt_plot_sin_vs_ready_init(&config->sin_vs_ready, &sin_vs_ready);
  if(config->din_vs_ready.doit)
    svt_plot_din_vs_ready_init(&config->din_vs_ready, &din_vs_ready);


  /***********************/
  /* Print Header        */
  /***********************/
  printf("Schedule dump: %s\n", filename);
  printf("Constraints:\n");
  for(i = 0; i < config->constraints->len; i++) {
    guint64 tmp_min_sin, tmp_min_din, tmp_min_cycle;
    guint64 tmp_max_sin, tmp_max_din, tmp_max_cycle;
    
    constraint = g_array_index(config->constraints, LinearConstraint_t*, i);
    printf("\t");
    print_constraint(constraint);

    find_minimums(constraint, &tmp_min_din, &tmp_min_sin, &tmp_min_cycle);
    find_maximums(constraint, &tmp_max_din, &tmp_max_sin, &tmp_max_cycle);
    min_sin = MAX(min_sin, tmp_min_sin);
    min_din = MAX(min_din, tmp_min_din);
    min_cycle = MAX(min_cycle, tmp_min_cycle);
    max_sin = MIN(max_sin, tmp_max_sin);
    max_din = MIN(max_din, tmp_max_din);
    max_cycle = MIN(max_cycle, tmp_max_cycle);
  }
  printf("\n\n");

  if(config->din_vs_ready.doit) {
    din_vs_ready.cur_din_bin = min_din/config->din_vs_ready.din_bin_size;
    config->din_vs_ready.mincycle = min_cycle;
    config->din_vs_ready.maxcycle = max_cycle;
  }
  if(config->sin_vs_ready.doit) {
    config->sin_vs_ready.mincycle = min_cycle;
    config->sin_vs_ready.maxcycle = max_cycle;
  }

  /*************************/
  /* Iterate over schedule */
  /*************************/
  setbuf(stdout, NULL);
  ctxt = svt_context_read_create(filename);
  
  svt_sched_scan(ctxt, min_din);

  while(!svt_schedule_finished(ctxt)) {
    gboolean display = TRUE;
    
    svt_sched_read(ctxt, &din, &sin, &cycle);
    if(din > max_din) break;
    
    for(i = 0; i < config->constraints->len; i++) {
      constraint = g_array_index(config->constraints, LinearConstraint_t*, i);
      if(!evaluate_constraint(constraint, din, sin, cycle)) {
	display = FALSE;
	break;
      }
    }
    
    if(display) {
      if(!config->quiet_mode)
	printf("%12llu 0x%016llx %12llu\n", din, sin, cycle);

      if(config->sin_vs_ready.doit) {
	svt_plot_sin_vs_ready(din, sin, cycle, 
			      &sin_vs_ready,
			      &config->sin_vs_ready);
      }
      
      if(config->din_vs_ready.doit) {
	svt_plot_din_vs_ready(din, cycle, 
			      &din_vs_ready, 
			      &config->din_vs_ready);
      }
    }
  }

  if(config->sin_vs_ready.doit)
    svt_plot_sin_vs_ready_finalize(&sin_vs_ready);
  if(config->din_vs_ready.doit)
    svt_plot_din_vs_ready_finalize(&din_vs_ready);


  svt_context_read_destroy(ctxt);
  
  return 0;
}


/**CFile***********************************************************************

  FileName    [fsmPortBnet.c]

  PackageName [fsm]

  Synopsis    [Nanotrav parts used by pdtrav]

  Description [This package contains functions, types and constants taken from
    nanotrav to be used in pdtrav. The original names have been modified adding
    the "Fsm_Port" prefix.<p>
    FsmPortBnet.c contains the parts taken from bnet.c.]

  SeeAlso     [fsmPortNtr.c pdtrav]

  Author      [Gianpiero Cabodi, Stefano Quer]

  Copyright [  Copyright (c) 2001 by Politecnico di Torino.
    All Rights Reserved. This software is for educational purposes only.
    Permission is given to academic institutions to use, copy, and modify
    this software and its documentation provided that this introductory
    message is not removed, that this software and its documentation is
    used for the institutions' internal research and educational purposes,
    and that no monies are exchanged. No guarantee is expressed or implied
    by the distribution of this code.
    Send bug-reports and/or questions to: {cabodi,quer}@polito.it. ]

******************************************************************************/

#include "fsmInt.h"

#define MAXLENGTH 131072

static	char	BuffLine[MAXLENGTH];
static	char	*CurPos;

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static int fsmPortBnetSetLevel(FsmPortBnetNetwork_t * net);
static int fsmPortBnetLevelDFS(FsmPortBnetNetwork_t * net, FsmPortBnetNode_t * node);
static char * readString(FILE * fp);
static char ** readList(FILE * fp, int * n);
static void printList(char ** list, int n);
static int buildExorBDD(Ddi_Mgr_t * dd, FsmPortBnetNode_t * nd, st_table * hash, int params, int nodrop);
static int buildMuxBDD(Ddi_Mgr_t * dd, FsmPortBnetNode_t * nd, st_table * hash, int params, int nodrop);
static FsmPortBnetNode_t ** fsmPortBnetOrderRoots(FsmPortBnetNetwork_t * net, int * nroots);
static int fsmPortBnetDfsOrder(Ddi_Mgr_t * dd, FsmPortBnetNetwork_t * net, FsmPortBnetNode_t * node);
static int fsmPortBnetLevelCompare(FsmPortBnetNode_t ** x, FsmPortBnetNode_t ** y);

/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of exfsmPorted functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Reads boolean network from blif file.]

  Description [Reads a boolean network from a blif file. A very restricted
  subset of blif is supfsmPorted. Specifically:
  <ul>
  <li> The only directives recognized are:
    <ul>
    <li> .model
    <li> .inputs
    <li> .outputs
    <li> .latch
    <li> .names
    <li> .exdc
    <li> .wire_load_slope
    <li> .end
    </ul>
  <li> Latches must have an initial values and no other parameters
       specified.
  <li> Lines must not exceed MAXLENGTH-1 characters, and individual names must
       not exceed 1023 characters.
  </ul>
  Caveat emptor: There may be other limitations as well. One should
  check the syntax of the blif file with some other tool before relying
  on this parser. Fsm_PortBnetReadNetwork returns a pointer to the network if
  successful; NULL otherwise.
  ]

  SideEffects [None]

  SeeAlso     [Fsm_PortBnetPrintNetwork Fsm_PortBnetFreeNetwork]

******************************************************************************/
FsmPortBnetNetwork_t *
Fsm_PortBnetReadNetwork(
  FILE * fp /* pointer to the blif file */,
  int  pr /* verbosity level */)
{
    char *savestring;
    char **list;
    int i, j, n;
    FsmPortBnetNetwork_t *net;
    FsmPortBnetNode_t *newnode;
    FsmPortBnetNode_t *lastnode = NULL;
    FsmPortBnetTabline_t *newline;
    FsmPortBnetTabline_t *lastline;
    char ***latches = NULL;
    int maxlatches = 0;
    int exdc = 0;
    FsmPortBnetNode_t	*node;
    int	count;

    /* Allocate network object and initialize symbol table. */
    net = ALLOC(FsmPortBnetNetwork_t,1);
    if (net == NULL) goto failure;
    memset((char *) net, 0, sizeof(FsmPortBnetNetwork_t));
    net->hash = st_init_table(strcmp,st_strhash);
    if (net->hash == NULL) goto failure;

    savestring = readString(fp);
    if (savestring == NULL) goto failure;
    net->nlatches = 0;
    while (strcmp(savestring, ".model") == 0 ||
	strcmp(savestring, ".inputs") == 0 ||
	strcmp(savestring, ".outputs") == 0 ||
	strcmp(savestring, ".latch") == 0 ||
	strcmp(savestring, ".wire_load_slope") == 0 ||
	strcmp(savestring, ".exdc") == 0 ||
	strcmp(savestring, ".names") == 0 || strcmp(savestring,".end") == 0) {
	if (strcmp(savestring, ".model") == 0) {
	    /* Read .model directive. */
	    FREE(savestring);
	    /* Read network name. */
	    savestring = readString(fp);
	    if (savestring == NULL) goto failure;
	    net->name = savestring;
	} else if (strcmp(savestring, ".inputs") == 0) {
	    /* Read .inputs directive. */
	    FREE(savestring);
	    /* Read input names. */
	    list = readList(fp,&n);
	    if (list == NULL) goto failure;
	    if (pr > 2) printList(list,n);
	    /* Expect at least one input. */
	    if (n < 1) {
		(void) fprintf(stdout,"Empty input list.\n");
		goto failure;
	    }
	    if (exdc) {
	    	for (i = 0; i < n; i++)
		    FREE(list[i]);
		FREE(list);
		savestring = readString(fp);
		if (savestring == NULL) goto failure;
		continue;
	    }
	    if (net->ninputs) {
		net->inputs = REALLOC(char *, net->inputs,
		    (net->ninputs + n) * sizeof(char *));
	    	for (i = 0; i < n; i++)
		    net->inputs[net->ninputs + i] = list[i];
	    }
	    else
		net->inputs = list;
	    /* Create a node for each primary input. */
	    for (i = 0; i < n; i++) {
		newnode = ALLOC(FsmPortBnetNode_t,1);
		memset((char *) newnode, 0, sizeof(FsmPortBnetNode_t));
		if (newnode == NULL) goto failure;
		newnode->name = list[i];
		newnode->inputs = NULL;
		newnode->type = PORT_BNETINPUT_NODE;
		newnode->active = FALSE;
		newnode->nfo = 0;
		newnode->ninp = 0;
		newnode->f = NULL;
		newnode->polarity = 0;
		newnode->dd = NULL;
		newnode->next = NULL;
		if (lastnode == NULL) {
		    net->nodes = newnode;
		} else {
		    lastnode->next = newnode;
		}
		lastnode = newnode;
	    }
	    net->npis += n;
	    net->ninputs += n;
	} else if (strcmp(savestring, ".outputs") == 0) {
	    /* Read .outputs directive. We do not create nodes for the primary
	    ** outputs, because the nodes will be created when the same names
	    ** appear as outputs of some gates.
	    */
	    FREE(savestring);
	    /* Read output names. */
	    list = readList(fp,&n);
	    if (list == NULL) goto failure;
	    if (pr > 2) printList(list,n);
	    if (n < 1) {
		(void) fprintf(stdout,"Empty .outputs list.\n");
		goto failure;
	    }
	    if (exdc) {
	    	for (i = 0; i < n; i++)
		    FREE(list[i]);
		FREE(list);
		savestring = readString(fp);
		if (savestring == NULL) goto failure;
		continue;
	    }
	    if (net->noutputs) {
		net->outputs = REALLOC(char *, net->outputs,
		    (net->noutputs + n) * sizeof(char *));
	    	for (i = 0; i < n; i++)
		    net->outputs[net->noutputs + i] = list[i];
	    } else {
		net->outputs = list;
	    }
	    net->npos += n;
	    net->noutputs += n;
	} else if (strcmp(savestring,".wire_load_slope") == 0) {
	    FREE(savestring);
	    savestring = readString(fp);
	    net->slope = savestring;
	} else if (strcmp(savestring,".latch") == 0) {
	    FREE(savestring);
	    newnode = ALLOC(FsmPortBnetNode_t,1);
	    if (newnode == NULL) goto failure;
	    memset((char *) newnode, 0, sizeof(FsmPortBnetNode_t));
	    newnode->type = PORT_BNETPRESENT_STATE_NODE;
	    list = readList(fp,&n);
	    if (list == NULL) goto failure;
	    if (pr > 2) printList(list,n);
	    /* Expect three names. */
	    if (n != 3) {
		(void) fprintf(stdout,
			       ".latch not followed by three tokens.\n");
		goto failure;
	    }
	    newnode->name = list[1];
	    newnode->inputs = NULL;
	    newnode->ninp = 0;
	    newnode->f = NULL;
	    newnode->active = FALSE;
	    newnode->nfo = 0;
	    newnode->polarity = 0;
	    newnode->dd = NULL;
	    newnode->next = NULL;
	    if (lastnode == NULL) {
		net->nodes = newnode;
	    } else {
		lastnode->next = newnode;
	    }
	    lastnode = newnode;
	    /* Add next state variable to list. */
	    if (maxlatches == 0) {
		maxlatches = 20;
		latches = ALLOC(char **,maxlatches);
	    } else if (maxlatches <= net->nlatches) {
		maxlatches += 20;
		latches = REALLOC(char **,latches,maxlatches);
	    }
	    latches[net->nlatches] = list;
	    net->nlatches++;
	    savestring = readString(fp);
	    if (savestring == NULL) goto failure;
	} else if (strcmp(savestring,".names") == 0) {
	    FREE(savestring);
	    newnode = ALLOC(FsmPortBnetNode_t,1);
	    memset((char *) newnode, 0, sizeof(FsmPortBnetNode_t));
	    if (newnode == NULL) goto failure;
	    list = readList(fp,&n);
	    if (list == NULL) goto failure;
	    if (pr > 2) printList(list,n);
	    /* Expect at least one name (the node output). */
	    if (n < 1) {
		(void) fprintf(stdout,"Missing output name.\n");
		goto failure;
	    }
	    newnode->name = list[n-1];
	    newnode->inputs = list;
	    newnode->ninp = n-1;
	    newnode->active = FALSE;
	    newnode->nfo = 0;
	    newnode->polarity = 0;
	    if (newnode->ninp > 0) {
		newnode->type = PORT_BNETINTERNAL_NODE;
		for (i = 0; i < net->noutputs; i++) {
		    if (strcmp(net->outputs[i], newnode->name) == 0) {
			newnode->type = PORT_BNETOUTPUT_NODE;
			break;
		    }
		}
	    } else {
		newnode->type = PORT_BNETCONSTANT_NODE;
	    }
	    newnode->dd = NULL;
	    newnode->next = NULL;
	    if (lastnode == NULL) {
		net->nodes = newnode;
	    } else {
		lastnode->next = newnode;
	    }
	    lastnode = newnode;
	    /* Read node function. */
	    newnode->f = NULL;
	    if (exdc) {
		newnode->exdc_flag = 1;
		node = net->nodes;
		while (node) {
		    if (node->type == PORT_BNETOUTPUT_NODE &&
			strcmp(node->name, newnode->name) == 0) {
			node->exdc = newnode;
			break;
		    }
		    node = node->next;
		}
	    }
	    savestring = readString(fp);
	    if (savestring == NULL) goto failure;
	    lastline = NULL;
	    while (savestring[0] != '.') {
		/* Reading a table line. */
		newline = ALLOC(FsmPortBnetTabline_t,1);
		if (newline == NULL) goto failure;
		newline->next = NULL;
		if (lastline == NULL) {
		    newnode->f = newline;
		} else {
		    lastline->next = newline;
		}
		lastline = newline;
		if (newnode->type == PORT_BNETINTERNAL_NODE ||
		    newnode->type == PORT_BNETOUTPUT_NODE) {
		    newline->values = savestring;
		    /* Read output 1 or 0. */
		    savestring = readString(fp);
		    if (savestring == NULL) goto failure;
		} else {
		    newline->values = NULL;
		}
		if (savestring[0] == '0') newnode->polarity = 1;
		FREE(savestring);
		savestring = readString(fp);
		if (savestring == NULL) goto failure;
	    }
	} else if (strcmp(savestring,".exdc") == 0) {
	    FREE(savestring);
	    exdc = 1;
	} else if (strcmp(savestring,".end") == 0) {
	    FREE(savestring);
	    break;
	}
	if ((!savestring) || savestring[0] != '.')
	    savestring = readString(fp);
	if (savestring == NULL) goto failure;
    }
    if (latches) {
	net->latches = latches;

	count = 0;
	net->outputs = REALLOC(char *, net->outputs,
	    (net->noutputs + net->nlatches) * sizeof(char *));
	for (i = 0; i < net->nlatches; i++) {
	    for (j = 0; j < net->noutputs; j++) {
		if (strcmp(latches[i][0], net->outputs[j]) == 0)
		    break;
	    }
	    if (j < net->noutputs)
		continue;
	    savestring = ALLOC(char, strlen(latches[i][0]) + 1);
	    strcpy(savestring, latches[i][0]);
	    net->outputs[net->noutputs + count] = savestring;
	    count++;
	    node = net->nodes;
	    while (node) {
		if (node->type == PORT_BNETINTERNAL_NODE &&
		    strcmp(node->name, savestring) == 0) {
		    node->type = PORT_BNETOUTPUT_NODE;
		    break;
		}
		node = node->next;
	    }
	}
	net->noutputs += count;

	net->inputs = REALLOC(char *, net->inputs,
	    (net->ninputs + net->nlatches) * sizeof(char *));
	for (i = 0; i < net->nlatches; i++) {
	    savestring = ALLOC(char, strlen(latches[i][1]) + 1);
	    strcpy(savestring, latches[i][1]);
	    net->inputs[net->ninputs + i] = savestring;
	}
	net->ninputs += net->nlatches;
    }

    /* Put nodes in symbol table. */
    newnode = net->nodes;
    while (newnode != NULL) {
	if (pr > 2) printf("Inserting %s\n",newnode->name);
	if (st_insert(net->hash,newnode->name,(char *) newnode) ==
	    ST_OUT_OF_MEM) {
	    goto failure;
	}
	newnode = newnode->next;
    }

    /* Compute fanout counts. For each node in the linked list, fetch
    ** all its fanins using the symbol table, and increment the fanout of
    ** each fanin.
    */
    newnode = net->nodes;
    while (newnode != NULL) {
	FsmPortBnetNode_t *auxnd;
	for (i = 0; i < newnode->ninp; i++) {
	    if (!st_lookup(net->hash,newnode->inputs[i],(char **) &auxnd)) {
		(void) fprintf(stdout,"%s not driven\n", newnode->inputs[i]);
		goto failure;
	    }
	    auxnd->nfo++;
	}
	newnode = newnode->next;
    }

    if (!fsmPortBnetSetLevel(net)) goto failure;

    return(net);

failure:
    /* Here we should clean up the mess. */
    (void) fprintf(stdout,"Error in reading network from file.\n");
    return(NULL);

} /* end of Fsm_PortBnetReadNetwork */

/**Function********************************************************************

  Synopsis    [Builds the BDD for the function of a node.]

  Description [Builds the BDD for the function of a node and stores a
  pointer to it in the dd field of the node itself. The reference count
  of the BDD is incremented. If params is PORT_BNETLOCAL_DD, then the BDD is
  built in terms of the local inputs to the node; otherwise, if params
  is PORT_BNETGLOBAL_DD, the BDD is built in terms of the network primary
  inputs. To build the global BDD of a node, the BDDs for its local
  inputs must exist. If that is not the case, Fsm_PortBnetBuildNodeBDD
  recursively builds them. Likewise, to create the local BDD for a node,
  the local inputs must have variables assigned to them. If that is not
  the case, Fsm_PortBnetBuildNodeBDD recursively assigns variables to nodes.
  Fsm_PortBnetBuildNodeBDD returns 1 in case of success; 0 otherwise.]

  SideEffects [Sets the dd field of the node.]

  SeeAlso     []

******************************************************************************/
int
Fsm_PortBnetBuildNodeBDD(
  Ddi_Mgr_t *dd /* DD manager */,
  FsmPortBnetNode_t * nd /* node of the boolean network */,
  st_table * hash /* symbol table of the boolean network */,
  int  params /* type of DD to be built */,
  int  nodrop /* retain the intermediate node DDs until the end */)
{
    FsmPortBnetNode_t *auxnd;
    Ddi_Bdd_t *prod, *var;
    FsmPortBnetTabline_t *line;
    int i;

    if (nd->dd != NULL) return(1);

    if (nd->type == PORT_BNETCONSTANT_NODE) {
      if (nd->f == NULL) { /* constant 0 */
	nd->dd = Ddi_BddMakeConst(dd,0);
      } 
      else { /* either constant depending on the polarity */
	nd->dd = Ddi_BddMakeConst(dd,1);
      }
    } else if (nd->type == PORT_BNETINPUT_NODE ||
	       nd->type == PORT_BNETPRESENT_STATE_NODE) {
       if (nd->active != TRUE) { 
          /* a variable is NOT already associated: use it */
         nd->var = Ddi_VarNew(dd);
         nd->active = TRUE;
       } 
       nd->dd = Ddi_BddMakeLiteral(nd->var,1);
    } else if (buildExorBDD(dd,nd,hash,params,nodrop)) {
        /* Do nothing */
    } else if (buildMuxBDD(dd,nd,hash,params,nodrop)) {
        /* Do nothing */
    } else { /* type == PORT_BNETINTERNAL_NODE or PORT_BNETOUTPUT_NODE */
      /* Initialize the sum to logical 0. */
      nd->dd = Ddi_BddMakeConst(dd,0);

      /* Build a term for each line of the table and add it to the
      ** accumulator (func).
      */
      line = nd->f;
      while (line != NULL) {
#ifdef PORT_BNETDEBUG
	(void) fprintf(stdout,"line = %s\n", line->values);
#endif
	/* Initialize the product to logical 1. */
        prod = Ddi_BddMakeConst(dd,1);
	/* Scan the table line. */
	for (i = 0; i < nd->ninp; i++) {
	  if (line->values[i] == '-') continue;
	  if (!st_lookup(hash,(char *) nd->inputs[i], (char **) &auxnd)) {
	      goto failure;
	  }
	  if (params == PORT_BNETLOCAL_DD) {
	    if (auxnd->active == FALSE) {
	    	if (!Fsm_PortBnetBuildNodeBDD(dd,auxnd,hash,params,nodrop)) {
	    	    goto failure;
	    	}
	    }
	    var = Ddi_BddMakeLiteral(auxnd->var,1);
	    if (line->values[i] == '0') {
	      Ddi_BddNotAcc(var);
	    }
	  } else { /* params == PORT_BNETGLOBAL_DD */
	    if (auxnd->dd == NULL) {
	    	if (!Fsm_PortBnetBuildNodeBDD(dd,auxnd,hash,params,nodrop)) {
	    	    goto failure;
	    	}
	    }
	    if (line->values[i] == '1') {
	    	var = Ddi_BddDup(auxnd->dd);
	    } else { /* line->values[i] == '0' */
	    	var = Ddi_BddNot(auxnd->dd);
	    }
	  }
	  Ddi_BddAndAcc(prod,var);
          Ddi_Free(var);
	}
        Ddi_BddOrAcc(nd->dd,prod);
        Ddi_Free(prod);
	line = line->next;
      }
      /* Associate a variable to this node if local BDDs are being
      ** built. This is done at the end, so that the primary inputs tend
      ** to get lower indices.
      */
      if (params == PORT_BNETLOCAL_DD && nd->active == FALSE) {
          nd->var = Ddi_VarNew(dd);
          nd->active = TRUE;
      }
    }
    if (nd->polarity == 1) {
      Ddi_BddNotAcc(nd->dd);
    }

    if (params == PORT_BNETGLOBAL_DD && nodrop == FALSE) {
	/* Decrease counters for all faninis.
	** When count reaches 0, the DD is freed.
	*/
	for (i = 0; i < nd->ninp; i++) {
	  if (!st_lookup(hash,(char *) nd->inputs[i], (char **) &auxnd)) {
	    goto failure;
	  }
	  auxnd->count--;
	  if (auxnd->count == 0) {
	    Ddi_Free(auxnd->dd);
          }
	}
    }
    return(1);

failure:
    /* Here we should clean up the mess. */
    return(0);

} /* end of Fsm_PortBnetBuildNodeBDD */


/**Function********************************************************************

  Synopsis    [Orders the BDD variables by DFS.]

  Description [Orders the BDD variables by DFS.  Returns 1 in case of
  success; 0 otherwise.]

  SideEffects [Uses the visited flags of the nodes.]

  SeeAlso     []

******************************************************************************/
int
Fsm_PortBnetDfsVariableOrder(
  Ddi_Mgr_t * dd,
  FsmPortBnetNetwork_t * net)
{
    FsmPortBnetNode_t **roots;
    FsmPortBnetNode_t *node;
    int nroots;
    int i;

    roots = fsmPortBnetOrderRoots(net,&nroots);
    if (roots == NULL) return(0);
    for (i = 0; i < nroots; i++) {
	if (!fsmPortBnetDfsOrder(dd,net,roots[i])) {
	    FREE(roots);
	    return(0);
	}
    }
    /* Clear visited flags. */
    node = net->nodes;
    while (node != NULL) {
	node->visited = 0;
	node = node->next;
    }
    FREE(roots);
    return(1);

} /* end of Fsm_PortBnetDfsVariableOrder */

/**Function********************************************************************

  Synopsis    [Reads the variable order from a file.]

  Description [Reads the variable order from a file.
    Returns 1 if successful; 0 otherwise.]

  SideEffects [The BDDs for the primary inputs and present state variables
    are built.
    ]

  SeeAlso     []

*****************************************************************************/

int
Fsm_PortBnetReadOrder(
  Ddi_Mgr_t *dd,
  char *ordFile,
  FsmPortBnetNetwork_t *net,
  int locGlob,
  int nodrop,
  Pdtutil_VariableOrderFormat_e ordFileFormat
  )
{
  st_table *dict;
  int i, result, *varauxids, nvars;
  FsmPortBnetNode_t *node;
  char **varnames;

  dict = st_init_table(strcmp,st_strhash);
  if (dict == NULL) {
    return(0);
  }

  /*
   *  StQ 01.04.1999
   *  Patch to deal with nanotrav+vis+pdtrav variable order files
   *  Call Pdtutil_OrdRead to read the file
   */

  nvars = Pdtutil_OrdRead (&varnames, &varauxids, ordFile, NULL,
    ordFileFormat);

  if (nvars < 0) {
    Pdtutil_Warning (1, "Error reading variable ordering.");
    st_free_table (dict);
    return (0);
  }

  for (i=0; i<nvars; i++) {
    if (strstr (varnames[i], "$NS") != NULL) {
      continue;
    }

    if (strlen (varnames[i]) > MAXLENGTH) {
      st_free_table (dict);
      return(0);
    }

    /* There should be a node named "name" in the network. */
    if (!st_lookup(net->hash, varnames[i], (char **) &node)) {
      fprintf(stderr,"Unknown name in order file (%s)\n", varnames[i]);
      st_free_table(dict);
      return(0);
    }

    /* A name should not appear more than once in the order. */
    if (st_is_member(dict, varnames[i])) {
      fprintf(stderr,"Duplicate name in order file (%s)\n", varnames[i]);
      st_free_table(dict);
      return(0);
    }

    /* The name should correspond to a primary input or present state. */
    if (node->type != PORT_BNETINPUT_NODE &&
       node->type != PORT_BNETPRESENT_STATE_NODE) {
       fprintf(stderr,"%s has the wrong type (%d)\n", varnames[i],
	 node->type);
       st_free_table(dict);
       return(0);
    }

    /* Insert in table. Use node->name rather than name, because the
     ** latter gets overwritten.
     */
    if (st_insert (dict, node->name, NULL) == ST_OUT_OF_MEM) {
      (void) fprintf(stderr,"Out of memory in Fsm_PortBnetReadOrder\n");
      st_free_table(dict);
      return(0);
    }

    result = Fsm_PortBnetBuildNodeBDD(dd,node,net->hash,locGlob,nodrop);
    if (result == 0) {
      (void) fprintf(stderr,"Construction of BDD failed\n");
      st_free_table(dict);
      return(0);
      }
    }

    /* The number of names in the order file should match exactly the
    ** number of primary inputs and present states.
    */
    if (st_count(dict) != net->ninputs) {
	(void) fprintf(stderr,"Order incomplete: %d names instead of %d\n",
		       st_count(dict), net->ninputs);
	st_free_table(dict);
	return(0);
    }

    st_free_table(dict);
    return(1);

} /* end of Fsm_PortBnetReadOrder */


/**Function********************************************************************

  Synopsis    [Sets the level of each node.]

  Description [Sets the level of each node. Returns 1 if successful; 0
  otherwise.]

  SideEffects [Changes the level and visited fields of the nodes it
  visits.]

  SeeAlso     [fsmPortBnetLevelDFS]

******************************************************************************/
static int
fsmPortBnetSetLevel(
  FsmPortBnetNetwork_t * net)
{
    FsmPortBnetNode_t *node;

    /* Recursively visit nodes. This is pretty inefficient, because we
    ** visit all nodes in this loop, and most of them in the recursive
    ** calls to fsmPortBnetLevelDFS. However, this approach guarantees that
    ** all nodes will be reached ven if there are dangling outputs. */
    node = net->nodes;
    while (node != NULL) {
	if (!fsmPortBnetLevelDFS(net,node)) return(0);
	node = node->next;
    }

    /* Clear visited flags. */
    node = net->nodes;
    while (node != NULL) {
	node->visited = 0;
	node = node->next;
    }
    return(1);

} /* end of fsmPortBnetSetLevel */

/**Function********************************************************************

  Synopsis    [Does a DFS from a node setting the level field.]

  Description [Does a DFS from a node setting the level field. Returns
  1 if successful; 0 otherwise.]

  SideEffects [Changes the level and visited fields of the nodes it
  visits.]

  SeeAlso     [fsmPortBnetSetLevel]

******************************************************************************/
static int
fsmPortBnetLevelDFS(
  FsmPortBnetNetwork_t * net,
  FsmPortBnetNode_t * node)
{
    int i;
    FsmPortBnetNode_t *auxnd;

    if (node->visited == 1) {
	return(1);
    }

    node->visited = 1;

    /* Graphical sources have level 0.  This is the final value if the
    ** node has no fan-ins. Otherwise the successive loop will
    ** increase the level. */
    node->level = 0;
    for (i = 0; i < node->ninp; i++) {
	if (!st_lookup(net->hash,(char *) node->inputs[i], (char **) &auxnd)) {
	    return(0);
	}
	if (!fsmPortBnetLevelDFS(net,auxnd)) {
	    return(0);
	}
	if (auxnd->level >= node->level) node->level = 1 + auxnd->level;
    }
    return(1);

} /* end of fsmPortBnetLevelDFS */


/**Function********************************************************************

  Synopsis    [Frees a boolean network created by Fsm_PortBnetReadNetwork.]

  Description []

  SideEffects [None]

  SeeAlso     [Fsm_PortBnetReadNetwork]

******************************************************************************/
void
Fsm_PortBnetFreeNetwork(
  FsmPortBnetNetwork_t * net)
{
    FsmPortBnetNode_t *node, *nextnode;
    FsmPortBnetTabline_t *line, *nextline;
    int i;

    FREE(net->name);
    /* The input name strings are already pointed by the input nodes.
    ** Here we only need to free the latch names and the array that
    ** points to them.
    */
    for (i = 0; i < net->nlatches; i++) {
	FREE(net->inputs[net->npis + i]);
    }
    FREE(net->inputs);
    /* Free the output name strings and then the array pointing to them.  */
    for (i = 0; i < net->noutputs; i++) {
	FREE(net->outputs[i]);
    }
    FREE(net->outputs);

    for (i = 0; i < net->nlatches; i++) {
	FREE(net->latches[i][0]);
	FREE(net->latches[i][1]);
	FREE(net->latches[i][2]);
	FREE(net->latches[i]);
    }
    if (net->nlatches) FREE(net->latches);
    node = net->nodes;
    while (node != NULL) {
	nextnode = node->next;
	if (node->type != PORT_BNETPRESENT_STATE_NODE)
	    FREE(node->name);
	for (i = 0; i < node->ninp; i++) {
	    FREE(node->inputs[i]);
	}
	if (node->inputs != NULL) {
	    FREE(node->inputs);
	}
	/* Free the function table. */
	line = node->f;
	while (line != NULL) {
	    nextline = line->next;
	    FREE(line->values);
	    FREE(line);
	    line = nextline;
	}
	FREE(node);
	node = nextnode;
    }
    st_free_table(net->hash);
    if (net->slope != NULL) FREE(net->slope);
    FREE(net);

} /* end of Fsm_PortBnetFreeNetwork */


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Reads a string from a file.]

  Description [Reads a string from a file. The string can be MAXLENGTH-1
  characters at most. readString allocates memory to hold the string and
  returns a pointer to the result if successful. It returns NULL
  otherwise.]

  SideEffects [None]

  SeeAlso     [readList]

******************************************************************************/
static char *
readString(
  FILE * fp /* pointer to the file from which the string is read */)
{
    char *savestring;
    int length;

    while (!CurPos) {
	if (!fgets(BuffLine, MAXLENGTH, fp))
	    return(NULL);
	BuffLine[strlen(BuffLine) - 1] = '\0';
	CurPos = strtok(BuffLine, " \t");
	if (CurPos && CurPos[0] == '#') CurPos = (char *)NULL;
    }
    length = strlen(CurPos);
    savestring = ALLOC(char,length+1);
    if (savestring == NULL)
	return(NULL);
    strcpy(savestring,CurPos);
    CurPos = strtok(NULL, " \t");
    return(savestring);

} /* end of readString */


/**Function********************************************************************

  Synopsis    [Reads a list of strings from a file.]

  Description [Reads a list of strings from a line of a file.
  The strings are sequences of characters separated by spaces or tabs.
  The total length of the list, white space included, must not exceed
  MAXLENGTH-1 characters.  readList allocates memory for the strings and
  creates an array of pointers to the individual lists. Only two pieces
  of memory are allocated by readList: One to hold all the strings,
  and one to hold the pointers to them. Therefore, when freeing the
  memory allocated by readList, only the pointer to the list of
  pointers, and the pointer to the beginning of the first string should
  be freed. readList returns the pointer to the list of pointers if
  successful; NULL otherwise.]

  SideEffects [n is set to the number of strings in the list.]

  SeeAlso     [readString printList]

******************************************************************************/
static char **
readList(
  FILE * fp /* pointer to the file from which the list is read */,
  int * n /* on return, number of strings in the list */)
{
    char	*savestring;
    int		length;
    char	*stack[8192];
    char	**list;
    int		i, count = 0;

    while (CurPos) {
	if (strcmp(CurPos, "\\") == 0) {
	    CurPos = (char *)NULL;
	    while (!CurPos) {
		if (!fgets(BuffLine, MAXLENGTH, fp)) return(NULL);
		BuffLine[strlen(BuffLine) - 1] = '\0';
		CurPos = strtok(BuffLine, " \t");
	    }
	}
	length = strlen(CurPos);
	savestring = ALLOC(char,length+1);
	if (savestring == NULL) return(NULL);
	strcpy(savestring,CurPos);
	stack[count] = savestring;
	count++;
	CurPos = strtok(NULL, " \t");
    }
    list = ALLOC(char *, count);
    for (i = 0; i < count; i++)
	list[i] = stack[i];
    *n = count;
    return(list);

} /* end of readList */


/**Function********************************************************************

  Synopsis    [Prints a list of strings to the standard output.]

  Description [Prints a list of strings to the standard output. The list
  is in the format created by readList.]

  SideEffects [None]

  SeeAlso     [readList Fsm_PortBnetPrintNetwork]

******************************************************************************/
static void
printList(
  char ** list /* list of pointers to strings */,
  int  n /* length of the list */)
{
    int i;

    for (i = 0; i < n; i++) {
	(void) fprintf(stdout," %s",list[i]);
    }
    (void) fprintf(stdout,"\n");

} /* end of printList */



/**Function********************************************************************

  Synopsis    [Builds BDD for a XOR function.]

  Description [Checks whether a function is a XOR with 2 or 3 inputs. If so,
  it builds the BDD. Returns 1 if the BDD has been built; 0 otherwise.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static int
buildExorBDD(
  Ddi_Mgr_t * dd,
  FsmPortBnetNode_t * nd,
  st_table * hash,
  int  params,
  int  nodrop)
{
    int check[8];
    int i;
    int nlines;
    FsmPortBnetTabline_t *line;
    Ddi_Bdd_t *var;
    FsmPortBnetNode_t *auxnd;

    if (nd->ninp < 2 || nd->ninp > 3) return(0);

    nlines = 1 << (nd->ninp - 1);
    for (i = 0; i < 8; i++) check[i] = 0;
    line = nd->f;
    while (line != NULL) {
	int num = 0;
	int count = 0;
	nlines--;
	for (i = 0; i < nd->ninp; i++) {
	    num <<= 1;
	    if (line->values[i] == '-') {
		return(0);
	    } else if (line->values[i] == '1') {
		count++;
		num++;
	    }
	}
	if ((count & 1) == 0) return(0);
	if (check[num]) return(0);
	line = line->next;
    }
    if (nlines != 0) return(0);

    /* Initialize the exclusive sum to logical 0. */

    nd->dd = Ddi_BddMakeConst(dd,0);

    /* Scan the inputs. */
    for (i = 0; i < nd->ninp; i++) {
	if (!st_lookup(hash,(char *) nd->inputs[i], (char **) &auxnd)) {
	    goto failure;
	}
	if (params == PORT_BNETLOCAL_DD) {
	    if (auxnd->active == FALSE) {
		if (!Fsm_PortBnetBuildNodeBDD(dd,auxnd,hash,params,nodrop)) {
		    goto failure;
		}
	    }
	    var = Ddi_BddMakeLiteral(auxnd->var,1);
	} else { /* params == PORT_BNETGLOBAL_DD */
	    if (auxnd->dd == NULL) {
		if (!Fsm_PortBnetBuildNodeBDD(dd,auxnd,hash,params,nodrop)) {
		    goto failure;
		}
	    }
	    var = auxnd->dd;
	}
	Ddi_BddXorAcc(nd->dd,var);
	if (params == PORT_BNETLOCAL_DD) {
	  Ddi_Free(var);
	}
    }

    /* Associate a variable to this node if local BDDs are being
    ** built. This is done at the end, so that the primary inputs tend
    ** to get lower indices.
    */
    if (params == PORT_BNETLOCAL_DD && nd->active == FALSE) {
	nd->var = Ddi_VarNew(dd);
	nd->active = TRUE;
    }

    return(1);
failure:
    return(0);

} /* end of buildExorBDD */


/**Function********************************************************************

  Synopsis    [Builds BDD for a multiplexer.]

  Description [Checks whether a function is a 2-to-1 multiplexer. If so,
  it builds the BDD. Returns 1 if the BDD has been built; 0 otherwise.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static int
buildMuxBDD(
  Ddi_Mgr_t * dd,
  FsmPortBnetNode_t * nd,
  st_table * hash,
  int  params,
  int  nodrop)
{
    FsmPortBnetTabline_t *line;
    char *values[2];
    int mux[2];
    int phase[2];
    int j;
    int nlines = 0;
    int controlC = -1;
    int controlR = -1;
    Ddi_Bdd_t *f, *g, *h;
    FsmPortBnetNode_t *auxnd;

    if (nd->ninp != 3) return(0);

    for (line = nd->f; line != NULL; line = line->next) {
	int dc = 0;
	if (nlines > 1) return(0);
	values[nlines] = line->values;
	for (j = 0; j < 3; j++) {
	    if (values[nlines][j] == '-') {
		if (dc) return(0);
		dc = 1;
	    }
	}
	if (!dc) return(0);
	nlines++;
    }
    if (nlines==1) 
      return(0);
    /* At this point we know we have:
    **   3 inputs
    **   2 lines
    **   1 dash in each line
    ** If the two dashes are not in the same column, then there is
    ** exaclty one column without dashes: the control column.
    */
    for (j = 0; j < 3; j++) {
	if (values[0][j] == '-' && values[1][j] == '-') return(0);
	if (values[0][j] != '-' && values[1][j] != '-') {
	    if (values[0][j] == values[1][j]) return(0);
	    controlC = j;
	    controlR = values[0][j] == '0';
	}
    }
    assert(controlC != -1 && controlR != -1);
    /* At this point we know that there is indeed no column with two
    ** dashes. The control column has been identified, and we know that
    ** its two elelments are different. */
    for (j = 0; j < 3; j++) {
	if (j == controlC) continue;
	if (values[controlR][j] == '1') {
	    mux[0] = j;
	    phase[0] = 0;
	} else if (values[controlR][j] == '0') {
	    mux[0] = j;
	    phase[0] = 1;
	} else if (values[1-controlR][j] == '1') {
	    mux[1] = j;
	    phase[1] = 0;
	} else if (values[1-controlR][j] == '0') {
	    mux[1] = j;
	    phase[1] = 1;
	}
    }

    /* Get the inputs. */
    if (!st_lookup(hash,(char *) nd->inputs[controlC], (char **) &auxnd)) {
	goto failure;
    }
    if (params == PORT_BNETLOCAL_DD) {
	if (auxnd->active == FALSE) {
	    if (!Fsm_PortBnetBuildNodeBDD(dd,auxnd,hash,params,nodrop)) {
		goto failure;
	    }
	}
	f = Ddi_BddMakeLiteral(auxnd->var,1);
    } else { /* params == PORT_BNETGLOBAL_DD */
	if (auxnd->dd == NULL) {
	    if (!Fsm_PortBnetBuildNodeBDD(dd,auxnd,hash,params,nodrop)) {
		goto failure;
	    }
	}
	f = Ddi_BddDup(auxnd->dd);
    }
    if (!st_lookup(hash,(char *) nd->inputs[mux[0]], (char **) &auxnd)) {
	goto failure;
    }
    if (params == PORT_BNETLOCAL_DD) {
	if (auxnd->active == FALSE) {
	    if (!Fsm_PortBnetBuildNodeBDD(dd,auxnd,hash,params,nodrop)) {
		goto failure;
	    }
	}
	g = Ddi_BddMakeLiteral(auxnd->var,1);
    } else { /* params == PORT_BNETGLOBAL_DD */
	if (auxnd->dd == NULL) {
	    if (!Fsm_PortBnetBuildNodeBDD(dd,auxnd,hash,params,nodrop)) {
		goto failure;
	    }
	}
	g = Ddi_BddDup(auxnd->dd);
    }
    if (phase[0]) {
      Ddi_BddNotAcc(g);
    }
    if (!st_lookup(hash,(char *) nd->inputs[mux[1]], (char **) &auxnd)) {
	goto failure;
    }
    if (params == PORT_BNETLOCAL_DD) {
	if (auxnd->active == FALSE) {
	    if (!Fsm_PortBnetBuildNodeBDD(dd,auxnd,hash,params,nodrop)) {
		goto failure;
	    }
	}
	h = Ddi_BddMakeLiteral(auxnd->var,1);
    } else { /* params == PORT_BNETGLOBAL_DD */
	if (auxnd->dd == NULL) {
	    if (!Fsm_PortBnetBuildNodeBDD(dd,auxnd,hash,params,nodrop)) {
		goto failure;
	    }
	}
	h = Ddi_BddDup(auxnd->dd);
    }
    if (phase[1]) {
      Ddi_BddNotAcc(h);
    }
    nd->dd = Ddi_BddIte(f,g,h);

    Ddi_Free(f);
    Ddi_Free(g);
    Ddi_Free(h);

    /* Associate a variable to this node if local BDDs are being
    ** built. This is done at the end, so that the primary inputs tend
    ** to get lower indices.
    */
    if (params == PORT_BNETLOCAL_DD && nd->active == FALSE) {
	nd->var = Ddi_VarNew(dd);
	nd->active = TRUE;
    }

    return(1);
failure:
    return(0);

} /* end of buildExorBDD */

/**Function********************************************************************

  Synopsis    [Orders network roots for variable ordering.]

  Description [Orders network roots for variable ordering. Returns
  an array with the ordered outputs and next state variables if
  successful; NULL otherwise.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static FsmPortBnetNode_t **
fsmPortBnetOrderRoots(
  FsmPortBnetNetwork_t * net,
  int * nroots)
{
    int i, noutputs;
    FsmPortBnetNode_t *node;
    FsmPortBnetNode_t **nodes = NULL;

    /* Initialize data structures. */
    noutputs = net->noutputs;
    nodes = ALLOC(FsmPortBnetNode_t *, noutputs);
    if (nodes == NULL) goto endgame;

    /* Find output names and levels. */
    for (i = 0; i < net->nlatches; i++) {
	if (!st_lookup(net->hash,net->latches[i][0],(char **)&node)) {
	    goto endgame;
	}
	nodes[i] = node;
    }
    for (i = 0; i < net->npos; i++) {
	if (!st_lookup(net->hash,net->outputs[i],(char **)&node)) {
	    goto endgame;
	}
	nodes[i + net->nlatches] = node;
    }

    qsort((void *)nodes, noutputs, sizeof(FsmPortBnetNode_t *),
	  (int (*)(const void *, const void *))fsmPortBnetLevelCompare);
    *nroots = noutputs;
    return(nodes);

endgame:
    if (nodes != NULL) FREE(nodes);
    return(NULL);

} /* end of fsmPortBnetOrderRoots */

/**Function********************************************************************

  Synopsis    [Does a DFS from a node ordering the inputs.]

  Description [Does a DFS from a node ordering the inputs. Returns
  1 if successful; 0 otherwise.]

  SideEffects [Changes visited fields of the nodes it visits.]

  SeeAlso     [Fsm_PortBnetDfsVariableOrder]

******************************************************************************/
static int
fsmPortBnetDfsOrder(
  Ddi_Mgr_t * dd,
  FsmPortBnetNetwork_t * net,
  FsmPortBnetNode_t * node)
{
    int i;
    FsmPortBnetNode_t *auxnd;
    FsmPortBnetNode_t **fanins;

    if (node->visited == 1) {
	return(1);
    }

    node->visited = 1;
    if (node->type == PORT_BNETINPUT_NODE ||
	node->type == PORT_BNETPRESENT_STATE_NODE) {
       node->var = Ddi_VarNew(dd);
       node->active = TRUE;
       node->dd = Ddi_BddMakeLiteral(node->var,1);
       return(1);
    }

    fanins = ALLOC(FsmPortBnetNode_t *, node->ninp);
    if (fanins == NULL) return(0);

    for (i = 0; i < node->ninp; i++) {
	if (!st_lookup(net->hash,(char *) node->inputs[i], (char **) &auxnd)) {
	    FREE(fanins);
	    return(0);
	}
	fanins[i] = auxnd;
    }

    qsort((void *)fanins, node->ninp, sizeof(FsmPortBnetNode_t *),
	  (int (*)(const void *, const void *))fsmPortBnetLevelCompare);
    for (i = 0; i < node->ninp; i++) {
	/* for (i = node->ninp - 1; i >= 0; i--) { */
	int res = fsmPortBnetDfsOrder(dd,net,fanins[i]);
	if (res == 0) {
	    FREE(fanins);
	    return(0);
	}
    }
    FREE(fanins);
    return(1);

} /* end of fsmPortBnetLevelDFS */


/**Function********************************************************************

  Synopsis    [Comparison function used by qsort.]

  Description [Comparison function used by qsort to order the
  variables according to the number of keys in the subtables.
  Returns the difference in number of keys between the two
  variables being compared.]

  SideEffects [None]

******************************************************************************/
static int
fsmPortBnetLevelCompare(
  FsmPortBnetNode_t ** x,
  FsmPortBnetNode_t ** y)
{
    return((*y)->level - (*x)->level);

} /* end of fsmPortBnetLevelCompare */








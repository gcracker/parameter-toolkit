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

#ifdef __cplusplus
extern "C" {
#endif
#ifndef _TCGEN_SCHED_H_
#define _TCGEN_SCHED_H_

/*
TCgen V1.0
compiling

# This spec is intended to compress AdamantScheduledOpers
TCgen Trace Specification;
0-Bit Header;
64-Bit Field 1 = {L1 = 1, L2 = 131072: FCM3[2]}; # SIN
64-Bit Field 2 = {L1 = 1, L2 = 131072: DFCM3[2]}; # DIN
64-Bit Field 3 = {L1 = 65536, L2 = 131072: DFCM3[1]}; #time
PC = Field 1;

-----

TCgen Trace Specification;

0-Bit Header;

64-Bit Field 1 = {L1 = 1, L2 = 131072: FCM3[2]};
# Field 1 uses 2 predictors with a combined size of 8388620 bytes (8.0MB)

64-Bit Field 2 = {L1 = 1, L2 = 131072: DFCM3[2]};
# Field 2 uses 2 predictors with a combined size of 8388628 bytes (8.0MB)

64-Bit Field 3 = {L1 = 65536, L2 = 131072: DFCM3[1]};
# Field 3 uses 1 predictors with a combined size of 5505024 bytes (5.2MB)

PC = Field 1;

-----

Copyright � 2004 Cornell Research Foundation, Inc.  All rights reserved.
Author:  Professor Martin Burtscher
 
Software License Terms and Conditions

1. SOFTWARE shall mean the TCgen tool available on the web page
   http://www.csl.cornell.edu/~burtscher/research/TCgen/ and described in
   Cornell Research Foundation, Inc. (�CRF�) file D-3577.  SOFTWARE OUTPUT
   CODE shall mean any code generated by the SOFTWARE.  SOFTWARE OUTPUT CODE
   includes, but is not limited to, source code, object code and executable
   code.  SOFTWARE and SOFTWARE OUTPUT CODE shall collectively be referred to
   herein as TCgen SOFTWARE.

2. CRF is a wholly owned subsidiary of Cornell University, is a fiduciary of
   Cornell University in intellectual property matters and holds all
   intellectual property rights in TCgen SOFTWARE.  

3. LICENSEE means the party to this Agreement and the user of TCgen SOFTWARE.
   By using TCgen SOFTWARE, LICENSEE enters into this Agreement with CRF.  

4. TCgen SOFTWARE is made available under this Agreement to allow certain
   non-commercial research and teaching use.  CRF reserves all commercial
   rights to TCgen SOFTWARE and these rights may be licensed by CRF to third
   parties.  

5. LICENSEE is hereby granted permission to:  a) use SOFTWARE for
   non-commercial research or teaching purposes, and b) download, compile,
   execute, copy, and modify SOFTWARE OUTPUT CODE for non-commercial research
   or teaching purposes provided that this notice accompanies all copies of
   SOFTWARE OUTPUT CODE.  Copies of modified SOFTWARE OUTPUT CODE may be
   distributed only for non-commercial research or teaching purposes (i) if
   this notice accompanies those copies, (ii) if said copies carry prominent
   notices stating that SOFTWARE OUTPUT CODE has been changed, and (iii) the
   date of any changes are clearly identified in SOFTWARE OUTPUT CODE.  

6. CRF may terminate this Agreement at any time if LICENSEE breaches a
   material provision of this Agreement.  CRF may also terminate this
   Agreement if the TCgen SOFTWARE becomes subject to any claim of
   infringement of patent, copyright or trade secret, or if in CRF�S opinion
   such a claim is likely to occur.  

7. LICENSEE agrees that the export of TCgen SOFTWARE from the United States
   may require approval from the U.S. government and failure to obtain such
   approval will result in the immediate termination of this license and may
   result in criminal liability under U.S. laws.

8. The work leading to the development of SOFTWARE was supported in part by
   various grants from an agency of the U.S. Government, and CRF is obligated
   to comply with U.S. OMB Circular A-124 and 37 CFR Part 401.  This license
   is subject to the applicable terms of U.S. Government regulations
   concerning Government funded inventions.

9. CRF provides TCgen SOFTWARE on an �as is� basis.  CRF does not warrant,
   guarantee, or make any representations regarding the use or results of
   TCgen SOFTWARE with respect to its correctness, accuracy, reliability or
   performance.  The entire risk of the use and performance of TCgen
   SOFTWARE is assumed by LICENSEE.  ALL WARRANTIES INCLUDING, WITHOUT
   LIMITATION, ANY WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE OR
   MERCHANTABILITY AND ANY WARRANTY OF NONINFRINGEMENT OF PATENTS,
   COPYRIGHTS, OR ANY OTHER INTELLECTUAL PROPERTY RIGHT ARE HEREBY EXCLUDED.

10. LICENSEE understands and agrees that neither CRF nor Cornell University
    is under any obligation to provide maintenance, support or update
    services, notices of latent defects, correction of defects, or future
    versions for TCgen SOFTWARE.  

11. Even if advised of the possibility of damages, under no circumstances
    shall CRF or Cornell University individually or jointly be liable to
    LICENSEE or any third party for damages of any character, including,
    without limitation, direct, indirect, incidental, consequential or
    special damages, loss of profits, loss of use, loss of goodwill,
    computer failure or malfunction.  LICENSEE agrees to indemnify and
    hold harmless CRF and Cornell University for any and all liability CRF
    or Cornell University may incur as a result of use of TCgen SOFTWARE
    by LICENSEE. 
*/

#include <glib.h>

typedef guint64 F0type;
typedef guint64 F1type;
typedef guint64 F2type;
typedef guint64 PCtype;

void TCgen_sched_Init();
F0type TCgen_sched_F0Decode(const int code, F0type val);
F1type TCgen_sched_F1Decode(const int code, F1type val);
F2type TCgen_sched_F2Decode(const int code, const PCtype pc, F2type val);
unsigned char TCgen_sched_F0Encode(const F0type val);
unsigned char TCgen_sched_F1Encode(const F1type val);
unsigned char TCgen_sched_F2Encode(const PCtype pc, const F2type val);

#define TCgen_sched_decode_sin TCgen_sched_F0Decode
#define TCgen_sched_decode_din TCgen_sched_F1Decode
#define TCgen_sched_decode_cycle TCgen_sched_F2Decode

#define TCgen_sched_encode_sin TCgen_sched_F0Encode
#define TCgen_sched_encode_din TCgen_sched_F1Encode
#define TCgen_sched_encode_cycle TCgen_sched_F2Encode


#endif /* _TCGEN_SCHED_H_ */
#ifdef __cplusplus
}
#endif


/*
 * Copyright (c) 1998 Sandia Corporation. Under the terms of Contract
 * DE-AC04-94AL85000 with Sandia Corporation, the U.S. Governement
 * retains certain rights in this software.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 * 
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.  
 * 
 *     * Neither the name of Sandia Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/* Function(s) contained in this file:
 *
 *      ne_put_n_side_set()
 *
 *****************************************************************************
 *
 *  Variable Index:
 *
 *      neid               - The NetCDF ID of an already open NemesisI file.
 *      side_set_id        - ID of side set to read.
 *      start_side_num     - The starting index of the sides to be read.
 *      num_sides          - The number of sides to read in.
 *      side_set_elem_list - List of element IDs in side set.
 *      side_set_side_list - List of side IDs in side set.
 */
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "exodusII.h"
#include "exodusII_int.h"

#include "ne_nemesisI_int.h"
#include "ne_nemesisI.h"

/*
 * writes the side set element list and side set side list for a single side set
 */

int ne_put_n_side_set (int   neid,
                       int   side_set_id,
                       int   start_side_num,
                       int   num_sides,
                       int  *side_set_elem_list,
                       int  *side_set_side_list)
{
  int status;
  int dimid;
  int elem_list_id, side_list_id, side_set_id_ndx;
  size_t  num_side_in_set, start[1], count[1]; 
  char errmsg[MAX_ERR_LENGTH];

  exerrval = 0; /* clear error code */

  /* first check if any side sets are specified */

  if ((status = nc_inq_dimid (neid, DIM_NUM_SS, &dimid)) != NC_NOERR) {
    exerrval = status;
    sprintf(errmsg,
            "Error: no side sets defined in file id %d",
            neid);
    ex_err("ne_put_n_side_set",errmsg,exerrval);
    return (EX_FATAL);
  }

  /* Lookup index of side set id in VAR_SS_IDS array */

  if ((side_set_id_ndx = ex_id_lkup(neid, EX_SIDE_SET, side_set_id)) < 0)
  {
    if (exerrval == EX_NULLENTITY) {
      sprintf(errmsg,
              "Warning: no data allowed for NULL side set %d in file id %d",
              side_set_id, neid);
      ex_err("ne_put_n_side_set",errmsg,EX_MSG);
      return (EX_WARN);
    } else {
      sprintf(errmsg,
     "Error: failed to locate side set id %d in VAR_SS_IDS array in file id %d",
              side_set_id, neid);
      ex_err("ne_put_n_side_set",errmsg,exerrval);
      return (EX_FATAL);
    }
  }

  /* inquire id's of previously defined dimensions  */

  if ((status = nc_inq_dimid (neid, DIM_NUM_SIDE_SS(side_set_id_ndx), &dimid)) != NC_NOERR) {
    exerrval = status;
    sprintf(errmsg,
      "Error: failed to locate number of sides in side set %d in file id %d",
            side_set_id, neid);
    ex_err("ne_put_n_side_set",errmsg,exerrval);
    return (EX_FATAL);
  }

  if ((status = nc_inq_dimlen(neid, dimid, &num_side_in_set)) != NC_NOERR) {
    exerrval = status;
    sprintf(errmsg,
         "Error: failed to get number of sides in side set %d in file id %d",
            side_set_id, neid);
    ex_err("ne_put_n_side_set",errmsg,exerrval);
    return (EX_FATAL);
  }

  /* Check input parameters for a valid range of side numbers */
  if (start_side_num < 0 || start_side_num > num_side_in_set) {
    exerrval = EX_BADPARAM;
    sprintf(errmsg, "Error: Invalid input");
    ex_err("ne_put_n_side_set",errmsg,exerrval);
    return (EX_FATAL);
  }

  if (num_sides < 0) {
    exerrval = EX_BADPARAM;
    sprintf(errmsg, "Error: Invalid number of elements in side set!");
    ex_err("ne_put_n_side_set",errmsg,exerrval);
    return (EX_FATAL);
  }

  /* start_side_num now starts at 1, not 0 */
  if ((start_side_num + num_sides - 1) > num_side_in_set) {
    exerrval = EX_BADPARAM;
    sprintf(errmsg, "Error: request larger than number of elements in set!");
    ex_err("ne_put_n_side_set",errmsg,exerrval);
    return (EX_FATAL);
  }

  /* inquire id's of previously defined variables  */
  if ((status = nc_inq_varid (neid, VAR_ELEM_SS(side_set_id_ndx), &elem_list_id)) != NC_NOERR) {
    exerrval = status;
    sprintf(errmsg,
           "Error: failed to locate element list for side set %d in file id %d",
            side_set_id, neid);
    ex_err("ne_put_n_side_set",errmsg,exerrval);
    return (EX_FATAL);
  }

  if ((status = nc_inq_varid (neid, VAR_SIDE_SS(side_set_id_ndx), &side_list_id)) != NC_NOERR) {
    exerrval = status;
    sprintf(errmsg,
            "Error: failed to locate side list for side set %d in file id %d",
            side_set_id, neid);
    ex_err("ne_put_n_side_set",errmsg,exerrval);
    return (EX_FATAL);
  }


  /* write out the element list and side list arrays */
  start[0] = --start_side_num;
  count[0] = num_sides;

  status = nc_put_vara_int(neid, elem_list_id, start, count, side_set_elem_list);

  if (status != NC_NOERR) {
    exerrval = status;
    sprintf(errmsg,
      "Error: failed to store element list for side set %d in file id %d",
            side_set_id, neid);
    ex_err("ne_put_n_side_set",errmsg,exerrval);
    return (EX_FATAL);
  }


  status = nc_put_vara_int(neid, side_list_id, start, count, side_set_side_list);

  if (status != NC_NOERR) {
    exerrval = status;
    sprintf(errmsg,
      "Error: failed to store side list for side set %d in file id %d",
            side_set_id, neid);
    ex_err("ne_put_n_side_set",errmsg,exerrval);
    return (EX_FATAL);
  }
  return (EX_NOERR);
}

/* Cleanup KiCAD file for rendering */
/* (c) 2025Adrian Kennard Andrews & Arnold Ltd */

#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <popt.h>
#include <err.h>
#include <float.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <math.h>
#include "pcb.h"

int
main (int argc, const char *argv[])
{

   const char *infile = NULL;
   const char *outfile = NULL;
   int layercase = 0;
   poptContext optCon;          /* context for parsing  command - line options */
   {
      const struct poptOption optionsTable[] = {
         {"in-file", 'i', POPT_ARG_STRING, &infile, 0, "PCB input", "filename"},
         {"out-file", 'o', POPT_ARG_STRING, &outfile, 0, "PCB output", "filename"},
         {"case", 0, POPT_ARG_INT, &layercase, 0, "Use User.N as case border instead of pcb (error if missing)", "N"},
         POPT_AUTOHELP {}
      };

      optCon = poptGetContext (NULL, argc, argv, optionsTable, 0);
      /* poptSetOtherOptionHelp(optCon, ""); */

      int c;
      if ((c = poptGetNextOpt (optCon)) < -1)
         errx (1, "%s: %s\n", poptBadOption (optCon, POPT_BADOPTION_NOALIAS), poptStrerror (c));

      if (poptPeekArg (optCon) && !infile)
         infile = poptGetArg (optCon);

      if (poptPeekArg (optCon) && !outfile)
         outfile = poptGetArg (optCon);

      if (poptPeekArg (optCon) || !infile)
      {
         poptPrintUsage (optCon, stderr, 0);
         return -1;
      }
   }
   pcb_t *pcb = pcb_load (infile);
   if (strcmp (pcb->tag, "kicad_pcb"))
      errx (1, "Not a kicad_pcb (%s)", pcb->tag);

   int zap (const char *layer, const char *newlayer)
   {
      int found = 0;
      pcb_t *check (pcb_t * parent, const char *tag, pcb_t * o)
      {
         if (!o)
            o = pcb_find (parent, tag, o);      /* First */
         if (!o)
            return o;
         pcb_t *n = pcb_find (parent, tag, o);  /* Next */
         pcb_t *l = pcb_find (o, "layer", NULL);
         if (!l || l->valuen != 1 || !l->values[0].istxt || strcmp (l->values[0].txt, layer))
            return n;           /* Not found */
         found++;
         if (newlayer)
            l->values[0].txt = pcb_add_string (newlayer, NULL);
         else
            pcb_delete (o);
         return n;
      }
      pcb_t *o = NULL;
      while ((o = check (pcb, "gr_line", o)));
      while ((o = check (pcb, "gr_arc", o)));
      while ((o = check (pcb, "gr_circle", o)));
      pcb_t *fp = NULL;
      while ((fp = pcb_find (pcb, "footprint", fp)))
      {
         while ((o = check (fp, "fp_line", o)));
         while ((o = check (fp, "fp_arc", o)));
         while ((o = check (fp, "fp_circle", o)));
      }
      return found;
   }

   if (layercase)
   {                            /* Replace Edge.Cuts with User.N */
      char casework[20];
      sprintf (casework, "User.%d", layercase);
      zap ("Edge.Cuts", NULL);
      if (!zap (casework, "Edge.Cuts"))
         errx (1, "Edge not found");
   }
   //Other deletes to clean up render


   if (outfile)
      pcb_write (outfile, pcb);
   pcb = pcb_free (pcb);
   poptFreeContext (optCon);
   return 0;
}

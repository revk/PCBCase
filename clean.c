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
#include "iec18004.h"

int
main (int argc, const char *argv[])
{
   const char *infile = NULL;
   const char *outfile = NULL;
   const char *qr = NULL;
   int qrsize = 8;
   int layercase = 0;
   int eco1 = 0;
   poptContext optCon;          /* context for parsing  command - line options */
   {
      const struct poptOption optionsTable[] = {
         {"in-file", 'i', POPT_ARG_STRING, &infile, 0, "PCB input", "filename"},
         {"out-file", 'o', POPT_ARG_STRING, &outfile, 0, "PCB output", "filename"},
         {"case", 0, POPT_ARG_INT, &layercase, 0, "Use User.N as case border instead of pcb (error if missing)", "N"},
         {"qr", 'q', POPT_ARG_STRING, &qr, 0, "Replace rectangle with QR", "Code"},
         {"qr-size", 0, POPT_ARG_INT, &qrsize, 0, "QR code size", "mm"},
         {"eco-1", 0, POPT_ARG_NONE, &eco1, 0, "Keep Eco1 layer", NULL},
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
      if (qr)
      {
         while ((o = pcb_find (pcb, "gr_rect", o)))
         {
            pcb_t *o2;
            const char *layer = NULL;
            double sx = 0,
               sy = 0,
               ex = 0,
               ey = 0;
            if (!(o2 = pcb_find (o, "fill", NULL)) || o2->valuen != 1 || !o2->values[0].islit || strcmp (o2->values[0].txt, "yes"))
               continue;
            if (!(o2 = pcb_find (o, "stroke", NULL)) || !(o2 = pcb_find (o2, "width", NULL)) || o2->valuen != 1
                || !o2->values[0].isnum || o2->values[0].num)
               continue;
            if (!(o2 = pcb_find (o, "layer", NULL)) || o2->valuen != 1 || !o2->values[0].istxt
                || !strstr (o2->values[0].txt, "SilkS"))
               continue;
            layer = o2->values[0].txt;
            if (!(o2 = pcb_find (o, "start", NULL)) || o2->valuen != 2 || !o2->values[0].isnum || !o2->values[1].isnum)
               continue;
            sx = o2->values[0].num;
            sy = o2->values[1].num;
            if (!(o2 = pcb_find (o, "end", NULL)) || o2->valuen != 2 || !o2->values[0].isnum || !o2->values[1].isnum)
               continue;
            ex = o2->values[0].num;
            ey = o2->values[1].num;
            if (lround ((ex - sx) * 10) != qrsize * 10 || lround ((ey - sy) * 10) != qrsize * 10)
               continue;
            int w;
            int num = time (0) % 10000;;
            char tag[5];
            snprintf (tag, sizeof (tag), "%04u", num);
            char *val;
            asprintf (&val, "%s_%s", qr, tag);
          unsigned char *map = qr_encode (strlen (val), val, widthp: &w, noquiet:1);
            if (map)
            {
               double u = (double) qrsize / w;
               unsigned char *p = map;
               for (int y = 0; y < w; y++)
                  for (int x = 0; x < w; x++)
                     if (*(p++) & QR_TAG_BLACK)
                     {
                        pcb_t *e,
                         *r = pcb_append_obj (pcb, "gr_rect");
                        e = pcb_append_obj (r, "start");
                        pcb_append_num (e, *layer == 'B' ? ex - u * x : sx + u * x);
                        pcb_append_num (e, sy + u * y);
                        e = pcb_append_obj (r, "end");
                        pcb_append_num (e, *layer == 'B' ? ex - u * (x + 1) : sx + u * (x + 1));
                        pcb_append_num (e, sy + u * (y + 1));
                        e = pcb_append_obj (r, "fill");
                        pcb_append_lit (e, "yes");
                        e = pcb_append_obj (r, "layer");
                        pcb_append_txt (e, layer);
                     }
               pcb_t *e,
                *t = pcb_append_obj (pcb, "gr_text");
               pcb_append_txt (t, tag);
               e = pcb_append_obj (t, "at");
               pcb_append_num (e, (sx + ex) / 2);
               pcb_append_num (e, ey + 1.5);
               e = pcb_append_obj (t, "layer");
               pcb_append_txt (e, layer);
               if (*layer == 'B')
               {
                  e = pcb_append_obj (t, "effects");
                  e = pcb_append_obj (e, "justify");
                  pcb_append_lit (e, "mirror");
               }
               free (map);
            }
            free (val);
            pcb_delete (o);
            break;
         }
      }
      while ((o = check (pcb, "dimension", o)));
      while ((o = check (pcb, "gr_text", o)));
      while ((o = check (pcb, "gr_line", o)));
      while ((o = check (pcb, "gr_poly", o)));
      while ((o = check (pcb, "gr_rect", o)));
      while ((o = check (pcb, "gr_arc", o)));
      while ((o = check (pcb, "gr_circle", o)));
      pcb_t *fp = NULL;
      while ((fp = pcb_find (pcb, "footprint", fp)))
      {
         while ((o = check (fp, "dimension", o)));
         while ((o = check (fp, "property", o)));
         while ((o = check (fp, "fp_text", o)));
         while ((o = check (fp, "fp_line", o)));
         while ((o = check (fp, "fp_poly", o)));
         while ((o = check (fp, "fp_rect", o)));
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
   /* Clean up things that do not look good */
   zap ("Dwgs.User", NULL);
   zap ("Cmts.User", NULL);
   if (!eco1)
      zap ("Eco1.User", NULL);
   zap ("Eco2.User", NULL);
   zap ("F.Fab", NULL);
   zap ("B.Fab", NULL);
   if (outfile)
      pcb_write (outfile, pcb);
   pcb = pcb_free (pcb);
   poptFreeContext (optCon);
   return 0;
}

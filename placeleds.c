/* LED grid */

#include <stdio.h>
#include <string.h>
#include <popt.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <ctype.h>
#include <err.h>
#include <math.h>
#include "pcb.h"

int             debug = 0;

#define	delta 0.1               /* Matched position */

int
main(int argc, const char *argv[])
{
   const char     *pcbfile = NULL;
   int             diode = 1;
   int             cap = 0;
   int             rows = 0;
   int             cols = 0;
   int             count = 0;
   double          spacing = 0;
   double          trackwidth = 0.2;
   double          padoffsetx = 0.6010407;
   double          padoffsety = 0.6010407;
   double          padsize = 0.3181981; /* extra to diagonal on power */
   double          padin = 0.174224;    /* pad towards centre of LED */
   double          clearance = 0.127;
   double          capoffset = 1.15;
   double          zonei = NAN;
   double          zoneo = NAN;
   double          startx = NAN;
   double          starty = NAN;
   double          diameter = 0;
   double          radius = 0;
   double          angle = 0;
   double          rotate = -135;
   int             group = 0;
   int             sides = 0;
   int             tracks = 0;
   double          viaoffset = 0;
   double          vias = 0;
   double          powervias = 0;
   const char     *fill = NULL;
   const char     *layer = NULL;
   int             SMD1010 = 0;
   int             SMD1615 = 0;
   int             SMD2020 = 0;
   int             capbelow = 0;
   int             powertrack = 1;
   /* TODO reverse option and B.Cu logic */
   poptContext     optCon = NULL;
   {
      const struct poptOption optionsTable[] = {
         {"pcb-file", 0, POPT_ARG_STRING, &pcbfile, 0, "PCB file", "filename"},
         {"diode", 0, POPT_ARG_INT | POPT_ARGFLAG_SHOW_DEFAULT, &diode, 0, "Start diode number", "N"},
         {"cap", 0, POPT_ARG_INT | POPT_ARGFLAG_SHOW_DEFAULT, &cap, 0, "Start cap number", "N"},
         {"rows", 0, POPT_ARG_INT | POPT_ARGFLAG_SHOW_DEFAULT, &rows, 0, "Number of rows (grid)", "N"},
         {"cols", 0, POPT_ARG_INT, &cols, 0, "Number of cols (grid)", "N"},
         {"spacing", 0, POPT_ARG_DOUBLE | POPT_ARGFLAG_SHOW_DEFAULT, &spacing, 0, "LED spacing (grid)", "mm"},
         {"sides", 0, POPT_ARG_NONE, &sides, 0, "LED on sides rather than starty/bottom (grid)"},
         {"count", 0, POPT_ARG_INT, &count, 0, "Number of leds", "N"},
         {"diameter", 'd', POPT_ARG_DOUBLE, &diameter, 0, "LED ring diameter", "mm"},
         {"radius", 'r', POPT_ARG_DOUBLE, &radius, 0, "LED ring radius", "mm"},
         {"zone-in", 0, POPT_ARG_DOUBLE, &zonei, 0, "Zone inside ", "mm"},
         {"zone-out", 0, POPT_ARG_DOUBLE, &zoneo, 0, "Zone outside ", "mm"},
         {"angle", 'd', POPT_ARG_DOUBLE, &angle, 0, "Angle offset", "degrees"},
         {"rotate", 0, POPT_ARG_DOUBLE | POPT_ARGFLAG_SHOW_DEFAULT, &rotate, 0, "Diode rotate", "degrees"},
         {"group", 0, POPT_ARG_INT, &group, 0, "Group LEDs (ring)", "N"},
         {"pad-offset-x", 0, POPT_ARG_DOUBLE | POPT_ARGFLAG_SHOW_DEFAULT, &padoffsetx, 0, "Pad offset X", "mm"},
         {"pad-offset-y", 0, POPT_ARG_DOUBLE | POPT_ARGFLAG_SHOW_DEFAULT, &padoffsety, 0, "Pad offset Y", "mm"},
         {"cap-offset", 0, POPT_ARG_DOUBLE | POPT_ARGFLAG_SHOW_DEFAULT, &capoffset, 0, "Cap offset", "mm"},
         {"via-offset", 0, POPT_ARG_DOUBLE, &viaoffset, 0, "Via offset", "mm"},
         {"tracks", 0, POPT_ARG_NONE, &tracks, 0, "Add tracks"},
         {"vias", 0, POPT_ARG_DOUBLE, &vias, 0, "Add vias", "mm"},
         {"power-vias", 0, POPT_ARG_DOUBLE, &powervias, 0, "Add power vias", "mm"},
         {"fill", 0, POPT_ARG_STRING, &fill, 0, "Fill GND/POWER", "Power net name"},
         {"startx", 'x', POPT_ARG_DOUBLE, &startx, 0, "Left (grid) / Centre (ring)", "X"},
         {"starty", 'y', POPT_ARG_DOUBLE, &starty, 0, "Top (grid) / Centre (ring)", "Y"},
         {"cap-below", 0, POPT_ARG_NONE, &capbelow, 0, "Cap below not in-line"},
         {"SMD1010", 0, POPT_ARG_NONE, &SMD1010, 0, "SMD1010 LEDs"},
         {"SMD1615", 0, POPT_ARG_NONE, &SMD1615, 0, "SMD1615 LEDs"},
         {"SMD2020", 0, POPT_ARG_NONE, &SMD2020, 0, "SMD2020 LEDs"},
         {"debug", 'v', POPT_ARG_NONE, &debug, 0, "Debug"},
         POPT_AUTOHELP {}
      };

      optCon = poptGetContext(NULL, argc, argv, optionsTable, 0);
      /* poptSetOtherOptionHelp(optCon, ""); */

      int             c;
      if ((c = poptGetNextOpt(optCon)) < -1)
         errx(1, "%s: %s\n", poptBadOption(optCon, POPT_BADOPTION_NOALIAS), poptStrerror(c));

      if (!pcbfile && poptPeekArg(optCon))
         pcbfile = poptGetArg(optCon);
      if (poptPeekArg(optCon) || !pcbfile)
      {
         poptPrintUsage(optCon, stderr, 0);
         return -1;
      }
   }
   if (SMD1615)
   {                            /* Settings for SMD1615 (1.6mm x 1.5mm) */
      rotate = 180;
      capbelow = 1;
      powertrack = 0;
      padoffsetx = 0.625;
      padoffsety = 0.475;
      padsize = 0.45 + 0.127 * 2;       /* small side plus clearance both sides */
   }
   if (radius && !diameter)
      diameter = radius * 2;
   if (radius && diameter && diameter != radius * 2)
      errx(1, "Pick --radius or --diameter");
   if ((rows || cols || sides) && diameter)
      errx(1, "Ring or grid, not both");
   if (diameter)
   {                            /* Sanity check ring */
      if (spacing && !group && !count)
         count = diameter * M_PI / spacing;
      if (count < 3)
         errx(1, "Specify LED count (more than 2)");
   } else
   {                            /* Sanity check grid */
      if (count && !rows)
         rows = count / cols;
      else if (count && !cols)
         cols = count / rows;
      if (!(sides ? rows : cols))
         errx(1, "Need to know size");
      count = rows * cols;
      if (!spacing)
         spacing = 2;
   }
   if (diode && !cap)
      cap = diameter ? diode : (diode - 1) / (sides ? rows : cols) + 1;
   if (!viaoffset)
      viaoffset = !diameter ? 1.9 : 1.15;
   if (isnan(zonei))
      zonei = padoffsety + padsize + clearance;
   else
      zonei -= clearance / 2;
   if (isnan(zoneo))
      zoneo = padoffsety + padsize + clearance;
   else
      zoneo -= clearance / 2;
   double          padax = 2 * padoffsetx / diameter;   /* Angle for pad offset */
   double          viaa = 2 * vias / diameter;  /* Angle for via offset */
   double          spacinga = 2 * spacing / diameter;   /* Angle for spacing of groups */
   pcb_t          *pcb = pcb_load(pcbfile);
   double          ad(double d)
   {                            /* Angle for diode */
      double          a(int d)
      {
         if (!group)
            return 2.0 * M_PI * d / count;
         return 2.0 * M_PI * (d / group) / (count / group) + spacinga * (-0.5 * (group - 1) + (d % group));
      }
      double          f = floor(d);
                      return a(f) * (f + 1 - d) + a(f + 1) * (d - f) + angle * M_PI / 180;
   }
   double          ax(double a, double o)
   {                            /* X for angle and offset */
      return startx + (diameter / 2 + o) * sin(a);
   }
   double          ay(double a, double o)
   {                            /* Y for angle and offset */
      return starty - (diameter / 2 + o) * cos(a);
   }
   double          cx(double d, double a, double o)
   {                            /* X for diode and offset */
      return ax(ad(d) + a, o);
   }
   double          cy(double d, double a, double o)
   {                            /* Y for diode and offset */
      return ay(ad(d) + a, o);
   }
   double          diodex(int d)
   {
      if (diameter)
         return cx(d, 0, 0);
      if (sides)
         return startx + spacing * (d % cols);
      return startx + spacing * (d / rows);
   }
   double          diodey(int d)
   {
      if (diameter)
         return cy(d, 0, 0);
      if (sides)
         return starty + spacing * (d / cols);
      return starty + spacing * (d % rows);
   }
   double          dioder(int d)
   {
      if (diameter)
         return rotate - ad(d) * 180 / M_PI;
      return sides ? rotate : -rotate;
   }
   enum
   {
      DI, DO, GND, VCC
   };
   double          padx(int d, int n)
   {
      if (diameter)
      {
         if (SMD1615 || SMD2020)
            switch (n)
            {
               case GND:
               return cx(d, padax, padoffsety);
               case VCC:
               return cx(d, -padax, -padoffsety);
               case DO:
               return cx(d, padax, -padoffsety);
               case DI:
               return cx(d, -padax, padoffsety);
         } else
            switch (n)
            {                   /* Default is SMD1010 (diagonal) */
               case GND:
               return cx(d, 0, padoffsety);
               case VCC:
               return cx(d, 0, -padoffsety);
               case DI:return cx(d, -padax, 0);
               case DO:return cx(d, padax, 0);
            }
      } else                    /* grid */
      {
         if (SMD1615 || SMD2020)
            switch (n)
            {
               case GND:
               return diodex(d) + padoffsetx;
               case VCC:
               return diodex(d) - padoffsetx;
               case DO:
               return diodex(d) + padoffsetx;
               case DI:
               return diodex(d) - padoffsetx;
         } else
            switch (n)
            {                   /* Default is SMD1010 (diagonal) */
               case GND:
               return diodex(d);        /* TODO */
               case VCC:
               return diodex(d);        /* TODO */
               case DI:
               return diodex(d);        /* TODO */
               case DO:
               return diodex(d);        /* TODO */
            }
      }
   }
   double          pady(int d, int n)
   {
      if (diameter)
      {
         if (SMD1615 || SMD2020)
            switch (n)
            {
               case GND:
               return cy(d, padax, padoffsety);
               case VCC:
               return cy(d, -padax, -padoffsety);
               case DO:
               return cy(d, padax, -padoffsety);
               case DI:
               return cy(d, -padax, padoffsety);
         } else
            switch (n)
            {                   /* Default is SMD1010 (diagonal) */
               case GND:return cy(d, 0, padoffsety);
               case VCC:return cy(d, 0, -padoffsety);
               case DI:
               return cy(d, -padax, 0);
               case DO:
               return cy(d, padax, 0);
            }
      } else
      {                         /* grid */
         if (SMD1615 || SMD2020)
            switch (n)
            {
               case GND:
               return diodey(d) - padoffsety;
               case VCC:
               return diodey(d) + padoffsety;
               case DO:
               return diodey(d) + padoffsety;
               case DI:
               return diodey(d) - padoffsety;
         } else
            switch (n)
            {                   /* Default is SMD1010 (diagonal) */
               case GND:return diodey(d);       /* TODO */
               case VCC:return diodey(d);       /* TODO */
               case DI:
               return diodey(d);        /* TODO */
               case DO:
               return diodey(d);        /* TODO */
            }
      }
   }
   double          capx(int c)
   {
      if (diameter)
         return cx(0.5 + c, 0, capbelow ? -zonei : 0);
      if (sides)
         return (c & 1) ? startx + spacing * (cols - 1) + capoffset : startx - capoffset;
      return startx + spacing * (c / 2);
   }
   double          capy(int c)
   {
      if (diameter)
         return cy(0.5 + c, 0, capbelow ? -zonei : 0);
      if (sides)
         return starty + spacing * (c / 2);
      return (c & 1) ? starty + spacing * (rows - 1) + capoffset : starty - capoffset;
   }
   double          capr(int c)
   {
      if (diameter)
         return 90 - ad(0.5 + c) * 180 / M_PI + (capbelow ? 180 : 0);
      return sides ? 90 : 0;
   }

   pcb_t          *footprint = NULL;
   if (isnan(startx) || isnan(starty))
   {
      while ((footprint = pcb_find(pcb, "footprint", footprint)))
      {
         pcb_t          *t = pcb_find(footprint, "property", NULL);
         if (!t)
            continue;
         if (!t || t->valuen < 2 || !t->values[0].istxt || strcmp(t->values[0].txt, "Reference") || !t->values[1].istxt)
            continue;
         const char     *ref = t->values[1].txt;
         if (*ref != (!group || (group & 1) ? 'D' : 'C'))
            continue;
         int             d = atoi(ref + 1);
         if (d != (!group || (group & 1) ? diode : cap))
            continue;
         t = pcb_find(footprint, "at", NULL);
         if (!t || t->valuen < 2 || !t->values[0].isnum || !t->values[1].isnum)
            continue;
         startx = t->values[0].num;
         starty = t->values[1].num;
         t = pcb_find(footprint, "layer", NULL);
         if (!t || t->valuen < 1 || !t->values[0].istxt)
            continue;
         layer = t->values[0].txt;
         break;
      }
      if (diameter)
      {                         /* Assuming start D/C is top */
         startx += diameter / 2;
         starty += diameter / 2;
      }
   }
   if (!layer)
      layer = "F.Cu";
   if (isnan(startx) || isnan(starty))
      warnx("Cannot find start point (i.e. %c%d)", !group || (group & 1) ? 'D' : 'C', !group || (group & 1) ? diode : cap);
   int             leds = 0;
   int             countcap = !diameter ? (sides ? rows : cols) * 2 : count;
   footprint = NULL;
   while ((footprint = pcb_find(pcb, "footprint", footprint)))
   {
      pcb_t          *t = pcb_find(footprint, "property", NULL);
      if (!t)
         continue;
      if (!t || t->valuen < 2 || !t->values[0].istxt || strcmp(t->values[0].txt, "Reference") || !t->values[1].istxt)
         continue;
      const char     *ref = t->values[1].txt;
      if (*ref == 'D')
      {                         /* Diode placement */
         int             d = atoi(ref + 1);
         if (d < diode)
            continue;           /* Before start */
         if (count && d >= diode + count)
            continue;           /* Off end */
         t = pcb_find(footprint, "at", NULL);
         if (!t)
            t = pcb_append_obj(footprint, "at");
         else
            pcb_clear(t);
         d -= diode;
         pcb_append_num(t, diodex(d));
         pcb_append_num(t, diodey(d));
         pcb_append_num(t, dioder(d));
         leds++;
         continue;
      }
      if (*ref == 'C')
      {                         /* Cap placement */
         int             c = atoi(ref + 1);
         if (c < cap)
            continue;           /* Before start */
         if (countcap && c >= cap + countcap)
            continue;           /* Off end */
         t = pcb_find(footprint, "at", NULL);
         if (!t)
            t = pcb_append_obj(footprint, "at");
         else
            pcb_clear(t);
         c -= cap;
         pcb_append_num(t, capx(c));
         pcb_append_num(t, capy(c));
         pcb_append_num(t, capr(c));
         continue;
      }
   }
   if (sides)
      rows = leds / cols;
   else
      cols = leds / rows;
   void            zaptrack(double x1, double y1, double x2, double y2)
   {
      pcb_t          *s = NULL,
                     *o;
      while           ((s = pcb_find(pcb, isnan(x2) ? "segment" : "arc", s)))
      {
         o = pcb_find(s, "layer", NULL);
         if (!o || o->valuen != 1 || !o->values[0].istxt || strcmp(o->values[0].txt, layer))
            continue;
         o = pcb_find(s, "start", NULL);
         if (!o || o->valuen != 2 || !o->values[0].isnum || !o->values[1].isnum)
            continue;
         double          d = (o->values[0].num - x1) * (o->values[0].num - x1) + (o->values[1].num - y1) * (o->values[1].num - y1);
         if              (d >= delta * delta)
                            continue;
         /* No need to check mid */
                         o = pcb_find(s, "end", NULL);
         if              (!o || o->valuen != 2 || !o->values[0].isnum || !o->values[1].isnum)
                            continue;
                         d = (o->values[0].num - x2) * (o->values[0].num - x2) + (o->values[1].num - y2) * (o->values[1].num - y2);
         if              (d >= delta * delta)
                            continue;
                         s->tag = NULL; /* Suppress as we are replacing */
      }
   }
   void            track(double x1, double y1, double x2, double y2, double x3, double y3, double w)
   {                            /* Add a track */
      pcb_t          *s = NULL,
                     *o;
                      zaptrack(x1, y1, x3, y3);
                      s = pcb_append_obj(pcb, isnan(x2) ? "segment" : "arc"), *o;
                      o = pcb_append_obj(s, "start");
                      pcb_append_num(o, x1);
                      pcb_append_num(o, y1);
      if              (!isnan(x2))
      {
         o = pcb_append_obj(s, "mid");
         pcb_append_num(o, x2);
         pcb_append_num(o, y2);
      }
                      o = pcb_append_obj(s, "end");
                      pcb_append_num(o, x3);
                      pcb_append_num(o, y3);
                      o = pcb_append_obj(s, "width");
                      pcb_append_num(o, w);
                      o = pcb_append_obj(s, "layer");
                      pcb_append_txt(o, layer);
   }
   double          checkpad(double x, double y, const char *net)
   {
      double          best = NAN;
      pcb_t          *f = NULL,
                     *o;
      while           ((f = pcb_find(pcb, "footprint", f)))
      {
         o = pcb_find(f, "at", NULL);
         if (!o || o->valuen < 2 || !o->values[0].isnum || !o->values[1].isnum)
            continue;
         double          fx = o->values[0].num,
                         fy = o->values[1].num;
         double          fa = 0;
         if              (o->valuen > 2 && o->values[2].isnum)
                            fa = -o->values[2].num * M_PI * 2 / 360;
         pcb_t          *p = NULL;
         while           ((p = pcb_find(f, "pad", p)))
         {
            o = pcb_find(p, "net", NULL);
            if (o && o->valuen >= 2 && o->values[1].istxt && !strcmp(o->values[1].txt, net))
               continue;
            o = pcb_find(p, "at", NULL);
            if (!o || o->valuen < 2 || !o->values[0].isnum || !o->values[1].isnum)
               continue;
            double          ax = o->values[0].num,
                            ay = o->values[1].num;
            double          px = fx + ax * cos(fa) - ay * sin(fa),
                            py = fy + ax * sin(fa) + ay * cos(fa);
            double          d = (px - x) * (px - x) + (py - y) * (py - y);
            if              (isnan(best) || d < best)
                               best = d;
         }
      }
                      return sqrt(best);
   }
   double          zapvia(double x, double y)
   {                            /* Zap a matching via, return closest non match */
      double          best = NAN;
      pcb_t          *s = NULL,
                     *o;
      while           ((s = pcb_find(pcb, "via", s)))
      {
         o = pcb_find(s, "at", NULL);
         if (!o || o->valuen != 2 || !o->values[0].isnum || !o->values[1].isnum)
            continue;
         double          d = (o->values[0].num - x) * (o->values[0].num - x) + (o->values[1].num - y) * (o->values[1].num - y);
         if              (d < delta * delta)
                            s->tag = NULL;      /* Suppress as we are replacing */
         else if         (isnan(best) || d < best)
                            best = d;
      }
                      return sqrt(best);
   }
   void            addvia(double x, double y, double size)
   {                            /* add a via */
      pcb_t          *s = NULL,
                     *o;
                      s = pcb_append_obj(pcb, "via");
                      o = pcb_append_obj(s, "at");
                      pcb_append_num(o, x);
                      pcb_append_num(o, y);
                      o = pcb_append_obj(s, "size");
                      pcb_append_num(o, size);
                      o = pcb_append_obj(s, "drill");
                      pcb_append_num(o, size / 2);
                      o = pcb_append_obj(s, "layers");
                      pcb_append_txt(o, "F.Cu");
                      pcb_append_txt(o, "B.Cu");
   }
   void            via(double x, double y, double size)
   {                            /* Add a via replacing existing */
      if (!size)
         return;                /* Not doing via */
      zapvia(x, y);
      addvia(x, y, size);
   }
   void            trackvia(double x1, double y1, double x2, double y2, double w, double v)
   {
      if (!v)
         return;
      track(x1, y1, NAN, NAN, x2, y2, w);
      via(x2, y2, v);
   }
   void            trackviamaybe(double x1, double y1, double x2, double y2, double w, double v, const char *net)
   {
      if (!v)
         return;
      double          d = zapvia(x2, y2);
      if              (!isnan(d) && d < (powervias ? : vias) + clearance)
      {
         zaptrack(x1, y1, x2, y2);
         return;                /* Too close to another via */
      }
                      d = checkpad(x2, y2, net);
      if              (!isnan(d) && d < (powervias ? : vias) / 2 + clearance + padsize)
      {
         zaptrack(x1, y1, x2, y2);
         return;                /* Too close to another pad (different net) */
      }
                      track(x1, y1, NAN, NAN, x2, y2, w);
                      addvia(x2, y2, v);
   }
pcb_t *         zone(const char *net, const char *layer)
   {
      pcb_t          *s,
                     *o,
                     *p;
                      s = pcb_append_obj(pcb, "zone");
                      o = pcb_append_obj(s, "priority");
                      pcb_append_num(o, 2);
                      o = pcb_append_obj(s, "net_name");
                      pcb_append_txt(o, net);
                      o = pcb_append_obj(s, "layer");
                      pcb_append_txt(o, layer);
                      o = pcb_append_obj(s, "hatch");
                      pcb_append_lit(o, "edge");
                      pcb_append_num(o, .5);
                      o = pcb_append_obj(s, "connect_pads");
                      pcb_append_lit(o, "yes");
                      p = pcb_append_obj(o, "clearance");
                      pcb_append_num(p, clearance);
                      o = pcb_append_obj(s, "min_thickness");
                      pcb_append_num(o, .25);
                      o = pcb_append_obj(s, "filled_areas_thickness");
                      pcb_append_lit(o, "no");
                      o = pcb_append_obj(s, "fill");
                      pcb_append_lit(o, "yes");
                      p = pcb_append_obj(o, "thermal_gap");
                      pcb_append_num(p, .5);
                      p = pcb_append_obj(o, "thermal_bridge_width");
                      pcb_append_num(p, .75);
                      o = pcb_append_obj(s, "polygon");
                      o = pcb_append_obj(o, "pts");
                      return o;
   }
   void            zapzone(double x, double y, const char *net, const char *layer)
   {
      pcb_t          *s = NULL,
                     *o;
      while           ((s = pcb_find(pcb, "zone", s)))
      {
         o = pcb_find(s, "net_name", NULL);
         if (!o || o->valuen != 1 || !o->values[0].istxt || strcmp(o->values[0].txt, net))
            continue;
         o = pcb_find(s, "layer", NULL);
         if (!o || o->valuen != 1 || !o->values[0].istxt || strcmp(o->values[0].txt, layer))
            continue;
         o = pcb_find(s, "polygon", NULL);
         o = pcb_find(o, "pts", NULL);
         o = pcb_find(o, "xy", NULL);
         if (!o || o->valuen != 2 || !o->values[0].isnum || !o->values[1].isnum)
            continue;
         double          d = (o->values[0].num - x) * (o->values[0].num - x) + (o->values[1].num - y) * (o->values[1].num - y);
         if              (d < delta * delta)
                            s->tag = NULL;
      }
   }
   void            ringzone(double a, double b, const char *net, const char *layer)
   {
      if (a == b)
         return;
      zapzone(ax(0, a), ay(0, a), net, layer);
      pcb_t          *z = zone(net, layer);
      void            xy(double d, double o)
      {
         pcb_t          *xy = pcb_append_obj(z, "xy");
                         pcb_append_num(xy, ax(d, o));
                         pcb_append_num(xy, ay(d, o));
      }
      int             steps = M_PI * diameter;
                      xy(0, a);
      for             (int d = 0; d <= steps; d++)
                         xy(M_PI * 2 * d / steps, b);
      for             (int d = steps; d > 0; d--)
                         xy(M_PI * 2 * d / steps, a);
   }
   void            boxzone(double x1, double y1, double x2, double y2, const char *net)
   {
      if (x1 == x2 || y1 == y2)
         return;
      zapzone(x1, y1, net, layer);
      pcb_t          *z = zone(net, layer);
      void            xy(double x, double y)
      {
         pcb_t          *xy = pcb_append_obj(z, "xy");
                         pcb_append_num(xy, x);
                         pcb_append_num(xy, y);
      }
                      xy(x1, y1);
                      xy(x1, y2);
                      xy(x2, y2);
                      xy(x2, y1);
   }

   if              (tracks && trackwidth)
   {
      if (diameter)
      {                         /* Ring */
         int             tooclose = ((!group && (M_PI * 2 - (ad(count - 0.5) - angle * M_PI / 180)) < padax * 4 + viaa * 4) || (group && spacing < padoffsetx * 6 + viaa * 4)); /* Too close together
                                                                                                                                                                                 * for data pins
                                                                                                                                                                                 * together at end */
         double          m = trackwidth / 2 + clearance + powervias / 2;
         double          zi = zonei - powervias / 2;
         if (zi < m)
            zi = m;
         else
            zi = (m + zi) / 2;
         double          zo = zoneo - powervias / 2;
         if (zo < m)
            zo = m;
         else
            zo = (m + zo) / 2;
         /* Data */
         for (int d = 0; d < count - 1; d++)
            track(padx(d, DO), pady(d, DO),
                  cx(0.5 + d, 0, 0), cy(0.5 + d, 0, 0),
                  padx(1 + d, DI), pady(1 + d, DI), trackwidth);
         /* Data end */
         if (tooclose)
         {                      /* Straight out to pad */
            double          o = (capbelow ? 0.333 : 0.25);
            track(padx(0, DI), pady(0, DI),
                  NAN, NAN,
                  cx(count - o, 0, padoffsety), cy(count - o, 0, padoffsety), trackwidth);
            trackvia(cx(count - o, 0, padoffsety), cy(count - o, 0, padoffsety),
                     cx(count - o, 0, -viaoffset), cy(count - o, 0, -viaoffset),
                     trackwidth, vias);
            track(padx(count - 1, DO), pady(count - 1, DO),
                  NAN, NAN,
                  cx(count - 1 + o, 0, -padoffsety), cy(count - 1 + o, 0, -padoffsety), trackwidth);
            trackvia(cx(count - 1 + o, 0, -padoffsety), cy(count - 1 + o, 0, -padoffsety),
                     cx(count - 1 + o, 0, viaoffset), cy(count - 1 + o, 0, viaoffset),
                     trackwidth, vias);
         } else
         {
            track(padx(0, DI), pady(0, DI),
                  NAN, NAN,
                  cx(0, -padax - viaa, padoffsety), cy(0, -padax - viaa, padoffsety), trackwidth);
            trackvia(cx(0, -padax - viaa, padoffsety), cy(0, -padax - viaa, padoffsety),
                     cx(0, -padax - viaa, -vias / 2), cy(0, -padax - viaa, -vias / 2),
                     trackwidth, vias);
            track(padx(count - 1, DO), pady(count - 1, DO),
                  cx(count - 0.5, 0, 0), cy(count - 0.5, 0, 0),
                  cx(0, -padax - viaa * 3, 0), cy(0, -padax - viaa * 3, 0), trackwidth);
            trackvia(cx(0, -padax - viaa * 3, 0), cy(0, -padax - viaa * 3, 0),
                     cx(0, -padax - viaa * 3, -vias / 3), cy(0, -padax - viaa * 3, -vias / 2), trackwidth, vias);
         }
         /* Power */
         if (powertrack)
         {
            for (int d = 0; d < count - 1; d++)
            {
               track(padx(d, GND), pady(d, GND),
                     cx(0.5 + d, 0, padoffsety), cy(0.5 + d, 0, padoffsety),
                     padx(1 + d, GND), pady(1 + d, GND), trackwidth);
               track(padx(d, VCC), pady(d, VCC),
                     cx(d, 0, -padoffsety), cy(d, 0, -padoffsety),
                     padx(1 + d, VCC), pady(1 + d, VCC), trackwidth);
            }
            /* Power end */
            track(cx(count - 0.5, 0, padoffsety), cy(count - 0.5, 0, padoffsety),
                  cx(count - 0.25, 0, padoffsety), cy(count - 0.25, 0, padoffsety),
                  padx(0, GND), pady(0, GND), trackwidth);
            track(padx(count - 1, VCC), pady(count - 1, VCC),
                  cx(count - 0.75, 0, -padoffsety), cy(count - 0.75, 0, -padoffsety),
                  cx(count - 0.5, 0, -padoffsety), cy(count - 0.5, 0, -padoffsety), trackwidth);
         }
         /* Power vias */
         if (powervias)
         {
            for (int d = 1; d < count - 1; d++)
               trackviamaybe(padx(d, GND), pady(d, GND),
                             cx(d + padax + 0.25, 0, viaoffset), cy(d + padax + 0.25, 0, viaoffset), trackwidth, powervias, "GND");
            for (int d = 1; d < count - 1; d++)
               trackviamaybe(padx(d, VCC), pady(d, VCC),
                 cx(d - padax - 0.25, 0, -viaoffset), cy(d - padax - 0.25, 0, -viaoffset), trackwidth, powervias, fill ? : "VCC");
         }
      } else if (SMD1615 || SMD2020)
      {                         /* New grid logic */
         if (!sides)
         {                      /* Link in/out */
            for (int r = 0; r < rows - 1; r++)
               for (int c = 0; c < cols; c++)
               {
                  int             d = c * rows + r;
                  if (c && c < cols - 1)
                     track(padx(d, DO), pady(d, DO), NAN, NAN, padx(d + 1, DI), pady(d + 1, DI), trackwidth);
                  else
                  {
                     track(padx(d, DO), pady(d, DO), NAN, NAN, padx(d, DO), (pady(d, DO) + pady(d + 1, GND)) / 2, trackwidth);
                     track(padx(d, DO), (pady(d, DO) + pady(d + 1, GND)) / 2, NAN, NAN, padx(d + 1, DI), (pady(d, DO) + pady(d + 1, GND)) / 2, trackwidth);
                     track(padx(d + 1, DI), (pady(d, DO) + pady(d + 1, GND)) / 2, NAN, NAN, padx(d + 1, DI), pady(d + 1, DI), trackwidth);
                  }
               }
            /* in/out end vias */
            for (int c = 0; c < cols; c++)
            {
               int             d = c * rows;
               trackvia(padx(d, DI), pady(d, DI), padx(d, GND) - padsize / 2 - vias / 2, pady(d, DI) + ((c && c < cols - 1) ? padin : 0), trackwidth, vias);
               d += rows - 1;
               trackvia(padx(d, DO), pady(d, DO), padx(d, VCC) + padsize / 2 + vias / 2, pady(d, VCC) - ((c && c < cols - 1) ? padin : 0), trackwidth, vias);
            }
            /* Power vias */
            if (powervias)
            {
               for (int r = 0; r < rows; r++)
                  for (int c = 0; c < cols; c += cols - 1)
                  {
                     int             d = c * rows + r;
                     if (r)
                        trackvia(padx(d, GND), pady(d, GND), padx(d, DI) + padsize / 2 + vias / 2, pady(d, DI), trackwidth, vias);
                     if (r < rows - 1)
                        trackvia(padx(d, VCC), pady(d, VCC), padx(d, DO) - padsize / 2 - vias / 2, pady(d, DO), trackwidth, vias);
                  }
               for (int c = 0; c < cols; c++)
               {
                  int             d = c * rows;
                  if (c < cols - 1)
                     trackvia(padx(d, GND), pady(d, GND), padx(d + rows, DI) - padsize / 2 - vias / 2, pady(d + rows, DI), trackwidth, vias);
                  d += rows - 1;
                  if (c)
                     trackvia(padx(d, VCC), pady(d, VCC), padx(d - rows, DO) + padsize / 2 + vias / 2, pady(d + rows, DO), trackwidth, vias);
               }
            }
            /* Power tracks */
            for (int c = 0; c < cols; c++)
            {
               int             d = c * rows;
               if (c < cols - 1)
                  track(padx(d, GND) + padsize / 2 + trackwidth / 2, pady(d, GND), NAN, NAN, padx(d + rows - 1, GND) + padsize / 2 + trackwidth / 2, pady(d + rows - 1, GND), trackwidth);
               if (c)
                  track(padx(d, VCC) - padsize / 2 - trackwidth / 2, pady(d, VCC), NAN, NAN, padx(d + rows - 1, VCC) - padsize / 2 - trackwidth / 2, pady(d + rows - 1, VCC), trackwidth);
               for (int r = 0; r < rows; r++)
               {
                  d = c * rows + r;
                  if (c < cols - 1)
                     track(padx(d, GND), pady(d, GND), NAN, NAN, padx(d, GND) + padsize / 2 + trackwidth / 2, pady(d, GND), trackwidth);
                  if (c)
                     track(padx(d, VCC), pady(d, VCC), NAN, NAN, padx(d, VCC) - padsize / 2 - trackwidth / 2, pady(d, VCC), trackwidth);
               }
            }

         } else
         {                      /* TODO */
         }
      } else if (sides)
      {                         /* Grid with caps on sides */
         for (int r = 0; r < rows; r++)
         {
            track(diodex(r * cols) - viaoffset, diodey(r * cols), NAN, NAN, diodex(r * cols) - padoffsety, diodey(r * cols),
                  trackwidth);
            trackvia(diodex(r * cols + cols - 1) + padoffsety, diodey(r * cols + cols - 1),
                     diodex(r * cols + cols - 1) + viaoffset, diodey(r * cols + cols - 1), trackwidth, vias);
         }
         for (int r = 0; r < rows; r++)
            for (int c = 0; c < cols - 1; c++)
               track(diodex(c + r * cols) + padoffsety, diodey(c + r * cols), NAN, NAN, diodex(c + r * cols + 1) - padoffsety,
                     diodey(c + r * cols + 1), trackwidth);
         for (int r = 0; r < rows; r++)
         {
            track(diodex(r * cols) - capoffset, diodey(r * cols) - padoffsety, NAN, NAN, diodex(r * cols),
                  diodey(r * cols) - padoffsety, trackwidth);
            track(diodex(r * cols) - capoffset, diodey(r * cols) + padoffsety, NAN, NAN, diodex(r * cols),
                  diodey(r * cols) + padoffsety, trackwidth);
            for (int c = 0; c < cols - 1; c++)
            {
               track(diodex(c + r * cols), diodey(c + r * cols) - padoffsety, NAN, NAN, diodex(c + r * cols + 1),
                     diodey(c + r * cols + 1) - padoffsety, trackwidth);
               track(diodex(c + r * cols), diodey(c + r * cols) + padoffsety, NAN, NAN, diodex(c + r * cols + 1),
                     diodey(c + r * cols + 1) + padoffsety, trackwidth);
            }
            track(diodex(r * cols + cols - 1), diodey(r * cols + cols - 1) - padoffsety, NAN, NAN,
                  diodex(r * cols + cols - 1) + capoffset, diodey(r * cols + cols - 1) - padoffsety, trackwidth);
            track(diodex(r * cols + cols - 1), diodey(r * cols + cols - 1) + padoffsety, NAN, NAN,
                  diodex(r * cols + cols - 1) + capoffset, diodey(r * cols + cols - 1) + padoffsety, trackwidth);
         }
      } else
      {                         /* Grid with caps at top/bottom */
         for (int c = 0; c < cols; c++)
         {
            trackvia(diodex(c * rows), diodey(c * rows) - padoffsety, diodex(c * rows), diodey(c * rows) - viaoffset,
                     trackwidth, vias);
            trackvia(diodex(c * rows + rows - 1), diodey(c * rows + rows - 1) + padoffsety, diodex(c * rows + rows - 1),
                     diodey(c * rows + rows - 1) + viaoffset, trackwidth, vias);
         }
         for (int c = 0; c < cols; c++)
            for (int r = 0; r < rows - 1; r++)
               track(diodex(r + c * rows), diodey(r + c * rows) + padoffsety, NAN, NAN, diodex(r + c * rows + 1),
                     diodey(r + c * rows + 1) - padoffsety, trackwidth);
         for (int c = 0; c < cols; c++)
         {
            track(diodex(c * rows) - padoffsety, diodey(c * rows) - capoffset, NAN, NAN, diodex(c * rows) - padoffsety,
                  diodey(c * rows), trackwidth);
            track(diodex(c * rows) + padoffsety, diodey(c * rows) - capoffset, NAN, NAN, diodex(c * rows) + padoffsety,
                  diodey(c * rows), trackwidth);
            for (int r = 0; r < rows - 1; r++)
            {
               track(diodex(r + c * rows) - padoffsety, diodey(r + c * rows), NAN, NAN, diodex(r + c * rows + 1) - padoffsety,
                     diodey(r + c * rows + 1), trackwidth);
               track(diodex(r + c * rows) + padoffsety, diodey(r + c * rows), NAN, NAN, diodex(r + c * rows + 1) + padoffsety,
                     diodey(r + c * rows + 1), trackwidth);
            }
            track(diodex(c * rows + rows - 1) - padoffsety, diodey(c * rows + rows - 1), NAN, NAN,
                  diodex(c * rows + rows - 1) - padoffsety, diodey(c * rows + rows - 1) + capoffset, trackwidth);
            track(diodex(c * rows + rows - 1) + padoffsety, diodey(c * rows + rows - 1), NAN, NAN,
                  diodex(c * rows + rows - 1) + padoffsety, diodey(c * rows + rows - 1) + capoffset, trackwidth);
         }
         if (powervias)
         {
            for (int c = 0; c < cols - 1; c++)
               trackviamaybe(diodex(c * rows + rows - 1) + padoffsety, diodey(c * rows + rows - 1) + capoffset,
                             diodex(c * rows + rows - 1) + spacing / 2, diodey(c * rows + rows - 1) + viaoffset, trackwidth,
                             powervias, "GND");
            for (int c = 1; c < cols; c++)
               trackviamaybe(diodex(c * rows) - padoffsety, diodey(c * rows) - capoffset, diodex(c * rows) - spacing / 2,
                             diodey(c * rows) - viaoffset, trackwidth, powervias, fill ? : "VCC");
         }
      }
   }
   if (fill)
   {
      if (diameter)
      {
         ringzone(clearance / 2, zoneo, "GND", layer);
         ringzone(-clearance / 2, -zonei, fill, layer);
         if (powervias)
         {
            ringzone(clearance / 2, zoneo, "GND", *layer == 'F' ? "B.Cu" : "F.Cu");
            ringzone(-clearance / 2, -zonei, fill, *layer == 'F' ? "B.Cu" : "F.Cu");
         }
      } else if (sides)
         for (int r = 0; r < rows; r++)
         {
            boxzone(diodex(r * cols) - spacing / 2, diodey(r * cols) - spacing / 2 + clearance / 2,
                    diodex(r * cols + cols - 1) + spacing / 2, diodey(r * cols + cols - 1), "GND");
            boxzone(diodex(r * cols) - spacing / 2, diodey(r * cols), diodex(r * cols + cols - 1) + spacing / 2,
                    diodey(r * cols + cols - 1) + spacing / 2 - clearance / 2, fill);
      } else
         for (int c = 0; c < cols; c++)
         {
            boxzone(diodex(c * rows) + clearance / 2, diodey(c * rows) - spacing / 2,
                    diodex(c * rows + rows - 1) + spacing / 2 - clearance / 2, diodey(c * rows + rows - 1) + spacing / 2, "GND");
            boxzone(diodex(c * rows) - clearance / 2, diodey(c * rows) - spacing / 2,
                    diodex(c * rows + rows - 1) - spacing / 2 + clearance / 2, diodey(c * rows + rows - 1) + spacing / 2, fill);
         }
   }

   pcb_write(pcbfile, pcb);
   pcb = pcb_free(pcb);

   poptFreeContext(optCon);
   return 0;
}

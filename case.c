/* Make an OpendScad file from a kicad_pcb file */
/* (c) 2021 Adrian Kennard Andrews & Arnold Ltd */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <popt.h>
#include <err.h>
#include <math.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <ctype.h>

#include "models.h"             /* the 3D models */

/* yet, all globals, what the hell */
int             debug = 0;
const char     *pcbfile = NULL;
char           *scadfile = NULL;
double          pcbthickness = 0;
double          casethickness = 3;
double          pcbwidth = 0;
double          pcblength = 0;
double          casebase = 10;
double          casetop = 10;


/* strings from file, lots of common, so make a table */
int             strn = 0;
const char    **strs = NULL;    /* the object tags */
const char     *
add_string(const char *s, const char *e)
{                               /* allocates a string */
   /* simplistic */
   int             n;
   for (n = 0; n < strn; n++)
      if (strlen(strs[n]) == (int)(e - s) && !memcmp(strs[n], s, (int)(e - s)))
         return strs[n];
   strs = realloc(strs, (++strn) * sizeof(*strs));
   if (!strs)
      errx(1, "malloc");
   strs[n] = strndup(s, (int)(e - s));
   return strs[n];
}

typedef struct obj_s obj_t;
typedef struct value_s value_t;

struct value_s
{                               /* value */
   /* only one set */
   unsigned char   isobj:1;     /* object */
   unsigned char   isnum:1;     /* number */
   unsigned char   isbool:1;    /* boolean */
   unsigned char   istxt:1;     /* text */
   union
   {                            /* the value */
      obj_t          *obj;
      double          num;
      const char     *txt;
      unsigned char   bool:1;
   };
};

obj_t          *pcb = NULL;

struct obj_s
{                               /* an object */
   const char     *tag;         /* object tag */
   int             valuen;      /* number of values */
   value_t        *values;      /* the values */
};

obj_t          *
parse_obj(const char **pp, const char *e)
{                               /* Scan an object */
   const char     *p = *pp;
   obj_t          *pcb = malloc(sizeof(*pcb));
   if (p >= e)
      errx(1, "EOF");
   memset(pcb, 0, sizeof(*pcb));
   if (*p != '(')
      errx(1, "Expecting (\n%.20s\n", p);
   p++;
   if (p >= e)
      errx(1, "EOF");
   /* tag */
   const char     *t = p;
   while (p < e && (isalnum(*p) || *p == '_'))
      p++;
   if (p == t)
      errx(1, "Expecting tag\n%.20s\n", t);
   pcb->tag = add_string(t, p);
   /* values */
   while (p < e)
   {
      while (p < e && isspace(*p))
         p++;
      if (*p == ')')
         break;
      pcb->values = realloc(pcb->values, (++(pcb->valuen)) * sizeof(*pcb->values));
      if (!pcb->values)
         errx(1, "malloc");
      value_t        *value = pcb->values + pcb->valuen - 1;
      memset(value, 0, sizeof(*value));
      /* value */
      if (*p == '(')
      {
         value->isobj = 1;
         value->obj = parse_obj(&p, e);
         continue;
      }
      if (*p == '"')
      {                         /* quoted text */
         p++;
         t = p;
         while (p < e && *p != '"')
            p++;
         if (p == e)
            errx(1, "EOF");
         value->istxt = 1;
         value->txt = add_string(t, p);
         p++;
         continue;
      }
      t = p;
      while (p < e && *p != ')' && *p != ' ')
         p++;
      if (p == e)
         errx(1, "EOF");
      /* work out some basic types */
      if ((p - t) == 4 && !memcmp(t, "true", (int)(p - t)))
      {
         value->isbool = 1;
         value->bool = 1;
         continue;;
      }
      if ((p - t) == 5 && !memcmp(t, "false", (int)(p - t)))
      {
         value->isbool = 1;
         continue;;
      }
      double          v = 0;
      if (sscanf(t, "%lf", &v) == 1)
      {                         /* safe as we know followed by space or close bracket and not EOF */
         value->isnum = 1;
         value->num = v;
         continue;
      }
      /* assume string */
      value->istxt = 1;
      value->txt = add_string(t, p);
   }
   if (p >= e)
      errx(1, "EOF");
   if (*p != ')')
      errx(1, "Expecting )\n%.20s\n", p);
   p++;
   while (p < e && isspace(*p))
      p++;
   *pp = p;
   return pcb;
}

void
dump_obj(obj_t * o)
{
   printf("(%s", o->tag);
   for (int n = 0; n < o->valuen; n++)
   {
      value_t        *v = &o->values[n];
      if (v->isobj)
         dump_obj(v->obj);
      else if (v->istxt)
         printf(" \"%s\"", v->txt);
      else if (v->isnum)
         printf(" %lf", v->num);
      else if (v->isbool)
         printf(" %s", v->bool ? "true" : "false");
   }
   printf(")\n");
}

obj_t          *
find_obj(obj_t * o, const char *tag, obj_t * prev)
{
   int             n = 0;
   if (prev)
      for (; n < o->valuen; n++)
         if (o->values[n].isobj && o->values[n].obj == prev)
         {
            n++;
            break;
         }
   for (; n < o->valuen; n++)
      if (o->values[n].isobj && !strcmp(o->values[n].obj->tag, tag))
         return o->values[n].obj;
   return NULL;
}

void
load_pcb(void)
{
   int             f = open(pcbfile, O_RDONLY);
   if (f < 0)
      err(1, "Cannot open %s", pcbfile);
   struct stat     s;
   if (fstat(f, &s))
      err(1, "Cannot stat %s", pcbfile);
   char           *data = mmap(NULL, s.st_size, PROT_READ, MAP_PRIVATE, f, 0);
   if (!data)
      errx(1, "Cannot access %s", pcbfile);
   const char     *p = data;
   pcb = parse_obj(&p, data + s.st_size);
   munmap(data, s.st_size);
}

void
process_pcb(void)
{
   /* some basic settings */
   obj_t          *o;
   if (strcmp(pcb->tag, "kicad_pcb"))
      errx(1, "Not a kicad_pcb (%s)", pcb->tag);
   obj_t          *general = find_obj(pcb, "general", NULL);
   if (general)
   {
      if ((o = find_obj(general, "thickness", NULL)) && o->valuen == 1 && o->values[0].isnum)
         pcbthickness = o->values[0].num;
   }
   /* sanity */
   if (!pcbthickness)
      errx(1, "Specify pcb thickness");
   /*
    * if (!pcbwidth || !pcblength) errx(1, "Specify pcb size");
    */
}

void
write_scad(void)
{
   obj_t          *o,
                  *o2;
   /* making scad file */
   FILE           *f = stdout;
   if (strcmp(scadfile, "-"))
      f = fopen(scadfile, "w");
   if (!f)
      err(1, "Cannot open %s", scadfile);
   fprintf(f, "// Generated case design for %s\n", pcbfile);
   fprintf(f, "// By https://github.com/revk/PCBCase\n");
   if ((o = find_obj(pcb, "title_block", NULL)))
      for (int n = 0; n < o->valuen; n++)
         if (o->values[n].isobj && (o2 = o->values[n].obj)->valuen == 1 && o2->values[0].istxt)
            fprintf(f, "// %s:\t%s\n", o2->tag, o2->values[0].txt);

   if (f != stdout)
      fclose(f);
}

int
main(int argc, const char *argv[])
{
   {                            /* POPT */
      poptContext     optCon;   /* context for parsing  command - line options */
      const struct poptOption optionsTable[] = {
         {"pcb-file", 'f', POPT_ARG_STRING, &pcbfile, 0, "PCB file", "filename"},
         {"scad-file", 'o', POPT_ARG_STRING, &scadfile, 0, "Openscad file", "filename"},
         {"width", 0, POPT_ARG_DOUBLE, &pcbwidth, 0, "PCB width (default: auto)", "mm"},
         {"length", 0, POPT_ARG_DOUBLE, &pcblength, 0, "PCB length (default: auto)", "mm"},
         {"pcb-thickness", 0, POPT_ARG_DOUBLE, &pcbthickness, 0, "PCB thickness (default: auto)", "mm"},
         {"base", 0, POPT_ARG_DOUBLE | POPT_ARGFLAG_SHOW_DEFAULT, &casebase, 0, "Case base", "mm"},
         {"top", 0, POPT_ARG_DOUBLE | POPT_ARGFLAG_SHOW_DEFAULT, &casetop, 0, "Case top", "mm"},
         {"thickness", 0, POPT_ARG_DOUBLE | POPT_ARGFLAG_SHOW_DEFAULT, &casethickness, 0, "Case thickness", "mm"},
         {"debug", 'v', POPT_ARG_NONE, &debug, 0, "Debug"},
         POPT_AUTOHELP {}
      };

      optCon = poptGetContext(NULL, argc, argv, optionsTable, 0);
      /* poptSetOtherOptionHelp(optCon, ""); */

      int             c;
      if ((c = poptGetNextOpt(optCon)) < -1)
         errx(1, "%s: %s\n", poptBadOption(optCon, POPT_BADOPTION_NOALIAS), poptStrerror(c));

      if (poptPeekArg(optCon) && !pcbfile)
         pcbfile = poptGetArg(optCon);

      if (poptPeekArg(optCon) || !pcbfile)
      {
         poptPrintUsage(optCon, stderr, 0);
         return -1;
      }
      poptFreeContext(optCon);
   }
   if (!scadfile)
   {
      const char     *f = strrchr(pcbfile, '/');
      if (f)
         f++;
      else
         f = pcbfile;
      const char     *e = strrchr(f, '.');
      if (!e || !strcmp(e, ".scad"))
         e = f + strlen(f);
      if (asprintf(&scadfile, "%.*s.scad", (int)(e - pcbfile), pcbfile) < 0)
         errx(1, "malloc");
   }
   load_pcb();
   process_pcb();
   write_scad();

   return 0;
}

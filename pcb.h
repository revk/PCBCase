// KiCad PCB file handling

typedef struct pcb_s pcb_t;
typedef struct pcb_val_s pcb_val_t;

struct pcb_val_s
{                               /* value */
   /* only one set */
   unsigned char isobj:1;       /* object */
   unsigned char isnum:1;       /* number */
   unsigned char isbool:1;      /* boolean */
   unsigned char istxt:1;       /* text */
   unsigned char islit:1;       /* literal */
   union
   {                            /* the value */
      pcb_t *obj;
      double num;
      const char *txt;
      unsigned char bool:1;
   };
};

struct pcb_s
{                               /* an object */
   const char *tag;             /* object tag */
   int valuen;                  /* number of values */
   pcb_val_t *values;           /* the values */
};

pcb_t *pcb_load (const char *pcbfile);	// Load a PCB from file
void pcb_write (const char *pcbfile, pcb_t *); // Write a PCB to file
pcb_t *pcb_find (pcb_t * pcb, const char *tag, pcb_t * prev); // Find next object by tag
pcb_t *pcb_delete (pcb_t * pcb);	// Delete PCB tree when finished


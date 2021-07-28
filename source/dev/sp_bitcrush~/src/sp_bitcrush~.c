#include "ext.h"
#include "ext_obex.h"
#include "z_dsp.h"
#include "soundpipe.h"

typedef struct _bitcrush {
    t_pxobject ob;
    long ob_in;
    void *ob_proxy; 

    // soundpipe-related
    sp_data *sp;
    sp_bitcrush *bitcrush;

} t_bitcrush;


// method prototypes
void* bitcrush_new(t_symbol* s, long argc, t_atom* argv);
void bitcrush_free(t_bitcrush* x);
void bitcrush_assist(t_bitcrush* x, void* b, long m, long a, char* s);
void bitcrush_float(t_bitcrush* x, double f);
void bitcrush_int(t_bitcrush *x, long l);
void bitcrush_dsp64(t_bitcrush* x, t_object* dsp64, short* count,
                     double samplerate, long maxvectorsize, long flags);
void bitcrush_perform64(t_bitcrush* x, t_object* dsp64, double** ins,
                         long numins, double** outs, long numouts,
                         long sampleframes, long flags, void* userparam);

// global class pointer variable
static t_class* bitcrush_class = NULL;


void ext_main(void* r)
{
    t_class* c = class_new("sp_bitcrush~", (method)bitcrush_new,
                           (method)bitcrush_free, (long)sizeof(t_bitcrush), 0L,
                           A_GIMME, 0);

    class_addmethod(c, (method)bitcrush_float, "float", A_FLOAT, 0);
    class_addmethod(c, (method)bitcrush_int, "int", A_LONG, 0);
    class_addmethod(c, (method)bitcrush_dsp64, "dsp64", A_CANT, 0);
    class_addmethod(c, (method)bitcrush_assist, "assist", A_CANT, 0);

    class_dspinit(c);
    class_register(CLASS_BOX, c);
    bitcrush_class = c;
}


void* bitcrush_new(t_symbol* s, long argc, t_atom* argv)
{
    t_bitcrush* x = (t_bitcrush*)object_alloc(bitcrush_class);

    if (x) {
        dsp_setup((t_pxobject*)x, 1); // i.e. 1 inlet + the default
        outlet_new(x, "signal"); // signal outlet

        // inlet proxy
        x->ob_proxy = proxy_new((t_object *)x, 1, &x->ob_in);

        // parameters as args at object creation
        long bitdepth = atom_getintarg(0, argc, argv);
        float srate = atom_getfloatarg(1, argc, argv);
        
        // module creation
        sp_create(&x->sp);
        sp_bitcrush_create(&x->bitcrush);

        // initialization
        sp_bitcrush_init(x->sp, x->bitcrush);

        x->bitcrush->bitdepth = bitdepth ? bitdepth : 8;        
        x->bitcrush->srate = srate ? srate : 1000.0;
        
    }
    return (x);
}

void bitcrush_free(t_bitcrush* x) {
    dsp_free((t_pxobject *)x);
    sp_bitcrush_destroy(&x->bitcrush);
    sp_destroy(&x->sp);
}

void bitcrush_assist(t_bitcrush *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_OUTLET)
        sprintf(s, "(Signal) output");
    else
    {
        switch (a)
        {
        
        case 0:
            sprintf(s, "(signal/long) inlet 0 (bitdepth)");
            break;
        
        case 1:
            sprintf(s, "(signal/float) inlet 1 (srate)");
            break;
        
        default:
            error("bitcrush_assist switch out-of-index");
        }
    }
}

void bitcrush_float(t_bitcrush *x, double f)
{
    switch (proxy_getinlet((t_object *)x)) {
        
        case 1:
            post("float received in 1th inlet (srate)");
            x->bitcrush->srate = f;
            break;

        default:
            error("bitcrush_float switch out-of-index");   
    }
}

void bitcrush_int(t_bitcrush *x, long l)
{
    switch (proxy_getinlet((t_object *)x)) {
        
        
        case 0:
            post("long received in 0th inlet (bitdepth)");
            x->bitcrush->bitdepth = l;
            break;

        default:
            error("bitcrush_long switch out-of-index");   
    }
}


void bitcrush_dsp64(t_bitcrush* x, t_object* dsp64, short* count,
                     double samplerate, long maxvectorsize, long flags)
{
    object_method(dsp64, gensym("dsp_add64"), x, bitcrush_perform64, 0, NULL);
}


void bitcrush_perform64(t_bitcrush* x, t_object* dsp64, double** ins,
                         long numins, double** outs, long numouts,
                         long sampleframes, long flags, void* userparam)
{
    t_double* inL = ins[0];
    t_double* outL = outs[0];
    long n = sampleframes;

    t_float bitcrush_in = 0, bitcrush_out = 0;

    while (n--) {
        bitcrush_in = *inL++;
        sp_bitcrush_compute(x->sp, x->bitcrush, &bitcrush_in, &bitcrush_out);
        *outL++ = bitcrush_out;
    }
}
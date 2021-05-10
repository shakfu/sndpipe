#include "ext.h"
#include "ext_obex.h"
#include "z_dsp.h"
#include "soundpipe.h"

typedef struct _pshift {
    t_pxobject ob;
    void *ob_proxy_1; // inlet proxy
    void *ob_proxy_2; // inlet proxy

    // soundpipe-related
    sp_data *sp;
    sp_pshift *pshift;

} t_pshift;


// method prototypes
void* pshift_new(t_symbol* s, long argc, t_atom* argv);
void pshift_free(t_pshift* x);
void pshift_assist(t_pshift* x, void* b, long m, long a, char* s);
void pshift_float(t_pshift *x, double f);
void pshift_dsp64(t_pshift* x, t_object* dsp64, short* count,
                     double samplerate, long maxvectorsize, long flags);
void pshift_perform64(t_pshift* x, t_object* dsp64, double** ins,
                         long numins, double** outs, long numouts,
                         long sampleframes, long flags, void* userparam);

// global class pointer variable
static t_class* pshift_class = NULL;


void ext_main(void* r)
{
    t_class* c = class_new("sp_pshift~", (method)pshift_new,
                           (method)pshift_free, (long)sizeof(t_pshift), 0L,
                           A_GIMME, 0);

    class_addmethod(c, (method)pshift_float, "float", A_FLOAT, 0);
    class_addmethod(c, (method)pshift_dsp64, "dsp64", A_CANT, 0);
    class_addmethod(c, (method)pshift_assist, "assist", A_CANT, 0);

    class_dspinit(c);
    class_register(CLASS_BOX, c);
    pshift_class = c;
}


void* pshift_new(t_symbol* s, long argc, t_atom* argv)
{
    t_pshift* x = (t_pshift*)object_alloc(pshift_class);

    if (x) {
        dsp_setup((t_pxobject*)x, 1); // i.e. 1 inlet + the default
        outlet_new(x, "signal"); // signal outlet

        // inlet proxy
        x->ob_proxy_2 = proxy_new(x, 2, NULL);
        x->ob_proxy_1 = proxy_new(x, 1, NULL);

        float shift  = atom_getfloatarg(0, argc, argv);
        float window = atom_getfloatarg(1, argc, argv);
        float xfade  = atom_getfloatarg(2, argc, argv);

        // module creation
        sp_create(&x->sp);
        sp_pshift_create(&x->pshift);

        sp_srand(x->sp, 1234567);

        // initialization
        sp_pshift_init(x->sp, x->pshift);

        *x->pshift->shift = shift ? shift : 7.0;
        *x->pshift->window = window ? window : 500.0;
        *x->pshift->xfade = xfade ? xfade : 250.0;

    }
    return (x);
}

void pshift_free(t_pshift* x) {
    dsp_free((t_pxobject *)x);
    sp_pshift_destroy(&x->pshift);
    sp_destroy(&x->sp);
    object_free(x->ob_proxy_2);
    object_free(x->ob_proxy_1);
}

void pshift_assist(t_pshift *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_OUTLET)
        sprintf(s, "(Signal) output");
    else
    {
        switch (a)
        {
        
        case 0:
            sprintf(s, "(signal/float) inlet 0 (shift)");
            break;
        
        case 1:
            sprintf(s, "(signal/float) inlet 1 (window)");
            break;
        
        case 2:
            sprintf(s, "(signal/float) inlet 2 (xfade)");
            break;
        
        default:
            error("pshift_assist switch out-of-index");
        }
    }
}

void pshift_float(t_pshift *x, double f)
{
    switch (proxy_getinlet((t_object *)x)) {
        case 0:
            // post("float received in 0th inlet (shift)");
            *x->pshift->shift = f;
            break;

        case 1:
            // post("float received in 1th inlet (window)");
            *x->pshift->window = f;
            break;
        
        case 2:
            // post("float received in 2th inlet (xfade)");
            *x->pshift->xfade = f;
            break;
        
        default:
            error("pshift_float switch out-of-index");   
    }
}

// registers a function for the signal chain in Max
void pshift_dsp64(t_pshift* x, t_object* dsp64, short* count,
                     double samplerate, long maxvectorsize, long flags)
{
    
    object_method(dsp64, gensym("dsp_add64"), x, pshift_perform64, 0, NULL);
}


void pshift_perform64(t_pshift* x, t_object* dsp64, double** ins,
                         long numins, double** outs, long numouts,
                         long sampleframes, long flags, void* userparam)
{
    t_double* inL = ins[0];
    t_double* outL = outs[0];
    long n = sampleframes;

    t_float pshift_in = 0, pshift_out = 0;

    while (n--) {
        pshift_in = *inL++;
        sp_pshift_compute(x->sp, x->pshift, &pshift_in, &pshift_out);
        *outL++ = pshift_out;
    }
}
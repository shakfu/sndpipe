#include "ext.h"
#include "ext_obex.h"
#include "z_dsp.h"
#include "soundpipe.h"

typedef struct _compressor {
    t_pxobject ob;
    long ob_in;
    void *ob_proxy; 

    // soundpipe-related
    sp_data *sp;
    sp_compressor *compressor;

} t_compressor;


// method prototypes
void* compressor_new(t_symbol* s, long argc, t_atom* argv);
void compressor_free(t_compressor* x);
void compressor_assist(t_compressor* x, void* b, long m, long a, char* s);
void compressor_float(t_compressor* x, double f);
void compressor_dsp64(t_compressor* x, t_object* dsp64, short* count,
                     double samplerate, long maxvectorsize, long flags);
void compressor_perform64(t_compressor* x, t_object* dsp64, double** ins,
                         long numins, double** outs, long numouts,
                         long sampleframes, long flags, void* userparam);

// global class pointer variable
static t_class* compressor_class = NULL;


void ext_main(void* r)
{
    t_class* c = class_new("sp_compressor~", (method)compressor_new,
                           (method)compressor_free, (long)sizeof(t_compressor), 0L,
                           A_GIMME, 0);

    class_addmethod(c, (method)compressor_float, "float", A_FLOAT, 0);
    class_addmethod(c, (method)compressor_dsp64, "dsp64", A_CANT, 0);
    class_addmethod(c, (method)compressor_assist, "assist", A_CANT, 0);

    class_dspinit(c);
    class_register(CLASS_BOX, c);
    compressor_class = c;
}


void* compressor_new(t_symbol* s, long argc, t_atom* argv)
{
    t_compressor* x = (t_compressor*)object_alloc(compressor_class);

    if (x) {
        dsp_setup((t_pxobject*)x, 3); // param count - 1
        outlet_new(x, "signal");      // signal outlet

        // inlet proxy
        x->ob_proxy = proxy_new((t_object *)x, 1, &x->ob_in);

        // parameters as args at object creation
        // float ratio = atom_getfloatarg(0, argc, argv);
        // float thresh = atom_getfloatarg(1, argc, argv);
        // float atk = atom_getfloatarg(2, argc, argv);
        // float rel = atom_getfloatarg(3, argc, argv);
        
        // module creation
        sp_create(&x->sp);
        sp_compressor_create(&x->compressor);

        // initialization
        sp_compressor_init(x->sp, x->compressor);
        // x->compressor->ratio = (float *)&ratio;
        // x->compressor->thresh = (float *)&thresh;
        // x->compressor->atk = (float *)&atk;
        // x->compressor->rel = (float *)&rel;
    }
    return (x);
}

void compressor_free(t_compressor* x) {
    dsp_free((t_pxobject *)x);
    sp_compressor_destroy(&x->compressor);
    sp_destroy(&x->sp);
    proxy_delete(x->ob_proxy);
}

void compressor_assist(t_compressor *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_OUTLET)
        sprintf(s, "(Signal) output");
    else
    {
        switch (a)
        {
        
        case 0:
            sprintf(s, "(signal/float) inlet 0 (ratio)");
            break;
        
        case 1:
            sprintf(s, "(signal/float) inlet 1 (thresh)");
            break;
        
        case 2:
            sprintf(s, "(signal/float) inlet 2 (atk)");
            break;
        
        case 3:
            sprintf(s, "(signal/float) inlet 3 (rel)");
            break;
        
        default:
            error("compressor_assist switch out-of-index");
        }
    }
}

void compressor_float(t_compressor *x, double f)
{

    switch (proxy_getinlet((t_object *)x)) {
        
        case 0:
            post("float received in 0th inlet (ratio)");
            x->compressor->ratio = (float *)&f;
            break;

        case 1:
            post("float received in 1th inlet (thresh)");
            x->compressor->thresh = (float *)&f;
            break;
        
        case 2:
            post("float received in 2th inlet (atk)");
            x->compressor->atk = (float *)&f;
            break;
        
        case 3:
            post("float received in 3th inlet (rel)");
            x->compressor->rel = (float *)&f;
            break;

        default:
            error("compressor_float switch out-of-index");   
    }
}


// registers a function for the signal chain in Max
void compressor_dsp64(t_compressor* x, t_object* dsp64, short* count,
                     double samplerate, long maxvectorsize, long flags)
{
    //post("my sample rate is: %f", samplerate);
    object_method(dsp64, gensym("dsp_add64"), x, compressor_perform64, 0, NULL);
}


void compressor_perform64(t_compressor* x, t_object* dsp64, double** ins,
                         long numins, double** outs, long numouts,
                         long sampleframes, long flags, void* userparam)
{
    t_double* inL = ins[0];
    t_double* outL = outs[0];
    long n = sampleframes;

    t_float compressor_in = 0, compressor_out = 0;

    while (n--) {
        compressor_in = *inL++;
        sp_compressor_compute(x->sp, x->compressor, &compressor_in, &compressor_out);
        *outL++ = compressor_out;
    }
}
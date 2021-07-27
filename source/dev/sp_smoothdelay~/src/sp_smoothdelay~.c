#include "ext.h"
#include "ext_obex.h"
#include "z_dsp.h"
#include "soundpipe.h"

typedef struct _smoothdelay {
    t_pxobject ob;
    long ob_in;
    void *ob_proxy; 

    // soundpipe-related
    sp_data *sp;
    sp_smoothdelay *smoothdelay;

} t_smoothdelay;


// method prototypes
void* smoothdelay_new(t_symbol* s, long argc, t_atom* argv);
void smoothdelay_free(t_smoothdelay* x);
void smoothdelay_assist(t_smoothdelay* x, void* b, long m, long a, char* s);
void smoothdelay_float(t_smoothdelay* x, double f);
void smoothdelay_int(t_smoothdelay *x, long l);
void smoothdelay_dsp64(t_smoothdelay* x, t_object* dsp64, short* count,
                     double samplerate, long maxvectorsize, long flags);
void smoothdelay_perform64(t_smoothdelay* x, t_object* dsp64, double** ins,
                         long numins, double** outs, long numouts,
                         long sampleframes, long flags, void* userparam);

// global class pointer variable
static t_class* smoothdelay_class = NULL;


void ext_main(void* r)
{
    t_class* c = class_new("sp_smoothdelay~", (method)smoothdelay_new,
                           (method)smoothdelay_free, (long)sizeof(t_smoothdelay), 0L,
                           A_GIMME, 0);

    class_addmethod(c, (method)smoothdelay_float, "float", A_FLOAT, 0);
    class_addmethod(c, (method)smoothdelay_int, "int", A_LONG, 0);
    class_addmethod(c, (method)smoothdelay_dsp64, "dsp64", A_CANT, 0);
    class_addmethod(c, (method)smoothdelay_assist, "assist", A_CANT, 0);

    class_dspinit(c);
    class_register(CLASS_BOX, c);
    smoothdelay_class = c;
}


void* smoothdelay_new(t_symbol* s, long argc, t_atom* argv)
{
    t_smoothdelay* x = (t_smoothdelay*)object_alloc(smoothdelay_class);

    if (x) {
        dsp_setup((t_pxobject*)x, 3); // i.e. 1 inlet + the default
        outlet_new(x, "signal"); // signal outlet

        // inlet proxy
        x->ob_proxy = proxy_new((t_object *)x, 1, &x->ob_in);

        // parameters as args at object creation
        
        float maxdel = atom_getfloatarg(0, argc, argv);

        long maxcount = atom_getintarg(1, argc, argv);

        float feedback = atom_getfloatarg(2, argc, argv);
        
        float del = atom_getfloatarg(3, argc, argv);
        

        // module creation
        sp_create(&x->sp);
        sp_smoothdelay_create(&x->smoothdelay);

        // initialization
        sp_smoothdelay_init(x->sp, x->smoothdelay, maxdel, maxcount);

        
        x->smoothdelay->maxdel = maxdel ? maxdel : 1.0;

        x->smoothdelay->maxcount = maxcount ? maxcount : 1024;

        x->smoothdelay->feedback = feedback ? feedback : 0.1;
        
        x->smoothdelay->del = del ? del : 0.5;
        
    }
    return (x);
}

void smoothdelay_free(t_smoothdelay* x) {
    dsp_free((t_pxobject *)x);
    sp_smoothdelay_destroy(&x->smoothdelay);
    sp_destroy(&x->sp);
}

void smoothdelay_assist(t_smoothdelay *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_OUTLET)
        sprintf(s, "(Signal) output");
    else
    {
        switch (a)
        {
        
        case 0:
            sprintf(s, "(signal/float) inlet 0 (maxdel)");
            break;
        
        case 1:
            sprintf(s, "(signal/long) inlet 1 (interp)");
            break;
        
        case 2:
            sprintf(s, "(signal/float) inlet 2 (feedback)");
            break;
        
        case 3:
            sprintf(s, "(signal/float) inlet 3 (del)");
            break;
        
        default:
            error("smoothdelay_assist switch out-of-index");
        }
    }
}

void smoothdelay_float(t_smoothdelay *x, double f)
{
    switch (proxy_getinlet((t_object *)x)) {
        
        
        case 0:
            post("float received in 0th inlet (maxdel)");
            x->smoothdelay->maxdel = f;
            break;
        
        case 2:
            post("float received in 2th inlet (feedback)");
            x->smoothdelay->feedback = f;
            break;

        case 3:
            post("float received in 3th inlet (del)");
            x->smoothdelay->del = f;
            break;

        default:
            error("smoothdelay_float switch out-of-index");   
    }
}

void smoothdelay_int(t_smoothdelay *x, long l)
{
    switch (proxy_getinlet((t_object *)x)) {
        
        case 1:
            post("long received in 1th inlet (interp)");
            x->smoothdelay->maxcount = l;
            break;
        
        default:
            error("smoothdelay_long switch out-of-index");   
    }
}


// registers a function for the signal chain in Max
void smoothdelay_dsp64(t_smoothdelay* x, t_object* dsp64, short* count,
                     double samplerate, long maxvectorsize, long flags)
{
    //post("my sample rate is: %f", samplerate);
    object_method(dsp64, gensym("dsp_add64"), x, smoothdelay_perform64, 0, NULL);
}


void smoothdelay_perform64(t_smoothdelay* x, t_object* dsp64, double** ins,
                         long numins, double** outs, long numouts,
                         long sampleframes, long flags, void* userparam)
{
    t_double* inL = ins[0];
    t_double* outL = outs[0];
    long n = sampleframes;

    t_float smoothdelay_in = 0, smoothdelay_out = 0;

    while (n--) {
        smoothdelay_in = *inL++;
        sp_smoothdelay_compute(x->sp, x->smoothdelay, &smoothdelay_in, &smoothdelay_out);
        *outL++ = smoothdelay_out;
    }
}
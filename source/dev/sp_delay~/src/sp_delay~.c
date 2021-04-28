#include "ext.h"
#include "ext_obex.h"
#include "z_dsp.h"
#include "soundpipe.h"


typedef struct _delay {
    t_pxobject ob;
    long ob_in;
    void *ob_proxy;

    // soundpipe-related
    sp_data *sp;
    sp_delay *delay;

} t_delay;


// method prototypes
void* delay_new(t_symbol* s, long argc, t_atom* argv);
void delay_free(t_delay* x);
void delay_assist(t_delay* x, void* b, long m, long a, char* s);
void delay_float(t_delay* x, double f);
void delay_dsp64(t_delay* x, t_object* dsp64, short* count,
                     double samplerate, long maxvectorsize, long flags);
void delay_perform64(t_delay* x, t_object* dsp64, double** ins,
                         long numins, double** outs, long numouts,
                         long sampleframes, long flags, void* userparam);


// global class pointer variable
static t_class* delay_class = NULL;


void ext_main(void* r)
{
    t_class* c = class_new("sp_delay~", (method)delay_new,
                           (method)delay_free, (long)sizeof(t_delay), 0L,
                           A_GIMME, 0);

    class_addmethod(c, (method)delay_float, "float", A_FLOAT, 0);
    class_addmethod(c, (method)delay_dsp64, "dsp64", A_CANT, 0);
    class_addmethod(c, (method)delay_assist, "assist", A_CANT, 0);

    class_dspinit(c);
    class_register(CLASS_BOX, c);
    delay_class = c;
}


void* delay_new(t_symbol* s, long argc, t_atom* argv)
{
    t_delay* x = (t_delay*)object_alloc(delay_class);

    if (x) {
        dsp_setup((t_pxobject*)x, 1); // i.e. 1 inlet + the default
        outlet_new(x, "signal");

        // inlet proxy
        x->ob_proxy = proxy_new((t_object *)x, 1, &x->ob_in);

        // parameters
        // as args at object creation
        float time = atom_getfloatarg(0, argc, argv);
        float feedback = atom_getfloatarg(1, argc, argv);
    
        // module creation
        sp_create(&x->sp);
        sp_delay_create(&x->delay);

        // initialization
        sp_delay_init(x->sp, x->delay, 1.0);

        x->delay->time = time ? time : 1.0;
        x->delay->feedback = feedback ? feedback : 0.0;
    }
    return (x);
}


void delay_free(t_delay* x) {
    dsp_free((t_pxobject *)x);
    sp_delay_destroy(&x->delay);
    sp_destroy(&x->sp);
}


void delay_assist(t_delay *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_OUTLET)
        sprintf(s, "(Signal) output");
    else
    {
        switch (a)
        {
        
        case 0:
            sprintf(s, "(signal/float) inlet 0 (time)");
            break;
        
        case 1:
            sprintf(s, "(signal/float) inlet 1 (feedback)");
            break;
        
        default:
            error("delay_assist switch out-of-index");
        }
    }
}


void delay_float(t_delay *x, double f)
{
    switch (proxy_getinlet((t_object *)x)) {
        
        case 0:
            post("float received in 0th inlet (time)");
            x->delay->time = f;
            break;
        
        case 1:
            post("float received in 1th inlet (feedback)");
            x->delay->feedback = f;
            break;
        
        default:
            error("delay_float switch out-of-index");
    }
}


void delay_dsp64(t_delay* x, t_object* dsp64, short* count,
                     double samplerate, long maxvectorsize, long flags)
{
    post("my sample rate is: %f", samplerate);
    object_method(dsp64, gensym("dsp_add64"), x, delay_perform64, 0, NULL);
}


void delay_perform64(t_delay* x, t_object* dsp64, double** ins,
                     long numins, double** outs, long numouts,
                     long sampleframes, long flags, void* userparam)
{
    t_double* inL = ins[0];
    t_double* outL = outs[0];
    long n = sampleframes;

    t_float delay_in = 0, delay_out = 0;

    while (n--) {
        delay_in = *inL++;
        sp_delay_compute(x->sp, x->delay, &delay_in, &delay_out);
        *outL++ = delay_out;
    }
}

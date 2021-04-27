#include "ext.h"
#include "ext_obex.h"
#include "z_dsp.h"
#include "soundpipe.h"

typedef struct _bigverb {
    t_pxobject ob;
    long ob_in; // holds the inlet number used by all parties
    void *ob_proxy; 

    // soundpipe-related
    sp_data *sp; // soundpipe data
    sp_bigverb *rev;

} t_bigverb;


// method prototypes
void* bigverb_new(t_symbol* s, long argc, t_atom* argv);
void bigverb_free(t_bigverb* x);
void bigverb_assist(t_bigverb* x, void* b, long m, long a, char* s);
void bigverb_float(t_bigverb* x, double f);
void bigverb_dsp64(t_bigverb* x, t_object* dsp64, short* count,
                     double samplerate, long maxvectorsize, long flags);
void bigverb_perform64(t_bigverb* x, t_object* dsp64, double** ins,
                         long numins, double** outs, long numouts,
                         long sampleframes, long flags, void* userparam);


// global class pointer variable
static t_class* bigverb_class = NULL;


//***********************************************************************************************

void ext_main(void* r)
{
    t_class* c = class_new("sp_bigverb~", (method)bigverb_new,
                           (method)bigverb_free, (long)sizeof(t_bigverb), 0L,
                           A_GIMME, 0);

    class_addmethod(c, (method)bigverb_float, "float", A_FLOAT, 0);
    class_addmethod(c, (method)bigverb_dsp64, "dsp64", A_CANT, 0);
    class_addmethod(c, (method)bigverb_assist, "assist", A_CANT, 0);

    class_dspinit(c);
    class_register(CLASS_BOX, c);
    bigverb_class = c;
}


void* bigverb_new(t_symbol* s, long argc, t_atom* argv)
{
    t_bigverb* x = (t_bigverb*)object_alloc(bigverb_class);

    if (x) {
        dsp_setup((t_pxobject*)x, 2); // i.e. 1 inlet + the default
        outlet_new(x, "signal"); // signal outlet (note "signal" rather than NULL)
        outlet_new(x, "signal");

        // inlet proxy
        x->ob_proxy = proxy_new((t_object *)x, 1, &x->ob_in);

        // parameters
        // get entry at object creation
        float revfb = atom_getfloatarg(7, argc, argv);  // rev fb 
        float revlpf = atom_getfloatarg(8, argc, argv); // rev flpf

        // module creation
        sp_create(&x->sp);

        sp_bigverb_create(&x->rev);

        // initialization
        sp_bigverb_init(x->sp, x->rev);

        x->rev->feedback = revfb ? revfb : 0.5;
        x->rev->lpfreq = revlpf ? revlpf : 10000.0;
    }
    return (x);
}


void bigverb_free(t_bigverb* x) {
    dsp_free((t_pxobject *)x);

    sp_bigverb_destroy(&x->rev);

    sp_destroy(&x->sp);
}

void bigverb_assist(t_bigverb *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_OUTLET)
        sprintf(s, "(Signal) output");
    else
    {
        switch (a)
        {
        case 0:
            sprintf(s, "(Signal/Float) Left inlet (RevFb)");
            break;
        case 1:
            sprintf(s, "(Signal/Float) (RevLpFreq)");
            break;
        }
    }
}

void bigverb_float(t_bigverb *x, double f)
{
    switch (proxy_getinlet((t_object *)x)) {
        case 0:
            post("float received in 7th inlet (RevFB)");
            x->rev->feedback = f;
            break;
        case 1:
            post("float received in 8th inlet (RevLpFreq)");
            x->rev->lpfreq = f;
            break;
        default:
            post("wrong inlet!!");
            break;
    }
}


// registers a function for the signal chain in Max
void bigverb_dsp64(t_bigverb* x, t_object* dsp64, short* count,
                     double samplerate, long maxvectorsize, long flags)
{
    post("my sample rate is: %f", samplerate);
    object_method(dsp64, gensym("dsp_add64"), x, bigverb_perform64, 0, NULL);
}


void bigverb_perform64(t_bigverb* x, t_object* dsp64, double** ins,
                         long numins, double** outs, long numouts,
                         long sampleframes, long flags, void* userparam)
{
    t_double *inL = ins[0];   // we get audio for each inlet of the object from the **ins argument
    t_double *inR = ins[1];   // we get audio for each inlet of the object from the **ins argument
    t_double *outL = outs[0]; // we get audio for each outlet of the object from the **outs argument
    t_double *outR = outs[1]; // we get audio for each outlet of the object from the **outs argument

    long n = sampleframes;

    // local soundpipe vars
    t_float leftin = 0, leftout = 0, rightin = 0, rightout = 0;

    while (n--) {
        // leftin = *inL;
        t_float leftin = *inL++;
        t_float rightin = *inR++;

        sp_bigverb_compute(x->sp, x->rev, &leftin, &rightin, &leftout, &rightout);

        *outL++ = leftout;
        *outR++ = rightout;

        // *outL++ = leftin;
        // *outR++ = rightin;
    }
}

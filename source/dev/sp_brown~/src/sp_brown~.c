#include "ext.h"
#include "ext_obex.h"
#include "z_dsp.h"
#include "soundpipe.h"

typedef struct _brown {
    t_pxobject ob;
    double offset;      // offset the signal
    sp_brown *brown;    // soundpipe brownian motion modulel
    sp_data *sp;        // soundpipe
    float brownian;     // value of brownian motion
} t_brown;


// method prototypes
void* brown_new(t_symbol* s, long argc, t_atom* argv);
void brown_free(t_brown* x);
void brown_assist(t_brown* x, void* b, long m, long a, char* s);
void brown_float(t_brown* x, double f);
void brown_dsp64(t_brown* x, t_object* dsp64, short* count,
                     double samplerate, long maxvectorsize, long flags);
void brown_perform64(t_brown* x, t_object* dsp64, double** ins,
                         long numins, double** outs, long numouts,
                         long sampleframes, long flags, void* userparam);


// global class pointer variable
static t_class* brown_class = NULL;


//***********************************************************************************************

void ext_main(void* r)
{
    t_class* c = class_new("sp_brown~", (method)brown_new,
                           (method)brown_free, (long)sizeof(t_brown), 0L,
                           A_GIMME, 0);

    class_addmethod(c, (method)brown_float, "float", A_FLOAT, 0);
    class_addmethod(c, (method)brown_dsp64, "dsp64", A_CANT, 0);
    class_addmethod(c, (method)brown_assist, "assist", A_CANT, 0);

    class_dspinit(c);
    class_register(CLASS_BOX, c);
    brown_class = c;
}


void* brown_new(t_symbol* s, long argc, t_atom* argv)
{
    t_brown* x = (t_brown*)object_alloc(brown_class);

    if (x) {
        dsp_setup((t_pxobject*)x,
                  1); // MSP inlets: arg is # of inlets and is REQUIRED!
                      // use 0 if you don't need inlets
        outlet_new(x,
                   "signal"); // signal outlet (note "signal" rather than NULL)
        x->offset = 0.0;
        x->brownian = 0.0;
        sp_brown_create(&x->brown);
        sp_create(&x->sp);
        sp_srand(x->sp, 0);
        sp_brown_init(x->sp, x->brown);
    }
    return (x);
}


void brown_free(t_brown* x) {
    dsp_free((t_pxobject *)x);
    sp_brown_destroy(&x->brown);
    sp_destroy(&x->sp);
}


void brown_assist(t_brown* x, void* b, long m, long a, char* s)
{
    if (m == ASSIST_INLET) { // inlet
        sprintf(s, "I am inlet %ld", a);
    } else { // outlet
        sprintf(s, "I am outlet %ld", a);
    }
}


void brown_float(t_brown* x, double f) { x->offset = f; }


// registers a function for the signal chain in Max
void brown_dsp64(t_brown* x, t_object* dsp64, short* count,
                     double samplerate, long maxvectorsize, long flags)
{
    post("my sample rate is: %f", samplerate);

    object_method(dsp64, gensym("dsp_add64"), x, brown_perform64, 0, NULL);
}


// this is the 64-bit perform method audio vectors
void brown_perform64(t_brown* x, t_object* dsp64, double** ins,
                         long numins, double** outs, long numouts,
                         long sampleframes, long flags, void* userparam)
{
    t_double* inL = ins[0];   // we get audio for each inlet of the object from the **ins argument
    t_double* outL = outs[0]; // we get audio for each outlet of the object from the **outs argument
    long n = sampleframes;
    // this perform method simply copies the input to the output, offsetting
    // the value
    while (n--) {
        sp_brown_compute(x->sp, x->brown, NULL, &x->brownian);

        *outL++ = *inL++ + x->brownian + x->offset;
    }
    
}

#include "ext.h"
#include "ext_obex.h"
#include "z_dsp.h"
#include "soundpipe.h"

typedef struct _sndpipe {
    t_pxobject ob;
    long ob_in; // holds the inlet number used by all parties
    void *ob_proxy; 

    // soundpipe-related
    sp_data *sp; // soundpipe data
    sp_delay *delay;
    sp_osc *osc;
    sp_metro *met;
    sp_tenv *tenv;
    sp_ftbl *ft;
    sp_bigverb *rev;

} t_sndpipe;


// method prototypes
void* sndpipe_new(t_symbol* s, long argc, t_atom* argv);
void sndpipe_free(t_sndpipe* x);
void sndpipe_assist(t_sndpipe* x, void* b, long m, long a, char* s);
void sndpipe_float(t_sndpipe* x, double f);
void sndpipe_type(t_sndpipe *x, t_symbol *s);
void sndpipe_dsp64(t_sndpipe* x, t_object* dsp64, short* count,
                     double samplerate, long maxvectorsize, long flags);
void sndpipe_perform64(t_sndpipe* x, t_object* dsp64, double** ins,
                         long numins, double** outs, long numouts,
                         long sampleframes, long flags, void* userparam);


// global class pointer variable
static t_class* sndpipe_class = NULL;


//***********************************************************************************************

void ext_main(void* r)
{
    t_class* c = class_new("sndpipe~", (method)sndpipe_new,
                           (method)sndpipe_free, (long)sizeof(t_sndpipe), 0L,
                           A_GIMME, 0);

    class_addmethod(c, (method)sndpipe_float, "float", A_FLOAT, 0);
    class_addmethod(c, (method)sndpipe_dsp64, "dsp64", A_CANT, 0);
    class_addmethod(c, (method)sndpipe_assist, "assist", A_CANT, 0);
    class_addmethod(c, (method)sndpipe_type, "type", A_SYM, 0);

    class_dspinit(c);
    class_register(CLASS_BOX, c);
    sndpipe_class = c;
}


void* sndpipe_new(t_symbol* s, long argc, t_atom* argv)
{
    t_sndpipe* x = (t_sndpipe*)object_alloc(sndpipe_class);

    if (x) {
        dsp_setup((t_pxobject*)x, 8); // i.e. 1 inlet + the default
        outlet_new(x, "signal"); // signal outlet (note "signal" rather than NULL)
        // outlet_new(x, "signal");

        // inlet proxy
        x->ob_proxy = proxy_new((t_object *)x, 1, &x->ob_in);

        // parameters
        // get entry at object creation
        float freq = atom_getfloatarg(0, argc, argv);   // osc freq
        float amp = atom_getfloatarg(1, argc, argv);    // osc amp
        float dt = atom_getfloatarg(2, argc, argv);     // delay time
        float fb = atom_getfloatarg(3, argc, argv);     // delay feedback
        float atk = atom_getfloatarg(4, argc, argv);    // env attack
        float hold = atom_getfloatarg(5, argc, argv);   // env hold
        float rel = atom_getfloatarg(6, argc, argv);    // env release
        float revfb = atom_getfloatarg(7, argc, argv);  // rev fb 
        float revlpf = atom_getfloatarg(8, argc, argv); // rev flpf

        // module creation
        sp_create(&x->sp);
        sp_srand(x->sp, 123456); // random seed

        sp_ftbl_create(x->sp, &x->ft, 2048);
        sp_osc_create(&x->osc);
        sp_delay_create(&x->delay);
        sp_metro_create(&x->met);
        sp_tenv_create(&x->tenv);
        sp_bigverb_create(&x->rev);

        // initialization
        sp_delay_init(x->sp, x->delay, dt ? dt : 0.75 * 0.5);
        sp_gen_sine(x->sp, x->ft);
        sp_osc_init(x->sp, x->osc, x->ft, 0.0);
        sp_metro_init(x->sp, x->met);
        sp_tenv_init(x->sp, x->tenv);
        sp_bigverb_init(x->sp, x->rev);

        x->osc->freq = freq ? freq : 440.0;
        x->osc->amp = amp ? amp : 0.2;
        x->delay->feedback = fb ? fb : 0.5;
        x->met->freq = freq ? freq : 1.0;
        x->tenv->atk = atk ? atk : 0.005;
        x->tenv->hold = hold ? hold : 0.1;
        x->tenv->rel = rel ? rel : 0.1;
        x->rev->feedback = revfb ? revfb : 0.5;
        x->rev->lpfreq = revlpf ? revlpf : 10000.0;
    }
    return (x);
}


void sndpipe_free(t_sndpipe* x) {
    dsp_free((t_pxobject *)x);

    sp_delay_destroy(&x->delay);
    sp_ftbl_destroy(&x->ft);
    sp_osc_destroy(&x->osc);
    sp_metro_destroy(&x->met);
    sp_tenv_destroy(&x->tenv);
    sp_bigverb_destroy(&x->rev);

    sp_destroy(&x->sp);
}

void sndpipe_assist(t_sndpipe *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_OUTLET)
        sprintf(s, "(Signal) output");
    else
    {
        switch (a)
        {
        case 0:
            sprintf(s, "(Signal/Float) Left inlet (OscFreq)");
            break;
        case 1:
            sprintf(s, "(Signal/Float) (OscAmp)");
            break;
        case 2:
            sprintf(s, "(Signal/Float) (DelayTime)");
            break;
        case 3:
            sprintf(s, "(Signal/Float) (DelayFb)");
            break;
        case 4:
            sprintf(s, "(Signal/Float) (EnvAtk)");
            break;
        case 5:
            sprintf(s, "(Signal/Float) (EnvHold)");
            break;
        case 6:
            sprintf(s, "(Signal/Float) (EnvRelease)");
            break;
        case 7:
            sprintf(s, "(Signal/Float) (RevFb)");
            break;
        case 8:
            sprintf(s, "(Signal/Float) (RevLpFreq)");
            break;
        }
    }
}

void sndpipe_float(t_sndpipe *x, double f)
{
    switch (proxy_getinlet((t_object *)x)) {
        case 0:
            post("float received in 0th inlet (OscFreq)");
            x->osc->freq = f;
            break;
        case 1:
            post("float received in 1st inlet (OscAmp)");
            x->osc->amp = f;
            break;
        case 2:
            post("float received in 2nd inlet (DelayTime)");
            x->delay->time = f;
            break;
        case 3:
            post("float received in 3rd inlet (DelayFB)");
            x->delay->feedback = f;
            break;
        case 4:
            post("float received in 4th inlet (EnvAtk)");
            x->tenv->atk = f;
            break;
        case 5:
            post("float received in 5th inlet (EnvHold)");
            x->tenv->hold = f;
            break;
        case 6:
            post("float received in 6th inlet (EnvRel)");
            x->tenv->rel = f;
            break;
        case 7:
            post("float received in 7th inlet (RevFB)");
            x->rev->feedback = f;
            break;
        case 8:
            post("float received in 8th inlet (RevLpFreq)");
            x->rev->lpfreq = f;
            break;
        default:
            post("wrong inlet!!");
            break;
    }
}


void sndpipe_type(t_sndpipe *x, t_symbol *s) {
    post("type: %s", s->s_name);
    
    if (s == gensym("triangle"))
    {
        sp_gen_triangle(x->sp, x->ft);
    }
    else
    {
        sp_gen_sine(x->sp, x->ft);
    }
}

// registers a function for the signal chain in Max
void sndpipe_dsp64(t_sndpipe* x, t_object* dsp64, short* count,
                     double samplerate, long maxvectorsize, long flags)
{
    post("my sample rate is: %f", samplerate);
    object_method(dsp64, gensym("dsp_add64"), x, sndpipe_perform64, 0, NULL);
}


void sndpipe_perform64(t_sndpipe* x, t_object* dsp64, double** ins,
                         long numins, double** outs, long numouts,
                         long sampleframes, long flags, void* userparam)
{
    t_double* inL = ins[0];   // we get audio for each inlet of the object from the **ins argument
    t_double* outL = outs[0]; // we get audio for each outlet of the object from the **outs argument
    long n = sampleframes;

    // local soundpipe vars
    float osc = 0, delay = 0, met = 0, tenv = 0;
    float leftin = 0, leftout = 0, rightin = 0, rightout = 0;

    while (n--) {

        sp_metro_compute(x->sp, x->met, NULL, &met);
        sp_tenv_compute(x->sp, x->tenv, &met, &tenv);
        if (met)
        {
            x->osc->freq = 100 + rand() % 500;
        }
        sp_osc_compute(x->sp, x->osc, NULL, &osc);
        osc *= tenv;
        sp_delay_compute(x->sp, x->delay, &osc, &delay);

        leftin = osc + delay;

        sp_bigverb_compute(x->sp, x->rev, &leftin, &rightin, &leftout, &rightout);

        *outL++ = *inL++ + leftout;
        // *outL++ = *inL++ + osc + delay;
    }
}

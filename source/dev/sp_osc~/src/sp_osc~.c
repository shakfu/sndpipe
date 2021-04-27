#include "ext.h"
#include "ext_obex.h"
#include "z_dsp.h"
#include "soundpipe.h"

typedef struct _sposc
{
    t_pxobject ob;
    long ob_in; // holds the inlet number used by all parties
    void *ob_proxy;

    // soundpipe-related
    sp_data *sp; // soundpipe data
    sp_osc *osc; // soundpipe osc
    sp_ftbl *ft; // soundpipe wavetable

    float output; // osc value
} t_sposc;

// method prototypes
void *sndpipe_new(t_symbol *s, long argc, t_atom *argv);
void sndpipe_free(t_sposc *x);
void sndpipe_assist(t_sposc *x, void *b, long m, long a, char *s);
void sndpipe_float(t_sposc *x, double f);
void sndpipe_type(t_sposc *x, t_symbol *s);
void sndpipe_dsp64(t_sposc *x, t_object *dsp64, short *count,
                   double samplerate, long maxvectorsize, long flags);
void sndpipe_perform64(t_sposc *x, t_object *dsp64, double **ins,
                       long numins, double **outs, long numouts,
                       long sampleframes, long flags, void *userparam);

// global class pointer variable
static t_class *sndpipe_class = NULL;

//***********************************************************************************************

void ext_main(void *r)
{
    t_class *c = class_new("sp_osc~", (method)sndpipe_new,
                           (method)sndpipe_free, (long)sizeof(t_sposc), 0L,
                           A_GIMME, 0);

    class_addmethod(c, (method)sndpipe_float, "float", A_FLOAT, 0);
    class_addmethod(c, (method)sndpipe_dsp64, "dsp64", A_CANT, 0);
    class_addmethod(c, (method)sndpipe_assist, "assist", A_CANT, 0);
    class_addmethod(c, (method)sndpipe_type, "type", A_SYM, 0);

    class_dspinit(c);
    class_register(CLASS_BOX, c);
    sndpipe_class = c;
}

void *sndpipe_new(t_symbol *s, long argc, t_atom *argv)
{
    t_sposc *x = (t_sposc *)object_alloc(sndpipe_class);

    if (x)
    {
        dsp_setup((t_pxobject *)x, 1); // i.e. 1 inlet + the default
        outlet_new(x, "signal");       // signal outlet (note "signal" rather than NULL)
        // inlet proxy
        x->ob_proxy = proxy_new((t_object *)x, 1, &x->ob_in);

        // parameters
        // get entry at object creation
        float freq = atom_getfloatarg(0, argc, argv);
        float amp = atom_getfloatarg(1, argc, argv);

        // init output
        x->output = 0.0;

        sp_create(&x->sp);
        sp_ftbl_create(x->sp, &x->ft, 2048);
        sp_osc_create(&x->osc);

        sp_osc_init(x->sp, x->osc, x->ft, 0.0);
        sp_gen_sine(x->sp, x->ft); // default

        x->osc->freq = freq ? freq : 440.0;
        x->osc->amp = amp ? amp : 1.0;
        post("osc freq: %02f amp: %02f", x->osc->freq, x->osc->amp);
    }
    return (x);
}

void sndpipe_free(t_sposc *x)
{
    dsp_free((t_pxobject *)x);
    sp_osc_destroy(&x->osc);
    sp_ftbl_destroy(&x->ft);
    sp_destroy(&x->sp);
}

void sndpipe_assist(t_sposc *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_OUTLET)
        sprintf(s, "(Signal) output");
    else
    {
        switch (a)
        {
        case 0:
            sprintf(s, "(Signal/Float) Left Inlet (Freq)");
            break;
        case 1:
            sprintf(s, "(Signal/Float) Right Inlet (Amp)");
            break;
        }
    }
}

void sndpipe_float(t_sposc *x, double f)
{
    switch (proxy_getinlet((t_object *)x))
    {
    case 0:
        post("float received in left inlet (Freq)");
        x->osc->freq = f;
        break;
    case 1:
        post("float received in right inlet (Amp)");
        x->osc->amp = f;
        break;
    default:
        post("wrong inlet!!");
        break;
    }
}

void sndpipe_type(t_sposc *x, t_symbol *s)
{
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
void sndpipe_dsp64(t_sposc *x, t_object *dsp64, short *count,
                   double samplerate, long maxvectorsize, long flags)
{
    post("my sample rate is: %f", samplerate);
    object_method(dsp64, gensym("dsp_add64"), x, sndpipe_perform64, 0, NULL);
}

void sndpipe_perform64(t_sposc *x, t_object *dsp64, double **ins,
                       long numins, double **outs, long numouts,
                       long sampleframes, long flags, void *userparam)
{
    t_double *inL = ins[0];   // we get audio for each inlet of the object from the **ins argument
    t_double *outL = outs[0]; // we get audio for each outlet of the object from the **outs argument
    long n = sampleframes;

    while (n--)
    {
        sp_osc_compute(x->sp, x->osc, NULL, &x->output);
        *outL++ = *inL++ + x->output;
    }
}

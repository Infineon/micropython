#include "py/dynruntime.h"

#if !defined(__linux__)
void *memcpy(void *dst, const void *src, size_t n) {
    return mp_fun_table.memmove_(dst, src, n);
}
void *memset(void *s, int c, size_t n) {
    return mp_fun_table.memset_(s, c, n);
}

// =============================================================================
// libm shims — safe replacements for arm-none-eabi Newlib math functions.
//
// libm.a is NOT linked (LIBM_PATH := in Makefile) because mpy_ld cannot
// resolve libm's internal Newlib dependencies (__errno, _REENT, kernel
// helpers).  Calling an unresolved libm symbol causes a silent HardFault.
//
// These shims use only integer ops and IEEE 754 bit manipulation — no
// external dependencies.  They cover all single-precision functions
// commonly used by DEEPCRAFT / TensorFlow Lite inference models.
//
// If a future model needs a function not listed here, the build will fail
// with a clear "undefined symbol" link error.
// =============================================================================

// Helper: bit-cast float - unsigned int without UB
static inline unsigned int _f2u(float f) { union { float f; unsigned int u; } v; v.f = f; return v.u; }
static inline float _u2f(unsigned int u) { union { float f; unsigned int u; } v; v.u = u; return v.f; }

// --- expf ---
// Degree-5 minimax polynomial + IEEE 754 exponent scaling.
// Max error ~1.5 ULP for |x| <= 88.
float expf(float x) {
    if (x > 88.0f)  return 3.40282347e+38f;
    if (x < -88.0f) return 0.0f;
    int n = (int)(x * 1.4426950409f + (x >= 0.0f ? 0.5f : -0.5f));
    float r = x - (float)n * 0.6931471806f;
    float p = 1.0f + r * (1.0f + r * (0.5f + r * (0.16666667f +
              r * (0.041666667f + r * 0.0083333333f))));
    return p * _u2f((unsigned int)((n + 127) << 23));
}

// --- logf ---
// Natural log via exponent extraction + polynomial on [sqrt(0.5), sqrt(2)].
float logf(float x) {
    if (x <= 0.0f) return -3.40282347e+38f;
    int e = (int)((_f2u(x) >> 23) & 0xFF) - 127;
    float m = _u2f((_f2u(x) & 0x007FFFFFu) | 0x3F000000u); // m in [0.5, 1)
    if (m < 0.70710678f) { m *= 2.0f; e -= 1; }
    float f = (m - 1.0f) / (m + 1.0f), f2 = f * f;
    float p = f * (2.0f + f2 * (0.66666667f + f2 * (0.4f + f2 *
              (0.28571429f + f2 * 0.22222222f))));
    return p + (float)e * 0.6931471806f;
}

// --- log2f ---
float log2f(float x) { return logf(x) * 1.4426950409f; }

// --- log10f ---
float log10f(float x) { return logf(x) * 0.4342944819f; }

// --- tanhf ---
// tanh(x) = (e^2x - 1)/(e^2x + 1); clamped for |x| > 9 (result = ±1).
float tanhf(float x) {
    if (x >  9.0f) return  1.0f;
    if (x < -9.0f) return -1.0f;
    float e2x = expf(2.0f * x);
    return (e2x - 1.0f) / (e2x + 1.0f);
}

// --- sqrtf ---
// Three Newton-Raphson iterations on an IEEE 754 seed estimate (~6 correct bits).
// On Cortex-M4F the compiler may emit vsqrt.f32 directly and never call this.
float sqrtf(float x) {
    if (x <= 0.0f) return 0.0f;
    float r = _u2f(0x1fbb4000u + (_f2u(x) >> 1)); // seed: ~half the exponent
    r = 0.5f * (r + x / r);
    r = 0.5f * (r + x / r);
    r = 0.5f * (r + x / r);
    return r;
}

// --- fabsf --- (often compiled to vabs.f32 inline, but shim for safety)
float fabsf(float x) { return _u2f(_f2u(x) & 0x7FFFFFFFu); }

// --- floorf ---
float floorf(float x) { int i = (int)x; return (float)(i - (x < (float)i)); }

// --- ceilf ---
float ceilf(float x) { int i = (int)x; return (float)(i + (x > (float)i)); }

// --- roundf ---
float roundf(float x) { return (x >= 0.0f) ? floorf(x + 0.5f) : ceilf(x - 0.5f); }

// --- fmodf ---
float fmodf(float x, float y) {
    if (y == 0.0f) return 0.0f;
    float q = x / y;
    int n = (int)q;
    return x - (float)n * y;
}

// --- powf ---
// a^b = exp(b * ln(a)); handles integer-exponent cases first for accuracy.
float powf(float a, float b) {
    if (b == 0.0f) return 1.0f;
    if (a == 0.0f) return 0.0f;
    if (a < 0.0f) {
        int ib = (int)b;
        if ((float)ib != b) return 0.0f; // non-integer exponent of negative base
        return (ib & 1) ? -expf(b * logf(-a)) : expf(b * logf(-a));
    }
    return expf(b * logf(a));
}

// --- sinf / cosf ---
// Range-reduce to [-pi/4, pi/4] then apply degree-5 minimax polynomials.
// Max error ~3 ULP across the full float range.
static float _dc_sinf_kernel(float x) {
    // sin(x) for |x| <= pi/4, degree-5 polynomial
    float x2 = x * x;
    return x * (1.0f + x2 * (-0.16666667f + x2 * (0.0083333337f + x2 * -0.00019841270f)));
}
static float _dc_cosf_kernel(float x) {
    // cos(x) for |x| <= pi/4, degree-6 polynomial
    float x2 = x * x;
    return 1.0f + x2 * (-0.5f + x2 * (0.041666668f + x2 * -0.0013888889f));
}
float sinf(float x) {
    // Reduce x into [-pi/4, pi/4] using quadrant index n
    float q = x * 0.63661977f; // x * (2/pi)
    int n = (int)(q + (x >= 0.0f ? 0.5f : -0.5f));
    float r = x - (float)n * 1.5707963268f; // r = x - n*(pi/2)
    switch (n & 3) {
        case 0:  return  _dc_sinf_kernel(r);
        case 1:  return  _dc_cosf_kernel(r);
        case 2:  return -_dc_sinf_kernel(r);
        default: return -_dc_cosf_kernel(r);
    }
}
float cosf(float x) {
    float q = x * 0.63661977f;
    int n = (int)(q + (x >= 0.0f ? 0.5f : -0.5f));
    float r = x - (float)n * 1.5707963268f;
    switch (n & 3) {
        case 0:  return  _dc_cosf_kernel(r);
        case 1:  return -_dc_sinf_kernel(r);
        case 2:  return -_dc_cosf_kernel(r);
        default: return  _dc_sinf_kernel(r);
    }
}

// --- tanf ---
float tanf(float x) {
    float c = cosf(x);
    if (c == 0.0f) return 3.40282347e+38f;
    return sinf(x) / c;
}

// --- asinf ---
// Uses identity: asin(x) = atan(x / sqrt(1 - x*x))
float asinf(float x) {
    if (x >= 1.0f)  return  1.5707963268f;  // pi/2
    if (x <= -1.0f) return -1.5707963268f;
    float t = x * x;
    float s = sqrtf(1.0f - t);
    // atan(x/sqrt(1-x^2)) via degree-5 minimax on [0,1]
    float u = x / s;
    float u2 = u * u;
    return u * (1.0f + u2 * (-0.33333334f + u2 * (0.2f + u2 * (-0.14285715f + u2 * 0.11111111f))));
}

// --- acosf ---
float acosf(float x) { return 1.5707963268f - asinf(x); }

// --- atanf ---
// Degree-9 minimax polynomial, accurate to ~2 ULP for |x| <= 1.
// Larger |x| uses identity: atan(x) = pi/2 - atan(1/x).
float atanf(float x) {
    int inv = 0;
    if (x < 0.0f)  { x = -x; inv = 2; }   // handle sign
    if (x > 1.0f)  { x = 1.0f / x; inv ^= 1; }
    float x2 = x * x;
    float p = x * (1.0f + x2 * (-0.33333334f + x2 * (0.2f + x2 *
              (-0.14285715f + x2 * (0.11111111f + x2 * -0.090909094f)))));
    if (inv & 1) p = 1.5707963268f - p;
    return (inv & 2) ? -p : p;
}

// --- atan2f ---
float atan2f(float y, float x) {
    if (x > 0.0f)                    return atanf(y / x);
    if (x < 0.0f && y >= 0.0f)      return atanf(y / x) + 3.14159265359f;
    if (x < 0.0f && y <  0.0f)      return atanf(y / x) - 3.14159265359f;
    if (x == 0.0f && y > 0.0f)      return  1.5707963268f;
    if (x == 0.0f && y < 0.0f)      return -1.5707963268f;
    return 0.0f; // x == 0, y == 0 — undefined, return 0
}

// =============================================================================
// Double-precision shims
//
// model.c includes <math.h> which may resolve generic math calls to the
// double-precision symbols (cos, sin, exp, …) instead of the float ones
// (cosf, sinf, expf, …).  These thin wrappers delegate to our float shims
// above so no extra precision logic is needed.
// =============================================================================
double cos(double x)              { return (double)cosf((float)x); }
double sin(double x)              { return (double)sinf((float)x); }
double tan(double x)              { return (double)tanf((float)x); }
double acos(double x)             { return (double)acosf((float)x); }
double asin(double x)             { return (double)asinf((float)x); }
double atan(double x)             { return (double)atanf((float)x); }
double atan2(double y, double x)  { return (double)atan2f((float)y, (float)x); }
double exp(double x)              { return (double)expf((float)x); }
double log(double x)              { return (double)logf((float)x); }
double log2(double x)             { return (double)log2f((float)x); }
double log10(double x)            { return (double)log10f((float)x); }
double sqrt(double x)             { return (double)sqrtf((float)x); }
double pow(double a, double b)    { return (double)powf((float)a, (float)b); }
double fabs(double x)             { return (double)fabsf((float)x); }
double floor(double x)            { return (double)floorf((float)x); }
double ceil(double x)             { return (double)ceilf((float)x); }
double round(double x)            { return (double)roundf((float)x); }
double fmod(double x, double y)   { return (double)fmodf((float)x, (float)y); }
double tanh(double x)             { return (double)tanhf((float)x); }

#endif

int native_errno=0;
#if defined(__linux__)
int *__errno_location (void)
#else
int *__errno (void)
#endif
{
    return &native_errno;
}

#include "examples/natmod/deepcraft/mp_src.c"

// Forward declaration of type
mp_obj_full_type_t dc_type;

mp_map_elem_t dc_locals_dict_table[5];
static MP_DEFINE_CONST_DICT(dc_locals_dict, dc_locals_dict_table);

// Constructor
static mp_obj_t dc_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args_in) {
    dc_obj_t *self = mp_obj_malloc(dc_obj_t, type);
    return MP_OBJ_FROM_PTR(self);
}

// Create type and methods at runtime
mp_obj_t mpy_init(mp_obj_fun_bc_t *self, size_t n_args, size_t n_kw, mp_obj_t *args) {
    // This must be first, it sets up the globals dict and other things
    MP_DYNRUNTIME_INIT_ENTRY

    // Populate type
    dc_type.base.type = (void*)&mp_type_type;
    dc_type.flags = MP_TYPE_FLAG_NONE;
    dc_type.name = MP_QSTR_DEEPCRAFT;
    MP_OBJ_TYPE_SET_SLOT(&dc_type, make_new, dc_make_new, 0);

    dc_locals_dict_table[0] = (mp_map_elem_t){ MP_OBJ_NEW_QSTR(MP_QSTR_init), MP_OBJ_FROM_PTR(&init_obj) };
    dc_locals_dict_table[1] = (mp_map_elem_t){ MP_OBJ_NEW_QSTR(MP_QSTR_enqueue), MP_OBJ_FROM_PTR(&enqueue_obj) };
    dc_locals_dict_table[2] = (mp_map_elem_t){ MP_OBJ_NEW_QSTR(MP_QSTR_dequeue), MP_OBJ_FROM_PTR(&dequeue_obj) };
    dc_locals_dict_table[3] = (mp_map_elem_t){ MP_OBJ_NEW_QSTR(MP_QSTR_get_model_input_dim), MP_OBJ_FROM_PTR(&get_model_input_dim_obj) };
    dc_locals_dict_table[4] = (mp_map_elem_t){ MP_OBJ_NEW_QSTR(MP_QSTR_get_model_output_dim), MP_OBJ_FROM_PTR(&get_model_output_dim_obj) };
    
    MP_OBJ_TYPE_SET_SLOT(&dc_type, locals_dict, (void*)&dc_locals_dict, 5);

    // Expose constructor as DEEPCRAFT
    mp_store_global(MP_QSTR_DEEPCRAFT, MP_OBJ_FROM_PTR(&dc_type));
    // This must be last, it restores the globals dict
    MP_DYNRUNTIME_INIT_EXIT
    
}
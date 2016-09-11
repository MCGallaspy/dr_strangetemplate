

/* The hypothetical C API */

# define ERR_THEY_REALLY_DID_IT 6
# define ERR_UNKNOWN 13

struct MyCoolStruct {
    int a, b;
    double f;
};


int alpha(MyCoolStruct *input, int param1, int param2) {
    input->a = param1*param1;
    input->b = param1*param2;
    return 0;
}

int beta(MyCoolStruct *input, int param1, int param2) {
    return ERR_THEY_REALLY_DID_IT;
}

int gamma(MyCoolStruct *input) {
    input->f = 7.0*6.0;
    return ERR_UNKNOWN;
}

int delta(MyCoolStruct *input, int param1, int param2, int param3) {
    input->b = param1 + param2 + param3;
    return 0;
}

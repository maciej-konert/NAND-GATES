#include "nand.h"
#include <malloc.h>
#include <errno.h>
#include <assert.h>

#define NAND_REALLOC_MULTIPLY_VAL 2
#define NAND_OUTPUT_START_VAL 10

// STRUCTS.

typedef struct nand nand_t;
typedef struct nandInputStruct nandInputStruct_t;
typedef struct nandOutputStruct nandOutputStruct_t;

// wasVisited = -1 --> wasn't visited / wasVisited = 0 --> currently calculating node / wasVisited = 1 --> visited.
struct nand {
    unsigned nandInputsSize;
    unsigned long nandOutputSize;
    unsigned long nandOutputInsertIndex;
    nandInputStruct_t **nandInputsArr;
    nandOutputStruct_t **nandOutputsArr;
    bool nandBooleanOutputVal;
    bool wasEvaluated;
    int wasVisited;
    ssize_t criticalPath;
};

// If *nandConnectedToInput = NULL then we use the bool signal to receive input rather than
// value of nand gate.
struct nandInputStruct {
    nand_t *nandConnectedToInput;
    unsigned nandIndexOutputArr;
    bool const* pBooleanInput;
};

struct nandOutputStruct {
    nand_t *nandConnectedToOutput;
    unsigned nandIndexInputArr;
};

// HELPER FUNCTIONS

static ssize_t max(ssize_t a, ssize_t b) {
    if (a >= b)
        return a;
    else
        return b;
}

static void disconnectSignalFromInput(nand_t *g, unsigned n) {
    if (g->nandInputsArr[n] && g->nandInputsArr[n]->nandConnectedToInput) {
        free(g->nandInputsArr[n]->nandConnectedToInput->nandOutputsArr[g->nandInputsArr[n]->nandIndexOutputArr]);
        g->nandInputsArr[n]->nandConnectedToInput->nandOutputsArr[g->nandInputsArr[n]->nandIndexOutputArr] = NULL;
    }

    free(g->nandInputsArr[n]);
    g->nandInputsArr[n] = NULL;
}

static void disconnectSignalFromOutput(nand_t *g, unsigned n) {
    if(g->nandOutputsArr[n] && g->nandOutputsArr[n]->nandConnectedToOutput) {
        free(g->nandOutputsArr[n]->nandConnectedToOutput->nandInputsArr[g->nandOutputsArr[n]->nandIndexInputArr]);
        g->nandOutputsArr[n]->nandConnectedToOutput->nandInputsArr[g->nandOutputsArr[n]->nandIndexInputArr] = NULL;
    }

    free(g->nandOutputsArr[n]);
    g->nandOutputsArr[n] = NULL;
}

static int wrongEvaluateParameters(nand_t **g, bool *s, size_t m) {
    if (!g || !s || m == 0) {
        errno = EINVAL;
        return -1;
    }

    size_t nullCounter = 0;

    for (size_t i = 0; i < m; i++) {
        if(!g[i])
            nullCounter++;
    }

    // If g array is full of NULLS we return an error (can't calculate critical path length).
    if (nullCounter == m) {
        errno = EINVAL;
        return -1;
    }

    return 0;
}

static void modifyNewNandParameters (nand_t *g, unsigned n) {
    g->nandInputsSize = n;
    g->nandOutputInsertIndex = 0;
    g->nandOutputSize = NAND_OUTPUT_START_VAL;
    g->wasVisited = -1;
    g->wasEvaluated = false;
}

static int reallocOutputs(nand_t *g) {
    nandOutputStruct_t **newArr = realloc(g->nandOutputsArr,
                                          NAND_REALLOC_MULTIPLY_VAL * sizeof(nandOutputStruct_t*) * g->nandOutputSize);

    if (!newArr) {
        errno = ENOMEM;
        return -1;
    }

    g->nandOutputSize *= NAND_REALLOC_MULTIPLY_VAL;

    for (unsigned i = g->nandOutputInsertIndex; i < g->nandOutputSize; i++) {
        newArr[i] = NULL;
    }

    g->nandOutputsArr = newArr;

    return 0;
}

// If dfs returns 0 it means that there's no cycle, if it returns 1 it means that there is a cycle, if it returns -1,
// then there is a gate with no signal connected to one of its inputs.
static int dfs(nand_t *g) {
    int isThereACycle = 0;

    if (g->wasVisited == -1) {
        g->wasVisited = 0;

        for (unsigned long i = 0; i < g->nandInputsSize; i++) {
            if (!g->nandInputsArr[i]) {
                return -1;
            }

            else if (g->nandInputsArr[i]->nandConnectedToInput) {
                isThereACycle = dfs(g->nandInputsArr[i]->nandConnectedToInput);

                if (isThereACycle != 0)
                    return isThereACycle;
            }
        }
        g->wasVisited = 1;
    }

    else if (g->wasVisited == 0) {
        return 1;
    }

    return isThereACycle;
}

// findCycles returns -1 if there was a cycle or there was a gate with no signal connected.
// It returns 0 if evaluating is ready to begin.
static int findCycles(nand_t **g, size_t m) {
    for (size_t i = 0; i < m; i++) {

        if(g[i] && dfs(g[i]) != 0) {
            errno = ECANCELED;
            return -1;
        }
    }

    return 0;
}

static ssize_t evalHelperFoo(nand_t *g, bool *calculatedVal) {
    if (!g->wasEvaluated) {
        ssize_t maxCriticalPath = 0;
        bool currBoolVal, foundFalseSignal = false;
        *calculatedVal = false;
        g->nandBooleanOutputVal = false;

        if (g->nandInputsSize == 0) {
            g->criticalPath = 0;
        } else {
            for (unsigned long i = 0; i < g->nandInputsSize; i++) {
                if (g->nandInputsArr[i]->nandConnectedToInput) {
                    maxCriticalPath = max(maxCriticalPath, evalHelperFoo(
                            g->nandInputsArr[i]->nandConnectedToInput,&currBoolVal) + 1);

                    if (!currBoolVal)
                        foundFalseSignal = true;
                } else {
                    if (!*(g->nandInputsArr[i]->pBooleanInput))
                        foundFalseSignal = true;
                    maxCriticalPath = max(maxCriticalPath, 1);
                }
            }

            if (foundFalseSignal) {
                *calculatedVal = true, g->nandBooleanOutputVal = true;
            }

            g->criticalPath = maxCriticalPath;
        }

        g->wasEvaluated = true;
    } else {
        *calculatedVal = g->nandBooleanOutputVal;
    }

    return g->criticalPath;
}

static void resetNandsHelper(nand_t *g) {
    if (g->wasVisited != -1) {
        g->wasVisited = -1;
        g->criticalPath = 0;
        g->wasEvaluated = false;

        for (unsigned long i = 0; i < g->nandInputsSize; i++) {
            if (g->nandInputsArr[i] && g->nandInputsArr[i]->nandConnectedToInput)
                resetNandsHelper(g->nandInputsArr[i]->nandConnectedToInput);
        }
    }
}

static void resetNandsVisitedStatus(nand_t **g, size_t m) {
    for (size_t i = 0; i < m; i++) {
        if(g[i])
            resetNandsHelper(g[i]);
    }
}

// MAIN TASK FUNCTIONS

nand_t * nand_new(unsigned n) {
    nand_t *newNandGate = malloc(sizeof(nand_t));
    nandInputStruct_t **newInputsArr = calloc(n, sizeof(nandInputStruct_t*));
    nandOutputStruct_t **newOutputsArr = calloc(NAND_OUTPUT_START_VAL, sizeof(nandOutputStruct_t*));

    if(!newNandGate || (!newInputsArr && n != 0) || !newOutputsArr) {
        errno = ENOMEM;
        free(newNandGate), free(newInputsArr), free(newOutputsArr);
        newNandGate = NULL, newInputsArr = NULL, newOutputsArr = NULL;

        return NULL;
    }

    modifyNewNandParameters(newNandGate, n);
    newNandGate->nandInputsArr = newInputsArr;
    newNandGate->nandOutputsArr = newOutputsArr;

    return newNandGate;
}

void nand_delete(nand_t *g) {
    if (g) {

        // Deletion of Inputs.
        for (unsigned i = 0; i < g->nandInputsSize; i++) {
            disconnectSignalFromInput(g, i);
        }

        // Deletion of Outputs.
        for (unsigned j = 0; j < g->nandOutputInsertIndex; j++) {
            disconnectSignalFromOutput(g, j);
        }

        free(g->nandInputsArr), free(g->nandOutputsArr);
        g->nandOutputsArr = NULL, g->nandInputsArr = NULL;
        free(g);
        g = NULL;
    }
}

int nand_connect_nand(nand_t *g_out, nand_t *g_in, unsigned k) {
    if (!g_out || !g_in || k >= g_in->nandInputsSize) {
        errno = EINVAL;
        return -1;
    }

    nandOutputStruct_t *newOutputStruct = malloc(sizeof(nandOutputStruct_t));
    nandInputStruct_t *newInputStruct = malloc(sizeof(nandInputStruct_t));

    if (!newOutputStruct || !newInputStruct) {
        free(newOutputStruct), free(newInputStruct);
        newOutputStruct = NULL, newInputStruct = NULL;
        errno = ENOMEM;
        return -1;
    }

    // Connection from g_out to g_in.
    if (g_out->nandOutputInsertIndex == g_out->nandOutputSize) {
        if (reallocOutputs(g_out) == -1) {
            free(newOutputStruct), free(newInputStruct);
            newOutputStruct = NULL, newInputStruct = NULL;
            return -1;
        }
    }

    disconnectSignalFromInput(g_in, k);
    newOutputStruct->nandConnectedToOutput = g_in;
    newOutputStruct->nandIndexInputArr = k;
    g_out->nandOutputsArr[g_out->nandOutputInsertIndex] = newOutputStruct;

    // Connection from g_in to g_out.
    newInputStruct->nandConnectedToInput = g_out;
    newInputStruct->nandIndexOutputArr = g_out->nandOutputInsertIndex;
    g_in->nandInputsArr[k] = newInputStruct;
    g_out->nandOutputInsertIndex++;

    return 0;
}

int nand_connect_signal(bool const *s, nand_t *g, unsigned k) {
    if (!s || !g || k >= g->nandInputsSize) {
        errno = EINVAL;
        return -1;
    }

    nandInputStruct_t *newInputStruct = malloc(sizeof(nandInputStruct_t));

    if (!newInputStruct) {
        errno = ENOMEM;
        return -1;
    }

    disconnectSignalFromInput(g, k);

    newInputStruct->pBooleanInput = s;
    newInputStruct->nandConnectedToInput = NULL;
    g->nandInputsArr[k] = newInputStruct;

    return 0;
}

ssize_t nand_evaluate(nand_t **g, bool *s, size_t m) {
    if (wrongEvaluateParameters(g, s, m) == -1) {
        return -1;
    }

    else if (findCycles(g, m) == -1) {
        resetNandsVisitedStatus(g, m);
        return -1;
    }

    for (size_t i = 0; i < m; i++) {
        if (g[i])
            evalHelperFoo(g[i], &s[i]);
    }

    ssize_t maxPathLen = 0;

    for (size_t i = 0; i < m; i++) {
        if(g[i])
            maxPathLen = max(maxPathLen, g[i]->criticalPath);
    }

    resetNandsVisitedStatus(g, m);

    return maxPathLen;
}

ssize_t nand_fan_out(nand_t const *g) {
    if (!g) {
        errno = EINVAL;
        return -1;
    }

    int numberOfOutputs = 0;

    for (unsigned i = 0; i < g->nandOutputInsertIndex; i++) {
        if (g->nandOutputsArr[i]) {
            numberOfOutputs++;
        }
    }

    return numberOfOutputs;
}

void* nand_input(nand_t const *g, unsigned k) {
    if (!g || k >= g->nandInputsSize) {
        errno = EINVAL;
        return NULL;
    }

    if (!g->nandInputsArr[k]) {
        errno = 0;
        return NULL;
    }

    if (g->nandInputsArr[k]->nandConnectedToInput) {
        return g->nandInputsArr[k]->nandConnectedToInput;
    }

    return (void*) g->nandInputsArr[k]->pBooleanInput;
}

nand_t* nand_output(nand_t const *g, ssize_t k) {
    if (g) {
        int outputNumber = 0, i = 0;

        while (true) {
            if (g->nandOutputsArr[i]) {
                if (k == outputNumber) {
                    return g->nandOutputsArr[i]->nandConnectedToOutput;
                }

                outputNumber++;
            }

            i++;
        }
    }

    return NULL;
}
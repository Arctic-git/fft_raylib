
#ifdef __cplusplus
extern "C" {
#endif
    



float calcMax(float* arr, int len);
float calcMin(float* arr, int len);
int calcMaxPosition(float* arr, int len);
void arrAddScalar(float* arr, int len, float value);

float calcDcgain(float* arr, int len);

void arrScale(float* arr, int len, float factor);

float calcISE(float* arr, int len);
float calcITSE(float* arr, int len);
float calcIAE(float* arr, int len);
float calcITAE(float* arr, int len);

float calcOvershoot(float* arr, int len);
float calcUndershoot(float* arr, int len);
float calcSettlingTime(float* arr, int len);

float calcCost(float* response, int len, float targetValue, int costCrit);

void analyzeStepResponse(float* response, int len, float targetValue);

    
#ifdef __cplusplus
}
#endif

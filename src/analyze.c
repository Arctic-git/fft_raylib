#include <stdio.h>
#include "math.h"

#include "analyze.h"

#define TARGET 1.0f
#define SAMPLE_TIME 0.001



float calcMax(float* arr, int len){
    float maxval = arr[0];

    for(int i=0; i< len; i++){
        if(arr[i] > maxval) maxval = arr[i];
    }

    return maxval;
}
float calcMin(float* arr, int len){
    float minval = arr[0];
    
    for(int i=0; i< len; i++){
        if(arr[i] < minval) minval = arr[i];
    }
    
    return minval;
}

int calcMaxPosition(float* arr, int len){
    float maxval = arr[0];
    int maxPosition = -1;
    
    for(int i=0; i< len; i++){
        if(arr[i] > maxval){
            maxval = arr[i];
            maxPosition = i;
        }
    }
    
    return maxPosition;
}

float calcDcgain(float* arr, int len){
    return arr[len-1];
}


void arrScale(float* arr, int len, float factor){
    for(int i=0; i< len; i++){
        arr[i] = arr[i] * factor;
    }
}
void arrAddScalar(float* arr, int len, float value){
    for(int i=0; i< len; i++){
        arr[i] = arr[i] + value;
    }
}


float calcISE(float* arr, int len){
    float sum =0;
    
    for(int i=0; i< len; i++){
        sum += (TARGET - arr[i]) * (TARGET - arr[i]);
    }
    
    return sum;
}
float calcITSE(float* arr, int len){
    float sum =0;
    
    for(int i=0; i< len; i++){
        sum += (TARGET - arr[i]) * (TARGET - arr[i]) * ((float)(i+1))/((float)len);
    }
    
    return sum;
}
float calcIAE(float* arr, int len){
    float sum =0;
    
    for(int i=0; i< len; i++){
        sum += fabsf(TARGET - arr[i]);
    }
    
    return sum;
}
float calcITAE(float* arr, int len){
    float sum =0;
    
    for(int i=0; i< len; i++){
        sum += fabsf(TARGET - arr[i]) * ((float)(i+1))/((float)len);
    }
    
    return sum;
}



float calcOvershoot(float* arr, int len){
    float max =0;
    
    for(int i=0; i< len; i++){
        float y = arr[i];
        
        
        if(y > max) max = y;
    }
    
    float overshoot = max - TARGET;
    if(overshoot < 0) overshoot  =0;
    
    return overshoot;
}

float calcUndershoot(float* arr, int len){
    
    int firstPeakPos = -1; //if never falling, say peak is at end
    float min = 100;
    
    
    for(int i=0; i< len; i++){
        float y = arr[i];
        static float lastY = 0;
        float dy = y - lastY;
        lastY = y;
        
        static int dncnt =0;
        
        if(firstPeakPos == -1){
            if(dy < 0) {
                dncnt++;
                if(dncnt >= 5)
                    firstPeakPos = i;
            }else{
                dncnt=0;
            }
        }else{ //after first peak
            if(y < min) min = y;
        }
        
    }
    
    
    float undershoot = TARGET - min;
    if(undershoot < 0) undershoot  =0;
    
    return undershoot;
}



float calcSettlingTime(float* arr, int len){
    
    int lastOutsideOfSettlingBand = len-1;
    
    for(int i=0; i< len; i++){
        float y = arr[i];
        float error = TARGET - y;
        
        if(fabsf(error) > 0.02) lastOutsideOfSettlingBand = i;
    }
    
    
    return (float)lastOutsideOfSettlingBand * SAMPLE_TIME;
}

float calcRiseTime1090(float* arr, int len){
    //10 to 90
    int t10 =-1;
    int t90 =-1;
    
    for(int i=0; i< len; i++){
        float y = arr[i];
        float perc = y / TARGET;
        
        if(t10 == -1 && perc >= 0.1) t10 = i;
        if(t90 == -1 && perc >= 0.9){ t90 = i; break;}
    }
    
    
    return (float)(t90 - t10) * SAMPLE_TIME;
}

float calcRiseTime0100(float* arr, int len){
    //0 to 100
    
    
    int t100 =-1;
    
    for(int i=0; i< len; i++){
        float y = arr[i];
        float perc = y / TARGET;
        
        if(perc >= 1.0){ t100 = i; break;}
    }
    
    
    return (float)t100 * SAMPLE_TIME;
}


float calcCost(float* response, int len, float targetValue, int costCrit){
    
    if(targetValue != 1) arrScale(response, len, 1/targetValue);
    
    float cost = -1;
    if (costCrit >= 0){
        float itae = calcITAE(response, len);
        float over = calcOvershoot(response, len);
        float under = calcUndershoot(response, len);
        
        cost = itae + over*50*costCrit + under*50*costCrit;
    }else if(costCrit == -1){
        float iae = calcIAE(response, len);
        
        cost = iae;
    }else if(costCrit == -2){
        float ise = calcISE(response, len);
        
        cost = ise;
    }else if(costCrit == -3){
        float itse = calcITSE(response, len);
        
        cost = itse;
    }
    
    return cost;
}

void analyzeStepResponse(float* response, int len, float targetValue){

    arrScale(response, len, 1/targetValue);
    
    float ise = calcISE(response, len);
    float itse = calcITSE(response, len);
    float iae = calcIAE(response, len);
    float itae = calcITAE(response, len);
    
    float over = calcOvershoot(response, len);
    float under = calcUndershoot(response, len);
    float settle = calcSettlingTime(response, len);
    
    float rise1090 = calcRiseTime1090(response, len);
    float rise0100 = calcRiseTime0100(response, len);
    //printf("analyzeStepResponse  ise: %.2f, iae: %.2f, itae: %.2f, over: %.4f, under: %.4f, settle: %.4f \n", ise, iae, itae, over, under, settle);
    printf("ise \t\titse\t\tiae \t\titae\t\tover\t\tunder\t\tsettle\t\trise1090\trise0100\n");
    printf("%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n", ise, itse, iae, itae, over, under, settle, rise1090, rise0100);
    
}

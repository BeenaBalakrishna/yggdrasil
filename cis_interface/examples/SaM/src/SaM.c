#include <stdio.h>
#include <stdlib.h>
#include "PsiInterface.h"

#define BSIZE 100


int main() {
    int ret;
    char adata[BSIZE];
    char bdata[BSIZE];
    char outbuf[BSIZE];

    // Get input and output channels matching yaml
    psiInput_t in1 = psiInput("input1_c");
    psiInput_t in2 = psiInput("static_c");
    psiOutput_t out1 = psiOutput("output_c");
    printf("SaM(C): Set up I/O channels\n");

    // Get input from input1 channel
    ret = psi_recv(in1, adata, BSIZE);
    if (ret < 0) {
      printf("SaM(C): ERROR RECV from input1\n");
      return -1;
    }
    int a = atoi(adata);
    printf("SaM(C): Received %d from input1\n", a);

    // Get input from static channel
    ret = psi_recv(in2, bdata, BSIZE);
    if (ret < 0) {
      printf("SaM(C): ERROR RECV from static\n");
      return -1;
    }
    int b = atoi(bdata);
    printf("SaM(C): Received %d from static\n", b);

    // Compute sum and send message to output channel
    int sum = a + b;
    sprintf(outbuf, "%d", sum);
    ret = psi_send(out1, outbuf, strlen(outbuf));
    if (ret != 0) {
      printf("SaM(C): ERROR SEND to output\n");
      return -1;
    }
    printf("SaM(C): Sent to output\n");

    return 0;
}
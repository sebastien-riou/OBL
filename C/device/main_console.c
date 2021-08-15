#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

#include <stdint.h>

#include "obl_device.h"

int main(int argc, char *argv[]){
    printf("Entering OBL\n");
    obl_main();
    printf("OBL exited\n");

    return 0;
}

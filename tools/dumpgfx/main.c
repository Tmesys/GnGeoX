#include <stdio.h>
#include <stdlib.h>
#include "unzip.h"
#include "type.h"
#include "driver.h"

DRIVER *d;

int main(int argc, char *argv[])
{
    if(argc!=2) {
        printf("Usage: dumpgfx2 roms.zip\n");
        return 1;
    }    
    if (dr_load_driver("romrc")) {
        d=get_driver_for_zip(argv[1]);
        if (d) {
                dr_load_game(d,argv[1]);
        } else {
                printf("Couldn't load %s\n",argv[1]);
        }
    } else {
        printf("Couldn't load romrc\n");
    }    
    return 0;
}

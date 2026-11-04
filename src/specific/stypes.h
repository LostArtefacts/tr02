#ifndef STYPES_H
#define STYPES_H

typedef struct pcx_header {
    char zsoft_flag, version, encoding, bits;
    unsigned short xmin, ymin, xmax, ymax;
    unsigned short h_res, v_res;
    char header_pal[48], reserved, planes;
    unsigned short bytes_per_line, pal_interp, width, height;
    char blank[54];
} PCX_HEADER;

#endif

#include <stdio.h>
#include <string.h>
#include <stdint.h>

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} color_t;

const color_t colors[] = {

        { 0x00, 0x00, 0x00},   // 0 BLACK
        { 0x00, 0x00, 0xc0},   // 1 BLUE
        { 0x00, 0xa0, 0x00},   // 2 GREEN
        { 0x00, 0xc0, 0xc0},   // 3 CYAN
        { 0xa0, 0x00, 0x00},   // 4 RED
        { 0xc0, 0x00, 0xc0},   // 5 MAGENTA
        { 0xa0, 0x60, 0x20},   // 6 BROWN
        { 0xc0, 0xc0, 0xc0},   // 7 GRAY
        { 0x80, 0x80, 0x80},   // 8 DARK GRAY
        { 0x00, 0x00, 0xf0},   // 9 LIGHT BLUE
        { 0x00, 0xf0, 0x00},   // A LIGHT GREEN
        { 0x00, 0xf0, 0xf0},   // B LIGHT CYAN
        { 0xf0, 0x00, 0x00},   // C LIGHT RED
        { 0xf0, 0x00, 0xf0},   // D LIGHT MAGENTA
        { 0xf0, 0xf0, 0x00},   // E YELLOW
        { 0xf0, 0xf0, 0xf0}    // F WHITE
};

////////////////////////////////////////////////////////////////////////////////
int scan_index(int r, int g, int b){

	int eR, eG, eB;
	int error2;
	int error2Min = 200000;
	int index = 0;

	for (int i = 0; i < 16; i++){

		eR = r - colors[i].r;
		eG = g - colors[i].g;
		eB = b - colors[i].b;

		error2 = (eR*eR) + (eG*eG) + (eB*eB);

		if (error2 < error2Min){

			error2Min = error2;
			index = i;
		}
	}

	return index;
}

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char *argv[]){

	FILE *f;
	char filename[200];

	if (argc < 2) return 0;

	strncpy(filename,argv[1],sizeof(filename));

	f = fopen(filename,"rb");

	if (!f){
		printf("File not found\n");
		return -1;
	}


	char buf[4];

	uint8_t r,g,b;

	int res, colorleft, colorright;
	int col = 0;

	printf("//%s\nconst uint8_t %s[] = {\n",filename,strtok(filename,"."));

	for (int i = 0; i < 256; i+=2){

		res = fread(buf,sizeof(buf),1,f);

		if (res < 1){
			printf("Error reading file\n");
			return -1;
		}

		r = buf[0]; g = buf[1]; b = buf[2];

		colorleft = scan_index(r,g,b);

		//printf("%02x%02x%02x->%d  ",r,g,b,colorleft);

		//////////////////////////////////////////////

		res = fread(buf,sizeof(buf),1,f);

		if (res < 1){
			printf("Error reading file\n");
			return -1;
		}

		r = buf[0]; g = buf[1]; b = buf[2];

		colorright = scan_index(r,g,b);

		//printf("%02x%02x%02x->%d  ",r,g,b,colorright);

		printf("0x%1x%1x,",colorleft,colorright);
		col++;
		if (col == 8){
			printf("\n");
			col = 0;
		}

	}

	printf("};\n");

	fclose(f);

	return 0;
}


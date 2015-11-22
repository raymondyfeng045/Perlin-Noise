/* Texture functions for cs580 GzLib	*/
#include    "stdafx.h" 
#include	"stdio.h"
#include	"Gz.h"

GzColor	*image = NULL;
int xs, ys;
int reset = 1;
bool set = false;

const int GSIZE = 16;
float gradientMap[GSIZE*GSIZE][2];

/* Image texture function */
int tex_fun(float u, float v, GzColor color)
{
	unsigned char		pixel[3];
	unsigned char     dummy;
	char  		foo[8];
	int   		i, j;
	FILE			*fd;

	if (reset) {          /* open and load texture file */
		fd = fopen("texture", "rb");
		if (fd == NULL) {
			fprintf(stderr, "texture file not found\n");
			exit(-1);
		}
		fscanf(fd, "%s %d %d %c", foo, &xs, &ys, &dummy);
		image = (GzColor*)malloc(sizeof(GzColor)*(xs + 1)*(ys + 1));
		if (image == NULL) {
			fprintf(stderr, "malloc for texture image failed\n");
			exit(-1);
		}

		for (i = 0; i < xs*ys; i++) {	/* create array of GzColor values */
			fread(pixel, sizeof(pixel), 1, fd);
			image[i][RED] = (float)((int)pixel[RED]) * (1.0 / 255.0);
			image[i][GREEN] = (float)((int)pixel[GREEN]) * (1.0 / 255.0);
			image[i][BLUE] = (float)((int)pixel[BLUE]) * (1.0 / 255.0);
		}

		reset = 0;          /* init is done */
		fclose(fd);
	}

	// clamp u and v
	if (u < 0) u = 0;
	else if (u > 1) u = 1;
	if (v < 0) v = 0;
	else if (v > 1) v = 1;

	int x = xs;
	int y = ys;

	float xpoint = u * xs;
	float ypoint = v * ys;
	int xbegin = xpoint, ybegin = ypoint;
	int xend = ceilf(xpoint), yend = ceilf(ypoint);

	// Prevent skewing values for the image
	if (xbegin >= xs - 1) {
		xbegin = xs - 2;
		xend = xs - 1;
	}
	if (ybegin >= ys - 1) {
		ybegin = ys - 2;
		yend = ys - 1;
	}
	if (xend <= 0) {
		xbegin = 0;
		xend = 1;
	}
	if (yend <= 0) {
		ybegin = 0;
		yend = 1;
	}


	int indexUR = xbegin + xs * ybegin;
	int indexUL = xend + xs * ybegin;
	int indexDL = xend + xs * yend;
	int indexDR = xbegin + xs * yend;

	int max = xs*ys;

	if (indexDL >= max)
		indexDL = max - 1;

	if (xpoint == xbegin && ypoint == ybegin) { // if texture lies on a point
		color[RED] = image[indexUR][RED];
		color[GREEN] = image[indexUR][GREEN];
		color[BLUE] = image[indexUR][BLUE];
	}
	else { // bilinear interpolation
		float s = xpoint - xbegin;
		float t = ypoint - ybegin;
		for (int c = 0; c < 3; ++c) {
			color[c] = ((1 - s) * (1 - t) * image[indexUR][c]) +
				(s * (1 - t) * image[indexUL][c]) +
				(s * t) * image[indexDL][c] +
				((1 - s) * t) * image[indexDR][c];
		}
	}
	return GZ_SUCCESS;
	/* bounds-test u,v to make sure nothing will overflow image array bounds */
	/* determine texture cell corner values and perform bilinear interpolation */
	/* set color to interpolated GzColor value and return */
}

void subVector(float *v1, float *v2, float *sol) { // subtracts the vector
	for (int i = 0; i < 2; ++i)
		sol[i] = v1[i] - v2[i];
}

float texDotProduct(float *v1, float *v2) { // find the dot product
	float value = v1[0] * v2[0] + v1[1] * v2[1];
	return value;
}

void texNormalize(float *v) { // normalize the vector
	float total = (v[0] * v[0]) + (v[1] * v[1]);
	total = sqrtf(total);
	for (int i = 0; i < 2; ++i)
		v[i] /= total;
}

float lerp(float v1, float v2, float a) { // normalize the vector
	return (v2 * a) + (v1 * (1 - a));
}

float findTileCoord(float u, float scale) { // Returns the coordinates on a tile
	float tileSize = 1.0f / scale;
	float value = u;
	while (value > tileSize)
		value -= tileSize;
	return value / tileSize;
}

float fade_func(float t) { // function for blending the values
	return 6 * pow(t, 5) - 15 * pow(t, 4) + 10 * pow(t, 3);
}

float smooth_func(float t) { // smooths the noise
	return 6 * pow(t, 5) - 15 * pow(t, 4) + 10 * pow(t, 3);
}

float perlinNoise(float u, float v)
{
	// gradient map represents the tile N x N
	// sets up pseudorandom gradient map
	if (!set) {
		for (int i = 0; i < GSIZE * GSIZE; ++i) {
			for (int j = 0; j < 2; ++j) {
				gradientMap[i][j] = (float)((rand() % 10000) - 5000) / 5000.0f;
			}
		}
		set = true;
	}
	/*
	int size = 2;
	const int GSIZE = 4;
	float gradientMap[GSIZE][2] =
	{
	{ 1, 1 }, { -1, 1 },
	{ 1, -1 }, { -1, -1 }
	};
	*/
	// Scale must be in power of 2
	float scale = GSIZE - 1;

	//	gets index at lowerright corner
	int xint = ceilf(u * scale);
	int yint = ceilf(v * scale);

	int up = (((yint - 1) % GSIZE)* GSIZE);
	int down = ((yint % GSIZE)* GSIZE);
	int right = (xint) % GSIZE;
	int left = (xint - 1) % GSIZE;

	// finds the four surrounding corners for gradient maps by indices
	int indexUL = up + left;
	int indexUR = up + right;
	int indexDL = down + left;
	int indexDR = down + right;

	// find surround gradients as a tile containing a point
	float gradients[4][2];
	for (int j = 0; j < 2; ++j) {
		gradients[0][j] = gradientMap[indexUL][j];
		gradients[1][j] = gradientMap[indexUR][j];
		gradients[2][j] = gradientMap[indexDL][j];
		gradients[3][j] = gradientMap[indexDR][j];
	}

	// tranform u and v to match coordinates for one tile
	float xTile = findTileCoord(u, scale);
	float yTile = findTileCoord(v, scale);

	// fade function
	//xTile = fade_func(xTile);
	//yTile = fade_func(yTile);

	// (0,0) ________(1,0)
	//       |      |
	//       |      |
	//       |______|
	// (0,1)         (1,1)
	float UL[2] = { 0, 0 };
	float UR[2] = { 1, 0 };
	float DL[2] = { 0, 1 };
	float DR[2] = { 1, 1 };

	float uv[2] = { xTile, yTile };
	float vectUL[2];
	float vectUR[2];
	float vectDL[2];
	float vectDR[2];

	// vect1 - vect2 = newVect
	subVector(UL, uv, vectUL);
	subVector(UR, uv, vectUR);
	subVector(DL, uv, vectDL);
	subVector(DR, uv, vectDR);

	// tkae dot product between distance vector and gradient vector
	float ulDist = texDotProduct(vectUL, gradients[0]);
	float urDist = texDotProduct(vectUR, gradients[1]);
	float dlDist = texDotProduct(vectDL, gradients[2]);
	float drDist = texDotProduct(vectDR, gradients[3]);

	// from 0-1 for t between those value
	float topValue = lerp(ulDist, urDist, xTile);
	float bottomValue = lerp(dlDist, drDist, xTile);
	return (lerp(topValue, bottomValue, yTile) + 1.0) / 2.0; // bounds the value between 0 and 1
}

/* Procedural texture function */
int ptex_fun(float u, float v, GzColor color) // currently set to checkerboard
{
	//http://flafla2.github.io/2014/08/09/perlinnoise.html
	//http://www.angelcode.com/dev/perlin/perlin.html
	//http://lodev.org/cgtutor/randomnoise.html
	//http://code.google.com/p/fractalterraingeneration/wiki/Perlin_Noise

	float x = u - (int)u;
	float y = v - (int)v;

	// TILING //////////////////////////////////////////////////////
	// Scale must be in power of 2
	//float scale = 2.0f;

	// tranform u and v to match coordinates for one tile
	//float xTile = findTileCoord(u, scale);
	//float yTile = findTileCoord(v, scale);

	/////////////////////////////////////////////////////////////////

	float noise_value = 0;
	float total = 0;
	float persist = 0.5;
	int octaves = 4;
	float freq = 1, ampl = 1;

	// turbulence
	//////////////////////////////////////////////////////

	for (int i = 0; i < octaves; ++i) {
		noise_value += perlinNoise(x * freq, y * freq) * ampl;
		total += ampl;
		freq *= 2.0f;
		ampl *= persist;
	}
	noise_value /= total;
	//////////////////////////////////////////////////////
	if (noise_value < 0) {
		noise_value *= -1;
	}
	float OFFSETx = 0;
	float OFFSETy = -0.65;
	float stretchX = 5;
	float stretchY = 2;
	float numRings = 1;
	float turbFactor = 7;

	float xCoord = u * GSIZE+OFFSETx*GSIZE;
	float yCoord = v * GSIZE+OFFSETy*GSIZE;
	float xScaled = (xCoord - GSIZE / 2) / GSIZE*stretchX;
	float yScaled = (yCoord - GSIZE / 2) / GSIZE*stretchY;
	
	float dist = sqrt(xScaled*xScaled + yScaled*yScaled)+turbFactor*noise_value/GSIZE;
	float newU = (sin(dist * 3.14159265358979*numRings*2));
	
	if (newU < 0) {
		newU = newU * -1;
	}
	

	// Perlin noise must first take u, v and then find the four points (each with a gradient vector)
	// implement WANG Tiles: a type of perlin noise

	for (int i = 0; i < 3; ++i) {
		color[i] = newU;
	}
	

	return 1;
}

/* Free texture memory */
int GzFreeTexture()
{
	if (image != NULL)
		free(image);
	return GZ_SUCCESS;
}

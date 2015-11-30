/* CS580 Homework 5 */

#include	"stdafx.h"
#include	"stdio.h"
#include	"math.h"
#include	"Gz.h"
#include	"rend.h"

#define M_PI       3.14159265358979323846

int GzRotXMat(float degree, GzMatrix mat)
{
// Create rotate matrix : rotate along x axis
// Pass back the matrix using mat value
	if (mat == NULL)
		return GZ_FAILURE;
	float rad = degree * M_PI / 180.0; // convert degrees to radians
	GzMatrix xRot =
	{
		1.0, 0.0,       0.0, 0.0,
		0.0, cos(rad),  -sin(rad), 0.0,
		0.0, sin(rad), cos(rad), 0.0,
		0.0, 0.0,       0.0, 1.0
	};
	for (int i = 0; i < 4; ++i) // copies rotate matrix into mat
		for (int j = 0; j < 4; ++j)
			mat[i][j] = xRot[i][j];
	return GZ_SUCCESS;
}


int GzRotYMat(float degree, GzMatrix mat)
{
// Create rotate matrix : rotate along y axis
// Pass back the matrix using mat value
	if (mat == NULL)
		return GZ_FAILURE;
	float rad = degree * M_PI / 180.0;
	GzMatrix yRot =
	{
		cos(rad), 0.0, sin(rad), 0.0,
		0.0,      1.0, 0.0,       0.0,
		-sin(rad), 0.0, cos(rad),  0.0,
		0.0,      0.0, 0.0,       1.0
	};
	for (int i = 0; i < 4; ++i) 
		for (int j = 0; j < 4; ++j)
			mat[i][j] = yRot[i][j];
	return GZ_SUCCESS;
}


int GzRotZMat(float degree, GzMatrix mat)
{
// Create rotate matrix : rotate along z axis
// Pass back the matrix using mat value
	if (mat == NULL)
		return GZ_FAILURE;
	float rad = degree * M_PI / 180.0;
	GzMatrix zRot =
	{
		cos(rad),  -sin(rad), 0.0, 0.0,
		sin(rad), cos(rad), 0.0, 0.0,
		0.0,       0.0,      1.0, 0.0,
		0.0,       0.0,      0.0, 1.0
	};
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			mat[i][j] = zRot[i][j];
	return GZ_SUCCESS;
}


int GzTrxMat(GzCoord translate, GzMatrix mat)
{
// Create translation matrix
// Pass back the matrix using mat value
	if (mat == NULL || translate == NULL)
		return GZ_FAILURE;

	GzMatrix tMat =
	{
		1.0, 0.0, 0.0, translate[X],
		0.0, 1.0, 0.0, translate[Y],
		0.0, 0.0, 1.0, translate[Z],
		0.0, 0.0, 0.0, 1.0
	};
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			mat[i][j] = tMat[i][j];
	return GZ_SUCCESS;
}


int GzScaleMat(GzCoord scale, GzMatrix mat)
{
// Create scaling matrix
// Pass back the matrix using mat value
	if (mat == NULL || scale == NULL)
		return GZ_FAILURE;

	GzMatrix sMat =
	{
		scale[X], 0.0,      0.0,      0.0,
		0.0,      scale[Y], 0.0,      0.0,
		0.0,      0.0,      scale[Z], 0.0,
		0.0,      0.0,      0.0,      1.0
	};
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			mat[i][j] = sMat[i][j];

	return GZ_SUCCESS;
}


//----------------------------------------------------------
// Begin main functions

int GzNewRender(GzRender **render, GzDisplay	*display)
{
/*  
- malloc a renderer struct 
- setup Xsp and anything only done once 
- save the pointer to display 
- init default camera 
*/ 
	*render = new GzRender;
	(*render)->display = display;
	(*render)->matlevel = 0;

	// Sets up camera based on default
	GzCamera defCamera;
	defCamera.FOV = DEFAULT_FOV;
	
	defCamera.lookat[X] = 0.0;
	defCamera.lookat[Y] = 0.0;
	defCamera.lookat[Z] = 0.0;

	defCamera.position[X] = DEFAULT_IM_X;
	defCamera.position[Y] = DEFAULT_IM_Y;
	defCamera.position[Z] = DEFAULT_IM_Z;

	defCamera.worldup[X] = 0.0;
	defCamera.worldup[Y] = 1.0;
	defCamera.worldup[Z] = 0.0;

	(*render)->camera = defCamera;

	float xs = (*render)->display->xres;
	float ys = (*render)->display->yres;

	GzMatrix Xsp = // finds the matrix based on xres and yres
	{
		xs/2, 0.0, 0.0,     xs/2,
		0.0, -ys/2, 0.0,    ys/2,
		0.0,  0.0,  MAXINT, 0.0,
		0.0,  0.0,  0.0,    1.0
	};

	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			(*render)->Xsp[j][i] = Xsp[j][i];

	return GZ_SUCCESS;

}


int GzFreeRender(GzRender *render)
{
/* 
-free all renderer resources
*/
	delete render;
	return GZ_SUCCESS;
}


int GzBeginRender(GzRender *render)
{
	/*
	- setup for start of each frame - init frame buffer color,alpha,z
	- compute Xiw and projection xform Xpi from camera definition
	- init Ximage - put Xsp at base of stack, push on Xpi and Xiw
	- now stack contains Xsw and app can push model Xforms when needed
	*/
	if (render == NULL)
		return GZ_FAILURE;

	int index; // used to cycle through the framebuffer

	// Initializes values for background color
	for (int i = 0; i < render->display->xres; ++i) {
		for (int j = 0; j < render->display->yres; ++j) {
			index = (j*render->display->xres) + i;
			render->display->fbuf[index].blue = 4095 / 2;
			render->display->fbuf[index].green = 4095 / 2;
			render->display->fbuf[index].red = 4095 / 2;
			render->display->fbuf[index].alpha = 4095;
			render->display->fbuf[index].z = MAXINT;
		}
	}

	float d_inv = tan((render->camera.FOV / 2) * M_PI / 180.0);

	GzMatrix Xpi = // finds matrix based on fov
	{
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, d_inv, 0.0,
		0.0, 0.0, d_inv, 1.0
	};

	float xaxis[3];
	float yaxis[3];
	float zaxis[3];
	float up[3];

	// find z-axis of camera
	for (int i = 0; i < 3; ++i)
		zaxis[i] = render->camera.lookat[i] - render->camera.position[i];

	normalize(zaxis);

	// find y-axis of camera
	for (int i = 0; i < 3; ++i)
		up[i] = dotProduct(render->camera.worldup, zaxis) * zaxis[i];
	for (int i = 0; i < 3; ++i)
		yaxis[i] = render->camera.worldup[i] - up[i];

	normalize(yaxis);

	// find x-axis of camera
	crossProduct(yaxis, zaxis, xaxis); // gets cross product for xaxis
	normalize(xaxis);


	GzMatrix Xiw = // finds matrix based on camera's coordinates
	{
		xaxis[X], xaxis[Y], xaxis[Z], -dotProduct(xaxis, render->camera.position),
		yaxis[X], yaxis[Y], yaxis[Z], -dotProduct(yaxis, render->camera.position),
		zaxis[X], zaxis[Y], zaxis[Z], -dotProduct(zaxis, render->camera.position),
		0.0, 0.0, 0.0, 1.0
	};

	for (int i = 0; i < 4; ++i) { // copy the value into the camera
		for (int j = 0; j < 4; ++j) {
			render->camera.Xiw[j][i] = Xiw[j][i];
			render->camera.Xpi[j][i] = Xpi[j][i];
		}
	}

	GzPushMatrix(render, render->Xsp);
	GzPushMatrix(render, render->camera.Xpi);
	GzPushMatrix(render, render->camera.Xiw);

	return GZ_SUCCESS;
}

int GzPutCamera(GzRender *render, GzCamera *camera)
{
/*
- overwrite renderer camera structure with new camera definition
*/
	if (render == NULL || camera == NULL)
		return GZ_FAILURE;
	
	render->camera = *camera;
	return GZ_SUCCESS;
}

int GzPushMatrix(GzRender *render, GzMatrix	matrix)
{
/*
- push a matrix onto the Ximage stack
- check for stack overflow
*/
	if (render->matlevel >= 100)
		return GZ_FAILURE;

	// pushes the matrix
	if (render->matlevel > 0) {
		for (int i = 0; i < 4; ++i) { // cycles through cols of current matrix
			for (int j = 0; j < 4; ++j) { // cycles through rows of current matrix
				render->Ximage[render->matlevel][j][i] = 0;
				for (int n = 0; n < 4; ++n) { // cycles through multiplication for current matrix
					render->Ximage[render->matlevel][j][i] += (render->Ximage[render->matlevel - 1][j][n] * matrix[n][i]);
				}
			}
		}
	}
	else { // set the initial matrix
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				render->Ximage[render->matlevel][i][j] = matrix[i][j];
				render->Xnorm[render->matlevel][i][j] = 0;
			}
		}
	}

	// for transformation of normal
	for (int i = 0; i < 4; ++i) // cycles through cols of current matrix
		for (int j = 0; j < 4; ++j) // cycles through rows of current matrix
			render->Xnorm[render->matlevel][j][i] = 0;

	// checks for unitary rotation
	bool unit = true;
	float total = 0;
	for (int i = 0; i < 3; ++i) {
		total = 0;
		for (int j = 0; j < 3; ++j) {
			total += (matrix[j][i] * matrix[j][i]);
		}
		total = sqrtf(total);
		if (total < 0.98 || total > 1.02) {
			unit = false;
			break;
		}
	}

	if (unit) { // prevents translation and scaling
		GzMatrix sub_m = { 0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 1 };

		for (int i = 0; i < 3; ++i)  // cycles through cols of current matrix
			for (int j = 0; j < 3; ++j)  // cycles through rows of current matrix
				sub_m[j][i] = matrix[j][i];

		if (render->matlevel > 0) {
			for (int i = 0; i < 4; ++i)  // cycles through cols of current matrix
				for (int j = 0; j < 4; ++j)  // cycles through rows of current matrix
					for (int n = 0; n < 4; ++n)  // cycles through multiplication for current matrix
						render->Xnorm[render->matlevel][j][i] += (render->Xnorm[render->matlevel - 1][j][n] * sub_m[n][i]) / total;
		}
		else {
			for (int i = 0; i < 3; ++i) // cycles through cols of current matrix
				for (int j = 0; j < 3; ++j) // cycles through rows of current matrix
					render->Xnorm[render->matlevel][j][i] = sub_m[j][i];
			render->Xnorm[render->matlevel][3][3] = 1;
		}
	}
	else {
		if (render->matlevel > 0) {
			for (int i = 0; i < 4; ++i) // cycles through cols of current matrix
				for (int j = 0; j < 4; ++j) // cycles through rows of current matrix
					render->Xnorm[render->matlevel][j][i] = render->Xnorm[render->matlevel - 1][j][i];
		}
		else { // prevents inserting of Xsp and Xpi
			for (int i = 0; i < 4; ++i) // set as identity matrix
				render->Xnorm[render->matlevel][i][i] = 1;
		}
	}

	++(render->matlevel);

	return GZ_SUCCESS;
}

int GzPopMatrix(GzRender *render)
{
/*
- pop a matrix off the Ximage stack
- check for stack underflow
*/
	if (render->matlevel <= 0)
		return GZ_FAILURE;

	--(render->matlevel);

	return GZ_SUCCESS;
}

/* NOT part of API - just for general assistance */

short	ctoi(float color)		/* convert float color to GzIntensity short */
{
	return(short)((int)(color * ((1 << 12) - 1)));
}

int GzPutAttribute(GzRender	*render, int numAttributes, GzToken	*nameList, 
	GzPointer	*valueList) /* void** valuelist */
{
/*
- set renderer attribute states (e.g.: GZ_RGB_COLOR default color)
- later set shaders, interpolaters, texture maps, and lights
*/
	for (int i = 0; i < numAttributes; ++i) {
		if (nameList[i] == GZ_RGB_COLOR) { // command for coloring in RGB
			float *color = (float*)valueList[i];

			// Assigns the color from float to GzIntensity
			for (int j = 0; j < 3; ++j) {
				if (color[j] > 1.0) color[j] = 1.0; // Clamps to 1.0
				else if (color[j] < 0.0) color[j] = 0.0; // Clamps to 0.0

				render->flatcolor[j] = ctoi(color[j]); // Put color for render*/
			}
		}
		else if (nameList[i] == GZ_DIRECTIONAL_LIGHT) {
			GzLight *dLight = (GzLight*)valueList[i];
			render->lights[i] = *dLight;
			render->numlights = numAttributes;
		}
		else if (nameList[i] == GZ_AMBIENT_LIGHT) { // gets ambient light
			GzLight *aLight = (GzLight*)valueList[i];
			render->ambientlight = *aLight;
		}

		else if (nameList[i] == GZ_INTERPOLATE) { // gets interpolation mode
			int *interp = (int*)valueList[i];
			render->interp_mode = *interp;
		}

		else if (nameList[i] == GZ_DIFFUSE_COEFFICIENT) { // gets diffuse coefficient
			float *color = (float*)valueList[i];
			for (int i = 0; i < 3; ++i)
				render->Kd[i] = color[i];

		}		
		else if (nameList[i] == GZ_AMBIENT_COEFFICIENT) { // gets ambient coefficient
			float *color = (float*)valueList[i];
			for (int i = 0; i < 3; ++i)
				render->Ka[i] = color[i];
		}
		else if (nameList[i] == GZ_SPECULAR_COEFFICIENT) { // gets specular coefficient
			float *color = (float*)valueList[i];
			for (int i = 0; i < 3; ++i)
				render->Ks[i] = color[i];
		}
		else if (nameList[i] == GZ_DISTRIBUTION_COEFFICIENT) { // gets specular color
			float *specolor = (float*)valueList[i];
			render->spec = *specolor;
		}

		else if (nameList[i] == GZ_TEXTURE_MAP) { // gets texture pixel
			GzTexture tex = (GzTexture)valueList[i];
			if (tex == NULL)
				render->tex_fun = NULL;
			else render->tex_fun = tex;
		}

	}
	return GZ_SUCCESS;
}

void interpolate(float y, float y1, float y2, float *data1, float *data2, float *result) {
	float t = ((float)y - y2) / (y1 - y2);
	for (int col = 0; col < 3; ++col) {
		result[col] = (t) * data1[col] + (1 - t)* data2[col];
	}
}

void interpolateTex(float y, float y1, float y2, float *data1, float *data2, float *result) {
	float t = ((float)y - y2) / (y1 - y2);
	for (int col = 0; col < 2; ++col) {
		result[col] = (t)* data1[col] + (1 - t)* data2[col];
	}
}

void scanLine(GzRender *render, float x_begin, float x_end, int y, float* norm, float d) { // Span line method

	int index; // used to find index within triangles
	int z;

	if (!(x_begin > render->display->xres - 1 || x_end < 0)) { // checks and see if line is within the screen

		// prevents same line from being drawn twice for later triangles
		int x = ceilf(x_begin);
		if (x == x_begin)
			++x;

		// Clamps the value between 0 and xres
		if (x_begin < 0)
			x = 0.0;
		if (x_end > render->display->xres - 1)
			x_end = render->display->xres - 1;

		for (x; x <= x_end; ++x) { // begins spanning line
			index = (render->display->xres * y) + x;

			// interpolates z at that point
			z = -((norm[0] * (float)x) + (norm[1] * (float)y) + d) / norm[2];

			if (z < render->display->fbuf[index].z && z >= 0) { // if the object is in front of what's on fbuf
				render->display->fbuf[index].red = render->flatcolor[0];
				render->display->fbuf[index].green = render->flatcolor[1];
				render->display->fbuf[index].blue = render->flatcolor[2];
				render->display->fbuf[index].z = z;
			}
		}
	}
}

void scanLineGouraud(GzRender *render, float x_begin, float x_end, int y, float* norm, float d, float *color1, float *color2, float* TexPoint1, float* TexPoint2) { // Span line method

	int index; // used to find index within triangles
	float z;

	if (!(x_begin > render->display->xres - 1 || x_end < 0)) { // checks and see if line is within the screen

		// prevents same line from being drawn twice for later triangles
		int x = ceilf(x_begin);
		if (x == x_begin)
			++x;

		float x_s = x_begin;
		float total = x_end - x_begin;

		// Clamps the value between 0 and xres
		if (x_begin < 0)
			x = 0.0;
		if (x_end > render->display->xres - 1)
			x_end = render->display->xres - 1;

		float t;

		for (x; x <= x_end; ++x) { // begins spanning line
			index = (render->display->xres * y) + x;

			t = (x - x_s) / total;

			// interpolates z at that point
			z = -((norm[0] * (float)x) + (norm[1] * (float)y) + d) / norm[2];

			if (z < render->display->fbuf[index].z && z >= 0) { // if the object is in front of what's on fbuf
				if (render->tex_fun == NULL) {
					render->display->fbuf[index].red = ctoi((color1[0] * (1 - t) + color2[0] * t));
					render->display->fbuf[index].green = ctoi((color1[1] * (1 - t) + color2[1] * t));
					render->display->fbuf[index].blue = ctoi((color1[2] * (1 - t) + color2[2] * t));
				}
				else {
					// look up kd and ka
					float newTex[2];
					for (int n = 0; n < 2; ++n) // interpolate u and v
						newTex[n] = (TexPoint1[n] * (1 - t) + TexPoint2[n] * t);

					for (int n = 0; n < 2; ++n) // revert the interpolated value back to screen space
						newTex[n] *= ((z / ((float)MAXINT - z)) + 1);

					float texColor[3];
					render->tex_fun(newTex[0], newTex[1], texColor); // grab color from texture

					float rgb[3];
					for (int c = 0; c < 3; ++c) { // multiply by Ktexture and clamp
						rgb[c] = ((color1[c] * (1 - t) + color2[c] * t) * texColor[c]);
						if (rgb[c] > 1)
							rgb[c] = 1;
						if (rgb[c] < 0)
							rgb[c] = 0;
					}
					render->display->fbuf[index].red = ctoi(rgb[0]);
					render->display->fbuf[index].green = ctoi(rgb[1]);
					render->display->fbuf[index].blue = ctoi(rgb[2]);
				}
				render->display->fbuf[index].z = z;
			}
		}
	}
}

void scanLinePhong(GzRender *render, float x_begin, float x_end, int y, float* norm, float d, float* norm1, float* norm2, float* TexPoint1, float* TexPoint2) { // Span line method

	int index; // used to find index within triangles
	float z;

	if (!(x_begin > render->display->xres - 1 || x_end < 0)) { // checks and see if line is within the screen

		// prevents same line from being drawn twice for later triangles
		int x = ceilf(x_begin);
		if (x == x_begin)
			++x;

		float x_s = x_begin;
		float total = x_end - x_begin;

		// Clamps the value between 0 and xres
		if (x_begin < 0)
			x = 0.0;
		if (x_end > render->display->xres - 1)
			x_end = render->display->xres - 1;

		float t;
		float newNormal[3];
		float newColor[3];

		for (x; x <= x_end; ++x) { // begins spanning line
			index = (render->display->xres * y) + x;

			// interpolates z at that point
			z = -((norm[0] * (float)x) + (norm[1] * (float)y) + d) / norm[2];

			if (z < render->display->fbuf[index].z && z >= 0) { // if the object is in front of what's on fbuf

				t = (x - x_s) / total;
				for (int n = 0; n < 3; ++n)
					newNormal[n] = (norm1[n] * (1 - t) + norm2[n] * t);
				normalize(newNormal);
				colorPixel(render, newNormal, newColor); // gets color based on normal

				if (render->tex_fun == NULL) {
					render->display->fbuf[index].red = ctoi(newColor[0]);
					render->display->fbuf[index].green = ctoi(newColor[1]);
					render->display->fbuf[index].blue = ctoi(newColor[2]);
				}
				else {
					// look up kd and ka
					float newTex[2];
					for (int n = 0; n < 2; ++n) // interpolate u and v
						newTex[n] = (TexPoint1[n] * (1 - t) + TexPoint2[n] * t);

					for (int n = 0; n < 2; ++n) // revert the interpolated value back to screen space
						newTex[n] *= ((z / ((float)MAXINT - z)) + 1);

					float texColor[3];
					float newTexColor[3];
					render->tex_fun(newTex[0], newTex[1], texColor); // grab color from texture
					colorPixelTex(render, newNormal, newTexColor, render->Ks, texColor, texColor); // get color for Phong
					
					render->display->fbuf[index].red = ctoi(newTexColor[0]);
					render->display->fbuf[index].green = ctoi(newTexColor[1]);
					render->display->fbuf[index].blue = ctoi(newTexColor[2]);
				}
				render->display->fbuf[index].z = z;
			}
		}
	}
}

int GzPutTriangle(GzRender	*render, int numParts, GzToken *nameList, GzPointer	*valueList)
/* numParts : how many names and values */
{
/*  
- pass in a triangle description with tokens and values corresponding to 
      GZ_POSITION:3 vert positions in model space 
- Xform positions of verts using matrix on top of stack 
- Clip - just discard any triangle with any vert(s) behind view plane 
       - optional: test for triangles with all three verts off-screen (trivial frustum cull)
- invoke triangle rasterizer  
*/ 
	if (render == NULL || nameList == NULL)
		return GZ_FAILURE;

	float *n1 = new float[3], *v1 = new float[3], *t1 = new float[2];
	float *n2 = new float[3], *v2 = new float[3], *t2 = new float[2];
	float *n3 = new float[3], *v3 = new float[3], *t3 = new float[2];

	float color1[3];
	float color2[3];
	float color3[3];

	for (int i = 0; i < numParts; ++i) { // iterates through number of parts
		if (nameList[i] == GZ_NULL_TOKEN) // do nothing
			;
		if (nameList[i] == GZ_TEXTURE_INDEX) {
			GzTextureIndex *tex = (GzTextureIndex*)valueList[i];

			for (int num = 0; num < 2; ++num) {
				t1[num] = tex[0][num];
				t2[num] = tex[1][num];
				t3[num] = tex[2][num];
			}
			for (int num = 0; num < 2; ++num) {
				t1[num] /= ((v1[2] / ((float)MAXINT - v1[2])) + 1.0);
				t2[num] /= ((v2[2] / ((float)MAXINT - v2[2])) + 1.0);
				t3[num] /= ((v3[2] / ((float)MAXINT - v3[2])) + 1.0);
			}

			if (render->tex_fun != NULL) { // for Gourad Shading
				float dummy[3] = { 1, 1, 1 };
				colorPixelTex(render, n1, color1, dummy, dummy, dummy);
				colorPixelTex(render, n2, color2, dummy, dummy, dummy);
				colorPixelTex(render, n3, color3, dummy, dummy, dummy);
			}
		}
		if (nameList[i] == GZ_NORMAL)
		{
			GzCoord *normal = (GzCoord*)valueList[i];
			for (int num = 0; num < 3; ++num) {
				n1[num] = normal[0][num];
				n2[num] = normal[1][num];
				n3[num] = normal[2][num];
			}

			float matrix[4][4];
			for (int i = 0; i < 4; ++i) // get matrix based on normals
				for (int j = 0; j < 4; ++j)
					matrix[i][j] = render->Xnorm[render->matlevel - 1][i][j];

			float n_v1[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			float n_v2[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			float n_v3[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

			for (int y = 0; y < 4; ++y) { // does matrix multiplication
				for (int x = 0; x < 4; ++x) {
					if (x != 3) {
						n_v1[y] += matrix[y][x] * n1[x];
						n_v2[y] += matrix[y][x] * n2[x];
						n_v3[y] += matrix[y][x] * n3[x];
					}
					else {
						n_v1[y] += matrix[y][x] * 1;
						n_v2[y] += matrix[y][x] * 1;
						n_v3[y] += matrix[y][x] * 1;
					}
				}
			}

			for (int x = 0; x < 3; ++x) { // converts the vertices from 4D to 3D 
				n_v1[x] /= n_v1[3];
				n_v2[x] /= n_v2[3];
				n_v3[x] /= n_v3[3];
			}

			// Reassign n1, n2, n3
			for (int num = 0; num < 3; ++num) {
				n1[num] = n_v1[num];
				n2[num] = n_v2[num];
				n3[num] = n_v3[num];
			}

			// gets the color based on vertices
			colorPixel(render, n1, color1);
			colorPixel(render, n2, color2);
			colorPixel(render, n3, color3);
		}
		if (nameList[i] == GZ_POSITION) { // Executes 

			GzCoord *coord = (GzCoord*)valueList[i];
			for (int num = 0; num < 3; ++num) {
				v1[num] = coord[0][num];
				v2[num] = coord[1][num];
				v3[num] = coord[2][num];
			}
			// Transform the coords based on the current matrix
			float matrix[4][4];
			for (int i = 0; i < 4; ++i)
				for (int j = 0; j < 4; ++j)
					matrix[i][j] = render->Ximage[render->matlevel - 1][i][j];

			float n_v1[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			float n_v2[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			float n_v3[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

			for (int y = 0; y < 4; ++y) { // does matrix multiplication
				for (int x = 0; x < 4; ++x) {
					if (x != 3) {
						n_v1[y] += matrix[y][x] * v1[x];
						n_v2[y] += matrix[y][x] * v2[x];
						n_v3[y] += matrix[y][x] * v3[x];
					}
					else {
						n_v1[y] += matrix[y][x] * 1;
						n_v2[y] += matrix[y][x] * 1;
						n_v3[y] += matrix[y][x] * 1;
					}
				}
			}

			for (int x = 0; x < 3; ++x) { // converts the vertices from 4D to 3D 
				n_v1[x] /= n_v1[3];
				n_v2[x] /= n_v2[3];
				n_v3[x] /= n_v3[3];
			}
			// Reassign v1, v2, v3
			for (int num = 0; num < 3; ++num) {
				v1[num] = n_v1[num];
				v2[num] = n_v2[num];
				v3[num] = n_v3[num];
			}

			// Offsets for Anti-aliasing
			{
				v1[0] -= render->xOff; 			v1[1] -= render->yOff;
				v2[0] -= render->xOff; 			v2[1] -= render->yOff;
				v3[0] -= render->xOff; 			v3[1] -= render->yOff;
			}
		}
	}

	// GZ_FLAT -> same
	// GZ_COLOR -> find color first then interpolate
	// GZ_NORMAL -> interpolate then find normals

	float *v_sub;
	float *n_sub;
	float c_sub;

	float *t_sub;

	for (int v = 0; v < 2; ++v) { // selection sort vertices by y from lowest to highest
		if (v1[1] > v2[1]) {
			v_sub = v2;
			v2 = v1;
			v1 = v_sub;

			if (render->interp_mode == GZ_NORMALS){
				n_sub = n2;
				n2 = n1;
				n1 = n_sub;
			}
			else if (render->interp_mode == GZ_COLOR) {
				for (int c = 0; c < 3; ++c) {
					c_sub = color2[c];
					color2[c] = color1[c];
					color1[c] = c_sub;
				}
			}

			if (render->tex_fun != NULL) {
				t_sub = t1;
				t1 = t2;
				t2 = t_sub;
			}
		}
		if (v2[1] > v3[1]) {
			v_sub = v3;
			v3 = v2;
			v2 = v_sub;

			if (render->interp_mode == GZ_NORMALS){
				n_sub = n3;
				n3 = n2;
				n2 = n_sub;
			}
			else if (render->interp_mode == GZ_COLOR) {
				for (int c = 0; c < 3; ++c) {
					c_sub = color3[c];
					color3[c] = color2[c];
					color2[c] = c_sub;
				}
			}

			if (render->tex_fun != NULL) {
				t_sub = t2;
				t2 = t3;
				t3 = t_sub;
			}
		}
	}

	// vectors that are based off the y-axis from low to high
	float vect1[3];
	float vect2[3];
	float vect3[3];
	int lr; // 0 = left, 1 = right, 2 = special traingle case

	for (int num = 0; num < 3; ++num) { // DEFAULT is set to L
		vect1[num] = v2[num] - v1[num]; // To v2 from v1
		vect2[num] = v3[num] - v2[num]; // To v3 from v2
		vect3[num] = v3[num] - v1[num]; // To v3 from v1
	}

	// Normalize the vectors
	normalize(vect1);
	normalize(vect2);
	normalize(vect3);

	// determines if the line is left or right

	if (v2[1] == v3[1] || v2[1] == v1[1]) // Checks if mid-vertex has same y-coord
		lr = 2;
	else
	{
		// looks at the edge vector opposite the vertex: v1 -> v3
		// based on y-intercept for v2
		float time, x;
		time = (v2[1] - v1[1]) / vect3[1];
		x = v1[0] + time * vect3[0];

		// find x and compare with v2 x-coordinate
		if (x < v2[0]) { // if the intersection occurs on the right
			lr = 1; // midpoint is on R
		}
		else lr = 0; // midpoint is on L
	}

	float norm[3], d;
	crossProduct(vect1, vect2, norm); // Calculates the cross product for the plane

	// Calculates d using vertex 1
	d = -((norm[0] * v1[0]) + (norm[1] * v1[1]) + (norm[2] * v1[2]));

	// variables for finding the bounds for coloring in triangle
	float x_sol;
	float x_begin;
	int y_begin;
	float x_end, y_end;

	// clamps the boundaries for y
	if (ceilf(v1[1]) < 0)
		y_begin = 0;
	else y_begin = ceilf(v1[1]);

	if (v3[1] > render->display->yres - 1)
		y_end = render->display->yres - 1;
	else y_end = v3[1];

	float t;
	float GouraudColor1[3];
	float GouraudColor2[3];
	float PhongNormal1[3];
	float PhongNormal2[3];
	float TexPoint1[3];
	float TexPoint2[3];

	// Computes rasterization using scan line
	if (lr == 0) { // vect 1 & vect 2 are on the left
		for (int y = y_begin; y <= y_end; ++y) { // cycles from top vertex to bottom vertex

			// finds the start point and the endpoint from v1 to v3 in y-coord
			// computes x_begin for v1 -> v2
			if (y < v2[1]) {
				t = ((float)y - v1[1]) / vect1[1];
				x_sol = (vect1[0] * t + v1[0]);
				x_begin = x_sol;

				if (render->interp_mode == GZ_COLOR)
					interpolate(y, v1[1], v2[1], color1, color2, GouraudColor1);
				else if (render->interp_mode == GZ_NORMALS)
					interpolate(y, v1[1], v2[1], n1, n2, PhongNormal1);

				if (render->tex_fun != NULL)
					interpolateTex(y, v1[1], v2[1], t1, t2, TexPoint1);
			}
			else { // compute for v2 -> v3
				t = ((float)y - v2[1]) / vect2[1];
				x_sol = (vect2[0] * t + v2[0]);
				x_begin = x_sol;

				if (render->interp_mode == GZ_COLOR)
					interpolate(y, v2[1], v3[1], color2, color3, GouraudColor1);
				else if (render->interp_mode == GZ_NORMALS)
					interpolate(y, v2[1], v3[1], n2, n3, PhongNormal1);

				if (render->tex_fun != NULL)
					interpolateTex(y, v2[1], v3[1], t2, t3, TexPoint1);
			}

			// computes x_end: v1 -> v3
			t = ((float)y - v1[1]) / vect3[1];
			x_sol = (vect3[0] * t + v1[0]);
			x_end = x_sol;

			if (render->interp_mode == GZ_COLOR)
				interpolate(y, v1[1], v3[1], color1, color3, GouraudColor2);
			else if (render->interp_mode == GZ_NORMALS)
				interpolate(y, v1[1], v3[1], n1, n3, PhongNormal2);

			if (render->tex_fun != NULL)
				interpolateTex(y, v1[1], v3[1], t1, t3, TexPoint2);

			if (render->interp_mode == GZ_FLAT)
				scanLine(render, x_begin, x_end, y, norm, d); // Spans line for y_value
			else if (render->interp_mode == GZ_COLOR) {
				scanLineGouraud(render, x_begin, x_end, y, norm, d, GouraudColor1, GouraudColor2, TexPoint1, TexPoint2); // Gouraud Shading
			}
			else if (render->interp_mode == GZ_NORMALS) {
				scanLinePhong(render, x_begin, x_end, y, norm, d, PhongNormal1, PhongNormal2, TexPoint1, TexPoint2); // Phong Shading
			}
			else scanLine(render, x_begin, x_end, y, norm, d); // Spans line for y_value
		}
	}
	else if (lr == 1) { // RIGHT
		for (int y = y_begin; y <= y_end; ++y) { // cycles from top vertex to bottom vertex

			// finds the start point and the endpoint from v1 to v3 in y-coord
			// computes x_begin for v1 -> v3
			t = ((float)y - v1[1]) / vect3[1];
			x_sol = (vect3[0] * t + v1[0]);
			x_begin = x_sol;

			if (render->interp_mode == GZ_COLOR)
				interpolate(y, v1[1], v3[1], color1, color3, GouraudColor1);
			else if (render->interp_mode == GZ_NORMALS)
				interpolate(y, v1[1], v3[1], n1, n3, PhongNormal1);

			if (render->tex_fun != NULL)
				interpolateTex(y, v1[1], v3[1], t1, t3, TexPoint1);

			// computes x_end v1 -> v2
			if (y < v2[1]) {
				t = ((float)y - v1[1]) / vect1[1];
				x_sol = (vect1[0] * t + v1[0]);
				x_end = x_sol;

				if (render->interp_mode == GZ_COLOR)
					interpolate(y, v1[1], v2[1], color1, color2, GouraudColor2);
				else if (render->interp_mode == GZ_NORMALS)
					interpolate(y, v1[1], v2[1], n1, n2, PhongNormal2);

				if (render->tex_fun != NULL)
					interpolateTex(y, v1[1], v2[1], t1, t2, TexPoint2);
			}
			else { // compute for v2 -> v3
				t = ((float)y - v2[1]) / vect2[1];
				x_sol = (vect2[0] * t + v2[0]);
				x_end = x_sol;

				if (render->interp_mode == GZ_COLOR)
					interpolate(y, v2[1], v3[1], color2, color3, GouraudColor2);
				else if (render->interp_mode == GZ_NORMALS)
					interpolate(y, v2[1], v3[1], n2, n3, PhongNormal2);

				if (render->tex_fun != NULL)
					interpolateTex(y, v2[1], v3[1], t2, t3, TexPoint2);
			}

			if (render->interp_mode == GZ_FLAT)
				scanLine(render, x_begin, x_end, y, norm, d); // Spans line for y_value
			else if (render->interp_mode == GZ_COLOR) {
				scanLineGouraud(render, x_begin, x_end, y, norm, d, GouraudColor1, GouraudColor2, TexPoint1, TexPoint2); // Gouraud Shading
			}
			else if (render->interp_mode == GZ_NORMALS) {
				scanLinePhong(render, x_begin, x_end, y, norm, d, PhongNormal1, PhongNormal2, TexPoint1, TexPoint2); // Phong Shading
			}
			else scanLine(render, x_begin, x_end, y, norm, d); // Spans line for y_value

		}
	}
	// special case of horizontal triangles
	else if (lr == 2) {
		// compare x
		float *vleft, *vright;
		float *vleftend, *vrightend;

		float *cleft, *cright;
		float *cleftend, *crightend;

		float *nleft, *nright;
		float *nleftend, *nrightend;

		float *tleft, *tright;
		float *tleftend, *trightend;

		float* vect_l;
		float* vect_r;

		if (v2[1] == v3[1]) { // bottom line triangle
			vleft = v1; vright = v1;
			if (render->interp_mode == GZ_COLOR)
			{
				cleft = color1; cright = color1;
			}
			else if (render->interp_mode == GZ_NORMALS)
			{
				nleft = n1; nright = n1;
			}
			if (render->tex_fun != NULL) {
				tleft = t1; tright = t1;
			}
			if (v2[0] > v3[0]) { // v2 is on the right; v1->v2 is on left, v1->v3 is on right
				vect_l = vect3;
				vect_r = vect1;
				vleftend = v2;
				vrightend = v3;

				if (render->interp_mode == GZ_COLOR)
				{
					cleftend = color2; crightend = color3;
				}
				else if (render->interp_mode == GZ_NORMALS)
				{
					nleftend = n2; nrightend = n3;
				}
				if (render->tex_fun != NULL) {
					tleftend = t2; trightend = t3;
				}
			}
			else { // v2 is on the left; v1->v2 is on right, v1->v3 is on left
				vect_l = vect1;
				vect_r = vect3;
				vleftend = v3;
				vrightend = v2;
				if (render->interp_mode == GZ_COLOR)
				{
					cleftend = color3;
					crightend = color2;
				}
				else if (render->interp_mode == GZ_NORMALS)
				{
					nleftend = n3; nrightend = n2;
				}
				if (render->tex_fun != NULL) {
					tleftend = t3; trightend = t2;
				}
			}
		}
		else if (v1[1] == v2[1]) { // top line triangle
			if (y_begin == v1[1]) // if top horizontal line lies on y_coord for row of pixels
				++y_begin; // prevents top and bottom triangle from overwriting same line for integer y
			if (v1[0] > v2[0]) { // v2 is on the left; v1 is on right
				vleft = v2;
				vright = v1;
				vect_l = vect2;
				vect_r = vect3;
				if (render->interp_mode == GZ_COLOR)
				{
					cleft = color2;
					cright = color1;
				}
				else if (render->interp_mode == GZ_NORMALS)
				{
					nleft = n2; nright = n3;
				}
				if (render->tex_fun != NULL) {
					tleft = t2; tright = t3;
				}
			}
			else { // v2 is on the right; v1 is on left
				vleft = v1;
				vright = v2;
				vect_l = vect3;
				vect_r = vect2;
				if (render->interp_mode == GZ_COLOR)
				{
					cleft = color1;
					cright = color2;
				}
				else if (render->interp_mode == GZ_NORMALS)
				{
					nleft = n1; nright = n2;
				}
				if (render->tex_fun != NULL) {
					tleft = t1; tright = t2;
				}
			}
			vleftend = v3;
			vrightend = v3;
			if (render->interp_mode == GZ_COLOR)
			{
				cleftend = color3;
				crightend = color3;
			}
			else if (render->interp_mode == GZ_NORMALS)
			{
				nleftend = n3; nrightend = n3;
			}
			if (render->tex_fun != NULL) {
				tleftend = t3; trightend = t3;
			}
		}
		for (int y = y_begin; y <= y_end; ++y) { // cycles from top vertex to bottom vertex

			// computes x_begin for left
			t = ((float)y - vleft[1]) / vect_l[1];
			x_sol = (vect_l[0] * t + vleft[0]);
			x_begin = x_sol;

			if (render->interp_mode == GZ_COLOR)
				interpolate(y, vleft[1], vleftend[1], cleft, cleftend, GouraudColor1);
			else if (render->interp_mode == GZ_NORMALS)
				interpolate(y, vleft[1], vleftend[1], nleft, nleftend, PhongNormal1);

			if (render->tex_fun != NULL)
				interpolateTex(y, vleft[1], vleftend[1], tleft, tleftend, TexPoint1);

			// computes x_end for right
			t = ((float)y - vright[1]) / vect_r[1];
			x_sol = (vect_r[0] * t + vright[0]);
			x_end = x_sol;

			if (render->interp_mode == GZ_COLOR)
				interpolate(y, vright[1], vrightend[1], cright, crightend, GouraudColor2);
			else if (render->interp_mode == GZ_NORMALS)
				interpolate(y, vright[1], vrightend[1], nright, nrightend, PhongNormal2);

			if (render->tex_fun != NULL)
				interpolateTex(y, vright[1], vrightend[1], tright, trightend, TexPoint2);

			if (render->interp_mode == GZ_FLAT)
				scanLine(render, x_begin, x_end, y, norm, d); // Spans line for y_value
			else if (render->interp_mode == GZ_COLOR) {
				scanLineGouraud(render, x_begin, x_end, y, norm, d, GouraudColor1, GouraudColor2, TexPoint1, TexPoint2); // Spans line for y_value
			}
			else if (render->interp_mode == GZ_NORMALS) {
				scanLinePhong(render, x_begin, x_end, y, norm, d, PhongNormal1, PhongNormal2, TexPoint1, TexPoint2); // Spans line for y_value
			}
			else scanLine(render, x_begin, x_end, y, norm, d); // Spans line for y_value

		}
	}
	delete t1;	delete t2;	delete t3;
	delete n1;	delete n2;	delete n3;
	delete v1;	delete v2;	delete v3;

	return GZ_SUCCESS;
}

void colorPixel(GzRender *render, float *norm, float *color) {
	float fbuf[3];
	for (int v = 0; v < 3; ++v) {
		fbuf[v] = 0;
	}
	float reflect[3];
	float eye[3] = {0, 0, -1};
	float normal[3];
	float view;
	float REdot, NLdot, NEdot;
	float lightdir[3];

	for (int l = 0; l < render->numlights; ++l) {
		// NOTE: All vectors must be NORMALIZED

		for (int v = 0; v < 3; ++v) {
			normal[v] = norm[v];
			lightdir[v] = render->lights[l].direction[v];
		}

		normalize(lightdir);

		// dot product of norm and light direction
		NLdot = dotProduct(normal, lightdir);

		// dot product of norm and camera eye direction
		NEdot = dotProduct(normal, eye);

		if ((NLdot > 0 && NEdot > 0) || (NLdot < 0 && NEdot < 0)) { // checks if light and eye are not opposite direction

			if (NLdot < 0 && NEdot < 0) {
				for (int v = 0; v < 3; ++v)
					normal[v] = -normal[v];
				NLdot = dotProduct(normal, lightdir);
				NEdot = dotProduct(normal, eye);
			}

			// Calculates the reflected ray
			for (int v = 0; v < 3; ++v) {
				reflect[v] = (2 * NLdot * normal[v]) - lightdir[v];
			}

			normalize(reflect);

			// specular lighting
			// reflect and eyedir
			REdot = dotProduct(reflect, eye);

			if (REdot > 1) // clamp the dot product
				REdot = 1;
			else if (REdot < 0)
				REdot = 0;

			// diffuse lighting

			for (int c = 0; c < 3; ++c) { // adding specular and diffuse lighting
				
				fbuf[c] += render->lights[l].color[c] * render->Ks[c] * pow(REdot, render->spec);
				fbuf[c] += render->lights[l].color[c] * render->Kd[c] * NLdot;
			}
		}
	}

	// Ambient lighting and clamping values
	for (int c = 0; c < 3; ++c) {
		fbuf[c] += render->Ka[c] * render->ambientlight.color[c];
		if (fbuf[c] > 1.0) fbuf[c] = 1.0; // Clamps to 1.0
		else if (fbuf[c] < 0.0) fbuf[c] = 0.0; // Clamps to 0.0

	}

	color[0] = fbuf[0];
	color[1] = fbuf[1];
	color[2] = fbuf[2];
}

void colorPixelTex(GzRender *render, float *norm, float *color, float *Ks, float *Kd, float *Ka) {
	float fbuf[3];
	for (int v = 0; v < 3; ++v) {
		fbuf[v] = 0;
	}
	float reflect[3];
	float eye[3] = { 0, 0, -1 };
	float normal[3];
	float view;
	float REdot, NLdot, NEdot;
	float lightdir[3];

	for (int l = 0; l < render->numlights; ++l) {
		// NOTE: All vectors must be NORMALIZED

		for (int v = 0; v < 3; ++v) {
			normal[v] = norm[v];
			lightdir[v] = render->lights[l].direction[v];
		}

		normalize(lightdir);

		// dot product of norm and light direction
		NLdot = dotProduct(normal, lightdir);

		// dot product of norm and camera eye direction
		NEdot = dotProduct(normal, eye);

		if ((NLdot > 0 && NEdot > 0) || (NLdot < 0 && NEdot < 0)) { // checks if light and eye are not opposite direction

			if (NLdot < 0 && NEdot < 0) {
				for (int v = 0; v < 3; ++v)
					normal[v] = -normal[v];
				NLdot = dotProduct(normal, lightdir);
				NEdot = dotProduct(normal, eye);
			}

			// Calculates the reflected ray
			for (int v = 0; v < 3; ++v) {
				reflect[v] = (2 * NLdot * normal[v]) - lightdir[v];
			}

			normalize(reflect);

			// specular lighting
			// reflect and eyedir
			REdot = dotProduct(reflect, eye);

			if (REdot > 1) // clamp the dot product
				REdot = 1;
			else if (REdot < 0)
				REdot = 0;

			// diffuse lighting

			for (int c = 0; c < 3; ++c) { // adding specular and diffuse lighting
				render->lights[l].color[c] = 0.5;
				fbuf[c] += render->lights[l].color[c] * Ks[c] * pow(REdot, render->spec);
				fbuf[c] += render->lights[l].color[c] * Kd[c] * NLdot;
			}
		}
	}

	// Ambient lighting and clamping values
	for (int c = 0; c < 3; ++c) {
		fbuf[c] += Ka[c] * render->ambientlight.color[c];
		if (fbuf[c] > 1.0) fbuf[c] = 1.0; // Clamps to 1.0
		else if (fbuf[c] < 0.0) fbuf[c] = 0.0; // Clamps to 0.0

	}

	color[0] = fbuf[0];
	color[1] = fbuf[1];
	color[2] = fbuf[2];
}

int displayTexture(GzDisplay	*display, GzRender *render) { // NEW: change the display to show the texture
	if (display == NULL)
		return GZ_FAILURE;

	int index = 0; // used to cycle through the framebuffer
	float u, v;
	GzColor color;

	// Initializes values for background color
	for (int j = 0; j < display->yres; ++j) {
		for (int i = 0; i < display->xres; ++i) {
			u = ((float)(i) + 0.5) / (float)display->xres;
			v = ((float)(j) + 0.5) / (float)display->yres;

			render->tex_fun(u, v, color);
			display->fbuf[index].red = color[0] * 4095;
			display->fbuf[index].green = color[1] * 4095;
			display->fbuf[index].blue = color[2] * 4095;
			display->fbuf[index].alpha = 4095;
			display->fbuf[index].z = MAXINT;

			++index;
		}
	}
	return GZ_SUCCESS;
}

float dotProduct(float *u, float *v) { // calculates dot product
	return u[X] * v[X] + u[Y] * v[Y] + u[Z] * v[Z];
}

void normalize(float *s) { // normalize the vector
	float length = sqrtf((s[0] * s[0]) + (s[1] * s[1]) + (s[2] * s[2]));
	s[0] = s[0] / length;
	s[1] = s[1] / length;
	s[2] = s[2] / length;
}

void crossProduct(float *u, float *v, float *s) { // calculates cross product
	s[0] = u[1] * v[2] - u[2] * v[1]; // sx = uy*vz - uz*vy
	s[1] = u[2] * v[0] - u[0] * v[2]; // sy = uz*vx - ux*vz
	s[2] = u[0] * v[1] - u[1] * v[0]; // sz = ux*vy - uy*vx

	normalize(s);
}

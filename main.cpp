#include<fstream>
#include<cmath>

class Vertice
{
public:
	float x, y, z;
	float u, v;
	Vertice(float x, float y, float z, float u, float v)
	{
		this->x = x;
		this->y = y;
		this->z = z;

		this->u = u;
		this->v = v;
	}
	void project()
	{
		x /= z;
		y /= z;
		u /= z;
		v /= z;
		z = 1.0f / z;
	}
	void fitToImage(int imageWidth, int imageHeight)
	{
		x = (x * (imageWidth / 2)) + (imageWidth / 2);
		y = (-y * (imageHeight / 2)) + (imageHeight / 2);
	}
};

class Image
{
public:
	int imageWidth, imageHeight;
	unsigned char* pixels;
	float* zbuffer;
	Image(int imageWidth, int imageHeight)
	{
		this->imageWidth = imageWidth;
		this->imageHeight = imageHeight;
		pixels = new unsigned char[imageWidth * imageHeight * 3];
		zbuffer = new float[imageWidth * imageHeight];
		for (int i = 0; i < imageWidth * imageHeight; i++)
			zbuffer[i] = 1000.0f;
	}
	~Image()
	{
		delete[] pixels;
		delete[] zbuffer;
	}
	void setPixel(int x, int y, int red, int green, int blue)
	{
		int help_var = ((y * imageWidth) + x) * 3;
		pixels[help_var + 0] = (unsigned char)red;
		pixels[help_var + 1] = (unsigned char)green;
		pixels[help_var + 2] = (unsigned char)blue;
	}
	void fillPixels(int red, int green, int blue)
	{
		int help_var = imageWidth * imageHeight * 3;
		for (int i = 0; i < help_var; i += 3) {
			pixels[i + 0] = (unsigned char)red;
			pixels[i + 1] = (unsigned char)green;
			pixels[i + 2] = (unsigned char)blue;
		}
	}
	void setZBufferAt(int x, int y, float value)
	{
		int hlp_var = ((y * imageWidth) + x);
		zbuffer[hlp_var] = value;
	}
	float getZBufferAt(int x, int y)
	{
		int hlp_var = ((y * imageWidth) + x);
		return zbuffer[hlp_var];
	}

	float edgeFunction(const Vertice& A, const Vertice& B, const Vertice& P)
	{
		return ((P.x - A.x)*(B.y - A.y) - (P.y - A.y)*(B.x - A.x));
	}	

	void fillTriangleBarycentric(const Vertice& v0, const Vertice& v1, const Vertice& v2)
	{
		Vertice p(0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		float area = edgeFunction(v0, v1, v2);
		float edge0[2] = {v2.x - v1.x, v2.y - v1.y};
		float edge1[2] = {v0.x - v2.x, v0.y - v2.y};
		float edge2[2] = {v1.x - v0.x, v1.y - v0.y};
		for (int x = 0; x < imageWidth; x++) {
			for (int y = 0; y < imageHeight; y++) {
				p.x = x + 0.5f;	p.y = y + 0.5f;
				float w0 = edgeFunction(v1, v2, p);
				float w1 = edgeFunction(v2, v0, p);
				float w2 = edgeFunction(v0, v1, p);
				bool inside = true;
				inside &= ((w0 == 0) ? ((edge0[1] == 0 && edge0[0] > 0) || edge0[1] > 0) : (w0 > 0));
				inside &= ((w1 == 0) ? ((edge1[1] == 0 && edge1[0] > 0) || edge1[1] > 0) : (w1 > 0));
				inside &= ((w2 == 0) ? ((edge2[1] == 0 && edge2[0] > 0) || edge2[1] > 0) : (w2 > 0));
				if (inside) {
					w0 /= area;
					w1 /= area;
					w2 /= area;
					float z = 1.0f / (w0 * v0.z + w1 * v1.z + w2 * v2.z);	
					
					if (z < getZBufferAt(x, y)) {
						setZBufferAt(x, y, z);

						float u = (w0 * v0.u + w1 * v1.u + w2 * v2.u) * z;
						float v = (w0 * v0.v + w1 * v1.v + w2 * v2.v) * z;
					
						const float M = 8;
						float p = (fmod(u * M, 1.0f) > 0.5f) ^ (fmod(v * M, 1.0f) < 0.5f);

						int red =   (int)(255.0f * p);
						int green = (int)(255.0f * p);
						int blue =  (int)(255.0f * p);
						setPixel(x, y, red, green, blue);
					}
				}
			}
		}
	}
};

int main()
{
	Image image(800, 600);
	//image.fillPixels(255, 255, 255);
	float sr = 90.0f, sg = 90.0f, sb = 130.0f;
	float er = 0.0f, eg = 0.0f, eb = 80.0f;
	float dr = ((er - sr) / 600.0f); 
	float dg = ((eg - sg) / 600.0f); 
	float db = ((eb - sb) / 600.0f);
	for (int y = 0; y < 600; y++) {
		sr += dr; sg += dg; sb += db;
		for (int x = 0; x < 800; x++)
			image.setPixel(x, y, (int)sr, (int)sg, (int)sb);
	}


	Vertice a(0.2f, 3.5f, 4.0f, 0.0f, 1.0f);
	Vertice b(-0.8f, 0.0f, 1.0f, 0.0f, 0.0f);
	Vertice c(1.6f, -1.6f, 2.0f, 1.0f, 0.0f);
	
	Vertice d(2.0f, 2.0f, 3.0f, 0.0f, 1.0f);
	Vertice e(-2.0f, -2.0f, 3.0f, 1.0f, 0.0f);
	Vertice f(2.0f, -2.0f, 3.0f, 0.0f, 0.0f);

	a.project();
	b.project();
	c.project();
	
	d.project();
	e.project();
	f.project();

	a.fitToImage(image.imageWidth, image.imageHeight);
	b.fitToImage(image.imageWidth, image.imageHeight);
	c.fitToImage(image.imageWidth, image.imageHeight);
	
	d.fitToImage(image.imageWidth, image.imageHeight);
	e.fitToImage(image.imageWidth, image.imageHeight);
	f.fitToImage(image.imageWidth, image.imageHeight);

	image.fillTriangleBarycentric(a, b, c);
	image.fillTriangleBarycentric(d, e, f);

	std::ofstream imageFile;
	imageFile.open("./image.ppm");
	imageFile << "P6\n" << image.imageWidth << " " << image.imageHeight << "\n255\n";
	imageFile.write((char*)image.pixels, image.imageWidth * image.imageHeight * 3);
	imageFile.close();
	
	return 0;
}

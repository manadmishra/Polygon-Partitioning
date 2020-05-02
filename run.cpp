#include <stdio.h>
#include <list>
#include <limits>
#include <bits/stdc++.h>

using namespace std;

#include "polypartition.h"

#include "image.h"
#include "imageio.h"

void DrawPoly(Image *img, CGPPPoly *poly, tppl_float xmin, tppl_float xmax, tppl_float ymin, tppl_float ymax) {
	CGPPPoint p1,p2,p1img,p2img,polymin,imgmin;
	long i;
	Image::Pixel color={0,0,0};

	polymin.x = xmin;
	polymin.y = ymin;
	imgmin.x = 5;
	imgmin.y = 5;

	tppl_float polySizeX = xmax - xmin;
	tppl_float polySizeY = ymax - ymin;
	tppl_float imgSizeX = (tppl_float)img->GetWidth()-10;
	tppl_float imgSizeY = (tppl_float)img->GetHeight()-10;
	
	tppl_float scalex = 0;	
	tppl_float scaley = 0;
	tppl_float scale;
	if(polySizeX>0) scalex = imgSizeX/polySizeX;
	if(polySizeY>0) scaley = imgSizeY/polySizeY;

	if(scalex>0 && scalex<scaley) scale = scalex;
	else if(scaley>0) scale = scaley;
	else scale = 1;

	for(i=0;i<poly->GetNumPoints();i++) {
		p1 = poly->GetPoint(i);
		p2 = poly->GetPoint((i+1)%poly->GetNumPoints());
		p1img = (p1 - polymin)*scale + imgmin;
		p2img = (p2 - polymin)*scale + imgmin;
		img->DrawLine((int)p1img.x,(int)p1img.y,(int)p2img.x,(int)p2img.y,color);
	}
}

void DrawPoly(const char *filename, CGPPPoly *poly) {
	Image img(500,500);
	Image::Pixel white={255,255,255};
	img.Clear(white);
	ImageIO io;

	tppl_float xmin = std::numeric_limits<tppl_float>::max();
	tppl_float xmax = std::numeric_limits<tppl_float>::min();
	tppl_float ymin = std::numeric_limits<tppl_float>::max();
	tppl_float ymax = std::numeric_limits<tppl_float>::min();
	for(int i=0;i<poly->GetNumPoints();i++) {
		if(poly->GetPoint(i).x < xmin) xmin = poly->GetPoint(i).x;
		if(poly->GetPoint(i).x > xmax) xmax = poly->GetPoint(i).x;
		if(poly->GetPoint(i).y < ymin) ymin = poly->GetPoint(i).y;
		if(poly->GetPoint(i).y > ymax) ymax = poly->GetPoint(i).y;
	}

	DrawPoly(&img, poly, xmin, xmax, ymin, ymax);

	io.SaveImage(filename,&img);
}

void ReadPoly(FILE *fp, CGPPPoly *poly) {
	int i,numpoints,hole;
	float x,y;

	fscanf(fp,"%d\n",&numpoints);
	poly->Init(numpoints);

	fscanf(fp,"%d\n",&hole);
	if(hole) poly->SetHole(true);

	for(i=0;i<numpoints;i++) {
		fscanf(fp,"%g %g\n",&x, &y);
		(*poly)[i].x = x;
		(*poly)[i].y = y;
	}
}

void ReadPoly(const char *filename, CGPPPoly *poly) {
	FILE *fp = fopen(filename,"r");
	if(!fp) {
		printf("Error reading file %s\n", filename);
		return;
	}
	ReadPoly(fp,poly);
	fclose(fp);	
}

void ReadPolyList(FILE *fp, list<CGPPPoly> *polys) {
	int i,numpolys;
	CGPPPoly poly;

	polys->clear();
	fscanf(fp,"%d\n",&numpolys);
	for(i=0;i<numpolys;i++) {
		ReadPoly(fp,&poly);
		polys->push_back(poly);
	}
}

void ReadPolyList(const char *filename, list<CGPPPoly> *polys) {
	FILE *fp = fopen(filename,"r");
	if(!fp) {
		printf("Error reading file %s\n", filename);
		return;
	}
	ReadPolyList(fp,polys);
	fclose(fp);
}

void DrawPolyList(const char *filename, list<CGPPPoly> *polys) {
	Image img(500,500);
	Image::Pixel white={255,255,255};
	img.Clear(white);

	ImageIO io;
	list<CGPPPoly>::iterator iter;

	tppl_float xmin = std::numeric_limits<tppl_float>::max();
	tppl_float xmax = std::numeric_limits<tppl_float>::min();
	tppl_float ymin = std::numeric_limits<tppl_float>::max();
	tppl_float ymax = std::numeric_limits<tppl_float>::min();
	for(iter=polys->begin(); iter!=polys->end(); iter++) {
		for(int i=0;i<iter->GetNumPoints();i++) {
			if(iter->GetPoint(i).x < xmin) xmin = iter->GetPoint(i).x;
			if(iter->GetPoint(i).x > xmax) xmax = iter->GetPoint(i).x;
			if(iter->GetPoint(i).y < ymin) ymin = iter->GetPoint(i).y;
			if(iter->GetPoint(i).y > ymax) ymax = iter->GetPoint(i).y;
		}
		
	}


	for(iter=polys->begin(); iter!=polys->end(); iter++) {
		DrawPoly(&img, &(*iter), xmin, xmax, ymin, ymax);
	}

	io.SaveImage(filename,&img);
}

int main() {
	CGPPPartition pp;
	list<CGPPPoly> testpolys,result, result1;

	ReadPolyList("input.txt",&testpolys);

	if(testpolys.size()>1)
	{
		// cout<<testpolys.front().GetOrientation()<<endl;
		// cout<<testpolys.back().GetOrientation()<<endl;
		testpolys.front().SetOrientation(1);
		testpolys.back().SetOrientation(-1);
		// cout<<testpolys.front().GetOrientation()<<endl;
		// cout<<testpolys.back().GetOrientation()<<endl;
	}

	else
		testpolys.front().SetOrientation(1);

	DrawPolyList("input.bmp", &testpolys);
	if(!pp.Triangulate_Ear_Clipping(&testpolys,&result)) printf("Error\n");
	if(!pp.Convex_Partition_Hertel_Mehlhorn(&testpolys,&result1)) printf("Error\n");
	DrawPolyList("triangulation.bmp", &result);
	DrawPolyList("polygonization.bmp", &result1);

    return 0;

}
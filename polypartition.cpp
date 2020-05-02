#include <bits/stdc++.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <list>
#include <algorithm>
#include <set>
#include <vector>
#include <stdexcept>

using namespace std;

#include "polypartition.h"

#define TPPL_VERTEXTYPE_REGULAR 0
#define TPPL_VERTEXTYPE_START 1
#define TPPL_VERTEXTYPE_END 2
#define TPPL_VERTEXTYPE_SPLIT 3
#define TPPL_VERTEXTYPE_MERGE 4

TPPLPoly::TPPLPoly() { 
	hole = false;
	numpoints = 0;
	points = NULL;
}

TPPLPoly::~TPPLPoly() {
	if(points) delete [] points;
}

void TPPLPoly::Clear() {
	if(points) delete [] points;
	hole = false;
	numpoints = 0;
	points = NULL;
}

void TPPLPoly::Init(long numpoints) {
	Clear();
	this->numpoints = numpoints;
	points = new TPPLPoint[numpoints];
}

void TPPLPoly::Triangle(TPPLPoint &p1, TPPLPoint &p2, TPPLPoint &p3) {
	Init(3);
	points[0] = p1;
	points[1] = p2;
	points[2] = p3;
}

TPPLPoly::TPPLPoly(const TPPLPoly &src) : TPPLPoly() {
	hole = src.hole;
	numpoints = src.numpoints;

	if(numpoints > 0) {
		points = new TPPLPoint[numpoints];
		memcpy(points, src.points, numpoints*sizeof(TPPLPoint));
	}
}

TPPLPoly& TPPLPoly::operator=(const TPPLPoly &src) {
	Clear();
	hole = src.hole;
	numpoints = src.numpoints;
	
	if(numpoints > 0) {
		points = new TPPLPoint[numpoints];
		memcpy(points, src.points, numpoints*sizeof(TPPLPoint));
	}
	
	return *this;
}

int TPPLPoly::GetOrientation() const {
	long i1,i2;
	tppl_float area = 0;
	for(i1=0; i1<numpoints; i1++) {
		i2 = i1+1;
		if(i2 == numpoints) i2 = 0;
		area += points[i1].x * points[i2].y - points[i1].y * points[i2].x;
	}
	if(area>0) return TPPL_CCW;
	if(area<0) return TPPL_CW;
	return 0;
}

void TPPLPoly::SetOrientation(int orientation) {
	int polyorientation = GetOrientation();
	if(polyorientation&&(polyorientation!=orientation)) {
		Invert();
	}
}

void TPPLPoly::Invert() {
	std::reverse(points, points + numpoints);
}

TPPLPartition::PartitionVertex::PartitionVertex() : previous(NULL), next(NULL) {

}

TPPLPoint TPPLPartition::Normalize(const TPPLPoint &p) {
	TPPLPoint r;
	tppl_float n = sqrt(p.x*p.x + p.y*p.y);
	if(n!=0) {
		r = p/n;
	} else {
		r.x = 0;
		r.y = 0;
	}
	return r;
}

tppl_float TPPLPartition::Distance(const TPPLPoint &p1, const TPPLPoint &p2) {
	tppl_float dx,dy;
	dx = p2.x - p1.x;
	dy = p2.y - p1.y;
	return(sqrt(dx*dx + dy*dy));
}

//checks if two lines intersect
int TPPLPartition::Intersects(TPPLPoint &p11, TPPLPoint &p12, TPPLPoint &p21, TPPLPoint &p22) {
	if((p11.x == p21.x)&&(p11.y == p21.y)) return 0;
	if((p11.x == p22.x)&&(p11.y == p22.y)) return 0;
	if((p12.x == p21.x)&&(p12.y == p21.y)) return 0;
	if((p12.x == p22.x)&&(p12.y == p22.y)) return 0;

	TPPLPoint v1ort,v2ort,v;
	tppl_float dot11,dot12,dot21,dot22;

	v1ort.x = p12.y-p11.y;
	v1ort.y = p11.x-p12.x;

	v2ort.x = p22.y-p21.y;
	v2ort.y = p21.x-p22.x;

	v = p21-p11;
	dot21 = v.x*v1ort.x + v.y*v1ort.y;
	v = p22-p11;
	dot22 = v.x*v1ort.x + v.y*v1ort.y;

	v = p11-p21;
	dot11 = v.x*v2ort.x + v.y*v2ort.y;
	v = p12-p21;
	dot12 = v.x*v2ort.x + v.y*v2ort.y;

	if(dot11*dot12>0) return 0;
	if(dot21*dot22>0) return 0;

	return 1;
}

//removes holes from inpolys by merging them with non-holes
int TPPLPartition::RemoveHoles(TPPLPolyList *inpolys, TPPLPolyList *outpolys) {
	TPPLPolyList polys;
	TPPLPolyList::iterator holeiter,polyiter,iter,iter2;
	long i,i2,holepointindex,polypointindex;
	TPPLPoint holepoint,polypoint,bestpolypoint;
	TPPLPoint linep1,linep2;
	TPPLPoint v1,v2;
	TPPLPoly newpoly;
	bool hasholes;
	bool pointvisible;
	bool pointfound;
	
	//check for trivial case (no holes)
	hasholes = false;
	for(iter = inpolys->begin(); iter!=inpolys->end(); iter++) {
		if(iter->IsHole()) {
			hasholes = true;
			break;
		}
	}
	if(!hasholes) {
		for(iter = inpolys->begin(); iter!=inpolys->end(); iter++) {
			outpolys->push_back(*iter);
		}
		return 1;
	}

	polys = *inpolys;

	while(1) {
		//find the hole point with the largest x
		hasholes = false;
		for(iter = polys.begin(); iter!=polys.end(); iter++) {
			if(!iter->IsHole()) continue;

			if(!hasholes) {
				hasholes = true;
				holeiter = iter;
				holepointindex = 0;
			}

			for(i=0; i < iter->GetNumPoints(); i++) {
				if(iter->GetPoint(i).x > holeiter->GetPoint(holepointindex).x) {
					holeiter = iter;
					holepointindex = i;
				}
			}
		}
		if(!hasholes) break;
		holepoint = holeiter->GetPoint(holepointindex);
		
		pointfound = false;
		for(iter = polys.begin(); iter!=polys.end(); iter++) {
			if(iter->IsHole()) continue;
			for(i=0; i < iter->GetNumPoints(); i++) {
				if(iter->GetPoint(i).x <= holepoint.x) continue;
				if(!InCone(iter->GetPoint((i+iter->GetNumPoints()-1)%(iter->GetNumPoints())),
					iter->GetPoint(i),
					iter->GetPoint((i+1)%(iter->GetNumPoints())),
					holepoint)) 
					continue;
				polypoint = iter->GetPoint(i);
				if(pointfound) {
					v1 = Normalize(polypoint-holepoint);
					v2 = Normalize(bestpolypoint-holepoint);
					if(v2.x > v1.x) continue;				
				}
				pointvisible = true;
				for(iter2 = polys.begin(); iter2!=polys.end(); iter2++) {
					if(iter2->IsHole()) continue;
					for(i2=0; i2 < iter2->GetNumPoints(); i2++) {
						linep1 = iter2->GetPoint(i2);
						linep2 = iter2->GetPoint((i2+1)%(iter2->GetNumPoints()));
						if(Intersects(holepoint,polypoint,linep1,linep2)) {
							pointvisible = false;
							break;
						}
					}
					if(!pointvisible) break;
				}
				if(pointvisible) {
					pointfound = true;
					bestpolypoint = polypoint;
					polyiter = iter;
					polypointindex = i;
				}
			}
		}

		if(!pointfound) return 0;

		newpoly.Init(holeiter->GetNumPoints() + polyiter->GetNumPoints() + 2);
		i2 = 0;
		for(i=0;i<=polypointindex;i++) {
			newpoly[i2] = polyiter->GetPoint(i);
			i2++;
		}
		for(i=0;i<=holeiter->GetNumPoints();i++) {
			newpoly[i2] = holeiter->GetPoint((i+holepointindex)%holeiter->GetNumPoints());
			i2++;
		}
		for(i=polypointindex;i<polyiter->GetNumPoints();i++) {
			newpoly[i2] = polyiter->GetPoint(i);
			i2++;
		}
		
		polys.erase(holeiter);
		polys.erase(polyiter);
		polys.push_back(newpoly);
	}

	for(iter = polys.begin(); iter!=polys.end(); iter++) {
		outpolys->push_back(*iter);
	}
	
	return 1;
}

bool TPPLPartition::IsConvex(TPPLPoint& p1, TPPLPoint& p2, TPPLPoint& p3) {
	tppl_float tmp;
	tmp = (p3.y-p1.y)*(p2.x-p1.x)-(p3.x-p1.x)*(p2.y-p1.y);
	if(tmp>0) return 1;
	else return 0;
}

bool TPPLPartition::IsReflex(TPPLPoint& p1, TPPLPoint& p2, TPPLPoint& p3) {
	tppl_float tmp;
	tmp = (p3.y-p1.y)*(p2.x-p1.x)-(p3.x-p1.x)*(p2.y-p1.y);
	if(tmp<0) return 1;
	else return 0;
}

bool TPPLPartition::IsInside(TPPLPoint& p1, TPPLPoint& p2, TPPLPoint& p3, TPPLPoint &p) {
	if(IsConvex(p1,p,p2)) return false;
	if(IsConvex(p2,p,p3)) return false;
	if(IsConvex(p3,p,p1)) return false;
	return true;
}

bool TPPLPartition::InCone(TPPLPoint &p1, TPPLPoint &p2, TPPLPoint &p3, TPPLPoint &p) {
	bool convex;

	convex = IsConvex(p1,p2,p3);

	if(convex) {
		if(!IsConvex(p1,p2,p)) return false;
		if(!IsConvex(p2,p3,p)) return false;
		return true;
	} else {
		if(IsConvex(p1,p2,p)) return true;
		if(IsConvex(p2,p3,p)) return true;
		return false;
	}
}

bool TPPLPartition::InCone(PartitionVertex *v, TPPLPoint &p) {
	TPPLPoint p1,p2,p3;

	p1 = v->previous->p;
	p2 = v->p;
	p3 = v->next->p;

	return InCone(p1,p2,p3,p);
}

void TPPLPartition::UpdateVertexReflexity(PartitionVertex *v) {
	PartitionVertex *v1 = NULL,*v3 = NULL;
	v1 = v->previous;
	v3 = v->next;
	v->isConvex = !IsReflex(v1->p,v->p,v3->p);	
}

void TPPLPartition::UpdateVertex(PartitionVertex *v, PartitionVertex *vertices, long numvertices) {
	long i;
	PartitionVertex *v1 = NULL,*v3 = NULL;
	TPPLPoint vec1,vec3;

	v1 = v->previous;
	v3 = v->next;

	v->isConvex = IsConvex(v1->p,v->p,v3->p);

	vec1 = Normalize(v1->p - v->p);
	vec3 = Normalize(v3->p - v->p);
	v->angle = vec1.x*vec3.x + vec1.y*vec3.y;

	if(v->isConvex) {
		v->isEar = true;
		for(i=0;i<numvertices;i++) {
			if((vertices[i].p.x==v->p.x)&&(vertices[i].p.y==v->p.y)) continue;
			if((vertices[i].p.x==v1->p.x)&&(vertices[i].p.y==v1->p.y)) continue;
			if((vertices[i].p.x==v3->p.x)&&(vertices[i].p.y==v3->p.y)) continue;
			if(IsInside(v1->p,v->p,v3->p,vertices[i].p)) {
				v->isEar = false;
				break;
			}
		}
	} else {
		v->isEar = false;
	}
}

//triangulation by ear removal
int TPPLPartition::Triangulate_EC(TPPLPoly *poly, TPPLPolyList *triangles) {
	if(!poly->Valid()) return 0;

	long numvertices;
	PartitionVertex *vertices = NULL;
	PartitionVertex *ear = NULL;
	TPPLPoly triangle;
	long i,j;
	bool earfound;

	if(poly->GetNumPoints() < 3) return 0;
	if(poly->GetNumPoints() == 3) {
		triangles->push_back(*poly);
		return 1;
	}

	numvertices = poly->GetNumPoints();

	vertices = new PartitionVertex[numvertices];
	for(i=0;i<numvertices;i++) {
		vertices[i].isActive = true;
		vertices[i].p = poly->GetPoint(i);
		if(i==(numvertices-1)) vertices[i].next=&(vertices[0]);
		else vertices[i].next=&(vertices[i+1]);
		if(i==0) vertices[i].previous = &(vertices[numvertices-1]);
		else vertices[i].previous = &(vertices[i-1]);
	}
	for(i=0;i<numvertices;i++) {
		UpdateVertex(&vertices[i],vertices,numvertices);
	}

	for(i=0;i<numvertices-3;i++) {
		earfound = false;
		//find the most extruded ear
		for(j=0;j<numvertices;j++) {
			if(!vertices[j].isActive) continue;
			if(!vertices[j].isEar) continue;
			if(!earfound) {
				earfound = true;
				ear = &(vertices[j]);
			} else {
				if(vertices[j].angle > ear->angle) {
					ear = &(vertices[j]);				
				}
			}
		}
		if(!earfound) {
			delete [] vertices;
			return 0;
		}

		triangle.Triangle(ear->previous->p,ear->p,ear->next->p);
		triangles->push_back(triangle);

		ear->isActive = false;
		ear->previous->next = ear->next;
		ear->next->previous = ear->previous;

		if(i==numvertices-4) break;

		UpdateVertex(ear->previous,vertices,numvertices);
		UpdateVertex(ear->next,vertices,numvertices);
	}
	for(i=0;i<numvertices;i++) {
		if(vertices[i].isActive) {
			triangle.Triangle(vertices[i].previous->p,vertices[i].p,vertices[i].next->p);
			triangles->push_back(triangle);
			break;
		}
	}

	delete [] vertices;

	return 1;
}

int TPPLPartition::Triangulate_EC(TPPLPolyList *inpolys, TPPLPolyList *triangles) {
	TPPLPolyList outpolys;
	TPPLPolyList::iterator iter;
	
	if(!RemoveHoles(inpolys,&outpolys)) return 0;
	for(iter=outpolys.begin();iter!=outpolys.end();iter++) {
		if(!Triangulate_EC(&(*iter),triangles)) return 0;
	}
	return 1;
}

int TPPLPartition::ConvexPartition_HM(TPPLPoly *poly, TPPLPolyList *parts) {
	if(!poly->Valid()) return 0;
	
	TPPLPolyList triangles;
	TPPLPolyList::iterator iter1,iter2;
	TPPLPoly *poly1 = NULL,*poly2 = NULL;
	TPPLPoly newpoly;
	TPPLPoint d1,d2,p1,p2,p3;
	long i11,i12,i21,i22,i13,i23,j,k;
	bool isdiagonal;
	long numreflex;

	//check if the poly is already convex
	numreflex = 0;
	for(i11=0;i11<poly->GetNumPoints();i11++) {
		if(i11==0) i12 = poly->GetNumPoints()-1;
		else i12=i11-1;
		if(i11==(poly->GetNumPoints()-1)) i13=0;
		else i13=i11+1;
		if(IsReflex(poly->GetPoint(i12),poly->GetPoint(i11),poly->GetPoint(i13))) {
			numreflex = 1;
			break;
		}
	}
	if(numreflex == 0) {
		parts->push_back(*poly);
		return 1;
	}

	if(!Triangulate_EC(poly,&triangles)) return 0;

	for(iter1 = triangles.begin(); iter1 != triangles.end(); iter1++) {
		poly1 = &(*iter1);
		for(i11=0;i11<poly1->GetNumPoints();i11++) {
			d1 = poly1->GetPoint(i11);
			i12 = (i11+1)%(poly1->GetNumPoints());
			d2 = poly1->GetPoint(i12);

			isdiagonal = false;
			for(iter2 = iter1; iter2 != triangles.end(); iter2++) {
				if(iter1 == iter2) continue;
				poly2 = &(*iter2);

				for(i21=0;i21<poly2->GetNumPoints();i21++) {
					if((d2.x != poly2->GetPoint(i21).x)||(d2.y != poly2->GetPoint(i21).y)) continue;
					i22 = (i21+1)%(poly2->GetNumPoints());
					if((d1.x != poly2->GetPoint(i22).x)||(d1.y != poly2->GetPoint(i22).y)) continue;
					isdiagonal = true;
					break;
				}
				if(isdiagonal) break;
			}

			if(!isdiagonal) continue;

			p2 = poly1->GetPoint(i11);
			if(i11 == 0) i13 = poly1->GetNumPoints()-1;
			else i13 = i11-1;
			p1 = poly1->GetPoint(i13);
			if(i22 == (poly2->GetNumPoints()-1)) i23 = 0;
			else i23 = i22+1;
			p3 = poly2->GetPoint(i23);

			if(!IsConvex(p1,p2,p3)) continue;
			
			p2 = poly1->GetPoint(i12);
			if(i12 == (poly1->GetNumPoints()-1)) i13 = 0;
			else i13 = i12+1;
			p3 = poly1->GetPoint(i13);
			if(i21 == 0) i23 = poly2->GetNumPoints()-1;
			else i23 = i21-1;
			p1 = poly2->GetPoint(i23);
			
			if(!IsConvex(p1,p2,p3)) continue;

			newpoly.Init(poly1->GetNumPoints()+poly2->GetNumPoints()-2);
			k = 0;
			for(j=i12;j!=i11;j=(j+1)%(poly1->GetNumPoints())) {
				newpoly[k] = poly1->GetPoint(j);
				k++;
			}
			for(j=i22;j!=i21;j=(j+1)%(poly2->GetNumPoints())) {
				newpoly[k] = poly2->GetPoint(j);
				k++;
			}

			triangles.erase(iter2);
			*iter1 = newpoly;
			poly1 = &(*iter1);
			i11 = -1;

			continue;
		}
	}

	for(iter1 = triangles.begin(); iter1 != triangles.end(); iter1++) {
		parts->push_back(*iter1);
	}

	return 1;
}

int TPPLPartition::ConvexPartition_HM(TPPLPolyList *inpolys, TPPLPolyList *parts) {
	TPPLPolyList outpolys;
	TPPLPolyList::iterator iter;
	
	if(!RemoveHoles(inpolys,&outpolys)) return 0;
	for(iter=outpolys.begin();iter!=outpolys.end();iter++) {
		if(!ConvexPartition_HM(&(*iter),parts)) return 0;
	}
	return 1;
}
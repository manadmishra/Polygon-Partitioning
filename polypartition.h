#ifndef POLYPARTITION_H
#define POLYPARTITION_H

#include <list>

#include <set>

typedef double tppl_float;
#include <bits/stdc++.h>
#define CGPP_CCW 1
#define CGPP_CW -1

struct CGPPPoint {
    tppl_float x;
    tppl_float y;
    int id;
    
    CGPPPoint operator + (const CGPPPoint& p) const {
        CGPPPoint r;
        r.x = x + p.x;
        r.y = y + p.y;
        return r;
    }
    
    CGPPPoint operator - (const CGPPPoint& p) const {
        CGPPPoint r;
        r.x = x - p.x;
        r.y = y - p.y;
        return r;
    }
    
    CGPPPoint operator * (const tppl_float f ) const {
        CGPPPoint r;
        r.x = x*f;
        r.y = y*f;
        return r;
    }
    
    CGPPPoint operator / (const tppl_float f ) const {
        CGPPPoint r;
        r.x = x/f;
        r.y = y/f;
        return r;
    }
    
    bool operator==(const CGPPPoint& p) const {
        if((x == p.x)&&(y==p.y)) return true;
        else return false;
    }
    
    bool operator!=(const CGPPPoint& p) const {
        if((x == p.x)&&(y==p.y)) return false;
        else return true;
    }
};


class CGPPPoly {
    protected:
        
        CGPPPoint *points;
        long numpoints;
        bool hole;
        
    public:
        
        CGPPPoly();
        ~CGPPPoly();
        
        CGPPPoly(const CGPPPoly &src);
        CGPPPoly& operator=(const CGPPPoly &src);
        
        long GetNumPoints() const {
            return numpoints;
        }
        
        bool IsHole() const {
            return hole;
        }
        
        void SetHole(bool hole) {
            this->hole = hole;
        }
        
        CGPPPoint &GetPoint(long i) {
            return points[i];
        }
        
        const CGPPPoint &GetPoint(long i) const {
            return points[i];
        }

        CGPPPoint *GetPoints() {
            return points;
        }
        
        CGPPPoint& operator[] (int i) {
            return points[i];
        }

        const CGPPPoint& operator[] (int i) const {
            return points[i];
        }
        
        void Clear();
        
        void Init(long numpoints);
        
        void Triangle(CGPPPoint &p1, CGPPPoint &p2, CGPPPoint &p3);
        
        void Invert();
        
        int GetOrientation() const;
        
        void SetOrientation(int orientation);

        inline bool Valid() const { return this->numpoints >= 3; }
};

#ifdef CGPP_ALLOCATOR
typedef std::list<CGPPPoly, CGPP_ALLOCATOR(CGPPPoly)> CGPPPolyList;
#else
typedef std::list<CGPPPoly> CGPPPolyList;
#endif

class CGPPPartition {
    protected:
        struct PartitionVertex {
            bool isActive;
            bool isConvex;
            bool isEar;
            
            CGPPPoint p;
            tppl_float angle;
            PartitionVertex *previous;
            PartitionVertex *next;
            
            PartitionVertex();
        };
        
        
        struct Diagonal {
            long index1;
            long index2;
        };
        
        bool IsConvex(CGPPPoint& p1, CGPPPoint& p2, CGPPPoint& p3);
        bool IsReflex(CGPPPoint& p1, CGPPPoint& p2, CGPPPoint& p3);
        bool IsInside(CGPPPoint& p1, CGPPPoint& p2, CGPPPoint& p3, CGPPPoint &p);
        
        bool InCone(CGPPPoint &p1, CGPPPoint &p2, CGPPPoint &p3, CGPPPoint &p);
        bool InCone(PartitionVertex *v, CGPPPoint &p);
        
        int Intersects(CGPPPoint &p11, CGPPPoint &p12, CGPPPoint &p21, CGPPPoint &p22);
        
        CGPPPoint Normalize(const CGPPPoint &p);
        tppl_float Distance(const CGPPPoint &p1, const CGPPPoint &p2);
        
        void UpdateVertexReflexity(PartitionVertex *v);
        void UpdateVertex(PartitionVertex *v,PartitionVertex *vertices, long numvertices);
        
        
    public:
        
        int RemoveHoles(CGPPPolyList *inpolys, CGPPPolyList *outpolys);
        
        int Triangulate_Ear_Clipping(CGPPPoly *poly, CGPPPolyList *triangles);
        
        int Triangulate_Ear_Clipping(CGPPPolyList *inpolys, CGPPPolyList *triangles);
        
        int Convex_Partition_Hertel_Mehlhorn(CGPPPoly *poly, CGPPPolyList *parts);
        
        int Convex_Partition_Hertel_Mehlhorn(CGPPPolyList *inpolys, CGPPPolyList *parts);
        
};


#endif

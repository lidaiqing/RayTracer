/*
   utils.c - F.J. Estrada, Dec. 9, 2010

   Utilities for the ray tracer. You will need to complete
   some of the functions in this file. Look for the sections
   marked "TO DO". Be sure to read the rest of the file and
   understand how the entire code works.
*/

#include "utils.h"
#include <stdlib.h>
#include <math.h>
using namespace tinyply;

typedef std::chrono::time_point<std::chrono::high_resolution_clock> timepoint;
std::chrono::high_resolution_clock c;

inline std::chrono::time_point<std::chrono::high_resolution_clock> now()
{
	return c.now();
}

inline double difference_micros(timepoint start, timepoint end)
{
	return (double)std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}

template<class T>
const T& min(const T& a, const T& b)
{
    return (b < a) ? b : a;
}

// A useful 4x4 identity matrix which can be used at any point to
// initialize or reset object transformations
double eye4x4[4][4]={{1.0, 0.0, 0.0, 0.0},
                    {0.0, 1.0, 0.0, 0.0},
                    {0.0, 0.0, 1.0, 0.0},
                    {0.0, 0.0, 0.0, 1.0}};

/////////////////////////////////////////////
// Primitive data structure section
/////////////////////////////////////////////
struct point3D *newPoint(double px, double py, double pz)
{
 // Allocate a new point structure, initialize it to
 // the specified coordinates, and return a pointer
 // to it.

 struct point3D *pt=(struct point3D *)calloc(1,sizeof(struct point3D));
 if (!pt) fprintf(stderr,"Out of memory allocating point structure!\n");
 else
 {
  pt->px=px;
  pt->py=py;
  pt->pz=pz;
  pt->pw=1.0;
 }
 return(pt);
}

struct pointLS *newPLS(struct point3D *p0, double r, double g, double b)
{
 // Allocate a new point light sourse structure. Initialize the light
 // source to the specified RGB colour
 // Note that this is a point light source in that it is a single point
 // in space, if you also want a uniform direction for light over the
 // scene (a so-called directional light) you need to place the
 // light source really far away.

 struct pointLS *ls=(struct pointLS *)calloc(1,sizeof(struct pointLS));
 if (!ls) fprintf(stderr,"Out of memory allocating light source!\n");
 else
 {
  memcpy(&ls->p0,p0,sizeof(struct point3D));	// Copy light source location

  ls->col.R=r;					// Store light source colour and
  ls->col.G=g;					// intensity
  ls->col.B=b;
 }
 return(ls);
}

/////////////////////////////////////////////
// Ray and normal transforms
/////////////////////////////////////////////
inline void rayTransform(struct ray3D *ray_orig, struct ray3D *ray_transformed, struct object3D *obj)
{
 // Transforms a ray using the inverse transform for the specified object. This is so that we can
 // use the intersection test for the canonical object. Note that this has to be done carefully!

 ///////////////////////////////////////////
 // TO DO: Complete this function
 ///////////////////////////////////////////
    // transform ray by obj's inverse matrix
    matVecMult(obj->Tinv, &ray_transformed->p0);
    matVecMult(obj->Tinv, &ray_transformed->d);
    ray_transformed->d.pw = 0;
}

inline void normalTransform(struct point3D *n_orig, struct point3D *n_transformed, struct object3D *obj)
{
 // Computes the normal at an affinely transformed point given the original normal and the
 // object's inverse transformation. From the notes:
 // n_transformed=A^-T*n normalized.

 ///////////////////////////////////////////
 // TO DO: Complete this function
 ///////////////////////////////////////////

 /* Create a new normal*/

 double Tinv_transpose3x3[4][4]; //Matrix after transpose//
 /* 3X3 matrix part */
 Tinv_transpose3x3[0][0]=obj->Tinv[0][0];
 Tinv_transpose3x3[0][1]=obj->Tinv[1][0];
 Tinv_transpose3x3[0][2]=obj->Tinv[2][0];
 Tinv_transpose3x3[1][0]=obj->Tinv[0][1];
 Tinv_transpose3x3[1][1]=obj->Tinv[1][1];
 Tinv_transpose3x3[1][2]=obj->Tinv[2][1];
 Tinv_transpose3x3[2][0]=obj->Tinv[0][2];
 Tinv_transpose3x3[2][1]=obj->Tinv[1][2];
 Tinv_transpose3x3[2][2]=obj->Tinv[2][2];

 /* should not change the affine part*/
 Tinv_transpose3x3[3][0]=obj->Tinv[3][0];
 Tinv_transpose3x3[3][1]=obj->Tinv[3][1];
 Tinv_transpose3x3[3][2]=obj->Tinv[3][2];
 Tinv_transpose3x3[3][3]=obj->Tinv[3][3];
 Tinv_transpose3x3[0][3]=obj->Tinv[0][3];
 Tinv_transpose3x3[1][3]=obj->Tinv[1][3];
 Tinv_transpose3x3[2][3]=obj->Tinv[2][3];

 /*Transpose the inverse matrix first*/

 //Multiply the matrix with the normal vector//
 matVecMult(Tinv_transpose3x3, n_transformed);
 n_transformed->pw = 0;
}

/////////////////////////////////////////////
// Object management section
/////////////////////////////////////////////
struct object3D *newTriangle(double ra, double rd, double rs, double rg, double r, double g, double b, double alpha, double r_index, double shiny)
{

 struct object3D *triangle=(struct object3D *)calloc(1,sizeof(struct object3D));

 if (!triangle) fprintf(stderr,"Unable to allocate new plane, out of memory!\n");
 else
 {
  triangle->alb.ra=ra;
  triangle->alb.rd=rd;
  triangle->alb.rs=rs;
  triangle->alb.rg=rg;
  triangle->col.R=r;
  triangle->col.G=g;
  triangle->col.B=b;
  triangle->alpha=alpha;
  triangle->r_index=r_index;
  triangle->shinyness=shiny;
  triangle->intersect=NULL;
  triangle->texImg=NULL;
  memcpy(&triangle->T[0][0],&eye4x4[0][0],16*sizeof(double));
  memcpy(&triangle->Tinv[0][0],&eye4x4[0][0],16*sizeof(double));
  triangle->textureMap=&texMap;
  triangle->frontAndBack=1;
  triangle->isLightSource=0;
 }
 return(triangle);
}

struct object3D *newPlane(double ra, double rd, double rs, double rg, double r, double g, double b, double alpha, double r_index, double shiny)
{
 // Intialize a new plane with the specified parameters:
 // ra, rd, rs, rg - Albedos for the components of the Phong model
 // r, g, b, - Colour for this plane
 // alpha - Transparency, must be set to 1 unless you are doing refraction
 // r_index - Refraction index if you are doing refraction.
 // shiny - Exponent for the specular component of the Phong model
 //
 // The plane is defined by the following vertices (CCW)
 // (1,1,0), (-1,1,0), (-1,-1,0), (1,-1,0)
 // With normal vector (0,0,1) (i.e. parallel to the XY plane)

 struct object3D *plane=(struct object3D *)calloc(1,sizeof(struct object3D));

 if (!plane) fprintf(stderr,"Unable to allocate new plane, out of memory!\n");
 else
 {
  plane->alb.ra=ra;
  plane->alb.rd=rd;
  plane->alb.rs=rs;
  plane->alb.rg=rg;
  plane->col.R=r;
  plane->col.G=g;
  plane->col.B=b;
  plane->alpha=alpha;
  plane->r_index=r_index;
  plane->shinyness=shiny;
  plane->intersect=&planeIntersect;
  plane->texImg=NULL;
  memcpy(&plane->T[0][0],&eye4x4[0][0],16*sizeof(double));
  memcpy(&plane->Tinv[0][0],&eye4x4[0][0],16*sizeof(double));
  plane->textureMap=&texMap;
  plane->frontAndBack=1;
  plane->isLightSource=0;
  plane->identity=1;
 }
 return(plane);
}

struct object3D *newSphere(double ra, double rd, double rs, double rg, double r, double g, double b, double alpha, double r_index, double shiny)
{
 // Intialize a new sphere with the specified parameters:
 // ra, rd, rs, rg - Albedos for the components of the Phong model
 // r, g, b, - Colour for this plane
 // alpha - Transparency, must be set to 1 unless you are doing refraction
 // r_index - Refraction index if you are doing refraction.
 // shiny -Exponent for the specular component of the Phong model
 //
 // This is assumed to represent a unit sphere centered at the origin.
 //

 struct object3D *sphere=(struct object3D *)calloc(1,sizeof(struct object3D));

 if (!sphere) fprintf(stderr,"Unable to allocate new sphere, out of memory!\n");
 else
 {
  sphere->alb.ra=ra;
  sphere->alb.rd=rd;
  sphere->alb.rs=rs;
  sphere->alb.rg=rg;
  sphere->col.R=r;
  sphere->col.G=g;
  sphere->col.B=b;
  sphere->alpha=alpha;
  sphere->r_index=r_index;
  sphere->shinyness=shiny;
  sphere->intersect=&sphereIntersect;
  sphere->texImg=NULL;
  memcpy(&sphere->T[0][0],&eye4x4[0][0],16*sizeof(double));
  memcpy(&sphere->Tinv[0][0],&eye4x4[0][0],16*sizeof(double));
  sphere->textureMap=&texMap;
  sphere->frontAndBack=0;
  sphere->isLightSource=0;
  sphere->identity=0;
 }
 return(sphere);
}

///////////////////////////////////////////////////////////////////////////////////////
// TO DO:
//	Complete the functions that compute intersections for the canonical plane
//      and canonical sphere with a given ray. This is the most fundamental component
//      of the raytracer.
///////////////////////////////////////////////////////////////////////////////////////
void planeIntersect(struct object3D *plane, struct ray3D *ray, double *lambda, struct point3D *p, struct point3D *n, double *a, double *b)
{
 // Computes and returns the value of 'lambda' at the intersection
 // between the specified ray and the specified canonical plane.

 /////////////////////////////////
 // TO DO: Complete this function.
 /////////////////////////////////
 // transform ray to model space
    double model_intersection_x;
    double model_intersection_y;
    struct ray3D* ray_transformed = newRay(&ray->p0, &ray->d);
    rayTransform(ray, ray_transformed, plane);
 // check if ray is parallel to unit plane
    if (ray_transformed->d.pz == 0) {
      *lambda = -1;
      free(ray_transformed);
      return;
    }
    double t = -ray_transformed->p0.pz / ray_transformed->d.pz;
    if (t < 0) {
      *lambda = -1;
      free(ray_transformed);
      return;
    }

    rayPosition(ray_transformed, t, p);
    model_intersection_x=p->px;
    model_intersection_y=p->py;
    // check if it is behind the camera
    if (p->px < -1 || p->px > 1 || p->py < -1 || p->py > 1) {
      *lambda = -1;
      free(ray_transformed);
      return;
    }
    *lambda = t;
    struct point3D* n_orig = newPoint(0,0,1);
    n_orig->pw = 0;
    // transform back to world space
    matVecMult(plane->T, p);
    n->px = 0;
    n->py = 0;
    n->pz = -1;
    n->pw = 0;
    normalTransform(n_orig, n, plane);
    normalize(n);
    //std::cout<<"normal " << n->px << " " << n->py << " " << n->pz << " " << n->pw << std::endl;

    // free space
    free(ray_transformed);
    free(n_orig);
    //std::cout << t << std::endl;
 // The plane is defined by the following vertices (CCW)
 // (1,1,0), (-1,1,0), (-1,-1,0), (1,-1,0)
 // With normal vector (0,0,1) (i.e. parallel to the XY plane)
 // choose point a as (1,1,0), b as (-1,1,0), c as (-1,-1,0)
  // The plane is defined by the following vertices (CCW)
 // (1,1,0), (-1,1,0), (-1,-1,0), (1,-1,0)
    if (plane->texImg != NULL && plane->textureMap != NULL) {
    double transformed_x=model_intersection_x+1.0;
    double transformed_y=model_intersection_y+1.0;
    /* Find normalized coordination on the plane */
    *a=transformed_x/2.0;
    *b=transformed_y/2.0;
    }




}

void sphereIntersect(struct object3D *sphere, struct ray3D *ray, double *lambda, struct point3D *p, struct point3D *n, double *a, double *b)
{
 // Computes and returns the value of 'lambda' at the intersection
 // between the specified ray and the specified canonical sphere.

 /////////////////////////////////
 // TO DO: Complete this function.
 /////////////////////////////////
struct ray3D *transformed_ray = newRay(&ray->p0, &ray->d);
rayTransform(ray,transformed_ray,sphere);
/* Coefficient to solve the quadratic equation */
double coe_a,coe_b,coe_c;
struct point3D *e_minus_c = newPoint(transformed_ray->p0.px,transformed_ray->p0.py,transformed_ray->p0.pz);
struct point3D *intersection = newPoint(0,0,0);
/* d dot d */
coe_a = dot(&(transformed_ray->d),&(transformed_ray->d));
/* A point structure to store he value of light_source minus the origin */
/* d dot e-c */
coe_b = (double)2*dot(&(transformed_ray->d),e_minus_c);
/* e-c dot e-c */
coe_c = dot(e_minus_c,e_minus_c)-1;

double under_root=coe_b*coe_b-(double)4*coe_a*coe_c;

    if(under_root<0)
    {
     /* no intersection found*/
     *lambda=-1;
     free(transformed_ray);
     free(e_minus_c);
     free(intersection);
     return;
    }

    *lambda=min((-coe_b-(double)sqrt(under_root))/(2*coe_a),(-coe_b+(double)sqrt(under_root))/(2*coe_a));

  //   if(((-coe_b-(double)sqrt(under_root))/(2*coe_a))<0&&((-coe_b-(double)sqrt(under_root))/(2*coe_a))<0)
  //   {
  //     free(transformed_ray);
  //     free(e_minus_c);
  //     free(intersection);
  //     *lambda  = -1;
  //     return;
  //   }
  //   else if(((-coe_b-(double)sqrt(under_root))/(2*coe_a))<0)
  //   {
  //    *lambda=(-coe_b+(double)sqrt(under_root))/(2*coe_a);
  //   }

 /* find the point this ray intersect on the sphere*/
 rayPosition(transformed_ray, *lambda, intersection);
 /* the sphere is at the origin, so the coordinate of the intersection is the direction for the normal */
 struct point3D *normal=newPoint(intersection->px,intersection->py,intersection->pz);
 /* Indicate it is a direction, not a point */
 normal->pw = 0;


 /* Transfer model space normal to world space*/
 struct point3D *world_normal = newPoint(normal->px, normal->py, normal->pz);
 world_normal->pw = 0;
 /* Indicate it is a direction, not a point */
 normalTransform(normal, world_normal, sphere);
 normalize(world_normal);

 /* transfer model space intersection to world space intersection */
 matVecMult(sphere->T, intersection);

 p->px = intersection->px;
 p->py = intersection->py;
 p->pz = intersection->pz;
 p->pw = 1;

 n->px = world_normal->px;
 n->py = world_normal->py;
 n->pz = world_normal->pz;
 n->pw = 0;



 if (sphere->texImg != NULL && sphere->textureMap != NULL) {
 struct point3D z;
 z.px=0;
 z.py=0;
 z.pz=1;
 z.pw=0;
 struct point3D x;
 x.px=1;
 x.py=0;
 x.pz=0;
 x.pw=0;

 double cos_phi=dot(normal,&z);
 double cos_theta=dot(normal,&x);
 double phi;
 double theta;
 /* Find normalized coordination on the plane */
 /* calculate the correct phi out of a pi */
 phi=acos(cos_phi);

 if(normal->py<0)
  {
   theta=(double)2*3.1415926-acos(cos_theta);
  }
  else
  {
   theta=acos(cos_theta);
  }
    *a=phi/3.141592;
    *b=theta/((double)2*3.141592);
    
      if(a<0||b<0)
  {
   std::cout<<"Error: "<<phi<<"  "<<theta<<std::endl; 
  }
    
}



 free(transformed_ray);
 free(intersection);
 free(e_minus_c);
 free(normal);
 free(world_normal);
}
void convert_xyz_to_cube_uv(double x, double y, double z, int *index, double *u, double *v)
{
/*
A cube texture indexes six texture maps from 0 to 5 in
order Positive X, Negative X, Positive Y, Negative Y, Positive Z, Negative Z
*/
    //std::cout<<" x: "<<x<<" y: "<<y<<" z "<<z<<std::endl;

  double absX = fabs(x);
  double absY = fabs(y);
  double absZ = fabs(z);

  int isXPositive = x > 0 ? 1 : 0;
  int isYPositive = y > 0 ? 1 : 0;
  int isZPositive = z > 0 ? 1 : 0;

  float maxAxis, uc, vc;

  //std::cout<<" absx: "<<absX<<" absY: "<<absY<<" absZ "<<absZ<<std::endl;

  // POSITIVE X
  if (isXPositive && absX >= absY && absX >= absZ) {
    // u (0 to 1) goes from +z to -z
    // v (0 to 1) goes from -y to +y
    maxAxis = absX;
    uc = -z;
    vc = y;
    *index = 0;
  }
  // NEGATIVE X
  if (!isXPositive && absX >= absY && absX >= absZ) {
    // u (0 to 1) goes from -z to +z
    // v (0 to 1) goes from -y to +y
    maxAxis = absX;
    uc = z;
    vc = y;
    *index = 1;
  }
  // POSITIVE Y
  if (isYPositive && absY >= absX && absY >= absZ) {
    // u (0 to 1) goes from -x to +x
    // v (0 to 1) goes from +z to -z
    maxAxis = absY;
    uc = x;
    vc = -z;
    *index = 2;
  }
  // NEGATIVE Y
  if (!isYPositive && absY >= absX && absY >= absZ) {
    // u (0 to 1) goes from -x to +x
    // v (0 to 1) goes from -z to +z
    maxAxis = absY;
    uc = x;
    vc = z;
    *index = 3;
  }
  // POSITIVE Z
  if (isZPositive && absZ >= absX && absZ >= absY) {
    // u (0 to 1) goes from -x to +x
    // v (0 to 1) goes from -y to +y
    maxAxis = absZ;
    uc = x;
    vc = y;
    *index = 4;
  }
  // NEGATIVE Z
  if (!isZPositive && absZ >= absX && absZ >= absY) {
    // u (0 to 1) goes from +x to -x
    // v (0 to 1) goes from -y to +y
    maxAxis = absZ;
    uc = -x;
    vc = y;
    *index = 5;
  }

   //std::cout<<" uc: "<<uc<<" vc "<<vc<<std::endl;

  // Convert range from -1 to 1 to 0 to 1
  *u = 0.5f * (uc / maxAxis + 1.0f);
  *v = 0.5f * (vc / maxAxis + 1.0f);
}

void convert_cube_uv_to_xyz(int index, float u, float v, float *x, float *y, float *z)
{}
void loadTexture(struct object3D *o, const char *filename)
{
 // Load a texture image from file and assign it to the
 // specified object
 if (o!=NULL)
 {
  if (o->texImg!=NULL)	// We have previously loaded a texture
  {			// for this object, need to de-allocate it
   if (o->texImg->rgbdata!=NULL) free(o->texImg->rgbdata);
   free(o->texImg);
  }
  o->texImg=readPPMimage(filename);	// Allocate new texture
 }
}

void texMap(struct image *img, double a, double b, double *R, double *G, double *B)
{
 /*
  Function to determine the colour of a textured object at
  the normalized texture coordinates (a,b).

  a and b are texture coordinates in [0 1].
  img is a pointer to the image structure holding the texture for
   a given object.

  The colour is returned in R, G, B. Uses bi-linear interpolation
  to determine texture colour.
 */

 //////////////////////////////////////////////////
 // TO DO (Assignment 4 only):
 //
 //  Complete this function to return the colour
 // of the texture image at the specified texture
 // coordinates. Your code should use bi-linear
 // interpolation to obtain the texture colour.
 //////////////////////////////////////////////////
 //*****Reminder:*****//
 //////////////////////////////////////////////////
 //     1                              2
 //   floor_x,ceiling_y           ceiling_x,ceiling_y
 //
 // ^                    + <---"some point"
 // |
 // |   3                              4
 // | floor_x,floor_y             ceiling_x,floor_y
 //  _________>
 /////////////////////////////////////////////////

  a=fabs(a) > 1 ? 0.5 : fabs(a);
  b=fabs(b) > 1 ? 0.5 : fabs(b);
  double x_location = (double)img->sx*a;
  double y_location = (double)img->sy*b;
  double *rgbIm=(double *)img->rgbdata;

  if(ceil(x_location)>=img->sx)
  {
  x_location-=1;
  }

  if(ceil(y_location)>=img->sy)
  {
  y_location-=1;
  }


  int floor_x = floor(x_location);
  int ceiling_x = ceil(x_location);

  int floor_y = floor(y_location);
  int ceiling_y = ceil(y_location);

  double R1,R2,R3,R4;
  double G1,G2,G3,G4;
  double B1,B2,B3,B4;



  /* Point 1:*/

  //std::cout<<" x: "<<img->sx<<" y: "<<img->sy<<std::endl;
  //std::cout<<" xp: "<<x_location<<" yp: "<<y_location<<std::endl;



  R1=*(rgbIm + 3 * (ceiling_y * img->sx + floor_x));
  G1=*(rgbIm + 3 * (ceiling_y * img->sx + floor_x) + 1);
  B1=*(rgbIm + 3 * (ceiling_y * img->sx + floor_x) + 2);


  /* Point 2:*/
  R2=*(rgbIm + 3*(ceiling_y*img->sx + ceiling_x));
  G2=*(rgbIm + 3*(ceiling_y*img->sx + ceiling_x) + 1);
  B2=*(rgbIm + 3*(ceiling_y*img->sx + ceiling_x) + 2);


  /* Point 3:*/
  R3=*(rgbIm + 3*(floor_y*img->sx + floor_x));
  G3=*(rgbIm + 3*(floor_y*img->sx + floor_x) + 1);
  B3=*(rgbIm + 3*(floor_y*img->sx + floor_x) + 2);


  /* Point 4:*/
  R4=*(rgbIm + 3*(floor_y*img->sx + ceiling_x));
  G4=*(rgbIm + 3*(floor_y*img->sx + ceiling_x) + 1);
  B4=*(rgbIm + 3*(floor_y*img->sx + ceiling_x) + 2);


  double x_dist = ceiling_x - x_location;
  double y_dist = y_location - floor_y;
  //double dist = x_dist * y_
  *R = x_dist * y_dist *R1+(x_location-(double)floor_x)*(y_location-(double)floor_y)*R2
    +((double)ceiling_x-x_location)*((double)ceiling_y-y_location)*R3+(x_location-(double)floor_x)*((double)ceiling_y-y_location)*R4;

  *G=((double)ceiling_x-x_location)*(y_location-(double)floor_y)*G1+(x_location-(double)floor_x)*(y_location-(double)floor_y)*G2
  +((double)ceiling_x-x_location)*((double)ceiling_y-y_location)*G3+(x_location-(double)floor_x)*((double)ceiling_y-y_location)*G4;

  *B=((double)ceiling_x-x_location)*(y_location-(double)floor_y)*B1+(x_location-(double)floor_x)*(y_location-(double)floor_y)*B2
  +((double)ceiling_x-x_location)*((double)ceiling_y-y_location)*B3+(x_location-(double)floor_x)*((double)ceiling_y-y_location)*B4;

  //std::cout<<" 1: "<<((double)ceiling_x-x_location)*(y_location-(double)floor_y)*R1<<" 2: "<<(x_location-(double)floor_x)*(y_location-(double)floor_y)*R2<<" 3: "
  //<<((double)ceiling_x-x_location)*((double)ceiling_y-y_location)*R3<<" R2: "<<R2<<std::endl;



  //im->rgbdata=(void *)calloc(size_x*size_y*3,sizeof(unsigned char));

 return;
}

void insertObject(struct object3D *o, struct object3D **list)
{
 if (o==NULL) return;
 // Inserts an object into the object list.
 if (*(list)==NULL)
 {
  *(list)=o;
  (*(list))->next=NULL;
 }
 else
 {
  o->next=(*(list))->next;
  (*(list))->next=o;
 }
}

void insertPLS(struct pointLS *l, struct pointLS **list)
{
 if (l==NULL) return;
 // Inserts a light source into the list of light sources
 if (*(list)==NULL)
 {
  *(list)=l;
  (*(list))->next=NULL;
 }
 else
 {
  l->next=(*(list))->next;
  (*(list))->next=l;
 }

}

void addAreaLight(float sx, float sy, float nx, float ny, float nz,\
                  float tx, float ty, float tz, int lx, int ly,\
                  float r, float g, float b, struct object3D **o_list, struct pointLS **l_list)
{
 /*
   This function sets up and inserts a rectangular area light source
   with size (sx, sy)
   orientation given by the normal vector (nx, ny, nz)
   centered at (tx, ty, tz)
   consisting of (lx x ly) point light sources (uniformly sampled)
   and with colour (r,g,b) - which also determines intensity

   Note that the light source is visible as a uniformly colored rectangle and
   casts no shadow. If you require a lightsource to shade another, you must
   make it into a proper solid box with backing and sides of non-light-emitting
   material
 */

  /////////////////////////////////////////////////////
  // TO DO: (Assignment 4!)
  // Implement this function to enable area light sources
  /////////////////////////////////////////////////////

}

///////////////////////////////////
// Geometric transformation section
///////////////////////////////////

void invert(double *T, double *Tinv)
{
 // Computes the inverse of transformation matrix T.
 // the result is returned in Tinv.

 float *U, *s, *V, *rv1;
 int singFlag, i;
 float T3x3[3][3],Tinv3x3[3][3];
 double tx,ty,tz;

 // Because of the fact we're using homogeneous coordinates, we must be careful how
 // we invert the transformation matrix. What we need is the inverse of the
 // 3x3 Affine transform, and -1 * the translation component. If we just invert
 // the entire matrix, junk happens.
 // So, we need a 3x3 matrix for inversion:
 T3x3[0][0]=(float)*(T+(0*4)+0);
 T3x3[0][1]=(float)*(T+(0*4)+1);
 T3x3[0][2]=(float)*(T+(0*4)+2);
 T3x3[1][0]=(float)*(T+(1*4)+0);
 T3x3[1][1]=(float)*(T+(1*4)+1);
 T3x3[1][2]=(float)*(T+(1*4)+2);
 T3x3[2][0]=(float)*(T+(2*4)+0);
 T3x3[2][1]=(float)*(T+(2*4)+1);
 T3x3[2][2]=(float)*(T+(2*4)+2);
 // Happily, we don't need to do this often.
 // Now for the translation component:
 tx=-(*(T+(0*4)+3));
 ty=-(*(T+(1*4)+3));
 tz=-(*(T+(2*4)+3));

 // Invert the affine transform
 U=NULL;
 s=NULL;
 V=NULL;
 rv1=NULL;
 singFlag=0;

 SVD(&T3x3[0][0],3,3,&U,&s,&V,&rv1);
 if (U==NULL||s==NULL||V==NULL)
 {
  fprintf(stderr,"Error: Matrix not invertible for this object, returning identity\n");
  memcpy(Tinv,eye4x4,16*sizeof(double));
  return;
 }

 // Check for singular matrices...
 for (i=0;i<3;i++) if (*(s+i)<1e-9) singFlag=1;
 if (singFlag)
 {
  fprintf(stderr,"Error: Transformation matrix is singular, returning identity\n");
  memcpy(Tinv,eye4x4,16*sizeof(double));
  return;
 }

 // Compute and store inverse matrix
 InvertMatrix(U,s,V,3,&Tinv3x3[0][0]);

 // Now stuff the transform into Tinv
 *(Tinv+(0*4)+0)=(double)Tinv3x3[0][0];
 *(Tinv+(0*4)+1)=(double)Tinv3x3[0][1];
 *(Tinv+(0*4)+2)=(double)Tinv3x3[0][2];
 *(Tinv+(1*4)+0)=(double)Tinv3x3[1][0];
 *(Tinv+(1*4)+1)=(double)Tinv3x3[1][1];
 *(Tinv+(1*4)+2)=(double)Tinv3x3[1][2];
 *(Tinv+(2*4)+0)=(double)Tinv3x3[2][0];
 *(Tinv+(2*4)+1)=(double)Tinv3x3[2][1];
 *(Tinv+(2*4)+2)=(double)Tinv3x3[2][2];
 *(Tinv+(0*4)+3)=Tinv3x3[0][0]*tx + Tinv3x3[0][1]*ty + Tinv3x3[0][2]*tz;
 *(Tinv+(1*4)+3)=Tinv3x3[1][0]*tx + Tinv3x3[1][1]*ty + Tinv3x3[1][2]*tz;
 *(Tinv+(2*4)+3)=Tinv3x3[2][0]*tx + Tinv3x3[2][1]*ty + Tinv3x3[2][2]*tz;
 *(Tinv+(3*4)+3)=1;

 free(U);
 free(s);
 free(V);
}

void RotateX(struct object3D *o, double theta)
{
 // Multiply the current object transformation matrix T in object o
 // by a matrix that rotates the object theta *RADIANS* around the
 // X axis.

 double R[4][4];
 memset(&R[0][0],0,16*sizeof(double));

 R[0][0]=1.0;
 R[1][1]=cos(theta);
 R[1][2]=-sin(theta);
 R[2][1]=sin(theta);
 R[2][2]=cos(theta);
 R[3][3]=1.0;

 matMult(R,o->T);
}

void RotateY(struct object3D *o, double theta)
{
 // Multiply the current object transformation matrix T in object o
 // by a matrix that rotates the object theta *RADIANS* around the
 // Y axis.

 double R[4][4];
 memset(&R[0][0],0,16*sizeof(double));

 R[0][0]=cos(theta);
 R[0][2]=sin(theta);
 R[1][1]=1.0;
 R[2][0]=-sin(theta);
 R[2][2]=cos(theta);
 R[3][3]=1.0;

 matMult(R,o->T);
}

void RotateZ(struct object3D *o, double theta)
{
 // Multiply the current object transformation matrix T in object o
 // by a matrix that rotates the object theta *RADIANS* around the
 // Z axis.

 double R[4][4];
 memset(&R[0][0],0,16*sizeof(double));

 R[0][0]=cos(theta);
 R[0][1]=-sin(theta);
 R[1][0]=sin(theta);
 R[1][1]=cos(theta);
 R[2][2]=1.0;
 R[3][3]=1.0;

 matMult(R,o->T);
}

void Translate(struct object3D *o, double tx, double ty, double tz)
{
 // Multiply the current object transformation matrix T in object o
 // by a matrix that translates the object by the specified amounts.

 double tr[4][4];
 memset(&tr[0][0],0,16*sizeof(double));

 tr[0][0]=1.0;
 tr[1][1]=1.0;
 tr[2][2]=1.0;
 tr[0][3]=tx;
 tr[1][3]=ty;
 tr[2][3]=tz;
 tr[3][3]=1.0;

 matMult(tr,o->T);
}

void Scale(struct object3D *o, double sx, double sy, double sz)
{
 // Multiply the current object transformation matrix T in object o
 // by a matrix that scales the object as indicated.

 double S[4][4];
 memset(&S[0][0],0,16*sizeof(double));

 S[0][0]=sx;
 S[1][1]=sy;
 S[2][2]=sz;
 S[3][3]=1.0;

 matMult(S,o->T);
}

void printmatrix(double mat[4][4])
{
 fprintf(stderr,"Matrix contains:\n");
 fprintf(stderr,"%f %f %f %f\n",mat[0][0],mat[0][1],mat[0][2],mat[0][3]);
 fprintf(stderr,"%f %f %f %f\n",mat[1][0],mat[1][1],mat[1][2],mat[1][3]);
 fprintf(stderr,"%f %f %f %f\n",mat[2][0],mat[2][1],mat[2][2],mat[2][3]);
 fprintf(stderr,"%f %f %f %f\n",mat[3][0],mat[3][1],mat[3][2],mat[3][3]);
}

/////////////////////////////////////////
// Camera and view setup
/////////////////////////////////////////
struct view *setupView(struct point3D *e, struct point3D *g, struct point3D *up, double f, double wl, double wt, double wsize)
{
 /*
   This function sets up the camera axes and viewing direction as discussed in the
   lecture notes.
   e - Camera center
   g - Gaze direction
   up - Up vector
   fov - Fild of view in degrees
   f - focal length
 */
 struct view *c;
 struct point3D *u, *v;

 u=v=NULL;

 // Allocate space for the camera structure
 c=(struct view *)calloc(1,sizeof(struct view));
 if (c==NULL)
 {
  fprintf(stderr,"Out of memory setting up camera model!\n");
  return(NULL);
 }

 // Set up camera center and axes
 c->e.px=e->px;		// Copy camera center location, note we must make sure
 c->e.py=e->py;		// the camera center provided to this function has pw=1
 c->e.pz=e->pz;
 c->e.pw=1;

 // Set up w vector (camera's Z axis). w=-g/||g||
 c->w.px=-g->px;
 c->w.py=-g->py;
 c->w.pz=-g->pz;
 c->w.pw=0;
 normalize(&c->w);

 // Set up the horizontal direction, which must be perpenticular to w and up
 u=cross(&c->w, up);
 normalize(u);
 c->u.px=u->px;
 c->u.py=u->py;
 c->u.pz=u->pz;
 c->u.pw=0;

 // Set up the remaining direction, v=(u x w)  - Mind the signs
 v=cross(&c->u, &c->w);
 normalize(v);
 c->v.px=v->px;
 c->v.py=v->py;
 c->v.pz=v->pz;
 c->v.pw=0;

 // Copy focal length and window size parameters
 c->f=f;
 c->wl=wl;
 c->wt=wt;
 c->wsize=wsize;

 // Set up coordinate conversion matrices
 // Camera2World matrix (M_cw in the notes)
 // Mind the indexing convention [row][col]
 c->C2W[0][0]=c->u.px;
 c->C2W[1][0]=c->u.py;
 c->C2W[2][0]=c->u.pz;
 c->C2W[3][0]=0;

 c->C2W[0][1]=c->v.px;
 c->C2W[1][1]=c->v.py;
 c->C2W[2][1]=c->v.pz;
 c->C2W[3][1]=0;

 c->C2W[0][2]=c->w.px;
 c->C2W[1][2]=c->w.py;
 c->C2W[2][2]=c->w.pz;
 c->C2W[3][2]=0;

 c->C2W[0][3]=c->e.px;
 c->C2W[1][3]=c->e.py;
 c->C2W[2][3]=c->e.pz;
 c->C2W[3][3]=1;

 // World2Camera matrix (M_wc in the notes)
 // Mind the indexing convention [row][col]
 c->W2C[0][0]=c->u.px;
 c->W2C[1][0]=c->v.px;
 c->W2C[2][0]=c->w.px;
 c->W2C[3][0]=0;

 c->W2C[0][1]=c->u.py;
 c->W2C[1][1]=c->v.py;
 c->W2C[2][1]=c->w.py;
 c->W2C[3][1]=0;

 c->W2C[0][2]=c->u.pz;
 c->W2C[1][2]=c->v.pz;
 c->W2C[2][2]=c->w.pz;
 c->W2C[3][2]=0;

 c->W2C[0][3]=-dot(&c->u,&c->e);
 c->W2C[1][3]=-dot(&c->v,&c->e);
 c->W2C[2][3]=-dot(&c->w,&c->e);
 c->W2C[3][3]=1;

 free(u);
 free(v);
 return(c);
}

/////////////////////////////////////////
// Image I/O section
/////////////////////////////////////////

void read_ply_file(const std::string & filename, std::vector<float>& verts, std::vector<uint32_t>& faces)
{
	// Tinyply can and will throw exceptions at you!
	try
	{
		// Read the file and create a std::istringstream suitable
		// for the lib -- tinyply does not perform any file i/o.
		std::ifstream ss(filename, std::ios::binary);

		// Parse the ASCII header fields
		PlyFile file(ss);

		for (auto e : file.get_elements())
		{
			std::cout << "element - " << e.name << " (" << e.size << ")" << std::endl;
			for (auto p : e.properties)
			{
				std::cout << "\tproperty - " << p.name << " (" << PropertyTable[p.propertyType].str << ")" << std::endl;
			}
		}
		std::cout << std::endl;

		for (auto c : file.comments)
		{
			std::cout << "Comment: " << c << std::endl;
		}

		// Define containers to hold the extracted data. The type must match
		// the property type given in the header. Tinyply will interally allocate the
		// the appropriate amount of memory.
		std::vector<float> norms;
		std::vector<uint8_t> colors;

		std::vector<float> uvCoords;

		uint32_t vertexCount, normalCount, colorCount, faceCount, faceTexcoordCount, faceColorCount;
		vertexCount = normalCount = colorCount = faceCount = faceTexcoordCount = faceColorCount = 0;

		// The count returns the number of instances of the property group. The vectors
		// above will be resized into a multiple of the property group size as
		// they are "flattened"... i.e. verts = {x, y, z, x, y, z, ...}
		vertexCount = file.request_properties_from_element("vertex", { "x", "y", "z" }, verts);
		normalCount = file.request_properties_from_element("vertex", { "nx", "ny", "nz" }, norms);
		colorCount = file.request_properties_from_element("vertex", { "red", "green", "blue", "alpha" }, colors);

		// For properties that are list types, it is possibly to specify the expected count (ideal if a
		// consumer of this library knows the layout of their format a-priori). Otherwise, tinyply
		// defers allocation of memory until the first instance of the property has been found
		// as implemented in file.read(ss)
		faceCount = file.request_properties_from_element("face", { "vertex_indices" }, faces, 3);
		faceTexcoordCount = file.request_properties_from_element("face", { "texcoord" }, uvCoords, 6);

		// Now populate the vectors...
		timepoint before = now();
		file.read(ss);
		timepoint after = now();

		// Good place to put a breakpoint!
		std::cout << "Parsing took " << difference_micros(before, after) << "μs: " << std::endl;
		std::cout << "\tRead " << verts.size() << " total vertices (" << vertexCount << " properties)." << std::endl;
		std::cout << "\tRead " << norms.size() << " total normals (" << normalCount << " properties)." << std::endl;
		std::cout << "\tRead " << colors.size() << " total vertex colors (" << colorCount << " properties)." << std::endl;
		std::cout << "\tRead " << faces.size() << " total faces (triangles) (" << faceCount << " properties)." << std::endl;
		std::cout << "\tRead " << uvCoords.size() << " total texcoords (" << faceTexcoordCount << " properties)." << std::endl;

		/*
		// Fixme - tinyply isn't particularly sensitive to mismatched properties and prefers to crash instead of throw. Use
		// actual data from parsed headers instead of manually defining properties added to a new file like below:

		std::filebuf fb;
		fb.open("converted.ply", std::ios::out | std::ios::binary);
		std::ostream outputStream(&fb);

		PlyFile myFile;

		myFile.add_properties_to_element("vertex", { "x", "y", "z" }, verts);
		myFile.add_properties_to_element("vertex", { "red", "green", "blue" }, colors);
		myFile.add_properties_to_element("face", { "vertex_indices" }, faces, 3, PlyProperty::Type::UINT8);

		myFile.comments.push_back("generated by tinyply");
		myFile.write(outputStream, true);

		fb.close();
		*/
	}

	catch (const std::exception & e)
	{
		std::cerr << "Caught exception: " << e.what() << std::endl;
	}
}

struct image *readPPMimage(const char *filename)
{
 // Reads an image from a .ppm file. A .ppm file is a very simple image representation
 // format with a text header followed by the binary RGB data at 24bits per pixel.
 // The header has the following form:
 //
 // P6
 // # One or more comment lines preceded by '#'
 // 340 200
 // 255
 //
 // The first line 'P6' is the .ppm format identifier, this is followed by one or more
 // lines with comments, typically used to inidicate which program generated the
 // .ppm file.
 // After the comments, a line with two integer values specifies the image resolution
 // as number of pixels in x and number of pixels in y.
 // The final line of the header stores the maximum value for pixels in the image,
 // usually 255.
 // After this last header line, binary data stores the RGB values for each pixel
 // in row-major order. Each pixel requires 3 bytes ordered R, G, and B.
 //
 // NOTE: Windows file handling is rather crotchetty. You may have to change the
 //       way this file is accessed if the images are being corrupted on read
 //       on Windows.
 //
 // readPPMdata converts the image colour information to floating point. This is so that
 // the texture mapping function doesn't have to do the conversion every time
 // it is asked to return the colour at a specific location.
 //

 FILE *f;
 struct image *im;
 char line[1024];
 int sizx,sizy;
 int i;
 unsigned char *tmp;
 double *fRGB;

 im=(struct image *)calloc(1,sizeof(struct image));
 if (im!=NULL)
 {
  im->rgbdata=NULL;
  f=fopen(filename,"rb+");
  if (f==NULL)
  {
   fprintf(stderr,"Unable to open file %s for reading, please check name and path\n",filename);
   free(im);
   return(NULL);
  }
  fgets(&line[0],1000,f);
  if (strcmp(&line[0],"P6\n")!=0)
  {
   fprintf(stderr,"Wrong file format, not a .ppm file or header end-of-line characters missing\n");
   free(im);
   fclose(f);
   return(NULL);
  }
  fprintf(stderr,"%s\n",line);
  // Skip over comments
  fgets(&line[0],511,f);
  while (line[0]=='#')
  {
   fprintf(stderr,"%s",line);
   fgets(&line[0],511,f);
  }
  sscanf(&line[0],"%d %d\n",&sizx,&sizy);           // Read file size
  fprintf(stderr,"nx=%d, ny=%d\n\n",sizx,sizy);
  im->sx=sizx;
  im->sy=sizy;

  fgets(&line[0],9,f);  	                // Read the remaining header line
  fprintf(stderr,"%s\n",line);
  tmp=(unsigned char *)calloc(sizx*sizy*3,sizeof(unsigned char));
  fRGB=(double *)calloc(sizx*sizy*3,sizeof(double));
  if (tmp==NULL||fRGB==NULL)
  {
   fprintf(stderr,"Out of memory allocating space for image\n");
   free(im);
   fclose(f);
   return(NULL);
  }

  fread(tmp,sizx*sizy*3*sizeof(unsigned char),1,f);
  fclose(f);

  // Conversion to floating point
  for (i=0; i<sizx*sizy*3; i++) *(fRGB+i)=((double)*(tmp+i))/255.0;
  free(tmp);
  im->rgbdata=(void *)fRGB;

  return(im);
 }

 fprintf(stderr,"Unable to allocate memory for image structure\n");
 return(NULL);
}

struct image *newImage(int size_x, int size_y)
{
 // Allocates and returns a new image with all zeros. Assumes 24 bit per pixel,
 // unsigned char array.
 struct image *im;

 im=(struct image *)calloc(1,sizeof(struct image));
 if (im!=NULL)
 {
  im->rgbdata=NULL;
  im->sx=size_x;
  im->sy=size_y;
  im->rgbdata=(void *)calloc(size_x*size_y*3,sizeof(unsigned char));
  if (im->rgbdata!=NULL) return(im);
 }
 fprintf(stderr,"Unable to allocate memory for new image\n");
 return(NULL);
}

void imageOutput(struct image *im, const char *filename)
{
 // Writes out a .ppm file from the image data contained in 'im'.
 // Note that Windows typically doesn't know how to open .ppm
 // images. Use Gimp or any other seious image processing
 // software to display .ppm images.
 // Also, note that because of Windows file format management,
 // you may have to modify this file to get image output on
 // Windows machines to work properly.
 //
 // Assumes a 24 bit per pixel image stored as unsigned chars
 //

 FILE *f;

 if (im!=NULL)
  if (im->rgbdata!=NULL)
  {
   f=fopen(filename,"wb+");
   if (f==NULL)
   {
    fprintf(stderr,"Unable to open file %s for output! No image written\n",filename);
    return;
   }
   fprintf(f,"P6\n");
   fprintf(f,"# Output from RayTracer.c\n");
   fprintf(f,"%d %d\n",im->sx,im->sy);
   fprintf(f,"255\n");
   fwrite((unsigned char *)im->rgbdata,im->sx*im->sy*3*sizeof(unsigned char),1,f);
   fclose(f);
   //fclose(f);
   return;
  }
 fprintf(stderr,"imageOutput(): Specified image is empty. Nothing output\n");
}

void deleteImage(struct image *im)
{
 // De-allocates memory reserved for the image stored in 'im'
 if (im!=NULL)
 {
  if (im->rgbdata!=NULL) free(im->rgbdata);
  free(im);
 }
}

void cleanup(struct object3D *o_list, struct pointLS *l_list)
{
 // De-allocates memory reserved for the object list and the point light source
 // list. Note that *YOU* must de-allocate any memory reserved for images
 // rendered by the raytracer.
 struct object3D *p, *q;
 struct pointLS *r, *s;

 p=o_list;		// De-allocate all memory from objects in the list
 while(p!=NULL)
 {
  q=p->next;
  if (p->texImg!=NULL && p->intersect != NULL)
  {
   if (p->texImg->rgbdata!=NULL) free(p->texImg->rgbdata);
   free(p->texImg);
  }
  free(p);
  p=q;
 }

 r=l_list;
 while(r!=NULL)
 {
  s=r->next;
  free(r);
  r=s;
 }
}

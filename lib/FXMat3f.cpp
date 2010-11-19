/********************************************************************************
*                                                                               *
*            S i n g l e - P r e c i s i o n   3 x 3   M a t r i x              *
*                                                                               *
*********************************************************************************
* Copyright (C) 2003,2010 by Jeroen van der Zijp.   All Rights Reserved.        *
*********************************************************************************
* This library is free software; you can redistribute it and/or modify          *
* it under the terms of the GNU Lesser General Public License as published by   *
* the Free Software Foundation; either version 3 of the License, or             *
* (at your option) any later version.                                           *
*                                                                               *
* This library is distributed in the hope that it will be useful,               *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 *
* GNU Lesser General Public License for more details.                           *
*                                                                               *
* You should have received a copy of the GNU Lesser General Public License      *
* along with this program.  If not, see <http://www.gnu.org/licenses/>          *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXHash.h"
#include "FXStream.h"
#include "FXObject.h"
#include "FXVec2f.h"
#include "FXVec3f.h"
#include "FXVec4f.h"
#include "FXQuatf.h"
#include "FXMat2f.h"
#include "FXMat3f.h"
#include "FXMat4f.h"


/*
  Notes:
  - Transformations pre-multiply.
  - Goal is same effect as OpenGL.
*/


using namespace FX;

/*******************************************************************************/

namespace FX {


// Initialize matrix from scalar
FXMat3f::FXMat3f(FXfloat s){
#if defined(FOX_HAS_SSE)
  register __m128 mm=_mm_set1_ps(s);
  _mm_storeu_ps(&m[0][0],mm);
  _mm_storeu_ps(&m[1][1],mm);
  _mm_store_ss (&m[2][2],mm);
#else
  m[0][0]=s; m[0][1]=s; m[0][2]=s;
  m[1][0]=s; m[1][1]=s; m[1][2]=s;
  m[2][0]=s; m[2][1]=s; m[2][2]=s;
#endif
  }


// Initialize with 2x2 rotation and scale matrix
FXMat3f::FXMat3f(const FXMat2f& s){
#if defined(FOX_HAS_SSE)
  register __m128 mm=_mm_loadu_ps(&s[0][0]);
  _mm_storel_pi((__m64*)&m[2][0],_mm_set1_ps(0.0f)); _mm_store_ss(&m[2][2],_mm_set1_ps(1.0f));
  _mm_storel_pi((__m64*)&m[0][0],mm);                _mm_store_ss(&m[0][2],_mm_set1_ps(0.0f));
  _mm_storeh_pi((__m64*)&m[1][0],mm);                _mm_store_ss(&m[1][2],_mm_set1_ps(0.0f));
#else
  m[0][0]=s[0][0]; m[0][1]=s[0][1]; m[0][2]=0.0f;
  m[1][0]=s[1][0]; m[1][1]=s[1][1]; m[1][2]=0.0f;
  m[2][0]=0.0f;    m[2][1]=0.0f;    m[2][2]=1.0f;
#endif
  }


// Initialize matrix from another matrix
FXMat3f::FXMat3f(const FXMat3f& s){
#if defined(FOX_HAS_SSE)
  _mm_storeu_ps(&m[0][0],_mm_loadu_ps(&s[0][0]));
  _mm_storeu_ps(&m[1][1],_mm_loadu_ps(&s[1][1]));
  _mm_store_ss (&m[2][2],_mm_load_ss (&s[2][2]));
#else
  m[0][0]=s[0][0]; m[0][1]=s[0][1]; m[0][2]=s[0][2];
  m[1][0]=s[1][0]; m[1][1]=s[1][1]; m[1][2]=s[1][2];
  m[2][0]=s[2][0]; m[2][1]=s[2][1]; m[2][2]=s[2][2];
#endif
  }


// Initialize from rotation and scaling part of 4x4 matrix
FXMat3f::FXMat3f(const FXMat4f& s){
#if defined(FOX_HAS_SSE)
  register __m128 m0=_mm_loadu_ps(&s[0][0]);
  register __m128 m1=_mm_loadu_ps(&s[1][0]);
  register __m128 m2=_mm_loadu_ps(&s[2][0]);
  _mm_storel_pi((__m64*)&m[0][0],m0); _mm_store_ss(&m[0][2],_mm_movehl_ps(m0,m0));
  _mm_storel_pi((__m64*)&m[1][0],m1); _mm_store_ss(&m[1][2],_mm_movehl_ps(m1,m1));
  _mm_storel_pi((__m64*)&m[2][0],m2); _mm_store_ss(&m[2][2],_mm_movehl_ps(m2,m2));
#else
  m[0][0]=s[0][0]; m[0][1]=s[0][1]; m[0][2]=s[0][2];
  m[1][0]=s[1][0]; m[1][1]=s[1][1]; m[1][2]=s[1][2];
  m[2][0]=s[2][0]; m[2][1]=s[2][1]; m[2][2]=s[2][2];
#endif
  }


// Initialize matrix from array
FXMat3f::FXMat3f(const FXfloat s[]){
#if defined(FOX_HAS_SSE)
  _mm_storeu_ps(&m[0][0],_mm_loadu_ps(s+0));
  _mm_storeu_ps(&m[1][1],_mm_loadu_ps(s+4));
  _mm_store_ss (&m[2][2],_mm_load_ss (s+8));
#else
  m[0][0]=s[0]; m[0][1]=s[1]; m[0][2]=s[2];
  m[1][0]=s[3]; m[1][1]=s[4]; m[1][2]=s[5];
  m[2][0]=s[6]; m[2][1]=s[7]; m[2][2]=s[8];
#endif
  }


// Initialize diagonal matrix
FXMat3f::FXMat3f(FXfloat a,FXfloat b,FXfloat c){
#if defined(FOX_HAS_SSE)
  _mm_storeu_ps(&m[0][0],_mm_set_ps(0.0f,0.0f,0.0f,a));
  _mm_storeu_ps(&m[1][1],_mm_set_ps(0.0f,0.0f,0.0f,b));
  _mm_store_ss (&m[2][2],_mm_set_ss(c));
#else
  m[0][0]=a;    m[0][1]=0.0f; m[0][2]=0.0f;
  m[1][0]=0.0f; m[1][1]=b;    m[1][2]=0.0f;
  m[2][0]=0.0f; m[2][1]=0.0f; m[2][2]=c;
#endif
  }


// Initialize matrix from components
FXMat3f::FXMat3f(FXfloat a00,FXfloat a01,FXfloat a02,FXfloat a10,FXfloat a11,FXfloat a12,FXfloat a20,FXfloat a21,FXfloat a22){
  m[0][0]=a00; m[0][1]=a01; m[0][2]=a02;
  m[1][0]=a10; m[1][1]=a11; m[1][2]=a12;
  m[2][0]=a20; m[2][1]=a21; m[2][2]=a22;
  }


// Initialize matrix from three vectors
FXMat3f::FXMat3f(const FXVec3f& a,const FXVec3f& b,const FXVec3f& c){
  m[0]=a;
  m[1]=b;
  m[2]=c;
  }


// Initialize matrix from quaternion
FXMat3f::FXMat3f(const FXQuatf& quat){
  quat.getAxes(m[0],m[1],m[2]);
  }


// Assign from scalar
FXMat3f& FXMat3f::operator=(FXfloat s){
#if defined(FOX_HAS_SSE)
  register __m128 mm=_mm_set1_ps(s);
  _mm_storeu_ps(&m[0][0],mm);
  _mm_storeu_ps(&m[1][1],mm);
  _mm_store_ss (&m[2][2],mm);
#else
  m[0][0]=s; m[0][1]=s; m[0][2]=s;
  m[1][0]=s; m[1][1]=s; m[1][2]=s;
  m[2][0]=s; m[2][1]=s; m[2][2]=s;
#endif
  return *this;
  }


// Assign from 2x2 rotation and scale matrix
FXMat3f& FXMat3f::operator=(const FXMat2f& s){
#if defined(FOX_HAS_SSE)
  register __m128 mm=_mm_loadu_ps(&s[0][0]);
  _mm_storel_pi((__m64*)&m[2][0],_mm_set1_ps(0.0f)); _mm_store_ss(&m[2][2],_mm_set1_ps(1.0f));
  _mm_storel_pi((__m64*)&m[0][0],mm);                _mm_store_ss(&m[0][2],_mm_set1_ps(0.0f));
  _mm_storeh_pi((__m64*)&m[1][0],mm);                _mm_store_ss(&m[1][2],_mm_set1_ps(0.0f));
#else
  m[0][0]=s[0][0]; m[0][1]=s[0][1]; m[0][2]=0.0f;
  m[1][0]=s[1][0]; m[1][1]=s[1][1]; m[1][2]=0.0f;
  m[2][0]=0.0f;    m[2][1]=0.0f;    m[2][2]=1.0f;
#endif
  return *this;
  }


// Assignment operator
FXMat3f& FXMat3f::operator=(const FXMat3f& s){
#if defined(FOX_HAS_SSE)
  _mm_storeu_ps(&m[0][0],_mm_loadu_ps(&s[0][0]));
  _mm_storeu_ps(&m[1][1],_mm_loadu_ps(&s[1][1]));
  _mm_store_ss (&m[2][2],_mm_load_ss (&s[2][2]));
#else
  m[0][0]=s[0][0]; m[0][1]=s[0][1]; m[0][2]=s[0][2];
  m[1][0]=s[1][0]; m[1][1]=s[1][1]; m[1][2]=s[1][2];
  m[2][0]=s[2][0]; m[2][1]=s[2][1]; m[2][2]=s[2][2];
#endif
  return *this;
  }


// Assign from rotation and scaling part of 4x4 matrix
FXMat3f& FXMat3f::operator=(const FXMat4f& s){
#if defined(FOX_HAS_SSE)
  register __m128 m0=_mm_loadu_ps(&s[0][0]);
  register __m128 m1=_mm_loadu_ps(&s[1][0]);
  register __m128 m2=_mm_loadu_ps(&s[2][0]);
  _mm_storel_pi((__m64*)&m[0][0],m0); _mm_store_ss(&m[0][2],_mm_movehl_ps(m0,m0));
  _mm_storel_pi((__m64*)&m[1][0],m1); _mm_store_ss(&m[1][2],_mm_movehl_ps(m1,m1));
  _mm_storel_pi((__m64*)&m[2][0],m2); _mm_store_ss(&m[2][2],_mm_movehl_ps(m2,m2));
#else
  m[0][0]=s[0][0]; m[0][1]=s[0][1]; m[0][2]=s[0][2];
  m[1][0]=s[1][0]; m[1][1]=s[1][1]; m[1][2]=s[1][2];
  m[2][0]=s[2][0]; m[2][1]=s[2][1]; m[2][2]=s[2][2];
#endif
  return *this;
  }


// Assignment from array
FXMat3f& FXMat3f::operator=(const FXfloat s[]){
#if defined(FOX_HAS_SSE)
  _mm_storeu_ps(&m[0][0],_mm_loadu_ps(s+0));
  _mm_storeu_ps(&m[1][1],_mm_loadu_ps(s+4));
  _mm_store_ss (&m[2][2],_mm_load_ss (s+8));
#else
  m[0][0]=s[0]; m[0][1]=s[1]; m[0][2]=s[2];
  m[1][0]=s[3]; m[1][1]=s[4]; m[1][2]=s[5];
  m[2][0]=s[6]; m[2][1]=s[7]; m[2][2]=s[8];
#endif
  return *this;
  }


// Set value from scalar
FXMat3f& FXMat3f::set(FXfloat s){
#if defined(FOX_HAS_SSE)
  register __m128 mm=_mm_set1_ps(s);
  _mm_storeu_ps(&m[0][0],mm);
  _mm_storeu_ps(&m[1][1],mm);
  _mm_store_ss (&m[2][2],mm);
#else
  m[0][0]=s; m[0][1]=s; m[0][2]=s;
  m[1][0]=s; m[1][1]=s; m[1][2]=s;
  m[2][0]=s; m[2][1]=s; m[2][2]=s;
#endif
  return *this;
  }


// Set value from 2x2 rotation and scale matrix
FXMat3f& FXMat3f::set(const FXMat2f& s){
#if defined(FOX_HAS_SSE)
  register __m128 mm=_mm_loadu_ps(&s[0][0]);
  _mm_storel_pi((__m64*)&m[2][0],_mm_set1_ps(0.0f)); _mm_store_ss(&m[2][2],_mm_set1_ps(1.0f));
  _mm_storel_pi((__m64*)&m[0][0],mm);                _mm_store_ss(&m[0][2],_mm_set1_ps(0.0f));
  _mm_storeh_pi((__m64*)&m[1][0],mm);                _mm_store_ss(&m[1][2],_mm_set1_ps(0.0f));
#else
  m[0][0]=s[0][0]; m[0][1]=s[0][1]; m[0][2]=0.0f;
  m[1][0]=s[1][0]; m[1][1]=s[1][1]; m[1][2]=0.0f;
  m[2][0]=0.0f;    m[2][1]=0.0f;    m[2][2]=1.0f;
#endif
  return *this;
  }


// Set value from another matrix
FXMat3f& FXMat3f::set(const FXMat3f& s){
#if defined(FOX_HAS_SSE)
  _mm_storeu_ps(&m[0][0],_mm_loadu_ps(&s[0][0]));
  _mm_storeu_ps(&m[1][1],_mm_loadu_ps(&s[1][1]));
  _mm_store_ss (&m[2][2],_mm_load_ss (&s[2][2]));
#else
  m[0][0]=s[0][0]; m[0][1]=s[0][1]; m[0][2]=s[0][2];
  m[1][0]=s[1][0]; m[1][1]=s[1][1]; m[1][2]=s[1][2];
  m[2][0]=s[2][0]; m[2][1]=s[2][1]; m[2][2]=s[2][2];
#endif
  return *this;
  }


// Set from rotation and scaling part of 4x4 matrix
FXMat3f& FXMat3f::set(const FXMat4f& s){
#if defined(FOX_HAS_SSE)
  register __m128 m0=_mm_loadu_ps(&s[0][0]);
  register __m128 m1=_mm_loadu_ps(&s[1][0]);
  register __m128 m2=_mm_loadu_ps(&s[2][0]);
  _mm_storel_pi((__m64*)&m[0][0],m0); _mm_store_ss(&m[0][2],_mm_movehl_ps(m0,m0));
  _mm_storel_pi((__m64*)&m[1][0],m1); _mm_store_ss(&m[1][2],_mm_movehl_ps(m1,m1));
  _mm_storel_pi((__m64*)&m[2][0],m2); _mm_store_ss(&m[2][2],_mm_movehl_ps(m2,m2));
#else
  m[0][0]=s[0][0]; m[0][1]=s[0][1]; m[0][2]=s[0][2];
  m[1][0]=s[1][0]; m[1][1]=s[1][1]; m[1][2]=s[1][2];
  m[2][0]=s[2][0]; m[2][1]=s[2][1]; m[2][2]=s[2][2];
#endif
  return *this;
  }


// Set value from array
FXMat3f& FXMat3f::set(const FXfloat s[]){
#if defined(FOX_HAS_SSE)
  _mm_storeu_ps(&m[0][0],_mm_loadu_ps(s+0));
  _mm_storeu_ps(&m[1][1],_mm_loadu_ps(s+4));
  _mm_store_ss (&m[2][2],_mm_load_ss (s+8));
#else
  m[0][0]=s[0]; m[0][1]=s[1]; m[0][2]=s[2];
  m[1][0]=s[3]; m[1][1]=s[4]; m[1][2]=s[5];
  m[2][0]=s[6]; m[2][1]=s[7]; m[2][2]=s[8];
#endif
  return *this;
  }


// Set diagonal matrix
FXMat3f& FXMat3f::set(FXfloat a,FXfloat b,FXfloat c){
#if defined(FOX_HAS_SSE)
  _mm_storeu_ps(&m[0][0],_mm_set_ps(0.0f,0.0f,0.0f,a));
  _mm_storeu_ps(&m[1][1],_mm_set_ps(0.0f,0.0f,0.0f,b));
  _mm_store_ss (&m[2][2],_mm_set_ss(c));
#else
  m[0][0]=a;    m[0][1]=0.0f; m[0][2]=0.0f;
  m[1][0]=0.0f; m[1][1]=b;    m[1][2]=0.0f;
  m[2][0]=0.0f; m[2][1]=0.0f; m[2][2]=c;
#endif
  return *this;
  }


// Set value from components
FXMat3f& FXMat3f::set(FXfloat a00,FXfloat a01,FXfloat a02,FXfloat a10,FXfloat a11,FXfloat a12,FXfloat a20,FXfloat a21,FXfloat a22){
  m[0][0]=a00; m[0][1]=a01; m[0][2]=a02;
  m[1][0]=a10; m[1][1]=a11; m[1][2]=a12;
  m[2][0]=a20; m[2][1]=a21; m[2][2]=a22;
  return *this;
  }


// Set value from three vectors
FXMat3f& FXMat3f::set(const FXVec3f& a,const FXVec3f& b,const FXVec3f& c){
  m[0]=a;
  m[1]=b;
  m[2]=c;
  return *this;
  }


// Set value from quaternion
FXMat3f& FXMat3f::set(const FXQuatf& quat){
  quat.getAxes(m[0],m[1],m[2]);
  return *this;
  }


// Add matrices
FXMat3f& FXMat3f::operator+=(const FXMat3f& w){
#if defined(FOX_HAS_SSE)
  _mm_storeu_ps(&m[0][0],_mm_add_ps(_mm_loadu_ps(&m[0][0]),_mm_loadu_ps(&w[0][0])));
  _mm_storeu_ps(&m[1][1],_mm_add_ps(_mm_loadu_ps(&m[1][1]),_mm_loadu_ps(&w[1][1])));
  _mm_store_ss (&m[2][2],_mm_add_ss(_mm_load_ss (&m[2][2]),_mm_load_ss (&w[2][2])));
#else
  m[0][0]+=w[0][0]; m[0][1]+=w[0][1]; m[0][2]+=w[0][2];
  m[1][0]+=w[1][0]; m[1][1]+=w[1][1]; m[1][2]+=w[1][2];
  m[2][0]+=w[2][0]; m[2][1]+=w[2][1]; m[2][2]+=w[2][2];
#endif
  return *this;
  }


// Substract matrices
FXMat3f& FXMat3f::operator-=(const FXMat3f& w){
#if defined(FOX_HAS_SSE)
  _mm_storeu_ps(&m[0][0],_mm_sub_ps(_mm_loadu_ps(&m[0][0]),_mm_loadu_ps(&w[0][0])));
  _mm_storeu_ps(&m[1][1],_mm_sub_ps(_mm_loadu_ps(&m[1][1]),_mm_loadu_ps(&w[1][1])));
  _mm_store_ss (&m[2][2],_mm_sub_ss(_mm_load_ss (&m[2][2]),_mm_load_ss (&w[2][2])));
#else
  m[0][0]-=w[0][0]; m[0][1]-=w[0][1]; m[0][2]-=w[0][2];
  m[1][0]-=w[1][0]; m[1][1]-=w[1][1]; m[1][2]-=w[1][2];
  m[2][0]-=w[2][0]; m[2][1]-=w[2][1]; m[2][2]-=w[2][2];
#endif
  return *this;
  }


// Multiply matrix by scalar
FXMat3f& FXMat3f::operator*=(FXfloat w){
#if defined(FOX_HAS_SSE)
  register __m128 ww=_mm_set1_ps(w);
  _mm_storeu_ps(&m[0][0],_mm_mul_ps(_mm_loadu_ps(&m[0][0]),ww));
  _mm_storeu_ps(&m[1][1],_mm_mul_ps(_mm_loadu_ps(&m[1][1]),ww));
  _mm_store_ss (&m[2][2],_mm_mul_ss(_mm_load_ss (&m[2][2]),ww));
#else
  m[0][0]*=w; m[0][1]*=w; m[0][2]*=w;
  m[1][0]*=w; m[1][1]*=w; m[1][2]*=w;
  m[2][0]*=w; m[2][1]*=w; m[2][2]*=w;
#endif
  return *this;
  }


// Multiply matrix by matrix
FXMat3f& FXMat3f::operator*=(const FXMat3f& s){
#if defined(FOX_HAS_SSE)
  register __m128 b0=_mm_loadh_pi(_mm_load_ss(&s[0][2]),(const __m64*)&s[0][0]);
  register __m128 b1=_mm_loadh_pi(_mm_load_ss(&s[1][2]),(const __m64*)&s[1][0]);
  register __m128 b2=_mm_loadh_pi(_mm_load_ss(&s[2][2]),(const __m64*)&s[2][0]);
  register __m128 r0,r1,r2;
  register __m128 xx,yy,zz;
  xx=_mm_set1_ps(m[0][0]);
  yy=_mm_set1_ps(m[0][1]);
  zz=_mm_set1_ps(m[0][2]);
  r0=_mm_add_ps(_mm_add_ps(_mm_mul_ps(b0,xx),_mm_mul_ps(b1,yy)),_mm_mul_ps(b2,zz));
  xx=_mm_set1_ps(m[1][0]);
  yy=_mm_set1_ps(m[1][1]);
  zz=_mm_set1_ps(m[1][2]);
  r1=_mm_add_ps(_mm_add_ps(_mm_mul_ps(b0,xx),_mm_mul_ps(b1,yy)),_mm_mul_ps(b2,zz));
  xx=_mm_set1_ps(m[2][0]);
  yy=_mm_set1_ps(m[2][1]);
  zz=_mm_set1_ps(m[2][2]);
  r2=_mm_add_ps(_mm_add_ps(_mm_mul_ps(b0,xx),_mm_mul_ps(b1,yy)),_mm_mul_ps(b2,zz));
  _mm_storeh_pi((__m64*)&m[0][0],r0); _mm_store_ss(&m[0][2],r0);
  _mm_storeh_pi((__m64*)&m[1][0],r1); _mm_store_ss(&m[1][2],r1);
  _mm_storeh_pi((__m64*)&m[2][0],r2); _mm_store_ss(&m[2][2],r2);
  return *this;
#else
  register FXfloat x,y,z;
  x=m[0][0]; y=m[0][1]; z=m[0][2];
  m[0][0]=x*s[0][0]+y*s[1][0]+z*s[2][0];
  m[0][1]=x*s[0][1]+y*s[1][1]+z*s[2][1];
  m[0][2]=x*s[0][2]+y*s[1][2]+z*s[2][2];
  x=m[1][0]; y=m[1][1]; z=m[1][2];
  m[1][0]=x*s[0][0]+y*s[1][0]+z*s[2][0];
  m[1][1]=x*s[0][1]+y*s[1][1]+z*s[2][1];
  m[1][2]=x*s[0][2]+y*s[1][2]+z*s[2][2];
  x=m[2][0]; y=m[2][1]; z=m[2][2];
  m[2][0]=x*s[0][0]+y*s[1][0]+z*s[2][0];
  m[2][1]=x*s[0][1]+y*s[1][1]+z*s[2][1];
  m[2][2]=x*s[0][2]+y*s[1][2]+z*s[2][2];
  return *this;
#endif
  }


// Divide matrix by scalar
FXMat3f& FXMat3f::operator/=(FXfloat w){
#if defined(FOX_HAS_SSE)
  register __m128 ww=_mm_set1_ps(w);
  _mm_storeu_ps(&m[0][0],_mm_div_ps(_mm_loadu_ps(&m[0][0]),ww));
  _mm_storeu_ps(&m[1][1],_mm_div_ps(_mm_loadu_ps(&m[1][1]),ww));
  _mm_store_ss (&m[2][2],_mm_div_ss(_mm_load_ss (&m[2][2]),ww));
#else
  m[0][0]/=w; m[0][1]/=w; m[0][2]/=w;
  m[1][0]/=w; m[1][1]/=w; m[1][2]/=w;
  m[2][0]/=w; m[2][1]/=w; m[2][2]/=w;
#endif
  return *this;
  }


// Negate matrix
FXMat3f FXMat3f::operator-() const {
#if defined(FOX_HAS_SSE)
  FXMat3f r;
  register __m128 zz=_mm_set1_ps(0.0f);
  _mm_storeu_ps(&r[0][0],_mm_sub_ps(zz,_mm_loadu_ps(&m[0][0])));
  _mm_storeu_ps(&r[1][1],_mm_sub_ps(zz,_mm_loadu_ps(&m[1][1])));
  _mm_store_ss (&r[2][2],_mm_sub_ss(zz,_mm_load_ss (&m[2][2])));
  return r;
#else
  return FXMat3f(-m[0][0],-m[0][1],-m[0][2],
                 -m[1][0],-m[1][1],-m[1][2],
                 -m[2][0],-m[2][1],-m[2][2]);
#endif
  }


// Set to identity matrix
FXMat3f& FXMat3f::identity(){
#if defined(FOX_HAS_SSE)
  _mm_storeu_ps(&m[0][0],_mm_set_ps(0.0f,0.0f,0.0f,1.0f));
  _mm_storeu_ps(&m[1][1],_mm_set_ps(0.0f,0.0f,0.0f,1.0f));
  _mm_store_ss (&m[2][2],_mm_set_ps(0.0f,0.0f,0.0f,1.0f));
#else
  m[0][0]=1.0f; m[0][1]=0.0f; m[0][2]=0.0f;
  m[1][0]=0.0f; m[1][1]=1.0f; m[1][2]=0.0f;
  m[2][0]=0.0f; m[2][1]=0.0f; m[2][2]=1.0f;
#endif
  return *this;
  }


// Return true if identity matrix
FXbool FXMat3f::isIdentity() const {
  return m[0][0]==1.0f && m[0][1]==0.0f && m[0][2]==0.0f &&
         m[1][0]==0.0f && m[1][1]==1.0f && m[1][2]==0.0f &&
         m[2][0]==0.0f && m[2][1]==0.0f && m[2][2]==1.0f;
  }


// Rotate by cosine, sine
FXMat3f& FXMat3f::rot(FXfloat c,FXfloat s){
  register FXfloat u,v;
  u=m[0][0]; v=m[1][0]; m[0][0]=c*u+s*v; m[1][0]=c*v-s*u;
  u=m[0][1]; v=m[1][1]; m[0][1]=c*u+s*v; m[1][1]=c*v-s*u;
  u=m[0][2]; v=m[1][2]; m[0][2]=c*u+s*v; m[1][2]=c*v-s*u;
  return *this;
  }


// Rotate by angle
FXMat3f& FXMat3f::rot(FXfloat phi){
  return rot(cosf(phi),sinf(phi));
  }


// Translate
FXMat3f& FXMat3f::trans(FXfloat tx,FXfloat ty){
  m[2][0]=m[2][0]+tx*m[0][0]+ty*m[1][0];
  m[2][1]=m[2][1]+tx*m[0][1]+ty*m[1][1];
  m[2][2]=m[2][2]+tx*m[0][2]+ty*m[1][2];
  return *this;
  }


// Scale unqual
FXMat3f& FXMat3f::scale(FXfloat sx,FXfloat sy){
  m[0][0]*=sx; m[0][1]*=sx; m[0][2]*=sx;
  m[1][0]*=sy; m[1][1]*=sy; m[1][2]*=sy;
  return *this;
  }


// Scale uniform
FXMat3f& FXMat3f::scale(FXfloat s){
  return scale(s,s);
  }


// Calculate determinant
FXfloat FXMat3f::det() const {
  return m[0][0]*(m[1][1]*m[2][2]-m[2][1]*m[1][2])+
         m[0][1]*(m[2][0]*m[1][2]-m[1][0]*m[2][2])+
         m[0][2]*(m[1][0]*m[2][1]-m[2][0]*m[1][1]);
  }


// Transpose matrix
FXMat3f FXMat3f::transpose() const {
  return FXMat3f(m[0][0],m[1][0],m[2][0],
                 m[0][1],m[1][1],m[2][1],
                 m[0][2],m[1][2],m[2][2]);
  }


// Invert matrix
FXMat3f FXMat3f::invert() const {
  register FXfloat dd;
  FXMat3f res;
  res[0][0]=m[1][1]*m[2][2]-m[1][2]*m[2][1];
  res[0][1]=m[0][2]*m[2][1]-m[0][1]*m[2][2];
  res[0][2]=m[0][1]*m[1][2]-m[0][2]*m[1][1];
  res[1][0]=m[1][2]*m[2][0]-m[1][0]*m[2][2];
  res[1][1]=m[0][0]*m[2][2]-m[0][2]*m[2][0];
  res[1][2]=m[0][2]*m[1][0]-m[0][0]*m[1][2];
  res[2][0]=m[1][0]*m[2][1]-m[1][1]*m[2][0];
  res[2][1]=m[0][1]*m[2][0]-m[0][0]*m[2][1];
  res[2][2]=m[0][0]*m[1][1]-m[0][1]*m[1][0];
  dd=m[0][0]*res[0][0]+m[0][1]*res[1][0]+m[0][2]*res[2][0];
  FXASSERT(dd!=0.0f);
  dd=1.0f/dd;
  res[0][0]*=dd;
  res[0][1]*=dd;
  res[0][2]*=dd;
  res[1][0]*=dd;
  res[1][1]*=dd;
  res[1][2]*=dd;
  res[2][0]*=dd;
  res[2][1]*=dd;
  res[2][2]*=dd;
  return res;
  }


// Matrix times vector
FXVec2f operator*(const FXMat3f& m,const FXVec2f& v){
  return FXVec2f(m[0][0]*v[0]+m[0][1]*v[1]+m[0][2], m[1][0]*v[0]+m[1][1]*v[1]+m[1][2]);
  }


// Matrix times vector
FXVec3f operator*(const FXMat3f& m,const FXVec3f& v){
#if defined(FOX_HAS_SSE)
  register __m128 vv=_mm_loadh_pi(_mm_load_ss(&v[2]),(const __m64*)&v[0]);                      // v1       v0       0   v2
  register __m128 r0=_mm_mul_ps(_mm_loadh_pi(_mm_load_ss(&m[0][2]),(const __m64*)&m[0][0]),vv); // m01*v1   m00*v0   0   m02*v2
  register __m128 r1=_mm_mul_ps(_mm_loadh_pi(_mm_load_ss(&m[1][2]),(const __m64*)&m[1][0]),vv); // m11*v1   m10*v0   0   m12*v2
  register __m128 r2=_mm_mul_ps(_mm_loadh_pi(_mm_load_ss(&m[2][2]),(const __m64*)&m[2][0]),vv); // m21*v1   m20*v0   0   m22*v2
  FXVec3f r;
  r0=_mm_hadd_ps(r0,r1);        // m11*v1+m10*v0   m12*v2   m01*v1+m00*v0   m02*v2
  r2=_mm_hadd_ps(r2,r2);        // m21*v1+m20*v0   m22*v2   m21*v1+m20*v0   m22*v2
  r2=_mm_hadd_ps(r2,r0);        // m11*v1+m10*v0+m12*v2   m01*v1+m00*v0+m02*v2   m21*v1+m20*v0+m22*v2   m21*v1+m20*v0+m22*v2
  _mm_storeh_pi((__m64*)&r[0],r2); 
  _mm_store_ss(&r[2],r2);
  return r;
#else
  return FXVec3f(m[0][0]*v[0]+m[0][1]*v[1]+m[0][2]*v[2], m[1][0]*v[0]+m[1][1]*v[1]+m[1][2]*v[2], m[2][0]*v[0]+m[2][1]*v[1]+m[2][2]*v[2]);
#endif
  }


// Vector times matrix
FXVec2f operator*(const FXVec2f& v,const FXMat3f& m){
  return FXVec2f(v[0]*m[0][0]+v[1]*m[1][0]+m[2][0],v[0]*m[0][1]+v[1]*m[1][1]+m[2][1]);
  }


// Vector times matrix
FXVec3f operator*(const FXVec3f& v,const FXMat3f& m){
#if defined(FOX_HAS_SSE)
  register __m128 v0=_mm_set1_ps(v[0]);
  register __m128 v1=_mm_set1_ps(v[1]);
  register __m128 v2=_mm_set1_ps(v[2]);
  register __m128 m0=_mm_loadh_pi(_mm_load_ss(&m[0][2]),(const __m64*)&m[0][0]);
  register __m128 m1=_mm_loadh_pi(_mm_load_ss(&m[1][2]),(const __m64*)&m[1][0]);
  register __m128 m2=_mm_loadh_pi(_mm_load_ss(&m[2][2]),(const __m64*)&m[2][0]);
  register __m128 rr=_mm_add_ps(_mm_add_ps(_mm_mul_ps(m0,v0),_mm_mul_ps(m1,v1)),_mm_mul_ps(m2,v2));
  FXVec3f r;
  _mm_storeh_pi((__m64*)&r[0],rr); 
  _mm_store_ss(&r[2],rr);
  return r;
#else
  return FXVec3f(v[0]*m[0][0]+v[1]*m[1][0]+v[2]*m[2][0], v[0]*m[0][1]+v[1]*m[1][1]+v[2]*m[2][1], v[0]*m[0][2]+v[1]*m[1][2]+v[2]*m[2][2]);
#endif
  }


// Matrix and matrix add
FXMat3f operator+(const FXMat3f& a,const FXMat3f& b){
#if defined(FOX_HAS_SSE)
  FXMat3f r;
  _mm_storeu_ps(&r[0][0],_mm_add_ps(_mm_loadu_ps(&a[0][0]),_mm_loadu_ps(&b[0][0])));
  _mm_storeu_ps(&r[1][1],_mm_add_ps(_mm_loadu_ps(&a[1][1]),_mm_loadu_ps(&b[1][1])));
  _mm_store_ss (&r[2][2],_mm_add_ss(_mm_load_ss (&a[2][2]),_mm_load_ss (&b[2][2])));
  return r;
#else
  return FXMat3f(a[0][0]+b[0][0],a[0][1]+b[0][1],a[0][2]+b[0][2],
                 a[1][0]+b[1][0],a[1][1]+b[1][1],a[1][2]+b[1][2],
                 a[2][0]+b[2][0],a[2][1]+b[2][1],a[2][2]+b[2][2]);
#endif
  }


// Matrix and matrix subtract
FXMat3f operator-(const FXMat3f& a,const FXMat3f& b){
#if defined(FOX_HAS_SSE)
  FXMat3f r;
  _mm_storeu_ps(&r[0][0],_mm_sub_ps(_mm_loadu_ps(&a[0][0]),_mm_loadu_ps(&b[0][0])));
  _mm_storeu_ps(&r[1][1],_mm_sub_ps(_mm_loadu_ps(&a[1][1]),_mm_loadu_ps(&b[1][1])));
  _mm_store_ss (&r[2][2],_mm_sub_ss(_mm_load_ss (&a[2][2]),_mm_load_ss (&b[2][2])));
  return r;
#else
  return FXMat3f(a[0][0]-b[0][0],a[0][1]-b[0][1],a[0][2]-b[0][2],
                 a[1][0]-b[1][0],a[1][1]-b[1][1],a[1][2]-b[1][2],
                 a[2][0]-b[2][0],a[2][1]-b[2][1],a[2][2]-b[2][2]);
#endif
  }


// Matrix and matrix multiply
FXMat3f operator*(const FXMat3f& a,const FXMat3f& b){
#if defined(FOX_HAS_SSE)
  register __m128 b0=_mm_loadh_pi(_mm_load_ss(&b[0][2]),(const __m64*)&b[0][0]);
  register __m128 b1=_mm_loadh_pi(_mm_load_ss(&b[1][2]),(const __m64*)&b[1][0]);
  register __m128 b2=_mm_loadh_pi(_mm_load_ss(&b[2][2]),(const __m64*)&b[2][0]);
  register __m128 r0,r1,r2;
  register __m128 xx,yy,zz;
  FXMat3f r;  
  xx=_mm_set1_ps(a[0][0]);
  yy=_mm_set1_ps(a[0][1]);
  zz=_mm_set1_ps(a[0][2]);
  r0=_mm_add_ps(_mm_add_ps(_mm_mul_ps(b0,xx),_mm_mul_ps(b1,yy)),_mm_mul_ps(b2,zz));
  xx=_mm_set1_ps(a[1][0]);
  yy=_mm_set1_ps(a[1][1]);
  zz=_mm_set1_ps(a[1][2]);
  r1=_mm_add_ps(_mm_add_ps(_mm_mul_ps(b0,xx),_mm_mul_ps(b1,yy)),_mm_mul_ps(b2,zz));
  xx=_mm_set1_ps(a[2][0]);
  yy=_mm_set1_ps(a[2][1]);
  zz=_mm_set1_ps(a[2][2]);
  r2=_mm_add_ps(_mm_add_ps(_mm_mul_ps(b0,xx),_mm_mul_ps(b1,yy)),_mm_mul_ps(b2,zz));
  _mm_storeh_pi((__m64*)&r[0][0],r0); _mm_store_ss(&r[0][2],r0);
  _mm_storeh_pi((__m64*)&r[1][0],r1); _mm_store_ss(&r[1][2],r1);
  _mm_storeh_pi((__m64*)&r[2][0],r2); _mm_store_ss(&r[2][2],r2);
  return r;
#else
  register FXfloat x,y,z;
  FXMat3f r;
  x=a[0][0]; y=a[0][1]; z=a[0][2];
  r[0][0]=x*b[0][0]+y*b[1][0]+z*b[2][0];
  r[0][1]=x*b[0][1]+y*b[1][1]+z*b[2][1];
  r[0][2]=x*b[0][2]+y*b[1][2]+z*b[2][2];
  x=a[1][0]; y=a[1][1]; z=a[1][2];
  r[1][0]=x*b[0][0]+y*b[1][0]+z*b[2][0];
  r[1][1]=x*b[0][1]+y*b[1][1]+z*b[2][1];
  r[1][2]=x*b[0][2]+y*b[1][2]+z*b[2][2];
  x=a[2][0]; y=a[2][1]; z=a[2][2];
  r[2][0]=x*b[0][0]+y*b[1][0]+z*b[2][0];
  r[2][1]=x*b[0][1]+y*b[1][1]+z*b[2][1];
  r[2][2]=x*b[0][2]+y*b[1][2]+z*b[2][2];
  return r;
#endif
  }


// Multiply scalar by matrix
FXMat3f operator*(FXfloat x,const FXMat3f& m){
#if defined(FOX_HAS_SSE)
  FXMat3f r;
  register __m128 xx=_mm_set1_ps(x);
  _mm_storeu_ps(&r[0][0],_mm_mul_ps(xx,_mm_loadu_ps(&m[0][0])));
  _mm_storeu_ps(&r[1][1],_mm_mul_ps(xx,_mm_loadu_ps(&m[0][0])));
  _mm_store_ss (&r[2][2],_mm_mul_ss(xx,_mm_load_ss (&m[2][2])));
  return r;
#else
  return FXMat3f(x*m[0][0],x*m[0][1],x*m[0][2],
                 x*m[1][0],x*m[1][1],x*m[1][2],
                 x*m[2][0],x*m[2][1],x*m[2][2]);
#endif
  }


// Multiply matrix by scalar
FXMat3f operator*(const FXMat3f& m,FXfloat x){
#if defined(FOX_HAS_SSE)
  FXMat3f r;
  register __m128 xx=_mm_set1_ps(x);
  _mm_storeu_ps(&r[0][0],_mm_mul_ps(_mm_loadu_ps(&m[0][0]),xx));
  _mm_storeu_ps(&r[1][1],_mm_mul_ps(_mm_loadu_ps(&m[0][0]),xx));
  _mm_store_ss (&r[2][2],_mm_mul_ss(_mm_load_ss (&m[2][2]),xx));
  return r;
#else
  return FXMat3f(m[0][0]*x,m[0][1]*x,m[0][2]*x,
                 m[1][0]*x,m[1][1]*x,m[1][2]*x,
                 m[2][0]*x,m[2][1]*x,m[2][2]*x);
#endif
  }


// Divide scalar by matrix
FXMat3f operator/(FXfloat x,const FXMat3f& m){
#if defined(FOX_HAS_SSE)
  FXMat3f r;
  register __m128 xx=_mm_set1_ps(x);
  _mm_storeu_ps(&r[0][0],_mm_div_ps(xx,_mm_loadu_ps(&m[0][0])));
  _mm_storeu_ps(&r[1][1],_mm_div_ps(xx,_mm_loadu_ps(&m[0][0])));
  _mm_store_ss (&r[2][2],_mm_div_ss(xx,_mm_load_ss (&m[2][2])));
  return r;
#else
  return FXMat3f(x/m[0][0],x/m[0][1],x/m[0][2],
                 x/m[1][0],x/m[1][1],x/m[1][2],
                 x/m[2][0],x/m[2][1],x/m[2][2]);
#endif
  }


// Divide matrix by scalar
FXMat3f operator/(const FXMat3f& m,FXfloat x){
#if defined(FOX_HAS_SSE)
  FXMat3f r;
  register __m128 xx=_mm_set1_ps(x);
  _mm_storeu_ps(&r[0][0],_mm_div_ps(_mm_loadu_ps(&m[0][0]),xx));
  _mm_storeu_ps(&r[1][1],_mm_div_ps(_mm_loadu_ps(&m[0][0]),xx));
  _mm_store_ss (&r[2][2],_mm_div_ss(_mm_load_ss (&m[2][2]),xx));
  return r;
#else
  return FXMat3f(m[0][0]/x,m[0][1]/x,m[0][2]/x,
                 m[1][0]/x,m[1][1]/x,m[1][2]/x,
                 m[2][0]/x,m[2][1]/x,m[2][2]/x);
#endif
  }


// Matrix and matrix equality
FXbool operator==(const FXMat3f& a,const FXMat3f& b){
  return a[0]==b[0] && a[1]==b[1] && a[2]==b[2];
  }


// Matrix and matrix inequality
FXbool operator!=(const FXMat3f& a,const FXMat3f& b){
  return a[0]!=b[0] || a[1]!=b[1] || a[2]!=b[2];
  }


// Matrix and scalar equality
FXbool operator==(const FXMat3f& a,FXfloat n){
  return a[0]==n && a[1]==n && a[2]==n;
  }


// Matrix and scalar inequality
FXbool operator!=(const FXMat3f& a,FXfloat n){
  return a[0]!=n || a[1]!=n || a[2]!=n;
  }


// Scalar and matrix equality
FXbool operator==(FXfloat n,const FXMat3f& a){
  return n==a[0] && n==a[1] && n==a[2];
  }


// Scalar and matrix inequality
FXbool operator!=(FXfloat n,const FXMat3f& a){
  return n!=a[0] || n!=a[1] || n!=a[2];
  }


// Save to archive
FXStream& operator<<(FXStream& store,const FXMat3f& m){
  store << m[0] << m[1] << m[2];
  return store;
  }


// Load from archive
FXStream& operator>>(FXStream& store,FXMat3f& m){
  store >> m[0] >> m[1] >> m[2];
  return store;
  }

}

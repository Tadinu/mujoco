// Copyright 2021 DeepMind Technologies Limited
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef MUJOCO_SRC_USER_USER_UTIL_H_
#define MUJOCO_SRC_USER_USER_UTIL_H_

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <mujoco/mjexport.h>

const double mjEPS = 1E-14;     // minimum value in various calculations
const double mjMINMASS = 1E-6;  // minimum mass allowed

// check if numeric variable is defined:  !_isnan(num)
MJAPI bool mjuu_defined(double num);

// compute linear address of M[g1][g2] where M is triangular n-by-n
// return -1 if inputs are invalid
MJAPI int mjuu_matadr(int g1, int g2, int n);

// set 4D vector
MJAPI void mjuu_setvec(double* dest, double x, double y, double z, double w);
MJAPI void mjuu_setvec(float* dest, double x, double y, double z, double w);

// set 3D vector
MJAPI void mjuu_setvec(double* dest, double x, double y, double z);
MJAPI void mjuu_setvec(float* dest, double x, double y, double z);

// set 2D vector
MJAPI void mjuu_setvec(double* dest, double x, double y);

// copy real-valued vector
template <class T1, class T2>
MJAPI void mjuu_copyvec(T1* dest, const T2* src, int n) {
  for (int i=0; i<n; i++) {
    dest[i] = (T1)src[i];
  }
}

// add to double array
MJAPI void mjuu_addtovec(double* dest, const double* src, int n);

// zero array
MJAPI void mjuu_zerovec(double* dest, int n);

// zero float array
MJAPI void mjuu_zerovec(float* dest, int n);

// dot-product in 3D
MJAPI double mjuu_dot3(const double* a, const double* b);

// distance between 3D points
MJAPI double mjuu_dist3(const double* a, const double* b);

// L1 norm between vectors
MJAPI double mjuu_L1(const double* a, const double* b, int n);

// normalize vector to unit length, return previous length
//  if norm(vec)<mjEPS, return 0 and do not change vector
MJAPI double mjuu_normvec(double* vec, int n);
MJAPI float mjuu_normvec(float* vec, int n);

// convert quaternion to rotation matrix
MJAPI void mjuu_quat2mat(double* res, const double* quat);

// multiply two unit quaternions
MJAPI void mjuu_mulquat(double* res, const double* qa, const double* qb);

// multiply matrix by vector, 3-by-3
MJAPI void mjuu_mulvecmat(double* res, const double* vec, const double* mat);

// multiply transposed matrix by vector, 3-by-3
MJAPI void mjuu_mulvecmatT(double* res, const double* vec, const double* mat);

// compute res = R * M * R'
MJAPI void mjuu_mulRMRT(double* res, const double* R, const double* M);

// compute res = A * B, all 3-by-3
MJAPI void mjuu_mulmat(double* res, const double* A, const double* B);

// transpose 3-by-3 matrix
MJAPI void mjuu_transposemat(double* res, const double* mat);

// convert global to local axis relative to given frame
MJAPI void mjuu_localaxis(double* al, const double* ag, const double* quat);

// convert global to local position relative to given frame
MJAPI void mjuu_localpos(double* pl, const double* pg, const double* pos, const double* quat);

// compute quaternion rotation from parent to child
MJAPI void mjuu_localquat(double* local, const double* child, const double* parent);

// compute vector cross-product a = b x c
MJAPI void mjuu_crossvec(double* a, const double* b, const double* c);

// compute normal vector to given triangle (uses float for OpenGL)
MJAPI double mjuu_makenormal(double* normal, const float* a, const float* b, const float* c);

// compute quaternion corresponding to minimal rotation from [0;0;1] to vec
MJAPI void mjuu_z2quat(double* quat, const double* vec);

// compute quaternion corresponding to given rotation matrix (i.e. frame)
MJAPI void mjuu_frame2quat(double* quat, const double* x, const double* y, const double* z);

// invert frame transformation
MJAPI void mjuu_frameinvert(double newpos[3], double newquat[4],
                      const double oldpos[3], const double oldquat[4]);

// accumulate frame transformation into parent frame
MJAPI void mjuu_frameaccum(double pos[3], double quat[4],
                     const double childpos[3], const double childquat[4]);

// accumulate frame transformation into child frame
MJAPI void mjuu_frameaccumChild(const double pos[3], const double quat[4],
                          double childpos[3], double childquat[4]);

// invert frame accumulation
MJAPI void mjuu_frameaccuminv(double pos[3], double quat[4],
                        const double childpos[3], const double childquat[4]);

// convert local_inertia[3] to global_inertia[6]
MJAPI void mjuu_globalinertia(double* global, const double* local, const double* quat);

// compute off-center correction to inertia matrix
MJAPI void mjuu_offcenter(double* res, double mass, const double* vec);

// compute viscosity coefficients from mass and inertia
MJAPI void mjuu_visccoef(double* visccoef, double mass, const double* inertia, double scl=1);

// rotate vector by quaternion
MJAPI void mjuu_rotVecQuat(double res[3], const double vec[3], const double quat[4]);

// update moving frame along a discrete curve or initialize it, returns edge length
MJAPI double mjuu_updateFrame(double quat[4], double normal[3], const double edge[3],
                       const double tprv[3], const double tnxt[3], int first);

// eigenvalue decomposition of symmetric 3x3 matrix
MJAPI int mjuu_eig3(double eigval[3], double eigvec[9], double quat[4], const double mat[9]);

// transform vector by pose
MJAPI void mjuu_trnVecPose(double res[3], const double pos[3], const double quat[4], const double vec[3]);

// compute frame quat and diagonal inertia from full inertia matrix, return error if any
MJAPI const char* mjuu_fullInertia(double quat[4], double inertia[3], const double fullinertia[6]);

namespace mujoco::user {

// utility class for handling file paths
class MJAPI FilePath {
 public:
  FilePath() = default;
  explicit FilePath(const std::string& str) : path_(PathReduce(str)) {}
  explicit FilePath(const char* str) { path_ = PathReduce(str); }
  FilePath(const std::string& str1, const std::string& str2) {
    path_ = PathReduce(Combine(str1, str2));
  }
  FilePath(FilePath&& other) = default;
  FilePath& operator=(FilePath&& other) = default;
  FilePath(const FilePath&) = default;
  FilePath& operator=(const FilePath&) = default;

  // return true if the path is absolute
  bool IsAbs() const { return !AbsPrefix(path_).empty(); }

  // return string with the absolute prefix of the path
  // e.g. "c:\", "http://", etc
  std::string AbsPrefix() const { return AbsPrefix(path_); }

  // return copy of the internal path string
  const std::string& Str() const { return path_; }

  // return copy of the internal path string in lower case
  // (for case insensitive purposes)
  std::string StrLower() const;

  // return the extension of the file path (e.g. "hello.txt" -> ".txt")
  std::string Ext() const;

  // concatenate two paths together
  FilePath operator+(const FilePath& path) const;

  // return a new FilePath with the extension stripped
  FilePath StripExt() const;

  // return a new FilePath with the path stripped
  FilePath StripPath() const;

  // return a new FilePath with path lower cased
  FilePath Lower() const { return FilePathFast(StrLower()); }

  // C++ string methods
  std::size_t size() const { return path_.size(); }
  const char* c_str() const { return path_.c_str(); }
  bool empty() const { return path_.empty(); }
  char operator[](int i) const { return path_[i]; }

 private:
  static std::string AbsPrefix(const std::string& str);
  static std::string PathReduce(const std::string& str);
  static bool IsSeparator(char c) {
    return c == '/' || c == '\\';
  }
  static std::string Combine(const std::string& s1, const std::string& s2);

  // fast constructor that does not call PathReduce
  static FilePath FilePathFast(const std::string& str) {
    FilePath path;
    path.path_ = str;
    return path;
  }

  static FilePath FilePathFast(std::string&& str) {
    FilePath path;
    path.path_ = str;
    return path;
  }

  std::string path_;
};

// read file into memory buffer
MJAPI std::vector<uint8_t> FileToMemory(const char* filename);

// convert vector to string separating elements by whitespace
template<typename T> MJAPI std::string VectorToString(const std::vector<T>& v);

// convert string to vector
template<typename T> MJAPI std::vector<T> StringToVector(char *cs);
template<typename T> MJAPI std::vector<T> StringToVector(const std::string& s);

}  // namespace mujoco::user

// strip path from filename
MJAPI std::string mjuu_strippath(std::string filename);

// strip extension from filename
MJAPI std::string mjuu_stripext(std::string filename);

// get the extension of a filename
MJAPI std::string mjuu_getext(std::string_view filename);

// check if path is absolute
MJAPI bool mjuu_isabspath(std::string path);

// assemble file paths
MJAPI std::string mjuu_combinePaths(const std::string& path1, const std::string& path2);
MJAPI std::string mjuu_combinePaths(const std::string& path1, const std::string& path2,
                              const std::string& path3);

// return type from content_type format {type}/{subtype}[;{parameter}={value}]
MJAPI std::optional<std::string_view> mjuu_parseContentTypeAttrType(std::string_view text);

// return subtype from content_type format {type}/{subtype}[;{parameter}={value}]
MJAPI std::optional<std::string_view> mjuu_parseContentTypeAttrSubtype(std::string_view text);

// convert filename extension to content type; return empty string if not found
MJAPI std::string mjuu_extToContentType(std::string_view filename);

// get the length of the dirname portion of a given path
MJAPI int mjuu_dirnamelen(const char* path);

#endif  // MUJOCO_SRC_USER_USER_UTIL_H_

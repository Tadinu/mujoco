#ifndef MUJOCO_SRC_OPTIMIZATION_UTILITYFUNCTIONS_H
#define MUJOCO_SRC_OPTIMIZATION_UTILITYFUNCTIONS_H

#include <dirent.h>
#include <cstring>
#include <sys/stat.h>
#include <ctime>
#include <iomanip>
#include "Macros.h"

namespace Utils {
inline static void insertIntoTriplets33(TripleVector& triplets, Eigen::Matrix<double, 3, 3> src, int startRow, int startCol) {
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      if (std::abs(src(i,j)) > 1e-10) {
        triplets.emplace_back(startRow + i, startCol + j, src(i,j));
      }
    }
  }
}


template<int rows, int cols>
inline static void insertIntoTriplets(TripleVector& triplets, Eigen::Matrix<double, rows, cols>& src, int startRow, int startCol) {
  for (int i = 0; i <rows; i++) {
    for (int j = 0; j < cols; j++) {
      if (std::abs(src(i,j)) > 1e-10) {
        triplets.emplace_back(startRow + i, startCol + j, src(i,j));
      }
    }
  }
}

template<int rows, int cols>
inline static void insertIntoTriplets(TripleVector& triplets, Eigen::Matrix<double, rows, cols>& src,
                               int blockRows, int blockCols, int srcStartRows, int srcStartCols, int dstStartRow, int dstStartCol ) {
  for (int i = 0; i <blockRows; i++) {
    for (int j = 0; j < blockCols; j++) {
      double val = src(srcStartRows + i, srcStartCols + j);
      if (std::abs(val) > 1e-12) {
        triplets.emplace_back(dstStartRow + i, dstStartCol + j, val);
      }
    }
  }
}

template<int m, int n, int p, int q>
inline static Eigen::Matrix<double, m * p, n * q>
kronecker(const Eigen::Matrix<double, m, n> &A, const Eigen::Matrix<double, p, q> &B) {
  Eigen::Matrix<double, m * p, n * q> C;
  C.setZero();
  for (int i = 0; i < m; i++)
    for (int j = 0; j < n; j++)
      for (int k = 0; k < p; k++)
        for (int l = 0; l < q; l++)
          C(i * p + k, j * q + l) = A(i, j) * B(k, l);

  return C;

}

inline static void checkFolderExistsAndCreate(std::string path) {

    if (mkdir(path.c_str(), 07777) != -1) {
//      std::printf("directory created\n");

    }

}

typedef Eigen::Transform<double, 3, Eigen::Affine> Rotation;
inline static Rotation axisToRotation(Vec3d finalDir, Vec3d initialDir) {
  Rotation rot;
  rot.setIdentity();
  finalDir.normalize();
  initialDir.normalize();
  if ((finalDir - initialDir).norm() > 1e-5) {
    Vec3d perp = initialDir.cross(finalDir);
    double angle = std::acos(finalDir.dot(initialDir));
    rot = Eigen::AngleAxis(angle, perp.normalized());
  }
  return rot;
}

inline static std::vector<std::string>  listFiles( std::string path, std::string extension) // extension = ".txt"
{
  std::vector<std::string> filePaths;
  DIR* dirFile = opendir( path.c_str() );
  if ( dirFile )
  {
    struct dirent* hFile;
    errno = 0;
    while (( hFile = readdir( dirFile )) != NULL )
    {

      // skip folder indices
      if ( !strcmp( hFile->d_name, "."  )) continue;
      if ( !strcmp( hFile->d_name, ".." )) continue;

      // skip hidden files
      if (  hFile->d_name[0] == '.' ) continue;

      char* find = std::strstr( hFile->d_name,  extension.c_str());
      if (find == NULL)
        continue;
      if (  std::string(find) == extension ) {
        filePaths.emplace_back(hFile->d_name);
      }
    }
    closedir( dirFile );
  }

  return filePaths;
}

inline static std::vector<std::string>  listDirectory( std::string path)
{
  std::vector<std::string> filePaths;
  DIR* dirFile = opendir( path.c_str() );
  if ( dirFile )
  {
    struct dirent* hFile;
    errno = 0;
    while (( hFile = readdir( dirFile )) != NULL )
    {
      // skip folder indices
      if ( !strcmp( hFile->d_name, "."  )) continue;
      if ( !strcmp( hFile->d_name, ".." )) continue;

      if (hFile->d_type == DT_DIR)
        filePaths.emplace_back(hFile->d_name);

    }
    closedir(dirFile);
  }

  return filePaths;
}

inline static std::string currentTimestampToStr() {
  time_t rawtime;
  struct tm *timeinfo;
  char buffer[80];

  time(&rawtime);
  timeinfo = localtime(&rawtime);

  strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", timeinfo);
  std::string str(buffer);

  return str;
}


inline static std::string vec3d2str(Vec3d v, int precision = 2) {
  std::ostringstream streamObj3;
  streamObj3 << std::fixed;
  streamObj3 << std::setprecision(precision);
  streamObj3 << "(" << v[0] << "," << v[1] << "," << v[2] << ")";


  return streamObj3.str();
}

inline static std::string vec2str(VecXd v, int precision = 2) {
  std::ostringstream streamObj3;
  streamObj3 << std::fixed;
  streamObj3 << std::setprecision(precision);
  streamObj3 << "(" << v[0];

  for (int i = 1; i < v.rows(); i++)
    streamObj3 << "," << v[i];

  streamObj3 << ")";


  return streamObj3.str();
}

template<int m>
inline static std::string vec2str(Eigen::Matrix<double, m, 1> v, int precision = 2) {
  std::ostringstream streamObj3;
  streamObj3 << std::fixed;
  streamObj3 << std::setprecision(precision);
  streamObj3 << "(" << v[0];

  for (int i = 1; i < m; i++)
    streamObj3 << "," << v[i];

  streamObj3 << ")";


  return streamObj3.str();
}


inline static std::string vecXd2str(VecXd v, int precision = 2) {
  std::ostringstream streamObj3;
  streamObj3 << std::fixed;
  streamObj3 << std::setprecision(precision);
  streamObj3 << "(" << v[0];

  for (int i = 1; i < v.rows(); i++)
    streamObj3 << "," << v[i];

  streamObj3 << ")";


  return streamObj3.str();
}


inline static std::string d2str(double v, int precision = 2) {
  std::ostringstream streamObj3;
  streamObj3 << std::fixed;
  streamObj3 << std::setprecision(precision);
  streamObj3 << v;
  return streamObj3.str();
}

inline static std::string d2scistr(double v, int precision = 2) {
  std::ostringstream streamObj3;
  streamObj3 << std::scientific;
  streamObj3 << std::setprecision(precision);
  streamObj3 << v;
  return streamObj3.str();
}


inline static std::string int2str(int v) {
  return std::to_string(v);
}

inline static std::string lld2str(long long v, int precision = 2) {
  std::ostringstream streamObj3;
  streamObj3 << std::fixed;
  streamObj3 << std::setprecision(precision);
  streamObj3 << v;
  return streamObj3.str();
}

inline static void writeStringToFile(std::string fileName, std::string content) {
  std::ofstream myfile;
  myfile.open(fileName.c_str(), std::ofstream::app);
  myfile << content;
  myfile.close();

}
}
#endif //MUJOCO_SRC_OPTIMIZATION_UTILITYFUNCTIONS_H

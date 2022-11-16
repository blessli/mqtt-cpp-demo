#include<iostream>

#include<Eigen/Dense>

#define MM_PI 3.14159265358979323846264338327950
int main()
{
    Eigen::Matrix3d Matrixcomp;
    Matrixcomp << cos(MM_PI * 3 / 4), -sin(MM_PI * 3 / 4), 0,
			sin(MM_PI * 3 / 4), cos(MM_PI * 3 / 4), 0,
			0, 0, 1;
   std::cout<<Matrixcomp;
   return 0;
}

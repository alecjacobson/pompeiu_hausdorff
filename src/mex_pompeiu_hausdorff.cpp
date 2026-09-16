#include "pompeiu_hausdorff.h"
#include <Eigen/Core>

#include <mex.h>
#include <igl/C_STR.h>
#include <igl/matlab/mexErrMsgTxt.h>
#undef assert
#define assert( isOK ) ( (isOK) ? (void)0 : (void) ::mexErrMsgTxt(C_STR(__FILE__<<":"<<__LINE__<<": failed assertion `"<<#isOK<<"'"<<std::endl) ) )

#include <igl/matlab/MexStream.h>
#include <igl/matlab/parse_rhs.h>
#include <igl/matlab/prepare_lhs.h>

void mexFunction(
  int nlhs,
  mxArray *plhs[],
  int nrhs,
  const mxArray *prhs[])
{
  using namespace std;
  using namespace igl::matlab;
  using namespace Eigen;

  igl::matlab::MexStream mout;
  std::streambuf *outbuf = std::cout.rdbuf(&mout);

  mexErrMsgTxt(nrhs>=4 && nrhs<=7,
    "Usage: [lower,upper_max,dA,time_taken_bvh,time_taken_bounds] = "
    "pompeiu_hausdorff(VA,FA,VB,FB,tol,max_factor,normalize)");

  Matrix<double,Dynamic,3,RowMajor> VA,VB;
  Matrix<int,Dynamic,3,RowMajor> FA,FB;
  parse_rhs_double(prhs+0,VA);
  parse_rhs_index(prhs+1,FA);
  parse_rhs_double(prhs+2,VB);
  parse_rhs_index(prhs+3,FB);
  mexErrMsgTxt(VA.cols()==3,"VA must be #VA by 3");
  mexErrMsgTxt(FA.cols()==3,"FA must be #FA by 3");
  mexErrMsgTxt(VB.cols()==3,"VB must be #VB by 3");
  mexErrMsgTxt(FB.cols()==3,"FB must be #FB by 3");

  double tol = 1e-8;
  double max_factor = 1000000;
  bool normalize = true;
  if(nrhs>4)
  {
    mexErrMsgTxt(mxIsDouble(prhs[4]) && mxGetNumberOfElements(prhs[4])==1,
      "tol should be a scalar");
    tol = *mxGetPr(prhs[4]);
  }
  if(nrhs>5)
  {
    mexErrMsgTxt(mxIsDouble(prhs[5]) && mxGetNumberOfElements(prhs[5])==1,
      "max_factor should be a scalar");
    max_factor = *mxGetPr(prhs[5]);
  }
  if(nrhs>6)
  {
    mexErrMsgTxt(mxGetNumberOfElements(prhs[6])==1,
      "normalize should be a scalar");
    normalize = (bool) *mxGetPr(prhs[6]);
  }

  double lower,upper_max,dA,time_taken_bvh,time_taken_bounds;
  try
  {
    std::tie(lower,upper_max,dA,time_taken_bvh,time_taken_bounds) =
      pompeiu_hausdorff(VA,FA,VB,FB,tol,max_factor,normalize);
  }
  catch(const std::exception & e)
  {
    std::cout.rdbuf(outbuf);
    mexErrMsgTxt(false,e.what());
  }

  switch(nlhs)
  {
    case 5:
      plhs[4] = mxCreateDoubleScalar(time_taken_bounds);
    case 4:
      plhs[3] = mxCreateDoubleScalar(time_taken_bvh);
    case 3:
      plhs[2] = mxCreateDoubleScalar(dA);
    case 2:
      plhs[1] = mxCreateDoubleScalar(upper_max);
    case 1:
      plhs[0] = mxCreateDoubleScalar(lower);
    default:break;
  }

  std::cout.rdbuf(outbuf);
  return;
}

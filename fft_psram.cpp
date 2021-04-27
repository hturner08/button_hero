#include <complex>
const double PI  =3.141592653589793238463;
const std::complex<double> i(0, 1);

void fft_radix2(int *x, std::complex<double> * X, int N, int s){
  int k;
  std::complex<double> t;
  if(N==1){
    X[0] = x[0];
    return;
  }

  fft_radix2(x,X,N/2,2*s);
  fft_radix2(x+s,X+N/2, N/2, 2*s);
  for(k= 0; k< N/2; k++){
    t = X[k];
    X[k] = t + std::exp(-2*PI*k/N*i)*X[k+N/2];
    X[k + N/2] = t - std::exp(-2*PI*k/N*i)*X[k+N/2];
  }
}

void fft(int* x, std::complex<double> * X,int N){
  fft_radix2(x,X,N,1);
}

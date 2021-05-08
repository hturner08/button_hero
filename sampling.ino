//int * downsample(int * arr,int q){
//  int nums[sizeof(arr)/(q*sizeof(int))];
//  int * new_arr = nums;
//  int j = 0;
//  for(int i = 0; i < sizeof(arr)/sizeof(int);i++){
//    if(i%q ==0){
//      new_arr[j] = arr[i];
//      j++; 
//    }
//  }
//  return new_arr;
//}
//int * upsample(int * arr, int p){
// int len = sizeof(arr)*p/sizeof(int);
//  int nums[len];
//  int * new_arr = nums;
// int j = 0;
// for(int i = 1; i < len;i++){
//  int dx = (arr[i]-arr[i-1])/p;
//  new_arr[j]= arr[i-1];
//  j++;
//  for(int k = 1; k < p; k++){
//    new_arr[j] = x[i-1] + k*dx;
//    j++;
//  }
// }
// new_arr[j] = arr[len-1];
// j++;
// int dx = (x[len-1]-x[len-2])/p;
//   for(int k = 1; k < p; k++){
//    new_arr[j] = x[len-1] + k*dx;
//    j++;
//  }
// return new_arr;
//}
//
//int * resample(int *arr, int desired_length){
//  int q = sizeof(arr)/sizeof(int);
//  return downsample(upsample(arr,desired_length),q);
//}
//
//from math import sqrt
//def offset_and_normalize(inp):
//    mean = sum(inp)/len(inp)
//    denom = sqrt(sum([(val-mean)**2 for val in inp]))
//    return [(val-mean)/denom for val in inp]
//
//
//def correlation(x,y):
//    return sum([xval*yval for xval,yval in zip(offset_and_normalize(x),offset_and_normalize(y))])

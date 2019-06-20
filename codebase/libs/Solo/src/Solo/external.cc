

 int hello(int x) {
  return x*100;
}

extern "C" {
  int hello2(int x) {return x*1000;}

}

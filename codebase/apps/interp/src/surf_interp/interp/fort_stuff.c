

// Small fortran routines.

float AMAX1(float x, float y)
{
  if (x>y) return x;
  return y;
}


float AMIN1(float x, float y)
{
  if (x<y) return x;
  return y;
}

int MAX0(int x, int y)
{
  if (x>y) return x;
  return y;
}


int MIN0(int x, int y)
{
  if (x<y) return x;
  return y;
}


float SIGN(float x, float y)
{
  if (y>0) return x;
  return -x;

}






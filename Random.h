#ifndef RANDOM
#define RANDOM
#include "common.h"

class Random
{
  public:
    Random(uint8_t Seed)
    {
      x++;
      a = Seed;
      b = Seed * Seed;
      c = Seed * Seed * Seed;
    }

    uint8_t Get()
    {
      x++;               //x is incremented every round and is not affected by any other variable
      a = (a ^ c ^ x);   //note the mix of addition and XOR
      b = (b + a);       //And the use of very few instructions
      c = (c + (b >> 1)^a); //the right shift is to ensure that high-order bits from b can affect
      return (c);
    }

  private:
    uint8_t a, b, c, x;
};

#endif

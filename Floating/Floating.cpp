// Floating.cpp: определяет точку входа для консольного приложения.
//
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <inttypes.h>
#include <limits.h>

//typedef union u64v{struct{uint32_t l, h;}; uint64_t v;}u64v;

#define FLOATING_DEBUG

template<typename TIntBase, typename TFloatBase, int FractionBits, int ExpBits = sizeof(TIntBase) * 8 - FractionBits-1>
class CFloating
{
public:
   CFloating() :m_ExponentBase(((TIntBase)1 << (ExpBits - 1)) - 1), m_FractionBase((TIntBase)1 << FractionBits){}
   ~CFloating(){}
   TIntBase PowerOfTwo(TIntBase nBits) {return (TIntBase)1 << nBits;}
   TIntBase FindMaxPowerOfTwo(TIntBase iVal){for (TIntBase i = sizeof(TIntBase) * 8 - 1; i > 0; i--){if ((iVal >> i) & 1){return i;}}return 0;}
   template <typename T>
   TFloatBase ToFloat(T iNum)
   {
      Floating_t FloatVal = { 0, 0, 0 };
      TIntBase Num, ExponentPower;
      if (iNum)
      {
         FloatVal.sign = !(iNum > 0);
         Num = (TIntBase)(FloatVal.sign ? -iNum : iNum);
         ExponentPower = FindMaxPowerOfTwo(Num);
         FloatVal.exponent = m_ExponentBase + ExponentPower;
         FloatVal.fraction = (Num - PowerOfTwo(ExponentPower))*(m_FractionBase / PowerOfTwo(ExponentPower));
      }
#ifdef FLOATING_DEBUG
      return FloatVal.flValue;
#else
      return *reinterpret_cast<TFloatBase*>(&FloatVal);
#endif
   }
   template<>TFloatBase ToFloat(float iNum)=delete;
   template<>TFloatBase ToFloat(double iNum)=delete;
   
#ifdef FLOATING_DEBUG
   typedef union Floating_u
   {
      struct
      {
#else
   typedef struct Floating_s
   {
#endif
         TIntBase fraction : FractionBits;//Мантисса
         TIntBase exponent : ExpBits;//Порядок
         TIntBase sign : 1;//Знак
#ifdef FLOATING_DEBUG
      };
      TFloatBase flValue;
#endif
   }Floating_t;
private:
   TIntBase m_ExponentBase;
   TIntBase m_FractionBase;
};

typedef CFloating<uint32_t, float, 23/*, 8*/> CFloat;
typedef CFloating<uint64_t, double, 52/*, 11*/> CDouble;


int main()
{

#ifdef _WIN64
   printf("x64\n");
   typedef int64_t TLoop;
#else
   printf("x32\n");
   typedef int32_t TLoop;
#endif
   CFloat f;
   CDouble d;
  
   bool bHaveError = false;
   #pragma omp parallel sections private(bHaveError)
   {
      #pragma omp section  
      {
         for (TLoop i = 0; i >INT_MIN; i--)
         {
            if ((TLoop)f.ToFloat(i) != i)
            {
               printf("Test float min: %i != %.3f  [0x%x]\n", i, f.ToFloat(i), i);
               bHaveError = true;
               break;
            }
         }
         if (!bHaveError)
         {
            printf("Test float min:OK\n");
         }
      }
      #pragma omp section  
      {
         bHaveError = false;
        
         for (TLoop i = 0; i < INT_MAX; i++)
         {
            if ((TLoop)f.ToFloat(i) != i)
            {
               printf("Test float max:%i != %.3f [0x%x]\n", i, f.ToFloat(i), i);
               bHaveError = true;
               break;
            }
         }
         if (!bHaveError)
         {
            printf("Test float max:OK\n");
         }
      }
      #pragma omp section  
      {
         bHaveError = false;
         for (TLoop i = 0; i > INT_MIN; i--)
         {
            if ((TLoop)d.ToFloat(i) != i)
            {
               printf("Test double min:%i != %.3f  [0x%x]\n", i, d.ToFloat(i), i);
               bHaveError = true;
               break;
            }
         }
         if (!bHaveError)
         {
            printf("Test double min:OK\n");
         }
      }
      #pragma omp section  
      {
         bHaveError = false;
         for (TLoop i = 0; i < INT_MAX; i++)
         {
            if ((TLoop)d.ToFloat(i) != i)
            {
               printf("Test double max:%i != %.3f [0x%x]\n", i, d.ToFloat(i), i);
               bHaveError = true;
               break;
            }
         }
         if (!bHaveError)
         {
            printf("Test double max:OK\n");
         }
      }

   }
   return 0;
}



#include "Libraries/DiscreteMathLibrary.h"

bool UDiscreteMathLibrary::IsPrime(int32 Number)
{
	// exclude most common divisors
	if (Number == 2 || Number == 3) return true;
	if (Number % 2 == 0 || Number % 3 == 0) return false;
	// math magic
	int d = 6;
	while (d * d - 2 * d + 1 <= Number)
	{
		if (Number % (d - 1) == 0) return false;
		if (Number % (d + 1) == 0) return false;
		d += 6;
	}
	return true;
}

int UDiscreteMathLibrary::NextPrime(int32 Number)
{
	while (!IsPrime(++Number)) {}
	return Number;
}

int32 UDiscreteMathLibrary::PosMod(int32 X, int32 N)
{
	if (N == 0) return(0);
	return((X % N + N) % N);
}

int32 UDiscreteMathLibrary::Log2(int32 X)
{
	// From: https://stackoverflow.com/questions/994593/how-to-do-an-integer-log2-in-c
#define S(k) if (X >= (1 << k)) { i += k; X >>= k; }
	int32 i = -(X == 0); S(16); S(8); S(4); S(2); S(1); return i;
#undef S
}

int32 UDiscreteMathLibrary::GetClassDistance(UClass* Base, UClass* Super)
{
	int32 Distance = 0;
	for (const UStruct* Struct = Base; Struct; Struct = Struct->GetSuperStruct())
	{
		if (Struct == Super)
			return Distance;
		Distance++;
	}

	return Distance;
}

float UDiscreteMathLibrary::SmoothStep(float X, int32 Order)
{
	// See https://en.wikipedia.org/wiki/Smoothstep
	float Out = 0.0f;
	for (int i = 0; i <= Order; i++)
	{
		const float A = PascalTriangle(-Order - 1, i);
		const float B = PascalTriangle(2 * Order + 1, Order - i);
		Out += A * B * IntPow(X, Order + i + 1);
	}
	return Out;
}

float UDiscreteMathLibrary::PascalTriangle(int32 A, int32 B)
{
	float Out = 1;
	for (int32 i = 0; i < B; ++i)
	{
		Out *= float(A - i) / (i + 1);
	}
	return Out;
}

float UDiscreteMathLibrary::IntPow(float Base, int32 Exp)
{
	// See https://stackoverflow.com/questions/101439/the-most-efficient-way-to-implement-an-integer-based-power-function-powint-int
	float Out = 1.0f;
	for (;;)
	{
		if (Exp & 1) Out *= Base;
		Exp >>= 1;
		if (!Exp) break;
		Base *= Base;
	}
	return Out;
}

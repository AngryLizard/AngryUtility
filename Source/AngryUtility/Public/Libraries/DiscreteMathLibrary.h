
#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "DiscreteMathLibrary.generated.h"

/**
 * Collection of discrete maths operations not available in standard Unreal
 */
UCLASS()
class ANGRYUTILITY_API UDiscreteMathLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	//	http://stackoverflow.com/questions/30052316/find-next-prime-number-algorithm
	/** Checks if number is prime */
	UFUNCTION(BlueprintPure, Category = "Math", Meta = (Keywords = "C++"))
		static bool IsPrime(int32 Number);

	/** Gets next number that is prime */
	UFUNCTION(BlueprintPure, Category = "Math", Meta = (Keywords = "C++"))
		static int32 NextPrime(int32 Number);

	/** Positive modulo */
	UFUNCTION(BlueprintPure, Category = "Math", Meta = (Keywords = "C++"))
		static int32 PosMod(int32 X, int32 N);

	/** Computes quick mod2 */
	UFUNCTION(BlueprintPure, Category = "Math", Meta = (Keywords = "C++"))
		static int32 Log2(int32 X);

	/** Get class distance (or -1 if not subclasses) */
	UFUNCTION(BlueprintPure, Category = "Math", Meta = (Keywords = "C++"))
		static int32 GetClassDistance(UClass* Base, UClass* Super);

	/** Higher order smoothstep. Assumes X in [0, 1]. */
	UFUNCTION(BlueprintPure, Category = "Math", meta = (Keywords = "C++"))
		static float SmoothStep(float X, int32 Order);

	/** Binomial coefficient (a; b) */
	UFUNCTION(BlueprintPure, Category = "Math", meta = (Keywords = "C++"))
		static float PascalTriangle(int32 A, int32 B);

	/** Integer pow (a; b) */
	UFUNCTION(BlueprintPure, Category = "Math", meta = (Keywords = "C++"))
		static float IntPow(float Base, int32 Exp);
};

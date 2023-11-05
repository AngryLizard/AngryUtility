// Maintained by AngryLizard, netliz.net

#pragma once

#include "CoreMinimal.h"

#include "Matrix3x3.generated.h"

/**
*
*/
USTRUCT(BlueprintType)
struct ANGRYUTILITY_API FMatrix3x3
{
	GENERATED_USTRUCT_BODY()

	FMatrix3x3();
	FMatrix3x3(const FQuat& Q); // Rotation
	FMatrix3x3(const FVector& A, const FVector& B); // Outer procuct
	FMatrix3x3(const TArray<FVector>& Samples); // Covariance
	FMatrix3x3(const FVector& X, const FVector& Y, const FVector& Z); // Build
	FVector operator* (const FVector& Other) const;

	double SizeSquared() const;
	FMatrix3x3 operator+=(const FMatrix3x3& Other);
	FMatrix3x3 operator+ (const FMatrix3x3& Other) const;
	FMatrix3x3 operator-=(const FMatrix3x3& Other);
	FMatrix3x3 operator- (const FMatrix3x3& Other) const;
	FMatrix3x3 operator* (double Scale) const;

	double& operator()(int32 I, int32 J);
	double operator()(int32 I, int32 J)const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector X;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector Y;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector Z;

	FVector Diag() const;
	FVector PowerMethod(const FVector& Input, int32 Iterations) const;
	FVector CholeskyInvert(const FVector& Input) const;
	double Det() const;

	static const FMatrix3x3 Identity;
};
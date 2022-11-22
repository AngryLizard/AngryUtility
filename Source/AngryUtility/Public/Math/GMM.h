// Maintained by AngryLizard, netliz.net

#pragma once

#include "CoreMinimal.h"
#include "Math/Matrix3x3.h"

#include "GMM.generated.h"

// TODO: Not validated to be working yet

USTRUCT(BlueprintType)
struct ANGRYUTILITY_API FGMMDistribution
{
	GENERATED_USTRUCT_BODY()

	FGMMDistribution();
	FGMMDistribution(const TArray<FVector>& Samples);
	float Pdf(const FVector& X) const;

	/** Covariance matrix */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		FMatrix3x3 Cov;

	/** Mean position */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		FVector Mu;

	/** Weight */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float Pi;
};

/**
*
*/
USTRUCT(BlueprintType)
struct ANGRYUTILITY_API FGMM
{
	GENERATED_USTRUCT_BODY()

		FGMM();

	float Step();
	void Simplify(float Threshold);
	void AddPoint(const FVector& Point);

	void EM(int32 MaxIterations, float Threshold);

	/** Points */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TArray<FVector> Points;

	/** Distributions */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TArray<FGMMDistribution> Distributions;
};
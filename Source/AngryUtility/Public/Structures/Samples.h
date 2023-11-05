// Maintained by AngryLizard, netliz.net

#pragma once

#include "CoreMinimal.h"

#include "Samples.generated.h"

USTRUCT(BlueprintType)
struct ANGRYUTILITY_API FSamples
{
	GENERATED_USTRUCT_BODY()

		FSamples();
	FSamples(const TArray<FVector>& Data);

	// Returns mean
	FVector Mean() const;

	// Returns largest distance to center
	float Radius(const FVector& Center) const;

	// Returns largest distance to center along a given normal
	float RadiusAlong(const FVector& Normal) const;

	// Centers data
	FSamples CenterData(const FVector& Center) const;

	// Projects data onto a plane around origin and returns max distance
	FSamples ProjectData(const FVector& Normal) const;

	// Compute Pca on this dataset to get an oriented bounding box
	void Pca(int Iterations, FQuat& Quat, FVector& Extend, FVector& Center) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FVector> Data;
};

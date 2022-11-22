// Maintained by AngryLizard, netliz.net

#pragma once

#include "CoreMinimal.h"

#include "Engine/UserDefinedStruct.h"
#include "Timestamp.generated.h"

#define SECONDS_PER_EPOCH 86'400.0f // One day

/**
* General timestamp structure that provides better accuracy than float but doesn't require casting from double.
* Counts seconds continuously and days discretely, since floats are sufficiently accurate up to 86400
*/
USTRUCT(BlueprintType)
struct ANGRYUTILITY_API FTimestamp
{
	GENERATED_USTRUCT_BODY()
		FTimestamp();

	void Normalize();
	void Clear(); // Set to 0
	bool IsZero() const; // Whether it is 0

	float Modf(float Period = (2*PI), float Speed = 1.0f, float Phase = 0.0f) const;
	float Value() const; // Continuous value for visuals, jumps every epoch
	FString Print() const;

	bool operator==(const FTimestamp& Other) const;
	bool operator!=(const FTimestamp& Other) const;
	bool operator>(const FTimestamp& Other) const;
	bool operator<(const FTimestamp& Other) const;
	float operator-(const FTimestamp& Other) const;
	FTimestamp operator+=(float DeltaSeconds);
	FTimestamp operator+(float DeltaSeconds) const;

	UPROPERTY()
		float Seconds;

	UPROPERTY()
		uint16 Epochs;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
};

template<> struct TStructOpsTypeTraits<FTimestamp> : public TStructOpsTypeTraitsBase2<FTimestamp>
{
	enum { WithNetSerializer = true };
};
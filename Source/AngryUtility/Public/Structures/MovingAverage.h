// Maintained by AngryLizard, netliz.net

#pragma once

#include "MovingAverage.generated.h"

/**
*
*/
USTRUCT()
struct ANGRYUTILITY_API FMovingAverage
{
	GENERATED_USTRUCT_BODY()

	FMovingAverage();
	FMovingAverage(float InitialThreshold);
	bool Update(float Deltatime, float Error);
	float GetAverage() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////

	UPROPERTY()
		float UpdateTime = 2.0f;

	UPROPERTY()
		float Strength = 1.4f;

	UPROPERTY()
		float MaxThreshold = 0.0001f;

private:
	float Timer = 0.0f;
	float Average = 0.0f;
	float Threshold = 0.02f;
};

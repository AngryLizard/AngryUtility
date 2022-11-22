// Maintained by AngryLizard, netliz.net

#include "Structures/MovingAverage.h"

FMovingAverage::FMovingAverage()
{
}

FMovingAverage::FMovingAverage(float InitialThreshold)
:	Threshold(InitialThreshold)
{
}

bool FMovingAverage::Update(float Deltatime, float Error)
{
	bool Update = false;

	// Count up average over one time update
	Timer += Deltatime;
	Average += Error * Deltatime;
	if (Timer >= UpdateTime)
	{
		// Compute average value
		const float Value = Average / Timer;

		// Update if over threshold
		if (Value > Threshold)
		{
			Update = true;
		}

		// Update threshold
		Threshold = FMath::Max(Value * Strength, MaxThreshold);
		Average = 0.0f;
		Timer = 0.0f;
	}

	return Update;
}

float FMovingAverage::GetAverage() const
{
	return Average;
}


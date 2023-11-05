// Maintained by AngryLizard, netliz.net

#include "Structures/Samples.h"
#include "Structures/Matrix3x3.h"

FSamples::FSamples()
{
}

FSamples::FSamples(const TArray<FVector>& Data)
	: Data(Data)
{
}

FVector FSamples::Mean() const
{
	FVector Sum = FVector::ZeroVector;
	for (const FVector& Sample : Data)
	{
		Sum += Sample;
	}
	return Sum / Data.Num();
}

float FSamples::Radius(const FVector& Center) const
{
	float Dist = 0.0f;
	for (const FVector& Sample : Data)
	{
		Dist = FMath::Max(Dist, (Sample - Center).SizeSquared());
	}
	return FMath::Sqrt(Dist);
}

float FSamples::RadiusAlong(const FVector& Normal) const
{
	float Max = 0.0f;
	for (const FVector& Sample : Data)
	{
		const float Dist = Sample | Normal;
		Max = FMath::Max(Max, FMath::Abs(Dist));
	}
	return Max;
}

FSamples FSamples::CenterData(const FVector& Center) const
{
	FSamples Samples(Data);
	for (FVector& Sample : Samples.Data)
	{
		Sample -= Center;
	}
	return Samples;
}

FSamples FSamples::ProjectData(const FVector& Normal) const
{
	FSamples Samples(Data);
	for (FVector& Sample : Samples.Data)
	{
		const float Dist = Sample | Normal;
		Sample -= Dist * Normal;
	}
	return Samples;
}

void FSamples::Pca(int Iterations, FQuat& Quat, FVector& Extend, FVector& Center) const
{
	Center = Mean();
	FSamples Centered = CenterData(Center);
	FMatrix3x3 Cov = FMatrix3x3(Centered.Data);

	const FVector Primary = Cov.PowerMethod(FVector::ForwardVector, Iterations);
	if (Primary.SizeSquared() < SMALL_NUMBER)
	{
		Quat = FQuat::Identity;
		Extend = FVector::ZeroVector;
	}
	else
	{
		Quat = FQuat::FindBetweenNormals(FVector::ForwardVector, Primary);

		Cov = FMatrix3x3(Centered.ProjectData(Primary).Data);
		const FVector Secondary = Cov.PowerMethod(FVector::UpVector, Iterations);
		if (Secondary.SizeSquared() < SMALL_NUMBER)
		{
			Extend = FVector(Centered.RadiusAlong(Primary), 0.0f, 0.0f);
		}
		else
		{
			Quat = FQuat::FindBetweenNormals(Quat.GetAxisY(), Secondary) * Quat;

			const FVector Ternary = (Primary ^ Secondary).GetSafeNormal();
			Extend = FVector(Centered.RadiusAlong(Primary), Centered.RadiusAlong(Secondary), Centered.RadiusAlong(Ternary));
		}
	}
}


#include "Structures/Timestamp.h"

FTimestamp::FTimestamp()
	: Seconds(0.0f), Epochs(0)
{
}

void FTimestamp::Normalize()
{
	while (Seconds > SECONDS_PER_EPOCH)
	{
		Seconds -= SECONDS_PER_EPOCH;
		Epochs++;
	}
}

void FTimestamp::Clear()
{
	Seconds = 0.0f;
	Epochs = 0;
}

bool FTimestamp::IsZero() const
{
	return Seconds == 0.0f && Epochs == 0;
}

float FTimestamp::Modf(float Period, float Speed, float Phase) const
{
	// Take rest of epoch first to prevent big numbers.
	const float EpochNum = SECONDS_PER_EPOCH * Speed / Period;
	const float EpochDiv = FMath::FloorToFloat(EpochNum);

	const float Num = (EpochNum - EpochDiv) * Epochs + (Seconds * Speed + Phase) / Period;
	const float Div = FMath::FloorToFloat(Num);
	return (Num - Div) * Period;
}

float FTimestamp::Value() const
{
	return Seconds;
}

FString FTimestamp::Print() const
{
	return FString::FromInt(Epochs) + " : " + FString::SanitizeFloat(Seconds, 2);
}

bool FTimestamp::operator==(const FTimestamp& Other) const
{
	return Other.Epochs == Epochs && Other.Seconds == Seconds;
}

bool FTimestamp::operator!=(const FTimestamp& Other) const
{
	return Other.Epochs != Epochs || Other.Seconds != Seconds;
}

bool FTimestamp::operator>(const FTimestamp& Other) const
{
	return (Epochs > Other.Epochs) || ((Epochs == Other.Epochs) && (Seconds > Other.Seconds));
}

bool FTimestamp::operator<(const FTimestamp& Other) const
{
	return (Epochs < Other.Epochs) || ((Epochs == Other.Epochs) && (Seconds < Other.Seconds));
}


float FTimestamp::operator-(const FTimestamp& Other) const
{
	return (Epochs - Other.Epochs) * SECONDS_PER_EPOCH + (Seconds - Other.Seconds);
}

FTimestamp FTimestamp::operator+=(float DeltaSeconds)
{
	Seconds += DeltaSeconds;
	return *this;
}

FTimestamp FTimestamp::operator+(float DeltaSeconds) const
{
	FTimestamp Out = *this;
	Out += DeltaSeconds;
	return Out;
}


bool FTimestamp::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	Ar << Seconds;
	Ar << Epochs;
	return bOutSuccess = true;
}

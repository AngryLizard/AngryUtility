#include "Libraries/SpringLibrary.h"

USpringLibrary::USpringLibrary(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

float USpringLibrary::GetFloatSpringStateVelocity(const FFloatSpringState& State)
{
	return State.Velocity;
}

FVector USpringLibrary::GetVectorSpringStateVelocity(const FVectorSpringState& State)
{
	return State.Velocity;
}

FVector USpringLibrary::GetQuaternionSpringStateAngularVelocity(const FQuaternionSpringState& State)
{
	return State.AngularVelocity;
}
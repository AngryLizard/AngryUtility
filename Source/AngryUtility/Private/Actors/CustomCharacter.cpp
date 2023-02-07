
#include "Actors/CustomCharacter.h"

#include "Components/CustomMovementComponent.h"

ACustomCharacter::ACustomCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCustomMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	bUseControllerRotationYaw = true;
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

float ACustomCharacter::ComputeWalkableFloorZ_Implementation(const FHitResult& Hit, const float Default) const
{
	return Default;
}

FVector ACustomCharacter::MovementTick_Implementation(float DeltaTime, const FVector& Velocity, const FVector& InputAcceleration, float InputStrength, FVector& DesiredDirection, FRotator& DesiredTurnRate, float& Friction, float& MaxSpeed)
{
	MaxSpeed *= InputStrength;

	if (!InputAcceleration.IsZero())
	{
		// Friction affects our ability to change direction. This is only done for input acceleration, not path following.
		const FVector AccelDir = InputAcceleration.GetSafeNormal();
		const float VelSize = Velocity.Size();
		const float FrictionFactor = FMath::Min(DeltaTime * Friction, 1.f) / DeltaTime;
		const FVector BrakeAcceleration = (Velocity - AccelDir * VelSize) * FrictionFactor;
		return InputAcceleration - BrakeAcceleration;
	}
	return FVector::ZeroVector;
}
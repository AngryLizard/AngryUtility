
#include "Actors/CustomCharacter.h"

#include "Components/CustomMovementComponent.h"

ACustomCharacter::ACustomCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCustomMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	bUseControllerRotationYaw = false;
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

float ACustomCharacter::ComputeWalkableFloorZ_Implementation(const FHitResult& Hit, const float Default) const
{
	return Default;
}

void ACustomCharacter::MovementTick_Implementation(
	float DeltaTime,
	const FVector& InVelocity,
	const FVector& InAcceleration,
	float InStrength,
	float InFriction,
	float InMaxSpeed,
	FVector& OutAcceleration,
	FVector& OutDesiredDirection,
	FRotator& OutDesiredTurnRate,
	float& OutFriction,
	float& OutMaxSpeed)
{
	// Default implementation that should behave about the same as default character movement
	OutMaxSpeed = InMaxSpeed * InStrength;

	OutDesiredDirection = InAcceleration;
	OutDesiredTurnRate = GetCharacterMovement()->RotationRate;
	OutFriction = InFriction;

	// Brake against going sideways so character doesn't slither when making turns
	if (!InAcceleration.IsZero())
	{
		const FVector AccelDir = InAcceleration.GetSafeNormal();
		const float VelSize = InVelocity.Size();
		const float FrictionFactor = FMath::Min(DeltaTime * OutFriction, 1.f) / DeltaTime;
		const FVector BrakeAcceleration = (InVelocity - AccelDir * VelSize) * FrictionFactor;
		OutAcceleration = InAcceleration - BrakeAcceleration;
	}
}
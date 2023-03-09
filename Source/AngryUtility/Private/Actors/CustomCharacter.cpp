
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

void ACustomCharacter::MovementTick_Implementation(
	float DeltaTime,
	const FVector& InVelocity,
	const FVector& InAcceleration,
	const FRotator& InRotationRate,
	float InStrength,
	float InFriction,
	float InMaxSpeed,
	FVector& OutAcceleration,
	FVector& DesiredDirection,
	FRotator& DesiredTurnRate,
	float& OutFriction,
	float& OutMaxSpeed)
{
	OutMaxSpeed = InMaxSpeed * InStrength;

	DesiredDirection = InAcceleration;
	DesiredTurnRate = InRotationRate;
	OutFriction = InFriction;

	if (!InAcceleration.IsZero())
	{
		const FVector AccelDir = InAcceleration.GetSafeNormal();
		const float VelSize = InVelocity.Size();
		const float FrictionFactor = FMath::Min(DeltaTime * OutFriction, 1.f) / DeltaTime;
		const FVector BrakeAcceleration = (InVelocity - AccelDir * VelSize) * FrictionFactor;
		OutAcceleration = InAcceleration - BrakeAcceleration;
	}
	OutAcceleration =  FVector::ZeroVector;
}
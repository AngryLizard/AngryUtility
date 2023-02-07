#pragma once

#include "Kismet/KismetSystemLibrary.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "CustomMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class ANGRYUTILITY_API UCustomMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	public:
		UCustomMovementComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
		virtual void SetUpdatedComponent(USceneComponent* NewUpdatedComponent) override;
		virtual void PostLoad() override;

		virtual bool IsWalkable(const FHitResult& Hit) const override;
		virtual void CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration) override;
		FRotator ComputeOrientToMovementRotation(const FRotator& CurrentRotation, float DeltaTime, FRotator& DeltaRotation) const override;

protected:

	/** Custom character this movement component belongs to */
	UPROPERTY(Transient, DuplicateTransient)
		TObjectPtr<ACustomCharacter> CustomCharacterOwner = nullptr;

	// We want to also control rotation from movement modes so we temporarily store them here
	FVector DesiredDirection = FVector::ZeroVector;
	FRotator DesiredTurnRate = FRotator::ZeroRotator;
};

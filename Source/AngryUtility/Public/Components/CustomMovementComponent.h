#pragma once

#include "Kismet/KismetSystemLibrary.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "CustomMovementComponent.generated.h"

/**
 * This custom movement component allows us to override custom movement behaviour in the character blueprint.
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
		TObjectPtr<class ACustomCharacter> CustomCharacterOwner = nullptr;

private:
	// Since movement and rotation are computed in different passes we store them here temporarily
	FVector DesiredDirection = FVector::ZeroVector;
	FRotator DesiredTurnRate = FRotator::ZeroRotator;
};

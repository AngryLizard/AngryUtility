#pragma once

#include "GameFramework/Character.h"
#include "CustomCharacter.generated.h"

/**
*/
UCLASS(Blueprintable)
class ANGRYUTILITY_API ACustomCharacter : public ACharacter
{
	GENERATED_BODY()

	public:
		ACustomCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

		UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Movement)
			float ComputeWalkableFloorZ(const FHitResult& Hit, const float Default) const;
	
		UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Movement)
			void MovementTick(
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
				float& OutMaxSpeed);

};
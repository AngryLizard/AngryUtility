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
			FVector MovementTick(float DeltaTime, const FVector& Velocity, const FVector& InputAcceleration, float InputStrength, FVector& DesiredDirection, FRotator& DesiredTurnRate, float& Friction, float& MaxSpeed);

};
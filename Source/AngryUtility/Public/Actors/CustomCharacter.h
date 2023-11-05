#pragma once

#include "GameFramework/Character.h"
#include "CustomCharacter.generated.h"

/**
* Custom character that allows defining custom movement behaviour in Blueprint.
*/
UCLASS(Blueprintable)
class ANGRYUTILITY_API ACustomCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ACustomCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/**
	* Overridable function that determines the Z component of the normal of the steepest walkable surface for the character.
	* Any lower than this and it is not walkable.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Movement)
		float ComputeWalkableFloorZ(const FHitResult& Hit, const float Default) const;

	/**
	* Overridable function for custom character movement.
	* This function is called by every movement mode, you can differentiate between different movement modes with a switch on the current mode.
	*
	* @param InVelocity Current velocity
	* @param InAcceleration Current desired input acceleration
	* @param InStrength Current input strength in [0..1]
	* @param InFriction Current friction against ground
	* @param InMaxSpeed Current max speed
	* @param OutAcceleration Output acceleration, velocity will be changed according to this
	* @param OutDesiredDirection Direction character will try to torn towards. Doesn't have to be normalized
	* @param OutDesiredTurnRate Turn rate for each axis for how fast character turns towards desired direction
	* @param OutFriction Friction to be used for braking force computation.
	* @param OutMaxSpeed Desired max speed character will decelerate towards when going too fast.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Movement)
		void MovementTick(
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
			float& OutMaxSpeed);

};
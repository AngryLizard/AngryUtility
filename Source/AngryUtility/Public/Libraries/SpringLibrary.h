
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "SpringLibrary.generated.h"

/**
* This library provides blueprint access to velocity information of spring states.
* Velocity is a super useful information e.g. when animating character movement.
*/
UCLASS(meta = (ScriptName = "SpringLibrary"))
class ANGRYUTILITY_API USpringLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

		UFUNCTION(BlueprintPure, Category = "Math|Interpolation")
		static float GetFloatSpringStateVelocity(const FFloatSpringState& State);

	UFUNCTION(BlueprintPure, Category = "Math|Interpolation")
		static FVector GetVectorSpringStateVelocity(const FVectorSpringState& State);

	UFUNCTION(BlueprintPure, Category = "Math|Interpolation")
		static FVector GetQuaternionSpringStateAngularVelocity(const FQuaternionSpringState& State);
};

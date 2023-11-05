// Maintained by AngryLizard, netliz.net
#pragma once

#include "CoreMinimal.h"
#include "IPC.generated.h"

/**
*
*/
USTRUCT(BlueprintType)
struct ANGRYUTILITY_API FIPCPosition
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY()
		FVector Position = FVector::ZeroVector;

	UPROPERTY()
		FQuat Rotation = FQuat::Identity;
};

/**
*
*/
USTRUCT(BlueprintType)
struct ANGRYUTILITY_API FIPCDynamic : public FIPCPosition
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY()
		FVector Velocity = FVector::ZeroVector;

	UPROPERTY()
		FVector Angular = FVector::ZeroVector;
};

/**
*
*/
USTRUCT(BlueprintType)
struct ANGRYUTILITY_API FIPCPendulumState
{
	GENERATED_USTRUCT_BODY();

	/** Pendulum X state relative to parent */
	UPROPERTY()
		FVector4 X = FVector4();

	/** Pendulum Y state relative to parent */
	UPROPERTY()
		FVector4 Y = FVector4();

	/** Pendulum Z state relative to parent */
	UPROPERTY()
		FVector2D Z = FVector2D::ZeroVector;

	/** Pendulum rotation state relative to parent */
	UPROPERTY()
		FVector2D R = FVector2D::ZeroVector;
};

/**
*
*/
USTRUCT(BlueprintType)
struct ANGRYUTILITY_API FIPCPendulumProperties
{
	GENERATED_USTRUCT_BODY();

	FVector4 PositionToPendulum(const FIPCPosition& Position, const FVector& UpDirection, const FVector& ForwardDirection) const;
	FVector4 DynamicToPendulum(const FIPCDynamic& Dynamic, const FVector& UpDirection, const FVector& ForwardDirection) const;

	FIPCPendulumState PositionToPendulum(const FIPCPosition& Position) const;
	FIPCPendulumState DynamicToPendulum(const FIPCDynamic& Dynamic) const;
	
	FIPCPosition PendulumToPosition(const FIPCPendulumState& State) const;
	FIPCDynamic PendulumToDynamic(const FIPCPendulumState& State) const;

	/** Pendulum length */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float Length = 1.0f;

	/** Pendulum mass */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float Mass = 1.0f;

	/** Pendulum inertia */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float Inertia = 0.01f;

	/** Pendulum gravity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float Gravity = 1.0f;
};

/**
*
*/
USTRUCT(BlueprintType)
struct ANGRYUTILITY_API FIPCRiccatiProperties
{
	GENERATED_USTRUCT_BODY();

	/** Ricatti Q weight */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float Q = 1.0f;

	/** Ricatti R weight */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float R = 1.0f;

	/** Ricatti step length */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float Dt = 0.001f;

	/** Ricatti max number of iterations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 Iterations = 32;
};

/**
*
*/
USTRUCT(BlueprintType)
struct ANGRYUTILITY_API FIPCProperties
{
	GENERATED_USTRUCT_BODY();

	void Generate(const FIPCPendulumProperties& PendulumProperties, const FIPCRiccatiProperties& RiccatiProperties);
	void SimulateForPosition(FVector4& State, float Position, float DeltaTime) const;
	void SimulateForVelocity(FVector4& State, float Velocity, float DeltaTime) const;

	/** Atate coefficients */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FMatrix StateCoef;

	/** Force coefficients */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector4 ForceCoef = FVector4();

	/** Force response */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector4 ForceResponse = FVector4();
};

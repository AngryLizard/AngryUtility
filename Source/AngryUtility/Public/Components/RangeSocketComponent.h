

#pragma once

#include "CoreMinimal.h"

#include "Components/ShapeComponent.h"
#include "RangeSocketComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (Custom), Blueprintable, meta = (BlueprintSpawnableComponent))
class ANGRYUTILITY_API URangeSocketComponent : public UShapeComponent
{
	GENERATED_BODY()

public:
	URangeSocketComponent();

	/** Radius of this range */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility")
		float Radius = 300.0f;

	/** Local rotation added to the socket Transform */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility")
		FTransform SocketOffset = FTransform::Identity;

	////////////////////////////////////////////////////////////////////////////////////////////////////
private:

	// USceneComponent interface
	virtual bool HasAnySockets() const override;
	virtual FTransform GetSocketTransform(FName InSocketName, ERelativeTransformSpace TransformSpace = RTS_World) const override;
	virtual void QuerySupportedSockets(TArray<FComponentSocketDescription>& OutSockets) const override;
	// End of USceneComponent interface

	/** The name of the range socket */
	static const FName SocketName;

	////////////////////////////////////////////////////////////////////////////////////////////////////
public:

	//~ Begin UObject Interface
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
	//~ End UObject Interface

	//~ Begin USceneComponent Interface
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	//~ End USceneComponent Interface

	//~ Begin UPrimitiveComponent Interface.
	virtual void UpdateBodySetup() override;
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual bool IsZeroExtent() const override;
	virtual bool AreSymmetricRotations(const FQuat& A, const FQuat& B, const FVector& Scale3D) const override;
	//~ End UPrimitiveComponent Interface.
};


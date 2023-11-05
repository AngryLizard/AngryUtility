

#include "Components/RangeSocketComponent.h"
#include "PhysicsEngine/BodySetup.h"

const FName URangeSocketComponent::SocketName(TEXT("RangeSocket"));

URangeSocketComponent::URangeSocketComponent()
	: Super()
{
}

bool URangeSocketComponent::HasAnySockets() const
{
	return true;
}

FTransform URangeSocketComponent::GetSocketTransform(FName InSocketName, ERelativeTransformSpace TransformSpace) const
{
	const FTransform RelativeTransform = SocketOffset;
	FTransform SocketTransform = RelativeTransform * GetComponentTransform();
	SocketTransform.SetScale3D(FVector::OneVector);

	switch (TransformSpace)
	{
	case RTS_World:
	{
		return SocketTransform;
		break;
	}
	case RTS_Actor:
	{
		if (const AActor* Actor = GetOwner())
		{
			return SocketTransform.GetRelativeTransform(Actor->GetTransform());
		}
		break;
	}
	case RTS_Component:
	{
		return RelativeTransform;
	}
	}
	return RelativeTransform;
}

void URangeSocketComponent::QuerySupportedSockets(TArray<FComponentSocketDescription>& OutSockets) const
{
	new (OutSockets) FComponentSocketDescription(SocketName, EComponentSocketType::Socket);
}

#if WITH_EDITOR
void URangeSocketComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	const FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(URangeSocketComponent, Radius))
	{
		Radius = FMath::Abs(Radius);
	}
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif	// WITH_EDITOR

FBoxSphereBounds URangeSocketComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	return FBoxSphereBounds(FVector::ZeroVector, FVector(Radius), Radius).TransformBy(LocalToWorld);
}

// Copy from SphereComponent
template <EShapeBodySetupHelper UpdateBodySetupAction, typename BodySetupType>
bool InvalidateOrUpdateSphereBodySetup(BodySetupType& ShapeBodySetup, bool bUseArchetypeBodySetup, float SphereRadius)
{
	check((bUseArchetypeBodySetup && UpdateBodySetupAction == EShapeBodySetupHelper::InvalidateSharingIfStale) || (!bUseArchetypeBodySetup && UpdateBodySetupAction == EShapeBodySetupHelper::UpdateBodySetup));
	check(ShapeBodySetup->AggGeom.SphereElems.Num() == 1);
	FKSphereElem* SphereElem = ShapeBodySetup->AggGeom.SphereElems.GetData();

	// check for mal formed values
	float Radius = SphereRadius;
	if (Radius < UE_KINDA_SMALL_NUMBER)
	{
		Radius = 0.1f;
	}

	if (UpdateBodySetupAction == EShapeBodySetupHelper::UpdateBodySetup)
	{
		// now set the PhysX data values
		SphereElem->Center = FVector::ZeroVector;
		SphereElem->Radius = Radius;
	}
	else
	{
		if (SphereElem->Radius != Radius)
		{
			ShapeBodySetup = nullptr;
			bUseArchetypeBodySetup = false;
		}
	}

	return bUseArchetypeBodySetup;
}

void URangeSocketComponent::UpdateBodySetup()
{
	if (PrepareSharedBodySetup<URangeSocketComponent>())
	{
		bUseArchetypeBodySetup = InvalidateOrUpdateSphereBodySetup<EShapeBodySetupHelper::InvalidateSharingIfStale>(ShapeBodySetup, bUseArchetypeBodySetup, Radius);
	}

	if (!IsValid(ShapeBodySetup))
	{
		ShapeBodySetup = NewObject<UBodySetup>(this, NAME_None, RF_Transient);
		if (GUObjectArray.IsDisregardForGC(this))
		{
			ShapeBodySetup->AddToRoot();
		}

		// If this component is in GC cluster, make sure we add the body setup to it to
		ShapeBodySetup->AddToCluster(this);
		// if we got created outside of game thread, but got added to a cluster, 
		// we no longer need the Async flag
		if (ShapeBodySetup->HasAnyInternalFlags(EInternalObjectFlags::Async) && GUObjectClusters.GetObjectCluster(ShapeBodySetup))
		{
			ShapeBodySetup->ClearInternalFlags(EInternalObjectFlags::Async);
		}

		ShapeBodySetup->CollisionTraceFlag = CTF_UseSimpleAsComplex;
		AddShapeToGeomArray<FKSphereElem>();
		ShapeBodySetup->bNeverNeedsCookedCollisionData = true;
		bUseArchetypeBodySetup = false;	//We're making our own body setup, so don't use the archetype's.

		//Update bodyinstance and shapes
		BodyInstance.BodySetup = ShapeBodySetup;
		{
			if (BodyInstance.IsValidBodyInstance())
			{
				FPhysicsCommand::ExecuteWrite(BodyInstance.GetActorReferenceWithWelding(), [this](const FPhysicsActorHandle& Actor)
				{
					TArray<FPhysicsShapeHandle> Shapes;
					BodyInstance.GetAllShapes_AssumesLocked(Shapes);

					for (FPhysicsShapeHandle& Shape : Shapes)	//The reason we iterate is we may have multiple scenes and thus multiple shapes, but they are all pointing to the same geometry
					{
						//Update shape with the new body setup. Make sure to only update shapes owned by this body instance
						if (BodyInstance.IsShapeBoundToBody(Shape))
						{
							SetShapeToNewGeom<FKSphereElem>(Shape);
						}
					}
				});
			}
		}
	}

	if (!bUseArchetypeBodySetup)
	{
		InvalidateOrUpdateSphereBodySetup<EShapeBodySetupHelper::UpdateBodySetup>(ShapeBodySetup, bUseArchetypeBodySetup, Radius);
	}
}

FPrimitiveSceneProxy* URangeSocketComponent::CreateSceneProxy()
{
	class FDrawRangeSocketSceneProxy final : public FPrimitiveSceneProxy
	{
	public:
		SIZE_T GetTypeHash() const
		{
			static size_t UniquePointer;
			return reinterpret_cast<size_t>(&UniquePointer);
		}

		FDrawRangeSocketSceneProxy(const URangeSocketComponent* InComponent)
			: FPrimitiveSceneProxy(InComponent)
			, bDrawOnlyIfSelected(InComponent->bDrawOnlyIfSelected)
			, ShapeColor(InComponent->ShapeColor)
			, LocalX(InComponent->SocketOffset.TransformVectorNoScale(FVector::ForwardVector))
			, LocalY(InComponent->SocketOffset.TransformVectorNoScale(FVector::RightVector))
			, Radius(InComponent->Radius)
		{
			bWillEverBeLit = false;
		}

		virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const
		{
			QUICK_SCOPE_CYCLE_COUNTER(STAT_GetDynamicMeshElements_DrawDynamicElements);

			const FMatrix& Transform = GetLocalToWorld();
			for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
			{
				if (VisibilityMap & (1 << ViewIndex))
				{
					const FSceneView* View = Views[ViewIndex];
					const FLinearColor DrawColor = GetViewSelectionColor(ShapeColor, *View, IsSelected(), IsHovered(), false, IsIndividuallySelected());

					FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);

					// Taking into account the min and maximum drawing distance
					const float DistanceSqr = (View->ViewMatrices.GetViewOrigin() - Transform.GetOrigin()).SizeSquared();
					if (DistanceSqr < FMath::Square(GetMinDrawDistance()) || DistanceSqr > FMath::Square(GetMaxDrawDistance()))
					{
						continue;
					}

					if (Radius > SMALL_NUMBER)
					{
						const FVector ScaledX = Transform.GetScaledAxis(EAxis::X).GetSafeNormal();
						const FVector ScaledY = Transform.GetScaledAxis(EAxis::Y).GetSafeNormal();
						const FVector ScaledZ = Transform.GetScaledAxis(EAxis::Z).GetSafeNormal();

						const int32 SphereSides = FMath::Clamp<int32>(Radius / 4.f, 16, 64);
						DrawCircle(PDI, Transform.GetOrigin(), ScaledX, ScaledY, DrawColor, Radius, SphereSides, SDPG_World);
						DrawCircle(PDI, Transform.GetOrigin(), ScaledX, ScaledZ, DrawColor, Radius, SphereSides, SDPG_World);
						DrawCircle(PDI, Transform.GetOrigin(), ScaledY, ScaledZ, DrawColor, Radius, SphereSides, SDPG_World);

						const FVector ScaledLocalX = Transform.TransformVector(LocalX).GetSafeNormal() * (Radius * 0.5f);
						const FVector ScaledLocalY = Transform.TransformVector(LocalY).GetSafeNormal() * (Radius * 0.5f);

						PDI->DrawLine(
							Transform.GetOrigin() + ScaledLocalX,
							Transform.GetOrigin() + ScaledLocalY,
							DrawColor, SDPG_World, 1.f);

						PDI->DrawLine(
							Transform.GetOrigin() + ScaledLocalX,
							Transform.GetOrigin() - ScaledLocalY,
							DrawColor, SDPG_World, 1.f);
					}
				}
			}
		}

		virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const
		{
			const bool bProxyVisible = !bDrawOnlyIfSelected || IsSelected();

			// Should we draw this because collision drawing is enabled, and we have collision
			const bool bShowForCollision = View->Family->EngineShowFlags.Collision && IsCollisionEnabled();

			FPrimitiveViewRelevance Result;
			Result.bDrawRelevance = (IsShown(View) && bProxyVisible) || bShowForCollision;
			Result.bDynamicRelevance = true;
			Result.bShadowRelevance = IsShadowCast(View);
			Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
			return Result;
		}

		virtual uint32 GetMemoryFootprint(void) const override
		{
			return sizeof(*this) + GetAllocatedSize();
		}

		uint32 GetAllocatedSize(void) const
		{
			return FPrimitiveSceneProxy::GetAllocatedSize();
		}

	private:
		const uint32  bDrawOnlyIfSelected : 1;
		const FColor  ShapeColor;
		const FVector LocalX;
		const FVector LocalY;
		const float Radius;
	};

	return new FDrawRangeSocketSceneProxy(this);
}

bool URangeSocketComponent::IsZeroExtent() const
{
	return Radius == 0.f;
}

bool URangeSocketComponent::AreSymmetricRotations(const FQuat& A, const FQuat& B, const FVector& Scale3D) const
{
	// All rotations are equal when scale is uniform.
	// Not detecting rotations around non-uniform scale.
	return Scale3D.GetAbs().AllComponentsEqual() || A.Equals(B);
}

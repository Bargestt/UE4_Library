#pragma once


#include "CoreMinimal.h"
#include "PrimitiveSceneProxy.h"
#include "OctreeTestActor.generated.h"


/** Represents a UComponent to the scene manager. */
class FBaseSceneProxy : public FPrimitiveSceneProxy
{
public:
	SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}

	FBaseSceneProxy(const UPrimitiveComponent* InComponent,bool DrawOnlyIfSelected, FColor BaseColor)
		: FPrimitiveSceneProxy(InComponent)
		, bDrawOnlyIfSelected(DrawOnlyIfSelected)
		, Color(BaseColor)
	{
		bWillEverBeLit = false;
	}

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_FBaseSceneProxy_GetDynamicMeshElements);

		const FMatrix& LocalToWorld = GetLocalToWorld();

		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
		{
			if (VisibilityMap & (1 << ViewIndex))
			{
				const FSceneView* View = Views[ViewIndex];

				const FLinearColor DrawColor = GetViewSelectionColor(Color, *View, IsSelected(), IsHovered(), false, IsIndividuallySelected());

				FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);
				Draw(PDI, LocalToWorld, DrawColor);
			}
		}
	}

	virtual void Draw(FPrimitiveDrawInterface* PDI, const FMatrix& LocalToWorld, const FLinearColor SelectionColor) const
	{
		DrawOrientedWireBox(PDI, LocalToWorld.GetOrigin(), LocalToWorld.GetScaledAxis(EAxis::X), LocalToWorld.GetScaledAxis(EAxis::Y), LocalToWorld.GetScaledAxis(EAxis::Z), FVector(100), SelectionColor, SDPG_World);
	}

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
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
	virtual uint32 GetMemoryFootprint(void) const override { return(sizeof(*this) + GetAllocatedSize()); }
	uint32 GetAllocatedSize(void) const { return(FPrimitiveSceneProxy::GetAllocatedSize()); }

protected:
	const uint32	bDrawOnlyIfSelected : 1;
	const FColor	Color;
};


namespace PrimitiveDraw
{
	void DrawQuad(FPrimitiveDrawInterface* PDI,const FVector& Location, const FQuat& Orintation, float FaceSize, const FLinearColor& SelectionColor, float Thickness = 0, uint8 DepthPriorityGroup = SDPG_World)
	{
		PDI->DrawLine(
			Location + FaceSize * Orintation.GetAxisZ() + FaceSize * Orintation.GetAxisY(),
			Location - FaceSize * Orintation.GetAxisZ() + FaceSize * Orintation.GetAxisY(),
			SelectionColor, SDPG_World, Thickness);

		PDI->DrawLine(
			Location + FaceSize * Orintation.GetAxisZ() - FaceSize * Orintation.GetAxisY(),
			Location - FaceSize * Orintation.GetAxisZ() - FaceSize * Orintation.GetAxisY(),
			SelectionColor, SDPG_World, Thickness);


		PDI->DrawLine(
			Location + FaceSize * Orintation.GetAxisY() + FaceSize * Orintation.GetAxisZ(),
			Location - FaceSize * Orintation.GetAxisY() + FaceSize * Orintation.GetAxisZ(),
			SelectionColor, SDPG_World, Thickness);

		PDI->DrawLine(
			Location + FaceSize * Orintation.GetAxisY() - FaceSize * Orintation.GetAxisZ(),
			Location - FaceSize * Orintation.GetAxisY() - FaceSize * Orintation.GetAxisZ(),
			SelectionColor, SDPG_World, Thickness);
	}
}
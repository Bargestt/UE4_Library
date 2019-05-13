// Fill out your copyright notice in the Description page of Project Settings.

#include "OctreeTesterComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"



DECLARE_CYCLE_STAT(TEXT("OctreeTester ~ PerformTest"), STAT_PerformOctreeTest, STATGROUP_OctreeTester);



UOctreeTesterComponent::UOctreeTesterComponent()
{
	Result = FTesterResult();

	DrawCells = true;
	DrawUnsafeOnly = true;
	RunSingleThread = false;

	TestInitialOverlaps = true; 

	LastTestDuration = 0;

	TestChannels.Add(TEnumAsByte<ECollisionChannel>(ECC_WorldStatic));

	SetMobility(EComponentMobility::Movable);

	UpdateParameters();
}

FPrimitiveSceneProxy* UOctreeTesterComponent::CreateSceneProxy()
{
	/** Represents a UBoxComponent to the scene manager. */
	class FOctreeTesterSceneProxy final : public FPrimitiveSceneProxy
	{
	public:
		SIZE_T GetTypeHash() const override
		{
			static size_t UniquePointer;
			return reinterpret_cast<size_t>(&UniquePointer);
		}

		FOctreeTesterSceneProxy(const UOctreeTesterComponent* InComponent)
			: FPrimitiveSceneProxy(InComponent)
			, bDrawOnlyIfSelected(false)
			, bDrawCells(InComponent->DrawCells)
			, bDrawOnlyUnsafe(InComponent->DrawUnsafeOnly)
			, BoxExtents(InComponent->VolumeSize)
			, BoxColor(FColor::Red)
			, Results(InComponent->GetAllCells())
			, CellSize(InComponent->CellSize)
		{
			bWillEverBeLit = false;
		}

		virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
		{
			QUICK_SCOPE_CYCLE_COUNTER(STAT_ZoneSceneProxy_GetDynamicMeshElements);

			const FMatrix& LocalToWorld = GetLocalToWorld();

			for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
			{
				if (VisibilityMap & (1 << ViewIndex))
				{
					const FSceneView* View = Views[ViewIndex];

					const FLinearColor DrawColor = GetViewSelectionColor(BoxColor, *View, IsSelected(), IsHovered(), false, IsIndividuallySelected());

					FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);
					DrawOrientedWireBox(PDI, LocalToWorld.GetOrigin(), LocalToWorld.GetScaledAxis(EAxis::X), LocalToWorld.GetScaledAxis(EAxis::Y), LocalToWorld.GetScaledAxis(EAxis::Z), BoxExtents, DrawColor, SDPG_World, 4);

					if (Results.Num() > 0 && bDrawCells)
					{
						for (const FTesterCell& cell : Results)
						{			
							if (cell.IsSafe && bDrawOnlyUnsafe) continue;

							DrawOrientedWireBox(PDI, LocalToWorld.TransformPosition(cell.RelativeLocation), LocalToWorld.GetScaledAxis(EAxis::X), LocalToWorld.GetScaledAxis(EAxis::Y), LocalToWorld.GetScaledAxis(EAxis::Z), CellSize, cell.IsSafe? FColor::Green : FColor::Red, SDPG_World, cell.IsSafe ? 0 : 1);
						}
					}
				}
			}
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

	private:
		const uint32	bDrawOnlyIfSelected : 1;
		const uint32	bDrawCells : 1;
		const uint32	bDrawOnlyUnsafe : 1;
		const FVector	BoxExtents;
		const FColor	BoxColor;
		const TArray<FTesterCell> Results;
		const FVector CellSize;
	};

	return new FOctreeTesterSceneProxy(this);
}


void UOctreeTesterComponent::OnRegister()
{
	Super::OnRegister();
	
	UpdateParameters();

	if (GetWorld() && GetWorld()->IsGameWorld())
	{
		UpdateCells();
	}
}


void UOctreeTesterComponent::UpdateCells()
{	
	//SCOPE_CYCLE_COUNTER(STAT_PerformOctreeTest);
	
	if (RunSingleThread) 
	{
		TestsDone = 0;
	}

	
	LastTestDuration = FPlatformTime::Seconds();

	UpdateParameters();
	UpdateCells(GetComponentLocation(), VolumeSize, TestDepth);

	LastTestDuration = (FPlatformTime::Seconds() - LastTestDuration)*1000 ;

	MarkRenderStateDirty();
}

void UOctreeTesterComponent::UpdateParameters()
{
	QueryAdvParameters.bFindInitialOverlaps = TestInitialOverlaps;

	QueryParameters = FCollisionObjectQueryParams();
	for (const TEnumAsByte<ECollisionChannel> cc : TestChannels)
	{
		QueryParameters.AddObjectTypesToQuery(cc);
	}

	CellsDimensions = FVector(FMath::Pow(2, TestDepth - 1));
	VolumeSize = FVector(CellsDimensions * CellSize);

	WorstCaseTestsDone = 1;
	for (int i = 1; i < TestDepth ; i++)
	{		
		WorstCaseTestsDone += FMath::Pow(2, 3 * i);
	}

	const FTransform& tr = GetComponentTransform();

	Result.Dimensions = FIntVector(CellsDimensions.X, CellsDimensions.Y, CellsDimensions.Z);
	Result.Cells.Empty(CellsDimensions.X*CellsDimensions.Y*CellsDimensions.Z);
	for (int z = 0; z < CellsDimensions.Z; z++)
	{
		for (int y = 0; y < CellsDimensions.Y; y++)
		{
			for (int x = 0; x < CellsDimensions.X; x++)
			{
				Result.Cells.Add(FTesterCell{ToRelativeLocation(FVector(x, y, z)), true });
			}
		}
	}
}

FIntVector UOctreeTesterComponent::ToCoordinates(const FVector& RelativeLoc) const
{
	return FIntVector((RelativeLoc + VolumeSize - FVector(CellSize)) / CellSize / 2);
}

FVector UOctreeTesterComponent::ToRelativeLocation(const FVector& Coord) const
{
	FVector vec = Coord * CellSize * 2 - VolumeSize + FVector(CellSize) + 0.5f;
	vec.X = FMath::FloorToFloat(vec.X);
	vec.Y = FMath::FloorToFloat(vec.Y);
	vec.Z = FMath::FloorToFloat(vec.Z);
	return vec;
}

bool UOctreeTesterComponent::IsInVolume(const FVector& RelativeLocation)
{
	return RelativeLocation.X > -VolumeSize.X && RelativeLocation.X < VolumeSize.X &&
		   RelativeLocation.Y > -VolumeSize.Y && RelativeLocation.Y < VolumeSize.Y &&
		   RelativeLocation.Z > -VolumeSize.Z && RelativeLocation.Z < VolumeSize.Z;
}

TArray<FTesterCell> UOctreeTesterComponent::GetAllCells() const
{
	return Result.Cells;
}

TArray<FTesterCell> UOctreeTesterComponent::GetSafeCells() const
{
	return Result.Cells.FilterByPredicate([](const FTesterCell& c) { return c.IsSafe; });
}

TArray<FTesterCell> UOctreeTesterComponent::GetCellsInBox(FIntVector Min, FIntVector Max) const
{
	TArray<FTesterCell> cells;

	if (Result.IsValidCoordinate(Min) && Result.IsValidCoordinate(Max) &&
		Min.X < Max.X && Min.Y < Max.Y && Min.Z < Max.Z
		)
	{
		for (int z = Min.Z; z < Max.Z; z++)
		{
			for (int y = Min.Y; y < Max.Y; y++)
			{
				for (int x = Min.X; x < Max.X; x++)
				{
					//cells.Add( Result.GetCell(x, y, z) );
				}
			}
		}
	}
	return cells;
}

bool UOctreeTesterComponent::UpdateCells(const FVector& Location, const FVector& Extent, int Depth)
{
	Depth--;

	if (RunSingleThread) 
	{
		TestsDone++;
	}

	bool wasOverlap = GetWorld()->OverlapAnyTestByObjectType(Location, GetComponentQuat(), QueryParameters, FCollisionShape::MakeBox(Extent), QueryAdvParameters);

	if (wasOverlap && Depth > 0)
	{
		FVector half = Extent / 2;

		if (half.X >= CellSize && half.Y >= CellSize && half.Z >= CellSize) 
		{
			const TArray<FVector> direction = {
				FVector(1, 1, 1),  FVector(1, -1, 1),  FVector(-1, 1, 1),  FVector(-1, -1, 1),
				FVector(1, 1, -1),	FVector(1, -1, -1), FVector(-1, 1, -1), FVector(-1, -1, -1)
			};

			ParallelFor(direction.Num(), [&](int32 idx)
			{
				UpdateCells(Location + GetComponentQuat() * (half * direction[idx]), half, Depth);
			}, RunSingleThread);
		}
	}

	if (Depth == 0)
	{
		FIntVector coord = ToCoordinates( GetComponentTransform().InverseTransformPosition(Location) );

		Result.GetCell(coord.X, coord.Y, coord.Z).IsSafe = !wasOverlap;
	}

	return wasOverlap;
}


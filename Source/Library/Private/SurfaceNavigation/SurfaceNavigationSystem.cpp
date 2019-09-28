// Fill out your copyright notice in the Description page of Project Settings.

#include "SurfaceNavigationSystem.h"
#include "Kismet/GameplayStatics.h"

#include "SurfaceNavigationActor.h"
#include "Components/BillboardComponent.h"
#include "SurfaceNavigationVolume.h"
#include "SurfaceSampler.h"
#include "SurfaceNavFunctionLibrary.h"
#include "DrawDebugHelpers.h"
#include "MarchingCubesBuilder.h"

DEFINE_LOG_CATEGORY(SurfaceNavigation);


//////////////////////////////////////////////////////////////////////////
//		ASurfaceNavigationActor
//////////////////////////////////////////////////////////////////////////

ASurfaceNavigationActor::ASurfaceNavigationActor()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
#if WITH_EDITOR
	BillboardVisual = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Visual"));
	BillboardVisual->SetupAttachment(RootComponent);
	BillboardVisual->RelativeScale3D = FVector(.25f);
	BillboardVisual->bIsScreenSizeScaled = true;
	ConstructorHelpers::FObjectFinder<UTexture2D> SpriteObj(TEXT("Texture2D'/Game/SurfaceNavigation/Icon.Icon'"));
	if (SpriteObj.Succeeded()) {
		BillboardVisual->SetSprite(SpriteObj.Object);
	}	
#endif //#if WITH_EDITOR
	SurfaceNavigationSystem = CreateDefaultSubobject<USurfaceNavigationSystem>(TEXT("NavSystem"));
}

void ASurfaceNavigationActor::ShowGraph() const
{
	SurfaceNavigationSystem->DrawGraph();
}

void ASurfaceNavigationActor::RebuildGraph()
{
	SurfaceNavigationSystem->RebuildGraph();
}

void ASurfaceNavigationActor::ClearGraph()
{
	SurfaceNavigationSystem->ClearGraph();
}
//////////////////////////////////////////////////////////////////////////



const FSurfacePathfindingResult FSurfacePathfindingResult::Failure = FSurfacePathfindingResult();


USurfaceNavigationSystem::USurfaceNavigationSystem()
{
	Info = TEXT("Info: No info");

	OverrideLocalSamplers = true;
	Sampler = CreateDefaultSubobject<USurfaceSamplerBase>(TEXT("DefaultSampler"));

	SurfaceValue = .5f;
	ShowGraph = true;
	
	VolumesNum = 0;

	
	CelledData.CellSize = 300;
}

void USurfaceNavigationSystem::PostInitProperties()
{
	Super::PostInitProperties();

	TArray<AActor*> FoundVolumes;
	UGameplayStatics::GetAllActorsOfClass(this, ASurfaceNavigationVolume::StaticClass(), FoundVolumes);

	for (AActor* v : FoundVolumes)
	{
		VolumeAdded(Cast<ASurfaceNavigationVolume>(v));
	}
	
}
DECLARE_CYCLE_STAT(TEXT("SurfaceNavigation ~ FindPath"), STAT_FindPath, STATGROUP_SurfaceNavigation);
DECLARE_DWORD_COUNTER_STAT(TEXT("SurfaceNavigation ~ PathLength"), STAT_PathLength, STATGROUP_SurfaceNavigation);
void USurfaceNavigationSystem::FindPathSync(const FVector& From, const FVector& To, FSurfacePathfindingResult& OutResult, FSurfacePathfindingParams Parameters) const
{
	SCOPE_CYCLE_COUNTER(STAT_FindPath);

	const FSurfaceNavigationBox* SharedBox = FindSharedBox(From, To);
	if (SharedBox == nullptr)
	{
		UE_LOG(SurfaceNavigation, Error, TEXT("Pathfind request from: %s To: %s failed. Points are outside of shared nav volume"), *From.ToString(), *To.ToString());
		OutResult = FSurfacePathfindingResult::Failure;
		return;
	}	


	const FSurfaceNavLocalData& NavData = SharedBox->NavData;
	

	int32 EdgeFrom = NavData.FindClosestEdgeIndex(SharedBox->ToLocal(From));
	int32 EdgeTo = NavData.FindClosestEdgeIndex(SharedBox->ToLocal(To));
	

	if (EdgeFrom < 0)
	{
		OutResult = FSurfacePathfindingResult::Failure;
		UE_LOG(SurfaceNavigation, Error, TEXT("Pathfind failed, could not find Start point of path"));
		return;
	}

	bool FindPartial = EdgeTo < 0;

	FSurfacePathfindResult Result = NavData.FindPath(EdgeFrom, EdgeTo);
	SET_DWORD_STAT(STAT_PathLength, Result.Path.Num());

	OutResult = FSurfacePathfindingResult(Result, NavData, SharedBox->BoundingBox.GetCenter());	
}




bool USurfaceNavigationSystem::GetClosestNodeLocation(const FVector& WorldLocation, FVector& OutLocation) const
{
	if (const FSurfaceNavigationBox* Box = FindBox(WorldLocation))
	{
		const FSurfaceNavLocalData NavData = Box->NavData;
		const FVector Center = Box->BoundingBox.GetCenter();
		OutLocation = Center + NavData.ToLocation(NavData.FindClosestEdgeIndex(WorldLocation - Center));
		return true;
	}
	return false;
}



void USurfaceNavigationSystem::VolumeAdded(ASurfaceNavigationVolume* Volume)
{
	if (Volume == nullptr) return;

	uint32 Id = Volume->GetUniqueID();	
	FBox Bounds = Volume->GetComponentsBoundingBox(true);
	VolumeUpdateRequest(FVolumeUpdateRequest(FVolumeUpdateRequest::Add, Id, Bounds));
}

void USurfaceNavigationSystem::VolumeUpdated(ASurfaceNavigationVolume* Volume)
{
	if (Volume == nullptr) return;

	uint32 Id = Volume->GetUniqueID();
	FBox Bounds = Volume->GetComponentsBoundingBox(true);
	VolumeUpdateRequest(FVolumeUpdateRequest(FVolumeUpdateRequest::Update, Id, Bounds));
}

void USurfaceNavigationSystem::VolumeRemoved(ASurfaceNavigationVolume* Volume)
{
	if (Volume == nullptr) return;

	VolumeUpdateRequest(FVolumeUpdateRequest(FVolumeUpdateRequest::Remove, Volume->GetUniqueID()));	
}




void USurfaceNavigationSystem::DrawGraph() const
{	
	//FlushPersistentDebugLines(GetWorld());
	for (const TPair<NavBoxID, FSurfaceNavigationBox>& VolumePair : Volumes)
	{
		const FSurfaceNavigationBox& Box = VolumePair.Value;
		USurfaceNavFunctionLibrary::DrawSurfaceGraph(this, FTransform(FRotator::ZeroRotator, Box.BoundingBox.GetCenter()), Box.NavData);
	}

	CelledData.DrawGraph();
}


void USurfaceNavigationSystem::RebuildGraph()
{
	for (const TPair<NavBoxID, FSurfaceNavigationBox>& VolumePair : Volumes)
	{
		BoxChanged(VolumePair.Key);
	}
}

void USurfaceNavigationSystem::ClearGraph()
{
	CelledData.ClearAllCells();
}

FSurfaceNavigationBox* USurfaceNavigationSystem::FindBoxByID(NavBoxID BoxID)
{
	return Volumes.Find(BoxID);
}

void USurfaceNavigationSystem::RemoveBoxByID(NavBoxID BoxID)
{
	Volumes.Remove(BoxID);
}

void USurfaceNavigationSystem::VolumeUpdateRequest(FVolumeUpdateRequest Request)
{
	if (Request.Type == FVolumeUpdateRequest::Remove)
	{
		RemoveBoxByID(Request.BoxID);		
	}
	else
	{
		if (FSurfaceNavigationBox* Box = FindBoxByID(Request.BoxID))
		{
			if (Request.Type == FVolumeUpdateRequest::Update)
			{
				//Clear old position cells
				TArray<FIntVector> cellsContainingBox = CelledData.GetCellsContainingBox(Box->BoundingBox);
				for (const FIntVector& Coord : cellsContainingBox)
				{
					CelledData.DrawCellBounds(Coord, FColor::Red, 15, 2);
					CelledData.ClearCell(Coord);
				}
			}

			Box->BoundingBox = Request.BoundingBox;
		}
		else
		{
			FSurfaceNavigationBox NewBox;
			NewBox.BoundingBox = Request.BoundingBox;
			Volumes.Add(Request.BoxID, NewBox);
		}

		BoxChanged(Request.BoxID);
	}

	VolumesNum = Volumes.Num();
}

void USurfaceNavigationSystem::BoxChanged(uint32 BoxID)
{
	FSurfaceNavigationBox* Box = FindBoxByID(BoxID);

	if (Box == nullptr)
	{
		UE_LOG(SurfaceNavigation, Error, TEXT("Box update failed. No box found"));
		return;
	}
	if (Sampler == nullptr)
	{
		UE_LOG(SurfaceNavigation, Error, TEXT("Box update failed. No sampler"));
		return;
	}

	// DEBUG //////////////////////////////////////////////////////////////////////////
	CelledData.SetWorld(GetWorld());
	//////////////////////////////////////////////////////////////////////////

	TArray<FIntVector> cellsContainingBox = CelledData.GetCellsContainingBox(Box->BoundingBox);
	for (const FIntVector& Coord : cellsContainingBox)
	{		
		Sampler->ScheduleSampleTask<FSamplerFinishedCell>(CelledData.GetCellBox(Coord), this, &USurfaceNavigationSystem::SamplerFinished, Coord);

		CelledData.DrawCellBounds(Coord, FColor::White, 15, 1);
	}	
}

void USurfaceNavigationSystem::SamplerFinished(FSamplerResult Result, FIntVector CellCoordinate)
{
	FMarchingCubesBuilder Builder(Result.Points, Result.Dimensions);
	Builder.FindBoundaryEdges = true;
	Builder.Build();
	
	FCellCreationData Data;
	Builder.GetData(Data.CellVertices, Data.CellTriangles);
	Builder.GetOuterVertices(Data.OuterVertices);

	CelledData.UpdateCell(CellCoordinate, Data);
	CelledData.DrawCellGraph(CellCoordinate, 5);
} 

const FSurfaceNavigationBox* USurfaceNavigationSystem::FindBox(const FVector& Location) const
{
 	float ClosestDist = TNumericLimits<float>::Max();
 	const FSurfaceNavigationBox* ClosestBox = nullptr;

	for (const TPair<uint32, FSurfaceNavigationBox>& VolumePair : Volumes)
	{
		const FSurfaceNavigationBox& Box = VolumePair.Value;

		if (Box.BoundingBox.IsInside(Location))
		{
			float Dist = (Location - Box.BoundingBox.GetCenter()).SizeSquared();
			if (Dist < ClosestDist)
			{
				ClosestDist = Dist;
				ClosestBox = &Box;
			}
		}
	}

	return ClosestBox;
}


const FSurfaceNavigationBox* USurfaceNavigationSystem::FindSharedBox(const FVector& Location1, const FVector& Location2) const
{
	for (const TPair<NavBoxID, FSurfaceNavigationBox>& VolumePair : Volumes)
	{
		const FSurfaceNavigationBox& Box = VolumePair.Value;
		if (Box.BoundingBox.IsInside(Location1) && Box.BoundingBox.IsInside(Location2))
		{
			return &Box;
		}
	}

	return nullptr;
}



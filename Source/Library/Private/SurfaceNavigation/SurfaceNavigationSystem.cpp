// Fill out your copyright notice in the Description page of Project Settings.

#include "SurfaceNavigationSystem.h"
#include "Kismet/GameplayStatics.h"

#include "SurfaceNavigationActor.h"
#include "Components/BillboardComponent.h"
#include "SurfaceNavigationVolume.h"
#include "SurfaceSampler.h"
#include "SurfaceNavFunctionLibrary.h"
#include "DrawDebugHelpers.h"

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
}

void USurfaceNavigationSystem::PostInitProperties()
{
	Super::PostInitProperties();

	TArray<AActor*> Volumes;
	UGameplayStatics::GetAllActorsOfClass(this, ASurfaceNavigationVolume::StaticClass(), Volumes);

	for (AActor* v : Volumes)
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
	if (FSurfaceNavigationBox* Box = FindBoxByID(Id))
	{
		Box->BoundingBox = Bounds;
	}
	else
	{
		FSurfaceNavigationBox NewBox;
		NewBox.BoundingBox = Bounds;
		Volumes.Add(Id, NewBox);		
	}
	VolumesNum = Volumes.Num();
	BoxChanged(Id);
}

void USurfaceNavigationSystem::VolumeUpdated(ASurfaceNavigationVolume* Volume)
{
	if (Volume == nullptr) return;

	uint32 Id = Volume->GetUniqueID();
	FBox Bounds = Volume->GetComponentsBoundingBox(true);
	if (FSurfaceNavigationBox* Box = FindBoxByID(Id))
	{
		Box->BoundingBox = Bounds;
	}
	else
	{
		FSurfaceNavigationBox NewBox;
		NewBox.BoundingBox = Bounds;
		Volumes.Add(Id, NewBox);
	}
	VolumesNum = Volumes.Num();
	BoxChanged(Id);
}

void USurfaceNavigationSystem::VolumeRemoved(ASurfaceNavigationVolume* Volume)
{
	if (Volume == nullptr) return;

	RemoveBoxByID(Volume->GetUniqueID());
	VolumesNum = Volumes.Num();
}

void USurfaceNavigationSystem::DrawGraph() const
{	
	for (const TPair<NavBoxID, FSurfaceNavigationBox>& VolumePair : Volumes)
	{
		const FSurfaceNavigationBox& Box = VolumePair.Value;
		USurfaceNavFunctionLibrary::DrawSurfaceGraph(this, FTransform(FRotator::ZeroRotator, Box.BoundingBox.GetCenter()), Box.NavData);
	}
}


void USurfaceNavigationSystem::RebuildGraph()
{
	for (const TPair<NavBoxID, FSurfaceNavigationBox>& VolumePair : Volumes)
	{
		BoxChanged(VolumePair.Key);
	}
}

FSurfaceNavigationBox* USurfaceNavigationSystem::FindBoxByID(NavBoxID BoxID)
{
	return Volumes.Find(BoxID);
}

void USurfaceNavigationSystem::RemoveBoxByID(NavBoxID BoxID)
{
	Volumes.Remove(BoxID);
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
	Sampler->ScheduleSampleTask<FSamplerFinished>(Box->BoundingBox, this, &USurfaceNavigationSystem::SamplerFinished, BoxID);
}

void USurfaceNavigationSystem::SamplerFinished(FSamplerResult Result, NavBoxID BoxID)
{
	if (FSurfaceNavigationBox* Box = FindBoxByID(BoxID))
	{
		FSurfaceNavBuilder Builder;
		Builder.SurfaceValue = SurfaceValue;
		Builder.BuildGraph(Result.Points, Result.Dimensions, Box->NavData);		
		UE_LOG(SurfaceNavigation, Log, TEXT("Updated {box:%d} successfully. %d Nodes from %d points"), BoxID, Box->NavData.GetGraph().Num(), Result.Points.Num());
	}

	if (ShowGraph)
	{
		DrawGraph();
	}
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



// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SurfaceNavigation.h"

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SurfaceNavLocalData.h"
#include "CelledSurfaceNavData.h"
#include "SurfaceNavigationSystem.generated.h"

class ASurfaceNavigationVolume;
class USurfaceSamplerBase;
struct FEdgeData;
struct FSamplerResult;




USTRUCT(BlueprintType)
struct LIBRARY_API FSurfacePathfindingParams
{
	GENERATED_BODY()
};


USTRUCT(BlueprintType)
struct LIBRARY_API FSurfacePathfindingResult
{
	GENERATED_BODY()

	static const FSurfacePathfindingResult Failure;
public:
	UPROPERTY(BlueprintReadOnly, Category = "PathfindResult")
		bool IsSuccess;
	UPROPERTY(BlueprintReadOnly, Category = "PathfindResult")
		bool IsPartial;
	UPROPERTY(BlueprintReadOnly, Category = "PathfindResult")
		TArray<FVector> PathLocal;


	FSurfacePathfindingResult()
		: IsSuccess(false)
		, IsPartial(false)
	{}

	FSurfacePathfindingResult(const FSurfacePathfindResult& Result, const FSurfaceNavLocalData& NavData, const FVector& Center)
		: IsSuccess(Result.IsSuccess())
		, IsPartial(Result.IsPartial())		
		, PathLocal(NavData.ToLocations(Result.Path, Center))
	{}
};


struct FSurfaceNavigationBox
{	
	uint32 BoxID;

	FBox BoundingBox;

	FSurfaceNavLocalData NavData;

	FSurfaceNavigationBox()
	{
		BoxID = -1;
		BoundingBox = FBox(FVector(0), FVector(0));
		NavData = FSurfaceNavLocalData();
	}

	bool operator == (const FSurfaceNavigationBox& Other) const { return BoxID == Other.BoxID; }

	bool IsValid() const {	return NavData.GetGraph().Num(); }

	FVector ToLocal(const FVector& World) const { return World - BoundingBox.GetCenter(); }
	FVector ToWorld(const FVector& Local) const { return Local + BoundingBox.GetCenter(); }
};


struct FVolumeUpdateRequest
{
	enum EType{ Add, Update, Remove };

	EType Type;

	uint32 BoxID;

	FBox BoundingBox;

	FVolumeUpdateRequest(EType Type, uint32 BoxID)
		: Type(Type)
		, BoxID(BoxID)
	{}

	FVolumeUpdateRequest(EType Type, uint32 BoxID, FBox BoundingBox)
		: Type(Type)
		, BoxID(BoxID)
		, BoundingBox(BoundingBox)
	{}
};



/**
 * 
 */
UCLASS(collapseCategories)
class LIBRARY_API USurfaceNavigationSystem : public UObject
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	FString Info;	

	UPROPERTY(VisibleAnywhere)
	int VolumesNum;

	/** Use shared sample parameters for every volume */
	UPROPERTY(EditInstanceOnly, meta = (AllowPrivateAccess))
	bool OverrideLocalSamplers;

	UPROPERTY(EditInstanceOnly, Instanced, meta = (AllowPrivateAccess, EditCondition="OverrideLocalSamplers"))
	USurfaceSamplerBase* Sampler;

	UPROPERTY(EditInstanceOnly, meta = (AllowPrivateAccess))
	float SurfaceValue;

	UPROPERTY(EditInstanceOnly, meta = (AllowPrivateAccess))
	bool ShowGraph;


	FCelledSurfaceNavData CelledData;

public:
	USurfaceNavigationSystem();

	virtual void PostInitProperties() override;

	void FindPathSync(const FVector& From, const FVector& To, FSurfacePathfindingResult& OutResult, FSurfacePathfindingParams Parameters) const;

	bool GetClosestNodeLocation(const FVector& Location, FVector& OutLocation) const;


	void VolumeAdded(ASurfaceNavigationVolume* Volume);
	void VolumeUpdated(ASurfaceNavigationVolume* Volume);
	void VolumeRemoved(ASurfaceNavigationVolume* Volume);	

	UFUNCTION(CallInEditor)
	void DrawGraph() const;
	
	void RebuildGraph();

	void ClearGraph();

private:
	typedef uint32 NavBoxID;

	TMap<NavBoxID, FSurfaceNavigationBox> Volumes;

	FSurfaceNavigationBox* FindBoxByID(NavBoxID BoxID);
	void RemoveBoxByID(NavBoxID BoxID);


	void VolumeUpdateRequest(FVolumeUpdateRequest Request);
	void BoxChanged(NavBoxID BoxID);

	DECLARE_DELEGATE_TwoParams(FSamplerFinishedCell, FSamplerResult, FIntVector);
	void SamplerFinished(FSamplerResult Result, FIntVector CellCoordinate);

protected:
	const FSurfaceNavigationBox* FindBox(const FVector& Location) const;

	const FSurfaceNavigationBox* FindSharedBox(const FVector& Location1, const FVector& Location2) const;


public:

	
};

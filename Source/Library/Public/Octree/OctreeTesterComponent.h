// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "OctreeTesterComponent.generated.h"

DECLARE_STATS_GROUP(TEXT("OctreeTester"), STATGROUP_OctreeTester, STATCAT_Advanced);

struct FTesterCell
{
	FVector RelativeLocation;

	bool IsSafe;
};



struct FTesterResult
{
	FIntVector Dimensions;

	TArray<FTesterCell> Cells;


	FTesterResult()
	{
		Dimensions = FIntVector::ZeroValue;
	}

	FTesterResult(const FIntVector& Dimensions)
		: FTesterResult()
	{}	

	const FTesterCell& GetCell(const FIntVector& Coordinates) const
	{
		return GetCell(Coordinates.X, Coordinates.Y, Coordinates.Z);
	}
	FTesterCell& GetCell(const FIntVector& Coordinates)
	{
		return GetCell(Coordinates.X, Coordinates.Y, Coordinates.Z);
	}

	FTesterCell& GetCell(int X, int Y, int Z)
	{
		ensureMsgf(X > 0 && X < Dimensions.X && Y > 0 && Y < Dimensions.Y && Z > 0 && Z < Dimensions.Z, TEXT("Dimension mismatch"));		
		return Cells[X + Y * Dimensions.X + Z * Dimensions.X*Dimensions.Y];		
	}
	const FTesterCell& GetCell(int X, int Y, int Z) const
	{
		ensureMsgf(X > 0 && X < Dimensions.X && Y > 0 && Y < Dimensions.Y && Z > 0 && Z < Dimensions.Z, TEXT("Dimension mismatch"));
		return Cells[X + Y * Dimensions.X + Z * Dimensions.X*Dimensions.Y];
	}

	FIntVector GetDimensions() { return Dimensions; }

	bool IsValidCoordinate(const FIntVector Coordinates) const
	{
		return Coordinates.X > 0 && Coordinates.X < Dimensions.X && Coordinates.Y > 0 && Coordinates.Y < Dimensions.Y && Coordinates.Z > 0 && Coordinates.Z < Dimensions.Z;
	}
};



/**
 * 
 */
UCLASS(meta = (BlueprintSpawnableComponent), Blueprintable, BlueprintType, hideCategories = (Physics, Collision, Lighting))
class LIBRARY_API UOctreeTesterComponent : public UPrimitiveComponent
{
	GENERATED_BODY()
	
public:
	UPROPERTY(VisibleAnywhere, Category = "Tester")
	FVector VolumeSize;

	UPROPERTY(VisibleAnywhere, Category = "Tester")
	FVector CellsDimensions;

	/** Debug property, shown only if RunSingleThread is true */
	UPROPERTY(VisibleAnywhere, Category = "Tester|Debug")
	int TestsDone;
	
	/** In Milliseconds */
	UPROPERTY(VisibleAnywhere, Category = "Tester|Debug")
	double LastTestDuration;

	UPROPERTY(VisibleAnywhere, Category = "Tester")
	int WorstCaseTestsDone;


	UPROPERTY(EditAnywhere, Category = "Tester")
	int TestDepth = 3;

	UPROPERTY(EditAnywhere, Category = "Tester")
	float CellSize = 50;



	UPROPERTY(EditAnywhere, Category = "Tester")
	bool TestInitialOverlaps;

	UPROPERTY(EditAnywhere, Category = "Tester")
	TArray<TEnumAsByte<ECollisionChannel>> TestChannels;
	
	FCollisionObjectQueryParams QueryParameters;

	FCollisionQueryParams QueryAdvParameters;


	UPROPERTY(EditAnywhere, Category = "Tester|Debug")
	bool DrawUnsafeOnly;

	UPROPERTY(EditAnywhere, Category = "Tester|Debug")
	bool DrawCells;

	/** Debug property for performance comparison */
	UPROPERTY(EditAnywhere, Category = "Tester|Debug")
	bool RunSingleThread;


	FTesterResult Result;	

public:
	UOctreeTesterComponent();

	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
protected:


	virtual void OnRegister() override;

	

public:
	/** Run test */
	UFUNCTION(BlueprintCallable, Category = "Tester")
	void UpdateCells();

	UFUNCTION(BlueprintCallable, Category = "Tester")
	void UpdateParameters();

	
	UFUNCTION(BlueprintCallable, Category = "Tester")
	FIntVector ToCoordinates(const FVector& RelativeLoc) const;

	UFUNCTION(BlueprintCallable, Category = "Tester")
	FVector ToRelativeLocation(const FVector& Coordinates) const;

	UFUNCTION(BlueprintCallable, Category = "Tester")
	bool IsInVolume(const FVector& RelativeLoc);

	TArray<FTesterCell> GetAllCells() const;

	TArray<FTesterCell> GetSafeCells() const;

	TArray<FTesterCell> GetCellsInBox(FIntVector Min, FIntVector Max) const;

private:
	bool UpdateCells(const FVector& Location, const FVector& Extent, int Depth);
};

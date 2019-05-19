// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoxNavigationVolume.generated.h"

class UBoxComponent;


struct FBoxNavCell
{
	FVector PositionLocal;
	bool bTraversable;
};

struct FBoxNavData
{
	TArray<TArray<TArray<FBoxNavCell>>> NavCells;

	FIntVector Dimensions;

	const FBoxNavCell& GetCell(const FIntVector& Coordinates) const
	{
		return NavCells[Coordinates.X][Coordinates.Y][Coordinates.Z];
	}

	bool IsValidCoordinates(const FIntVector& Coordinates) const
	{
		return  Coordinates.X >= 0 && Coordinates.X < Dimensions.X &&
				Coordinates.Y >= 0 && Coordinates.Y < Dimensions.Y &&
				Coordinates.Z >= 0 && Coordinates.Z < Dimensions.Z;
	}



	//////////////////////////////////////////////////////////////////////////
	// FGraphAStar: TGraph
	typedef FIntVector FNodeRef;

	static const FIntVector Directions[26];

	int32 GetNeighbourCount(FNodeRef NodeRef) const { return 26; }

	bool IsValidRef(FNodeRef NodeRef) const { return IsValidCoordinates(NodeRef); }
	
	FNodeRef GetNeighbour(const FNodeRef NodeRef, const int32 NeiIndex) const
	{
		return NodeRef + Directions[NeiIndex];
	}
	//////////////////////////////////////////////////////////////////////////
};

UCLASS()
class LIBRARY_API ABoxNavigationVolume : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, meta=(AllowPrivateAccess = true))
	UBoxComponent* Box;

	FIntVector Dimensions;

	FBoxNavData NavData;
	
public:
	UPROPERTY(EditAnywhere, Category = Cells)
	float CellSize;

	UPROPERTY(EditAnywhere, Category = Cells)
	bool bShowCells;

	UPROPERTY(VisibleAnywhere, Category = Cells)
	bool bCellsCreated;
		
public:	
	// Sets default values for this actor's properties
	ABoxNavigationVolume();


	UFUNCTION(CallInEditor, Category = Cells)
	void CreateCells();


public:
	UPROPERTY(EditAnywhere, Category = NavigationTest)
	bool bUpdateCells;

	UPROPERTY(EditAnywhere, Category = NavigationTest)
	FIntVector FromCell;
	
	UPROPERTY(EditAnywhere, Category = NavigationTest)
	FIntVector ToCell;

	UFUNCTION(CallInEditor, Category = NavigationTest)
	void ShowPath();

protected:

	bool FindPath(const FIntVector& From, const FIntVector& To, TArray<FIntVector>& OutPath);
};

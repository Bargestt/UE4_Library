// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EdgeFinder.h"

/**
 * 
 */
class LIBRARY_API FEdgeFinderMap : public FEdgeFinder
{
	typedef TArray<int32> EdgeRefs;

	TMap<int32, EdgeRefs> EdgeMap;

	FIntVector Dimensions;

	float MapCellSize;

	static const float DefaultCellSize;

	bool bIsBuilt;


	static const FIntVector ExpansionDirection[6];

public:
	FEdgeFinderMap(const FSurfaceNavLocalData& NavData);
	~FEdgeFinderMap() {}

	
	virtual int32 FindEdgeIndex(const FVector& Location) const override;
	virtual bool IsReady() const override;
	virtual bool Rebuild() override;

protected:

	int32 ToIndex(const FVector& Location) const;
	
	int32 FindEdgeInMap(const FVector& Location, bool SearchNeighbours = false) const;

	int32 FindClosest(const FVector& Location, const TArray<int32>& EdgeIndices) const;


	TArray<int32> GetNeighboursCoords(const FIntVector& CenterCoords, const FIntVector& Direction, int32 Distance) const;
	

};

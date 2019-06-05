// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class FSurfaceNavLocalData;



/**
 * 
 */
class LIBRARY_API FEdgeFinder
{
protected:

	const FSurfaceNavLocalData& NavData;

public:

	FEdgeFinder(const FSurfaceNavLocalData& NavData) : NavData(NavData){ }

	virtual ~FEdgeFinder() {}


	virtual int32 FindEdgeIndex(const FVector& Location) const = 0;
	virtual bool IsReady() const = 0;
	virtual bool Rebuild() = 0;

public:


#if !UE_BUILD_SHIPPING
	UWorld* World;

	FTransform Transform;
#endif // !UE_BUILD_SHIPPING
	void SetDebugInfo(UWorld* World, FTransform Tr) 
	{ 
#if !UE_BUILD_SHIPPING
		this->World = World, Transform = Tr; 
#endif // !UE_BUILD_SHIPPING
	}


};

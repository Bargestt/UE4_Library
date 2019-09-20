// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StaticIndexArray.h"
#include "PerfomanceTester.generated.h"

DECLARE_STATS_GROUP(TEXT("PerfomanceTester"), STATGROUP_PerfomanceTester, STATCAT_Advanced);

/**
*
*/
UCLASS(NotBlueprintable, hideCategories = ("Rendering", "LOD", "Cooking", "Input"))
class LIBRARY_API APerfomanceTester : public AActor
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, Category = "TestParams")
	int MinSize;

	UPROPERTY(EditAnywhere, Category = "TestParams")
	int MaxSize;

	UPROPERTY(EditAnywhere, Category = "TestParams")
	int Steps;

private:
	UPROPERTY()
	bool InProgress;

	int CurrentStep;

public:
	APerfomanceTester();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaSeconds) override;

protected:

	struct VectValidator
	{
		static bool IsValid(const FVector& Vector)
		{
			return FMath::Abs(Vector.X) < MAX_FLT && FMath::Abs(Vector.Y) < MAX_FLT && FMath::Abs(Vector.Z) < MAX_FLT;
		}
	};
	TStaticIndexArray<FVector, VectValidator> Arr;

	void StartTests();
	void FinishTests();

	void PerformTest(int Size);


public:

	UFUNCTION(CallInEditor)
		void Test();

};
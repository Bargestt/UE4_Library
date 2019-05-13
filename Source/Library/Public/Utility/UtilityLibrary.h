// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UtilityLibrary.generated.h"

/**
 * 
 */
UCLASS()
class LIBRARY_API UUtilityLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()


	UFUNCTION(BlueprintCallable, Category = "Utility")
	static void PrintStringWithKey(int key = -1, float time = 5, FLinearColor color = FLinearColor(0,1,1), const FString& data = TEXT(""))
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(key, time, color.ToFColor(false), data);
		}
	}
	
};

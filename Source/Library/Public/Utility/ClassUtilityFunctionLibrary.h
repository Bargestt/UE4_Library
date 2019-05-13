// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/SCS_Node.h"
#include "ClassUtilityFunctionLibrary.generated.h"


/**
 * 
 */
UCLASS()
class LIBRARY_API UClassUtilityFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Get root component of Class Default Object  
	 */ 
	UFUNCTION(BlueprintCallable, Category = "Class")
	static USceneComponent* GetDefaultRootComponent(const TSubclassOf<AActor> InActorClass);

	/**
	 * Search components in Class Default Object. 
	 * Searches for C++ added component first
	 * If is Blueprint class, search will be done from components added by children classes
	 * @return Found component
	 */
	UFUNCTION(BlueprintCallable, Category = "Class", meta = (ComponentClass = "ActorComponent"), meta = (DeterminesOutputType = "ComponentClass"))
	static UActorComponent* GetDefaultComponent(const TSubclassOf<AActor> InActorClass, const TSubclassOf<UActorComponent> ComponentClass);

	/** Templatized version for syntactic nicety. */
	template< class T >
	static T* GetDefaultComponent(const TSubclassOf<AActor> InActorClass)
	{
		static_assert(TPointerIsConvertibleFromTo<T, const UActorComponent>::Value, "'T' template parameter to FindDefaultComponentInClass must be derived from UActorComponent");
	
		return (T*) GetDefaultComponent(InActorClass, T::StaticClass());
	}

	/**
	 * Search components of specified class in Class Default Object. 
	 */
	UFUNCTION(BlueprintCallable, Category = "Class", meta = (ComponentClass = "ActorComponent"), meta = (DeterminesOutputType = "ComponentClass"))
	static TArray<UActorComponent*> GetDefaultComponents(const TSubclassOf<AActor> InActorClass, const TSubclassOf<UActorComponent> ComponentClass);
	
	/**
	 * Search components of specified class in Class Default Object. 
	 * Template version.
	 */
	template< class T >
	static void GetDefaultComponents( const TSubclassOf<AActor> InActorClass, TArray<T*>& OutComponents)
	{
		static_assert(TPointerIsConvertibleFromTo<T, const UActorComponent>::Value, "'T' template parameter to GetDefaultComponentsInClass must be derived from UActorComponent");

		OutComponents.Reset(0);

		if (!IsValid(InActorClass))
		{
			return;
		}

		// Check CDO(Class default object).		
		AActor* ActorCDO = InActorClass->GetDefaultObject<AActor>();
		ActorCDO->GetComponents<T>(OutComponents);

		// Get Blueprint components
		UBlueprintGeneratedClass* RootBlueprintGeneratedClass = Cast<UBlueprintGeneratedClass>(InActorClass);
		UClass* ActorClass = InActorClass;

		while (ActorClass != AActor::StaticClass() && ActorClass != nullptr)
		{
			UBlueprintGeneratedClass* ActorBlueprintGeneratedClass = Cast<UBlueprintGeneratedClass>(ActorClass);
			if (ActorBlueprintGeneratedClass)
			{
				const TArray<USCS_Node*>& ActorBlueprintNodes = ActorBlueprintGeneratedClass->SimpleConstructionScript->GetAllNodes();
				for (USCS_Node* Node : ActorBlueprintNodes)
				{
					if (Node->ComponentClass->IsChildOf(T::StaticClass()))
					{
						OutComponents.Add(Cast<T>(Node->GetActualComponentTemplate(RootBlueprintGeneratedClass)));
					}
				}
			}
			else
			{
				break;
			}
			// Step up inheritance
			ActorClass = Cast<UClass>(ActorClass->GetSuperStruct());			
		}
	}

	UFUNCTION(BlueprintCallable)
	static void Test(const TSubclassOf<AActor> InActorClass);
};

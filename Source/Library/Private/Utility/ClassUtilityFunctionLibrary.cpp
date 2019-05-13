// Fill out your copyright notice in the Description page of Project Settings.

#include "ClassUtilityFunctionLibrary.h"




USceneComponent* UClassUtilityFunctionLibrary::GetDefaultRootComponent(const TSubclassOf<AActor> InActorClass)
{
	if (!IsValid(InActorClass))
	{
		return nullptr;
	}

	// Check CDO.
	AActor* ActorCDO = InActorClass->GetDefaultObject<AActor>();
	USceneComponent* FoundComponent = ActorCDO->GetRootComponent();
	// Check blueprint 
	if (FoundComponent == nullptr)
	{
		UBlueprintGeneratedClass* BlueprintGeneratedClass = Cast<UBlueprintGeneratedClass>(InActorClass);
		if (BlueprintGeneratedClass)
		{
			FoundComponent = BlueprintGeneratedClass->SimpleConstructionScript->GetSceneRootComponentTemplate();
		}
	}

	return FoundComponent;
}

UActorComponent* UClassUtilityFunctionLibrary::GetDefaultComponent(const TSubclassOf<AActor> InActorClass, const TSubclassOf<UActorComponent> ComponentClass)
{
	if (!IsValid(InActorClass) || !IsValid(ComponentClass))
	{
		return nullptr;
	}

	// Check CDO.
	AActor* ActorCDO = InActorClass->GetDefaultObject<AActor>();
	UActorComponent* FoundComponent = ActorCDO->FindComponentByClass(ComponentClass);

	if (FoundComponent == nullptr)
	{
		// Check blueprint nodes. 
		// Components added in blueprint editor only (and not in code) are not available from CDO.
		UBlueprintGeneratedClass* RootBlueprintGeneratedClass = Cast<UBlueprintGeneratedClass>(InActorClass);
		UClass* ActorClass = InActorClass;

		// Go up the inheritance tree to find nodes that were added to parent blueprints of our blueprint graph.
		while (ActorClass != AActor::StaticClass() && ActorClass != nullptr)
		{
			UBlueprintGeneratedClass* ActorBlueprintGeneratedClass = Cast<UBlueprintGeneratedClass>(ActorClass);
			if (ActorBlueprintGeneratedClass)
			{
				const TArray<USCS_Node*>& ActorBlueprintNodes = ActorBlueprintGeneratedClass->SimpleConstructionScript->GetAllNodes();

				USCS_Node* FoundNode = *ActorBlueprintNodes.FindByPredicate(
					[ComponentClass](USCS_Node* node) {	return node->ComponentClass->IsChildOf(ComponentClass); });

				if (FoundNode)
				{
					FoundComponent = FoundNode->GetActualComponentTemplate(RootBlueprintGeneratedClass);
					break;
				}
			}
			else
			{
				break;
			}
			ActorClass = Cast<UClass>(ActorClass->GetSuperStruct());			
		}
	}

	return FoundComponent;
}


TArray<UActorComponent*> UClassUtilityFunctionLibrary::GetDefaultComponents(const TSubclassOf<AActor> InActorClass, const TSubclassOf<UActorComponent> ComponentClass)
{
	TArray<UActorComponent*> Components;

	if (!IsValid(InActorClass) || !IsValid(ComponentClass))
	{
		return Components;
	}

	// Check CDO.
	AActor* ActorCDO = InActorClass->GetDefaultObject<AActor>();	
	Components = ActorCDO->GetComponentsByClass(ComponentClass);	

	// Blueprint components
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
				if (Node->ComponentClass->IsChildOf(ComponentClass))
				{
					Components.Add(Node->GetActualComponentTemplate(RootBlueprintGeneratedClass));					
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
	return Components;
}

void UClassUtilityFunctionLibrary::Test(const TSubclassOf<AActor> InActorClass)
{
	TArray<UActorComponent*> bpArr;
	TArray<USceneComponent*> tArr;
	bool isEqual = false;

	bpArr = GetDefaultComponents(InActorClass, USceneComponent::StaticClass());
	GetDefaultComponents<USceneComponent>(InActorClass, tArr);
	
	UE_LOG(LogTemp, Warning, TEXT(""));
	UE_LOG(LogTemp, Warning, TEXT("Compare Templated : BPexposed"));
	UE_LOG(LogTemp, Warning, TEXT("Len: %d : %d"), tArr.Num(), bpArr.Num());

	if (tArr.Num() == bpArr.Num())
	{
		for (int i = 0; i < tArr.Num(); i++)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s  %s  |  %s"), (tArr[i] == bpArr[i] ? TEXT("Equal") : TEXT("NotEqual")), *tArr[i]->GetName(), *bpArr[i]->GetName());
		}
	}

}



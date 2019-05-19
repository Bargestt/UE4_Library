// Fill out your copyright notice in the Description page of Project Settings.

#include "GraphNode.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "EngineUtils.h"
#include "GraphAStar.h"


// Sets default values
AGraphNode::AGraphNode()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
	Visual->SetupAttachment(RootComponent);
	Visual->SetRelativeScale3D(FVector(0.25f));
	Visual->SetCollisionObjectType(ECC_WorldDynamic);

	ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"));
	Visual->SetStaticMesh(Mesh.Object);
	
	bAutoLink = false;
	AutoLinkRadius = 500;
}

void AGraphNode::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (bAutoLink)
	{
		AutoLink();
	}
}

void AGraphNode::AutoLink()
{
	TArray<AActor*> Nodes;
	UKismetSystemLibrary::SphereOverlapActors(GetWorld(), GetActorLocation(), AutoLinkRadius, { TEnumAsByte<EObjectTypeQuery>(ECC_WorldDynamic) }, GetClass(), { this }, Nodes);
	
	for (AActor* a : Nodes)
	{
		if (AGraphNode* Node = Cast<AGraphNode>(a))
		{
			Neighbours.AddUnique(Node);
			Node->Neighbours.AddUnique(this);
		}
	}

	DrawDebugSphere(GetWorld(), GetActorLocation(), AutoLinkRadius, 12, FColor::White, false, 1);
}



void AGraphNode::DrawAllLinks()
{
	const UWorld* world = GetWorld();

	FlushPersistentDebugLines(world);

	for (TActorIterator<AGraphNode> It(GetWorld()); It; ++It)
	{
		AGraphNode* Node = *It;
		if (!Node->IsPendingKill())
		{
			for (AGraphNode* childnode : Node->Neighbours)
			{
				DrawDebugLine(world, Node->GetActorLocation(), childnode->GetActorLocation(), FColor::Red, false, 60, 0, 3);				
			}
		}
	}
}

void AGraphNode::AutoLinkAll()
{
	for (TActorIterator<AGraphNode> It(GetWorld()); It; ++It)
	{
		AGraphNode* Node = *It;
		if (!Node->IsPendingKill())
		{
			Node->Neighbours.Empty();
			Node->AutoLink();
		}
	}
}

void AGraphNode::ResetPathValues()
{
	Parent = nullptr;
	H = MAX_FLT;
	G = H;
	F = H;
}

void AGraphNode::ResetAll()
{
	for (TActorIterator<AGraphNode> It(GetWorld()); It; ++It)
	{
		AGraphNode* Node = *It;
		if (!Node->IsPendingKill())
		{
			Node->ResetPathValues();
		}
	}
}

void AGraphNode::FindPathToTarget()
{
	if (Target == nullptr)
	{
		return;
	}
	const UWorld* world = GetWorld();
	DrawDebugLine(world, Target->GetActorLocation(), Target->GetActorLocation() + FVector(0, 0, 150), FColor::Blue, false, 15);
	DrawDebugLine(world, GetActorLocation(), GetActorLocation() + FVector(0, 0, 150), FColor::Blue, false, 15);

	TArray<AGraphNode*> Path;	
	if (AStar(this, Target, Path))
	{
		for (int Index = 0; Index < Path.Num() - 1; Index++)
		{
			DrawDebugLine(world, Path[Index]->GetActorLocation(), Path[Index + 1]->GetActorLocation(), FColor::Cyan, false, 15, 0, 4);
		}
	}
}


bool AGraphNode::AStar(AGraphNode* From, AGraphNode* To, TArray<AGraphNode*>& OutPath)
{
	if (From == nullptr || To == nullptr) return false;

	if (From == To)
	{
		OutPath = { From };
		return true;
	}


	TArray<AGraphNode*> OpenList;
	TArray<AGraphNode*> ClosedList;

	From->F = 0;
	OpenList.Add(From);

	while (OpenList.Num() > 0)
	{
		float minF = OpenList[0]->F;
		int minIndex = 0;
		for (int Index = 0; Index < OpenList.Num() ; Index++)
		{
			if (OpenList[Index]->F < minF)
			{
				minF = OpenList[Index]->F;
				minIndex = Index;
			}
		}

		AGraphNode* Current = OpenList[minIndex];
		OpenList.RemoveAt(minIndex);

		for (AGraphNode* Child : Current->Neighbours)
		{
			if (Child == To)
			{
				Child->Parent = Current;
				ClosedList.Add(Child);
				break;
			}

			float ChildG = Current->G + (Child->GetActorLocation() - Current->GetActorLocation()).Size();
			
			if (OpenList.Contains(Child))
			{
				float ChildF = ChildG + Child->H;

				if (ChildF < Child->F)
				{					
					Child->G = ChildG;
					Child->F = Child->G + Child->H;
					Child->Parent = Current;
				}

				continue;
			}

			if (ClosedList.Contains(Child))
			{
				float ChildF = ChildG + Child->H;

				if (ChildF < Child->F)
				{					
					Child->G = ChildG;
					Child->F = Child->G + Child->H;
					Child->Parent = Current;

					OpenList.Add(Child);
				}

				continue;
			}

			Child->Parent = Current;
			Child->G = ChildG;			
			Child->H = (Child->GetActorLocation() - To->GetActorLocation()).Size();
			Child->F = Child->G + Child->H;
			OpenList.Add(Child);			
		}
		ClosedList.Add(Current);
	}


	AGraphNode* Prev = To;
	while (Prev != nullptr)
	{
		DrawDebugSphere(Prev->GetWorld(), Prev->GetActorLocation(), 25, 12, FColor::Yellow, false, 5);		

		OutPath.Insert(Prev, 0);
		Prev = Prev->Parent;
	}
	   
	return To->Parent != nullptr;
}

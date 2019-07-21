// Fill out your copyright notice in the Description page of Project Settings.

#include "MarchingCubesDisplay.h"
#include "Components/BoxComponent.h"

#include "DrawDebugHelpers.h"
#include "MarchingCubesBuilder.h"


// Sets default values
AMarchingCubesDisplay::AMarchingCubesDisplay()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	CubeSize = 50;
	BoxFrame = CreateDefaultSubobject<UBoxComponent>(TEXT("Bounds"));
	BoxFrame->SetBoxExtent(FVector(CubeSize));
	RootComponent = BoxFrame;

	MeshDisplay = CreateDefaultSubobject<USimpleMeshDrawingComponent>(TEXT("DisplayMesh"));
	MeshDisplay->SetupAttachment(RootComponent);
	
	
	ConstructorHelpers::FObjectFinder<UMaterialInterface> MeshMat(TEXT("Material'/Game/Materials/DisplayPoligons.DisplayPoligons'"));
	if (MeshMat.Succeeded())
	{
		MeshMaterial = MeshMat.Object;
	}	
	ConstructorHelpers::FObjectFinder<UMaterialInterface> RedMat(TEXT("Material'/Engine/EditorMaterials/WidgetMaterial_X.WidgetMaterial_X'"));
	if (RedMat.Succeeded())
	{
		RedMaterial = RedMat.Object;
	}
	ConstructorHelpers::FObjectFinder<UMaterialInterface> GreenMat(TEXT("Material'/Engine/EditorMaterials/WidgetMaterial_Y.WidgetMaterial_Y'"));
	if (GreenMat.Succeeded())
	{
		GreenMaterial = GreenMat.Object;
	}
	ConstructorHelpers::FObjectFinder<UMaterialInterface> BlueMat(TEXT("Material'/Engine/EditorMaterials/WidgetMaterial_Z.WidgetMaterial_Z'"));
	if (BlueMat.Succeeded())
	{
		BlueMaterial = BlueMat.Object;
	}

	CubeVertexStatus = { 0,0,0,0, 0,0,0,0 };
	CubeVertices = {
		{-CubeSize,  CubeSize, -CubeSize},
		{ CubeSize,  CubeSize, -CubeSize},
		{ CubeSize, -CubeSize, -CubeSize},
		{-CubeSize, -CubeSize, -CubeSize},

		{-CubeSize,  CubeSize,  CubeSize},
		{ CubeSize,  CubeSize,  CubeSize},
		{ CubeSize, -CubeSize,  CubeSize},
		{-CubeSize, -CubeSize,  CubeSize}
	};	
}

void AMarchingCubesDisplay::OnConstruction(const FTransform& Transform)
{	
	Super::OnConstruction(Transform);

	MeshDisplay->ClearAllMeshSections();
	
	DrawConfig(CubeVertexStatus);
	

	int Config = FMarchingCubesBuilder::EdgeTable[ConfigNumber];
	DrawConfig(
		{ 
			(bool)(Config & (1 << 0)),
			(bool)(Config & (1 << 1)),
			(bool)(Config & (1 << 2)),
			(bool)(Config & (1 << 3)),
			(bool)(Config & (1 << 4)),
			(bool)(Config & (1 << 5)),
			(bool)(Config & (1 << 6)),
			(bool)(Config & (1 << 7))
		}
		, FVector(0, CubeSize * 2.5f, 0)
	);
	
	// Marching cubes unique configurations. Others 200+ are rotation/mirror of this
	TArray<TArray<bool>> BaseCases
	{
		{ 1,0,0,0, 0,0,0,0 },

		{ 1,1,0,0, 0,0,0,0 },
		{ 1,0,0,0, 0,1,0,0 },
		{ 1,0,0,0, 0,0,1,0 },

		{ 0,1,1,1, 0,0,0,0 },
		{ 1,1,0,0, 0,0,1,0 },

		{ 1,1,0,0, 1,0,1,0 },
		{ 1,1,1,1, 0,0,0,0 },
		{ 1,0,1,1, 0,0,0,1 },
		{ 1,0,1,0, 1,0,1,0 },
		{ 1,0,1,1, 0,0,1,0 },
		{ 0,1,1,1, 1,0,0,0 },
		{ 1,0,1,0, 0,1,0,1 },
		{ 0,1,1,1, 0,0,0,1 }
	};
		

	float Step = CubeSize*2.3f;
	float Base = -Step * BaseCases.Num()/2;
	for (int Index = 0; Index < BaseCases.Num(); Index++)
	{
		TArray<bool>& Case = BaseCases[Index];
		DrawConfig(Case, FVector(0, Index*Step + Base, -CubeSize*5));
	}	

	if (bShowAll)
	{
		DrawAllCases();
	}

	MeshDisplay->ApplyContext(DrawContext);
}

void AMarchingCubesDisplay::BeginPlay()
{
	Super::BeginPlay();

	if (bShowAll)
	{
		DrawAllCases();
		MeshDisplay->ApplyContext(DrawContext);
	}
}

void AMarchingCubesDisplay::DrawConfig(TArray<bool> Config, FVector Offset /*= FVector(0)*/)
{
	TArray<int32> Indices;
	TArray<FVector> Verts;
	
	FVector4 Cube[] =
	{
		FVector4(CubeVertices[0] + Offset, Config[0] ? 1 : 0),
		FVector4(CubeVertices[1] + Offset, Config[1] ? 1 : 0),
		FVector4(CubeVertices[2] + Offset, Config[2] ? 1 : 0),
		FVector4(CubeVertices[3] + Offset, Config[3] ? 1 : 0),
		FVector4(CubeVertices[4] + Offset, Config[4] ? 1 : 0),
		FVector4(CubeVertices[5] + Offset, Config[5] ? 1 : 0),
		FVector4(CubeVertices[6] + Offset, Config[6] ? 1 : 0),
		FVector4(CubeVertices[7] + Offset, Config[7] ? 1 : 0)
	};
	FMarchingCubesBuilder::PoligonizeSingle(Cube, 0.5f, Verts, Indices);
	

	DrawMesh(Verts, Indices, MeshMaterial);

	float BoxSize = 3;
	for (int Index = 0; Index < 8 ; Index++)
	{
		USimpleMeshDrawingComponent::GetBoxMesh(CubeVertices[Index] + Offset, FVector(BoxSize), Verts, Indices);
		DrawMesh(Verts, Indices, Config[Index] ? RedMaterial : GreenMaterial);
	}	
}



void AMarchingCubesDisplay::DrawAllCases()
{
	float Step = CubeSize * 2.3f;
	float Base = -Step * 8;
	float HeightOffset = -CubeSize * 15;

	for (int X = 0; X < 16; X++)
	{
		for (int Y = 0; Y < 16; Y++)
		{
			int CaseIndex = X * 16 + Y;
			FVector Loc = FVector(X*Step + Base, Y*Step + Base, HeightOffset);

			TArray<bool> CaseConfig;
			CaseConfig.Init(false, 8);
			for (int Index = 0; Index < 8 ; Index++)
			{
				CaseConfig[Index] = (bool)(CaseIndex & (1 << Index));
			}
			DrawConfig(CaseConfig, Loc);
			//Only for play mode
			DrawDebugString(GetWorld(), GetActorTransform().TransformPosition(Loc), FString::FromInt(CaseIndex));
		}
	}
}

void AMarchingCubesDisplay::DrawMesh(const TArray<FVector>& Verts, const TArray<int32>& Indices, UMaterialInterface* DrawMaterial /*= nullptr*/)
{
	DrawContext.DrawMesh(Verts, Indices, DrawMaterial);
}







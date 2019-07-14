// Fill out your copyright notice in the Description page of Project Settings.

#include "MarchingCubesDisplay.h"
#include "ProceduralMeshComponent.h"
#include "Components/BoxComponent.h"

#include "MarchingCubesFunctionLibrary.h"
#include "DrawDebugHelpers.h"




void FDrawContext::DrawMesh(const TArray<FVector>& Verts, const TArray<int32>& Indices, UMaterialInterface* Material)
{
	FSection* pSection = Sections.Find(Material);
	FSection& Section = pSection ? *pSection : Sections.Add(Material);

	int32 VerticesBeginIndex = Section.Vertices.Num();
	int32 IndicesBeginIndex = Section.Indices.Num();

	Section.Vertices.Append(Verts);
	Section.Indices.Append(Indices);
	
	if (VerticesBeginIndex > 0) 
	{
		for (int Index = IndicesBeginIndex; Index < Section.Indices.Num(); Index++)
		{
			Section.Indices[Index] += VerticesBeginIndex;
		}
	}
}





// Sets default values
AMarchingCubesDisplay::AMarchingCubesDisplay()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	CubeSize = 50;
	BoxFrame = CreateDefaultSubobject<UBoxComponent>(TEXT("Bounds"));
	BoxFrame->SetBoxExtent(FVector(CubeSize));
	RootComponent = BoxFrame;

	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("DisplayMesh"));
	Mesh->SetupAttachment(RootComponent);
	
	
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

	Mesh->ClearAllMeshSections();
	
	DrawConfig(CubeVertexStatus);
	

	int Config = UMarchingCubesFunctionLibrary::EdgeTable[ConfigNumber];
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

	ApplyContext(Context);
}

void AMarchingCubesDisplay::BeginPlay()
{
	Super::BeginPlay();

	if (bShowAll)
	{
		DrawAllCases();
		ApplyContext(Context);
	}
}

void AMarchingCubesDisplay::DrawConfig(TArray<bool> Config, FVector Offset /*= FVector(0)*/)
{
	TArray<int32> Indices;
	TArray<FVector> Verts;
	
	UMarchingCubesFunctionLibrary::Poligonise
	(
		{
			FVector4(CubeVertices[0] + Offset, Config[0] ? 1 : 0),
			FVector4(CubeVertices[1] + Offset, Config[1] ? 1 : 0),
			FVector4(CubeVertices[2] + Offset, Config[2] ? 1 : 0),
			FVector4(CubeVertices[3] + Offset, Config[3] ? 1 : 0),
			FVector4(CubeVertices[4] + Offset, Config[4] ? 1 : 0),
			FVector4(CubeVertices[5] + Offset, Config[5] ? 1 : 0),
			FVector4(CubeVertices[6] + Offset, Config[6] ? 1 : 0),
			FVector4(CubeVertices[7] + Offset, Config[7] ? 1 : 0)
		}
	, 0.5f, Verts, Indices);

	DrawMesh(Verts, Indices, MeshMaterial);

	float BoxSize = 3;
	for (int Index = 0; Index < 8 ; Index++)
	{
		GetBoxMesh(CubeVertices[Index] + Offset, BoxSize, Verts, Indices);
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
	Context.DrawMesh(Verts, Indices, DrawMaterial);
}






void AMarchingCubesDisplay::DrawLine(FVector From, FVector To, float LineSize /*= 5*/)
{
	TArray<FVector> Verts;
	TArray<int32> Indices;

	GetLineMesh(From, To, CubeSize, Verts, Indices);
	DrawMesh(Verts, Indices, MeshMaterial);
}

void AMarchingCubesDisplay::DrawBox(FVector Pos, float Size)
{
	TArray<FVector> Verts;
	TArray<int32> Indices;

	GetBoxMesh(Pos, Size, Verts, Indices);
	DrawMesh(Verts, Indices, MeshMaterial);
}

void AMarchingCubesDisplay::GetLineMesh(FVector From, FVector To, float Size, TArray<FVector>& OutVerts, TArray<int32>& OutIndices)
{
	FVector Dir = (To - From).GetSafeNormal(Size);
	if (Dir.IsNearlyZero())
	{
		Dir = FVector::ForwardVector;
		To = From + Dir * Size * 2;
	}

	FQuat Rotation = Dir.ToOrientationQuat();

	FVector Up = Rotation.GetUpVector();
	FVector Right = Rotation.GetRightVector();

	OutVerts =
	{
		From + Up * Size,
		From - Up * Size,
		To + Up * Size,
		To - Up * Size,

		From + Right * Size,
		From - Right * Size,
		To + Right * Size,
		To - Right * Size
	};

	OutIndices =
	{
		0, 1, 3,
		0, 3, 2,
		4, 5, 7,
		4, 7, 6
	};
}


void AMarchingCubesDisplay::GetBoxMesh(FVector Pos, float Size, TArray<FVector>& OutVerts, TArray<int32>& OutIndices)
{
	FBox Box(Pos - Size, Pos + Size);
	OutVerts =
	{
		FVector(Box.Min.X, Box.Min.Y, Box.Max.Z),
		FVector(Box.Max.X, Box.Min.Y, Box.Max.Z),
		FVector(Box.Min.X, Box.Min.Y, Box.Min.Z),
		FVector(Box.Max.X, Box.Min.Y, Box.Min.Z),
		FVector(Box.Min.X, Box.Max.Y, Box.Max.Z),
		FVector(Box.Max.X, Box.Max.Y, Box.Max.Z),
		FVector(Box.Min.X, Box.Max.Y, Box.Min.Z),
		FVector(Box.Max.X, Box.Max.Y, Box.Min.Z)
	};
	OutIndices =
	{
		3,2,0,
		3,0,1,
		7,3,1,
		7,1,5,
		6,7,5,
		6,5,4,
		2,6,4,
		2,4,0,
		1,0,4,
		1,4,5,
		7,6,2,
		7,2,3
	};
}







void AMarchingCubesDisplay::ApplyContext(FDrawContext& Context)
{
	if (!Mesh) return;

	for (auto& SectionPair : Context.Sections)
	{
		UMaterialInterface* Material = SectionPair.Key;
		FSection& Section = SectionPair.Value;

		int32 Index = Materials.Find(Material);
		if (Index < 0)
		{
			Index = Materials.Add(Material);			
		}
		
		Mesh->CreateMeshSection(Index, Section.Vertices, Section.Indices, {}, {}, {}, {}, {}, {}, {}, false);
		if (Material != nullptr)
		{
			Mesh->SetMaterial(Index, Material);
		}
	}

	Context.Sections.Empty();
}


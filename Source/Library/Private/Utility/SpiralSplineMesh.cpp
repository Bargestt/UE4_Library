// Fill out your copyright notice in the Description page of Project Settings.


#include "SpiralSplineMesh.h"
#include <Components/SplineMeshComponent.h>
#include <Components/SplineComponent.h>

DECLARE_CYCLE_STAT(TEXT("SpiralSplineMesh ~ UpdateSpline"), STAT_UpdateSpline, STATGROUP_SpiralSplineMesh);
DECLARE_CYCLE_STAT(TEXT("SpiralSplineMesh ~ UpdateSplineMeshes"), STAT_UpdateSplineMeshes, STATGROUP_SpiralSplineMesh);
DECLARE_CYCLE_STAT(TEXT("SpiralSplineMesh ~ CreateStaticMeshes"), STAT_CreateStaticMeshes, STATGROUP_SpiralSplineMesh);
DECLARE_CYCLE_STAT(TEXT("SpiralSplineMesh ~ Copy"), STAT_Copy, STATGROUP_SpiralSplineMesh);

DECLARE_DWORD_COUNTER_STAT(TEXT("SpiralSplineMesh ~ Num Points"), STAT_NumPoints, STATGROUP_SpiralSplineMesh);

//#define PROFILE_SPIRAL

// Sets default values for this component's properties
USpiralSplineMesh::USpiralSplineMesh()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	
	Revolutions = 6;
	StartAngle = 0;
	EndPointRadius = 25;

	TransitionExponent = 2;

	BaseStep = 50;
	SpiralBaseStepMod = 0.1;

	MaxPointsNum = 100;

	// ...
}


// Called when the game starts
void USpiralSplineMesh::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void USpiralSplineMesh::OnVisibilityChanged()
{
	Super::OnVisibilityChanged();

	bool isVisible = IsVisible();

	int SplineSegmentsNum = Spiral->GetNumberOfSplinePoints() - 1;
	for (int Index = 0; Index < Segments.Num() ; Index++)
	{		
		Segments[Index]->SetVisibility(isVisible && Index < SplineSegmentsNum);
	}
}

void USpiralSplineMesh::OnRegister()
{	
	Super::OnRegister();

	if (Spiral == nullptr)
	{
		Spiral = NewObject<USplineComponent>(this, NAME_None, RF_Transient | RF_Transactional | RF_TextExportTransient);
		Spiral->SetupAttachment(this);
		Spiral->RegisterComponent();
	}
	UpdateSplinePoints();

	CreateSplineMeshes();
	UpdateSplineMeshes();
}

void USpiralSplineMesh::SetMaterial(int32 ElementIndex, class UMaterialInterface* Material)
{
	Super::SetMaterial(ElementIndex, Material);

	SegmentMaterial = Material;
	for (USplineMeshComponent* segment : Segments)
	{
		segment->SetMaterial(ElementIndex, SegmentMaterial);
	}
}





void USpiralSplineMesh::UpdateSplinePoints() const
{

#ifdef PROFILE_SPIRAL
	SCOPE_CYCLE_COUNTER(STAT_UpdateSpline);
#endif // PROFILE_SPIRAL	

	if (Spiral == nullptr) return;

	float Height = GetSplineLength();


	TArray<FSplinePoint> Points;
	float CurrentHeight = 0;
	int Index = 0;
	float SpiralStep = (SpiralBaseStepMod > 0)? BaseStep * SpiralBaseStepMod : BaseStep;

	while (CurrentHeight < Height && Index < MaxPointsNum)
	{		
		float Alpha = FMath::Pow(CurrentHeight / Height, TransitionExponent);
		

		float Angle = FMath::DegreesToRadians(StartAngle) + PI * Revolutions * CurrentHeight / Height;
		FVector SpiralPosition = FVector(0, FMath::Cos(Angle) * EndPointRadius, FMath::Sin(Angle) * EndPointRadius);

		FSplinePoint Point;
		Point.InputKey = Index;				
		Point.Position = GetTransformAtDistanceAlongSpline(CurrentHeight, ESplineCoordinateSpace::Local)
						 .TransformPosition(FMath::Lerp(FVector(0), SpiralPosition, Alpha));
		Points.Add(Point);

		Index++;
		CurrentHeight += FMath::Lerp(BaseStep, SpiralStep, Alpha);
	}

	Spiral->ClearSplinePoints(false);
	Spiral->AddPoints(Points, true);
}

void USpiralSplineMesh::CreateSplineMeshes()
{

#ifdef PROFILE_SPIRAL
	SCOPE_CYCLE_COUNTER(STAT_CreateStaticMeshes);
#endif // PROFILE_SPIRAL

	if (Spiral && Spiral->GetNumberOfSplinePoints() >= 2)
	{
		int SplineSegmentsNum = Spiral->GetNumberOfSplinePoints() - 1;
		
		Segments.Reserve(SplineSegmentsNum);
		for (int Index = 0; Index < SplineSegmentsNum; Index++)
		{
			USplineMeshComponent* Comp = nullptr;

			if (Index < Segments.Num())
			{
				Comp = Segments[Index];
			}
			if(Comp == nullptr)
			{				
				Comp = NewObject<USplineMeshComponent>(this, NAME_None, RF_Transient | RF_Transactional | RF_TextExportTransient);
				Comp->Mobility = EComponentMobility::Movable;
				Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				Comp->SetupAttachment(this);
				Comp->RegisterComponent();				

				if (Index < Segments.Num())
				{
					Segments[Index] = Comp;
				}
				else 
				{
					Segments.Add(Comp);
				}
			}			

			Comp->SetStaticMesh(Mesh);
			Comp->SetMaterial(0, SegmentMaterial);
		}
	}
}

void USpiralSplineMesh::UpdateSplineMeshes(int StartFrom /*= 0*/)
{

#ifdef PROFILE_SPIRAL
	SCOPE_CYCLE_COUNTER(STAT_UpdateSplineMeshes);
#endif // PROFILE_SPIRAL

	if (StartFrom >= Segments.Num())
	{
		return;
	}

	if (Spiral && Mesh)
	{
		int SplineSegmentsNum = Spiral->GetNumberOfSplinePoints() - 1;

		VisibleSegments = SplineSegmentsNum;
		HiddenSegments = Segments.Num() - SplineSegmentsNum;

		bool isVisible = IsVisible();

		for (int Index = StartFrom; Index < Segments.Num(); Index++)
		{		
			USplineMeshComponent* Comp = Segments[Index];

			if (Index >= SplineSegmentsNum)
			{
				Comp->SetVisibility(false);
				continue;
			}
			
			FVector LocationStart, TangentStart, LocationEnd, TangentEnd;
			Spiral->GetLocationAndTangentAtSplinePoint(Index, LocationStart, TangentStart, ESplineCoordinateSpace::Local);			
			Spiral->GetLocationAndTangentAtSplinePoint(Index+1, LocationEnd, TangentEnd, ESplineCoordinateSpace::Local);

			Comp->SetStartAndEnd(LocationStart, TangentStart, LocationEnd, TangentEnd, true);
			Comp->SetVisibility(isVisible);
		}
	}
}


void USpiralSplineMesh::CopyFromOther(const USpiralSplineMesh* Other)
{
	if (Other && Other->GetSpiral() && Spiral)
	{

#ifdef PROFILE_SPIRAL
		SCOPE_CYCLE_COUNTER(STAT_Copy);
#endif // PROFILE_SPIRAL
		USplineComponent* OtherSpiral = Other->GetSpiral();

		this->ClearSplinePoints(false);
		for (int Index = 0; Index < Other->GetNumberOfSplinePoints(); Index++)
		{
			this->AddSplinePoint(Other->GetLocationAtSplinePoint(Index, ESplineCoordinateSpace::Local), ESplineCoordinateSpace::Local, false);
			this->SetTangentAtSplinePoint(Index, Other->GetTangentAtSplinePoint(Index, ESplineCoordinateSpace::Local), ESplineCoordinateSpace::Local, false);
		}
		this->UpdateSpline();

		Spiral->ClearSplinePoints(false);
		for (int Index = 0; Index < OtherSpiral->GetNumberOfSplinePoints(); Index++)
		{
			Spiral->AddSplinePoint(OtherSpiral->GetLocationAtSplinePoint(Index, ESplineCoordinateSpace::Local), ESplineCoordinateSpace::Local, false);
		}
		Spiral->UpdateSpline();

		Revolutions = Other->Revolutions;
		StartAngle = Other->StartAngle;
		EndPointRadius = Other->EndPointRadius;
		TransitionExponent = Other->TransitionExponent;
		BaseStep = Other->BaseStep;
		SpiralBaseStepMod = Other->SpiralBaseStepMod;
		MaxPointsNum = Other->MaxPointsNum;

		if (Spiral->GetNumberOfSplinePoints() > Segments.Num())
		{
			CreateSplineMeshes();
		}
		UpdateSplineMeshes();
	}
}

void USpiralSplineMesh::UpdateSpiral(bool UpdateMesh)
{
	UpdateSplinePoints();

	if (UpdateMesh)
	{
		if (Spiral->GetNumberOfSplinePoints() > Segments.Num())
		{
			CreateSplineMeshes();
		}
		UpdateSplineMeshes();
	}
}



USplineComponent* USpiralSplineMesh::GetSpiral() const
{
	return Spiral;
}

TArray<USplineMeshComponent*> USpiralSplineMesh::GetSegments() const
{
	return Segments;
}

void USpiralSplineMesh::SetMesh(UStaticMesh* NewMesh)
{
	if (NewMesh == Mesh) return;

	Mesh = NewMesh;

	for (USplineMeshComponent* segment : Segments)
	{
		segment->SetStaticMesh(Mesh);
	}
}



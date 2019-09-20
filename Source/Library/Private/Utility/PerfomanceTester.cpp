#include "PerfomanceTester.h"
#include "Kismet/KismetSystemLibrary.h"
#include <set>

APerfomanceTester::APerfomanceTester()
{
	MinSize = 100;

	MaxSize = 10000;

	Steps = 10;

	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("root"));
	RootComponent->bVisualizeComponent = true;
}

void APerfomanceTester::BeginPlay()
{
	Super::BeginPlay();
	
	StartTests();
}

void APerfomanceTester::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!InProgress) return;
	CurrentStep++;
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Cyan, TEXT("test ") + FString::FromInt(CurrentStep));

	int StepSize = (MaxSize - MinSize) / Steps;
	int Size = CurrentStep * StepSize;
	PerformTest(Size);
	
	if (CurrentStep > Steps)
	{
		FinishTests();
	}
}

void APerfomanceTester::StartTests()
{
	CurrentStep = 0;
	InProgress = true;
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Cyan, TEXT("Start test"));
	UKismetSystemLibrary::ExecuteConsoleCommand(this, TEXT("stat startfile"));
}

void APerfomanceTester::FinishTests()
{
	InProgress = false;
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Cyan, TEXT("Stop test"));
	UKismetSystemLibrary::ExecuteConsoleCommand(this, TEXT("stat stopfile"));
}



DECLARE_CYCLE_STAT(TEXT("PerfomanceTester ~ Duration"), STAT_Duration, STATGROUP_PerfomanceTester);
DECLARE_DWORD_COUNTER_STAT(TEXT("PerfomanceTester ~ Num"), STAT_Num, STATGROUP_PerfomanceTester);



void APerfomanceTester::PerformTest(int Size)
{
	SCOPE_CYCLE_COUNTER(STAT_Duration);
	SET_DWORD_STAT(STAT_Num, Size);

	int32 Data = 0;
	for (int Index = 0; Index < Size ; Index++)
	{
		Data += FMath::Sqrt(FMath::FRand()*FMath::Rand());
	}
}






void APerfomanceTester::Test()
{
	std::set<int32> Set;
	Set.emplace(1);
	Set.emplace(5);
	Set.emplace(3);
	Set.emplace(2);
	Set.emplace(4);


	FString str = TEXT("Order: ");
	for (auto& Item : Set)
	{
		str += FString::Printf(TEXT(" %d"), Item);
	}

	GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Cyan, str);
}
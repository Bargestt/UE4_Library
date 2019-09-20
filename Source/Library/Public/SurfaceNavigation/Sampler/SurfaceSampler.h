// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SurfaceNavigation.h"
#include "UObject/NoExportTypes.h"

#include "SurfaceSampler.generated.h"

class ASurfaceNavigationVolume;
class USurfaceSamplerBase;

struct FSamplingTask;
struct FSamplerResult;

DECLARE_DELEGATE_OneParam(FSampleFinishedDelegate, FSamplerResult);

DECLARE_DELEGATE_OneParam(FSamplingTaskStateChangedSignature, FSamplingTask*);

struct FSamplerResult
{
	TArray<FVector4> Points;

	FIntVector Dimensions;

	FBox Box;

	FSamplerResult(){}
};


struct SamplingTaskParameters
{
	float VoxelSize;

	bool bDrawDebug;

	bool bSaveInWorldSpace;
	
	SamplingTaskParameters() 
		: VoxelSize(25)
		, bDrawDebug(false)
		, bSaveInWorldSpace(true)
	{}	
};


struct FSamplingTask
{
private:
	FSamplerResult Result;

protected:
	const UWorld* World;

	const SamplingTaskParameters Parameters;

	const FBox SampleBox;

public:
	FSamplingTask(const UWorld* World, const FBox& SampleBox, const SamplingTaskParameters Params = SamplingTaskParameters())
		: World(World)
		, SampleBox(SampleBox)
		, State(TaskState::Waiting)
		, Parameters(Params)
	{ }
	virtual ~FSamplingTask() {
		OnFinish.Unbind();
	}

	void Sample()
	{
		SetState(TaskState::InProgress);
		Result = FSamplerResult();
		SampleData(Result);

		Finish(); //
	}

	/** Sneak peek at result without freeing the task */
	const FSamplerResult& GetResult() const { return Result; }

	/** Grab data and free the task */


	bool IsWaiting() const { return State == TaskState::Waiting; }

	bool IsActive() const { return State == TaskState::InProgress; }

	bool IsDataReady() const { return State == TaskState::DataReady; }

	bool IsDataExtracted() const { return State == TaskState::DataExtracted; }	

protected:

	virtual void SampleData(FSamplerResult& OutResult);

	void Finish()
	{
		SetState(TaskState::DataReady);
		OnFinish.ExecuteIfBound(Result);
		SetState(TaskState::DataExtracted);
	}


	//~ Start State
public:
	enum TaskState { Waiting, InProgress, DataReady, DataExtracted };
private:
	TaskState State;

	FSamplingTaskStateChangedSignature OnStateChanged;

	void SetState(TaskState NewState) { State = NewState; OnStateChanged.ExecuteIfBound(this); }
	TaskState GetState() const { return State; }
	//~ End State


	//~ Start Delegate interface
private:
	friend USurfaceSamplerBase;

	FSampleFinishedDelegate OnFinish;	

	template<class UserClass>
	void BindDelegate(UserClass* Object, typename FSampleFinishedDelegate::TUObjectMethodDelegate<UserClass>::FMethodPtr Func)
	{
		OnFinish.Unbind();
		OnFinish.BindUObject(Object, Func);
	}
	template<class FunctionDelegate, class UserClass, typename... VarTypes>
	void BindDelegate(UserClass* Object, typename FunctionDelegate::template TUObjectMethodDelegate<UserClass>::FMethodPtr Func, VarTypes... Vars)
	{
		OnFinish.Unbind();
		OnFinish.BindUObject(Object, Func, Vars...);
	}
};  // FSamplingTask




/**
 * 
 */
UCLASS(collapseCategories, autoExpandCategories=("Dimensions"))
class LIBRARY_API USurfaceSamplerBase : public UObject
{
	GENERATED_BODY()

protected:
	TArray<FSamplingTask> Tasks;


public:
	UPROPERTY(EditAnywhere)
		bool bDrawDebug;

	UPROPERTY(EditAnywhere)
		float VoxelSize = 25;

public:
	USurfaceSamplerBase() {	}

	virtual ~USurfaceSamplerBase() {}

	
	/**
	 * Schedule task
	 * Result should be consumed on delegate call. ID will be invalid with next task scheduled
	 */
	template<class UserClass>
	void ScheduleSampleTask(const FBox& SampleBox, UserClass* Object, typename FSampleFinishedDelegate::TUObjectMethodDelegate<UserClass>::FMethodPtr Func)
	{
		FSamplingTask& Task = SetupTask(SampleBox);
		Task.BindDelegate(Object, Func);
		ScheduleTask(Task);
	}

	/** 
	 * Schedule task with payload
	 * Result should be consumed on delegate call. ID will be invalid with next task scheduled
	 * Template FunctionDelegate - delegate with target method signature
	 */
	template<class FunctionDelegate, class UserClass, typename... VarTypes>
	void ScheduleSampleTask(const FBox& SampleBox, UserClass* Object, typename FunctionDelegate::template TUObjectMethodDelegate<UserClass>::FMethodPtr Func, VarTypes... Vars)
	{
		FSamplingTask& Task = SetupTask(SampleBox);
		Task.BindDelegate<FunctionDelegate>(Object, Func, Vars...);
		ScheduleTask(Task);
	}

protected:
	inline const FSamplingTask& GetTask(int32 TaskID) const { return Tasks[TaskID]; }
	inline FSamplingTask& GetTask(int32 TaskID) { return Tasks[TaskID]; }

	virtual void ScheduleTask(FSamplingTask& Task)
	{		
		Task.Sample();
		UE_LOG(SurfaceNavigation, Warning, TEXT("Scheduled new task. Task queue size: %d"), Tasks.Num());

		ClearFinishedTasks();		
	}

	virtual FSamplingTask& SetupTask(const FBox& SampleBox)
	{
		SamplingTaskParameters Params = SamplingTaskParameters();
		Params.bDrawDebug = bDrawDebug;
		Params.VoxelSize = VoxelSize;				
		
		FSamplingTask& Task = Tasks.Add_GetRef(FSamplingTask(GetWorld(), SampleBox, Params));
		Task.OnStateChanged.BindUObject(this, &USurfaceSamplerBase::TaskStateChanged);
		return Task;
	}

	void TaskStateChanged(FSamplingTask* Task)
	{
		if (Task == nullptr) return;	

		if (Task->IsDataExtracted())
		{
			int32 Num = Tasks.RemoveAll([Task](const FSamplingTask& Entry) { return &Entry == Task; });
		}
	}
	void ClearFinishedTasks()
	{
		Tasks.RemoveAllSwap([](const FSamplingTask& Task) { return Task.IsDataExtracted(); });
	}


};




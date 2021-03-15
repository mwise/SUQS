// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "SuqsQuestState.h"
#include "UObject/Object.h"
#include "SuqsObjectiveState.generated.h"


UENUM(BlueprintType)
enum class ESuqsObjectiveStatus : uint8
{
	/// No progress has been made
	NotStarted = 0,
    /// At least one element of progress has been made
    InProgress = 4,
    /// All mandatory elements have been completed
    Completed = 8,
    /// This item has been failed and cannot be progressed without being explicitly reset
    Failed = 20
};

/**
 * Objective state
 */
UCLASS()
class SUQS_API USuqsObjectiveState : public UObject
{
	GENERATED_BODY()

	friend class USuqsQuestState;
protected:
	/// Whether this objective has been started, completed, failed (quick access to looking at tasks)
	UPROPERTY(BlueprintReadOnly, Category="Objective Status")
	ESuqsObjectiveStatus Status = ESuqsObjectiveStatus::NotStarted;

	/// List of detailed task status
	UPROPERTY(BlueprintReadOnly, Category="Objective Status")
	TArray<USuqsTaskState*> Tasks;

	const FSuqsObjective* ObjectiveDefinition;
	TWeakObjectPtr<USuqsQuestState> ParentQuest;
	TWeakObjectPtr<USuqsPlayState> PlayState;

	int MandatoryTasksNeededToComplete;

	
	void Initialise(const FSuqsObjective* ObjDef, USuqsQuestState* QuestState, USuqsPlayState* Root);
	void Tick(float DeltaTime);
	// Private fail/complete since users should only ever call task fail/complete
	void ChangeStatus(ESuqsObjectiveStatus NewStatus);

public:
	// C++ access
	
	ESuqsObjectiveStatus GetStatus() const { return Status; }
	const TArray<USuqsTaskState*>& GetTasks() { return Tasks; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
    const FName& GetIdentifier() const { return ObjectiveDefinition->Identifier; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
    const FText& GetTitle() const { return ObjectiveDefinition->Title; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
    USuqsQuestState* GetParentQuest() const { return ParentQuest.Get(); }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	const FName& GetBranch() const { return ObjectiveDefinition->Branch; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
    bool AreTasksSequential() const { return ObjectiveDefinition->bSequentialTasks; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
    bool AreAllMandatoryTasksRequired() const { return ObjectiveDefinition->bAllMandatoryTasksRequired; }
	/// Get the additional description to be added to quest description for this objective, if any
	UFUNCTION(BlueprintCallable, BlueprintPure)
    const FText& GetDescription() const;
	/// Return whether an objective is neither complete nor failed 
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsIncomplete() const { return Status != ESuqsObjectiveStatus::Completed && Status != ESuqsObjectiveStatus::Failed; }
	/// Reset the progress on this objective back to the initial state
	UFUNCTION(BlueprintCallable)
    void Reset();
	/// Manually fail this objective. Normally you should fail a specific task, but if you call this then it will mark
	/// the current task(s) failed. Does nothing if there are no outstanding tasks
	UFUNCTION(BlueprintCallable)
    void FailOutstandingTasks();

	/// Get the next incomplete mandatory task required to fulfil this objective
	/// If there are multiple mandatory tasks and ordering doesn't matter, returns the first one found
	UFUNCTION(BlueprintCallable)
	USuqsTaskState* GetNextMandatoryTask() const;

	/// Get a list of tasks which are incomplete
	UFUNCTION(BlueprintCallable)
	void GetIncompleteTasks(TArray<USuqsTaskState*>& IncompleteTasksOut) const;
	/// Get a list of tasks which are completed
	UFUNCTION(BlueprintCallable)
    void GetCompletedTasks(TArray<USuqsTaskState*>& CompletedTasksOut) const;
	/// Get a list of tasks which have been failed
	UFUNCTION(BlueprintCallable)
    void GetFailedTasks(TArray<USuqsTaskState*>& FailedTasksOut) const;

	
	void NotifyTaskStatusChanged();
};

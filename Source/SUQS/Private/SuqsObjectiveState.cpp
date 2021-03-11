#include "SuqsObjectiveState.h"
#include "SuqsPlayState.h"
#include "SuqsTaskState.h"

void USuqsObjectiveState::Initialise(const FSuqsObjective* ObjDef, USuqsQuestState* QuestState,
	USuqsPlayState* Root)
{
	ObjectiveDefinition = ObjDef;
	ParentQuest = QuestState;
	PlayState = Root;

	Status = ESuqsObjectiveStatus::NotStarted;
	MandatoryTasksNeededToComplete = ObjDef->bAllMandatoryTasksRequired ? 0 : 1;

	for (const auto& TaskDef : ObjDef->Tasks)
	{
		auto Task = NewObject<USuqsTaskState>(this);
		Task->Initialise(&TaskDef, this, Root);
		Tasks.Add(Task);

		if (ObjDef->bAllMandatoryTasksRequired && TaskDef.bMandatory)
			++MandatoryTasksNeededToComplete;
	}

	NotifyTaskStatusChanged();
	
}

void USuqsObjectiveState::Tick(float DeltaTime)
{
	for (auto& Task : Tasks)
	{
		Task->Tick(DeltaTime);
	}
	
}


const FText& USuqsObjectiveState::GetDescription() const
{
	switch (Status)
	{
	case ESuqsObjectiveStatus::NotStarted:
	case ESuqsObjectiveStatus::InProgress:
	case ESuqsObjectiveStatus::Failed:
	default:
		return ObjectiveDefinition->DescriptionWhenActive;
	case ESuqsObjectiveStatus::Completed:
		// Don't replace with active def when blank, blank is OK
		return ObjectiveDefinition->DescriptionWhenCompleted;
	}
}

void USuqsObjectiveState::Reset()
{
	for (auto Task : Tasks)
	{
		// This will cause notifications
		Task->Reset();
	}
}

void USuqsObjectiveState::FailOustandingTasks()
{
	for (auto Task : Tasks)
	{
		if (Task->IsIncomplete() && !Task->bHidden)
		{
			Task->Fail();
		}
	}
}

void USuqsObjectiveState::NotifyTaskStatusChanged()
{
	// Re-scan our tasks and decide what this means for our own state
	int MandatoryTasksFailed = 0;
	int MandatoryTasksComplete = 0;
	int IncompleteMandatoryTasks = 0;
	for (auto& Task : Tasks)
	{
		Task->bHidden = false;
		if (Task->IsMandatory())
		{
			switch(Task->Status)
			{
			case ESuqsTaskStatus::Completed:
				++MandatoryTasksComplete;
				break;
			case ESuqsTaskStatus::Failed:
				++MandatoryTasksFailed;
				break;
			case ESuqsTaskStatus::NotStarted:
			case ESuqsTaskStatus::InProgress:
				if (ObjectiveDefinition->bSequentialTasks && IncompleteMandatoryTasks > 0)
				{
					// If tasks are sequential, and this is an incomplete task after the first, suggest hiding it
					Task->bHidden = true;
				}
				++IncompleteMandatoryTasks;
				break;
			default:
				break;
			}
		}
	}
	
	if (MandatoryTasksFailed > 0)
		ChangeStatus(ESuqsObjectiveStatus::Failed);

	if (MandatoryTasksComplete >= MandatoryTasksNeededToComplete)
	{
		ChangeStatus(ESuqsObjectiveStatus::Completed);
	}
	else
	{
		ChangeStatus(MandatoryTasksComplete > 0 ? ESuqsObjectiveStatus::InProgress : ESuqsObjectiveStatus::NotStarted);
	}

}


void USuqsObjectiveState::ChangeStatus(ESuqsObjectiveStatus NewStatus)
{
	if (Status != NewStatus)
	{
		Status = NewStatus;

		switch(NewStatus)
		{
		case ESuqsObjectiveStatus::Completed: 
			PlayState->RaiseObjectiveCompleted(this);
			break;
		case ESuqsObjectiveStatus::Failed:
			PlayState->RaiseObjectiveFailed(this);
			break;
		default: break;
		}

		ParentQuest->NotifyObjectiveStatusChanged();
		PlayState->RaiseQuestUpdated(GetParentQuest());
		
	}
}

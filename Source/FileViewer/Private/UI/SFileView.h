// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EngineMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"


DECLARE_DELEGATE_OneParam(FDbClickDelegate, const FString&);

struct FFileInfo
{
	FString FileName;

	bool IsDirectory;

	FFileInfo(const FString& _InName, const bool& _InDirectory)
		:FileName(_InName), IsDirectory(_InDirectory) {};
};

/**
 * 
 */
class SFileView : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SFileView)
		: _ListDbClicked()
	{}
	SLATE_EVENT(FDbClickDelegate, ListDbClicked)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& Args);
	~SFileView();

	void Refresh_FileList(const FString& InPath);

	void Nav_Parent();

public:
	void On_InfoDbClicked(TSharedRef<FFileInfo> InItem);

	TSharedRef<ITableRow> On_GenerateRow(TSharedRef<FFileInfo> Item, const TSharedRef<STableViewBase>& OwnerTable);

	FDbClickDelegate OnDoubleClick;

private:
	/** The list view */
	TSharedPtr<SListView<TSharedRef<FFileInfo>>>  mv_FileList;

	/** The list data|File infos */
	TArray<TSharedRef<FFileInfo>> marr_FileInofs;

public:
	/** Current path */
	FString mstr_CurPath;
};

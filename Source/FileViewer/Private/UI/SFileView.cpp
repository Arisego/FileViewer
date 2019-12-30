// Fill out your copyright notice in the Description page of Project Settings.

#include "SFileView.h"
#include "FileViewer.h"
#include "GenericPlatform/GenericPlatformFile.h"



class FileListVisitor : public IPlatformFile::FDirectoryVisitor
{
public:
	virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory);

	FileListVisitor(TArray<TSharedRef<FFileInfo>>* _InPtr)
		: mp_FileInofs(_InPtr) {};
private:
	FileListVisitor() = delete;

	/** Data */
	TArray<TSharedRef<FFileInfo>>* mp_FileInofs;
};

bool FileListVisitor::Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory)
{
	if (!mp_FileInofs) return false;

	FString RelativeFilename = FilenameOrDirectory;
	FPaths::MakeStandardFilename(RelativeFilename);
	
	mp_FileInofs->Add(MakeShared<FFileInfo>(FFileInfo(RelativeFilename, bIsDirectory)));
	return true;
}

void SFileView::Construct(const FArguments& InArgs)
{
	mv_FileList =
		SNew(SListView<TSharedRef<FFileInfo>>)
		.SelectionMode(ESelectionMode::Single)
		.ListItemsSource(&marr_FileInofs)
		.OnGenerateRow(this, &SFileView::On_GenerateRow)
		.OnMouseButtonDoubleClick(this, &SFileView::On_InfoDbClicked);

	ChildSlot[mv_FileList.ToSharedRef()];

	this->OnDoubleClick = InArgs._ListDbClicked;
}

SFileView::~SFileView()
{
}

void SFileView::Refresh_FileList(const FString& InPath)
{
	if (FPaths::IsSamePath(InPath, mstr_CurPath)) return;
	if (!FPaths::IsDrive(InPath) && InPath.EndsWith(TEXT("/")))
	{
		mstr_CurPath = FPaths::GetPath(InPath);
	}
	else
	{
		mstr_CurPath = InPath;
	}

	if (FPaths::IsSamePath(FPaths::GetPath(mstr_CurPath), mstr_CurPath))
	{
		mstr_CurPath = FPaths::GetPath(InPath);
	}

	FPaths::MakeStandardFilename(mstr_CurPath);
	UE_LOG(LogFileViewer, Log, TEXT("[SFileView] Refresh_FileList %s|%s"), *InPath, *mstr_CurPath);
	marr_FileInofs.Empty();

	IPlatformFile &PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	FileListVisitor tp_FileVisit(&marr_FileInofs);
	PlatformFile.IterateDirectory(*mstr_CurPath, tp_FileVisit);
	mv_FileList->RequestListRefresh();
}

void SFileView::Nav_Parent()
{
	if (FPaths::IsDrive(mstr_CurPath)) return;

	UE_LOG(LogTemp, Log, TEXT("[SFileView] Nav_Parent(): %s"), *mstr_CurPath);
	Refresh_FileList(FPaths::GetPath(mstr_CurPath));
}

void SFileView::On_InfoDbClicked(TSharedRef<FFileInfo> InItem)
{
	if (InItem->IsDirectory)
	{
		Refresh_FileList(InItem->FileName);
	}
	else
	{
		UE_LOG(LogFileViewer, Log, TEXT("[SFileView] On_InfoDbClicked(): %s is not a directory"), *InItem->FileName);
		OnDoubleClick.ExecuteIfBound(InItem->FileName);
	}
}

TSharedRef<ITableRow> SFileView::On_GenerateRow(TSharedRef<FFileInfo> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow< TSharedRef<FFileInfo> >, OwnerTable)
		[
			SNew(STextBlock)
			.Text(FText::FromString(Item->FileName))
			.Font(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 12))
			.ColorAndOpacity(FLinearColor(1, 0, 1, 1))
			.ShadowColorAndOpacity(FLinearColor::Black)
			.ShadowOffset(FIntPoint(-2, 2))
		];
}
